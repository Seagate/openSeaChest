#
# Do NOT modify or remove this copyright and license
#
# Copyright (c) 2012 - 2017 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
#
# This software is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ******************************************************************************************

# Hand Written Makefile (Edit with caution) -Muhammad
# 

DESC_ACCEPTANCE_LEVEL=certified

# When this is not empty, the infrastructure will look for the files
# $(DEV_SIGNING_PREFIX).key and $(DEV_SIGNING_PREFIX).cert.  If the
# prefix contains a slash, it is assumed to be a full path to the
# directory containing the key/cert files, and that simply appending
# ".key" and ".cert" will give the actual file names.  Otherwise, it
# looks in the vib-suite installation directory's "testcerts"
# directory for the files.
DEV_SIGNING_PREFIX=vmware

BULL_KBURL=http://kb.vmware.com/kb/example.html
BULL_PLATFORMS=bulletin-platforms.url
BULL_VENDOR_CODE=VMW

# Calculate VMKAPIDDK_ROOT, BUILD_NUMBER

HAS_NATIVEDDK := $(shell rpm -qa | grep vmware-esx-nativeddk-devtools)
ifdef HAS_NATIVEDDK
DDK=vmware-esx-nativeddk-devtools
HEXDK=vmware-esx-hexdk-devtools
else
DDK=vmware-esx-vmkapiddk-devtools
HEXDK=vmware-esx-esxcli-devtools
endif

VMKAPIDDK_ROOT = $(shell rpm -q --qf '%{INSTPREFIXES}\n' $(DDK))
BUILD_NUMBER=$(shell echo $(VMKAPIDDK_ROOT) | cut -d '-' -f3)
DESC_ESX_VERSION=$(shell echo $(VMKAPIDDK_ROOT) | cut -d '-' -f2)

# include $(VMKAPIDDK_ROOT)/src/partners/samples/common-mgmt-infrastructure/Makefile.user

# Makefile
#
# This builds the Userworld Tool binary for the example being packaged here.
#
# type:
#  make
# to build the Userworld Tool.
#
# Expected usage is that this Makefile is copied to a new source directory,
# and the variables above the "Nothing beyond this point" comment are
# adjusted appropriately.
#



################################################################
# Set build configuration options here.
#
# Variables starting with "DEV_" are reserved for communicating
# build parameters into the core makefiles (in .../samples/common).
#
# Those starting with "VMW_" are reserved for VMware use, and should
# be considered unsupported with no compatibility guarantees unless
# explicitly documented otherwise.
#
# Note that many variables do not currently follow the above
# patterns.  This is to minimize changes required in existing
# partner code.

################
# First, define the end environment(s) with the following two
# variables.  As shown here, one is defined.  If more than one is
# necessary, these values will need to be set on a per-target basis,
# or in a loop that recursively invokes make with each set of values
# provided on the command line.
#
# This parameter must be set.

# Legal values for DEV_TARGET_CONTEXT are "vmkernel" or "uw".
DEV_TARGET_CONTEXT = uw

################
# This controls which class of compiler options to use.  Allowed
# values are "beta" (for debugging) and "release".  The default is
# the type of the ESX build that produced this devkit.
DEV_OBJDIR = beta

################
# Typically, it won't be necessary to modify this setting, but the
# substitutions pattern may need to put the intermediate files in a
# more convenient location.

################
# If creating a standalone userworld application, as opposed to a
# shared object that gets loaded by some other program, uncomment
# the following assignment.  The value needs to be empty or non-
# empty.  If the latter, any content may be used.
DEV_APP = 1
#DEV_APP = 

################
# Extra compiler and preprocessor flags based on kind of build
# (release or beta).  These default to being empty.
# DEV_beta_COMPILEFLAGS = -Dmy_debug_flag
# DEV_release_COMPILEFLAGS = -Dmy_optimized_flag

################
# Define here any additional libraries required by uncommenting the
# appropriate variable(s) and modifying as needed.  If the library is
# not included in the devkit distribution, include a -L /some/path
# parameter before the -lname parameter so that the linker can find
# it.  The *PRE* variables are included as the first library
# directives.  The *POST* variables are included as the last library
# directives.
#
# The default values are empty.

# DEV_release_PRE_LIBS =
# DEV_beta_PRE_LIBS =

# The third assignment below is semantically equivalent to the first
# two assignments.

# DEV_release_POST_LIBS = -lm
# DEV_beta_POST_LIBS = -lm
# or
# DEV_$(DEV_OBJDIR)_POST_LIBS = -lm

