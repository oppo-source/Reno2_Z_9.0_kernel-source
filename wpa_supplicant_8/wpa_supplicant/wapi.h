/*
 * Copyright (c) [2017], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#include "l2_packet/l2_packet.h"

#define WLAN_EID_WAPI 68

struct wapi_context {
	struct l2_packet_data *l2_wai;
	u8 bss_wapi_ie[64];
	u8 bss_wapi_ie_len;
};

int wapi_set_suites(struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid,
		    struct wpa_bss *bss, u8 *wpa_ie, size_t *wpa_ie_len);
int wapi_init_l2(struct wpa_supplicant *wpa_s);
void wapi_handle_cert_list_changed(struct wpa_supplicant *wpa_s);
void wapi_event_disassoc(struct wpa_supplicant *wpa_s, const u8 *bssid);
int wapi_init(struct wpa_supplicant *wpa_s);
int wapi_deinit(struct wpa_supplicant *wpa_s);
void wapi_event_assoc(struct wpa_supplicant *wpa_s);
