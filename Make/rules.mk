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
# \file rules.mk
# \brief Pattern rules with automatic dependency tracking (.d files)

#===============================================================================
# Automatic Dependency Tracking (.d files)
#===============================================================================

# Add flags to generate .d dependency files alongside .o files
# -MMD: generate dependencies, excluding system headers
# -MP: add phony targets for each header to prevent errors if headers are removed
CFLAGS += -MMD -MP
CXXFLAGS += -MMD -MP

#===============================================================================
# VPATH Setup (Source File Discovery)
#===============================================================================

# VPATH tells Make where to find source files
# This allows us to build out-of-tree (objects in build/, sources in src/)

# Common source directories
vpath %.c $(COMMON_DIR)/src
vpath %.c $(TRANSPORT_DIR)/src
vpath %.c $(OPERATIONS_DIR)/src
vpath %.c $(JSONFORMAT_DIR)/src
vpath %.c $(UTILS_DIR)
vpath %.c $(PROJECT_SRC_DIR)

# Platform-specific source directories (will be added by platform modules)
# vpath %.c $(TRANSPORT_DIR)/src/linux
# vpath %.c $(TRANSPORT_DIR)/src/windows
# etc.

#===============================================================================
# Compilation Rules
#===============================================================================

# Quiet compilation output (unless V=1)
ifeq ($(VERBOSE),1)
    Q :=
    COMPILE_MSG =
    LINK_MSG =
    AR_MSG =
else
    Q := @
    COMPILE_MSG = @echo "  CC      $<"
    LINK_MSG = @echo "  LD      $@"
    AR_MSG = @echo "  AR      $@"
endif

# Pattern rule: Compile .c to .o in object directory
# Automatically generates .d dependency file alongside .o file
# $< = source file, $@ = target file (object)
$(OBJ_DIR_COMMON)/%.o: %.c | $(OBJ_DIR_COMMON)
	$(COMPILE_MSG)
	$(Q)$(CC) $(CFLAGS) $(COMMON_CFLAGS) -c $< -o $@
	$(Q)if [ "$(CROSS_COMPILING)" = "1" ] && [ -f $(@:.o=.d) ]; then sed -i 's|C:/|/mnt/c/|g; s|D:/|/mnt/d/|g; s|E:/|/mnt/e/|g; s|\\\\|/|g' $(@:.o=.d); fi

$(OBJ_DIR_TRANSPORT)/%.o: %.c | $(OBJ_DIR_TRANSPORT)
	$(COMPILE_MSG)
	$(Q)$(CC) $(CFLAGS) $(TRANSPORT_CFLAGS) -c $< -o $@
	$(Q)if [ "$(CROSS_COMPILING)" = "1" ] && [ -f $(@:.o=.d) ]; then sed -i 's|C:/|/mnt/c/|g; s|D:/|/mnt/d/|g; s|E:/|/mnt/e/|g; s|\\\\|/|g' $(@:.o=.d); fi

$(OBJ_DIR_OPERATIONS)/%.o: %.c | $(OBJ_DIR_OPERATIONS)
	$(COMPILE_MSG)
	$(Q)$(CC) $(CFLAGS) $(OPERATIONS_CFLAGS) -c $< -o $@
	$(Q)if [ "$(CROSS_COMPILING)" = "1" ] && [ -f $(@:.o=.d) ]; then sed -i 's|C:/|/mnt/c/|g; s|D:/|/mnt/d/|g; s|E:/|/mnt/e/|g; s|\\\\|/|g' $(@:.o=.d); fi

$(OBJ_DIR_JSONFORMAT)/%.o: %.c | $(OBJ_DIR_JSONFORMAT)
	$(COMPILE_MSG)
	$(Q)$(CC) $(CFLAGS) $(JSONFORMAT_CFLAGS) -c $< -o $@
	$(Q)if [ "$(CROSS_COMPILING)" = "1" ] && [ -f $(@:.o=.d) ]; then sed -i 's|C:/|/mnt/c/|g; s|D:/|/mnt/d/|g; s|E:/|/mnt/e/|g; s|\\\\|/|g' $(@:.o=.d); fi

$(OBJ_DIR_TOOLS)/%.o: %.c | $(OBJ_DIR_TOOLS)
	$(COMPILE_MSG)
	$(Q)$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@
	$(Q)if [ "$(CROSS_COMPILING)" = "1" ] && [ -f $(@:.o=.d) ]; then sed -i 's|C:/|/mnt/c/|g; s|D:/|/mnt/d/|g; s|E:/|/mnt/e/|g; s|\\\\|/|g' $(@:.o=.d); fi

#===============================================================================
# Linking Rules
#===============================================================================

# Static library creation (.a)
# $^ = all prerequisites (object files)
define create_static_lib
	$(AR_MSG)
	$(Q)$(AR) rcs $@ $^
	$(Q)$(RANLIB) $@
endef

# Shared library creation (.so)
define create_shared_lib
	$(LINK_MSG)
	$(Q)$(CC) -shared $(LDFLAGS) -o $@ $^ $(LIBS)
endef

# Executable linking
define link_executable
	$(LINK_MSG)
	$(Q)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
endef

#===============================================================================
# Clean Targets
#===============================================================================

# Note: clean, clean-all, show-config, and help targets are defined in GNUmakefile
# to avoid "overriding recipe" warnings. Only define pattern rules here.

.PHONY: clean-%

# Clean specific library or tool (will be defined by library.mk and tools.mk)
clean-%:
	@echo "Cleaning $*..."
	$(Q)rm -rf $(OBJ_DIR_$*)
	$(Q)rm -f $(LIB_DIR)/libopensea-$*.a
	$(Q)rm -f $(LIB_DIR)/libopensea-$*.so*
	$(Q)rm -f $(BIN_DIR)/openSeaChest_$*

#===============================================================================
# Phony Targets
#===============================================================================

# Note: Main targets (all, clean, clean-all, install, uninstall, package,
# show-config, help) are defined in GNUmakefile
