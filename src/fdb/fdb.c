#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "nds32_intrinsic.h"
#include "debug_probe.h"
#include "wmt_if.h"
#include "nds32_v2_fdb.h"

//#define DI_IRQ_REG 0x81040118
//#define DI_IRQ_REG 0x81040118

#define DI_IRQ_REG CONN_MCU_CIRQ_IRQ_SOFT_ADDR

#define CMD_STR_LEN 100
#define WAIT_RET_US 10000
#define WAIT_HALT_US 1000000
#define CMD_WAIT_CNT_MAX 30

#define HW_BKPT_NUM 4
#define STACK_DEPTH 100

#define PRINT_T32_MSG       1
#define PRINT_IWPRIV_MSG    0
#define PRINT_INFO_MSG      0
#define PRINT_DBG_MSG       0
#define PRINT_SCRIPT_MSG    0

#define MAX_CMM_BUFFER 64

#define NON_PREFIX_PRINTF(prefix, fmt, arg...) \
    if (PRINT_##prefix##_MSG)    \
        printf(fmt , ## arg)





//ziv
#if 1
#define PREFIX_PRINTF(prefix, fmt, arg...) \
    if (PRINT_##prefix##_MSG)   \
        printf("[" #prefix "] " fmt , ## arg)
#endif
#if 0
#define PREFIX_PRINTF(prefix, fmt, arg...) \
    if (PRINT_##prefix##_MSG)   \
        printf(fmt , ## arg)
#endif

#if 1
#define CMM_PRINTF(prefix, fmt, arg...) \
    if (1){ \
        uint32_t len; \
        len = snprintf(cmm_buf, MAX_CMM_BUFFER, fmt , ## arg); \
        fwrite(cmm_buf, 1, len, file); \
    }

#endif

#if 0
#define PRINT_CONTEXT_CR_T32(srname)  { \
                                    intern_debug_probe_cmd_call(DIC_FDB_GET_CPU_REG, (uintptr_t)#srname, 0); \
                                    PREFIX_PRINTF(T32, "R.S " #srname " 0x%x\n", ret0); \
                                }
#endif

#if 1
#define PRINT_CONTEXT_CR_T32(srname)  { \
                                    intern_debug_probe_cmd_call(DIC_FDB_GET_CPU_REG, (uintptr_t)#srname, 0); \
                                    CMM_PRINTF(T32, "R.S " #srname " 0x%x\n", ret0); \
                                }
#endif


#define SET_SR_ID_IF_MATCH(string, srname, val) \
    if (!strcmp(string, #srname)){  \
        val = srname + CONTEXT_NUM; \
    }

enum available_cpu_state{
    AVAILABLE_RUNNING,
    AVAILABLE_HALTED,
    AVAILABLE_ALL
};

enum intern_cmds_id{
    IC_CALL_TRACE,
    IC_CALL_TARGET_STATE,
    IC_CALL_WAIT_HALTED,
    IC_CALL_HELP,
    IC_NUM
};

struct cmd_line{
    char *exe_name;
    char *cmd;
    uint8_t arg_num;
    char **args;
};

struct di_cmd{
    uint32_t id;
    char *name;
    uint8_t arg_num;
    uint8_t res_num;
    bool interrupt;
    int (* pre_handle)(uint32_t arg0, uint32_t arg1);
    int (* post_handle)(uint32_t arg0, uint32_t arg1);
    enum available_cpu_state available_state;
    char *help_msg;
};

uintptr_t arg0, arg1;
uint32_t ret0, ret1, status, di_cmd_num, cmd_done_cnt;
bool intern_debug_probe_cmd_flag = false;
char *str_tmp;

char screen_buf[256];
char cmm_buf[MAX_CMM_BUFFER];

struct breakpoint breakpoints[HW_BKPT_NUM];

uint32_t read_cr(uint32_t addr);
void write_cr(uint32_t addr, uint32_t val);
uint32_t mcu_cr_mapping(char *mcu_cr_str);
int set_cpu_reg_pre();
int get_cpu_reg_pre();
int get_cpu_reg_post();
int self_test_5e_post();
int single_step_post();
int memory_read_post();
int target_stop_post();
int target_continue_post();
int dump_call_trace();
int print_help_msg();
int show_target_state();
int wait_target_halted();
int fdb_add_breakpoint(uint32_t index, uint32_t address);
int fdb_remove_breakpoint(uint32_t index, uint32_t arg1);
int fdb_list_breakpoint();
int intern_debug_probe_cmd_call(uint32_t cmd_id, uintptr_t _arg0, uintptr_t _arg1);
int check_available_state(struct di_cmd *cmd);
void print_err_msg(uint32_t err);

struct di_cmd di_cmds[] = {
    {
        DIC_READ1,
        "rb",
        1,
        1,
        false,
        NULL,
        memory_read_post,
        AVAILABLE_HALTED,
        "\t\t\t-- read address(byte)"
    },
    {
        DIC_READ2,
        "rh",
        1,
        1,
        false,
        NULL,
        memory_read_post,
        AVAILABLE_HALTED,
        "\t\t\t-- read address(half-word)"
    },
    {
        DIC_READ4,
        "rw",
        1,
        1,
        false,
        NULL,
        memory_read_post,
        AVAILABLE_HALTED,
        "\t\t\t-- read address(word)"
    },
    {
        DIC_WRITE1,
        "wb",
        2,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "\t\t\t-- write address(byte)"
    },
    {
        DIC_WRITE2,
        "wh",
        2,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "\t\t\t-- write address(half-word)"
    },
    {
        DIC_WRITE4,
        "ww",
        2,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "\t\t\t-- write address(word)"
    },
    {
        DIC_GET_BUF,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_SELF_TEST_CONST5E,
        "testc",
        0,
        2,
        true,
        NULL,
        self_test_5e_post,
        AVAILABLE_ALL,
        "\t\t\t-- test debug interface command channel\n"
        "\t\t\t\t   ret0 and ret1 will be 0x5e5e5e5e\n"
    },
    {
        DIC_FDB_ENABLE,
        "enable",
        0,
        0,
        true,
        NULL,
        NULL,
        AVAILABLE_ALL,
        "\t\t\t-- enable fdb"
    },
    {
        DIC_FDB_DISABLE,
        "disable",
        0,
        0,
        true,
        NULL,
        NULL,
        AVAILABLE_ALL,
        "\t\t\t-- disable fdb"
    },
    {
        DIC_FDB_HW_BKPT_SET,
        "hb",
        2,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<index> <addr>\t\t-- set hardware breakpoint"
    },
    {
        DIC_FDB_HW_BKPT_DELETE,
        "d",
        1,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<index>\t\t\t-- delect breakpoint"
    },
    {
        DIC_FDB_HW_BKPT_ENABLE,
        "en",
        1,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<index>\t\t-- enable breakpoint"
    },
    {
        DIC_FDB_HW_BKPT_DISABLE,
        "dis",
        1,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<index>\t\t-- disable breakpoint"
    },
    {
        DIC_FDB_CONTINUE,
        "c",
        0,
        0,
        false,
        NULL,
        target_continue_post,
        AVAILABLE_HALTED,
        "\t\t\t-- continue"
    },
    {
        DIC_FDB_STEP,
        "si",
        0,
        0,
        false,
        NULL,
        single_step_post,
        AVAILABLE_HALTED,
        "\t\t\t-- single step"
    },
    {
        DIC_FDB_STOP,
        "stop",
        0,
        0,
        true,
        NULL,
        target_stop_post,
        AVAILABLE_RUNNING,
        "\t\t\t-- stop request"
    },
    {
        DIC_FDB_GET_CPU_REG,
        "get",
        1,
        1,
        false,
        get_cpu_reg_pre,
        get_cpu_reg_post,
        AVAILABLE_HALTED,
        "<CR name>\t\t-- read MCU register\n"
        "\t\t\t\t   <CR name> list: PC, PSW, R0~R10, R15, R28~R31\n"
        "\t\t\t\t   <CR name> list: NDS32_SR_ITYPE, NDS32_SR_EVA ...\n"
    },
    {
        DIC_FDB_SET_CPU_REG,
        "set",
        2,
        0,
        false,
        set_cpu_reg_pre,
        NULL,
        AVAILABLE_HALTED,
        "<CR name> <value>\t-- set MCU register"
    },
    {
        DIC_FDB_GET_TASK,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_GET_STACK,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_WATCH,
        "watch",
        2,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<index> <addr>\t-- set a write watchpoint"
    },
    {
        DIC_FDB_RWATCH,
        "rwatch",
        2,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<index> <addr>\t-- set a read watchpoint"
    },
    {
        DIC_FDB_AWATCH,
        "awatch",
        2,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<index> <addr>\t-- set a read/write watchpoint"
    },
    {
        DIC_FDB_WATCH_RANGE,
        "range",
        2,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<index> <size>\t-- set a range for braekpoint/watchpoint"
    },
    {
        DIC_FDB_WATCH_SYMBOL,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_WATCH_EXPR_CONST,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_WATCH_EXPR_VARIABLE,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_WATCH_EXPR_DONE,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_WATCH_ENABLE,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_WATCH_DISABLE,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_WATCH_DELETE,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_FDB_FUNC_CALL,
        "call",
        2,
        1,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<addr> <arg0>\t-- function call with a argument immediately\n"
        "\t\t\t\t   arg1 and arg2 can be set by './fdb arg <arg1> <arg2>'\n"
    },
    {
        DIC_FDB_FUNC_CALL_BH,
        "callbh",
        2,
        1,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<addr> <arg0>\t-- function call with a argument by wmt task bottom half\n"
        "\t\t\t\t   arg1 and arg2 can be set by './fdb arg <arg1> <arg2>'\n"
    },
    {
        DIC_SYSTEM_STACK_INFO,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_SYSTEM_QUEUE_INFO,
        "",
        0,
        0,
        false,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        ""
    },
    {
        DIC_SET_TEST,
        "test",
        1,
        0,
        true,
        NULL,
        NULL,
        AVAILABLE_ALL,
        "<arg0>\t\t-- enter test env immediately\n"
        "\t\t\t\t   0 = infinite loop, 1 = assert, 2 = exception, 3 = r 0x1234\n"
        "\t\t\t\t   4 = show test info\n"
        "\t\t\t\t   5 = read DW variable (dic_test_dw_variable)\n"
        "\t\t\t\t   6 = write DW variable (dic_test_dw_variable)\n"
        "\t\t\t\t   7 = execute a test func (dic_test_func)\n"
    },
    {
        DIC_SET_TEST_BH,
        "testbh",
        1,
        0,
        true,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<arg0>\t\t-- enter test env by wmt task bottom half\n"
        "\t\t\t\t   the argument is as test\n"

    },

    {
        DIC_BREAK_ASSERT_EXCEPTION_LOOP,
        "bl",
        0,
        0,
        true,
        NULL,
        NULL,
        AVAILABLE_ALL,
        "\t\t\t-- break assert or exception infinite loop\n"
    },
    {
        DIC_FDB_BKPT_INFO,
        "info",
        0,
        1,
        false,
        NULL,
        fdb_list_breakpoint,
        AVAILABLE_HALTED,
        "\t\t\t-- breakpoint info"
    },
    {
        DIC_FDB_SET_ARG,
        "arg",
        2,
        0,
        true,
        NULL,
        NULL,
        AVAILABLE_HALTED,
        "<arg0> <arg1>\t\t-- set more arguments"
        "\t\t\t\t   \n"
    },
};

struct di_cmd intern_cmds[] = {
    {
        IC_CALL_TRACE,
        "bt",
        0,
        0,
        false,
        dump_call_trace,
        NULL,
        AVAILABLE_HALTED,
        "\t\t\t-- dump call trace"
    },
    {
        IC_CALL_TARGET_STATE,
        "state",
        0,
        0,
        false,
        show_target_state,
        NULL,
        AVAILABLE_ALL,
        "\t\t\t-- show target state, runing or halted"
    },
    {
        IC_CALL_WAIT_HALTED,
        "wait",
        0,
        0,
        false,
        wait_target_halted,
        NULL,
        AVAILABLE_ALL,
        "\t\t\t-- wait for target halted and show the halt reason"
    },
    {
        IC_CALL_HELP,
        "help",
        0,
        0,
        false,
        print_help_msg,
        NULL,
        AVAILABLE_ALL,
        "\t\t\t-- print this help message"
    },
};

char *cr_mapping[] = {
    "PC",
    "PSW",
    "R28",
    "R29",
    "R30",
    "R15",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R0",
    "R1",
    "R2",
    "R31"
};

char cmd_str[CMD_STR_LEN];
struct cmd_line cmd_line;

int fdb_list_breakpoint()
{
    int i;
    uint8_t index;
    uint32_t target_bp_tbl_addr = ret0;
    uint32_t *breakpoints_p = (uint32_t *)breakpoints;

    printf("Num     Type      Enb Address    range\n");

    for (i = 0; i < (int)(sizeof(breakpoints) / 4); i++){
        intern_debug_probe_cmd_call(DIC_READ4, target_bp_tbl_addr + i * 4, 0);
        breakpoints_p[i] = ret0;
    }

    for (index = 0; index < HW_BKPT_NUM; index ++)
    {
        if (breakpoints[index].used == true)
        {
            printf("  %d        %d        %d 0x%x     0x%x\n",
                    index,
                    breakpoints[index].watchpoint,
                    breakpoints[index].enabled,
                    breakpoints[index].address,
                    breakpoints[index].mask + 1);
        }
    }

    return ERR_OK;
}

uint32_t mcu_cr_mapping(char *mcu_cr_str){
    int i, val = 0xffffffff;

    str_tmp = mcu_cr_str;

    for (i = 0; i < CONTEXT_NUM; i++){
        if (!strcmp(mcu_cr_str, cr_mapping[i])){
            return i;
        }
    }
    /* Configuration System Registers */
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_CPU_VER, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_ICM_CFG, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_DCM_CFG, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_MMU_CFG, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_MSC_CFG, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_CORE_ID, val);
    /* Interrupt System Registers */
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_PSW, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_IPSW, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_IVB, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_EVA, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_P_EVA, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_ITYPE, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_P_ITYPE, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_MERR, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_IPC, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_P_IPC, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_OIPC, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_P_P0, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_P_P1, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_INT_MASK, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_INT_MASK2, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_INT_PEND, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_INT_PEND2, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_INT_PRI, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_INT_PRI2, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_INT_CTRL, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_INT_TRIGGER, val);
    /* MMU System Registers */
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_MMU_CTL, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_ILMB, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_DLMB, val);

    /* EDM System Registers */
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_BPA0, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_BPA1, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_BPA2, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_BPA3, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_BPC0, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_BPC1, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_BPC2, val);
    SET_SR_ID_IF_MATCH(mcu_cr_str, NDS32_SR_BPC3, val);
    return val;
}

int get_cpu_reg_pre(){
    arg0 = mcu_cr_mapping((char *)arg0);
    return 0;
}

int set_cpu_reg_pre(){
    arg0 = mcu_cr_mapping((char *)arg0);
    return 0;
}

int self_test_5e_post(){
    printf("ret0 = 0x%x, ret1 = 0x%x\n", ret0, ret1);
    if ((ret0 == 0x5e5e5e5e) && (ret1 == 0x5e5e5e5e))
    {
        printf("self test pass\n");
    }
    else
    {
        printf("self test fail\n");
    }
    return 0;
}

int get_cpu_reg_post(){
    if (!intern_debug_probe_cmd_flag)
        printf("%s = 0x%x\n", str_tmp, ret0);
    return 0;
}

int single_step_post(){
    intern_debug_probe_cmd_call(DIC_FDB_GET_CPU_REG, (uintptr_t)&cr_mapping[PC], 0);
    printf("%s = 0x%x\n", "PC", ret0);
    return 0;
}

int memory_read_post(){
    if (!intern_debug_probe_cmd_flag)
        printf("0x%x = 0x%x\n", (uint32_t)arg0, (uint32_t)ret0);
    return 0;
}

int target_stop_post(){
    show_target_state();
//  intern_debug_probe_cmd_call(DIC_FDB_GET_CPU_REG, (uintptr_t)&cr_mapping[PC], 0);
//  printf("%s = 0x%x\n", "PC", ret0);
    return 0;
}

int target_continue_post(){
    show_target_state();
    return 0;
}

int dump_call_trace(){

    uint32_t i;
    uint32_t *context_sp;
    FILE *file;
    file = fopen("fdb_bt.cmm", "w");

    PRINT_CONTEXT_CR_T32(R28);
    PRINT_CONTEXT_CR_T32(R29);
    PRINT_CONTEXT_CR_T32(R30);
    PRINT_CONTEXT_CR_T32(R15);
    PRINT_CONTEXT_CR_T32(R3);
    PRINT_CONTEXT_CR_T32(R4);
    PRINT_CONTEXT_CR_T32(R5);
    PRINT_CONTEXT_CR_T32(R6);
    PRINT_CONTEXT_CR_T32(R7);
    PRINT_CONTEXT_CR_T32(R8);
    PRINT_CONTEXT_CR_T32(R9);
    PRINT_CONTEXT_CR_T32(R10);
    PRINT_CONTEXT_CR_T32(R0);
    PRINT_CONTEXT_CR_T32(R1);
    PRINT_CONTEXT_CR_T32(R2);
    PRINT_CONTEXT_CR_T32(R31);
    PRINT_CONTEXT_CR_T32(PSW);
    PRINT_CONTEXT_CR_T32(PC);


    intern_debug_probe_cmd_call(DIC_FDB_GET_CPU_REG, (uintptr_t)"R31", 0);

    context_sp = (uint32_t *)(uintptr_t)ret0;

    for (i = 0; i < STACK_DEPTH; i++)
    {
        intern_debug_probe_cmd_call(DIC_READ4, (uintptr_t)&context_sp[i], 0);
        CMM_PRINTF(T32, "D.S D:0x%x %%LE %%LONG 0x%x\n",
                (uint32_t)(uintptr_t)&context_sp[i],
                ret0);
    }
    fclose(file);
    return 0;
}

void write_cr(uint32_t addr, uint32_t val){

    wmt_write_cr(addr, val);
}

uint32_t read_cr(uint32_t addr){

    return wmt_read_cr(addr);
}

void intern_cmd(struct di_cmd *cmd){

    if (check_available_state(cmd) == -1)
        return;

    if (cmd->pre_handle)
        cmd->pre_handle(arg0, arg1);

    /* do something */

    if (cmd->post_handle)
        cmd->post_handle(arg0, arg1);
}

int show_target_state(){

    char *abnormal_str = "debug";

    if (read_cr(DI_STATUS_REG) & DI_STATUS_N9_EXCEPTION_MASK)
        abnormal_str = "exception";

    if (read_cr(DI_STATUS_REG) & DI_STATUS_N9_ASSERT_MASK)
        abnormal_str = "assert";

    if ((read_cr(DI_STATUS_REG) & DI_STATUS_TARGET_STATE_MASK) >>
            DI_STATUS_TARGET_STATE_SHFT == TARGET_RUNNING)
        printf("target is running\n");
    else
        printf("target is %s halted\n", abnormal_str);
    return 0;
}

int wait_target_halted(){

    enum target_debug_reason debug_reason;
    time_t timep;

    time(&timep);

    printf("wait for target halted since %s\n", asctime(gmtime(&timep)));
    while ((read_cr(DI_STATUS_REG) & DI_STATUS_TARGET_STATE_MASK) >>
            DI_STATUS_TARGET_STATE_SHFT == TARGET_RUNNING){
        usleep(WAIT_HALT_US);
    }

    time(&timep);
    printf("target halted at %s\n", asctime(gmtime(&timep)));

    debug_reason = (read_cr(DI_STATUS_REG) & DI_STATUS_TARGET_DEBUG_REASON_MASK) >>
                    DI_STATUS_TARGET_DEBUG_REASON_SHFT;

    printf("halted reason = ");

    switch (debug_reason){
        case DBG_REASON_DBGRQ:
            printf("debug request\n");
            break;

        case DBG_REASON_BREAKPOINT:
            printf("breakpoint\n");
            break;

        case DBG_REASON_WATCHPOINT:
            printf("watchpoint\n");
            break;

        case DBG_REASON_SINGLESTEP:
            printf("single step\n");
            break;

        default:
            printf("undefined\n");
    }

    intern_debug_probe_cmd_call(DIC_FDB_GET_CPU_REG, (uintptr_t)&cr_mapping[PC], 0);
    printf("PC stop at 0x%x\n\n", ret0);

    if (debug_reason == DBG_REASON_BREAKPOINT ||
        debug_reason == DBG_REASON_WATCHPOINT){
        intern_debug_probe_cmd_call(DIC_FDB_BKPT_INFO, 0, 0);
    }

    return 0;
}

uint32_t get_cmd_done_cnt(){
    return read_cr(DI_STATUS_REG) & DI_STATUS_CMD_DONE_CNT_MASK;
}

void increase_cmd_done_cnt(){
    cmd_done_cnt++;
    cmd_done_cnt &= DI_STATUS_CMD_DONE_CNT_MASK;
}

bool is_cmd_done(){
    return ((cmd_done_cnt + 1) & DI_STATUS_CMD_DONE_CNT_MASK) == get_cmd_done_cnt();
}

int check_available_state(struct di_cmd *cmd){

    if (cmd->available_state == AVAILABLE_ALL)
        return 0;

    if ((read_cr(DI_STATUS_REG) & DI_STATUS_TARGET_STATE_MASK) >>
            DI_STATUS_TARGET_STATE_SHFT == TARGET_RUNNING){
        if (cmd->available_state == AVAILABLE_HALTED){
            printf("command \"%s\" should be run in \"halted\" state!\n", cmd->name);
            return -1;
        }
    }
    else
    {
        if (cmd->available_state == AVAILABLE_RUNNING){
            printf("command \"%s\" should be run in \"running\" state!\n", cmd->name);
            return -1;
        }
    }

    return 0;
}

void _debug_probe_cmd(struct di_cmd *cmd){

    uint32_t cmd_wait_cnt;
    uint32_t err_code;

    PREFIX_PRINTF(INFO, "\n");
    PREFIX_PRINTF(INFO, "====== cmd id: 0x%x, cmd name = %s ======\n", cmd->id, cmd->name);

    if (check_available_state(cmd) == -1)
        return;

    if (cmd->pre_handle)
        cmd->pre_handle(arg0, arg1);

    if (cmd->arg_num > 0){
        PREFIX_PRINTF(INFO, "set arg0: 0x%x\n", (uint32_t)arg0);
        write_cr(DI_ARG0_REG, arg0);
    }

    if (cmd->arg_num > 1){
        PREFIX_PRINTF(INFO, "set arg1: 0x%x\n", (uint32_t)arg1);
        write_cr(DI_ARG1_REG, arg1);
    }

    PREFIX_PRINTF(INFO, "set cmd: 0x%x\n", (uint32_t)arg1);
    write_cr(DI_CMD_REG, cmd->id);

    if (cmd->interrupt)
//      write_cr(DI_IRQ_REG, 1);
        write_cr(DI_IRQ_REG, (1 << IRQ_MCU_DBG_IF_SW_IRQ_CODE));

    PREFIX_PRINTF(SCRIPT, "sleep 0.1\n");

    cmd_wait_cnt = 0;

    /* read status CR */
    PREFIX_PRINTF(INFO, "wait cmd done...\n");
    while(!is_cmd_done()){
        PREFIX_PRINTF(DBG, "cmd_done_cnt = 0x%x\n", cmd_done_cnt);
        usleep(WAIT_RET_US);
        if (cmd_wait_cnt > CMD_WAIT_CNT_MAX){
            status = read_cr(DI_STATUS_REG);
            printf("command \"%s\" timeout!!!\n", cmd->name);
            err_code = mGetVariableEntry(status, DI_STATUS_CMD_ERR_CODE);
            printf("status reg: 0x%x, CMD_ERR_CODE: 0x%x\n", status, err_code);
            return;
        }
        cmd_wait_cnt++;
    }
    increase_cmd_done_cnt();

    PREFIX_PRINTF(INFO, "status:\n");
    status = read_cr(DI_STATUS_REG);
    err_code = mGetVariableEntry(status, DI_STATUS_CMD_ERR_CODE);

    if (err_code != ERR_OK)
    {
        printf("status reg: 0x%x\n", status);
        print_err_msg(err_code);
    }

    /* read return CR */
    if (cmd->res_num > 0){
        PREFIX_PRINTF(INFO, "get ret0:\n");
        ret0 = read_cr(DI_RET0_REG);
    }

    if (cmd->res_num > 1){
        PREFIX_PRINTF(INFO, "get ret1:\n");
        ret1 = read_cr(DI_RET1_REG);
    }

    if (cmd->post_handle)
        cmd->post_handle(arg0, arg1);
}

void debug_probe_cmd(struct di_cmd *cmd){
    int i;

    for (i = 2; i < cmd->arg_num; i += 2){
        /* transmit extra args first */
        if (i + 1 >= cmd->arg_num)
            intern_debug_probe_cmd_call(DIC_FDB_SET_ARG, strtoul(cmd_line.args[i], NULL, 16), 0);
        else
            intern_debug_probe_cmd_call(DIC_FDB_SET_ARG, strtoul(cmd_line.args[i], NULL, 16),
                                        strtoul(cmd_line.args[i + 1], NULL, 16));
    }

    _debug_probe_cmd(cmd);
}

int print_help_msg(){
    int i;

    for (i = 0; i < (int)di_cmd_num; i++){
        if (strcmp(di_cmds[i].name, ""))
            printf("%s %s %s\n", cmd_line.exe_name, di_cmds[i].name, di_cmds[i].help_msg);
    }

    for (i = 0; i < IC_NUM; i++){
        printf("%s %s %s\n", cmd_line.exe_name, intern_cmds[i].name, intern_cmds[i].help_msg);
    }
    return 0;
}

void print_err_msg(uint32_t err)
{
    printf("Error!!!\n");
    printf("error reason (0x%x):", err);
    switch(err)
    {
        case ERR_OK:
            printf("ok\n");
            break;
        case ERR_FAIL:
            printf("No classification error\n");
            break;
        case ERR_TARGET_ACCESS_REG_FAIL:
            printf("Access MCU system register fail\n");
            break;
        case ERR_TARGET_NOT_HALTED:
            printf("This cmd should be run in halted state\n");
            break;
        case ERR_TARGET_NOT_RUNNING:
            printf("This cmd should be run in running state\n");
            break;
        case ERR_TARGET_UNALIGNED_ACCESS:
            printf("The address is not aligned\n");
            break;
        case ERR_TARGET_RESOURCE_NOT_AVAILABLE:
            printf("The HW resource is not availabe\n");
            break;
        case ERR_TARGET_INVALID_CMD:
            printf("This cmd is invalid \n");
            break;
        case ERR_TARGET_FDB_PROTECT:
            printf("FDB is protected\n");
            break;
        case ERR_TARGET_FDB_DISABLE:
            printf("FDB is disable\n");
            printf("Please use ./fdb enable first\n");
            break;
        default:
            break;
    }
}

int intern_debug_probe_cmd_call(uint32_t cmd_id, uintptr_t _arg0, uintptr_t _arg1){

    int i;
    uintptr_t arg0_backup;
    uintptr_t arg1_backup;

    /* back argument */
    arg0_backup = arg0;
    arg1_backup = arg1;

    /* perform new command */
    arg0 = _arg0;
    arg1 = _arg1;

    for (i = 0; i < (int)di_cmd_num; i++){
        if(cmd_id == di_cmds[i].id){
            intern_debug_probe_cmd_flag = true;
            debug_probe_cmd(&di_cmds[i]);
            intern_debug_probe_cmd_flag = false;
            break;
        }
    }

    arg0 = arg0_backup;
    arg1 = arg1_backup;

    /* restore argument */
    return 0;
}

int main(int argc, char *argv[]){
    int i;
    bool is_cmd_handled = false;

    cmd_line.exe_name = argv[0];
    cmd_line.cmd = argv[1];
    cmd_line.arg_num = argc;
    cmd_line.args = &argv[2];

    /* if argument not a number, save it's address */
    if (argc >= 3){
        if (isdigit(cmd_line.args[0][0]))
            arg0 = strtoul(cmd_line.args[0], NULL, 16);
        else
            arg0 = (uintptr_t)cmd_line.args[0];
    }

    if (argc >= 4){
        if (isdigit(cmd_line.args[1][0]))
            arg1 = strtoul(cmd_line.args[1], NULL, 16);
        else
            arg1 = (uintptr_t)cmd_line.args[1];
    }

    cmd_done_cnt = get_cmd_done_cnt();
    di_cmd_num = sizeof(di_cmds) / sizeof(struct di_cmd);

    for (i = 0; i < (int)di_cmd_num; i++){
        if(!strcmp(cmd_line.cmd, di_cmds[i].name)){
            debug_probe_cmd(&di_cmds[i]);
            is_cmd_handled = true;
            break;
        }
    }

    for (i = 0; i < IC_NUM; i++){
        if(!strcmp(cmd_line.cmd, intern_cmds[i].name)){
            intern_cmd(&intern_cmds[i]);
            is_cmd_handled = true;
            break;
        }
    }

    if (is_cmd_handled == false)
        printf("error: no such command: %s\n", cmd_line.cmd);

    return 0;
}

