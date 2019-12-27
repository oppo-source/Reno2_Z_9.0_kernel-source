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

#ifndef CHECK_PATCH_H
#define CHECK_PATCH_H

#define CP_PATH_ACTIVE         "/data/misc/firmware/active/"
#define CP_PATH_PUSH           "/data/misc/firmware/push/"
#define CP_PATH_TEMP           "/data/misc/firmware/"

//#ifndef VENDOR_EDIT
//Xiaomin.Yang@PSW.CN.BT.Basic.Customize.1586031,2018/12/02, Add for updating wcn firmware by sau_res
/*
#define CP_PATH_LOG            "/data/misc/firmware/log.txt"
*/
//#else /* VENDOR_EDIT */
#define CP_PATH_LOG            "/data/misc/firmware/wcn_fw_update_result.conf"
//#endif /* VENDOR_EDIT */

#define CP_PUSH_LOG            "push.log"
#define CP_STOP_LOG            "stop.log"

#define CP_WMT_DEV             "/dev/stpwmt"

#define CP_MAX_PATH_LENGTH     (255)
#define CP_MAX_RETRY_NUM       (5)
#define CP_VERSION_LENGTH      (14)

enum cp_folder {
    CP_FOLDER_ACTIVE = 0, /* where request_firmware will search */
    CP_FOLDER_PUSH,       /* where vendors push patches */
    CP_FOLDER_TEMP,       /* where temp folder is created to update firmware */
    CP_FOLDER_VENDOR,     /* where original firmware resides */
};

int cp_get_vendor_patch(int wmt_fd, struct wmt_vendor_patch *patch);

int cp_get_patch_id(int wmt_fd, char *substr);

/*
 * Read version from file
 * Notice that according to SePolicy rules, system(vendor) process cannot
 * read files in vendor(system) partition
 * @folder_path: folder where file resides in
 * @file_name: file name of the patch
 * @type: enum WMT_PATCH_TYPE_ENUM
 * @version: used to store the version read from file
 * @chip_id: chip id
 */
int cp_get_version(const char *folder_path, const char *file_name, int type,
                   char *version, int chip_id);
#endif

