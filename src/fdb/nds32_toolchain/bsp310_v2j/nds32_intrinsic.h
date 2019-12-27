/* Definitions of target machine for NDS32.
   Copyright (C) 2006 Free Software Foundation, Inc.
   Contributed by Andes Technology Corporation.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#ifndef _NDS32_INTRINSIC_H
#define _NDS32_INTRINSIC_H

/* system registers  */

enum nds32_sr {
NDS32_SR_CPU_VER,
NDS32_SR_ICM_CFG,
NDS32_SR_DCM_CFG,
NDS32_SR_MMU_CFG,
NDS32_SR_MSC_CFG,
NDS32_SR_CORE_ID,
NDS32_SR_FUCOP_EXIST,
NDS32_SR_PSW,
NDS32_SR_IPSW,
NDS32_SR_P_IPSW,
NDS32_SR_IVB,
NDS32_SR_EVA,
NDS32_SR_P_EVA,
NDS32_SR_ITYPE,
NDS32_SR_P_ITYPE,
NDS32_SR_MERR,
NDS32_SR_IPC,
NDS32_SR_P_IPC,
NDS32_SR_OIPC,
NDS32_SR_P_P0,
NDS32_SR_P_P1,
NDS32_SR_INT_MASK,
NDS32_SR_INT_PEND,
NDS32_SR_SP_USR,
NDS32_SR_SP_PRIV,
NDS32_SR_INT_PRI,
NDS32_SR_INT_CTRL,
NDS32_SR_MMU_CTL,
NDS32_SR_L1_PPTB,
NDS32_SR_TLB_VPN,
NDS32_SR_TLB_DATA,
NDS32_SR_TLB_MISC,
NDS32_SR_VLPT_IDX,
NDS32_SR_ILMB,
NDS32_SR_DLMB,
NDS32_SR_CACHE_CTL,
NDS32_SR_HSMP_SADDR,
NDS32_SR_HSMP_EADDR,
NDS32_SR_SDZ_CTL,
NDS32_SR_N12MISC_CTL,
NDS32_SR_MISC_CTL,
NDS32_SR_BPC0,
NDS32_SR_BPC1,
NDS32_SR_BPC2,
NDS32_SR_BPC3,
NDS32_SR_BPC4,
NDS32_SR_BPC5,
NDS32_SR_BPC6,
NDS32_SR_BPC7,
NDS32_SR_BPA0,
NDS32_SR_BPA1,
NDS32_SR_BPA2,
NDS32_SR_BPA3,
NDS32_SR_BPA4,
NDS32_SR_BPA5,
NDS32_SR_BPA6,
NDS32_SR_BPA7,
NDS32_SR_BPAM0,
NDS32_SR_BPAM1,
NDS32_SR_BPAM2,
NDS32_SR_BPAM3,
NDS32_SR_BPAM4,
NDS32_SR_BPAM5,
NDS32_SR_BPAM6,
NDS32_SR_BPAM7,
NDS32_SR_BPV0,
NDS32_SR_BPV1,
NDS32_SR_BPV2,
NDS32_SR_BPV3,
NDS32_SR_BPV4,
NDS32_SR_BPV5,
NDS32_SR_BPV6,
NDS32_SR_BPV7,
NDS32_SR_BPCID0,
NDS32_SR_BPCID1,
NDS32_SR_BPCID2,
NDS32_SR_BPCID3,
NDS32_SR_BPCID4,
NDS32_SR_BPCID5,
NDS32_SR_BPCID6,
NDS32_SR_BPCID7,
NDS32_SR_EDM_CFG,
NDS32_SR_EDMSW,
NDS32_SR_EDM_CTL,
NDS32_SR_EDM_DTR,
NDS32_SR_BPMTC,
NDS32_SR_DIMBR,
NDS32_SR_TECR0,
NDS32_SR_TECR1,
NDS32_SR_EDM_PROBE,
NDS32_SR_PFMC0,
NDS32_SR_PFMC1,
NDS32_SR_PFMC2,
NDS32_SR_PFM_CTL,
NDS32_SR_FUCOP_CTL,
NDS32_SR_PRUSR_ACC_CTL,
NDS32_SR_DMA_CFG,
NDS32_SR_DMA_GCSW,
NDS32_SR_DMA_CHNSEL,
NDS32_SR_DMA_ACT,
NDS32_SR_DMA_SETUP,
NDS32_SR_DMA_ISADDR,
NDS32_SR_DMA_ESADDR,
NDS32_SR_DMA_TCNT,
NDS32_SR_DMA_STATUS,
NDS32_SR_DMA_2DSET,
NDS32_SR_DMA_2DSCTL
};

