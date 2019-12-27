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
#include "wmt_ioctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <log/log.h>
#include <dirent.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <ftw.h>
#include "check_patch.h"
#include <cutils/properties.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG    "check_patch"
//#ifdef VENDOR_EDIT
//Xiaomin.Yang@PSW.CN.BT.Basic.Customize.1586031,2018/12/02, Add for updating wcn firmware by sau_res
#ifndef BT_FW_VERSION_PROPERTY
#define BT_FW_VERSION_PROPERTY "persist.vendor.connsys.patch.version"
#endif

#ifndef BT_FW_DEFAULT_VERSION
#define BT_FW_DEFAULT_VERSION "20081231235959"
#endif

#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 92
#endif
//#endif /* VENDOR_EDIT */

enum cp_error_number {
    CP_ERROR_OK = 0,
    CP_ERROR_DISABLE,
    CP_ERROR_FAIL,
    CP_ERROR_CRC_FAIL,
    CP_ERROR_GET_VERSION_FAIL,
    CP_ERROR_VERSION_OLDER_THAN_VENDOR,
    CP_ERROR_VERSION_OLDER_THAN_ACTIVE,
    CP_ERROR_UPDATE_FOLDER_FAIL,
    CP_ERROR_MAX
};

typedef int(*CP_CHECK_CRC_FUNC) (int fd);

struct ram_patch_hdr {
    unsigned char date[16];
    unsigned char other[30];
    unsigned short crc;
};

struct rom_patch_hdr {
    unsigned char date[16];
    unsigned char other[12];
    unsigned short crc;
};


static int check_crc_rom(int fd);
static int check_crc_ram(int fd);
static int check_crc_wifi(int fd);

static const CP_CHECK_CRC_FUNC check_crc_func[] = {
    [WMT_PATCH_TYPE_ROM] = check_crc_rom,
    [WMT_PATCH_TYPE_RAM] = check_crc_ram,
    [WMT_PATCH_TYPE_WIFI] = check_crc_wifi,
};

static const char *g_folder_path[] = {
    [CP_FOLDER_ACTIVE] = CP_PATH_ACTIVE,
    [CP_FOLDER_PUSH] = CP_PATH_PUSH,
    [CP_FOLDER_TEMP] = CP_PATH_TEMP,
};

//#ifndef VENDOR_EDIT
//Xiaomin.Yang@PSW.CN.BT.Basic.Customize.1586031,2018/12/02, Add for updating wcn firmware by sau_res
/*
static const char *g_error_message[] = {
    [CP_ERROR_OK] = "success",
    [CP_ERROR_DISABLE] = "disable",
    [CP_ERROR_FAIL] = "fail",
    [CP_ERROR_CRC_FAIL] = "check crc fail",
    [CP_ERROR_GET_VERSION_FAIL] = "get version fail",
    [CP_ERROR_VERSION_OLDER_THAN_VENDOR] = "version older than vendor",
    [CP_ERROR_VERSION_OLDER_THAN_ACTIVE] = "version older than active",
    [CP_ERROR_UPDATE_FOLDER_FAIL] = "update active folder fail",
};
*/
//#else /* VENDOR_EDIT */
static const char *g_error_message[] = {
    [CP_ERROR_OK] = "success",
    [CP_ERROR_DISABLE] = "disable",
    [CP_ERROR_FAIL] = "fail",
    [CP_ERROR_CRC_FAIL] = "check_crc_fail",
    [CP_ERROR_GET_VERSION_FAIL] = "get_version_fail",
    [CP_ERROR_VERSION_OLDER_THAN_VENDOR] = "version_older_than_vendor",
    [CP_ERROR_VERSION_OLDER_THAN_ACTIVE] = "version_older_than_active",
    [CP_ERROR_UPDATE_FOLDER_FAIL] = "update_active_folder_fail",
};
//#endif /* VENDOR_EDIT */

static int g_wmt_fd = -1;
static int g_chip_id = -1;
static int g_wait_reset = 0;

static const char *get_folder_path(int folder) {
    return g_folder_path[folder];
}

