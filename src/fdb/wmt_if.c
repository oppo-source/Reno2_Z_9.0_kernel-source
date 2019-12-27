#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "wmt_ioctl.h"

#define CUST_COMBO_WMT_DEV       "/dev/stpwmt"

uint32_t wmt_read_cr(uint32_t addr)
{
    int wmt_fd = -1;
    WMT_FDB_CTRL fdb_ctrl;

    wmt_fd = open(CUST_COMBO_WMT_DEV, O_RDWR | O_NOCTTY);
    if (wmt_fd < 0) {
        printf("Can't open device node(%s) error:%d \n", CUST_COMBO_WMT_DEV, wmt_fd);
        return 0;
    }

    fdb_ctrl.is_write = 0;
    fdb_ctrl.offset = addr & 0xffff;
    fdb_ctrl.base_index = (addr & 0xffff0000) >> 16;
    fdb_ctrl.value = 0;
    ioctl(wmt_fd, WMT_IOCTL_FDB_CTRL, &fdb_ctrl);

    if (wmt_fd >= 0) {
        close(wmt_fd);
        wmt_fd = -1;
    }

    return fdb_ctrl.value;
}

void wmt_write_cr(uint32_t addr, uint32_t val)
{
    int wmt_fd = -1;
    WMT_FDB_CTRL fdb_ctrl;

    wmt_fd = open(CUST_COMBO_WMT_DEV, O_RDWR | O_NOCTTY);
    if (wmt_fd < 0) {
        printf("Can't open device node(%s) error:%d \n", CUST_COMBO_WMT_DEV, wmt_fd);
        return;
    }

    fdb_ctrl.is_write = 1;
    fdb_ctrl.offset = addr & 0xffff;
    fdb_ctrl.base_index = (addr & 0xffff0000) >> 16;
    fdb_ctrl.value = val;
    ioctl(wmt_fd, WMT_IOCTL_FDB_CTRL, &fdb_ctrl);

    if (wmt_fd >= 0) {
        close(wmt_fd);
        wmt_fd = -1;
    }
}