################
# If the vdsPublic library needs to be included in the compilation,
# uncomment the following assignment.  Currently, that library only
# comes as part of the "kmdk-devtools-vds" devkit, so it and any
# other devkit components it depends on will need to be installed
# on the same development host as this devkit.  The value must be
# empty for not needing vdsPublic, and non-empty for needing it.
# The actual content is not examined.

# DEV_NEEDVDSLIB = 1

################
# Request that the management library be included when we link.  This
# has to be explicitly done here because we're hooking into the base
# HExDK's build system, and it doesn't do that inclusion by default.

DEV_NEED_VMKMGMT = 1

################################################################
# Nothing beyond this point should require modification.

GREP := /build/toolchain/lin32/grep-2.5.1a/bin/grep

IMPORT_DEVKIT = $(shell rpm -q --qf '%{INSTPREFIXES}\n' $(1) | $(GREP) $(BUILD_NUMBER))

HEXDK_BASE := $(call IMPORT_DEVKIT,$(HEXDK))
VMKAPIDDK_BASE := $(call IMPORT_DEVKIT,$(DDK))

ifeq ($(DESC_ESX_VERSION),6.5.0)
VMKAPI_LOCATION = $(VMKAPIDDK_BASE)/src/bora/vmkernel/include/vmkapi/v2_4_0_0
else
ifeq ($(DESC_ESX_VERSION),6.0.0)
VMKAPI_LOCATION = $(VMKAPIDDK_BASE)/src/bora/vmkernel/include/vmkapi/v2_3_0_0
else
VMKAPI_LOCATION = $(VMKAPIDDK_BASE)/src/bora/vmkernel/include/vmkapi/v2_2_0_0
endif
endif

VMKAPI_LOCATION = $(dir $(shell find $(VMKAPIDDK_ROOT)/src/bora -name vmkapi.h))

# Only need to evaluate once (aka use ":=").
CURRENT_DIR := $(shell pwd)
BUILD_DIR := build
SCRATCH_DIR := $(BUILD_DIR)/scratch
VIB_DIR := $(BUILD_DIR)/vib
BUNDLE_DIR := $(BUILD_DIR)/bundle
STAGE_DIR := $(BUILD_DIR)/stage
TARGET_XML_DIR := /usr/lib/vmware/esxcli/ext

VIB_DESCRIPTOR_XML := $(SCRATCH_DIR)/descriptor.xml
VIB_BULLETIN_XML := $(SCRATCH_DIR)/bulletin.xml

# Need to evaluate at every reference, as DEV_OBJDIR, etc, can change
# during a build (aka use "=").  Also, some things get set/reset in
# common included files, we reference them, and we haven't included the files
# yet.
BINARIES_DIR = $(BUILD_DIR)/$(DEV_OBJDIR)
INSTALL_DIR = /opt/vmware/nvme
STAGE_XML_DIR = $(STAGE_DIR)$(TARGET_XML_DIR)

#
# Check paths
#

ifeq ($(wildcard $(HEXDK_BASE)/samples/common/userworld-tool.inc),)
$(error Host-Extensions devkit build $(BUILD_NUMBER) required but not installed.)
endif

ifeq ($(wildcard $(VMKAPI_LOCATION)/vmkapi.h),)
$(error VMKAPI DDK devkit build $(BUILD_NUMBER) required but not installed.)
endif

# Note that this hard-codes the prefix path at installation
# time.  This allows for having multiple different versions
# of the devkit installed and usable simultaneously.

include $(HEXDK_BASE)/samples/common/userworld-tool.inc

#VMW_EXTRA_DEFS += -I$(VMKAPI_LOCATION) -I$(VMKAPI_LOCATION)/** -I$(DEV_COMMON_DIR) \
	-DVMK_DEVKIT_HAS_API_VMKAPI_BASE \
	-DVMK_DEVKIT_USES_PUBLIC_APIS \
	-DVMK_DEVKIT_USES_BINARY_COMPATIBLE_APIS

VMW_VERSION := 6.5.0
VMW_BUILD   := 4598673
VMW_RELEASE := 0.0.4598673

ifeq ($(strip $(DEV_OBJDIR)),)
   # Compress all the internal build types down to the two external types.
   ifeq ($(strip release),release)
      DEV_OBJDIR = release
   else
      DEV_OBJDIR = beta
   endif
endif

# For this devkit, it's reasonable to default to userworld builds.
# So do that instead of bailing out if the user didn't tell us.
DEV_TARGET_CONTEXT ?= uw

# This will enable later generalization for supporting both userworld
# and vmkernel builds with the same make infrastructure.

ifeq ($(strip $(DEV_TARGET_BITNESS)),)
   ifeq ($(strip $(DEV_TARGET_CONTEXT)),uw)
      DEV_TARGET_BITNESS = 32
   else
      DEV_TARGET_BITNESS = 64
   endif
