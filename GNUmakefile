# SPDX-License-Identifier: MPL-2.0
#
# Do NOT modify or remove this copyright and license
#
# Copyright (c) 2012-2025 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
#
# This software is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ******************************************************************************************
#
# \file GNUmakefile (root convenience wrapper)
# \brief Forwards all targets to Make/GNUmakefile
#
# USAGE:
#   make [target] [VARIABLES]
#
# This wrapper allows running `make` from the project root without needing to
# specify the Make/ directory. All targets and variables are forwarded to the
# actual build system in Make/GNUmakefile.

# Forward all targets to Make/GNUmakefile
%:
	@$(MAKE) -C Make $(MAKECMDGOALS)

# Special handling for no target (default)
.DEFAULT_GOAL := all

all:
	@$(MAKE) -C Make all

# Ensure this Makefile doesn't interfere with target detection
.PHONY: all
