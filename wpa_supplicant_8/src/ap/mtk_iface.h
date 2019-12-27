/*
 * Control interface for shared AP commands
 * Copyright (c) 2004-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef MTK_IFACE_H
#define MTK_IFACE_H

#ifdef CONFIG_HOTSPOT_MGR_SUPPORT
int hostapd_drv_set_max_client(struct hostapd_data *hapd, unsigned int number);
int hostapd_driver_cmd(struct hostapd_data *hapd, char *cmd, char *buf, size_t buflen);
#endif

#ifdef CONFIG_MTK_LTE_COEX
int lte_request_scan(struct hostapd_iface *iface);
#endif

#endif /* MTK_IFACE_H */