endif

VMW_TARGET = $(DEV_TARGET_CONTEXT)$(DEV_TARGET_BITNESS)_$(DEV_OBJDIR)


CAYMAN_ESX_GDB_DIR = $(shell /bin/rpm -q --qf "%{instprefixes}\n" vmware-esx-cayman_esx_gdb-24f65a7191349603bb2c712d51114d75e92e5065-toolchain)
export GOBUILD_CAYMAN_ESX_GDB_ROOT = $(CAYMAN_ESX_GDB_DIR)
CAYMAN_ESX_TOOLCHAIN_DIR = $(shell /bin/rpm -q --qf "%{instprefixes}\n" vmware-esx-cayman_esx_toolchain-9307983f3985bde80570de52ee3b8b1e0b7b6c85-toolchain)
export GOBUILD_CAYMAN_ESX_TOOLCHAIN_ROOT = $(CAYMAN_ESX_TOOLCHAIN_DIR)
CAYMAN_ESX_NCURSES_DIR = $(shell /bin/rpm -q --qf "%{instprefixes}\n" vmware-esx-cayman_esx_ncurses-a09432d1ad9e9327bda85fa9293f8d2f980dedd6-toolchain)
export GOBUILD_CAYMAN_ESX_NCURSES_ROOT = $(CAYMAN_ESX_NCURSES_DIR)
CAYMAN_ESX_GLIBC_DIR = $(shell /bin/rpm -q --qf "%{instprefixes}\n" vmware-esx-cayman_esx_glibc-7fc626201c38c3a7e2472179c969a565b101f7ba-toolchain)
export GOBUILD_CAYMAN_ESX_GLIBC_ROOT = $(CAYMAN_ESX_GLIBC_DIR)
CAYMAN_PYTHON_DIR = $(shell /bin/rpm -q --qf "%{instprefixes}\n" vmware-esx-cayman_python-bc9711fc81a21c5629bda33c48e1f301ff43cbae-toolchain)
export GOBUILD_CAYMAN_PYTHON_ROOT = $(CAYMAN_PYTHON_DIR)
VMW_COMPONENTS = cayman_esx_gdb cayman_esx_toolchain cayman_esx_ncurses cayman_esx_glibc cayman_python
VMW_COMPONENT_DIRS = $(CAYMAN_ESX_GDB_DIR) $(CAYMAN_ESX_TOOLCHAIN_DIR) $(CAYMAN_ESX_NCURSES_DIR) $(CAYMAN_ESX_GLIBC_DIR) $(CAYMAN_PYTHON_DIR)

AWK = /build/toolchain/lin32/gawk-3.1.6/bin/awk
CHMOD = /build/toolchain/lin32/coreutils-5.97/bin/chmod
CP = /build/toolchain/lin32/coreutils-5.97/bin/cp
ECHO = /build/toolchain/lin32/coreutils-5.97/bin/echo
GREP = /build/toolchain/lin32/grep-2.5.1a/bin/grep
GZIP = /build/toolchain/lin32/gzip-1.3.5/bin/gzip
MAKE = /build/toolchain/lin32/make-3.81/bin/make
MKDIR = /build/toolchain/lin32/coreutils-5.97/bin/mkdir
MV = /build/toolchain/lin32/coreutils-5.97/bin/mv
RM = /build/toolchain/lin32/coreutils-5.97/bin/rm
SED = /build/toolchain/lin32/sed-4.1.5/bin/sed
TR = /build/toolchain/lin32/coreutils-8.6/bin/tr

_VMW_RPM_PREFIX	:= vmware-esx

# This import capability is generic, so that user code can get a reference
# onto a companion devkit (say, a management app needs to see the headers
# for a driver).  Since we've got it anyway, might as well use it for our
# own needs as well.
#
# Note: because of the way make expands the call function, extraneous
# whitespace can cause obscure problems.  In particular, don't put any
# next to the commas.

VMW_IMPORT_DEVKIT = $(shell /bin/rpm -q --qf '%{INSTPREFIXES}\n' $(1)$(2))

VMW_KMDK_BASE := $(call VMW_IMPORT_DEVKIT,$(_VMW_RPM_PREFIX)-kmdk-devtools-vds-$(VMW_VERSION)-,$(VMW_RELEASE))
VMW_DRIVER_DEVKIT_BASE := $(call VMW_IMPORT_DEVKIT,$(_VMW_RPM_PREFIX)-nativeddk-devtools-$(VMW_VERSION)-,$(VMW_RELEASE))

#VMW_VIB_BASE := $(shell $(HEXDK_BASE)/tools/find_rpm_install_dir.sh $(_VMW_RPM_PREFIX)-vib-suite)