/* user registers  */

enum nds32_usr {
NDS32_USR_PC,
NDS32_USR_DMA_CFG,
NDS32_USR_DMA_GCSW,
NDS32_USR_DMA_CHNSEL,
NDS32_USR_DMA_ACT,
NDS32_USR_DMA_SETUP,
NDS32_USR_DMA_ISADDR,
NDS32_USR_DMA_ESADDR,
NDS32_USR_DMA_TCNT,
NDS32_USR_DMA_STATUS,
NDS32_USR_DMA_2DSET,
NDS32_USR_DMA_2DSCTL,
NDS32_USR_PFMC0,
NDS32_USR_PFMC1,
NDS32_USR_PFMC2,
NDS32_USR_PFM_CTL,
};

/* cctl subtype  */

enum nds32_cctl_valck {
NDS32_CCTL_L1D_VA_FILLCK,
NDS32_CCTL_L1D_VA_ULCK,
NDS32_CCTL_L1I_VA_FILLCK,
NDS32_CCTL_L1I_VA_ULCK,
};

enum nds32_cctl_idxwbinv {
NDS32_CCTL_L1D_IX_WBINVAL,
NDS32_CCTL_L1D_IX_INVAL,
NDS32_CCTL_L1D_IX_WB,
NDS32_CCTL_L1I_IX_INVAL,
};

enum nds32_cctl_vawbinv {
NDS32_CCTL_L1D_VA_INVAL,
NDS32_CCTL_L1D_VA_WB,
NDS32_CCTL_L1D_VA_WBINVAL,
NDS32_CCTL_L1I_VA_INVAL,
};

enum nds32_cctl_idxread {
NDS32_CCTL_L1D_IX_RTAG,
NDS32_CCTL_L1D_IX_RWD,
NDS32_CCTL_L1I_IX_RTAG,
NDS32_CCTL_L1I_IX_RWD,
};

enum nds32_cctl_idxwrite {
NDS32_CCTL_L1D_IX_WTAG,
NDS32_CCTL_L1D_IX_WWD,
NDS32_CCTL_L1I_IX_WTAG,
NDS32_CCTL_L1I_IX_WWD,
};

/* subtypes for dpref  */

enum nds32_dpref {
NDS32_DPREF_SRD,
NDS32_DPREF_MRD,
NDS32_DPREF_SWR,
NDS32_DPREF_MWR,
NDS32_DPREF_PTE,
NDS32_DPREF_CLWR,
};

#ifdef NDS32_BASELINE_V3M

struct __attribute__((packed)) u4 { int data; };
struct __attribute__((packed)) u8 { long long data; };

static __inline unsigned short
__nds32__get_unaligned_hw (void *a)
{
  unsigned char *ptr = (unsigned char *) a;
#ifdef __NDS32_EL__
  return ptr[0] | ptr[1] << 8;
#else
  return ptr[0] << 8 | ptr[1];
#endif
}

static __inline void
__nds32__put_unaligned_hw (void *a, unsigned short data)
{
  unsigned char *ptr = (unsigned char *) a;
#ifdef __NDS32_EL__
  *ptr++ = data;
  *ptr++ = data >> 8;
#else
  *ptr++ = data >> 8;
  *ptr++ = data;
#endif
}

static __inline int
__nds32__get_unaligned_w (void *ptr)
{
  return ((struct u4 *) ptr)->data;
}

static __inline void
__nds32__put_unaligned_w (void *ptr, int data)
{
  ((struct u4 *) ptr)->data = data;
}

static __inline long long
__nds32__get_unaligned_dw (void *ptr)
{
  return ((struct u8 *) ptr)->data;
}