static void open_wmt_device() {
    do {
        g_wmt_fd = open(CP_WMT_DEV, O_RDWR | O_NOCTTY);
        if (g_wmt_fd < 0) {
            ALOGE("Can't open wmt driver, error:%s", strerror(errno));
            usleep(300000);
        } else {
            g_chip_id = ioctl(g_wmt_fd, WMT_IOCTL_WMT_QUERY_CHIPID, NULL);
            break;
        }
    }while(1);

    ALOGI("open wmt driver succeed. fd:%d chip_id:%d", g_wmt_fd, g_chip_id);
}

static int get_patch_num(){
    return ioctl(g_wmt_fd, WMT_IOCTL_GET_VENDOR_PATCH_NUM, 0);
}

unsigned short checksume16(unsigned char *pData, unsigned long len) {
    int sum = 0;

    while (len > 1) {
        sum += *((unsigned short*)pData);
        pData = pData + 2;

        if (sum & 0x80000000) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        len -= 2;
    }

    if (len) {
        sum += *((unsigned char*)pData);
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

unsigned int crc32(unsigned char *buf, unsigned int len) {
    unsigned int i, crc32 = 0xFFFFFFFF;
    const unsigned int crc32_ccitt_table[256] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
        0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
        0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
        0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
        0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
        0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
        0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
        0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
        0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
        0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
        0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
        0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
        0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
        0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
        0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
        0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
        0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
        0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
        0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
        0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
        0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
        0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
        0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
        0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
        0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
        0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
        0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
        0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
        0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
        0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
        0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
        0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
        0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
        0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
        0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
        0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
        0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
        0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
        0x2d02ef8d };

    for (i = 0; i < len; i++)
        crc32 = crc32_ccitt_table[(crc32 ^ buf[i]) & 0xff] ^ (crc32 >> 8);

    return ( ~crc32 );
}


static int check_crc(int folder, char *file_name, int type) {
    char path[CP_MAX_PATH_LENGTH];
    int fd, ret;

    snprintf(path, sizeof(path), "%s%s",
             get_folder_path(folder), file_name);

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        ALOGE("check crc fail when open %s, %s", path, strerror(errno));
        return -1;
    }

    ret = (*check_crc_func[type])(fd);
    close(fd);
    return ret;
}

static int read_file(int fd, unsigned char *buffer, int length) {
    int ret;
    while(length > 0) {
        ret = read(fd, buffer, length);
        if (ret > 0) {
            length -= ret;
            buffer += ret;
        } else if (ret == 0) {
            return 0;
        } else {
            return -1;
        }
    }
    return 0;
}

static int check_crc_helper(int fd, int type) {
    unsigned char *buffer;
    int size;
    int header_size;
    unsigned short crc, hdr_crc;

    /* get size of data to check */
    if (type == WMT_PATCH_TYPE_ROM)
        header_size = sizeof(struct rom_patch_hdr);
    else if (type == WMT_PATCH_TYPE_RAM)
        header_size = sizeof(struct ram_patch_hdr);
    else
        return -1;

    size = lseek(fd, 0, SEEK_END) - header_size;
    if (size < 0) {
        ALOGE("file is corrupted, size = %d", size);
        return -1;
    }

    buffer = malloc(size);
    if (buffer == NULL) {
        ALOGE("oom when check crc %d", size);
        return -1;
    }

    /* get crc specified in header */
    lseek(fd, 0, SEEK_SET);
    if (read_file(fd, buffer, header_size) < 0) {
        ALOGE("read file error");
        return -1;
    }

    if (type == WMT_PATCH_TYPE_ROM)
        hdr_crc = ((struct rom_patch_hdr *)buffer)->crc;
    else
        hdr_crc = ((struct ram_patch_hdr *)buffer)->crc;

    read_file(fd, buffer, size);
    crc = checksume16(buffer, size);
    free(buffer);

    return crc == hdr_crc ? 0 : -1;
}

static int check_crc_rom(int fd) {
    return check_crc_helper(fd, WMT_PATCH_TYPE_ROM);
}