#ifeq ($(strip $(VMW_VIB_BASE)),)
#   $(error Could not locate vib-suite.)
#endif

# This is for backwards compatibility.  The VIBPATH variable is
# mostly being deprecated.

#VIBPATH := $(VMW_VIB_BASE)

VMW_uw32_beta_CCBIN = $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/bin/x86_64-vmk-linux-gnu-gcc
VMW_uw32_beta_OBJCOPYBIN = $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/bin/x86_64-vmk-linux-gnu-objcopy
#VMW_uw32_beta_COMPILEFLAGS = -pthread -fwrapv -pipe -fno-working-directory -fno-strict-aliasing -funwind-tables -fasynchronous-unwind-tables -Wall -Werror -O2 -march=i686 -g -fPIC -fstack-protector -fstack-protector-all -fno-strict-aliasing -Wno-unused-but-set-variable -Wdeclaration-after-statement -Wno-unused-value -Wno-pointer-sign -Wno-strict-prototypes -Wno-enum-compare -Wno-switch --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot -m32 -isystem $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/lib/gcc/x86_64-vmk-linux-gnu/4.8.4/include -D_LARGEFILE64_SOURCE -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -DVMX86_DEBUG -D_FORTIFY_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -Dlinux -DVMX86_BETA
#VMW_uw32_beta_COMPILEFLAGS = -pthread -fwrapv -pipe -fno-working-directory -fno-strict-aliasing -funwind-tables -fasynchronous-unwind-tables -Wall -Werror -O2 -march=i686 -g -fPIC -fstack-protector -fstack-protector-all -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-value -Wno-enum-compare -Wno-switch --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot -m32 -isystem $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/lib/gcc/x86_64-vmk-linux-gnu/4.8.4/include -D_LARGEFILE64_SOURCE -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -DVMX86_DEBUG -D_FORTIFY_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -Dlinux -DVMX86_BETA
#The above compilation flags is the correct one but it is causing issue dur to -Werror so removing it
VMW_uw32_beta_COMPILEFLAGS = -pthread -fwrapv -pipe -fno-working-directory -fno-strict-aliasing -funwind-tables -fasynchronous-unwind-tables -Wall -O2 -march=i686 -g -fPIC -fstack-protector -fstack-protector-all -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-value -Wno-enum-compare -Wno-switch --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot -m32 -isystem $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/lib/gcc/x86_64-vmk-linux-gnu/4.8.4/include -D_LARGEFILE64_SOURCE -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -DVMX86_DEBUG -D_FORTIFY_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -Dlinux -DVMX86_BETA
#VMW_uw32_beta_COMPILEFLAGS = -pthread -fwrapv -pipe -fno-working-directory -fno-strict-aliasing -funwind-tables -fasynchronous-unwind-tables -Wall -O2 -march=i686 -g -fPIC -fstack-protector -fstack-protector-all -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-value -Wno-enum-compare -Wno-switch --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot -m32 -isystem $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/lib/gcc/x86_64-vmk-linux-gnu/4.8.4/include -D_LARGEFILE64_SOURCE -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -DVMX86_DEBUG -D_FORTIFY_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -Dlinux -DVMX86_BETA
VMW_uw32_beta_LINKFLAGS =  -pthread -Wl,-z,relro -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,defs -Wl,--hash-style=gnu --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot

