#ifndef WMT_IF_H
#define WMT_IF_H

#define CONN_MCU_CIRQ_IRQ_SOFT_ADDR         (0x50000 | 0x0C0)   /* reg index | offset*/
#define CONN_MCU_CONFG_MCU_DBG_IF_CMD_ADDR      (0x00000 | 0x0510)
#define CONN_MCU_CONFG_MCU_DBG_IF_ARG0_ADDR     (0x00000 | 0x0514)
#define CONN_MCU_CONFG_MCU_DBG_IF_ARG1_ADDR     (0x00000 | 0x0518)
#define CONN_MCU_CONFG_ON_MCU_DBG_IF_STATUS_ADDR    (0x40000 | 0x004)

uint32_t wmt_read_cr(uint32_t addr);
void wmt_write_cr(uint32_t addr, uint32_t val);

#endif /* WMT_IF_H */