static int check_crc_ram(int fd) {
    return check_crc_helper(fd, WMT_PATCH_TYPE_RAM);
}

static int check_crc_wifi_gen3(int fd) {
    unsigned char *buffer;
    int size;
    unsigned int crc = 0;
    unsigned int hdr_crc;

    size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        ALOGE("file is corrupted, size = %d", size);
        return -1;
    }

    buffer = malloc(size);
    if (buffer == NULL) {
        ALOGE("oom when check crc, size = %d", size);
        return -1;
    }

    lseek(fd, 0, SEEK_SET);
    read_file(fd, buffer, size);

    /* crc is located in the first 4 to 7 bytes */
    hdr_crc = *((unsigned int *)&buffer[4]);
    /* skip first 8 bytes header */
    crc = crc32(buffer + 8, size - 8);
    free(buffer);

    return crc == hdr_crc ? 0 : -1;
}

static int check_crc_wifi_soc1(int fd) {
    unsigned char *buffer;
    int size;
    unsigned int crc = 0;
    unsigned int hdr_crc;

    size = lseek(fd, 0, SEEK_END) - sizeof(hdr_crc);
    if (size < 0) {
        ALOGE("file is corrupted, size = %d", size);
        return -1;
    }

    buffer = malloc(size);
    if (buffer == NULL) {
        ALOGE("oom when check crc, size = %d", size);
        return -1;
    }

    lseek(fd, 0, SEEK_SET);
    read_file(fd, buffer, size);
    crc = crc32(buffer, size);
    free(buffer);

    if (read(fd, &hdr_crc, sizeof(hdr_crc)) != sizeof(hdr_crc)) {
        ALOGE("read file error");
        return -1;
    }

    return crc == hdr_crc ? 0 : -1;
}

static int check_crc_wifi(int fd) {
    if (g_chip_id == 0x6771)
        return check_crc_wifi_gen3(fd);
    else
        return check_crc_wifi_soc1(fd);
}


static void convert_version_format(char *version) {
    if (version[2] == '0' || (version[2] == '1' && version[3] != '9')) {
        char temp[CP_VERSION_LENGTH];

        memcpy(temp, version, CP_VERSION_LENGTH);
        snprintf(version, CP_VERSION_LENGTH + 1, "20%s", temp);
    }
}

static int compare_version(char *v1, char *v2) {
    char ver1[CP_VERSION_LENGTH + 1];
    char ver2[CP_VERSION_LENGTH + 1];

    memset(ver1, '\0', sizeof(ver1));
    memset(ver2, '\0', sizeof(ver2));
    strncpy(ver1, v1, CP_VERSION_LENGTH + 1);
    strncpy(ver2, v2, CP_VERSION_LENGTH + 1);
    convert_version_format(ver1);
    convert_version_format(ver2);
    return strcmp(ver1, ver2);
}
/**
 * Check a folder to see if it contains all the patches required.
 * In addition, it has to satisfy two conditions
 * 1. all patches have to be non-earlier than patches in the foldes we
 * want to compare.
 * 2. all patches have to pass CRC check. */
