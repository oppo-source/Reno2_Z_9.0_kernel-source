#ifndef DEBUG_PROBE_H
#define DEBUG_PROBE_H

#include <stdint.h>

extern void debug_probe_init();
extern void (* debug_probe_handler)() __attribute__ ((unused));
extern void (* debug_probe_handler_post)() __attribute__ ((unused));
extern uint32_t get_debug_probe_cmd();


#define CONFIG_HIF_TYPE_PCIE 0
#define CONFIG_HIF_TYPE_SDIO 1
#define CONFIG_HIF_TYPE_INTERNAL_BUS 2


/* MCU Debug Prob */
#define IRQ_MCU_DBG_IF_SW_IRQ_CODE  (31)    /* mcusys_n9-mcu_dbg_if_sw_irq_b    */

#if 0
#define DI_CMD_REG      0x8206c068
#define DI_ARG0_REG     0x8206c06c
#define DI_ARG1_REG     0x8206c070
#define DI_RET0_REG     DI_ARG0_REG
#define DI_RET1_REG     DI_ARG1_REG
#define DI_STATUS_REG   0x8206c064
#endif

//#if (CONFIG_HIF_TYPE == CONFIG_HIF_TYPE_PCIE)
#if 1
#define DI_CMD_REG      CONN_MCU_CONFG_MCU_DBG_IF_CMD_ADDR
#define DI_ARG0_REG     CONN_MCU_CONFG_MCU_DBG_IF_ARG0_ADDR
#define DI_ARG1_REG     CONN_MCU_CONFG_MCU_DBG_IF_ARG1_ADDR
#define DI_RET0_REG     DI_ARG0_REG
#define DI_RET1_REG     DI_ARG1_REG
#define DI_STATUS_REG   CONN_MCU_CONFG_ON_MCU_DBG_IF_STATUS_ADDR

#else
#define DI_CMD_REG      SDIO_FW_HCFH2DRM2R_H2D_RM2_ADDR
#define DI_ARG0_REG     SDIO_FW_HCFH2DRM3R_H2D_RM3_ADDR
#define DI_ARG1_REG     SDIO_FW_HCFH2DRM4R_H2D_RM4_ADDR
#define DI_RET0_REG     SDIO_FW_HCFD2HSM3R_ADDR
#define DI_RET1_REG     SDIO_FW_HCFD2HSM4R_ADDR
#define DI_STATUS_REG   SDIO_FW_D2HSM2R_D2H_SM2_ADDR

#endif

#define DI_EXTRA_ARG_MAX    4

#define DI_STATUS_CMD_DONE_CNT_ADDR           DI_STATUS_REG
#define DI_STATUS_CMD_DONE_CNT_SHFT           0
#define DI_STATUS_CMD_DONE_CNT_MASK           (0xF << DI_STATUS_CMD_DONE_CNT_SHFT)

#define DI_STATUS_CMD_ERR_CODE_ADDR           DI_STATUS_REG
#define DI_STATUS_CMD_ERR_CODE_SHFT           4
#define DI_STATUS_CMD_ERR_CODE_MASK           (0xF << DI_STATUS_CMD_ERR_CODE_SHFT)

#define DI_STATUS_N9_EXCEPTION_ADDR           DI_STATUS_REG
#define DI_STATUS_N9_EXCEPTION_SHFT           8
#define DI_STATUS_N9_EXCEPTION_MASK           (0x1 << DI_STATUS_N9_EXCEPTION_SHFT)

#define DI_STATUS_N9_SER_ADDR                 DI_STATUS_REG
#define DI_STATUS_N9_SER_SHFT                 9
#define DI_STATUS_N9_SER_MASK                 (0x1 << DI_STATUS_N9_SER_SHFT)

#define DI_STATUS_N9_ASSERT_ADDR              DI_STATUS_REG
#define DI_STATUS_N9_ASSERT_SHFT              10
#define DI_STATUS_N9_ASSERT_MASK              (0x1 << DI_STATUS_N9_ASSERT_SHFT)

#define DI_STATUS_TARGET_STATE_ADDR           DI_STATUS_REG
#define DI_STATUS_TARGET_STATE_SHFT           31
#define DI_STATUS_TARGET_STATE_MASK           (0x1 << DI_STATUS_TARGET_STATE_SHFT)