static __inline void
__nds32__put_unaligned_dw (void *ptr, long long data)
{
  ((struct u8 *) ptr)->data = data;
}

#else /* Not V3M */

#define __nds32__get_unaligned_dw(a) (__builtin_nds32_unalign_load_dw ((a)))
#define __nds32__get_unaligned_w(a) (__builtin_nds32_unalign_load_w ((a)))
#define __nds32__put_unaligned_dw(a, data) (__builtin_nds32_unalign_store_dw ((a), (data)))
#define __nds32__put_unaligned_w(a, data) (__builtin_nds32_unalign_store_w ((a), (data)))

__extension__ static __inline unsigned short __attribute__ ((__always_inline__))
__nds32__get_unaligned_hw (void *a)
{
  unsigned char *ptr = (unsigned char *) a;
#ifdef __NDS32_EL__
  return ptr[0] | ptr[1] << 8;
#else
  return ptr[0] << 8 | ptr[1];
#endif
}

__extension__ static __inline void __attribute__ ((__always_inline__))
__nds32__put_unaligned_hw (unsigned short *a, unsigned short data)
{
  unsigned char *ptr = (unsigned char *) a;
#ifdef __NDS32_EL__
  *ptr++ = data;
  *ptr++ = data >> 8;
#else
  *ptr++ = data >> 8;
  *ptr++ = data;
#endif
}

#endif /* NDS32_BASELINE_V3M */

