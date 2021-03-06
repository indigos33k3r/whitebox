#include <linux/dma-mapping.h>

#include "whitebox.h"
#include "whitebox_block.h"

#include "pdma.h"

#define d_printk(level, fmt, args...)				\
	if (whitebox_debug >= level) printk(KERN_INFO "%s: " fmt,	\
					__func__, ## args)

void whitebox_rf_sink_init(struct whitebox_rf_sink *rf_sink,
        int dma_ch, void (*dma_cb)(void*), void *dma_cb_data,
        struct whitebox_exciter *exciter)
{
    spin_lock_init(&rf_sink->lock);
    rf_sink->off = 0;
    rf_sink->dma_ch = dma_ch;
    rf_sink->dma_cb = dma_cb;
    rf_sink->dma_cb_data = dma_cb_data;
    rf_sink->exciter = exciter;
    d_printk(3, "\n");
}

int whitebox_rf_sink_alloc(struct whitebox_rf_sink *rf_sink)
{
    return pdma_request(rf_sink->dma_ch,
            (pdma_irq_handler_t)rf_sink->dma_cb,
            rf_sink->dma_cb_data,
            10,
            rf_sink->exciter->pdma_config);
}

void whitebox_rf_sink_free(struct whitebox_rf_sink *rf_sink)
{
    pdma_release(rf_sink->dma_ch);
}

size_t whitebox_rf_sink_space_available(struct whitebox_rf_sink *rf_sink,
        unsigned long *dest)
{
    size_t count;
    if (pdma_buffers_available(rf_sink->dma_ch) > 0)
        count = rf_sink->exciter->ops->space_available(rf_sink->exciter, dest);
    else {
        d_printk(7, "txen %d buffs-avail %d\n",
                rf_sink->exciter->ops->get_state(rf_sink->exciter) & WES_TXEN,
                pdma_buffers_available(rf_sink->dma_ch));
        count = 0;
    }
    return count;
}

int whitebox_rf_sink_produce(struct whitebox_rf_sink *rf_sink, size_t count)
{
    d_printk(1, "%d\n", count);
    rf_sink->off += count;
    return rf_sink->exciter->ops->produce(rf_sink->exciter, count);
}

int whitebox_rf_sink_work(struct whitebox_rf_sink *rf_sink,
        unsigned long src, size_t src_count,
        unsigned long dest, size_t dest_count)
{
    size_t count = min(src_count, dest_count);
    dma_addr_t mapping;
    int buf;

    // If empty, just return
    if (count >> 2 == 0)
        return -1;

    d_printk(3, "work start\n");

    // If there's less than a quantum at the source, do it in a tight loop.
    /*if (rf_sink->dma_count < rf_sink->exciter->quantum) {
        int i;
        for (i = 0; i < rf_sink->dma_count >> 2; ++i) {
            if (!rf_sink->exciter->incr_dest) {
                *(u32*)dest = ((u32*)src)[i];
            } else {
                ((u32*)dest)[i] = ((u32*)src)[i];
            }
        }
        return rf_sink->dma_count;
    }*/

    // Else, use the DMA to transfer the data

    d_printk(1, "src=%08lx src_count=%zu dest=%08lx dest_count=%zu\n",
            src, src_count, dest, dest_count);

    mapping = dma_map_single(NULL,
            (void*)src, count, DMA_TO_DEVICE);
    if (dma_mapping_error(NULL, mapping)) {
        d_printk(0, "failed to map dma\n");
        return -EFAULT;
    }

    if ((buf = pdma_start(rf_sink->dma_ch,
            mapping,
            dest,
            count >> 2)) < 0) {
        d_printk(0, "failed to start dma\n");
        return -EFAULT;
    }
    d_printk(1, "pdma started count=%d txen=%d\n",
        count,
        rf_sink->exciter->ops->get_state(rf_sink->exciter) & WES_TXEN);
    rf_sink->dma[buf].count = count;
    rf_sink->dma[buf].mapping = mapping;
    d_printk(3, "work finish\n");
    return 0;
}

int whitebox_rf_sink_work_done(struct whitebox_rf_sink *rf_sink, int buf)
{
    d_printk(1, "pdma finish\n");
    dma_unmap_single(NULL, rf_sink->dma[buf].mapping,
            rf_sink->dma[buf].count, DMA_TO_DEVICE);
    return (int)rf_sink->dma[buf].count;
}
