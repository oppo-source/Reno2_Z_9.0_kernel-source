/*
* helper for oppo modify stp_dump related operations
*/

#ifndef __OPPO_STP_DUMP_UTILS_H__
#define __OPPO_STP_DUMP_UTILS_H__

#define OPPO_STP_DUMP_DCS_PATH "/data/oppo/log/DCS/de/network_logs/stp_dump/"
#define OPPO_STP_DUMP_DCS_PATH_WITHOUT_SLASH "/data/oppo/log/DCS/de/network_logs/stp_dump"

void tar_dump_and_kernel_log();
void collect_kernel_log(char*);
int generate_random_log_id(char*);

#endif /* __OPPO_STP_DUMP_UTILS_H__ */