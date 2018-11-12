#*********************************************************************************************************
#
#                                    �й�������Դ��֯
#
#                                   Ƕ��ʽʵʱ����ϵͳ
#
#                                SylixOS(TM)  LW : long wing
#
#                               Copyright All Rights Reserved
#
#--------------�ļ���Ϣ--------------------------------------------------------------------------------
#
# ��   ��   ��: libffi.mk
#
# ��   ��   ��: RealEvo-IDE
#
# �ļ���������: 2017 �� 05 �� 24 ��
#
# ��        ��: ���ļ��� RealEvo-IDE ���ɣ��������� Makefile ���ܣ������ֶ��޸�
#*********************************************************************************************************

#*********************************************************************************************************
# Clear setting
#*********************************************************************************************************
include $(CLEAR_VARS_MK)

#*********************************************************************************************************
# Target
#*********************************************************************************************************
LOCAL_TARGET_NAME := libffi.so

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS := \
./libffi/src/java_raw_api.c \
./libffi/src/closures.c \
./libffi/src/raw_api.c \
./libffi/src/prep_cif.c \
./libffi/src/types.c

LOCAL_ARM_SRCS := \
./libffi/src/arm/ffi.c \
./libffi/src/arm/sysv.S

LOCAL_MIPS_SRCS := \
./libffi/src/mips/ffi.c \
./libffi/src/mips/o32.S

LOCAL_MIPS64_SRCS := \
./libffi/src/mips/ffi.c \
./libffi/src/mips/n32.S

LOCAL_PPC_SRCS := \
./libffi/src/powerpc/ffi.c \
./libffi/src/powerpc/ffi_sysv.c \
./libffi/src/powerpc/sysv.S \
./libffi/src/powerpc/ppc_closure.S

LOCAL_SPARC_SRCS := \
./libffi/src/sparc/ffi.c \
./libffi/src/sparc/v8.S \
./libffi/src/sparc/v9.S

LOCAL_X86_SRCS := \
./libffi/src/x86/ffi.c \
./libffi/src/x86/sysv.S

LOCAL_X64_SRCS := \
./libffi/src/x86/ffi64.c \
./libffi/src/x86/unix64.S

LOCAL_RISCV_SRCS := \
./libffi/src/riscv/ffi.c \
./libffi/src/riscv/sysv.S

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH :=  \
-I"./libffi/include" \
-I"./libffi/src"

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL :=  \
-DHAVE_CONFIG_H

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS := 
LOCAL_CXXFLAGS := 

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB      := 
LOCAL_DEPEND_LIB_PATH := 

#*********************************************************************************************************
# C++ config
#*********************************************************************************************************
LOCAL_USE_CXX        := no
LOCAL_USE_CXX_EXCEPT := no

#*********************************************************************************************************
# Code coverage config
#*********************************************************************************************************
LOCAL_USE_GCOV := no

#*********************************************************************************************************
# OpenMP config
#*********************************************************************************************************
LOCAL_USE_OMP := no

#*********************************************************************************************************
# User link command
#*********************************************************************************************************
LOCAL_PRE_LINK_CMD   := 
LOCAL_POST_LINK_CMD  := 
LOCAL_PRE_STRIP_CMD  := 
LOCAL_POST_STRIP_CMD := 

include $(LIBRARY_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************