static int check_folder(int folder, int *folder_to_compare, int folder_num) {
    int i, j;
    int patch_num = get_patch_num();
    char version[CP_VERSION_LENGTH + 1] = "";
    char version_temp[CP_VERSION_LENGTH + 1] = "";
    struct wmt_vendor_patch patch;
    const char *folder_path;

    ALOGI("check_folder: f: %d num: %d", folder, patch_num);
    for (i = 0; i < patch_num; i++) {
        patch.id = i;
        if (cp_get_vendor_patch(g_wmt_fd, &patch) < 0) {
            ALOGE("fail to get patch: %d", i);
            return CP_ERROR_FAIL;
        }

        if (check_crc(folder, patch.file_name, patch.type) < 0) {
            ALOGE("check crc error: %d %s", folder, patch.file_name);
            return CP_ERROR_CRC_FAIL;
        }

        /* version comparison */
        folder_path = get_folder_path(folder);
        if (cp_get_version(folder_path, patch.file_name, patch.type,
                version, g_chip_id) < 0) {
            ALOGE("get version error: %d %s", folder, patch.file_name);
            return CP_ERROR_GET_VERSION_FAIL;
        }
        ALOGI("version of %s: %s", patch.file_name, version);

        /* version of new patch should be larger than others */
        for (j = 0; j < folder_num; j++) {
            if (folder_to_compare[j] == CP_FOLDER_VENDOR) {
                if (compare_version(version, patch.version) < 0) {
                    ALOGE("version of %d is less than %d: %s",
                        folder, folder_to_compare[j], patch.version);
                    return CP_ERROR_VERSION_OLDER_THAN_VENDOR;
                }
            } else {
                folder_path = get_folder_path(folder_to_compare[j]);
                if (cp_get_version(folder_path, patch.file_name, patch.type,
                    version_temp, g_chip_id) < 0)
                    continue;

                if (compare_version(version, version_temp) < 0) {
                    ALOGE("version of %d is less than %d: %s",
                        folder, folder_to_compare[j], version_temp);
                    return CP_ERROR_VERSION_OLDER_THAN_ACTIVE;
                }
            }
        }
    }
    return 0;
}

static int remove_cb(const char *path, const struct stat *sb,
                         int typeflag, struct FTW *ftwbuf) {
    int ret = remove(path);
    (void)sb;
    (void)typeflag;
    (void)ftwbuf;

    if (ret)
        ALOGE("remove cb error %s %s", path, strerror(errno));

    return ret;
}

static void remove_folder(const char *path, int remove_self) {
    int ret = nftw(path, remove_cb, 32, FTW_DEPTH | FTW_PHYS);
    if (!ret && !remove_self) {
        if (mkdir(path, 0777) != 0) {
            ALOGE("mkdir %s fail: %s", path, strerror(errno));
            return;
        }

        if (chmod(path, 0777) != 0)
            ALOGE("chmod %s fail: %s", path, strerror(errno));
    }
}

static void remove_files_in_folder(int folder) {
    const char *path;
    path = get_folder_path(folder);
    remove_folder(path, 0);
}

static void remove_non_patch_in_folder(int folder) {
    DIR *dir;
    struct dirent *dirent;
    int i, found;
    const char *path;
    char file_path[CP_MAX_PATH_LENGTH];
    int patch_num = get_patch_num();
    struct wmt_vendor_patch patch;

    path = get_folder_path(folder);
    dir = opendir(path);
    if (dir == NULL) {
        ALOGE("opendir fail %s, %s", path, strerror(errno));
        return;
    }

    while (NULL != (dirent = readdir(dir))) {
        found = 0;
        if (!strcmp(dirent->d_name, ".") ||
            !strcmp(dirent->d_name, "..")) {
            continue;
        }

        for (i = 0; i < patch_num; i++) {
            patch.id = i;
            cp_get_vendor_patch(g_wmt_fd, &patch);
            ALOGI("patch %d: %s", i, patch.file_name);
            if (strcmp(dirent->d_name, patch.file_name) == 0) {
                found = 1;
                break;
            }
        }

        if (found == 0) {
            snprintf(file_path, sizeof(file_path), "%s%s", path, dirent->d_name);
            if (dirent->d_type == DT_DIR)
                remove_folder(file_path, 1);
            else
                unlink(file_path);
            ALOGI("nonpatch is found, %s", file_path);
        }
    }
    closedir(dir);
}
static void set_active_version_substr(int folder, char *substr) {
    struct wmt_vendor_patch arg;
    int id = cp_get_patch_id(g_wmt_fd, substr);

    if (id < 0) {
        ALOGE("fail to get patch id");
        return;
    }

    arg.id = id;
    cp_get_vendor_patch(g_wmt_fd, &arg);

    if (folder == CP_FOLDER_VENDOR) {
        arg.id = id;
        ioctl(g_wmt_fd, WMT_IOCTL_SET_ACTIVE_PATCH_VERSION, &arg);
    } else if (folder == CP_FOLDER_ACTIVE) {
        char version[CP_VERSION_LENGTH + 1];
        const char *folder_path = get_folder_path(CP_FOLDER_ACTIVE);

        cp_get_version(folder_path, arg.file_name, arg.type, version, g_chip_id);
        strncpy(arg.version, version, sizeof(arg.version));
        arg.id = id;
        ioctl(g_wmt_fd, WMT_IOCTL_SET_ACTIVE_PATCH_VERSION, &arg);
    }
}

