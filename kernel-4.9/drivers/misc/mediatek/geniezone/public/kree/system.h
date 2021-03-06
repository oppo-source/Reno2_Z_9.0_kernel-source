/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/*
 * Header files for basic KREE functions.
 */

#ifndef __KREE_SYSTEM_H__
#define __KREE_SYSTEM_H__

#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) || defined(CONFIG_TRUSTY)

#include <tz_cross/trustzone.h>

void KREE_SESSION_LOCK(int32_t handle);
void KREE_SESSION_UNLOCK(int32_t handle);

int gz_get_cpuinfo_thread(void *data);
void set_gz_bind_cpu(int on);
int get_gz_bind_cpu(void);
int ree_dummy_thread(void *data);

struct _cpus_cluster_freq {
	unsigned int max_freq;
	unsigned int min_freq;
};

#include "mem.h"

#ifdef CONFIG_GZ_VPU_WITH_M4U
int gz_do_m4u_map(KREE_SHAREDMEM_HANDLE handle,
					phys_addr_t pa, uint32_t size,
					uint32_t region_id);
int gz_do_m4u_umap(KREE_SHAREDMEM_HANDLE handle);
#endif


/* Session Management */
/**
 *  Create a new TEE sesssion
 *
 * @param ta_uuid UUID of the TA to connect to.
 * @param pHandle Handle for the new session. Return KREE_SESSION_HANDLE_FAIL if
 * fail.
 * @return return code
 */
TZ_RESULT KREE_CreateSession(const char *ta_uuid, KREE_SESSION_HANDLE *pHandle);


/**
 *  Create a new TEE sesssion with tag for debug purpose
 *
 * @param ta_uuid UUID of the TA to connect to.
 * @param pHandle Handle for the new session.
 *	  Return KREE_SESSION_HANDLE_FAIL if fail.
 * @param tag string can be printed when querying memory usage.
 * @return return code
 */
/*fix mtee sync*/
TZ_RESULT KREE_CreateSessionWithTag(const char *ta_uuid,
				    KREE_SESSION_HANDLE *pHandle,
				    const char *tag);

/**
 * Close TEE session
 *
 * @param handle Handle for session to close.
 * @return return code
 */
TZ_RESULT KREE_CloseSession(KREE_SESSION_HANDLE handle);


/**
 * Make a TEE service call
 *
 * @param handle      Session handle to make the call
 * @param command     The command to call.
 * @param paramTypes  Types for the parameters, use TZ_ParamTypes() to
 * consturct.
 * @param param       The parameters to pass to TEE. Maximum 4 params.
 * @return            Return value from TEE service.
 */
TZ_RESULT KREE_TeeServiceCall(KREE_SESSION_HANDLE handle, uint32_t command,
			      uint32_t paramTypes, union MTEEC_PARAM param[4]);


/* System Hardware Counter */
/**
 * Get system counter value.
 *
 * @return The system counter value
 */
u64 KREE_GetSystemCnt(void);


/**
 * Get system counter frequency.
 *
 * @return The system counter frequency
 */
u32 KREE_GetSystemCntFrq(void);


#endif /* CONFIG_MTK_IN_HOUSE_TEE_SUPPORT || CONFIG_TRUSTY */
#endif /* __KREE_H__ */
