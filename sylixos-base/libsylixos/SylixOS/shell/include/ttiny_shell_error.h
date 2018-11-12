/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: ttiny_shell_error.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 07 �� 27 ��
**
** ��        ��: ���� ttniy_shell ������붨��.
*********************************************************************************************************/

#ifndef __TTINY_SHELL_ERROR_H
#define __TTINY_SHELL_ERROR_H

/*********************************************************************************************************
  ttiny_shell ������붨�� 90000 - 90500
*********************************************************************************************************/
#define ERROR_TSHELL_EPARAM             90000                           /*  ��������                    */
#define ERROR_TSHELL_OVERLAP            90001                           /*  �ؼ��ֻ�����ظ�            */
#define ERROR_TSHELL_EKEYWORD           90002                           /*  �ؼ��ʴ���                  */
#define ERROR_TSHELL_EVAR               90003                           /*  ����������                  */
#define ERROR_TSHELL_CANNT_OVERWRITE    90004                           /*  �������ܱ���д              */
#define ERROR_TSHELL_ENOUSER            90005                           /*  �û�������                  */
#define ERROR_TSHELL_EUSER              90006                           /*  �û�����                    */
#define ERROR_TSHELL_ELEVEL             90007                           /*  Ȩ�޲���                    */
#define ERROR_TSHELL_CMDNOTFUND         90008                           /*  �޷��ҵ�����                */

#endif                                                                  /*  __TTINY_SHELL_ERROR_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/