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
** ��   ��   ��: ttinyShellReadline.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2012 �� 03 �� 25 ��
**
** ��        ��: shell ���ն˶�ȡһ������.
                 ֧��Ŀ¼ƥ�䲹��, ����λ�ò���, ɾ��, ��ʷ������¼.
                 
** BUG:
2012.10.19  ���� __tshellCharTab() �����б������������, Ӧ��Ϊ LW_CFG_SHELL_MAX_PARAMNUM + 1.
2014.02.24  tab ����ʱ, �������ƶȲ���.
2014.10.29  tab ����ʱ, ��ʾ�ĵ��п���ͨ�� TIOCGWINSZ ��ȡ.
2016.05.27  tab ֧��ƥ�� HOME Ŀ¼.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/shell/include/ttiny_shell.h"
#include "sys/ioctl.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0
#include "ttinyShellLib.h"
#include "ttinyShellSysCmd.h"
#include "ttinyString.h"
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
extern VOID  __tshellShowPrompt(VOID);
/*********************************************************************************************************
  ���⹦�ܼ�
*********************************************************************************************************/
#define __KEY_IS_ESC(c)             (c == 27)
#define __KEY_IS_EOT(c)             (c ==  4)
#define __KEY_IS_TAB(c)             (c ==  9)
#define __KEY_IS_LF(c)              (c == 10)
#define __KEY_IS_BS(c)              (c == 127 || c == 8)

#define __FUNC_KEY_LEFT             68
#define __FUNC_KEY_UP               65
#define __FUNC_KEY_RIGHT            67
#define __FUNC_KEY_DOWN             66
#define __FUNC_KEY_HOME             49
#define __FUNC_KEY_DELETE           51
#define __FUNC_KEY_END              52
/*********************************************************************************************************
  ���⹦�ܼ��ش�
*********************************************************************************************************/
#define __KEY_WRITE_LEFT(fd)        write(fd, "\x1b\x5b\x44", 3)
#define __KEY_WRITE_UP(fd)          write(fd, "\x1b\x5b\x41", 3)
#define __KEY_WRITE_RIGHT(fd)       write(fd, "\x1b\x5b\x43", 3)
#define __KEY_WRITE_DOWN(fd)        write(fd, "\x1b\x5b\x42", 3)
/*********************************************************************************************************
  ��
*********************************************************************************************************/
#define CTX_TAB                     psicContext->SIC_uiTabCnt
#define CTX_CURSOR                  psicContext->SIC_uiCursor
#define CTX_TOTAL                   psicContext->SIC_uiTotalLen
#define CTX_BUFFER                  psicContext->SIC_cInputBuffer
/*********************************************************************************************************
  ��ǰ����������
*********************************************************************************************************/
typedef struct {
    UINT        SIC_uiCursor;
    UINT        SIC_uiTotalLen;
    CHAR        SIC_cInputBuffer[LW_CFG_SHELL_MAX_COMMANDLEN + 1];
} __SHELL_INPUT_CTX;
typedef __SHELL_INPUT_CTX       *__PSHELL_INPUT_CTX;
/*********************************************************************************************************
  ������ʷ��Ϣ
*********************************************************************************************************/
#define __INPUT_SAVE_MAX            20
typedef struct {
    LW_LIST_RING_HEADER          SIHC_pringHeader;
    UINT                         SIHC_uiCounter;
} __SHELL_HISTORY_CTX;
typedef __SHELL_HISTORY_CTX     *__PSHELL_HISTORY_CTX;

