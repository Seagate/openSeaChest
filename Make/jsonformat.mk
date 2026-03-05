# SPDX-License-Identifier: MPL-2.0
#
# Do NOT modify or remove this copyright and license
#
# Copyright (c) 2026-2026 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
#
# This software is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ******************************************************************************************
#
# \file jsonformat.mk
# \brief JSON-C and opensea-jsonformat library build (optional, BUILD_JSON=1)

#===============================================================================
# JSON-C Library Build (External Dependency)
#===============================================================================

# JSON-C build directory
JSONC_BUILD_DIR := $(JSONC_DIR)/build

# JSON-C library output
LIBJSONC := $(JSONC_BUILD_DIR)/libjson-c.a

# Build JSON-C using CMake via buildjsonc.sh script
$(LIBJSONC):
	@echo "Building JSON-C library via CMake..."
	@cd $(JSONFORMAT_DIR)/Make && ./buildjsonc.sh build $(CC)
	@echo "JSON-C library built: $(LIBJSONC)"

.PHONY: json-c
json-c: $(LIBJSONC)

#===============================================================================
# opensea-jsonformat Sources
#===============================================================================

JSONFORMAT_SOURCES := \
    common_format.c \
    device_format.c \
    farm_format.c \
    log_format.c \
    odata_format.c

#===============================================================================
# opensea-jsonformat Include Directories
#===============================================================================

JSONFORMAT_INCLUDES := \
    -I$(JSONFORMAT_DIR)/include \
    -I$(OPERATIONS_DIR)/include \
    -I$(TRANSPORT_DIR)/include \
    -I$(COMMON_DIR)/include \
    -I$(JSONC_DIR) \
    -I$(JSONC_BUILD_DIR)

JSONFORMAT_CFLAGS := $(CFLAGS)
JSONFORMAT_CFLAGS += $(JSONFORMAT_INCLUDES)

#===============================================================================
# opensea-jsonformat Object Files
#===============================================================================

JSONFORMAT_OBJS := $(addprefix $(OBJ_DIR_JSONFORMAT)/,$(JSONFORMAT_SOURCES:.c=.o))
JSONFORMAT_DEPS := $(JSONFORMAT_OBJS:.o=.d)

#===============================================================================
# opensea-jsonformat Library Targets
#===============================================================================

# Static library
LIBJSONFORMAT := $(LIB_DIR)/libopensea-jsonformat.a

# Shared library (optional)
ifeq ($(BUILD_SHARED_LIBS),1)
    LIBJSONFORMAT_SO := $(LIB_DIR)/libopensea-jsonformat.so.$(JSONFORMAT_VERSION)
    LIBJSONFORMAT_SO_LINK := $(LIB_DIR)/libopensea-jsonformat.so
endif

#===============================================================================
# Build Rules
#===============================================================================

# Static library (depends on JSON-C and opensea-operations)
$(LIBJSONFORMAT): $(JSONFORMAT_OBJS) $(LIBJSONC) $(LIBOPERATIONS) | $(LIB_DIR)
	$(AR_MSG)
	$(Q)$(AR) rcs $@ $(JSONFORMAT_OBJS)
	$(Q)$(RANLIB) $@

# Shared library (optional)
ifeq ($(BUILD_SHARED_LIBS),1)
$(LIBJSONFORMAT_SO): $(JSONFORMAT_OBJS) $(LIBJSONC) $(LIBOPERATIONS) | $(LIB_DIR)
	$(LINK_MSG)
	$(Q)$(CC) -shared $(LDFLAGS) -o $@ $(JSONFORMAT_OBJS) \
		-L$(LIB_DIR) -lopensea-operations -lopensea-transport -lopensea-common \
		$(LIBJSONC) -Wl,-soname,libopensea-jsonformat.so.$(JSONFORMAT_MAJOR)
	$(Q)cd $(LIB_DIR) && ln -sf libopensea-jsonformat.so.$(JSONFORMAT_VERSION) libopensea-jsonformat.so.$(JSONFORMAT_MAJOR)
	$(Q)cd $(LIB_DIR) && ln -sf libopensea-jsonformat.so.$(JSONFORMAT_MAJOR) libopensea-jsonformat.so
endif

#===============================================================================
# Phony Targets
#===============================================================================

.PHONY: opensea-jsonformat

# Build jsonformat library (depends on operations, which depends on transport and common)
opensea-jsonformat: opensea-operations $(LIBJSONFORMAT)
ifeq ($(BUILD_SHARED_LIBS),1)
opensea-jsonformat: $(LIBJSONFORMAT_SO)
endif

#===============================================================================
# Automatic Dependency Inclusion (.d files)
#===============================================================================

# When cross-compiling with MinGW, fix Windows paths in existing .d files BEFORE including them
ifeq ($(CROSS_COMPILING),1)
    $(shell find $(OBJ_DIR_JSONFORMAT) -name '*.d' -exec sed -i 's|C:/|/mnt/c/|g; s|D:/|/mnt/d/|g; s|E:/|/mnt/e/|g; s|\\\\|/|g' {} + 2>/dev/null || true)
endif

# .d files are post-processed during compilation to fix Windows paths on cross-compile
-include $(JSONFORMAT_DEPS)

#===============================================================================
# Export Variables
#===============================================================================

export JSONFORMAT_SOURCES
export JSONFORMAT_INCLUDES
export JSONFORMAT_CFLAGS
export JSONFORMAT_OBJS
export JSONFORMAT_DEPS
export LIBJSONFORMAT
export LIBJSONFORMAT_SO
export LIBJSONC