static void set_active_version(int folder) {
    ALOGE("set_active_version %d", folder);
    set_active_version_substr(folder, "ram_bt");
    set_active_version_substr(folder, "patch_mcu");
}


static int update_active_folder() {
    char temp_folder[CP_MAX_PATH_LENGTH];
    int folder[] = {CP_FOLDER_VENDOR};
    const char *active_folder, *push_folder;
    int retry = 0;

    ALOGI("update active folder");
    do {
        snprintf(temp_folder, sizeof(temp_folder), "%s%d",
                 get_folder_path(CP_FOLDER_TEMP), time(NULL) % 10000);
        ALOGI("create temp folder %s", temp_folder);
        retry++;
    } while (mkdir(temp_folder, 0777) < 0 && retry < CP_MAX_RETRY_NUM);

    active_folder = get_folder_path(CP_FOLDER_ACTIVE);
    push_folder = get_folder_path(CP_FOLDER_PUSH);

    remove_non_patch_in_folder(CP_FOLDER_PUSH);

    if (rename(active_folder, temp_folder) < 0) {
        ALOGE("rename active fail %s", strerror(errno));
        return -1;
    }

    if (rename(push_folder, active_folder) < 0) {
        ALOGE("rename push fail %s", strerror(errno));
        return -1;
    }

    if (rename(temp_folder, push_folder) < 0) {
        ALOGE("rename temp fail %s", strerror(errno));
    }
    remove_files_in_folder(CP_FOLDER_PUSH);

    if (check_folder(CP_FOLDER_ACTIVE, folder, 1) < 0) {
        remove_files_in_folder(CP_FOLDER_ACTIVE);
        ALOGE("active folder is invalid after update");
        return -1;
    }

    return 0;
}

static void add_stop_watch(int fd, int *wd_stop) {
    const char *path;
    int retry;

    ALOGI("add stop watch");
    if (wd_stop) {
        path = get_folder_path(CP_FOLDER_TEMP);
        retry = 0;
        do {
            *wd_stop = inotify_add_watch(fd, path, IN_CREATE | IN_DELETE);
            if (*wd_stop < 0) {
                ALOGE("add watch error: wd_s = %d, %s", *wd_stop, strerror(errno));
                mkdir(path, 0777);
                chmod(path, 0777);
                retry++;
            }
        } while(*wd_stop < 0 && retry < CP_MAX_RETRY_NUM);
    }
}

static void add_watch(int fd, int *wd_active, int *wd_push) {
    const char *path;
    int retry;

    ALOGI("add watch");
    if (wd_active) {
        path = get_folder_path(CP_FOLDER_ACTIVE);
        retry = 0;
        do {
            *wd_active = inotify_add_watch(fd, path,
               IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE | IN_ACCESS);
            if (*wd_active < 0) {
                ALOGE("add watch error: wd_a = %d, %s", *wd_active, strerror(errno));
                mkdir(path, 0777);
                chmod(path, 0777);
                retry++;
            }
        } while(*wd_active < 0 && retry < CP_MAX_RETRY_NUM);
    }

    if (wd_push) {
        path = get_folder_path(CP_FOLDER_PUSH);
        retry = 0;
        do {
            *wd_push = inotify_add_watch(fd, path, IN_CREATE);
            if (*wd_push < 0) {
                ALOGE("add watch error: wd_p = %d, %s", *wd_push, strerror(errno));
                mkdir(path, 0777);
                chmod(path, 0777);
                retry++;
            }
        } while(*wd_push < 0 && retry < CP_MAX_RETRY_NUM);
    }
}

