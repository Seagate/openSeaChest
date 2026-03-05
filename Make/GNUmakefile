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
# \file Make/GNUmakefile
# \brief Main orchestrator for openSeaChest modular build system
#
# USAGE:
#   make [PLATFORM=linux|windows|freebsd|...] [BUILD_TYPE=release|debug|relwithdebinfo|minsize] [BUILD_JSON=1] [V=1]
#
# EXAMPLES:
#   make                           # Build all (auto-detect platform, release build with -O3)
#   make -j$(nproc)                # Parallel build with all CPU cores
#   make BUILD_TYPE=debug          # Debug build (-O0 -g3, no optimization)
#   make BUILD_TYPE=release        # Release build (-O3, default)
#   make BUILD_TYPE=relwithdebinfo # Optimized with debug symbols (-O2 -g)
#   make BUILD_TYPE=minsize        # Minimize binary size (-Os)
#   make BUILD_JSON=1              # Enable JSON support
#   make install PREFIX=/usr       # Install to /usr instead of /usr/local
#   make package                   # Create distribution tarball
#   make show-config               # Show build configuration
#   make clean                     # Clean current build type
#   make clean-all                 # Clean all build types
#
#===============================================================================
# Build System Modules (Order Matters)
#===============================================================================

# Determine Make directory for includes
MAKE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# 1. Core configuration (platform detection, build directories)
include $(MAKE_DIR)config.mk

# 2. Compiler detection (GCC/Clang version checks)
include $(MAKE_DIR)compiler-detection.mk

# 3. Compiler flags (warning flags with cc_supports testing)
include $(MAKE_DIR)compiler-flags.mk

# 4. Security hardening (OpenSSF best practices)
include $(MAKE_DIR)security-hardening.mk

# 5. Platform-specific configuration (detect or override)
include $(MAKE_DIR)platforms/$(PLATFORM).mk

# Set default EXE_EXT if platform didn't define it (Unix platforms = no extension)
EXE_EXT ?=

# 6. Library build rules (opensea-common, opensea-transport, opensea-operations)
include $(MAKE_DIR)library.mk

# 7. JSON support (optional, BUILD_JSON=1)
ifeq ($(BUILD_JSON),1)
    -include $(MAKE_DIR)jsonformat.mk
endif

# 8. Tool build rules (17 openSeaChest utilities)
include $(MAKE_DIR)tools.mk

# 9. Pattern rules and targets (.d dependency tracking, clean, help)
include $(MAKE_DIR)rules.mk

#===============================================================================
# Default Target
#===============================================================================

.PHONY: all
all: libraries tools

#===============================================================================
# Library Targets
#===============================================================================

.PHONY: libraries opensea-common opensea-transport opensea-operations

libraries: opensea-common opensea-transport opensea-operations

ifeq ($(BUILD_JSON),1)
libraries: opensea-jsonformat
endif

#===============================================================================
# Build Type Shortcuts
#===============================================================================

.PHONY: release debug relwithdebinfo minsize

# Prevent parallel execution for these targets to avoid race conditions
# Package and install operations must run serially for filesystem consistency
.NOTPARALLEL: package install install-strip

release:
	$(MAKE) BUILD_TYPE=release

debug:
	$(MAKE) BUILD_TYPE=debug

relwithdebinfo:
	$(MAKE) BUILD_TYPE=relwithdebinfo

minsize:
	$(MAKE) BUILD_TYPE=minsize

# Legacy shortcuts (deprecated)
.PHONY: static-release static-debug

static-release:
	@echo "Warning: static-release is deprecated. Use BUILD_TYPE=release IS_STATIC=1"
	$(MAKE) BUILD_TYPE=release IS_STATIC=1

static-debug:
	@echo "Warning: static-debug is deprecated. Use BUILD_TYPE=debug IS_STATIC=1"
	$(MAKE) BUILD_TYPE=debug IS_STATIC=1

#===============================================================================
# Installation Targets
#===============================================================================

# Default installation prefix
PREFIX ?= /usr/local
DESTDIR ?=

# Installation program (can be overridden for install-strip)
INSTALL_PROGRAM ?= install

# Installation directories
INSTALL_BIN_DIR := $(DESTDIR)$(PREFIX)/bin
INSTALL_LIB_DIR := $(DESTDIR)$(PREFIX)/lib
INSTALL_INC_DIR := $(DESTDIR)$(PREFIX)/include/opensea

.PHONY: install uninstall install-tools install-libs install-headers install-strip installdirs

