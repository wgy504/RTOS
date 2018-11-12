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
** ��   ��   ��: s3c2440a_pm.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 07 �� 20 ��
**
** ��        ��: ��Դ��������, Ŀǰ�������� CPU ģʽ���� (ʵ������)
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "config.h"                                                     /*  �������� & ���������       */
#include "SylixOS.h"
/*********************************************************************************************************
  S3C2440A ��Դ������
*********************************************************************************************************/
typedef struct {
    LW_PMA_FUNCS    pmafuncs;
    PVOID           reserved;
} S3C2440A_PMA;
/*********************************************************************************************************
** Function name:           pmPowerOn
** Descriptions:            ָ���豸�ϵ�
** input parameters:        pmadapter   ��Դ����������
** output parameters:       pmdev       �豸
** Returned value:          OK
** Created by:              Hanhui
** Created Date:            2014-07-20
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static INT  pmPowerOn (PLW_PM_ADAPTER  pmadapter,
                       PLW_PM_DEV      pmdev)
{
    rCLKCON |= (1 << pmdev->PMD_uiChannel);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** Function name:           pmPowerOff
** Descriptions:            ָ���豸�µ�
** input parameters:        pmadapter   ��Դ����������
** output parameters:       pmdev       �豸
** Returned value:          OK
** Created by:              Hanhui
** Created Date:            2014-07-20
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static INT  pmPowerOff (PLW_PM_ADAPTER  pmadapter,
                        PLW_PM_DEV      pmdev)
{
    rCLKCON &= ~(1 << pmdev->PMD_uiChannel);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** Function name:           pmPowerIsOn
** Descriptions:            ָ���豸�Ƿ��Ѿ��ϵ�
** input parameters:        pmadapter   ��Դ����������
** output parameters:       pmdev       �豸
**                          pbIsOn      �Ƿ��ϵ�
** Returned value:          OK
** Created by:              Hanhui
** Created Date:            2014-07-20
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static INT  pmPowerIsOn (PLW_PM_ADAPTER  pmadapter,
                         PLW_PM_DEV      pmdev,
                         BOOL           *pbIsOn)
{
    if (pbIsOn) {
        if (rCLKCON & (1 << pmdev->PMD_uiChannel)) {
            *pbIsOn = LW_TRUE;
        } else {
            *pbIsOn = LW_FALSE;
        }
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** Function name:           pmGetFuncs
** Descriptions:            ��ȡ��Դ����������������
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          pfuncs      ��������
** Created by:              Hanhui
** Created Date:            2014-07-20
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
PLW_PMA_FUNCS  pmGetFuncs (VOID)
{
    static S3C2440A_PMA     pma;

    pma.pmafuncs.PMAF_pfuncOn   = pmPowerOn;
    pma.pmafuncs.PMAF_pfuncOff  = pmPowerOff;
    pma.pmafuncs.PMAF_pfuncIsOn = pmPowerIsOn;

    return  (&pma.pmafuncs);
}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/