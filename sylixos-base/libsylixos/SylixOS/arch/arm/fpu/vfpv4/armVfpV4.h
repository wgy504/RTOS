/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: armVfpV4.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2013 年 12 月 09 日
**
** 描        述: ARM 体系架构 VFPv4 支持.
*********************************************************************************************************/

#ifndef __ARMVFPV4_H
#define __ARMVFPV4_H

PARM_FPU_OP  armVfpV4PrimaryInit(CPCHAR  pcMachineName, CPCHAR  pcFpuName);
VOID         armVfpV4SecondaryInit(CPCHAR  pcMachineName, CPCHAR  pcFpuName);

#endif                                                                  /*  __ARMVFPV4_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
