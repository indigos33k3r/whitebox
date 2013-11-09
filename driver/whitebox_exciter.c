#include <asm/io.h>
#include <linux/vmalloc.h>

#include "whitebox_exciter.h"

static int whitebox_rf_exciter_debug = 0;
#define d_printk(level, fmt, args...)				\
	if (whitebox_rf_exciter_debug >= level) printk(KERN_INFO "%s: " fmt,	\
					__func__, ## args)

void _exciter_free(struct whitebox_exciter *exciter)
{
    iounmap(exciter->regs);
}

u32 _exciter_get_state(struct whitebox_exciter *exciter)
{
    u32 state;
    state = WHITEBOX_EXCITER(exciter)->state;
    state = WHITEBOX_EXCITER(exciter)->state;
    return state;
}

void _exciter_set_state(struct whitebox_exciter *exciter, u32 state_mask)
{
    u32 state;
    state = WHITEBOX_EXCITER(exciter)->state;
    state = WHITEBOX_EXCITER(exciter)->state;
    WHITEBOX_EXCITER(exciter)->state = state | state_mask;
}

void _exciter_clear_state(struct whitebox_exciter *exciter, u32 state_mask)
{
    u32 state;
    state = WHITEBOX_EXCITER(exciter)->state;
    state = WHITEBOX_EXCITER(exciter)->state;
    WHITEBOX_EXCITER(exciter)->state = state & ~state_mask;
}

u32 _exciter_get_interp(struct whitebox_exciter *exciter)
{
    u32 interp;
    interp = WHITEBOX_EXCITER(exciter)->interp;
    interp = WHITEBOX_EXCITER(exciter)->interp;
    return interp;
}

void _exciter_set_interp(struct whitebox_exciter *exciter, u32 interp)
{
    WHITEBOX_EXCITER(exciter)->interp = interp;
}

u32 _exciter_get_fcw(struct whitebox_exciter *exciter)
{
    u32 fcw;
    fcw = WHITEBOX_EXCITER(exciter)->fcw;
    fcw = WHITEBOX_EXCITER(exciter)->fcw;
    return fcw;
}

void _exciter_set_fcw(struct whitebox_exciter *exciter, u32 fcw)
{
    WHITEBOX_EXCITER(exciter)->fcw = fcw;
}

u32 _exciter_get_threshold(struct whitebox_exciter *exciter)
{
    u32 threshold;
    threshold = WHITEBOX_EXCITER(exciter)->threshold;
    threshold = WHITEBOX_EXCITER(exciter)->threshold;
    return threshold;
}

void _exciter_set_threshold(struct whitebox_exciter *exciter, u32 threshold)
{
    WHITEBOX_EXCITER(exciter)->threshold = threshold;
}

u32 _exciter_get_runs(struct whitebox_exciter *exciter)
{
    u32 runs;
    runs = WHITEBOX_EXCITER(exciter)->runs;
    runs = WHITEBOX_EXCITER(exciter)->runs;
    return runs;
}

struct whitebox_exciter_operations _exciter_ops = {
    .free = _exciter_free,
    .get_state = _exciter_get_state,
    .set_state = _exciter_set_state,
    .clear_state = _exciter_clear_state,
    .get_interp = _exciter_get_interp,
    .set_interp = _exciter_set_interp,
    .get_fcw = _exciter_get_fcw,
    .set_fcw = _exciter_set_fcw,
    .get_threshold = _exciter_get_threshold,
    .set_threshold = _exciter_set_threshold,
    .get_runs = _exciter_get_runs,
};

int whitebox_exciter_create(struct whitebox_exciter *exciter,
        unsigned long regs_start, size_t regs_size)
{
    exciter->ops = &_exciter_ops;
    exciter->regs = ioremap(regs_start, regs_size);
    if (!exciter->regs) {
        d_printk(0, "unable to map registers for "
            "whitebox exciter base=%08lx\n", regs_start);
        return -EINVAL;
    }
    return 0;
}

void _mock_exciter_free(struct whitebox_exciter *exciter)
{
    struct whitebox_mock_exciter *mock_exciter = 
        container_of(exciter, struct whitebox_mock_exciter, exciter);
    free_pages((unsigned long)mock_exciter->buf.buf, mock_exciter->order);
    vfree(exciter->regs);
}

struct whitebox_exciter_operations _mock_exciter_ops = {
    .free = _mock_exciter_free,
    .get_state = _exciter_get_state,
    .set_state = _exciter_set_state,
    .clear_state = _exciter_clear_state,
    .get_interp = _exciter_get_interp,
    .set_interp = _exciter_set_interp,
    .get_fcw = _exciter_get_fcw,
    .set_fcw = _exciter_set_fcw,
    .get_threshold = _exciter_get_threshold,
    .set_threshold = _exciter_set_threshold,
    .get_runs = _exciter_get_runs,
};

int whitebox_mock_exciter_create(struct whitebox_mock_exciter *mock_exciter,
        size_t regs_size, int order)
{
    struct whitebox_exciter *exciter = &mock_exciter->exciter;
    exciter->ops = &_mock_exciter_ops;
    exciter->regs = vmalloc(regs_size);
    if (!exciter->regs) {
        d_printk(0, "unable to alloc registers for "
            "whitebox mock exciter\n");
        return -EINVAL;
    }

    mock_exciter->order = order;
    mock_exciter->buf_size = PAGE_SIZE << mock_exciter->order;
    mock_exciter->buf.buf = (char*)
            __get_free_pages(GFP_KERNEL | __GFP_DMA | __GFP_COMP |
            __GFP_NOWARN, mock_exciter->order);
    if (!mock_exciter->buf.buf) {
        vfree(exciter->regs);
        d_printk(0, "failed to create port buffer\n");
        return -ENOMEM;
    }
    mock_exciter->buf.head = 0;
    mock_exciter->buf.tail = 0;
    return 0;
}

void whitebox_exciter_destroy(struct whitebox_exciter *exciter)
{
    exciter->ops->free(exciter);
}