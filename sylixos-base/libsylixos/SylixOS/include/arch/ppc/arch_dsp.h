/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: arch_dsp.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 01 月 10 日
**
** 描        述: PowerPC DSP 相关.
*********************************************************************************************************/

#ifndef __PPC_ARCH_DSP_H
#define __PPC_ARCH_DSP_H

#if defined(__SYLIXOS_KERNEL) || defined(__ASSEMBLY__) || defined(ASSEMBLY)

/*********************************************************************************************************
  ALTIVEC 浮点数据寄存器的总数
*********************************************************************************************************/

#define ALTIVEC_REG_NR          32

/*********************************************************************************************************
  定义 ARCH_FPU_CTX ALTIVEC 成员偏移
*********************************************************************************************************/

#define ALTIVEC_OFFSET(n)       ((n) * 16)
#define ALTIVEC_VSCR_OFFSET     (ALTIVEC_REG_NR * 16)
#define ALTIVEC_VRSAVE_OFFSET   (ALTIVEC_VSCR_OFFSET + 16)
#define ALTIVEC_CTX_SIZE        (16 * (ALTIVEC_REG_NR + 2))

/*********************************************************************************************************
  汇编代码不包含以下内容
*********************************************************************************************************/

#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))

/*********************************************************************************************************
  定义 ALTIVEC 寄存器
*********************************************************************************************************/

typedef struct {
    UINT32              ALTREG_regs[4];                                 /*  128 位矢量寄存器            */
} __attribute__((aligned(16))) ALTIVEC_REG;

/*********************************************************************************************************
  DSP 上下文
*********************************************************************************************************/

#define ARCH_DSP_CTX_ALIGN      16                                      /*  DSP 上下文对齐大小          */

union arch_dsp_ctx {                                                    /*  DSP 上下文                  */
    struct {
        ALTIVEC_REG     VECCTX_regs[ALTIVEC_REG_NR];                    /*  32 个矢量寄存器             */
        UINT32          VECCTX_uiVscr[4];                               /*  VSCR                        */
        UINT32          VECCTX_uiVrsave;                                /*  VRSAVE                      */
        UINT32          VECCTX_uiPad0;                                  /*  16 字节大小对齐             */
        UINT32          VECCTX_uiPad1;
        UINT32          VECCTX_uiPad2;
    };
} __attribute__ ((aligned(ARCH_DSP_CTX_ALIGN)));

typedef union arch_dsp_ctx ARCH_DSP_CTX;                                /*  DSP 上下文                  */

#endif                                                                  /*  __ASSEMBLY__                */
#endif                                                                  /*  __SYLIXOS_KERNEL            */
#endif                                                                  /*  __PPC_ARCH_DSP_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
