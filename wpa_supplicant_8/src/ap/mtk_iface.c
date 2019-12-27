/*
 * Control interface for shared AP commands
 * Copyright (c) 2004-2014, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"
#include "utils/common.h"
#include "hostapd.h"
#include "ap_drv_ops.h"
#include "hw_features.h"
#include "ap/sta_info.h"


#ifdef CONFIG_HOTSPOT_MGR_SUPPORT
int hostapd_drv_set_max_client(struct hostapd_data *hapd, unsigned int number)
{
	char szcmd[30] = "";
	if (!hapd->driver || !hapd->driver->driver_cmd || !hapd->drv_priv) {
		wpa_printf(MSG_ERROR, "Failed to set config to driver");
		return -1;
	}
	sprintf(szcmd, "set_max_client %d", number);
	return hapd->driver->driver_cmd(hapd->drv_priv, szcmd, NULL, 0);
}

int hostapd_driver_cmd(struct hostapd_data *hapd, char *cmd, char *buf, size_t buflen)
{
	int ret;

	if (hapd->driver == NULL || hapd->driver->driver_cmd == NULL)
		return -1;

	ret = hapd->driver->driver_cmd(hapd->drv_priv, cmd, buf, buflen);
	if (ret == 0) {
		if (os_strncmp(cmd, "STA-BLOCK ", 10) == 0) {
			u8 bssid[ETH_ALEN];
			if (hwaddr_aton(cmd + 10, bssid))
				return -1;

			ap_sta_disconnect(hapd, NULL, bssid, WLAN_REASON_PREV_AUTH_NOT_VALID);
		}

		ret = sprintf(buf, "%s\n", "OK");
	}

	return ret;
}
#endif

#ifdef CONFIG_MTK_LTE_COEX
int hostapd_drv_do_mtk_acs(struct hostapd_data *hapd, u8 *channel)
{
	if (hapd->driver == NULL || hapd->driver->driver_cmd == NULL)
		return -1;
	return hapd->driver->driver_cmd(hapd->drv_priv, "MTK-ACS", (char *)channel, sizeof(u8));
}

void lte_scan_complete(struct hostapd_iface *iface)
{
	u8 ch_selected;
	int ret;

	iface->scan_cb = NULL;
	wpa_printf(MSG_INFO, "LTE: Scan complete");

	if ((hostapd_drv_do_mtk_acs(iface->bss[0], &ch_selected) < 0) ||
		(ch_selected == 0)) {
		wpa_printf(MSG_ERROR, "MTK ACS failed to select channel, use default channel 6");
		iface->conf->channel = 6;
	} else {
		wpa_printf(MSG_INFO, "MTK ACS picked channel %d", ch_selected);
		iface->conf->channel = ch_selected;
	}

	ret = hostapd_check_ht_capab(iface);
	if (ret < 0)
		goto out;
	if (ret == 1) {
		wpa_printf(MSG_DEBUG, "Interface initialization will be completed in a callback");
		return;
	}

	ret = 0;
out:
	hostapd_setup_interface_complete(iface, ret);
}

int lte_request_scan(struct hostapd_iface *iface)
{
	struct wpa_driver_scan_params params;
	struct hostapd_channel_data *chan;
	int i, *freq;

	os_memset(&params, 0, sizeof(params));
	params.freqs = os_calloc(iface->current_mode->num_channels + 1, sizeof(params.freqs[0]));
	if (params.freqs == NULL)
		return -1;

	freq = params.freqs;
	for (i = 0; i < iface->current_mode->num_channels; i++) {
		chan = &iface->current_mode->channels[i];
		if (chan->flag & HOSTAPD_CHAN_DISABLED)
			continue;

		*freq++ = chan->freq;
	}
	*freq = 0;

	iface->scan_cb = lte_scan_complete;

	wpa_printf(MSG_INFO, "LTE: Scanning");

	if (hostapd_driver_scan(iface->bss[0], &params) < 0) {
		wpa_printf(MSG_ERROR, "LTE: Failed to request initial scan");
		os_free(params.freqs);
		iface->scan_cb = NULL;
		return -1;
	}

	os_free(params.freqs);
	return 0;
}
#endif

