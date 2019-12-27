#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

enum breakpoint_type {
    BKPT_HARD,
    BKPT_SOFT,
};

enum watchpoint_rw {
    WPT_READ = 0, WPT_WRITE = 1, WPT_ACCESS = 2
};

struct breakpoint{
    uint32_t address;
    uint32_t mask;
    uint32_t value;
    union {
        enum breakpoint_type bkp_imp;
        enum watchpoint_rw wpt_rw;
    } type;
    uint32_t orig_instr;
    bool used;
    bool enabled;
    bool watchpoint;
};

#endif /* BREAKPOINTS_H */