# Create installation directories (GNU Coding Standards requirement)
installdirs:
	@$(call print_status,"Creating installation directories")
	@mkdir -p $(INSTALL_BIN_DIR)
	@mkdir -p $(INSTALL_LIB_DIR)
	@mkdir -p $(INSTALL_INC_DIR)
	@$(call print_success,"Installation directories created")

install: installdirs install-tools install-libs install-headers
	@$(call print_success,"Installation complete to $(PREFIX)")

install-strip: installdirs
	@$(call print_status,"Installing stripped binaries to $(PREFIX)")
	$(MAKE) INSTALL_PROGRAM='install -s' install-tools
	$(MAKE) install-libs install-headers
	@$(call print_success,"Installation complete (stripped)")

install-tools: tools | $(INSTALL_BIN_DIR)
	@$(call print_status,"Installing tools to $(INSTALL_BIN_DIR)")
	@$(INSTALL_PROGRAM) -m 0755 $(TOOL_EXES) $(INSTALL_BIN_DIR)/
	@$(call print_success,"Installed $(words $(TOOL_EXES)) tools")

install-libs: libraries | $(INSTALL_LIB_DIR)
	@$(call print_status,"Installing libraries to $(INSTALL_LIB_DIR)")
	@install -m 0644 $(LIB_DIR)/lib*.a $(INSTALL_LIB_DIR)/ 2>/dev/null || true
ifeq ($(BUILD_SHARED_LIBS),1)
	@install -m 0755 $(LIB_DIR)/lib*.so* $(INSTALL_LIB_DIR)/ 2>/dev/null || true
endif
	@$(call print_success,"Installed libraries")