VMW_uw32_release_CCBIN = $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/bin/x86_64-vmk-linux-gnu-gcc
VMW_uw32_release_OBJCOPYBIN = $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/bin/x86_64-vmk-linux-gnu-objcopy
#VMW_uw32_release_COMPILEFLAGS = -pthread -fwrapv -pipe -fno-working-directory -fno-strict-aliasing -funwind-tables -fasynchronous-unwind-tables -Wall -Werror -DNDEBUG -O2 -march=i686 -g -fPIC -fstack-protector -fno-strict-aliasing -Wno-unused-but-set-variable -Wdeclaration-after-statement -Wno-unused-value -Wno-pointer-sign -Wno-strict-prototypes -Wno-enum-compare -Wno-switch --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot -m32 -isystem $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/lib/gcc/x86_64-vmk-linux-gnu/4.8.4/include -D_LARGEFILE64_SOURCE -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -DVMX86_RELEASE -D_FORTIFY_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -Dlinux
#VMW_uw32_release_COMPILEFLAGS = -pthread -fwrapv -pipe -fno-working-directory -fno-strict-aliasing -funwind-tables -fasynchronous-unwind-tables -Wall -Werror -DNDEBUG -O2 -march=i686 -g -fPIC -fstack-protector -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-value -Wno-enum-compare -Wno-switch --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot -m32 -isystem $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/lib/gcc/x86_64-vmk-linux-gnu/4.8.4/include -D_LARGEFILE64_SOURCE -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -DVMX86_RELEASE -D_FORTIFY_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -Dlinux
#The above compilation flags is the correct one but it is causing issue dur to -Werror so removing it
VMW_uw32_release_COMPILEFLAGS = -pthread -fwrapv -pipe -fno-working-directory -fno-strict-aliasing -funwind-tables -fasynchronous-unwind-tables -Wall -DNDEBUG -O2 -march=i686 -g -fPIC -fstack-protector -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-value -Wno-enum-compare -Wno-switch --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot -m32 -isystem $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/lib/gcc/x86_64-vmk-linux-gnu/4.8.4/include -D_LARGEFILE64_SOURCE -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -DVMX86_RELEASE -D_FORTIFY_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -Dlinux
#VMW_uw32_release_COMPILEFLAGS = -pthread -fwrapv -pipe -fno-working-directory -fno-strict-aliasing -funwind-tables -fasynchronous-unwind-tables -Wall -DNDEBUG -O2 -march=i686 -g -fPIC -fstack-protector -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-value -Wno-enum-compare -Wno-switch --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot -m32 -isystem $(CAYMAN_ESX_TOOLCHAIN_DIR)/usr/lib/gcc/x86_64-vmk-linux-gnu/4.8.4/include -D_LARGEFILE64_SOURCE -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -DVMX86_RELEASE -D_FORTIFY_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -Dlinux
VMW_uw32_release_LINKFLAGS =  -pthread -Wl,-z,relro -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,defs -Wl,--hash-style=gnu --sysroot=$(CAYMAN_ESX_GLIBC_DIR)/sysroot

# In theory, the compiler already knows what version of the basic libraries
# it should be using, so we don't need to specify any of them here.  Any
# path modifications were done when the devkit was put together.

VMW_EXTRA_DEFS := -DBUILT_BY_$(USER) $(DEV_$(DEV_OBJDIR)_COMPILEFLAGS)

# Need to set variable type so that it's always fully expanded upon
# reference.
VMW_LINK_FLAGS = $(VMW_$(DEV_OBJDIR)_LINKFLAGS)

ifneq ($(strip $(DEV_APP)),)
   # Userworld apps need to be position-independent executables, so
   # tell gcc about it.
   VMW_LINK_FLAGS += -pie
else
   VMW_LINK_FLAGS += -shared
endif

# If the user requested that the vdsPublic library be used, and it's
# available on the system, add the bits the compiler/linker need
# for finding it.

# xxx should rename to DEV_NEED_VDSLIB.
ifneq ($(strip $(DEV_NEEDVDSLIB)),)
   ifneq ($(strip $(VMW_KMDK_BASE)),)
      VMW_VDS_LIB := -L $(VMW_KMDK_BASE)/uw-vds/lib/$(DEV_OBJDIR) -lvdsPublic
      VMW_EXTRA_DEFS += -I$(VMW_KMDK_BASE)/uw-vds/include
   else
      $(error VDS library requested, but kmdk-devtools-vds devkit not found.)
   endif
endif

# Similarly, if we have a request for the VMKernel Management library,
# find it and update variables accordingly.

ifneq ($(strip $(DEV_NEED_VMKMGMT)),)
   ifneq ($(strip $(VMW_DRIVER_DEVKIT_BASE)),)
      VMW_MGMT_LIB += \
         -L $(VMW_DRIVER_DEVKIT_BASE)/lib \
         -L $(VMW_DRIVER_DEVKIT_BASE)/lib$(DEV_TARGET_BITNESS) \
         -lvmkmgmt -lstdc++
      VMW_EXTRA_DEFS += -I$(VMW_DRIVER_DEVKIT_BASE)/src/bora/public \
         -I$(VMW_DRIVER_DEVKIT_BASE)/src/bora/vmkernel/include/vmkapi/v2_4_0_0
   else
      $(error VMKernel Management library requested, but build $(VMW_BUILD) of nativeddk-devtools rpm not found.)
   endif
else
   VMW_MGMT_LIB :=
endif

ifneq ($(strip $(DEV_NEED_NICMGMT)),)
   VMW_NICMGMT_LIB += -L $(HEXDK_BASE)/lib \
                      -lnicmgmt -lvmkuserlib
   VMW_EXTRA_DEFS += -I $(HEXDK_BASE)/include
else
   VMW_NIC_LIB :=
endif

VMW_LINK_FLAGS += \
	$(DEV_$(DEV_OBJDIR)_PRE_LIBS) \
	$(VMW_VDS_LIB) $(VMW_MGMT_LIB) $(VMW_NICMGMT_LIB)\
	$(DEV_$(DEV_OBJDIR)_POST_LIBS)

