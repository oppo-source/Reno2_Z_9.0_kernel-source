/************************************************************************************
** File: - android/system/connectivity/wificond/oppo_softap_utils.h
** VENDOR_EDIT
** Copyright (C), 2008-2012, OPPO Mobile Comm Corp., Ltd
**
** Description:
**      mtk coredump utils
**
** Version: 1.0
** Date created: 15:40:09,28/12/2018
** Author: Laixin@PSW.CN.WiFi.Basic.SoftAP
** TAG: PSW.CN.WiFi.Basic.Log.1162004
**
** --------------------------- Revision History: --------------------------------
** 	<author>	<data>			<desc>
**  Laixin      28/12/2018      create
************************************************************************************/

#include <cutils/properties.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "stp_dump.h"
#include "oppo_utils.h"

void collect_kernel_log(char* date) {
    char scommand[128]={""};
    sprintf(scommand,"dmesg > %s/combo_t32_%s.kinfo", OPPO_STP_DUMP_DCS_PATH, date);
    system(scommand);
    stp_printf(MSG_INFO,"collect kernel log wifi dump\n");
}

//
int generate_random_log_id(char* buf) {
    const char *c = "1379abef";
    char *p = buf;
    int n;

    if (buf == NULL) {
        return -1;
    }

    for(n = 0; n < 8; ++n) {
        srand((unsigned)(time(NULL)+n));
        int b = rand()%255;

        switch(n) {
            case 3:
                sprintf(p, "4%x", b%15);
                break;
            case 6:
                sprintf(p, "%c%x", c[rand()%strlen(c)], b%15);
                break;
            default:
                sprintf(p, "%02x", b);
                break;
        }
        p += 2;
    }
    *p = '\0';

    return 0;
}

void tar_dump_and_kernel_log(char* file_name) {
    char tar_command[288]={""};
    char rm_command[128]={""};
    char chmod_command[256]={""};
    char uuid[17];
    char ota_version[PROP_VALUE_MAX];
    char tar_file_path[256]={""};
    char ori_file_path[128]={""};

    if (generate_random_log_id(uuid)) {
        stp_printf(MSG_INFO, "create log id failed!");
        return;
    }

    property_get("ro.build.version.ota", ota_version, "XXXX");
    sprintf(ori_file_path, "%s/combo_t32_%s.*", OPPO_STP_DUMP_DCS_PATH_WITHOUT_SLASH, file_name);
    sprintf(tar_file_path, "%s/connsys_core_dump@%s@%s@helloWorld.tar.gz",
                OPPO_STP_DUMP_DCS_PATH_WITHOUT_SLASH, /* core dump DCS path */
                uuid, /* log id */
                ota_version /* ota version */);
    sprintf(tar_command,"/system/xbin/xkit_10 tar -czvf %s %s",
                tar_file_path, ori_file_path);
    system(tar_command);

    sprintf(rm_command, "rm %s", ori_file_path);
    system(rm_command);

    sprintf(chmod_command, "chown system:system %s", tar_file_path);
    system(chmod_command);

    return;
}