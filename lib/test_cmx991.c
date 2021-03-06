#include <math.h>

#include "cmx991.h"
#include "whitebox.h"

#include "whitebox_test.h"

#define WHITEBOX_DEV "/dev/whitebox"

int test_cmx991_unpack_state(void *data) {
    cmx991_t cmx991;
    cmx991_load(&cmx991, 0x11, 0x8f);
    cmx991_load(&cmx991, 0x14, 0x50);
    cmx991_load(&cmx991, 0x15, 0x14);
    cmx991_load(&cmx991, 0x16, 0x00);
    cmx991_load(&cmx991, 0x20, 0xc0);
    cmx991_load(&cmx991, 0x21, 0xa0);
    cmx991_load(&cmx991, 0x22, 0x08);
    cmx991_load(&cmx991, 0x23, 0x07);

    assert(192 == cmx991_get_m(&cmx991));
    assert(1800 == cmx991_get_n(&cmx991));
    // Assert less than 1 Hz offset from expected value
    assert(fabs(180.0e6 - cmx991_pll_actual_frequency(&cmx991, 19.2e6)) < 1.0);
    return 0;
}

int test_cmx991_pack_unpack(void *data) {
    cmx991_t cmx991;
    cmx991_load(&cmx991, 0x11, 0x8f);
    cmx991_load(&cmx991, 0x14, 0x50);
    cmx991_load(&cmx991, 0x15, 0x14);
    cmx991_load(&cmx991, 0x16, 0x00);
    cmx991_load(&cmx991, 0x20, 0xc0);
    cmx991_load(&cmx991, 0x21, 0xa0);
    cmx991_load(&cmx991, 0x22, 0x08);
    cmx991_load(&cmx991, 0x23, 0x07);

    assert(0x8f == cmx991_pack(&cmx991, 0x11));
    assert(0x50 == cmx991_pack(&cmx991, 0x14));
    assert(0x14 == cmx991_pack(&cmx991, 0x15));
    assert(0x00 == cmx991_pack(&cmx991, 0x16));
    assert(0xc0 == cmx991_pack(&cmx991, 0x20));
    assert(0xa0 == cmx991_pack(&cmx991, 0x21));
    assert(0x08 == cmx991_pack(&cmx991, 0x22));
    assert(0x07 == cmx991_pack(&cmx991, 0x23));
    return 0;
}

int test_cmx991_pll_enable(void *data) {
    cmx991_t cmx991;
    cmx991_init(&cmx991);
    cmx991_pll_enable_m_n(&cmx991, 19.2e6, 192, 1800);
    assert(cmx991_get_m(&cmx991) == 192);
    assert(cmx991_get_n(&cmx991) == 1800);
    assert(cmx991_pll_actual_frequency(&cmx991, 19.2e6) == 180.00e6);
    return 0;
}

int test_cmx991_rf_responds(void *data) {
    int fd;
    whitebox_args_t w;
    cmx991_t cmx991;

    fd = open(WHITEBOX_DEV, O_WRONLY);
    assert(fd > 0);
    cmx991_ioctl_get(&cmx991, &w);

    cmx991.hi_lo = HI_LO_HIGHER;
    cmx991_ioctl_set(&cmx991, &w);
    ioctl(fd, WC_SET, &w);
    ioctl(fd, WC_GET, &w);
    cmx991_ioctl_get(&cmx991, &w);
    assert(cmx991.hi_lo == HI_LO_HIGHER);

    cmx991.hi_lo = HI_LO_LOWER;
    cmx991_ioctl_set(&cmx991, &w);
    ioctl(fd, WC_SET, &w);
    ioctl(fd, WC_GET, &w);
    cmx991_ioctl_get(&cmx991, &w);
    assert(cmx991.hi_lo == HI_LO_LOWER);
    close(fd);
    return 0;
}

int main(int argc, char **argv) {
    whitebox_test_t tests[] = {
        WHITEBOX_TEST(test_cmx991_unpack_state),
        WHITEBOX_TEST(test_cmx991_pack_unpack),
        WHITEBOX_TEST(test_cmx991_pll_enable),
        WHITEBOX_TEST(test_cmx991_rf_responds),
        WHITEBOX_TEST(0),
    };
    return whitebox_test_main(tests, NULL, argc, argv);
}