# Usage:
# objects: sources
#	$(CC_CMD)
# The $* and $@ references take care of the grungy details as long as
# the objects/sources mapping is valid.

CC_CMD = $(VMW_$(VMW_TARGET)_CCBIN) -c -o $@ \
	$(VMW_EXTRA_DEFS) \
	$(VMW_$(VMW_TARGET)_COMPILEFLAGS) \
	$<

# Usage:
# deliverable: objects
#	$(LINK_CMD)
# This assumes that $(DEV_$(DEV_OBJDIR)_OBJECTS) lists all the object
# files that are to be linked to produce "deliverable".  That is, in
# fact, the semantic definition of that variable.

LINK_CMD = $(VMW_$(VMW_TARGET)_CCBIN) -o $@ \
	$(VMW_$(VMW_TARGET)_COMPILEFLAGS) \
	$(DEV_$(DEV_OBJDIR)_OBJECTS) \
	$(VMW_LINK_FLAGS)

VMW_EXTRA_DEFS += -I$(VMKAPI_LOCATION) -I$(VMKAPI_LOCATION)lib \
	-DVMK_DEVKIT_HAS_API_VMKAPI_BASE \
	-DVMK_DEVKIT_USES_PUBLIC_APIS \
	-DVMK_DEVKIT_USES_BINARY_COMPATIBLE_APIS
#VMW_EXTRA_DEFS += -I$(VMKAPI_LOCATION) -I$(VMKAPI_LOCATION)lib \
	-DVMK_DEVKIT_USES_PUBLIC_APIS \
	-DVMK_DEVKIT_USES_BINARY_COMPATIBLE_APIS


UTIL_SRC_DIR=../../utils/C/openSeaChest
CC = $(VMW_$(VMW_TARGET)_CCBIN)
LIB_FILE_OUTPUT_DIR=lib
STRIP = strip
CFLAGS = -Wall -c $(VMW_$(VMW_TARGET)_COMPILEFLAGS) -std=gnu99
CFLAGS += -DVMK_CROSS_COMP
#CFLAGS_I686 = -Wall -c -m32
LFLAGS = \
	-Wall \
	../../opensea-operations/Make/vmware/$(LIB_FILE_OUTPUT_DIR)/libopensea-operations.a \
	../../opensea-transport/Make/vmware/$(LIB_FILE_OUTPUT_DIR)/libopensea-transport.a \
	../../opensea-common/Make/vmware/$(LIB_FILE_OUTPUT_DIR)/libopensea-common.a \
	$(VMW_$(VMW_TARGET)_COMPILEFLAGS) \
	$(DEV_$(DEV_OBJDIR)_OBJECTS) $(VMW_LINK_FLAGS) \
	-lm -lrt
TCGLFLAGS = \
	-Wall \
	../../opensea-operations/Make/vmware/$(LIB_FILE_OUTPUT_DIR)/libopensea-operations.a \
	../../SeaCTCGOperations/Make/vmware/$(LIB_FILE_OUTPUT_DIR)/libSeaCTCGOperations.a \
	../../SeaCTCG/Make/vmware/$(LIB_FILE_OUTPUT_DIR)/libSeaCTCG.a \
	../../opensea-transport/Make/vmware/$(LIB_FILE_OUTPUT_DIR)/libopensea-transport.a \
	../../opensea-common/Make/vmware/$(LIB_FILE_OUTPUT_DIR)/libopensea-common.a \
	$(VMW_$(VMW_TARGET)_COMPILEFLAGS) \
	$(DEV_$(DEV_OBJDIR)_OBJECTS) $(VMW_LINK_FLAGS) \
	-lm -lrt
INC_DIR= \
	-I../../opensea-common/include \
	-I../../opensea-transport/include \
	-I../../include \
	-I../../opensea-operations/include

MACHINE := $(shell uname -m)

FILE_OUTPUT_DIR=openseachest_exes

ifeq ($(MACHINE), x86_64)

endif

#Files for the final binary
SOURCES = $(UTIL_SRC_DIR)/openSeaChest_Legacy.c ../../src/EULA.c
OUTFILE = openSeaChest
OBJS = $(SOURCES:.c=.o)

#add any defines needed for tool release.
#PROJECT_DEFINES += -DDISABLE_NVME_PASSTHROUGH -DDISABLE_TCG_SUPPORT
PROJECT_DEFINES += -DDISABLE_TCG_SUPPORT #-DDISABLE_NVME_PASSTHROUGH
PROJECT_DEFINES += $(VMW_EXTRA_DEFS) 

