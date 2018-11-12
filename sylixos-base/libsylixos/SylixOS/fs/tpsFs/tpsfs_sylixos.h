/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: tpsfs_sylixos.h
**
** ��   ��   ��: Jiang.Taijin (��̫��)
**
** �ļ���������: 2015 �� 9 �� 21 ��
**
** ��        ��: tpsfs SylixOS FS �ӿ�.

** BUG:
*********************************************************************************************************/

#ifndef __TPSFS_SYLIXOS_H
#define __TPSFS_SYLIXOS_H

/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if LW_CFG_TPSFS_EN > 0

/*********************************************************************************************************
  API
*********************************************************************************************************/
LW_API INT      API_TpsFsDrvInstall(VOID);
LW_API INT      API_TpsFsDevCreate(PCHAR   pcName, PLW_BLK_DEV  pblkd);
LW_API INT      API_TpsFsDevDelete(PCHAR   pcName);

#define tpsFsDrv                API_TpsFsDrvInstall
#define tpsFsDevCreate          API_TpsFsDevCreate
#define tpsFsDevDelete          API_TpsFsDevDelete

#endif                                                                  /* LW_CFG_TPSFS_EN > 0          */
#endif                                                                  /* __TPSFS_SYLIXOS_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/