#define __nds32__llw(a) (__builtin_nds32_llw ((a)))
#define __nds32__lwup(a) (__builtin_nds32_lwup ((a)))
#define __nds32__scw(a, b) (__builtin_nds32_scw ((a), (b)))
#define __nds32__swup(a, b) (__builtin_nds32_swup ((a), (b)))
#define __nds32__mfsr(srname) (__builtin_nds32_mfsr ((srname)))
#define __nds32__mfusr(usrname) (__builtin_nds32_mfusr ((usrname)))
#define __nds32__mtsr(val, srname) (__builtin_nds32_mtsr ((val), (srname)))
#define __nds32__mtusr(val, usrname) (__builtin_nds32_mtusr ((val), (usrname)))
#define __nds32__break(swid) (__builtin_nds32_break(swid))
#define __nds32__cctlva_lck(subtype, va) (__builtin_nds32_cctl_va_lck ((subtype), (va)))
#define __nds32__cctlidx_wbinval(subtype, idx) (__builtin_nds32_cctl_idx_wbinval ((subtype), (idx)))
#define __nds32__cctlva_wbinval_alvl(subtype, va) (__builtin_nds32_cctl_va_wbinval_la ((subtype), (va)))
#define __nds32__cctlva_wbinval_one_lvl(subtype, va) (__builtin_nds32_cctl_va_wbinval_l1 ((subtype), (va)))
#define __nds32__cctlidx_read(subtype, idx) (__builtin_nds32_cctl_idx_read ((subtype), (idx)))
#define __nds32__cctlidx_write(subtype, b, idxw) ( __builtin_nds32_cctl_idx_write ((subtype), (b), (idxw)))
#define __nds32__cctl_l1d_invalall  __builtin_nds32_cctl_l1d_invalall
#define __nds32__dsb __builtin_nds32_dsb
#define __nds32__isb __builtin_nds32_isb
#define __nds32__msync_store __builtin_nds32_msync_store
#define __nds32__msync_all __builtin_nds32_msync_all
#define __nds32__nop __builtin_nds32_nop
#define __nds32__standby_wait_done __builtin_nds32_standby_wait_done
#define __nds32__standby_no_wake_grant __builtin_nds32_standby_no_wake_grant
#define __nds32__standby_wake_grant __builtin_nds32_standby_wake_grant
#define __nds32__schedule_barrier __builtin_nds32_schedule_barrier
#define __nds32__setend_big __builtin_nds32_setend_big
#define __nds32__setend_little __builtin_nds32_setend_little
#define __nds32__setgie_en __builtin_nds32_setgie_en
#define __nds32__setgie_dis __builtin_nds32_setgie_dis
#define __nds32__dpref_qw(a, b, subtype) (__builtin_nds32_dpref_qw ((a), (b), (subtype)))
#define __nds32__dpref_hw(a, b, subtype) (__builtin_nds32_dpref_hw ((a), (b), (subtype)))
#define __nds32__dpref_w(a, b, subtype) (__builtin_nds32_dpref_w ((a), (b), (subtype)))
#define __nds32__dpref_dw(a, b, subtype) (__builtin_nds32_dpref_dw ((a), (b), (subtype)))
#define __nds32__teqz(a, swid) (__builtin_nds32_teqz ((a), (swid)))
#define __nds32__tnez(a, swid) ( __builtin_nds32_tnez ((a), (swid)))
#define __nds32__trap(swid) (__builtin_nds32_trap ((swid)))
#define __nds32__isync(a) (__builtin_nds32_isync ((a)))
#define __nds32__rotr(val, ror) (__builtin_nds32_rotr ((val), (ror)))
#define __nds32__wsbh(a) (__builtin_nds32_wsbh ((a)))
#define __nds32__syscall(a) (__builtin_nds32_syscall ((a)))
#define __nds32__return_address __builtin_nds32_return_address
#define __nds32__get_current_sp __builtin_nds32_get_current_sp
#define __nds32__set_current_sp(a) (__builtin_nds32_set_current_sp ((a)))
#define __nds32__ave(a, b) (__builtin_nds32_pe_ave ((a), (b)))
#define __nds32__bclr(a, pos) (__builtin_nds32_pe_bclr ((a), (pos)))
#define __nds32__bset(a, pos) (__builtin_nds32_pe_bset ((a), (pos)))
#define __nds32__btgl(a, pos) (__builtin_nds32_pe_btgl ((a), (pos)))
#define __nds32__btst(a, pos) (__builtin_nds32_pe_btst ((a), (pos)))
#define __nds32__clip(a, imm) (__builtin_nds32_pe_clip ((a), (imm)))
#define __nds32__clips(a, imm) (__builtin_nds32_pe_clips ((a), (imm)))
#define __nds32__clz(a) (__builtin_nds32_pe_clz ((a)))
#define __nds32__clo(a) (__builtin_nds32_pe_clo ((a)))
#define __nds32__bse(r, a, b) (__builtin_nds32_pe2_bse ((r), (a), (b)))
#define __nds32__bsp(r, a, b) (__builtin_nds32_pe2_bsp ((r), (a), (b)))
#define __nds32__pbsad(a, b) (__builtin_nds32_pe2_pbsad ((a), (b)))
#define __nds32__pbsada(acc, a, b) (__builtin_nds32_pe2_pbsada ((acc), (a), (b)))
#define __nds32__ffb(a, b) (__builtin_nds32_se_ffb ((a), (b)))
#define __nds32__ffmism(a, b) (__builtin_nds32_se_ffmism ((a), (b)))
#define __nds32__flmism(a, b) (__builtin_nds32_se_flmism ((a), (b)))
#define __nds32__fcpynsd(a, b) (__builtin_nds32_fcpynsd ((a), (b)))
#define __nds32__fcpynss(a, b) (__builtin_nds32_fcpynss ((a), (b)))
#define __nds32__fcpysd(a, b) (__builtin_nds32_fcpysd ((a), (b)))
#define __nds32__fcpyss(a, b) (__builtin_nds32_fcpyss ((a), (b)))
#define __nds32__fmfcsr __builtin_nds32_fmfcsr
#define __nds32__fmtcsr(fpcsr) (__builtin_nds32_fmtcsr ((fpcsr)))
#define __nds32__fmfcfg __builtin_nds32_fmfcfg

__extension__ static __inline void __attribute__ ((__always_inline__))
__nds32__gie_dis ()
{
  __builtin_nds32_setgie_dis ();
  __builtin_nds32_dsb ();
}

__extension__ static __inline void __attribute__ ((__always_inline__))
 __nds32__gie_en ()
{
  __builtin_nds32_setgie_en ();
  __builtin_nds32_dsb ();

}

#endif /* nds32_intrinsic.h */