static void remove_watch(int fd, int wd_active, int wd_push) {
    ALOGI("remove watch wd_active = %d wd_push = %d", wd_active, wd_push);
    inotify_rm_watch(fd, wd_active);
    inotify_rm_watch(fd, wd_push);
}

static void convert_format_version(char *source, char *dest, int size) {
    memset(dest, 0, size);
    strncpy(dest, source, 4);
    strncat(dest, "-", 1);
    strncat(dest, source+=4, 2);
    strncat(dest, "-", 1);
    strncat(dest, source+=2, 2);
    strncat(dest, "-", 1);
    strncat(dest, source+=2, 6);
}

static void output_version_log() {
    int fd = open(CP_PATH_LOG, O_RDWR | O_CREAT | O_APPEND, 0666);
    char version[20];
    char format_version[30];
    char buffer[50];
//#ifdef VENDOR_EDIT
//Xiaomin.Yang@PSW.CN.BT.Basic.Customize.1586031,2018/12/02,
//Add for updating wcn firmware by sau_res 
    time_t rawtime;
    struct tm * timeinfo;
//#endif /* VENDOR_EDIT */
    int patch_num = get_patch_num();
    struct wmt_vendor_patch patch;
    int i;
    const char *folder_path;

    if (fd < 0)
        return;

    folder_path = get_folder_path(CP_FOLDER_PUSH);
//#ifdef VENDOR_EDIT
//Xiaomin.Yang@PSW.CN.BT.Basic.Customize.1586031,2018/12/02,
//Add for updating wcn firmware by sau_res 
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime(buffer, sizeof(buffer), "[SectionTime=%Y%m%d%H%M%S]\r\n", timeinfo);
    write(fd, buffer, strlen(buffer));
//#endif /* VENDOR_EDIT */
    for (i = 0; i < patch_num; i++) {
        patch.id = i;
        if (cp_get_vendor_patch(g_wmt_fd, &patch) < 0) {
            ALOGE("fail to get patch: %d", i);
            return;
        }

        if (cp_get_version(folder_path, patch.file_name, patch.type,
                version, g_chip_id) < 0) {
            ALOGE("get version error: %s", patch.file_name);
            break;
        }

        if ((strstr(patch.file_name, "be_patch_1_0") > 0) || (strstr(patch.file_name, "ram_bt") > 0)) {
            convert_format_version(version, format_version, sizeof(format_version));
            snprintf(buffer, sizeof(buffer), "BluetoothFWVersion=%s\r\n", format_version);
        } else if (strstr(patch.file_name, "WIFI_RAM") > 0) {
            convert_format_version(version, format_version, sizeof(format_version));
            snprintf(buffer, sizeof(buffer), "WifiFWVersion=%s\r\n", format_version);
        } else {
            continue;
        }
        write(fd, buffer, strlen(buffer));
    }

    close(fd);
}

static void output_ret_log(int ret) {
    int fd;
    char buffer[50];
    time_t rawtime;
    struct tm * timeinfo;

    if (ret < 0 || ret >= CP_ERROR_MAX) {
        ALOGE("output log fail when ret out of bound: %d", ret);
        return;
    }

    fd = open(CP_PATH_LOG, O_RDWR | O_CREAT | O_APPEND, 0666);
    if (fd < 0) {
        ALOGE("output log fail when open file, %s", strerror(errno));
        return;
    }

    snprintf(buffer, sizeof(buffer), "Reason=%s\r\n", g_error_message[ret]);
    write(fd, buffer, strlen(buffer));

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime(buffer, sizeof(buffer), "PushTime=%Y%m%d%H%M%S\r\n", timeinfo);
    write(fd, buffer, strlen(buffer));

//#ifdef VENDOR_EDIT
//Xiaomin.Yang@PSW.CN.BT.Basic.Customize.1586031,2018/12/02,
//Add for updating wcn firmware by sau_res
    if(ret != 0) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      write(fd, buffer, strlen(buffer));
    }
//#endif /* VENDOR_EDIT */

    close(fd);
}

