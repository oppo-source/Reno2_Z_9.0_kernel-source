/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <log/log.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "wmt_ioctl.h"
#include "check_patch.h"
#include "wmt_ioctl.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG    "check_patch"

typedef int(*CP_GET_VERSION_FUNC) (int fd, char *version, int chip_id);

static int get_version_rom(int fd, char *version, int chip_id);
static int get_version_ram(int fd, char *version, int chip_id);
static int get_version_wifi(int fd, char *version, int chip_id);

static const CP_GET_VERSION_FUNC get_version_func[] = {
    [WMT_PATCH_TYPE_ROM] = get_version_rom,
    [WMT_PATCH_TYPE_RAM] = get_version_ram,
    [WMT_PATCH_TYPE_WIFI] = get_version_wifi,
};

static int get_version_rom(int fd, char *version, int chip_id) {
    return get_version_ram(fd, version, chip_id);
}

static int get_version_ram(int fd, char *version, int chip_id) {
    (void)chip_id;
    if (read(fd, version, CP_VERSION_LENGTH) != CP_VERSION_LENGTH) {
        return -1;
    }
    return 0;
}

struct wifi_tailer {
    char date[15];
    char crc[4];
};

static int get_version_wifi_gen3(int fd, char *version) {
    int start;
    unsigned char verno[8];
    unsigned int major, minor, beta;

    /* version number is from the 16th bytes from the last. */
    start = lseek(fd, 0, SEEK_END) - 16;
    if (start < 0) {
         ALOGE("get wifi version fail when seek file 1");
         return -1;
    }

    if (lseek(fd, start, SEEK_SET) < 0) {
         ALOGE("get wifi version fail when seek file 2");
         return -1;
    }

    if (read(fd, &verno, 8) != 8) {
        return -1;
    }

    major = verno[0] + (verno[1] << 8);
    minor = verno[2] + (verno[3] << 8);
    beta = verno[4] + (verno[5] << 8);

    snprintf(version, CP_VERSION_LENGTH, "%03x%03x%04x", major, minor, beta);
    return 0;
}

static int get_version_wifi_soc1(int fd, char *version) {
    int start;

    start = lseek(fd, 0, SEEK_END) - sizeof(struct wifi_tailer);
    if (start < 0) {
         ALOGE("get wifi version fail when seek file 1");
         return -1;
    }

    if (lseek(fd, start, SEEK_SET) < 0) {
         ALOGE("get wifi version fail when seek file 2");
         return -1;
    }

    if (read(fd, version, CP_VERSION_LENGTH) != CP_VERSION_LENGTH) {
        return -1;
    }
    return 0;
}

static int get_version_wifi(int fd, char *version, int chip_id) {
    if (chip_id == 0x6771)
        return get_version_wifi_gen3(fd, version);
    else
        return get_version_wifi_soc1(fd, version);
}

int cp_get_vendor_patch(int wmt_fd, struct wmt_vendor_patch *patch) {
    return ioctl(wmt_fd, WMT_IOCTL_GET_VENDOR_PATCH_VERSION, patch);
}

int cp_get_patch_id(int wmt_fd, char *substr) {
    int num = ioctl(wmt_fd, WMT_IOCTL_GET_VENDOR_PATCH_NUM, 0);
    int i;
    struct wmt_vendor_patch patch;

    for (i = 0; i < num; i++) {
        patch.id = i;
        cp_get_vendor_patch(wmt_fd, &patch);
        if (strstr(patch.file_name, substr) != NULL)
            return i;
    }
    return -1;
}

int cp_get_version(const char *folder_path, const char *file_name, int type, char *version, int chip_id) {
    int ret = -1;
    int fd;
    char path[CP_MAX_PATH_LENGTH];

    snprintf(path, sizeof(path), "%s/%s", folder_path, file_name);

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        ALOGE("get version fail when open %s, %s", path, strerror(errno));
        version[0] = '\0';
        return -1;
    }

    ret = (*get_version_func[type])(fd, version, chip_id);
    close(fd);
    if (ret != 0)
        version[0] = '\0';
    else
        version[CP_VERSION_LENGTH] = '\0';

    return ret;
}