typedef struct {
    LW_LIST_RING                 SIH_ringManage;
    CHAR                         SIH_cInputSave[1];
} __SHELL_HISTORY;
typedef __SHELL_HISTORY         *__PSHELL_HISTORY;
/*********************************************************************************************************
** ��������: __tshellReadlineClean
** ��������: shell �˳�ʱ��� readline ��Ϣ.
** �䡡��  : ulId                          �߳� ID
**           pvRetVal                      ����ֵ
**           ptcbDel                       ɾ���� TCB
** �䡡��  : ��ȡ����
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __tshellReadlineClean (LW_OBJECT_HANDLE  ulId, PVOID  pvRetVal, PLW_CLASS_TCB  ptcbDel)
{
    __PSHELL_HISTORY_CTX    psihc = __TTINY_SHELL_GET_HIS(ptcbDel);
    __PSHELL_HISTORY        psihHistory;
    
    if (psihc) {
        while (psihc->SIHC_pringHeader) {
            psihHistory = _LIST_ENTRY(psihc->SIHC_pringHeader, __SHELL_HISTORY, SIH_ringManage);
            _List_Ring_Del(&psihHistory->SIH_ringManage, 
                           &psihc->SIHC_pringHeader);
            __SHEAP_FREE(psihHistory);
        }
        
        __SHEAP_FREE(psihc);
        __TTINY_SHELL_SET_HIS(ptcbDel, LW_NULL);
    }
}
/*********************************************************************************************************
** ��������: __tshellReadlineInit
** ��������: shell ��ʼ�� readline.
** �䡡��  : NONE
** �䡡��  : ��ʼ���Ƿ�ɹ�
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellReadlineInit (VOID)
{
    PLW_CLASS_TCB           ptcbCur;
    __PSHELL_HISTORY_CTX    psihc;
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    psihc = __TTINY_SHELL_GET_HIS(ptcbCur);
    
    if (psihc == LW_NULL) {
        psihc = (__PSHELL_HISTORY_CTX)__SHEAP_ALLOC(sizeof(__SHELL_HISTORY_CTX));
        if (psihc == LW_NULL) {
            fprintf(stderr, "read line tool no memory!\n");
            return  (PX_ERROR);
        }
        lib_bzero(psihc, sizeof(__SHELL_HISTORY_CTX));
        __TTINY_SHELL_SET_HIS(ptcbCur, psihc);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellTtyInputHistorySave
** ��������: shell ��¼һ������.
** �䡡��  : psicContext           ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellHistorySave (__PSHELL_INPUT_CTX  psicContext)
{
    PLW_CLASS_TCB           ptcbCur;
    __PSHELL_HISTORY_CTX    psihc;
    __PSHELL_HISTORY        psihHistory;
    PLW_LIST_RING           pring;
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    psihc = __TTINY_SHELL_GET_HIS(ptcbCur);
    
    pring = psihc->SIHC_pringHeader;
    while (pring) {                                                     /*  ѭ�������Ƿ�����ǰ����ͬ    */
        psihHistory = _LIST_ENTRY(pring, __SHELL_HISTORY, SIH_ringManage);
        
        if (lib_strcmp(psihHistory->SIH_cInputSave, CTX_BUFFER) == 0) { /*  �����ͬ                    */
            
            if (pring == psihc->SIHC_pringHeader) {
                return;                                                 /*  ����Ǳ�ͷ, ����Ҫ����    */
            
            } else {
                _List_Ring_Del(&psihHistory->SIH_ringManage,
                               &psihc->SIHC_pringHeader);
                _List_Ring_Add_Ahead(&psihHistory->SIH_ringManage,
                                     &psihc->SIHC_pringHeader);         /*  ���ڱ�ͷλ��                */
                return;
            }
        }
        pring = _list_ring_get_next(pring);
        if (pring == psihc->SIHC_pringHeader) {
            break;
        }
    }
    
    psihHistory = (__PSHELL_HISTORY)__SHEAP_ALLOC(sizeof(__SHELL_HISTORY) + 
                                                  lib_strlen(CTX_BUFFER));
    if (psihHistory) {
        lib_strcpy(psihHistory->SIH_cInputSave, CTX_BUFFER);
        _List_Ring_Add_Ahead(&psihHistory->SIH_ringManage,
                             &psihc->SIHC_pringHeader);                 /*  �����µ�����                */
        psihc->SIHC_uiCounter++;
        
        if (psihc->SIHC_uiCounter > __INPUT_SAVE_MAX) {                 /*  ��Ҫɾ�����ϵ�һ��          */
            PLW_LIST_RING   pringPrev = _list_ring_get_prev(psihc->SIHC_pringHeader);
            psihHistory = _LIST_ENTRY(pringPrev, __SHELL_HISTORY, SIH_ringManage);
            _List_Ring_Del(&psihHistory->SIH_ringManage,
                           &psihc->SIHC_pringHeader);
            __SHEAP_FREE(psihHistory);
            psihc->SIHC_uiCounter--;
        }
    }
}
/*********************************************************************************************************
** ��������: __tshellTtyInputHistoryGet
** ��������: shell ��ȡһ��������ʷ.
** �䡡��  : bNext                 ��ǰ�������
**           ppvCookie             �ϴλ�ȡ��λ��
**           psicContext           ��ǰ������������
** �䡡��  : �Ƿ��ȡ�ɹ�
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static BOOL  __tshellHistoryGet (BOOL  bNext, PVOID  *ppvCookie, __PSHELL_INPUT_CTX  psicContext)
{
    PLW_CLASS_TCB           ptcbCur;
    __PSHELL_HISTORY_CTX    psihc;
    __PSHELL_HISTORY        psihHistory = (__PSHELL_HISTORY)*ppvCookie;
    PLW_LIST_RING           pringGet;
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    psihc = __TTINY_SHELL_GET_HIS(ptcbCur);

    if (psihc->SIHC_pringHeader == LW_NULL) {                           /*  û����ʷ��¼                */
        return  (LW_FALSE);
    }

    if (psihHistory == LW_NULL) {
        pringGet = psihc->SIHC_pringHeader;
        
    } else {
        if (bNext) {
            pringGet = _list_ring_get_next(&psihHistory->SIH_ringManage);
        } else {
            pringGet = _list_ring_get_prev(&psihHistory->SIH_ringManage);
        }
    }

    psihHistory = _LIST_ENTRY(pringGet, __SHELL_HISTORY, SIH_ringManage);
    lib_strlcpy(CTX_BUFFER, psihHistory->SIH_cInputSave, LW_CFG_SHELL_MAX_COMMANDLEN);
    *ppvCookie = (PVOID)psihHistory;
    
    return  (LW_TRUE);
}
/*********************************************************************************************************
** ��������: __tshellTtyCursorMoveLeft
** ��������: shell �������.
** �䡡��  : iFd                           �ļ�������
**           iNum                          ����λ��
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellTtyCursorMoveLeft (INT  iFd, INT  iNum)
{
    fdprintf(iFd, "\x1b[%dD", iNum);
}
/*********************************************************************************************************
** ��������: __tshellTtyCursorMoveRight
** ��������: shell �������.
** �䡡��  : iFd                           �ļ�������
**           iNum                          ����λ��
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellTtyCursorMoveRight (INT  iFd, INT  iNum)
{
    fdprintf(iFd, "\x1b[%dC", iNum);
}
/*********************************************************************************************************
** ��������: __tshellTtyClearEndLine
** ��������: shell �ӹ�괦ɾ������.
** �䡡��  : iFd                           �ļ�������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellTtyClearEndLine (INT  iFd)
{
    fdprintf(iFd, "\x1b[K");
}
/*********************************************************************************************************
** ��������: __tshellCharLeft
** ��������: shell �յ������.
** �䡡��  : iFd                           �ļ�������
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharLeft (INT  iFd, __PSHELL_INPUT_CTX  psicContext)
{
    if (CTX_CURSOR > 0) {
        CTX_CURSOR--;
        __KEY_WRITE_LEFT(iFd);
    }
}
/*********************************************************************************************************
** ��������: __tshellCharRight
** ��������: shell �յ��ҷ����.
** �䡡��  : iFd                           �ļ�������
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharRight (INT  iFd, __PSHELL_INPUT_CTX  psicContext)
{
    if (CTX_CURSOR < CTX_TOTAL) {
        CTX_CURSOR++;
        __KEY_WRITE_RIGHT(iFd);
    }
}
/*********************************************************************************************************
** ��������: __tshellCharUp
** ��������: shell �յ��Ϸ����.
** �䡡��  : iFd                           �ļ�������
**           ppvCookie                     ��һ�ε�λ��
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharUp (INT  iFd, PVOID *ppvCookie, __PSHELL_INPUT_CTX  psicContext)
{
    BOOL    bGetOk = __tshellHistoryGet(LW_TRUE, ppvCookie, psicContext);
                                                                        /*  ������ڱ�ͷ, ����İ�˳��  */
    if (bGetOk) {
        if (CTX_CURSOR > 0) {
            __tshellTtyCursorMoveLeft(iFd, CTX_CURSOR);
        }
        __tshellTtyClearEndLine(iFd);
        CTX_TOTAL  = (UINT)lib_strlen(CTX_BUFFER);
        write(iFd, CTX_BUFFER, CTX_TOTAL);
        CTX_CURSOR = CTX_TOTAL;
    }
}
/*********************************************************************************************************
** ��������: __tshellCharDown
** ��������: shell �յ��·����.
** �䡡��  : iFd                           �ļ�������
**           ppvCookie                     ��һ�ε�λ��
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharDown (INT  iFd, PVOID *ppvCookie, __PSHELL_INPUT_CTX  psicContext)
{
    BOOL    bGetOk = __tshellHistoryGet(LW_FALSE, ppvCookie, psicContext);
                                                                        /*  ����                        */
    if (bGetOk) {
        if (CTX_CURSOR > 0) {
            __tshellTtyCursorMoveLeft(iFd, CTX_CURSOR);
        }
        __tshellTtyClearEndLine(iFd);
        CTX_TOTAL  = (UINT)lib_strlen(CTX_BUFFER);
        write(iFd, CTX_BUFFER, CTX_TOTAL);
        CTX_CURSOR = CTX_TOTAL;
    }
}
/*********************************************************************************************************
** ��������: __tshellCharBackspace
** ��������: shell �յ��˸��.
** �䡡��  : iFd                           �ļ�������
**           cChar                         ��ֵ
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharBackspace (INT  iFd, CHAR  cChar, __PSHELL_INPUT_CTX  psicContext)
{
#define __KEY_BS    0x08

    CHAR    cBsCharList[3];
    
    if (CTX_CURSOR == 0) {
        return;
    }
    
    if (CTX_CURSOR == CTX_TOTAL) {
        cBsCharList[0] = __KEY_BS;
        cBsCharList[1] = ' ';
        cBsCharList[2] = __KEY_BS;
        write(iFd, cBsCharList, 3);
        CTX_CURSOR--;
        CTX_TOTAL--;
    
    } else if (CTX_CURSOR < CTX_TOTAL) {
        cChar = __KEY_BS;
        write(iFd, &cChar, 1);
        lib_memcpy(&CTX_BUFFER[CTX_CURSOR - 1], 
                   &CTX_BUFFER[CTX_CURSOR], 
                   CTX_TOTAL - CTX_CURSOR);
        CTX_BUFFER[CTX_TOTAL - 1] = ' ';
        CTX_CURSOR--;
        write(iFd, &CTX_BUFFER[CTX_CURSOR], CTX_TOTAL - CTX_CURSOR);
        __tshellTtyCursorMoveLeft(iFd, CTX_TOTAL - CTX_CURSOR);
        CTX_TOTAL--;
    }
}
/*********************************************************************************************************
** ��������: __tshellCharDelete
** ��������: shell �յ�һ�� del ��ֵ.
** �䡡��  : iFd                           �ļ�������
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharDelete (INT  iFd, __PSHELL_INPUT_CTX  psicContext)
{
    if (CTX_CURSOR < CTX_TOTAL) {
        lib_memcpy(&CTX_BUFFER[CTX_CURSOR], 
                   &CTX_BUFFER[CTX_CURSOR + 1], 
                   CTX_TOTAL - CTX_CURSOR);
        CTX_BUFFER[CTX_TOTAL - 1] = ' ';
        write(iFd, &CTX_BUFFER[CTX_CURSOR], CTX_TOTAL - CTX_CURSOR);
        __tshellTtyCursorMoveLeft(iFd, CTX_TOTAL - CTX_CURSOR);
        CTX_TOTAL--;
    }
}
/*********************************************************************************************************
** ��������: __tshellCharHome
** ��������: shell �յ�һ�� home ��ֵ.
** �䡡��  : iFd                           �ļ�������
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharHome (INT  iFd, __PSHELL_INPUT_CTX  psicContext)
{
    if (CTX_CURSOR > 0) {
        __tshellTtyCursorMoveLeft(iFd, CTX_CURSOR);
        CTX_CURSOR = 0;
    }
}
/*********************************************************************************************************
** ��������: __tshellCharEnd
** ��������: shell �յ�һ�� end ��ֵ.
** �䡡��  : iFd                           �ļ�������
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharEnd (INT  iFd, __PSHELL_INPUT_CTX  psicContext)
{
    if (CTX_CURSOR < CTX_TOTAL) {
        __tshellTtyCursorMoveRight(iFd, CTX_TOTAL - CTX_CURSOR);
        CTX_CURSOR = CTX_TOTAL;
    }
}
/*********************************************************************************************************
** ��������: __tshellCharInster
** ��������: shell �յ�һ����ͨ��ֵ.
** �䡡��  : iFd                           �ļ�������
**           cChar                         ��ֵ
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharInster (INT  iFd, CHAR  cChar, __PSHELL_INPUT_CTX  psicContext)
{
    if (CTX_CURSOR == CTX_TOTAL) {
        write(iFd, &cChar, 1);
        CTX_BUFFER[CTX_CURSOR] = cChar;
        CTX_CURSOR++;
        CTX_TOTAL++;

    } else if (CTX_CURSOR < CTX_TOTAL) {
        lib_memcpy(&CTX_BUFFER[CTX_CURSOR + 1], 
                   &CTX_BUFFER[CTX_CURSOR], 
                   CTX_TOTAL - CTX_CURSOR);
                   
        CTX_BUFFER[CTX_CURSOR] = cChar;
        CTX_TOTAL++;
        write(iFd, &CTX_BUFFER[CTX_CURSOR], CTX_TOTAL - CTX_CURSOR);
        CTX_CURSOR++;
        __tshellTtyCursorMoveLeft(iFd, CTX_TOTAL - CTX_CURSOR);
    }
}
/*********************************************************************************************************
** ��������: __fillWhite
** ��������: �������ɸ��ո�
** �䡡��  : stLen      �ո�ĳ���
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __fillWhite (size_t  stLen)
{
    CHAR    cFmt[16];
    
    sprintf(cFmt, "%%-%zds", stLen);
    printf(cFmt, "");                                                   /*  ����ո�                    */
}
/*********************************************************************************************************
** ��������: __similarLen
** ��������: ����࿪ʼ��ѯ�����ַ������Ƶ��ַ���
** �䡡��  : pcStr1     �ַ���1
**           pcStr2     �ַ���2
** �䡡��  : ���Ƶĸ���
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static size_t  __similarLen (CPCHAR  pcStr1, CPCHAR  pcStr2)
{
    size_t  stSimilar = 0;

    while (*pcStr1 == *pcStr2) {
        if (*pcStr1 == PX_EOS) {
            break;
        }
        pcStr1++;
        pcStr2++;
        stSimilar++;
    }
    
    return  (stSimilar);
}
/*********************************************************************************************************
** ��������: __tshellFileMatch
** ��������: shell ���ݵ�ǰ�����������ƥ��.
** �䡡��  : iFd                           �ļ�������
**           pcDir                         �ļ���
**           pcFileName                    �ļ���
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellFileMatch (INT  iFd, PCHAR  pcDir, PCHAR  pcFileName, 
                                __PSHELL_INPUT_CTX  psicContext)
{
#define __TSHELL_BYTES_PERLINE          80                              /*  ���� 80 �ַ�                */
#define __TSHELL_BYTES_PERFILE          16                              /*  ÿ���ļ�����ʾ�񳤶�        */

    UINT             uiMath = 0;
    BOOL             bMathPrint = LW_FALSE;
    
    INT              iError;
    
    size_t           stPrintLen;
    size_t           stTotalLen = 0;
    size_t           stPad;
    size_t           stFileNameLen = lib_strlen(pcFileName);
    
    CHAR             cStat[MAX_FILENAME_LENGTH];
    CHAR             cHome[MAX_FILENAME_LENGTH];
    
    struct stat      statGet;
    struct winsize   winsz;
    
    struct dirent    direntcMatch;
    struct dirent   *pdirent;
    DIR             *pdir;
    size_t           stDirLen;
    
    size_t           stMinSimilar = 0;
    size_t           stSimilar;
    
    
    if (*pcDir == '~') {                                                /*  ��Ҫʹ�� HOME Ŀ¼���      */
        if (API_TShellGetUserHome(getuid(), cHome, 
                                  MAX_FILENAME_LENGTH) == ERROR_NONE) {
            lib_strlcat(cHome, pcDir + 1, MAX_FILENAME_LENGTH);
            pcDir = cHome;
        }
    }
    
    pdir = opendir(pcDir);                                              /*  �޷���Ŀ¼                */
    if (pdir == LW_NULL) {
        return;
    }
    
    if (ioctl(STD_OUT, TIOCGWINSZ, &winsz)) {                           /*  ��ô�����Ϣ                */
        winsz.ws_col = __TSHELL_BYTES_PERLINE;
    } else {
        winsz.ws_col = (unsigned short)ROUND_DOWN(winsz.ws_col, __TSHELL_BYTES_PERFILE);
    }
    
    stDirLen = lib_strlen(pcDir);
    
    do {
        pdirent = readdir(pdir);
        if (pdirent) {
            if ((stFileNameLen == 0) ||
                (lib_strncmp(pcFileName, pdirent->d_name, stFileNameLen) == 0)) {
                
                uiMath++;
                if (uiMath > 1) {                                       /*  ֻ��ӡƥ����              */
                    if (uiMath == 2) {
                        printf("\n");                                   /*  ����                        */
                    }
                    
                    stSimilar = __similarLen(direntcMatch.d_name, pdirent->d_name);
                    if (stSimilar < stMinSimilar) {
                        stMinSimilar = stSimilar;                       /*  ������С���ƶ�              */
                    }
                    
__print_dirent:
                    if (pcDir[stDirLen - 1] != PX_DIVIDER) {
                        snprintf(cStat, MAX_FILENAME_LENGTH, "%s/%s", pcDir, pdirent->d_name);
                    } else {
                        snprintf(cStat, MAX_FILENAME_LENGTH, "%s%s", pcDir, pdirent->d_name);
                    }
                    iError = stat(cStat, &statGet);
                    if (iError < 0) {
                        statGet.st_mode = DEFAULT_FILE_PERM | S_IFCHR;  /*  Ĭ������                    */
                    }
                    
                    if (S_ISDIR(statGet.st_mode)) {
                        lib_strlcat(pdirent->d_name, PX_STR_DIVIDER, MAX_FILENAME_LENGTH);
                    }
                    
                    API_TShellColorStart(pdirent->d_name, "", statGet.st_mode, STD_OUT);
                    stPrintLen = printf("%-15s ", pdirent->d_name);     /*  ��ӡ�ļ���                  */
                    if (stPrintLen > __TSHELL_BYTES_PERFILE) {
                        stPad = ROUND_UP(stPrintLen, __TSHELL_BYTES_PERFILE)
                              - stPrintLen;                             /*  �����������                */
                        __fillWhite(stPad);
                    } else {
                        stPad = 0;
                    }
                    stTotalLen += stPrintLen + stPad;
                    API_TShellColorEnd(STD_OUT);
                    
                    if (stTotalLen >= winsz.ws_col) {
                        printf("\n");                                   /*  ����                        */
                        stTotalLen = 0;
                    }
                
                    if (bMathPrint == LW_FALSE) {                       /*  �Ƿ�Ҳ��Ҫ�ѵ�һ��ƥ��Ĵ�ӡ*/
                        bMathPrint =  LW_TRUE;
                        pdirent    = &direntcMatch;
                        goto    __print_dirent;
                    
                    } else {                                            /*  �����¼��һ�εĽ��        */
                        lib_strcpy(direntcMatch.d_name, pdirent->d_name);
                    }
                
                } else {
                    direntcMatch = *pdirent;                            /*  ��¼��һ�� math �Ľڵ�      */
                    stMinSimilar = lib_strlen(pdirent->d_name);
                }
            }
        }
    } while (pdirent);
    
    closedir(pdir);
    
    if (uiMath > 1) {                                                   /*  ���ƥ��                    */
        if (stTotalLen) {
            printf("\n");                                               /*  ��������                    */
        }
        __tshellShowPrompt();                                           /*  ��ʾ������ʾ��              */
        
        if (stMinSimilar > stFileNameLen) {
            direntcMatch.d_name[stMinSimilar] = PX_EOS;                 /*  �Զ�ƥ�����������ƴ�        */
            lib_strlcat(CTX_BUFFER, 
                        &direntcMatch.d_name[stFileNameLen],
                        LW_CFG_SHELL_MAX_COMMANDLEN);
            CTX_TOTAL  = lib_strlen(CTX_BUFFER);
            CTX_CURSOR = CTX_TOTAL;
        }
        write(iFd, CTX_BUFFER, CTX_TOTAL);
        
    } else if (uiMath == 1) {                                           /*  ����һ��ƥ��                */
        size_t  stCatLen;
        
        if (pcDir[stDirLen - 1] != PX_DIVIDER) {
            snprintf(cStat, MAX_FILENAME_LENGTH, "%s/%s", pcDir, direntcMatch.d_name);
        } else {
            snprintf(cStat, MAX_FILENAME_LENGTH, "%s%s", pcDir, direntcMatch.d_name);
        }
        iError = stat(cStat, &statGet);
        if (iError < 0) {
            statGet.st_mode = DEFAULT_FILE_PERM | S_IFCHR;              /*  Ĭ������                    */
        }
        
        if (S_ISDIR(statGet.st_mode)) {                                 /*  �����Ŀ¼                  */
            lib_strlcat(direntcMatch.d_name, PX_STR_DIVIDER, 
                        MAX_FILENAME_LENGTH);                           /*  ��һ��Ŀ¼������            */
        }
        stCatLen = lib_strlen(direntcMatch.d_name) - stFileNameLen;
        
        write(iFd, &direntcMatch.d_name[stFileNameLen], stCatLen);
        
        lib_strlcat(CTX_BUFFER, &direntcMatch.d_name[stFileNameLen], LW_CFG_SHELL_MAX_COMMANDLEN);
        CTX_TOTAL  += (UINT)stCatLen;
        CTX_CURSOR  = CTX_TOTAL;
    }
}
/*********************************************************************************************************
** ��������: __tshellCharTab
** ��������: shell �յ�һ�� tab ����.
** �䡡��  : iFd                           �ļ�������
**           psicContext                   ��ǰ����������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __tshellCharTab (INT  iFd, __PSHELL_INPUT_CTX  psicContext)
{
#define __TTINY_SHELL_CMD_ISWHITE(pcCmd)    \
        ((*(pcCmd) == ' ') || (*(pcCmd) == '\t') || (*(pcCmd) == '\r') || (*(pcCmd) == '\n'))

#define __TTINY_SHELL_CMD_ISEND(pcCmd)      (*(pcCmd) == PX_EOS)

             INT         i;
             PCHAR       pcCmd;
             size_t      stStrLen;
             CHAR        cCommandBuffer[LW_CFG_SHELL_MAX_COMMANDLEN + 1];
             PCHAR       pcParamList[LW_CFG_SHELL_MAX_PARAMNUM + 1];    /*  �����б�                    */
             
             PCHAR       pcDir;
             PCHAR       pcFileName;
    REGISTER ULONG       ulError;
    
    if (CTX_CURSOR < CTX_TOTAL) {                                       /*  ������ƶ�����ĩ            */
        __tshellTtyCursorMoveRight(iFd, CTX_TOTAL - CTX_CURSOR);
        CTX_CURSOR = CTX_TOTAL;
    }
    CTX_BUFFER[CTX_TOTAL] = PX_EOS;                                     /*  �����������                */
    
    pcCmd = CTX_BUFFER;
    
    if (!pcCmd || __TTINY_SHELL_CMD_ISEND(pcCmd)) {                     /*  �������                    */
        return;
    }
    
    while (__TTINY_SHELL_CMD_ISWHITE(pcCmd)) {                          /*  ����ǰ��Ĳ��ɼ��ַ�        */
        pcCmd++;
        if (__TTINY_SHELL_CMD_ISEND(pcCmd)) {
            return;                                                     /*  ������Ч��������            */
        }
    }
    
    if (*pcCmd == '#') {                                                /*  ע����ֱ�Ӻ���              */
        return;
    }
    
    stStrLen = lib_strnlen(pcCmd, LW_CFG_SHELL_MAX_COMMANDLEN + 1);     /*  �����ַ�������              */
    if ((stStrLen > LW_CFG_SHELL_MAX_COMMANDLEN - 1) ||
        (stStrLen < 1)) {                                               /*  �ַ������ȴ���              */
        return;
    }
    
    lib_bzero(cCommandBuffer, LW_CFG_SHELL_MAX_COMMANDLEN + 1);         /*  ��� cCommandBuffer ������  */
    
    ulError = __tshellStrConvertVar(pcCmd, cCommandBuffer);             /*  �����滻                    */
    if (ulError) {
        return;
    }
    __tshellStrFormat(cCommandBuffer, cCommandBuffer);                  /*  ���� shell ����             */
    
    pcParamList[0] = cCommandBuffer;
    for (i = 0; i < LW_CFG_SHELL_MAX_PARAMNUM; i++) {                   /*  ��ʼ��ѯ����                */
        __tshellStrGetToken(pcParamList[i], 
                            &pcParamList[i + 1]);
        __tshellStrDelTransChar(pcParamList[i], pcParamList[i]);        /*  ɾ��ת���ַ����ת������    */
        if (pcParamList[i + 1] == LW_NULL) {                            /*  ��������                    */
            break;
        }
    }
    
    pcDir = pcParamList[i];                                             /*  ���������һ���ֶ�          */
    if (pcDir == LW_NULL) {
        return;
    }
    
    if (lib_strlen(pcDir) == 0) {                                       /*  û������, ��ǰĿ¼          */
        pcDir = ".";
        pcFileName = "";
        __tshellFileMatch(iFd, pcDir, pcFileName, psicContext);         /*  ��ʾĿ¼��ָ��ƥ����ļ�    */
        return;
    }
    
    pcFileName = lib_rindex(pcDir, PX_DIVIDER);
    if (pcFileName == LW_NULL) {                                        /*  ��ǰĿ¼                    */
        pcFileName = pcDir;
        pcDir = ".";

    } else {
        if (pcFileName == pcDir) {                                      /*  ��Ŀ¼�µ�һ��Ŀ¼          */
            pcDir = PX_STR_DIVIDER;
            pcFileName++;
        
        } else {
            *pcFileName = PX_EOS;
            pcFileName++;
        }
    }
    
    __tshellFileMatch(iFd, pcDir, pcFileName, psicContext);             /*  ��ʾĿ¼��ָ��ƥ����ļ�    */
}
/*********************************************************************************************************
** ��������: __tshellReadline
** ��������: shell ���ն˶�ȡһ������.
** �䡡��  : iFd                           �ļ�������
**           pvBuffer                      ���ջ�����
**           stMaxBytes                    ���ջ�������С
** �䡡��  : ��ȡ����
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
ssize_t  __tshellReadline (INT  iFd, PVOID  pcBuffer, size_t  stSize)
{
#define __CHK_READ_OUT(res)     if (sstReadNum <= 0) {  \
                                    goto    __out;  \
                                }

    INT                         iOldOption = OPT_TERMINAL;
    INT                         iNewOption;
    ssize_t                     sstReadNum;
    __SHELL_INPUT_CTX           sicContext;
    CHAR                        cRead;
    PVOID                       pvCookie = LW_NULL;
    
    if (__tshellReadlineInit() < ERROR_NONE) {                          /*  ��ʼ����ǰ�߳� Readline��� */
        return  (0);
    }
    
    sicContext.SIC_uiCursor   = 0;
    sicContext.SIC_uiTotalLen = 0;
    sicContext.SIC_cInputBuffer[0] = PX_EOS;

    ioctl(iFd, FIOGETOPTIONS, &iOldOption);
    iNewOption = iOldOption & ~(OPT_ECHO | OPT_LINE);                   /*  no echo no line mode        */
    ioctl(iFd, FIOSETOPTIONS, iNewOption);

    while (sicContext.SIC_uiTotalLen < LW_CFG_SHELL_MAX_COMMANDLEN) {
        sstReadNum = read(iFd, &cRead, 1);
        __CHK_READ_OUT(sstReadNum);
        
__re_check_key:
        if (__KEY_IS_ESC(cRead)) {                                      /*  ���ܼ���ʼ                  */
            sstReadNum = read(iFd, &cRead, 1);
            __CHK_READ_OUT(sstReadNum);
            
            if (cRead == 91) {                                          /*  ������м��                */
                sstReadNum = read(iFd, &cRead, 1);
                __CHK_READ_OUT(sstReadNum);
                
                switch (cRead) {
                
                case __FUNC_KEY_LEFT:                                   /*  ��                          */
                    __tshellCharLeft(iFd, &sicContext);
                    break;
                
                case __FUNC_KEY_UP:                                     /*  ��                          */
                    __tshellCharUp(iFd, &pvCookie, &sicContext);
                    break;
                    
                case __FUNC_KEY_RIGHT:                                  /*  ��                          */
                    __tshellCharRight(iFd, &sicContext);
                    break;
                
                case __FUNC_KEY_DOWN:                                   /*  ��                          */
                    __tshellCharDown(iFd, &pvCookie, &sicContext);
                    break;
                    
                case __FUNC_KEY_HOME:                                   /*  home key                    */
                    __tshellCharHome(iFd, &sicContext);
                    sstReadNum = read(iFd, &cRead, 1);
                    __CHK_READ_OUT(sstReadNum);
                    if (cRead != 126) {
                        goto    __re_check_key;
                    }
                    break;
                
                case __FUNC_KEY_DELETE:                                 /*  delete key                  */
                    __tshellCharDelete(iFd, &sicContext);
                    sstReadNum = read(iFd, &cRead, 1);
                    __CHK_READ_OUT(sstReadNum);
                    if (cRead != 126) {
                        goto    __re_check_key;
                    }
                    break;
                
                case __FUNC_KEY_END:                                    /*  end key                     */
                    __tshellCharEnd(iFd, &sicContext);
                    sstReadNum = read(iFd, &cRead, 1);
                    __CHK_READ_OUT(sstReadNum);
                    if (cRead != 126) {
                        goto    __re_check_key;
                    }
                    break;
                }
            } else {
                goto    __re_check_key;
            }
        
        } else if (__KEY_IS_TAB(cRead)) {                               /*  tab                         */
            __tshellCharTab(iFd, &sicContext);
        
        } else if (__KEY_IS_BS(cRead)) {                                /*  backspace                   */
            __tshellCharBackspace(iFd, cRead, &sicContext);
        
        } else if (__KEY_IS_LF(cRead)) {                                /*  ����                        */
            write(iFd, &cRead, 1);                                      /*  echo                        */
            break;
        
        } else if (__KEY_IS_EOT(cRead)) {                               /*  CTL+D ǿ���˳�              */
            lib_strcpy(sicContext.SIC_cInputBuffer, 
                       __TTINY_SHELL_FORCE_ABORT);
            sicContext.SIC_uiTotalLen = (UINT)lib_strlen(__TTINY_SHELL_FORCE_ABORT);
            break;
        
        } else {
            __tshellCharInster(iFd, cRead, &sicContext);
        }
    }
    sicContext.SIC_cInputBuffer[sicContext.SIC_uiTotalLen] = PX_EOS;    /*  �������                    */
    
    if (lib_strlen(sicContext.SIC_cInputBuffer)) {
        __tshellHistorySave(&sicContext);                               /*  ��¼��ʷ����                */
    }

__out:
    ioctl(iFd, FIOSETOPTIONS, iOldOption);                              /*  �ָ���ǰ�ն�״̬            */
    
    lib_strlcpy((PCHAR)pcBuffer, sicContext.SIC_cInputBuffer, stSize);
    
    return  ((ssize_t)sicContext.SIC_uiTotalLen);
}

#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/