static void output_update_log() {
    int fd = open(CP_PATH_LOG, O_RDWR | O_CREAT | O_APPEND, 0666);
    char buffer[100];
    time_t rawtime;
    struct tm * timeinfo;
//#ifdef VENDOR_EDIT
//Xiaomin.Yang@PSW.CN.BT.Basic.Customize.1586031,2018/12/02,
//Add for updating wcn firmware by sau_res
    char version[20];
    int patch_num = get_patch_num();
    struct wmt_vendor_patch patch;
    int i;
    const char *folder_path;
    char bt_fw_version[PROPERTY_VALUE_MAX] = {0};
//#endif /* VENDOR_EDIT */

    ALOGI("start to write update log");
    if (fd < 0) {
        ALOGE("output log fail when open file, %s", strerror(errno));
        return;
    }

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime(buffer, sizeof(buffer), "UpdateTime=%Y%m%d%H%M%S\r\n", timeinfo);
    if (write(fd, buffer, strlen(buffer)) != strlen(buffer)) {
        ALOGE("error when write update log, log: %s error:%s", strerror(errno));
    }

//#ifdef VENDOR_EDIT
//Xiaomin.Yang@PSW.CN.BT.Basic.Customize.1586031,2018/12/02,
//Add for updating wcn firmware by sau_res
    folder_path = get_folder_path(CP_FOLDER_ACTIVE);
    for (i = 0; i < patch_num; i++) {
        patch.id = i;
        if (cp_get_vendor_patch(g_wmt_fd, &patch) < 0) {
            ALOGE("fail to get patch: %d", i);
            return;
        }

        if (cp_get_version(folder_path, patch.file_name, patch.type,
                version, g_chip_id) < 0) {
            ALOGE("get version error: %s", patch.file_name);
            break;
        }

        if ((strstr(patch.file_name, "be_patch_1_0") > 0) || (strstr(patch.file_name, "ram_bt") > 0)) {
            property_get(BT_FW_VERSION_PROPERTY, bt_fw_version, BT_FW_DEFAULT_VERSION);
            if (strcmp(bt_fw_version, version) == 0) {
               snprintf(buffer, sizeof(buffer), "FirmwareUpdateResult=success_%s__%s\r\n", bt_fw_version, version);
            } else {
               snprintf(buffer, sizeof(buffer), "FirmwareUpdateResult=fail_%s__%s\r\n", bt_fw_version, version);
            }
        } else {
            continue;
        }
        write(fd, buffer, strlen(buffer));
    }

    snprintf(buffer, sizeof(buffer), "\r\n");
    write(fd, buffer, strlen(buffer));
//#endif /* VENDOR_EDIT */

    close(fd);
}

