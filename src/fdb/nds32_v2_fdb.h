#ifndef __NDS32_V2_H__
#define __NDS32_V2_H__

#include <stdint.h>
#include <stdbool.h>
#include "kal_release.h"
#include "config_hw.h"
#include "breakpoints.h"

enum target_debug_reason {
    DBG_REASON_DBGRQ = 0,
    DBG_REASON_BREAKPOINT = 1,
    DBG_REASON_WATCHPOINT = 2,
    DBG_REASON_SINGLESTEP = 3,
    DBG_REASON_UNDEFINED = 4
};

enum nds32_debug_reason {
    NDS32_DEBUG_BREAK = 0,
    NDS32_DEBUG_BREAK_16,
    NDS32_DEBUG_INST_BREAK,
    NDS32_DEBUG_DATA_ADDR_WATCHPOINT_PRECISE,
    NDS32_DEBUG_DATA_VALUE_WATCHPOINT_PRECISE,
    NDS32_DEBUG_DATA_VALUE_WATCHPOINT_IMPRECISE,
    NDS32_DEBUG_DEBUG_INTERRUPT,
    NDS32_DEBUG_HARDWARE_SINGLE_STEP,
    NDS32_DEBUG_DATA_ADDR_WATCHPOINT_NEXT_PRECISE,
    NDS32_DEBUG_DATA_VALUE_WATCHPOINT_NEXT_PRECISE,
    NDS32_DEBUG_LOAD_STORE_GLOBAL_STOP,
};

enum target_state {
    TARGET_RUNNING = 0,
    TARGET_HALTED = 1,
};

enum nds32_v2_context{
    PC,
    PSW,
    R28,
    R29,
    R30,
    R15,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R0,
    R1,
    R2,
    R31,
    CONTEXT_NUM
};

extern int (* nds32_v2_fdb_init)();
extern int (* nds32_v2_fdb_disable)();
extern void (* debug_exception_handler)(void);
extern int (* nds32_v2_list_breakpoint)(uint32_t *val);
extern int (* nds32_v2_get_debug_reason)();
extern int (* nds32_v2_activate_hardware_breakpoint)();
extern int (* nds32_v2_deactivate_hardware_breakpoint)();
extern int (* nds32_v2_debug_entry)();
extern int (* nds32_v2_leave_debug_state)();
extern int (* nds32_v2_add_breakpoint)(uint8_t index, uint32_t address);
extern int (* nds32_v2_enable_breakpoint)(uint8_t index);
extern int (* nds32_v2_disable_breakpoint)(uint8_t index);
extern int (* nds32_v2_remove_breakpoint)(uint8_t index);
extern int (* nds32_v2_add_watchpoint)(uint8_t index, uint32_t address, enum watchpoint_rw rw);
extern int (* nds32_v2_breakpoint_range)(uint8_t index, uint32_t size);
extern int (* nds32_v2_set_reg)(uint32_t reg, uint32_t val);
extern int (* nds32_v2_get_reg)(uint32_t reg, uint32_t *val);
extern int (* nds32_v2_stop)();
extern int (* nds32_v2_continue)();
extern int (* nds32_v2_single_step)();
extern int (* debug_exception_info)();
extern int (* nds32_v2_read_mem)(uint32_t addr, uint8_t len, uint32_t *val);
extern int (* nds32_v2_write_mem)(uint32_t addr, uint32_t val, uint8_t len);
extern int (* nds32_v2_function_call)(uint32_t addr, uint32_t arg0, uint32_t *val);

#define NDS32_V2_EDM_BCP_WP_POS      0
#define NDS32_V2_EDM_BCP_WP_MSK      (0x1 << NDS32_V2_EDM_BCP_WP_POS)

#define NDS32_V2_EDM_BCP_EL_POS      1
#define NDS32_V2_EDM_BCP_EL_MSK      (0x1 << NDS32_V2_EDM_BCP_EL_POS)

#define NDS32_V2_EDM_BCP_S_POS       2
#define NDS32_V2_EDM_BCP_S_MSK       (0x1 << NDS32_V2_EDM_BCP_S_POS)

#define NDS32_V2_EDM_BCP_BE0_POS     5
#define NDS32_V2_EDM_BCP_BE0_MSK     (0x1 << NDS32_V2_EDM_BCP_BE0_POS)

#define NDS32_V2_EDM_BCP_BE1_POS     6
#define NDS32_V2_EDM_BCP_BE1_MSK     (0x1 << NDS32_V2_EDM_BCP_BE1_POS)

#define NDS32_V2_EDM_BCP_BE2_POS     7
#define NDS32_V2_EDM_BCP_BE2_MSK     (0x1 << NDS32_V2_EDM_BCP_BE2_POS)

#define NDS32_V2_EDM_BCP_BE3_POS     8
#define NDS32_V2_EDM_BCP_BE3_MSK     (0x1 << NDS32_V2_EDM_BCP_BE3_POS)

#endif  /* __NDS32_V2_H__ */