ERASEOUTFILE = openSeaChest_Erase
ERASESOURCES = $(UTIL_SRC_DIR)/openSeaChest_Erase.c ../../src/EULA.c ../../src/openseachest_util_options.c
ERASEOBJS = $(ERASESOURCES:.c=.o)

SMARTOUTFILE = openSeaChest_SMART
SMARTSOURCES = $(UTIL_SRC_DIR)/openSeaChest_SMART.c ../../src/EULA.c ../../src/openseachest_util_options.c
SMARTOBJS = $(SMARTSOURCES:.c=.o)

POWERCONTROLOUTFILE = openSeaChest_PowerControl
POWERCONTROLSOURCES = $(UTIL_SRC_DIR)/openSeaChest_PowerControl.c ../../src/EULA.c ../../src/openseachest_util_options.c
POWERCONTROLOBJS = $(POWERCONTROLSOURCES:.c=.o)

GENERICTESTSOUTFILE = openSeaChest_GenericTests
GENERICTESTSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_GenericTests.c ../../src/EULA.c ../../src/openseachest_util_options.c
GENERICTESTSOBJS = $(GENERICTESTSSOURCES:.c=.o)

BASICSOUTFILE = openSeaChest_Basics
BASICSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Basics.c ../../src/EULA.c ../../src/openseachest_util_options.c
BASICSOBJS = $(BASICSSOURCES:.c=.o)

CONFIGUREOUTFILE = openSeaChest_Configure
CONFIGURESOURCES = $(UTIL_SRC_DIR)/openSeaChest_Configure.c ../../src/EULA.c ../../src/openseachest_util_options.c
CONFIGUREOBJS = $(CONFIGURESOURCES:.c=.o)

INFOOUTFILE = openSeaChest_Info
INFOSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Info.c ../../src/EULA.c ../../src/openseachest_util_options.c
INFOOBJS = $(INFOSOURCES:.c=.o)

ZBDOUTFILE = openSeaChest_ZBD
ZBDSOURCES = $(UTIL_SRC_DIR)/openSeaChest_ZBD.c ../../src/EULA.c ../../src/openseachest_util_options.c
ZBDOBJS = $(ZBDSOURCES:.c=.o)

FORMATOUTFILE = openSeaChest_FormatUnit
FORMATSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Format.c ../../src/EULA.c ../../src/openseachest_util_options.c
FORMATOBJS = $(FORMATSOURCES:.c=.o)

NVMEOUTFILE = openSeaChest_NVMe
NVMESOURCES = $(UTIL_SRC_DIR)/openSeaChest_NVMe.c ../../src/EULA.c ../../src/openseachest_util_options.c
NVMEOBJS = $(NVMESOURCES:.c=.o)

FIRMWAREOUTFILE = openSeaChest_Firmware

.PHONY: clean_all all 

all: clean_all debug 

debug: CXXFLAGS += -g
debug: CFLAGS += -g
debug: PROJECT_DEFINES += -D_DEBUG
debug: ERASEOUTFILE := $(ERASEOUTFILE)_dbg
debug: SMARTOUTFILE := $(SMARTOUTFILE)_dbg
debug: POWERCONTROLOUTFILE := $(POWERCONTROLOUTFILE)_dbg
debug: GENERICTESTOUTFILE := $(GENERICTESTSOUTFILE)_dbg
debug: BASICSOUTFILE := $(BASICSOUTFILE)_dbg
debug: CONFIGUREOUTFILE := $(CONFIGUREOUTFILE)_dbg
debug: INFOOUTFILE := $(INFOOUTFILE)_dbg
debug: ZBDOUTFILE := $(ZBDOUTFILE)_dbg
debug: FORMATOUTFILE := $(FORMATOUTFILE)_dbg
debug: NVMEOUTFILE := $(NVMEOUTFILE)_dbg
#debug: mkoutputdir $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE)
debug: mkoutputdir $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(NVMEOUTFILE)

release: CXXFLAGS += -O3
release: CFLAGS += -O3
#release: mkoutputdir $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE)
release: mkoutputdir $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE)

export CFLAGS
export CXXFLAGS
export PROJECT_DEFINES


opensea-libs:
	$(MAKE) -C ../../opensea-common/Make/vmware
	$(MAKE) -C ../../opensea-transport/Make/vmware
	$(MAKE) -C ../../opensea-operations/Make/vmware

