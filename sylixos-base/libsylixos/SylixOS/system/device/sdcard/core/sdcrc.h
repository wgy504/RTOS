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
** 文   件   名: sdcrc.h
**
** 创   建   人: Zeng.Bo (曾波)
**
** 文件创建日期: 2011 年 02 月 21 日
**
** 描        述: sd 卡在 spi 模式下的 crc 校验工具库头文件.

** BUG:
*********************************************************************************************************/

#ifndef __SD_CRC_H
#define __SD_CRC_H

/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if LW_CFG_SDCARD_CRC_EN > 0

/*********************************************************************************************************
  CRC functions
*********************************************************************************************************/

UINT16 __sdCrc16(UINT8 *pucData, UINT16 usLen);
UINT8  __sdCrcCmdCrc7(UINT8 ucCmd, UINT8 *pucParam);

#endif                                                                  /*  LW_CFG_SDCARD_CRC_EN > 0    */
#endif                                                                  /*  __SD_CRC_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