#define DI_STATUS_TARGET_DEBUG_REASON_ADDR    DI_STATUS_REG
#define DI_STATUS_TARGET_DEBUG_REASON_SHFT    28
#define DI_STATUS_TARGET_DEBUG_REASON_MASK    (0x7 << DI_STATUS_TARGET_DEBUG_REASON_SHFT)

enum DI_CMD {

    /* common debug interface command */
    DIC_READ1 = 0x1001,
    DIC_READ2,
    DIC_READ4,
    DIC_WRITE1,
    DIC_WRITE2,
    DIC_WRITE4,
    DIC_GET_BUF,
    DIC_SELF_TEST_CONST5E,
    DIC_SELF_TEST_CONSTA1,
    DIC_SELF_TEST_ADD1,
    DIC_SET_TEST,
    DIC_SET_TEST_BH,
    DIC_BREAK_ASSERT_EXCEPTION_LOOP,
    DIC_HOOK_DIC_CORE_DUMP,
    DIC_COMMON_CMD1_END = 0x1100,

    /* FDB debug interface command */
    DIC_FDB_ENABLE = 0x1101,
    DIC_FDB_DISABLE,
    DIC_FDB_HW_BKPT_SET,
    DIC_FDB_HW_BKPT_DELETE,
    DIC_FDB_HW_BKPT_ENABLE,
    DIC_FDB_HW_BKPT_DISABLE,
    DIC_FDB_CONTINUE,
    DIC_FDB_STEP,
    DIC_FDB_STOP,
    DIC_FDB_GET_CPU_REG,
    DIC_FDB_SET_CPU_REG,
    DIC_FDB_GET_TASK,
    DIC_FDB_GET_STACK,
    DIC_FDB_WATCH,
    DIC_FDB_RWATCH,
    DIC_FDB_AWATCH,
    DIC_FDB_WATCH_RANGE,
    DIC_FDB_WATCH_SYMBOL,
    DIC_FDB_WATCH_EXPR_CONST,
    DIC_FDB_WATCH_EXPR_VARIABLE,
    DIC_FDB_WATCH_EXPR_DONE,
    DIC_FDB_WATCH_ENABLE,
    DIC_FDB_WATCH_DISABLE,
    DIC_FDB_WATCH_DELETE,
    DIC_FDB_FUNC_CALL,
    DIC_FDB_SET_ARG,
    DIC_FDB_FUNC_CALL_BH,       /* function call bottom half */
    DIC_FDB_CMD1_END = 0x1200,

    /* System debug interface commnad */
    DIC_SYSTEM_STACK_INFO = 0x1201,
    DIC_SYSTEM_QUEUE_INFO,
    DIC_SYSTEM_CMD1_END = 0x1f00,

    /* For fdb cmd2 */
    DIC_FDB_WMT_TEST = 0x1f01,
    DIC_FDB_BKPT_INFO,
    DIC_FDB_CMD2_END = 0x2000,

    /* For WiFi */
    DIC_WIFI_CMD1_START = 0x2001,
    DIC_WIFI_CMD1_END = 0x3000,

    /* For BT */
    DIC_BT_CMD1_START = 0x3001,
    DIC_BT_CMD1_END = 0x4000,

    /* For GPS */
    DIC_GPS_CMD1_START = 0x4001,
    DIC_GPS_CMD1_END = 0x5000,

    /* For FM */
    DIC_FM_CMD1_START = 0x5001,
    DIC_FM_CMD1_END = 0x6000
};

enum DI_CMD_ERR_CODE {
    ERR_OK,
    ERR_FAIL,
    ERR_TARGET_ACCESS_REG_FAIL,
    ERR_TARGET_NOT_HALTED,
    ERR_TARGET_NOT_RUNNING,
    ERR_TARGET_UNALIGNED_ACCESS,
    ERR_TARGET_RESOURCE_NOT_AVAILABLE,
    ERR_TARGET_INVALID_CMD,
    ERR_TARGET_FDB_PROTECT,
    ERR_TARGET_FDB_DISABLE,
    ERR_NUM
};

#define u_intHW u_int32

#define MASK(Field)     (Field##_MASK)
#define SHFT(Field)     (Field##_SHFT)

#define mSetVariableEntry(Variable, Field, Value) \
   { Variable = ((((u_intHW)(Value) << SHFT(Field)) \
                & MASK(Field)) | (Variable & ~MASK(Field))) ; }

#define mGetVariableEntry(Variable, Field) \
  ( ( Variable & (MASK(Field)) ) >> SHFT(Field) )


#endif /* DEBUG_PROBE_H */