$(SMARTOUTFILE): $(SMARTOBJS) opensea-libs mkoutputdir 
	$(CC) $(SMARTOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(SMARTOUTFILE)
	#$(CC) $(SMARTOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE)_static --static

$(GENERICTESTSOUTFILE) : $(GENERICTESTSOBJS) opensea-libs mkoutputdir 
	$(CC) $(GENERICTESTSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(GENERICTESTSOUTFILE)
	#$(CC) $(GENERICTESTSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE)_static --static

$(POWERCONTROLOUTFILE) : $(POWERCONTROLOBJS) opensea-libs mkoutputdir 
	$(CC) $(POWERCONTROLOBJS) -o $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE) $(LFLAGS)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(POWERCONTROLOUTFILE)
	#$(CC) $(POWERCONTROLOBJS) -o $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE)_static $(LFLAGS) --static

$(OUTFILE): $(OBJS) opensea-libs mkoutputdir 
	$(CC) $(OBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(OUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(OUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(OUTFILE)
	#$(CC) $(OBJS) -static -o $(FILE_OUTPUT_DIR)/$(OUTFILE)_static $(LFLAGS)

$(BASICSOUTFILE): $(BASICSOBJS) opensea-libs mkoutputdir 
	$(CC) $(BASICSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(BASICSOUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(BASICSOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(BASICSOUTFILE)
	#$(CC) $(BASICSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(BASICSOUTFILE)_static --static
	
$(CONFIGUREOUTFILE): $(CONFIGUREOBJS) opensea-libs mkoutputdir 
	$(CC) $(CONFIGUREOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(CONFIGUREOUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(CONFIGUREOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(CONFIGUREOUTFILE)
	#$(CC) $(CONFIGUREOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(CONFIGUREOUTFILE)_static --static
	
#Commenting out for time being. Need to add VMWare support to Firmware makefile	
#$(FIRMWAREOUTFILE):
	#$(MAKE) -f Makefile.openSeaChest_firmware $(MAKECMDGOALS)
	
$(INFOOUTFILE): $(INFOOBJS) opensea-libs mkoutputdir 
	$(CC) $(INFOOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(INFOOUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(INFOOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(INFOOUTFILE)
	#$(CC) $(INFOOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(INFOOUTFILE)_static --static
	
$(ZBDOUTFILE): $(ZBDOBJS) opensea-libs mkoutputdir 
	$(CC) $(ZBDOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(ZBDOUTFILE)
	#$(CC) $(ZBDOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE)_static --static
	
$(FORMATOUTFILE): $(FORMATOBJS) opensea-libs mkoutputdir 
	$(CC) $(FORMATOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(FORMATOUTFILE)
	#$(CC) $(FORMATOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE)_static --static


$(NVMEOUTFILE): $(NVMEOBJS) opensea-libs mkoutputdir
	$(CC) $(NVMEOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(NVMEOUTFILE)
	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(NVMEOUTFILE)

$(ERASEOUTFILE): $(ERASEOBJS) opensea-libs mkoutputdir 
    ifneq (,$(findstring DISABLE_TCG_SUPPORT,$(PROJECT_DEFINES)))
		$(CC) $(ERASEOBJS) $(PROJECT_DEFINES) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
	    $(STRIP) -s $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(ERASEOUTFILE)
		#$(CC) $(ERASEOBJS) $(PROJECT_DEFINES) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)_static --static
    else
		$(CC) $(ERASEOBJS) $(TCGLFLAGS) -o $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
	    $(STRIP) -s $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE) -o $(FILE_OUTPUT_DIR)/stripped___$(ERASEOUTFILE)
		#$(CC) $(ERASEOBJS) $(TCGLFLAGS) -o $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)_static --static
    endif

%.o: %.c
	$(CC) $(CFLAGS) $(INC_DIR) $(PROJECT_DEFINES) $< -o $@

clean:
	rm -f *.o *.a $(FILE_OUTPUT_DIR)/$(OUTFILE) $(FILE_OUTPUT_DIR)/$(OUTFILE)_static $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)* $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE)* $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE)* $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE)* $(FILE_OUTPUT_DIR)/stripped* $(FILE_OUTPUT_DIR)/$(FIRMWAREOUTFILE)* $(FILE_OUTPUT_DIR)/$(INFOOUTFILE)* $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE)* $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE)* $(FILE_OUTPUT_DIR)/$(NVMEOUTFILE)*
	rm -f ../../src/*.o
	rm -f $(UTIL_SRC_DIR)/*.o
	rm -rf $(FILE_OUTPUT_DIR)
	$(MAKE) -f Makefile.openSeaChest_firmware clean

clean_all: clean
	rm -f *.o *.a
	$(MAKE) -C ../../opensea-common/Make/vmware clean
	$(MAKE) -C ../../opensea-transport/Make/vmware clean
	$(MAKE) -C ../../opensea-operations/Make/vmware clean
	$(MAKE) -f Makefile.openSeaChest_firmware clean_all

mkoutputdir:
	mkdir -p $(FILE_OUTPUT_DIR)