install-headers: | $(INSTALL_INC_DIR)
	@$(call print_status,"Installing headers to $(INSTALL_INC_DIR)")
	@install -m 0644 $(COMMON_DIR)/include/*.h $(INSTALL_INC_DIR)/
	@install -m 0644 $(TRANSPORT_DIR)/include/*.h $(INSTALL_INC_DIR)/
	@install -m 0644 $(OPERATIONS_DIR)/include/*.h $(INSTALL_INC_DIR)/
	@$(call print_success,"Installed headers")

# Create installation directories
$(INSTALL_BIN_DIR) $(INSTALL_LIB_DIR) $(INSTALL_INC_DIR):
	@mkdir -p $@

uninstall:
	@$(call print_status,"Uninstalling from $(PREFIX)")
	@rm -f $(addprefix $(INSTALL_BIN_DIR)/,$(notdir $(TOOL_EXES)))
	@rm -f $(INSTALL_LIB_DIR)/libopensea-*.a $(INSTALL_LIB_DIR)/libopensea-*.so*
	@rm -rf $(INSTALL_INC_DIR)
	@$(call print_success,"Uninstall complete")

#===============================================================================
# Package Target (Create Distribution Tarball)
#===============================================================================

# Extract version from git or use default
VERSION ?= $(shell git describe --tags --always --dirty 2>/dev/null || echo "unknown")
PACKAGE_NAME := openSeaChest-$(VERSION)-$(PLATFORM)-$(ARCH)
PACKAGE_FILE := $(PACKAGE_NAME).tar.gz
PACKAGE_DIR := $(PROJECT_ROOT)/build/package/$(PACKAGE_NAME)

.PHONY: package

package: all
	@echo "Creating package $(PACKAGE_FILE)..."
	@rm -rf $(PROJECT_ROOT)/build/package
	@mkdir -p $(PACKAGE_DIR)/bin
	@mkdir -p $(PACKAGE_DIR)/lib
	@mkdir -p $(PACKAGE_DIR)/include
	@cp $(TOOL_EXES) $(PACKAGE_DIR)/bin/
	@cp $(LIB_DIR)/lib*.a $(PACKAGE_DIR)/lib/ 2>/dev/null || true
ifeq ($(BUILD_SHARED_LIBS),1)
	@cp $(LIB_DIR)/lib*.so* $(PACKAGE_DIR)/lib/ 2>/dev/null || true
endif
	@cp -r $(COMMON_DIR)/include/*.h $(PACKAGE_DIR)/include/
	@cp -r $(TRANSPORT_DIR)/include/*.h $(PACKAGE_DIR)/include/
	@cp -r $(OPERATIONS_DIR)/include/*.h $(PACKAGE_DIR)/include/
	@cp README.md LICENSE.md $(PACKAGE_DIR)/ 2>/dev/null || true
	@cd $(PROJECT_ROOT)/build/package && tar czf $(PACKAGE_FILE) $(PACKAGE_NAME)
	@mv $(PROJECT_ROOT)/build/package/$(PACKAGE_FILE) $(PROJECT_ROOT)/
	@echo "Package created: $(PACKAGE_FILE)"

#===============================================================================
# Utility Targets
#===============================================================================

.PHONY: show-config print-tools print-libs

show-config:
	@echo "$(COLOR_HEADER)openSeaChest Build Configuration$(COLOR_RESET)"
	@echo "$(COLOR_STATUS)=================================$(COLOR_RESET)"
	@echo "Platform:       $(PLATFORM)"
	@echo "Architecture:   $(ARCH)"
	@echo "Build Type:     $(BUILD_TYPE)"
	@echo "Optimization:   $(OPTIMIZATION_FLAGS)"
	@echo "Defines:        $(BUILD_DEFINES)"
	@echo "Compiler:       $(COMPILER_NAME)"
	@echo "Build Dir:      $(BUILD_DIR)"
	@echo "Output Dirs:    bin=$(BIN_DIR) lib=$(LIB_DIR) obj=$(OBJ_DIR)"
	@echo "Build JSON:     $(if $(filter 1,$(BUILD_JSON)),Yes,No)"
	@echo "Shared Libs:    $(if $(filter 1,$(BUILD_SHARED_LIBS)),Yes,No)"
	@echo "Static Build:   $(if $(filter 1,$(IS_STATIC)),Yes,No)"
	@echo "Parallel Jobs:  $(or $(patsubst -j%,%,$(filter -j%,$(MAKEFLAGS))),1)"
	@echo ""
	@echo "$(COLOR_STATUS)Compiler Flags:$(COLOR_RESET)"
	@echo "CFLAGS:         $(CFLAGS)"
	@echo "LDFLAGS:        $(LDFLAGS)"
	@echo ""

print-tools:
	@echo "$(COLOR_HEADER)Tools to Build ($(words $(TOOLS))):$(COLOR_RESET)"
	@for tool in $(TOOLS); do echo "  - $$tool"; done

print-libs:
	@echo "$(COLOR_HEADER)Libraries to Build:$(COLOR_RESET)"
	@echo "  - libopensea-common.a"
	@echo "  - libopensea-transport.a"
	@echo "  - libopensea-operations.a"
ifeq ($(BUILD_JSON),1)
	@echo "  - libopensea-jsonformat.a"
	@echo "  - libjson-c.a (external)"
endif

# Debug any variable: make print-CFLAGS, make print-BUILD_TYPE, etc.
# Shows value, origin (file/environment/command-line), and flavor (simple/recursive)
.PHONY: print-%
print-%:
	@echo '$* = $($*)'
	@echo 'Origin: $(origin $*)'
	@echo 'Flavor: $(flavor $*)'

#===============================================================================
# Clean Targets
#===============================================================================

.PHONY: clean clean-all mostlyclean distclean

mostlyclean:
	@echo "Cleaning object files and tools (keeping libraries)..."
	@rm -f $(OBJ_DIR_COMMON)/*.o $(OBJ_DIR_COMMON)/*.d 2>/dev/null || true
	@rm -f $(OBJ_DIR_TRANSPORT)/*.o $(OBJ_DIR_TRANSPORT)/*.d 2>/dev/null || true
	@rm -f $(OBJ_DIR_OPERATIONS)/*.o $(OBJ_DIR_OPERATIONS)/*.d 2>/dev/null || true
	@rm -f $(OBJ_DIR_JSONFORMAT)/*.o $(OBJ_DIR_JSONFORMAT)/*.d 2>/dev/null || true
	@rm -f $(OBJ_DIR_TOOLS)/*.o $(OBJ_DIR_TOOLS)/*.d 2>/dev/null || true
	@rm -f $(BIN_DIR)/* 2>/dev/null || true
	@echo "Mostlyclean complete (libraries preserved)"

clean:
	@echo "Cleaning $(BUILD_TYPE) build..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete"

clean-all:
	@echo "Cleaning all build types..."
	@rm -rf build/
	@echo "Clean all complete"

distclean: clean-all
	@$(call print_status,"Removing all generated files")
	@rm -rf $(PROJECT_ROOT)/*.tar.gz
	@find $(PROJECT_ROOT) -name '*.o' -delete 2>/dev/null || true
	@find $(PROJECT_ROOT) -name '*.d' -delete 2>/dev/null || true
	@$(call print_success,"Distclean complete")

#===============================================================================
# Test/Check Target
#===============================================================================

.PHONY: check test

# Smoke tests: verify tools run and respond to --version
check test: tools
	@echo "Running basic smoke tests..."
	@failed=0; \
	for tool in $(TOOL_EXES); do \
		echo -n "  Testing $$(basename $$tool)... "; \
		if $$tool --version >/dev/null 2>&1; then \
			echo "OK"; \
		else \
			echo "FAILED"; \
			failed=$$((failed + 1)); \
		fi; \
	done; \
	if [ $$failed -eq 0 ]; then \
		echo "All $(words $(TOOL_EXES)) tools passed smoke tests"; \
	else \
		echo "$$failed tool(s) failed smoke tests"; \
		exit 1; \
	fi

#===============================================================================
# Help Target
#===============================================================================

.PHONY: help

help:
	@echo "$(COLOR_HEADER)openSeaChest Build System$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_STATUS)TARGETS:$(COLOR_RESET)"
	@echo "  all               Build all libraries and tools (default)"
	@echo "  libraries         Build only libraries (opensea-common, transport, operations)"
	@echo "  tools             Build only openSeaChest utilities (requires libraries)"
	@echo "  check / test      Run basic smoke tests on built tools"
	@echo "  install           Install binaries, libraries, and headers"
	@echo "  install-strip     Install binaries with debug symbols stripped"
	@echo "  installdirs       Create installation directories only"
	@echo "  uninstall         Remove installed files"
	@echo "  package           Create distribution tarball"
	@echo "  mostlyclean       Clean objects and tools (keep libraries)"
	@echo "  clean             Clean current build type"
	@echo "  clean-all         Clean all build types"
	@echo "  distclean         Remove all generated files including packages"
	@echo "  show-config       Display build configuration"
	@echo "  print-tools       List all tools to build"
	@echo "  print-libs        List all libraries to build"
	@echo "  help              Show this help message"
	@echo ""
	@echo "$(COLOR_STATUS)BUILD TYPES:$(COLOR_RESET)"
	@echo "  release           Optimized build (-O3, default)"
	@echo "  debug             Debug build (-O0 -g3, no optimization)"
	@echo "  relwithdebinfo    Optimized with debug symbols (-O2 -g)"
	@echo "  minsize           Minimize binary size (-Os)"
	@echo ""
	@echo "$(COLOR_STATUS)VARIABLES:$(COLOR_RESET)"
	@echo "  PLATFORM=<name>      Override platform detection (linux, windows, freebsd, etc.)"
	@echo "  BUILD_TYPE=<type>    Set build type (release, debug, relwithdebinfo, minsize)"
	@echo "  BUILD_JSON=1         Enable JSON support (requires CMake and json-c)"
	@echo "  BUILD_SHARED_LIBS=1  Build shared libraries in addition to static"
	@echo "  IS_STATIC=1          Build static binaries (link runtime statically)"
	@echo "  PREFIX=/path         Installation prefix (default: /usr/local)"
	@echo "  DESTDIR=/path        Staging directory for installation"
	@echo "  V=1                  Verbose build output"
	@echo "  CC=<compiler>        Override compiler (gcc, clang)"
	@echo ""
	@echo "$(COLOR_STATUS)EXAMPLES:$(COLOR_RESET)"
	@echo "  make                                # Build all with auto-detected platform (optimized)"
	@echo "  make -j\$$(nproc)                     # Parallel build using all CPU cores"
	@echo "  make BUILD_TYPE=debug               # Debug build (no optimization, full debug info)"
	@echo "  make BUILD_TYPE=relwithdebinfo      # Optimized build with debug symbols (for profiling)"
	@echo "  make BUILD_TYPE=minsize             # Smallest binary size (embedded systems)"
	@echo "  make BUILD_JSON=1                   # Build with JSON support"
	@echo "  make PLATFORM=windows CC=gcc        # Cross-compile for Windows"
	@echo "  make install PREFIX=/usr            # Install to /usr instead of /usr/local"
	@echo "  make package                        # Create openSeaChest-VERSION-PLATFORM.tar.gz"
	@echo "  make show-config                    # Show current configuration"
	@echo ""
	@echo "$(COLOR_STATUS)SUPPORTED PLATFORMS:$(COLOR_RESET)"
	@echo "  linux, windows, freebsd, openbsd, netbsd, dragonflybsd,"
	@echo "  sunos, illumos, aix, vmware"
	@echo ""

#===============================================================================
# Phony Targets Declaration
#===============================================================================

.PHONY: all libraries tools \
        release debug static-release static-debug \
        install uninstall install-tools install-libs install-headers \
        package \
        show-config print-tools print-libs \
        clean clean-all distclean \
        help