int main(__attribute__((unused))int argc, __attribute__((unused))char *argv[]){
    int fd, wd_a, wd_p, wd_s;
    struct inotify_event *event;
    int folder[] = {CP_FOLDER_VENDOR};
    char buf[256];
    int retry = 0;
    int stop_check_patch = 0;
    struct wmt_vendor_patch patch;

    open_wmt_device();

    do {
        int status = ioctl(g_wmt_fd, WMT_IOCTL_GET_CHECK_PATCH_STATUS);
        if (status >= WMT_CP_READY_TO_CHECK)
            break;

        ALOGI("check patch status %d, times %d", status, retry);
        usleep(300000);
        retry++;
    } while(retry < 50);


    snprintf(buf, sizeof(buf), "%s%s", CP_PATH_TEMP, CP_STOP_LOG);
    if (access(buf, F_OK) != -1) {
        stop_check_patch = 1;
        ALOGI("stop check patch");
    }

    patch.id = 0;
    cp_get_vendor_patch(g_wmt_fd, &patch);

    if (stop_check_patch == 1 || check_folder(CP_FOLDER_ACTIVE, folder, 1) != 0) {
        remove_files_in_folder(CP_FOLDER_ACTIVE);
        set_active_version(CP_FOLDER_VENDOR);
    } else {
        remove_non_patch_in_folder(CP_FOLDER_ACTIVE);
        set_active_version(CP_FOLDER_ACTIVE);
    }
    remove_files_in_folder(CP_FOLDER_PUSH);

    ioctl(g_wmt_fd, WMT_IOCTL_SET_CHECK_PATCH_STATUS, WMT_CP_CHECK_DONE);

    fd = inotify_init();
    if (fd < 0) {
        ALOGE("inotify_init error: %s", strerror(errno));
    }

    add_stop_watch(fd, &wd_s);
    add_watch(fd, &wd_a, &wd_p);

    do {
        ssize_t len = read(fd, buf, sizeof(buf));
        if (len < 0) {
            ALOGE("read watch error: %s", strerror(errno));
            continue;
        } else {
            ALOGI("read event with %d bytes\n", (int)len);
        }

        for (event = (struct inotify_event *)buf;
             (char *)event < &buf[len];
             event = (struct inotify_event *)((char *)event + sizeof(*event) + event->len)) {
            ALOGI("event: wd = %d name = %s mask = %x\n", event->wd,  event->name, event->mask);
            if ((event->wd == wd_s) && (strcmp(event->name, CP_STOP_LOG) == 0)) {
                if (event->mask & IN_CREATE) {
                    ALOGI("detect stop.log\n");
                    remove_files_in_folder(CP_FOLDER_ACTIVE);
                    set_active_version(CP_FOLDER_VENDOR);
                    stop_check_patch = 1;
                } else if (event->mask & IN_DELETE) {
                    stop_check_patch = 0;
                }
            }

            if ((event->wd == wd_p) && (event->mask & IN_CREATE) &&
                (strcmp(event->name, CP_PUSH_LOG) == 0)) {
                int folders[] = {CP_FOLDER_ACTIVE, CP_FOLDER_VENDOR};
                int ret = 0;
                ALOGI("detect push.log\n");
                remove_watch(fd, wd_a, wd_p);
                output_version_log();

                if (stop_check_patch == 1) {
                    ret = CP_ERROR_DISABLE;
                } else {
                    ret = check_folder(CP_FOLDER_PUSH, folders, 2);
                    if (ret == 0) {
                        ret = update_active_folder();
                        if (ret == 0) {
                            set_active_version(CP_FOLDER_ACTIVE);
                            ioctl(g_wmt_fd, WMT_IOCTL_FW_PATCH_UPDATE_RST, 1);
                            g_wait_reset = 1;
                        } else {
                            ret = CP_ERROR_UPDATE_FOLDER_FAIL;
                            set_active_version(CP_FOLDER_VENDOR);
                        }
                    }
                }
                output_ret_log(ret);

                remove_files_in_folder(CP_FOLDER_PUSH);
                add_watch(fd, &wd_a, &wd_p);
                break;
            }

            if ((g_wait_reset == 1) && (event->wd == wd_a) && (event->mask & IN_ACCESS)
                && (strcmp(event->name, patch.file_name) == 0)) {
                ALOGI("file %s is opened", patch.file_name);
                g_wait_reset = 0;
                output_update_log();
            }

            if ((event->wd == wd_a) && ((event->mask & IN_DELETE) || (event->mask & IN_MODIFY)
                || (event->mask & IN_MOVE) || (event->mask & IN_CREATE))) {
                remove_watch(fd, wd_a, wd_p);
                int folders[] = {CP_FOLDER_VENDOR};
                if (stop_check_patch == 1 || check_folder(CP_FOLDER_ACTIVE, folders, 1) != 0) {
                    remove_files_in_folder(CP_FOLDER_ACTIVE);
                    set_active_version(CP_FOLDER_VENDOR);
                }
                else {
                    remove_non_patch_in_folder(CP_FOLDER_ACTIVE);
                }

                add_watch(fd, &wd_a, &wd_p);
            }

            if (event->mask & IN_IGNORED) {
                if (event->wd == wd_a) {
                    add_watch(fd, &wd_a, NULL);
                } else if (event->wd == wd_p) {
                    add_watch(fd, NULL, &wd_p);
                } else if (event->wd == wd_s) {
                    add_stop_watch(fd, &wd_s);
                }
            }

        }
    } while (1);

    return 0;
}

