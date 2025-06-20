// SPDX-License-Identifier: MPL-2.0
//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2025 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
//
// \file OpenSeaChest_Configure.c command line that performs various Configuration methods on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "io_utils.h"
#include "memory_safety.h"
#include "secure_file.h"
#include "string_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"

#include "EULA.h"
#include "ata_device_config_overlay.h"
#include "device_statistics.h"
#include "drive_info.h"
#include "getopt.h"
#include "logs.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "power_control.h" //PUIS. transitions users to using openSeaChest_PowerControl for this feature.
#include "power_control.h"
#include "set_max_lba.h"
#include "smart.h"
#include "trim_unmap.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name    = "openSeaChest_Configure";
const char* buildVersion = "2.9.2";

////////////////////////////
//  functions to declare  //
////////////////////////////
static void utility_Usage(bool shortUsage);
//-----------------------------------------------------------------------------
//
//  main()
//
//! \brief   Description:  main function that runs and decides what to do based on the passed in args
//
//  Entry:
//!   \param argc =
//!   \param argv =
//!
//  Exit:
//!   \return exitCode = error code returned by the application
//
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    /////////////////
    //  Variables  //
    /////////////////
    // common utility variables
    // clang-format off
    eReturnValues ret      = SUCCESS;
    int           exitCode = UTIL_EXIT_NO_ERROR;
    DEVICE_UTIL_VARS
    DEVICE_INFO_VAR
    CAPACITY_MODEL_NUMBER_MAPPING_VAR
    CHANGE_ID_STRING_VAR
    SAT_INFO_VAR
    DATA_ERASE_VAR
    POSSIBLE_DATA_ERASE_VAR
    LICENSE_VAR
    ECHO_COMMAND_LINE_VAR
    SCAN_FLAG_VAR
    NO_BANNER_VAR
    AGRESSIVE_SCAN_FLAG_VAR
    SHOW_BANNER_VAR
    SHOW_HELP_VAR
    TEST_UNIT_READY_VAR
    FAST_DISCOVERY_VAR
    MODEL_MATCH_VARS
    FW_MATCH_VARS
    CHILD_MODEL_MATCH_VARS
    CHILD_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    FORCE_DRIVE_TYPE_VARS
    ENABLE_LEGACY_PASSTHROUGH_VAR
    // scan output flags
    SCAN_FLAGS_UTIL_VARS
    // tool specific
    SET_MAX_LBA_VARS
    RESTORE_MAX_LBA_VAR
    SET_PHY_SPEED_VARS
    SET_PHY_SAS_PHY_IDENTIFIER_VAR
    SET_READY_LED_VARS
    READ_LOOK_AHEAD_VARS
    NV_CACHE_VARS
    WRITE_CACHE_VARS
    PROVISION_VAR
    TRIM_UNMAP_VARS // need these with provision var
    LOW_CURRENT_SPINUP_VARS
    SCT_WRITE_CACHE_VARS
    SCT_WRITE_CACHE_REORDER_VARS
    VOLATILE_VAR
    PUIS_FEATURE_VARS
    SSC_FEATURE_VARS
#if defined(ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    SCT_ERROR_RECOVERY_CONTROL_VARS
    FREE_FALL_VARS
    WRV_VARS

    SCSI_MP_RESET_VARS
    SCSI_MP_RESTORE_VARS
    SCSI_MP_SAVE_VARS
    SCSI_SHOW_MP_VARS
    SCSI_RESET_LP_VARS
    SCSI_SET_MP_VARS
    LOWLEVEL_INFO_VAR

    ATA_DCO_RESTORE_VAR
    ATA_DCO_FREEZE_VAR
    ATA_DCO_IDENTIFY_VAR
    ATA_DCO_SETMAXLBA_VARS
    ATA_DCO_SETMAXMODE_VARS
    ATA_DCO_DISABLE_FEATURES_VARS

    SET_TIMESTAMP_VAR

    int args        = 0;
    int argIndex    = 0;
    int optionIndex = 0;

    struct option longopts[] =
    {
        // common command line options
        DEVICE_LONG_OPT,
        HELP_LONG_OPT,
        DEVICE_INFO_LONG_OPT,
        CAPACITY_MODEL_NUMBER_MAPPING_LONG_OPT,
        CHANGE_ID_STRING_LONG_OPT,
        SAT_INFO_LONG_OPT,
        
        SCAN_LONG_OPT,
        AGRESSIVE_SCAN_LONG_OPT,
        SCAN_FLAGS_LONG_OPT,
        NO_BANNER_OPT,
        VERSION_LONG_OPT,
        VERBOSE_LONG_OPT,
        QUIET_LONG_OPT,
        LICENSE_LONG_OPT,
        ECHO_COMMAND_LIN_LONG_OPT,
        TEST_UNIT_READY_LONG_OPT,
        FAST_DISCOVERY_LONG_OPT,
        ONLY_SEAGATE_LONG_OPT,
        MODEL_MATCH_LONG_OPT,
        FW_MATCH_LONG_OPT,
        CHILD_MODEL_MATCH_LONG_OPT,
        CHILD_FW_MATCH_LONG_OPT,
        CONFIRM_LONG_OPT,
        VOLATILE_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined(ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        LOWLEVEL_INFO_LONG_OPT,
        // tool specific options go here
        SET_MAX_LBA_LONG_OPT,
        PROVISION_LONG_OPT,
        RESTORE_MAX_LBA_LONG_OPT,
        SET_PHY_SPEED_LONG_OPT,
        SET_PHY_SAS_PHY_LONG_OPT,
        SET_READY_LED_LONG_OPTS,
        READ_LOOK_AHEAD_LONG_OPT,
        NV_CACHE_LONG_OPT,
        WRITE_CACHE_LONG_OPT,
        LOW_CURRENT_SPINUP_LONG_OPT,
        SCT_WRITE_CACHE_LONG_OPT,
        SCT_WRITE_CACHE_REORDER_LONG_OPT,
        PUIS_FEATURE_LONG_OPT,
        SSC_FEATURE_LONG_OPT,
        SCT_ERROR_RECOVERY_CONTROL_LONG_OPTS,
        FREE_FALL_LONG_OPT,
        SCSI_MP_RESET_LONG_OPT,
        SCSI_MP_RESTORE_LONG_OPT,
        SCSI_MP_SAVE_LONG_OPT,
        SCSI_SHOW_MP_LONG_OPTS,
        SCSI_RESET_LP_LONG_OPTS,
        SCSI_SET_MP_LONG_OPT,
        ATA_DCO_RESTORE_LONG_OPT,
        ATA_DCO_FREEZE_LONG_OPT,
        ATA_DCO_IDENTIFY_LONG_OPT,
        ATA_DCO_SETMAXLBA_LONG_OPT,
        ATA_DCO_SETMAXMODE_LONG_OPT,
        ATA_DCO_DISABLE_FEEATURES_LONG_OPT,
        WRV_LONG_OPT,
        SET_TIMESTAMP_LONG_OPT,
        LONG_OPT_TERMINATOR
    };
    // clang-format on

    eVerbosityLevels toolVerbosity = VERBOSITY_DEFAULT;

#if defined(UEFI_C_SOURCE)
    // NOTE: This is a BSD function used to ensure the program name is set correctly for warning or error functions.
    //       This is not necessary on most modern systems other than UEFI.
    //       This is not used in linux so that we don't depend on libbsd
    //       Update the above #define check if we port to another OS that needs this to be done.
    if (getprogname() == M_NULLPTR)
    {
        setprogname(util_name);
    }
#endif

    ////////////////////////
    //  Argument Parsing  //
    ////////////////////////
    if (argc < 2)
    {
        openseachest_utility_Info(util_name, buildVersion);
        utility_Usage(true);
        printf("\n");
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    // get options we know we need
    while (1) // changed to while 1 in order to parse multiple options when longs options are involved
    {
        args = getopt_long(argc, argv, "d:hisSF:Vv:q%:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        // printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:
            // parse long options that have no short option and required arguments here
            if (strcmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, DATA_ERASE_ACCEPT_STRING) == 0)
                {
                    DATA_ERASE_FLAG = true;
                }
                else if (strcmp(optarg, POSSIBLE_DATA_ERASE_ACCEPT_STRING) == 0)
                {
                    POSSIBLE_DATA_ERASE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SET_MAX_LBA_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &SET_MAX_LBA_VALUE))
                {
                    SET_MAX_LBA_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_MAX_LBA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SET_PHY_SPEED_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &SET_PHY_SPEED_GEN) &&
                    SET_PHY_SPEED_GEN < SET_PHY_SPEED_MAX_GENERATION)
                {
                    SET_PHY_SPEED_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_PHY_SPEED_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SET_PHY_SAS_PHY_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                         &SET_PHY_SAS_PHY_IDENTIFIER))
                {
                    SET_PHY_ALL_PHYS = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_PHY_SAS_PHY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if ((strcmp(longopts[optionIndex].name, SET_READY_LED_LONG_OPT_STRING) == 0))
            {
                if (strcmp(optarg, "default") == 0)
                {
                    SET_READY_LED_FLAG    = true;
                    SET_READY_LED_DEFAULT = true;
                }
                else if (strcmp(optarg, "on") == 0)
                {
                    SET_READY_LED_FLAG = true;
                    SET_READY_LED_MODE = true;
                }
                else if (strcmp(optarg, "off") == 0)
                {
                    SET_READY_LED_FLAG = true;
                    SET_READY_LED_MODE = false;
                }
                else if (strcmp(optarg, "info") == 0)
                {
                    READY_LED_INFO_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_READY_LED_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NV_CACHE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    NV_CACHE_INFO = true;
                }
                else
                {
                    NV_CACHE_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        NV_CACHE_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        NV_CACHE_SETTING = false;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(NV_CACHE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, READ_LOOK_AHEAD_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    READ_LOOK_AHEAD_INFO = true;
                }
                else
                {
                    READ_LOOK_AHEAD_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        READ_LOOK_AHEAD_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        READ_LOOK_AHEAD_SETTING = false;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(READ_LOOK_AHEAD_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, WRITE_CACHE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    WRITE_CACHE_INFO = true;
                }
                else
                {
                    WRITE_CACHE_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        WRITE_CACHE_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        WRITE_CACHE_SETTING = false;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(WRITE_CACHE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, PROVISION_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &SET_MAX_LBA_VALUE))
                {
                    SET_MAX_LBA_FLAG = true;
                    // now, based on the new MaxLBA, set the TRIM/UNMAP start flag to get rid of the LBAs that will not
                    // be above the new maxLBA (the range will be set later)
                    TRIM_UNMAP_START_FLAG = SET_MAX_LBA_VALUE + 1;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PROVISION_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, LOW_CURRENT_SPINUP_LONG_OPT_STRING) == 0)
            {
                LOW_CURRENT_SPINUP_FLAG = true;
                if (strcmp(optarg, "low") == 0)
                {
                    LOW_CURRENT_SPINUP_STATE = 1;
                }
                else if (strcmp(optarg, "ultra") == 0)
                {
                    LOW_CURRENT_SPINUP_STATE = 3;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    LOW_CURRENT_SPINUP_STATE = 2;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(LOW_CURRENT_SPINUP_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCT_WRITE_CACHE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SCT_WRITE_CACHE_INFO = true;
                }
                else
                {
                    SCT_WRITE_CACHE_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        SCT_WRITE_CACHE_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        SCT_WRITE_CACHE_SETTING = false;
                    }
                    else if (strcmp(optarg, "default") == 0)
                    {
                        SCT_WRITE_CACHE_SET_DEFAULT = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCT_WRITE_CACHE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCT_WRITE_CACHE_REORDER_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SCT_WRITE_CACHE_REORDER_INFO = true;
                }
                else
                {
                    SCT_WRITE_CACHE_REORDER_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        SCT_WRITE_CACHE_REORDER_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        SCT_WRITE_CACHE_REORDER_SETTING = false;
                    }
                    else if (strcmp(optarg, "default") == 0)
                    {
                        SCT_WRITE_CACHE_REORDER_SET_DEFAULT = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCT_WRITE_CACHE_REORDER_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, PUIS_FEATURE_LONG_OPT_STRING) == 0)
            {
                PUIS_FEATURE_FLAG = true;
                if (strcmp(optarg, "info") == 0)
                {
                    PUIS_FEATURE_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    PUIS_FEATURE_STATE_FLAG = true;
                    PUIS_STATE_VALID_FLAG   = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    PUIS_FEATURE_STATE_FLAG = false;
                    PUIS_STATE_VALID_FLAG   = true;
                }
                else if (strcmp(optarg, "spinup") == 0)
                {
                    PUIS_FEATURE_SPINUP_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PUIS_FEATURE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SSC_FEATURE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    GET_SSC_FLAG = true;
                }
                else
                {
                    SET_SSC_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        SSC_MODE = SSC_ENABLED;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        SSC_MODE = SSC_DISABLED;
                    }
                    else if (strcmp(optarg, "default") == 0)
                    {
                        SSC_MODE = SSC_DEFAULT;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SSC_FEATURE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SCT_ERROR_RECOVERY_CONTROL_READ_INFO = true;
                }
                else if (strcmp(optarg, "default") == 0)
                {
                    SCT_ERROR_RECOVERY_CONTROL_READ_SET_DEFAULT = true;
                }
                else
                {
                    uint32_t multiplier = UINT32_C(100); // 100 millisecond conversion
                    char*    unit       = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME,
                                                              &SCT_ERROR_RECOVERY_CONTROL_READ_TIMER_VALUE))
                    {
                        // first check is a unit is provided.
                        if (unit)
                        {
                            if (strcmp(unit, "ms") == 0 || strcmp(unit, "") == 0)
                            {
                                multiplier = UINT32_C(1);
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                multiplier = UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                multiplier = UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                multiplier = UINT32_C(3600000);
                            }
                            else
                            {
                                print_Error_In_Cmd_Line_Args(SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        SCT_ERROR_RECOVERY_CONTROL_SET_READ_TIMER = true;
                        SCT_ERROR_RECOVERY_CONTROL_READ_TIMER_VALUE *= multiplier;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO = true;
                }
                else if (strcmp(optarg, "default") == 0)
                {
                    SCT_ERROR_RECOVERY_CONTROL_WRITE_SET_DEFAULT = true;
                }
                else
                {
                    uint32_t multiplier = UINT32_C(100); // 100 millisecond conversion
                    char*    unit       = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME,
                                                              &SCT_ERROR_RECOVERY_CONTROL_READ_TIMER_VALUE))
                    {
                        // first check is a unit is provided.
                        if (unit)
                        {
                            if (strcmp(unit, "ms") == 0 || strcmp(unit, "") == 0)
                            {
                                multiplier = UINT32_C(1);
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                multiplier = UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                multiplier = UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                multiplier = UINT32_C(3600000);
                            }
                            else
                            {
                                print_Error_In_Cmd_Line_Args(SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        SCT_ERROR_RECOVERY_CONTROL_SET_WRITE_TIMER = true;
                        SCT_ERROR_RECOVERY_CONTROL_WRITE_TIMER_VALUE *= multiplier;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, FREE_FALL_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    FREE_FALL_INFO = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    FREE_FALL_FLAG        = true;
                    FREE_FALL_SENSITIVITY = 0; // enable to vendor recommended if this arg is given
                }
                else
                {
                    // this is a value to read in.
                    if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                              &FREE_FALL_SENSITIVITY))
                    {
                        print_Error_In_Cmd_Line_Args(FREE_FALL_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    else
                    {
                        FREE_FALL_FLAG = true;
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_MP_RESET_LONG_OPT_STRING) == 0)
            {
                SCSI_MP_RESET_OP  = true;
                char*   saveptr   = M_NULLPTR;
                rsize_t duplen    = RSIZE_T_C(0);
                char*   dupoptarg = M_NULLPTR;
                char*   token     = M_NULLPTR;
                uint8_t count     = UINT8_C(0);
                bool    errorInCL = false;
                if (safe_strdup(&dupoptarg, optarg) == 0 && dupoptarg != M_NULLPTR)
                {
                    duplen = safe_strlen(dupoptarg);
                    token  = safe_String_Token(dupoptarg, &duplen, "-", &saveptr);
                }
                else
                {
                    errorInCL = true;
                }
                while (token && !errorInCL && count < UINT8_C(2))
                {
                    uint8_t value = UINT8_C(0);
                    if (get_And_Validate_Integer_Input_Uint8(token, M_NULLPTR, ALLOW_UNIT_NONE, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_MP_RESET_PAGE_NUMBER = value;
                            if (value > MP_RETURN_ALL_PAGES)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_MP_RESET_SUBPAGE_NUMBER = value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = safe_String_Token(M_NULLPTR, &duplen, "-", &saveptr);
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                safe_free(&dupoptarg);
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_MP_RESET_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_MP_RESTORE_LONG_OPT_STRING) == 0)
            {
                SCSI_MP_RESTORE_OP = true;
                char*   saveptr    = M_NULLPTR;
                rsize_t duplen     = RSIZE_T_C(0);
                char*   dupoptarg  = M_NULLPTR;
                char*   token      = M_NULLPTR;
                uint8_t count      = UINT8_C(0);
                bool    errorInCL  = false;
                if (safe_strdup(&dupoptarg, optarg) == 0 && dupoptarg != M_NULLPTR)
                {
                    duplen = safe_strlen(dupoptarg);
                    token  = safe_String_Token(dupoptarg, &duplen, "-", &saveptr);
                }
                else
                {
                    errorInCL = true;
                }
                while (token && !errorInCL && count < 2)
                {
                    uint8_t value = UINT8_C(0);
                    if (get_And_Validate_Integer_Input_Uint8(token, M_NULLPTR, ALLOW_UNIT_NONE, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_MP_RESTORE_PAGE_NUMBER = value;
                            if (value > MP_RETURN_ALL_PAGES)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_MP_RESTORE_SUBPAGE_NUMBER = value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = safe_String_Token(M_NULLPTR, &duplen, "-", &saveptr);
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                safe_free(&dupoptarg);
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_MP_RESTORE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_MP_SAVE_LONG_OPT_STRING) == 0)
            {
                SCSI_MP_SAVE_OP   = true;
                char*   saveptr   = M_NULLPTR;
                rsize_t duplen    = RSIZE_T_C(0);
                char*   dupoptarg = M_NULLPTR;
                char*   token     = M_NULLPTR;
                uint8_t count     = UINT8_C(0);
                bool    errorInCL = false;
                if (safe_strdup(&dupoptarg, optarg) == 0 && dupoptarg != M_NULLPTR)
                {
                    duplen = safe_strlen(dupoptarg);
                    token  = safe_String_Token(dupoptarg, &duplen, "-", &saveptr);
                }
                else
                {
                    errorInCL = true;
                }
                while (token && !errorInCL && count < UINT8_C(2))
                {
                    uint8_t value = UINT8_C(0);
                    if (get_And_Validate_Integer_Input_Uint8(token, M_NULLPTR, ALLOW_UNIT_NONE, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_MP_SAVE_PAGE_NUMBER = value;
                            if (value > MP_RETURN_ALL_PAGES)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_MP_SAVE_SUBPAGE_NUMBER = value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = safe_String_Token(M_NULLPTR, &duplen, "-", &saveptr);
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                safe_free(&dupoptarg);
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_MP_SAVE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_SHOW_MP_LONG_OPT_STRING) == 0)
            {
                SCSI_SHOW_MP_OP   = true;
                char*   saveptr   = M_NULLPTR;
                rsize_t duplen    = RSIZE_T_C(0);
                char*   dupoptarg = M_NULLPTR;
                char*   token     = M_NULLPTR;
                uint8_t count     = UINT8_C(0);
                bool    errorInCL = false;
                if (safe_strdup(&dupoptarg, optarg) == 0 && dupoptarg != M_NULLPTR)
                {
                    duplen = safe_strlen(dupoptarg);
                    token  = safe_String_Token(dupoptarg, &duplen, "-", &saveptr);
                }
                else
                {
                    errorInCL = true;
                }
                while (token && !errorInCL && count < UINT8_C(2))
                {
                    uint8_t value = UINT8_C(0);
                    if (get_And_Validate_Integer_Input_Uint8(token, M_NULLPTR, ALLOW_UNIT_NONE, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_SHOW_MP_PAGE_NUMBER = value;
                            if (value > MP_RETURN_ALL_PAGES)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_SHOW_MP_SUBPAGE_NUMBER = value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = safe_String_Token(M_NULLPTR, &duplen, "-", &saveptr);
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                safe_free(&dupoptarg);
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_SHOW_MP_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_SET_MP_LONG_OPT_STRING) == 0)
            {
                SCSI_SET_MP_OP = true;
                // first check if they are specifying a file!
                if (strncmp(optarg, "file=", 5) == 0)
                {
                    // format is file=filename.txt
                    char* filenameptr = strstr(optarg, "=");
                    if (filenameptr && safe_strlen(filenameptr) > 1)
                    {
                        filenameptr += 1; // go past the =
                        if (snprintf_err_handle(SCSI_SET_MP_FILENAME, SCSI_SET_MP_FILENAME_LEN, "%s", filenameptr) <= 0)
                        {
                            print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
                else
                {
                    // formatted as mp[-sp]:byte:highbit:fieldWidth=value
#define PARSE_MP_PAGE_AND_SUBPAGE_LENGTH 8
                    DECLARE_ZERO_INIT_ARRAY(char, pageAndSubpage, PARSE_MP_PAGE_AND_SUBPAGE_LENGTH);
                    char*   saveptr   = M_NULLPTR;
                    rsize_t duplen    = RSIZE_T_C(0);
                    char*   dupoptarg = M_NULLPTR;
                    if (safe_strdup(&dupoptarg, optarg) == 0 && dupoptarg != M_NULLPTR)
                    {
                        duplen               = safe_strlen(dupoptarg);
                        char*   token        = safe_String_Token(dupoptarg, &duplen, ":=", &saveptr);
                        uint8_t tokenCounter = UINT8_C(0);
                        while (token && tokenCounter < UINT8_C(5))
                        {
                            // go through each string and convert it from a string into a value we can use in this tool
                            // start with page and subpage
                            switch (tokenCounter)
                            {
                            case 0: // page-subpage
                            {
                                snprintf_err_handle(pageAndSubpage, PARSE_MP_PAGE_AND_SUBPAGE_LENGTH, "%s", token);
                                // parse later outside this loop. If we tokenize again in here, we'll break the way the
                                // parsing works... :(
                            }
                            break;
                            case 1: // byte
                                if (!get_And_Validate_Integer_Input_Uint16(token, M_NULLPTR, ALLOW_UNIT_NONE,
                                                                           &SCSI_SET_MP_BYTE))
                                {
                                    print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                                }
                                break;
                            case 2: // bit
                                if (!get_And_Validate_Integer_Input_Uint8(token, M_NULLPTR, ALLOW_UNIT_NONE,
                                                                          &SCSI_SET_MP_BIT) &&
                                    SCSI_SET_MP_BIT > 7)
                                {
                                    print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                                }
                                break;
                            case 3: // field width
                                if (!get_And_Validate_Integer_Input_Uint8(token, M_NULLPTR, ALLOW_UNIT_NONE,
                                                                          &SCSI_SET_MP_FIELD_LEN_BITS) &&
                                    SCSI_SET_MP_FIELD_LEN_BITS > 64)
                                {
                                    print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                                }
                                break;
                            case 4: // value
                                if (!get_And_Validate_Integer_Input_Uint64(token, M_NULLPTR, ALLOW_UNIT_NONE,
                                                                           &SCSI_SET_MP_FIELD_VALUE))
                                {
                                    print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                                }
                                break;
                            default:
                                // shouldn't get here!!! throw an error?!
                                break;
                            }
                            ++tokenCounter;
                            token = safe_String_Token(M_NULLPTR, &duplen, ":=", &saveptr);
                        }
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    safe_free(&dupoptarg);
                    saveptr           = M_NULLPTR;
                    rsize_t pageSPlen = safe_strlen(pageAndSubpage);
                    char*   pagetoken = safe_String_Token(pageAndSubpage, &pageSPlen, "-", &saveptr);
                    if (pagetoken)
                    {
                        unsigned long temp = 0UL;
                        if (0 == safe_strtoul(&temp, pagetoken, M_NULLPTR, BASE_16_HEX) && temp < UINT8_MAX)
                        {
                            SCSI_SET_MP_PAGE_NUMBER = C_CAST(uint8_t, temp);
                            pagetoken               = safe_String_Token(M_NULLPTR, &pageSPlen, "-", &saveptr);
                            if (pagetoken)
                            {
                                if (0 == safe_strtoul(&temp, pagetoken, M_NULLPTR, BASE_16_HEX) && temp < UINT8_MAX)
                                {
                                    SCSI_SET_MP_SUBPAGE_NUMBER = C_CAST(uint8_t, temp);
                                }
                                else
                                {
                                    print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                                }
                            }
                        }
                        else
                        {
                            print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    else // should this be an error condition since tokenize failed?
                    {
                        // no subpage
                        unsigned long temp = 0UL;
                        if (0 == safe_strtoul(&temp, pageAndSubpage, M_NULLPTR, BASE_16_HEX) && temp < UINT8_MAX)
                        {
                            SCSI_SET_MP_PAGE_NUMBER    = C_CAST(uint8_t, temp);
                            SCSI_SET_MP_SUBPAGE_NUMBER = 0;
                        }
                        else
                        {
                            print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    if (SCSI_SET_MP_PAGE_NUMBER > 0x3F)
                    {
                        print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_SHOW_MP_MPC_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "all") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE =
                        4; // bigger than possible for an actual MPC value, so we can filter below off of this
                }
                else if (strcmp(optarg, "current") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = MPC_CURRENT_VALUES;
                }
                else if (strcmp(optarg, "default") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = MPC_DEFAULT_VALUES;
                }
                else if (strcmp(optarg, "saved") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = MPC_SAVED_VALUES;
                }
                else if (strcmp(optarg, "changeable") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = MPC_CHANGABLE_VALUES;
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "classic") == 0)
                {
                    SCSI_SHOW_MP_BUFFER_MODE = false;
                }
                else if (strcmp(optarg, "buffer") == 0)
                {
                    SCSI_SHOW_MP_BUFFER_MODE = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_RESET_LP_LONG_OPT_STRING) == 0)
            {
                SCSI_RESET_LP_OP = true;
                if (strcmp(optarg, "all") == 0)
                {
                    SCSI_RESET_LP_LPC =
                        4; // bigger than possible for an actual LPC value, so we can filter below off of this
                }
                else if (strcmp(optarg, "threshold") == 0)
                {
                    SCSI_RESET_LP_LPC = LPC_THRESHOLD_VALUES;
                }
                else if (strcmp(optarg, "cumulative") == 0)
                {
                    SCSI_RESET_LP_LPC = LPC_CUMULATIVE_VALUES;
                }
                else if (strcmp(optarg, "defThreshold") == 0)
                {
                    SCSI_RESET_LP_LPC = LPC_DEFAULT_THRESHOLD_VALUES;
                }
                else if (strcmp(optarg, "defCumulative") == 0)
                {
                    SCSI_RESET_LP_LPC = LPC_DEFAULT_CUMULATIVE_VALUES;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SCSI_RESET_LP_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_RESET_LP_PAGE_LONG_OPT_STRING) == 0)
            {
                char*   saveptr   = M_NULLPTR;
                rsize_t duplen    = RSIZE_T_C(0);
                char*   dupoptarg = M_NULLPTR;
                char*   token     = M_NULLPTR;
                uint8_t count     = UINT8_C(0);
                bool    errorInCL = false;
                if (safe_strdup(&dupoptarg, optarg) == 0 && dupoptarg != M_NULLPTR)
                {
                    duplen = safe_strlen(dupoptarg);
                    token  = safe_String_Token(dupoptarg, &duplen, "-", &saveptr);
                }
                else
                {
                    errorInCL = true;
                }
                while (token && !errorInCL && count < UINT8_C(2))
                {
                    uint8_t value = UINT8_C(0);
                    if (get_And_Validate_Integer_Input_Uint8(token, M_NULLPTR, ALLOW_UNIT_NONE, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_RESET_LP_PAGE_NUMBER = value;
                            if (value > 0x3F)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_RESET_LP_SUBPAGE_NUMBER = value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = safe_String_Token(M_NULLPTR, &duplen, "-", &saveptr);
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                safe_free(&dupoptarg);
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_RESET_LP_PAGE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_DCO_SETMAXLBA_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &ATA_DCO_SETMAXLBA_VALUE))
                {
                    ATA_DCO_SETMAXLBA = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(ATA_DCO_SETMAXLBA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_DCO_SETMAXMODE_LONG_OPT_STRING) == 0)
            {
                ATA_DCO_SETMAXMODE = true;
                if (strcmp(optarg, ATA_DCO_MODE_UDMA6) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 10;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_UDMA5) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 9;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_UDMA4) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 8;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_UDMA3) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 7;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_UDMA2) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 6;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_UDMA1) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 5;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_UDMA0) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 4;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_MWDMA2) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 3;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_MWDMA1) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 2;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_MWDMA0) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 1;
                }
                else if (strcmp(optarg, ATA_DCO_MODE_NODMA) == 0)
                {
                    ATA_DCO_SETMAXMODE_VALUE = 0;
                }
                else
                {
                    ATA_DCO_SETMAXMODE = false;
                    print_Error_In_Cmd_Line_Args(ATA_DCO_SETMAXLBA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_DCO_DISABLE_FEEATURES_LONG_OPT_STRING) == 0)
            {
                // this needs to parse a comma separated list of things to disable on the drive
                char* dcoDisableFeatList = M_NULLPTR;
                if (safe_strdup(&dcoDisableFeatList, optarg) == 0 && dcoDisableFeatList != M_NULLPTR)
                {
                    char*   saveptr          = M_NULLPTR;
                    rsize_t featlistlen      = safe_strlen(dcoDisableFeatList);
                    char*   dcoFeatToken     = safe_String_Token(dcoDisableFeatList, &featlistlen, ",", &saveptr);
                    ATA_DCO_DISABLE_FEATURES = true;
                    while (dcoFeatToken)
                    {
                        if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_WRV) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT14;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_SMART_CONVEYANCE) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT13;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_SMART_SELECTIVE) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT12;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_FUA) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT11;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_TLC) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT10;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_STREAMING) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT9;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_48BIT) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT8;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_HPA) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT7;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_AAM) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT6;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_TCG) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT5;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_PUIS) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT4;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_SECURITY) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT3;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_SMART_ERRORLOG) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT2;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_SMART_SELF_TEST) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT1;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_SMART_FEATURE) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT0;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_SSP) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT20;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_ASYNC_NOTIFICATION) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT19;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_INTERFACE_POWER_MGMT) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT18;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_NZ_BUFF) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT17;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_NCQ) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT16;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_NVCACHE) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT47;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_NVC_PM) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT46;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_WUE) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT45;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_TCG) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT44;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_FREE_FALL) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT43;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_DSM) == 0 ||
                                 strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_TRIM) == 0)
                        {
                            // allowing DSM or TRIM to work with a commonly known name from the spec to mean the same
                            // thing since these are directly related.-TJE
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT42;
                        }
                        else if (strcmp(dcoFeatToken, ATA_DCO_FEATURE_OPTION_EPC) == 0)
                        {
                            ATA_DCO_DISABLE_FEATURES_VALUE |= BIT41;
                        }
                        else
                        {
                            // error, unknown option
                            safe_free(&dcoDisableFeatList);
                            ATA_DCO_DISABLE_FEATURES = false;
                            print_Error_In_Cmd_Line_Args(ATA_DCO_DISABLE_FEEATURES_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                        dcoFeatToken = safe_String_Token(M_NULLPTR, &featlistlen, ",", &saveptr);
                    }
                    safe_free(&dcoDisableFeatList);
                }
                else
                {
                    ATA_DCO_DISABLE_FEATURES = false;
                    print_Error_In_Cmd_Line_Args(ATA_DCO_DISABLE_FEEATURES_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, WRV_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    WRV_INFO = true;
                }
                else if (strcmp(optarg, "all") == 0)
                {
                    WRV_FLAG = true;
                    WRV_ALL  = true;
                }
                else if (strcmp(optarg, "vendor") == 0)
                {
                    WRV_FLAG   = true;
                    WRV_VENDOR = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    WRV_FLAG    = true;
                    WRV_DISABLE = true;
                }
                else if (get_And_Validate_Integer_Input_Uint32(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &WRV_USER_VALUE))
                {
                    WRV_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(WRV_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING) == 0)
            {
                MODEL_MATCH_FLAG = true;
                snprintf_err_handle(MODEL_STRING_FLAG, MODEL_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING) == 0)
            {
                FW_MATCH_FLAG = true;
                snprintf_err_handle(FW_STRING_FLAG, FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                snprintf_err_handle(CHILD_MODEL_STRING_FLAG, CHILD_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING) == 0)
            {
                CHILD_FW_MATCH_FLAG = true;
                snprintf_err_handle(CHILD_FW_STRING_FLAG, CHILD_FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            break;
        case ':': // missing required argument
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            switch (optopt)
            {
            case 0:
                // check long options for missing arguments
                break;
            case DEVICE_SHORT_OPT:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must specify a device handle\n");
                }
                return UTIL_EXIT_INVALID_DEVICE_HANDLE;
            case VERBOSE_SHORT_OPT:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must specify a verbosity level. 0 - 4 are the valid levels\n");
                }
                break;
            case SCAN_FLAGS_SHORT_OPT:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must specify which scan options flags you want to use.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Option %c requires an argument\n", optopt);
                }
                utility_Usage(true);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\n");
                }
                exit(exitCode);
            }
            break;
        case DEVICE_SHORT_OPT: // device
            if (0 != parse_Device_Handle_Argument(optarg, &RUN_ON_ALL_DRIVES, &USER_PROVIDED_HANDLE, &DEVICE_LIST_COUNT,
                                                  &HANDLE_LIST))
            {
                // Free any memory allocated so far, then exit.
                free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\n");
                }
                exit(255);
            }
            break;
        case DEVICE_INFO_SHORT_OPT: // device information
            DEVICE_INFO_FLAG = true;
            break;
        case SCAN_SHORT_OPT: // scan
            SCAN_FLAG = true;
            break;
        case AGRESSIVE_SCAN_SHORT_OPT:
            AGRESSIVE_SCAN_FLAG = true;
            break;
        case VERSION_SHORT_OPT:
            SHOW_BANNER_FLAG = true;
            break;
        case VERBOSE_SHORT_OPT: // verbose
            if (!set_Verbosity_From_String(optarg, &toolVerbosity))
            {
                print_Error_In_Cmd_Line_Args_Short_Opt(VERBOSE_SHORT_OPT, optarg);
                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
            }
            break;
        case QUIET_SHORT_OPT: // quiet mode
            toolVerbosity = VERBOSITY_QUIET;
            break;
        case SCAN_FLAGS_SHORT_OPT: // scan flags
            get_Scan_Flags(&SCAN_FLAGS, optarg);
            break;
        case '?': // unknown option
            printf("%s: Unable to parse %s command line option\nPlease use --%s for more information.\n", util_name,
                   argv[optind - 1], HELP_LONG_OPT_STRING);
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\n");
            }
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
        case 'h': // help
            SHOW_HELP_FLAG = true;
            openseachest_utility_Info(util_name, buildVersion);
            utility_Usage(false);
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\n");
            }
            exit(UTIL_EXIT_NO_ERROR);
        default:
            break;
        }
    }

    if (0 != atexit(print_Final_newline))
    {
        perror("Registering final newline print");
    }

    if (ECHO_COMMAND_LINE_FLAG)
    {
        int commandLineIter =
            1; // start at 1 as starting at 0 means printing the directory info+ SeaChest.exe (or ./SeaChest)
        for (commandLineIter = 1; commandLineIter < argc; commandLineIter++)
        {
            if (strcmp(argv[commandLineIter], "--echoCommandLine") == 0)
            {
                continue;
            }
            printf("%s ", argv[commandLineIter]);
        }
        printf("\n");
    }

    if ((VERBOSITY_QUIET < toolVerbosity) && !NO_BANNER_FLAG)
    {
        openseachest_utility_Info(util_name, buildVersion);
    }

    if (SHOW_BANNER_FLAG)
    {
        utility_Full_Version_Info(util_name, buildVersion, OPENSEA_TRANSPORT_MAJOR_VERSION,
                                  OPENSEA_TRANSPORT_MINOR_VERSION, OPENSEA_TRANSPORT_PATCH_VERSION,
                                  OPENSEA_COMMON_VERSION, OPENSEA_OPERATION_VERSION);
    }

    if (LICENSE_FLAG)
    {
        print_EULA_To_Screen();
    }

    if (SCAN_FLAG || AGRESSIVE_SCAN_FLAG)
    {
        if (!is_Running_Elevated())
        {
            print_Elevated_Privileges_Text();
        }
        unsigned int scanControl = DEFAULT_SCAN;
        if (AGRESSIVE_SCAN_FLAG)
        {
            scanControl |= AGRESSIVE_SCAN;
        }
#if defined(__linux__)
        if (SCAN_FLAGS.scanSD)
        {
            scanControl |= SD_HANDLES;
        }
        if (SCAN_FLAGS.scanSDandSG)
        {
            scanControl |= SG_TO_SD;
        }
#endif
        // set the drive types to show (if none are set, the lower level code assumes we need to show everything)
        if (SCAN_FLAGS.scanATA)
        {
            scanControl |= ATA_DRIVES;
        }
        if (SCAN_FLAGS.scanUSB)
        {
            scanControl |= USB_DRIVES;
        }
        if (SCAN_FLAGS.scanSCSI)
        {
            scanControl |= SCSI_DRIVES;
        }
        if (SCAN_FLAGS.scanNVMe)
        {
            scanControl |= NVME_DRIVES;
        }
        if (SCAN_FLAGS.scanRAID)
        {
            scanControl |= RAID_DRIVES;
        }
        // set the interface types to show (if none are set, the lower level code assumes we need to show everything)
        if (SCAN_FLAGS.scanInterfaceATA)
        {
            scanControl |= IDE_INTERFACE_DRIVES;
        }
        if (SCAN_FLAGS.scanInterfaceUSB)
        {
            scanControl |= USB_INTERFACE_DRIVES;
        }
        if (SCAN_FLAGS.scanInterfaceSCSI)
        {
            scanControl |= SCSI_INTERFACE_DRIVES;
        }
        if (SCAN_FLAGS.scanInterfaceNVMe)
        {
            scanControl |= NVME_INTERFACE_DRIVES;
        }
#if defined(ENABLE_CSMI)
        if (SCAN_FLAGS.scanIgnoreCSMI)
        {
            scanControl |= IGNORE_CSMI;
        }
        if (SCAN_FLAGS.scanAllowDuplicateDevices)
        {
            scanControl |= ALLOW_DUPLICATE_DEVICE;
        }
#endif
        if (ONLY_SEAGATE_FLAG)
        {
            scanControl |= SCAN_SEAGATE_ONLY;
        }
        scan_And_Print_Devs(scanControl, toolVerbosity);
    }
    // Add to this if list anything that is suppose to be independent.
    // e.g. you can't say enumerate & then pull logs in the same command line.
    // SIMPLE IS BEAUTIFUL
    if (SCAN_FLAG || AGRESSIVE_SCAN_FLAG || SHOW_BANNER_FLAG || LICENSE_FLAG || SHOW_HELP_FLAG)
    {
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_NO_ERROR);
    }

    // print out errors for unknown arguments for remaining args that haven't been processed yet
    for (argIndex = optind; argIndex < argc; argIndex++)
    {
        if (VERBOSITY_QUIET < toolVerbosity)
        {
            printf("Invalid argument: %s\n", argv[argIndex]);
        }
    }

    if (!is_Running_Elevated())
    {
        print_Elevated_Privileges_Text();
    }

    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        uint64_t flags = UINT64_C(0);
        if (SUCCESS != get_Device_Count(&DEVICE_LIST_COUNT, flags))
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Unable to get number of devices\n");
            }
            if (!is_Running_Elevated())
            {
                exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
            }
            else
            {
                exit(UTIL_EXIT_OPERATION_FAILURE);
            }
        }
    }
    else if (DEVICE_LIST_COUNT == 0)
    {
        if (VERBOSITY_QUIET < toolVerbosity)
        {
            printf("You must specify one or more target devices with the --%s option to run this command.\n",
                   DEVICE_LONG_OPT_STRING);
            utility_Usage(true);
            printf("Use -h option for detailed description\n\n");
        }
        exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
    }

    if ((FORCE_SCSI_FLAG && FORCE_ATA_FLAG) || (FORCE_SCSI_FLAG && FORCE_NVME_FLAG) ||
        (FORCE_ATA_FLAG && FORCE_NVME_FLAG) || (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG) ||
        (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG) || (FORCE_ATA_PIO_FLAG && FORCE_ATA_UDMA_FLAG) ||
        (FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG) ||
        (FORCE_SCSI_FLAG &&
         (FORCE_ATA_PIO_FLAG || FORCE_ATA_DMA_FLAG ||
          FORCE_ATA_UDMA_FLAG)) // We may need to remove this. At least when software SAT gets used. (currently only
                                // Windows ATA passthrough and FreeBSD ATA passthrough)
    )
    {
        printf("\nError: Only one force flag can be used at a time.\n");
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    // need to make sure this is set when we are asked for SAT Info
    if (SAT_INFO_FLAG)
    {
        DEVICE_INFO_FLAG = goTrue;
    }
    // check that we were given at least one test to perform...if not, show the help and exit
    if (!(DEVICE_INFO_FLAG || TEST_UNIT_READY_FLAG ||
          LOWLEVEL_INFO_FLAG
          // check for other tool specific options here
          || CAPACITY_MODEL_NUMBER_MAPPING_FLAG || RESTORE_MAX_LBA_FLAG || SET_MAX_LBA_FLAG || SET_PHY_SPEED_FLAG ||
          SET_READY_LED_FLAG || READY_LED_INFO_FLAG || WRITE_CACHE_FLAG || READ_LOOK_AHEAD_FLAG ||
          READ_LOOK_AHEAD_INFO || NV_CACHE_INFO || NV_CACHE_FLAG || WRITE_CACHE_INFO || PROVISION_FLAG ||
          LOW_CURRENT_SPINUP_FLAG || SCT_WRITE_CACHE_INFO || SCT_WRITE_CACHE_FLAG || SCT_WRITE_CACHE_REORDER_FLAG ||
          SCT_WRITE_CACHE_REORDER_INFO || PUIS_FEATURE_FLAG || SET_SSC_FLAG || GET_SSC_FLAG ||
          SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO || SCT_ERROR_RECOVERY_CONTROL_READ_INFO ||
          SCT_ERROR_RECOVERY_CONTROL_SET_READ_TIMER || SCT_ERROR_RECOVERY_CONTROL_SET_WRITE_TIMER ||
          SCT_ERROR_RECOVERY_CONTROL_WRITE_SET_DEFAULT || SCT_ERROR_RECOVERY_CONTROL_READ_SET_DEFAULT ||
          FREE_FALL_FLAG || FREE_FALL_INFO || SCSI_MP_RESET_OP || SCSI_MP_RESTORE_OP || SCSI_MP_SAVE_OP ||
          SCSI_SHOW_MP_OP || SCSI_RESET_LP_OP || SCSI_SET_MP_OP || ATA_DCO_DISABLE_FEATURES || ATA_DCO_SETMAXMODE ||
          ATA_DCO_SETMAXLBA || ATA_DCO_IDENTIFY || ATA_DCO_FREEZE || ATA_DCO_RESTORE || WRV_FLAG || WRV_INFO || SET_TIMESTAMP))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    uint64_t flags = UINT64_C(0);
    DEVICE_LIST    = M_REINTERPRET_CAST(tDevice*, safe_calloc(DEVICE_LIST_COUNT, sizeof(tDevice)));
    if (!DEVICE_LIST)
    {
        if (VERBOSITY_QUIET < toolVerbosity)
        {
            printf("Unable to allocate memory\n");
        }
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_OPERATION_FAILURE);
    }
    versionBlock version;
    safe_memset(&version, sizeof(versionBlock), 0, sizeof(versionBlock));
    version.version = DEVICE_BLOCK_VERSION;
    version.size    = sizeof(tDevice);

    if (TEST_UNIT_READY_FLAG)
    {
        flags = DO_NOT_WAKE_DRIVE;
    }

    if (FAST_DISCOVERY_FLAG)
    {
        flags = FAST_SCAN;
    }

    // set flags that can be passed down in get device regarding forcing specific ATA modes.
    if (FORCE_ATA_PIO_FLAG)
    {
        flags |= FORCE_ATA_PIO_ONLY;
    }

    if (FORCE_ATA_DMA_FLAG)
    {
        flags |= FORCE_ATA_DMA_SAT_MODE;
    }

    if (FORCE_ATA_UDMA_FLAG)
    {
        flags |= FORCE_ATA_UDMA_SAT_MODE;
    }

    eReturnValues getDevsRet = SUCCESS;
    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {

        for (uint32_t devi = UINT32_C(0); devi < DEVICE_LIST_COUNT; ++devi)
        {
            DEVICE_LIST[devi].deviceVerbosity = toolVerbosity;
        }
        getDevsRet = get_Device_List(DEVICE_LIST, DEVICE_LIST_COUNT * sizeof(tDevice), version, flags);
        ret = getDevsRet;
        if (SUCCESS != ret)
        {
            if (ret == WARN_NOT_ALL_DEVICES_ENUMERATED)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("WARN: Not all devices enumerated correctly\n");
                }
            }
            else if (ret == PERMISSION_DENIED)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("WARN: Not all devices were opened. Some failed for lack of permissions\n");
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unable to get device list\n");
                }
                if (!is_Running_Elevated())
                {
                    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                    exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
                }
                else
                {
                    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                    exit(UTIL_EXIT_OPERATION_FAILURE);
                }
            }
        }
    }
    else
    {
        /*need to go through the handle list and attempt to open each handle.*/
        for (uint32_t handleIter = UINT32_C(0); handleIter < DEVICE_LIST_COUNT; ++handleIter)
        {
            /*Initializing is necessary*/
            deviceList[handleIter].sanity.size    = sizeof(tDevice);
            deviceList[handleIter].sanity.version = DEVICE_BLOCK_VERSION;
#if defined(UEFI_C_SOURCE)
            deviceList[handleIter].os_info.fd = M_NULLPTR;
#elif !defined(_WIN32)
            deviceList[handleIter].os_info.fd     = -1;
#    if defined(VMK_CROSS_COMP)
            deviceList[handleIter].os_info.nvmeFd = M_NULLPTR;
#    endif
#else
            deviceList[handleIter].os_info.fd = INVALID_HANDLE_VALUE;
#endif
            deviceList[handleIter].dFlags = flags;

            deviceList[handleIter].deviceVerbosity = toolVerbosity;

            if (ENABLE_LEGACY_PASSTHROUGH_FLAG)
            {
                deviceList[handleIter].drive_info.ata_Options.enableLegacyPassthroughDetectionThroughTrialAndError =
                    true;
            }

            /*get the device for the specified handle*/
#if defined(_DEBUG)
            printf("Attempting to open handle \"%s\"\n", HANDLE_LIST[handleIter]);
#endif
            ret = get_Device(HANDLE_LIST[handleIter], &deviceList[handleIter]);
#if !defined(_WIN32)
#    if !defined(VMK_CROSS_COMP)
            if ((deviceList[handleIter].os_info.fd < 0) ||
#    else
            if (((deviceList[handleIter].os_info.fd < 0) && (deviceList[handleIter].os_info.nvmeFd == M_NULLPTR)) ||
#    endif
                (ret != SUCCESS))
#else
            if ((deviceList[handleIter].os_info.fd == INVALID_HANDLE_VALUE) ||
                (ret != SUCCESS))
#endif
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Error: Could not open handle to %s\n", HANDLE_LIST[handleIter]);
                }
                free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                if (ret == PERMISSION_DENIED || !is_Running_Elevated())
                {
                    exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
                }
                else if (ret == DEVICE_BUSY)
                {
                    exit(UTIL_EXIT_DEVICE_BUSY);
                }
                else if (ret == DEVICE_INVALID)
                {
                    exit(UTIL_EXIT_NO_DEVICE);
                }
                else
                {
                    exit(UTIL_EXIT_OPERATION_FAILURE);
                }
            }
        }
    }
    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
    uint32_t skippedDevices = UINT32_C(0);
    for (uint32_t deviceIter = UINT32_C(0); deviceIter < DEVICE_LIST_COUNT; ++deviceIter)
    {
        deviceList[deviceIter].deviceVerbosity = toolVerbosity;
        if (ONLY_SEAGATE_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                /*if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) is not a Seagate drive.\n", deviceList[deviceIter].os_info.name,
                deviceList[deviceIter].drive_info.product_identification);
                }*/
                ++skippedDevices;
                continue;
            }
        }

        // check for model number match
        if (MODEL_MATCH_FLAG)
        {
            if (strstr(deviceList[deviceIter].drive_info.product_identification, MODEL_STRING_FLAG) == M_NULLPTR)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) does not match the input model number: %s\n",
                           deviceList[deviceIter].os_info.name,
                           deviceList[deviceIter].drive_info.product_identification, MODEL_STRING_FLAG);
                }
                ++skippedDevices;
                continue;
            }
        }
        // check for fw match
        if (FW_MATCH_FLAG)
        {
            if (strcmp(FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision) != 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input firmware revision: %s\n",
                           deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_revision,
                           FW_STRING_FLAG);
                }
                ++skippedDevices;
                continue;
            }
        }

        // check for child model number match
        if (CHILD_MODEL_MATCH_FLAG)
        {
            if (safe_strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN) == 0 ||
                strstr(deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG) ==
                    M_NULLPTR)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) does not match the input child model number: %s\n",
                           deviceList[deviceIter].os_info.name,
                           deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG);
                }
                ++skippedDevices;
                continue;
            }
        }
        // check for child fw match
        if (CHILD_FW_MATCH_FLAG)
        {
            if (strcmp(CHILD_FW_STRING_FLAG, deviceList[deviceIter].drive_info.bridge_info.childDriveFW) != 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input child firmware revision: %s\n",
                           deviceList[deviceIter].os_info.name,
                           deviceList[deviceIter].drive_info.bridge_info.childDriveFW, CHILD_FW_STRING_FLAG);
                }
                ++skippedDevices;
                continue;
            }
        }

        if (FORCE_SCSI_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\tForcing SCSI Drive\n");
            }
            deviceList[deviceIter].drive_info.drive_type = SCSI_DRIVE;
        }

        if (FORCE_ATA_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\tForcing ATA Drive\n");
            }
            deviceList[deviceIter].drive_info.drive_type = ATA_DRIVE;
        }

        if (FORCE_NVME_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\tForcing NVME Drive\n");
            }
            deviceList[deviceIter].drive_info.drive_type = NVME_DRIVE;
        }

        if (FORCE_ATA_PIO_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\tAttempting to force ATA Drive commands in PIO Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaSupported                  = false;
            deviceList[deviceIter].drive_info.ata_Options.dmaMode                       = ATA_DMA_MODE_NO_DMA;
            deviceList[deviceIter].drive_info.ata_Options.downloadMicrocodeDMASupported = false;
            deviceList[deviceIter].drive_info.ata_Options.readBufferDMASupported        = false;
            deviceList[deviceIter].drive_info.ata_Options.readLogWriteLogDMASupported   = false;
            deviceList[deviceIter].drive_info.ata_Options.writeBufferDMASupported       = false;
        }

        if (FORCE_ATA_DMA_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\tAttempting to force ATA Drive commands in DMA Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaMode = ATA_DMA_MODE_DMA;
        }

        if (FORCE_ATA_UDMA_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\tAttempting to force ATA Drive commands in UDMA Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaMode = ATA_DMA_MODE_UDMA;
        }

        if (deviceList[deviceIter].drive_info.interface_type == UNKNOWN_INTERFACE)
        {
            ++skippedDevices;
            continue;
        }

        if (VERBOSITY_QUIET < toolVerbosity)
        {
            printf("\n%s - %s - %s - %s - %s\n", deviceList[deviceIter].os_info.name,
                   deviceList[deviceIter].drive_info.product_identification,
                   deviceList[deviceIter].drive_info.serialNumber, deviceList[deviceIter].drive_info.product_revision,
                   print_drive_type(&deviceList[deviceIter]));
        }

        // now start looking at what operations are going to be performed and kick them off
        if (DEVICE_INFO_FLAG)
        {
            if (SUCCESS != print_Drive_Information(&deviceList[deviceIter], SAT_INFO_FLAG))
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: failed to get device information\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (LOWLEVEL_INFO_FLAG)
        {
            print_Low_Level_Info(&deviceList[deviceIter]);
        }

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (CAPACITY_MODEL_NUMBER_MAPPING_FLAG)
        {
            if (is_Change_Identify_String_Supported(&deviceList[deviceIter]))
            {
                ptrcapacityModelNumberMapping capModelMap = get_Capacity_Model_Number_Mapping(&deviceList[deviceIter]);
                if (capModelMap)
                {
                    print_Capacity_Model_Number_Mapping(capModelMap);
                    delete_Capacity_Model_Number_Mapping(capModelMap);
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: failed to get Capacity / Model Number Mapping\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Capacity / Model Number Mapping not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SCSI_SHOW_MP_OP)
        {
            if (SCSI_SHOW_MP_MPC_VALUE > 3)
            {
                // showing all MPC values for the page
                show_SCSI_Mode_Page_All(&deviceList[deviceIter], SCSI_SHOW_MP_PAGE_NUMBER, SCSI_SHOW_MP_SUBPAGE_NUMBER,
                                        SCSI_SHOW_MP_BUFFER_MODE);
            }
            else
            {
                // show the specific MPC value
                show_SCSI_Mode_Page(&deviceList[deviceIter], SCSI_SHOW_MP_PAGE_NUMBER, SCSI_SHOW_MP_SUBPAGE_NUMBER,
                                    C_CAST(eScsiModePageControl, SCSI_SHOW_MP_MPC_VALUE), SCSI_SHOW_MP_BUFFER_MODE);
            }
        }

        if (SET_TIMESTAMP)
        {
            switch (set_Date_And_Time_Timestamp(&deviceList[deviceIter]))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set date and time timestamp\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting timestamp is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set date and time timestamp\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (ATA_DCO_IDENTIFY)
        {
            dcoData dco;
            safe_memset(&dco, sizeof(dcoData), 0, sizeof(dcoData));
            switch (dco_Identify(&deviceList[deviceIter], &dco))
            {
            case SUCCESS:
                show_DCO_Identify_Data(&dco);
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DCO not supported by this device.\n");
                }
                break;
            case FROZEN:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DCO is Frozen. Cannot identify.\n");
                    printf("Device must be power cycled to clear freeze-lock.\n");
                    printf("Some BIOS's will send the freeze-lock command on boot. Moving the drive to a different\n");
                    printf("system/HBA may be necessary in order to avoid the freeze-lock from occuring.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed DCO Identify command for unknown reason.\n");
                    printf("Device may be in DCO frozen state or command may be blocked by the OS/BIOS/driver.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (ATA_DCO_RESTORE)
        {
            bool scsiAtaInSync = false;
            switch (dco_Restore(&deviceList[deviceIter]))
            {
            case SUCCESS:
                scsiAtaInSync = is_Max_LBA_In_Sync_With_Adapter_Or_Driver(&deviceList[deviceIter], false);
                fill_Drive_Info_Data(&deviceList[deviceIter]);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully restored factory settings using DCO.\n");
                    if (!scsiAtaInSync)
                    {
                        printf("\nWARNING: The adapter/driver/bridge is not in sync with the capacity change!\n");
                        printf("         A reboot is strongly recommended to make sure the system works without\n");
                        printf("         errors with the drive at its new capacity.\n\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DCO not supported by this device.\n");
                }
                break;
            case FROZEN:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DCO is Frozen. Cannot restore DCO settings to factory.\n");
                    printf("Device must be power cycled to clear freeze-lock.\n");
                    printf("Some BIOS's will send the freeze-lock command on boot. Moving the drive to a different\n");
                    printf("system/HBA may be necessary in order to avoid the freeze-lock from occuring.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case ABORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed DCO Restore command.\n");
                    printf("DCO may already be in factory state (restored) or the\n");
                    printf("device may have active HPA that must be\n");
                    printf("disabled before DCO restore can be used.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed DCO Restore command.\n");
                    printf("Device may be in DCO frozen state or may have active HPA that must be\n");
                    printf("disabled before DCO restore can be used.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (ATA_DCO_DISABLE_FEATURES || ATA_DCO_SETMAXMODE || ATA_DCO_SETMAXLBA)
        {
            dcoData dco;
            bool    scsiAtaInSync = false;
            safe_memset(&dco, sizeof(dcoData), 0, sizeof(dcoData));
            switch (dco_Identify(&deviceList[deviceIter], &dco))
            {
            case SUCCESS:
                // change the features we want to disable, then issue the set command
                if (ATA_DCO_SETMAXMODE)
                {
                    // turn all modes to false, then set which one should be the highest available and fall-through
                    // setting all other lower modes
                    dco.udma.udma0   = false;
                    dco.udma.udma1   = false;
                    dco.udma.udma2   = false;
                    dco.udma.udma3   = false;
                    dco.udma.udma4   = false;
                    dco.udma.udma5   = false;
                    dco.udma.udma6   = false;
                    dco.mwdma.mwdma0 = false;
                    dco.mwdma.mwdma1 = false;
                    dco.mwdma.mwdma2 = false;
                    // now go turn modes on to highest supported value
                    switch (ATA_DCO_SETMAXMODE_VALUE)
                    {
                    case 10: // udma6
                        dco.udma.udma6 = true;
                        M_FALLTHROUGH;
                    case 9: // udma5
                        dco.udma.udma5 = true;
                        M_FALLTHROUGH;
                    case 8: // udma4
                        dco.udma.udma4 = true;
                        M_FALLTHROUGH;
                    case 7: // udma3
                        dco.udma.udma3 = true;
                        M_FALLTHROUGH;
                    case 6: // udma2
                        dco.udma.udma2 = true;
                        M_FALLTHROUGH;
                    case 5: // udma1
                        dco.udma.udma1 = true;
                        M_FALLTHROUGH;
                    case 4: // udma0
                        dco.udma.udma0 = true;
                        M_FALLTHROUGH;
                    case 3: // mwdma2
                        dco.mwdma.mwdma2 = true;
                        M_FALLTHROUGH;
                    case 2: // mwdma1
                        dco.mwdma.mwdma1 = true;
                        M_FALLTHROUGH;
                    case 1: // mwdma0
                        dco.mwdma.mwdma0 = true;
                        M_FALLTHROUGH;
                    case 0: // no DMA modes at all
                        break;
                    }
                }
                if (ATA_DCO_SETMAXLBA)
                {
                    // change MaxLBA to requested value
                    dco.maxLBA = ATA_DCO_SETMAXLBA_VALUE;
                }
                if (ATA_DCO_DISABLE_FEATURES)
                {
                    // change which features are allowed
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT14)
                    {
                        // wrv
                        dco.feat1.writeReadVerify = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT13)
                    {
                        // smart conveyance self test
                        dco.feat1.smartConveyanceSelfTest = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT12)
                    {
                        // smart selective self test
                        dco.feat1.smartSelectiveSelfTest = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT11)
                    {
                        // force unit access
                        dco.feat1.forceUnitAccess = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT10)
                    {
                        // tlc
                        dco.feat1.timeLimitedCommands = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT9)
                    {
                        // streaming
                        dco.feat1.streaming = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT8)
                    {
                        // 48bit addressing;
                        dco.feat1.fourtyEightBitAddress = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT7)
                    {
                        // HPA
                        dco.feat1.hostProtectedArea = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT6)
                    {
                        // AAM
                        dco.feat1.automaticAccousticManagement = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT5)
                    {
                        // TCQ
                        dco.feat1.readWriteDMAQueued = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT4)
                    {
                        // PUIS
                        dco.feat1.powerUpInStandby = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT3)
                    {
                        // ATA security
                        dco.feat1.ATAsecurity = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT2)
                    {
                        // SMART error log;
                        dco.feat1.smartErrorLog = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT1)
                    {
                        // smart self-test
                        dco.feat1.smartSelfTest = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT0)
                    {
                        // smart feature;
                        dco.feat1.smartFeature = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT20)
                    {
                        // sata ssp;
                        dco.sataFeat.softwareSettingsPreservation = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT19)
                    {
                        // sata asynchronous notification
                        dco.sataFeat.asynchronousNotification = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT18)
                    {
                        // sata interface power management
                        dco.sataFeat.interfacePowerManagement = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT17)
                    {
                        // sata non-zero buffer offsets
                        dco.sataFeat.nonZeroBufferOffsets = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT16)
                    {
                        // sata ncq
                        dco.sataFeat.ncqFeature = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT47)
                    {
                        // nv cache
                        dco.feat2.nvCache = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT46)
                    {
                        // nvcache power management
                        dco.feat2.nvCachePowerManagement = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT45)
                    {
                        // write uncorrectable ext
                        dco.feat2.writeUncorrectable = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT44)
                    {
                        // tcg
                        dco.feat2.trustedComputing = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT43)
                    {
                        // freefall
                        dco.feat2.freeFall = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT42)
                    {
                        // data set management (DSM) or (TRIM)
                        dco.feat2.dataSetManagement = false;
                    }
                    if (ATA_DCO_DISABLE_FEATURES_VALUE & BIT41)
                    {
                        // extended power conditions (EPC)
                        dco.feat2.extendedPowerConditions = false;
                    }
                }
                switch (dco_Set(&deviceList[deviceIter], &dco))
                {
                case SUCCESS:
                    if (ATA_DCO_SETMAXLBA)
                    {
                        scsiAtaInSync = is_Max_LBA_In_Sync_With_Adapter_Or_Driver(&deviceList[deviceIter], false);
                        fill_Drive_Info_Data(&deviceList[deviceIter]);
                    }
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Successfully configured available features/modes/maxLBA using DCO.\n");
                        if (!scsiAtaInSync)
                        {
                            printf("\nWARNING: The adapter/driver/bridge is not in sync with the capacity change!\n");
                            printf("         A reboot is strongly recommended to make sure the system works without\n");
                            printf("         errors with the drive at its new capacity.\n\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DCO not supported by this device.\n");
                    }
                    break;
                case FROZEN:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DCO is Frozen. Cannot set DCO features.\n");
                        printf("Device must be power cycled to clear freeze-lock.\n");
                        printf(
                            "Some BIOS's will send the freeze-lock command on boot. Moving the drive to a different\n");
                        printf("system/HBA may be necessary in order to avoid the freeze-lock from occuring.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                case ABORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed DCO set command.\n");
                        printf("DCO set may already have been used to configure the device and\n");
                        printf("a DCO restore settings may be required before making more modifications with DCO.\n");
                        printf("Device may have active HPA that must be\n");
                        printf("disabled before DCO set can be used.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed DCO set command.\n");
                        printf("Device may be in DCO frozen state or may have active HPA that must be\n");
                        printf("disabled before DCO set can be used.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
                break;
            case NOT_SUPPORTED:
            case FROZEN:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DCO is Frozen. Cannot set DCO features.\n");
                    printf("Device must be power cycled to clear freeze-lock.\n");
                    printf("Some BIOS's will send the freeze-lock command on boot. Moving the drive to a different\n");
                    printf("system/HBA may be necessary in order to avoid the freeze-lock from occuring.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case ABORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed DCO identify command.\n");
                    printf("Device may have active HPA that must be\n");
                    printf("disabled before DCO identify can be used.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed DCO identify command.\n");
                    printf("Device may be in DCO frozen state or may have active HPA that must be\n");
                    printf("disabled before DCO identify can be used.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (ATA_DCO_FREEZE)
        {
            switch (dco_Freeze_Lock(&deviceList[deviceIter]))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully froze DCO command access.\n");
                }
                break;
            case FROZEN:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DCO is already frozen.\n");
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DCO not supported by this device.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed DCO Freeze-lock command.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_PHY_SPEED_FLAG)
        {
            switch (
                set_phy_speed(&deviceList[deviceIter], SET_PHY_SPEED_GEN, SET_PHY_ALL_PHYS, SET_PHY_SAS_PHY_IDENTIFIER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set the PHY speed. Please power cycle the device to complete this change.\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Operation not supported by this device.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the PHY speed of the device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_SSC_FLAG)
        {
            switch (set_SSC_Feature_SATA(&deviceList[deviceIter], C_CAST(eSSCFeatureState, SSC_MODE)))
            {
            case SUCCESS:
                printf("Successfully set SSC feature to ");
                switch (SSC_MODE)
                {
                case SSC_DEFAULT:
                    printf("the drive's default value\n");
                    break;
                case SSC_ENABLED:
                    printf("enabled\n");
                    break;
                case SSC_DISABLED:
                    printf("disabled\n");
                    break;
                }
                printf("Please power cycle the drive to make the change take effect.\n");
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                printf("SSC feature is not supported on this drive.\n");
                break;
            case FAILURE:
            default:
                printf("Failed to set the drive's SSC state.\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (GET_SSC_FLAG)
        {
            switch (get_SSC_Feature_SATA(&deviceList[deviceIter], C_CAST(eSSCFeatureState*, &SSC_MODE)))
            {
            case SUCCESS:
                printf("SSC Feature is ");
                switch (SSC_MODE)
                {
                case SSC_DEFAULT:
                    printf("set to the drive's default value\n");
                    break;
                case SSC_ENABLED:
                    printf("enabled\n");
                    break;
                case SSC_DISABLED:
                    printf("disabled\n");
                    break;
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                printf("SSC feature is not supported on this drive.\n");
                break;
            case FAILURE:
            default:
                printf("Failed to get the drive's SSC state.\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (READY_LED_INFO_FLAG)
        {
            bool readyLEDValue = false;
            switch (get_Ready_LED_State(&deviceList[deviceIter], &readyLEDValue))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (readyLEDValue)
                    {
                        printf("Ready LED is set to \"On\"\n");
                    }
                    else
                    {
                        printf("Ready LED is set to \"Off\"\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unable to read ready LED info on this device or this device type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to read ready LED info!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_READY_LED_FLAG)
        {
            switch (change_Ready_LED(&deviceList[deviceIter], SET_READY_LED_DEFAULT, SET_READY_LED_MODE))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully changed Ready LED behavior!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Changing Ready LED behavior is not supported on this device or this device type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to change Ready LED behavior!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (WRITE_CACHE_INFO)
        {
            if (is_Write_Cache_Enabled(&deviceList[deviceIter]))
            {
                printf("Write Cache is Enabled\n");
            }
            else
            {
                printf("Write Cache is Disabled\n");
            }
        }

        if (WRITE_CACHE_FLAG)
        {
            switch (set_Write_Cache(&deviceList[deviceIter], WRITE_CACHE_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Write cache successfully enabled!\n");
                    }
                    else
                    {
                        printf("Write cache successfully disabled!\n");
                    }
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Enabling Write cache not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling Write cache not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Failed to enable Write cache!\n");
                    }
                    else
                    {
                        printf("Failed to disable Write cache!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_WRITE_CACHE_INFO)
        {
            uint16_t sctFlags = UINT16_C(0);
            uint16_t state    = UINT16_C(0);
            if (SUCCESS == sct_Get_Feature_Control(&deviceList[deviceIter], SCT_FEATURE_CONTROL_WRITE_CACHE_STATE,
                                                   &SCT_WRITE_CACHE_SETTING, &SCT_WRITE_CACHE_SET_DEFAULT, &state,
                                                   &sctFlags))
            {
                if (state == 0 || state > 0x0003)
                {
                    printf("Unable to retrieve SCT Write Cache Status.\n");
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
                else
                {
                    if (SCT_WRITE_CACHE_SET_DEFAULT)
                    {
                        printf("Write Cache is controlled by set features command (default)");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_SETTING)
                        {
                            printf("Write Cache is Enabled");
                        }
                        else
                        {
                            printf("Write Cache is Disabled");
                        }
                    }
                    if (sctFlags == 0x0001)
                    {
                        printf(" (non-volatile)");
                    }
                    else if (sctFlags == 0x0000)
                    {
                        printf(" (volatile)");
                    }
                    printf("\n");
                }
            }
            else
            {
                printf("Unable to retrieve SCT Write Cache Status.\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SCT_WRITE_CACHE_FLAG)
        {
            switch (sct_Set_Feature_Control(&deviceList[deviceIter], SCT_FEATURE_CONTROL_WRITE_CACHE_STATE,
                                            SCT_WRITE_CACHE_SETTING, SCT_WRITE_CACHE_SET_DEFAULT, VOLATILE_FLAG, 0))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SCT_WRITE_CACHE_SET_DEFAULT)
                    {
                        printf("SCT Write cache successfully restored to defaults!\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_SETTING)
                        {
                            printf("SCT Write cache successfully enabled!\n");
                        }
                        else
                        {
                            printf("SCT Write cache successfully disabled!\n");
                        }
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SCT_WRITE_CACHE_SET_DEFAULT)
                    {
                        printf("Setting SCT Write cache to defaults is not supported on this device\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_SETTING)
                        {
                            printf("SCT Enabling Write cache not supported on this device.\n");
                        }
                        else
                        {
                            printf("SCT Disabling Write cache not supported on this device.\n");
                        }
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SCT_WRITE_CACHE_SET_DEFAULT)
                    {
                        printf("Failed to restore SCT Write cache to defaults!\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_SETTING)
                        {
                            printf("SCT Failed to enable Write cache!\n");
                        }
                        else
                        {
                            printf("SCT Failed to disable Write cache!\n");
                        }
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_WRITE_CACHE_REORDER_INFO)
        {
            uint16_t sctFlags = UINT16_C(0);
            uint16_t state    = UINT16_C(0);
            if (SUCCESS == sct_Get_Feature_Control(&deviceList[deviceIter], SCT_FEATURE_CONTROL_WRITE_CACHE_REORDERING,
                                                   &SCT_WRITE_CACHE_REORDER_SETTING,
                                                   &SCT_WRITE_CACHE_REORDER_SET_DEFAULT, &state, &sctFlags))
            {
                if (state == 0 || state > 0x0002)
                {
                    printf("Unable to retrieve SCT Write Cache Reordering Status.\n");
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
                else
                {

                    if (SCT_WRITE_CACHE_REORDER_SETTING)
                    {
                        printf("Write Cache Reordering is Enabled (default)");
                    }
                    else
                    {
                        printf("Write Cache Reordering is Disabled");
                    }
                    if (sctFlags == 0x0001)
                    {
                        printf(" (non-volatile)");
                    }
                    else if (sctFlags == 0x0000)
                    {
                        printf(" (volatile)");
                    }
                    printf("\n");
                }
            }
            else
            {
                printf("Unable to retrieve SCT Write Cache Reordering Status.\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SCT_WRITE_CACHE_REORDER_FLAG)
        {
            switch (sct_Set_Feature_Control(&deviceList[deviceIter], SCT_FEATURE_CONTROL_WRITE_CACHE_REORDERING,
                                            SCT_WRITE_CACHE_REORDER_SETTING, SCT_WRITE_CACHE_REORDER_SET_DEFAULT,
                                            VOLATILE_FLAG, 0))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SCT_WRITE_CACHE_REORDER_SET_DEFAULT)
                    {
                        printf("SCT Write cache Reordering successfully restored to defaults!\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_REORDER_SETTING)
                        {
                            printf("SCT Write cache Reordering successfully enabled!\n");
                        }
                        else
                        {
                            printf("SCT Write cache Reordering successfully disabled!\n");
                        }
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SCT_WRITE_CACHE_REORDER_SET_DEFAULT)
                    {
                        printf("Setting SCT Write cache Reordering to defaults is not supported on this device\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_REORDER_SETTING)
                        {
                            printf("SCT Enabling Write cache Reordering not supported on this device.\n");
                        }
                        else
                        {
                            printf("SCT Disabling Write cache Reordering not supported on this device.\n");
                        }
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SCT_WRITE_CACHE_REORDER_SET_DEFAULT)
                    {
                        printf("Failed to restore SCT Write cache Reordering to defaults!\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_REORDER_SETTING)
                        {
                            printf("SCT Failed to enable Write cache Reordering!\n");
                        }
                        else
                        {
                            printf("SCT Failed to disable Write cache Reordering!\n");
                        }
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_READ_SET_DEFAULT)
        {
            switch (sct_Restore_Command_Timer(&deviceList[deviceIter], SCT_ERC_READ_COMMAND))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully restore SCT error recovery read command timer to default!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf(
                        "Restoring SCT error recovery read command timer to default is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to restore SCT error recovery read command timer to default!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_WRITE_SET_DEFAULT)
        {
            switch (sct_Restore_Command_Timer(&deviceList[deviceIter], SCT_ERC_WRITE_COMMAND))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully restore SCT error recovery write command timer to default!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Restoring SCT error recovery write command timer to default is not supported on this "
                           "device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to restore SCT error recovery write command timer to default!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_SET_READ_TIMER)
        {
            switch (sct_Set_Command_Timer(&deviceList[deviceIter], SCT_ERC_READ_COMMAND,
                                          SCT_ERROR_RECOVERY_CONTROL_READ_TIMER_VALUE, VOLATILE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set SCT error recovery read command timer!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting SCT error recovery read command timer is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set SCT error recovery read command timer!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
            if (VOLATILE_FLAG == false && exitCode != UTIL_EXIT_NO_ERROR)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Please think of using --%s flag as workaround\n", VOLATILE_LONG_OPT_STRING);
                }
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_SET_WRITE_TIMER)
        {
            switch (sct_Set_Command_Timer(&deviceList[deviceIter], SCT_ERC_WRITE_COMMAND,
                                          SCT_ERROR_RECOVERY_CONTROL_WRITE_TIMER_VALUE, VOLATILE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set SCT error recovery write command timer!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting SCT error recovery write command timer is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set SCT error recovery write command timer!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
            if (VOLATILE_FLAG == false && exitCode != UTIL_EXIT_NO_ERROR)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Please think of using --%s flag as workaround\n", VOLATILE_LONG_OPT_STRING);
                }
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_READ_INFO || SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO)
        {
            uint32_t minRcvTimeLmtMilliseconds = UINT32_C(0);
            switch (sct_Get_Min_Recovery_Time_Limit(&deviceList[deviceIter], &minRcvTimeLmtMilliseconds))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (minRcvTimeLmtMilliseconds > 0)
                    {
                        printf("SCT error recovery control timer minimum supported value is %" PRIu32 "ms\n",
                               minRcvTimeLmtMilliseconds);
                    }
                    else
                    {
                        printf("SCT error recovery control timer minimum supported value is not reported\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get SCT error recovery command timer minimum supported value!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_READ_INFO)
        {
            uint32_t timerValueMilliseconds = UINT32_C(0);
            switch (sct_Get_Command_Timer(&deviceList[deviceIter], SCT_ERC_READ_COMMAND, &timerValueMilliseconds, true))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control read timer is %" PRIu32 "ms (volatile)\n",
                           timerValueMilliseconds);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get SCT error recovery read command timer (volatile)!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
            if (exitCode == UTIL_EXIT_NO_ERROR)
            {
                switch (sct_Get_Command_Timer(&deviceList[deviceIter], SCT_ERC_READ_COMMAND, &timerValueMilliseconds,
                                              false))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("SCT error recovery control read timer is %" PRIu32 "ms (non-volatile)\n",
                               timerValueMilliseconds);
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("SCT error recovery control is not supported on this device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to get SCT error recovery read command timer (non-volatile)!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO)
        {
            uint32_t timerValueMilliseconds = UINT32_C(0);
            switch (
                sct_Get_Command_Timer(&deviceList[deviceIter], SCT_ERC_WRITE_COMMAND, &timerValueMilliseconds, true))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control write timer is %" PRIu32 "ms (volatile)\n",
                           timerValueMilliseconds);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get SCT error recovery write command timer (volatile)!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
            if (exitCode == UTIL_EXIT_NO_ERROR)
            {
                switch (sct_Get_Command_Timer(&deviceList[deviceIter], SCT_ERC_WRITE_COMMAND, &timerValueMilliseconds,
                                              false))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("SCT error recovery control write timer is %" PRIu32 "ms (non-volatile)\n",
                               timerValueMilliseconds);
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("SCT error recovery control is not supported on this device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to get SCT error recovery write command timer (non-volatile)!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (READ_LOOK_AHEAD_INFO)
        {
            if (is_Read_Look_Ahead_Enabled(&deviceList[deviceIter]))
            {
                printf("Read Look Ahead is Enabled.\n");
            }
            else
            {
                printf("Read Look Ahead is Disabled.\n");
            }
        }

        if (NV_CACHE_INFO)
        {
            if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
            {
                if (is_NV_Cache_Supported(&deviceList[deviceIter]))
                {
                    if (is_NV_Cache_Enabled(&deviceList[deviceIter]))
                    {
                        printf("Non-Volatile Cache is Enabled (Caching Mode Page NV_DIS Bit is set to 0)\n");
                    }
                    else
                    {
                        printf("Non-Volatile Cache is Disabled (Caching Mode Page NV_DIS Bit is set to 1)\n");
                    }
                }
                else
                {
                    printf("Non-Volatile Cache is not supported on this drive.\n");
                }
            }
            else
            {
                printf("Non-Volatile Cache info is not available on this drive.\n");
            }
        }

        if (NV_CACHE_FLAG)
        {
            switch (scsi_Set_NV_DIS(&deviceList[deviceIter], NV_CACHE_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (NV_CACHE_SETTING)
                    {
                        printf("Non-Volatile Cache successfully enabled!\n");
                    }
                    else
                    {
                        printf("Non-Volatile Cache successfully disabled!\n");
                    }
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (NV_CACHE_SETTING)
                    {
                        printf("Enabling Non-Volatile Cache not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling Non-Volatile Cache not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (NV_CACHE_SETTING)
                    {
                        printf("Failed to enable Non-Volatile Cache!\n");
                    }
                    else
                    {
                        printf("Failed to disable Non-Volatile Cache!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (READ_LOOK_AHEAD_FLAG)
        {
            switch (set_Read_Look_Ahead(&deviceList[deviceIter], READ_LOOK_AHEAD_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Read look-ahead successfully enabled!\n");
                    }
                    else
                    {
                        printf("Read look-ahead successfully disabled!\n");
                    }
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Enabling read look-ahead not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling read look-ahead not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Failed to enable read look-ahead!\n");
                    }
                    else
                    {
                        printf("Failed to disable read look-ahead!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (PROVISION_FLAG)
        {
            // this means we are performing a TRIM/UNMAP operation, then setting the maxlba. Since we've already set the
            // trim start location, time to set the range
            TRIM_UNMAP_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba - TRIM_UNMAP_START_FLAG;
            RUN_TRIM_UNMAP_FLAG   = true;
        }

        if (RUN_TRIM_UNMAP_FLAG)
        {
            uint64_t localStartLBA = TRIM_UNMAP_START_FLAG;
            uint64_t localRange    = TRIM_UNMAP_RANGE_FLAG;
            if (localStartLBA != UINT64_MAX)
            {
                if (localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                }
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG == UINT64_MAX)
                {
                    localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
                }
                if (POSSIBLE_DATA_ERASE_FLAG || DATA_ERASE_FLAG)
                {
                    switch (trim_Unmap_Range(&deviceList[deviceIter], localStartLBA, localRange))
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Successfully trimmed/unmapped LBAs %" PRIu64 " to %" PRIu64 "\n", localStartLBA,
                                   localStartLBA + localRange - 1);
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Trim/Unmap is not supported on this drive type.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to trim/unmap LBAs %" PRIu64 " to %" PRIu64 "\n", localStartLBA,
                                   localStartLBA + localRange - 1);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("\n");
                        printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                        printf("to the command line arguments to run a trim/unmap operation.\n\n");
                        printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample,
                               TRIM_LONG_OPT_STRING, POSSIBLE_DATA_ERASE_ACCEPT_STRING,
                               POSSIBLE_DATA_ERASE_ACCEPT_STRING);
                    }
                }
            }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An invalid start LBA has been entered. Please enter a valid value.\n");
                }
            }
        }

        if (SET_MAX_LBA_FLAG)
        {
            bool scsiAtaInSync = false;
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Setting MaxLBA to %" PRIu64 "\n", SET_MAX_LBA_VALUE);
            }
            switch (set_Max_LBA_2(&deviceList[deviceIter], SET_MAX_LBA_VALUE, false, CHANGE_ID_STRING_FLAG))
            {
            case SUCCESS:
                scsiAtaInSync = is_Max_LBA_In_Sync_With_Adapter_Or_Driver(&deviceList[deviceIter], false);
                fill_Drive_Info_Data(&deviceList[deviceIter]);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    double mCapacity = 0.0;
                    double capacity  = 0.0;
                    DECLARE_ZERO_INIT_ARRAY(char, mCapUnits, UNIT_STRING_LENGTH);
                    DECLARE_ZERO_INIT_ARRAY(char, capUnits, UNIT_STRING_LENGTH);
                    char* mCapUnit = &mCapUnits[0];
                    char* capUnit  = &capUnits[0];
                    if (deviceList[deviceIter].drive_info.bridge_info.isValid)
                    {
                        mCapacity = C_CAST(double, deviceList[deviceIter]
                                                       .drive_info.bridge_info.childDeviceMaxLba* deviceList[deviceIter]
                                                       .drive_info.bridge_info.childDeviceBlockSize);
                    }
                    else
                    {
                        mCapacity = C_CAST(double, deviceList[deviceIter]
                                                       .drive_info.deviceMaxLba* deviceList[deviceIter]
                                                       .drive_info.deviceBlockSize);
                    }
                    capacity = mCapacity;
                    metric_Unit_Convert(&mCapacity, &mCapUnit);
                    capacity_Unit_Convert(&capacity, &capUnit);
                    printf("Successfully set the max LBA to %" PRIu64 "\n", SET_MAX_LBA_VALUE);
                    printf("New Drive Capacity (%s/%s): %0.02f/%0.02f\n", mCapUnit, capUnit, mCapacity, capacity);
                    if (!scsiAtaInSync)
                    {
                        printf("\nWARNING: The adapter/driver/bridge is not in sync with the capacity change!\n");
                        printf("         A reboot is strongly recommended to make sure the system works without\n");
                        printf("         errors with the drive at its new capacity.\n\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting the max LBA is not supported by this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the max LBA!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (RESTORE_MAX_LBA_FLAG)
        {
            bool scsiAtaInSync = false;
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Restoring max LBA\n");
            }
            switch (set_Max_LBA_2(&deviceList[deviceIter], 0, true, CHANGE_ID_STRING_FLAG))
            {
            case SUCCESS:
                scsiAtaInSync = is_Max_LBA_In_Sync_With_Adapter_Or_Driver(&deviceList[deviceIter], false);
                fill_Drive_Info_Data(&deviceList[deviceIter]);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    double mCapacity = 0.0;
                    double capacity  = 0.0;
                    DECLARE_ZERO_INIT_ARRAY(char, mCapUnits, UNIT_STRING_LENGTH);
                    DECLARE_ZERO_INIT_ARRAY(char, capUnits, UNIT_STRING_LENGTH);
                    char* mCapUnit = &mCapUnits[0];
                    char* capUnit  = &capUnits[0];
                    if (deviceList[deviceIter].drive_info.bridge_info.isValid)
                    {
                        mCapacity = C_CAST(double, deviceList[deviceIter]
                                                       .drive_info.bridge_info.childDeviceMaxLba* deviceList[deviceIter]
                                                       .drive_info.bridge_info.childDeviceBlockSize);
                    }
                    else
                    {
                        mCapacity = C_CAST(double, deviceList[deviceIter]
                                                       .drive_info.deviceMaxLba* deviceList[deviceIter]
                                                       .drive_info.deviceBlockSize);
                    }
                    capacity = mCapacity;
                    metric_Unit_Convert(&mCapacity, &mCapUnit);
                    capacity_Unit_Convert(&capacity, &capUnit);
                    printf("Successfully restored the max LBA\n");
                    printf("New Drive Capacity (%s/%s): %0.02f/%0.02f\n", mCapUnit, capUnit, mCapacity, capacity);
                    if (!scsiAtaInSync)
                    {
                        printf("\nWARNING: The adapter/driver/bridge is not in sync with the capacity change!\n");
                        printf("         A reboot is strongly recommended to make sure the system works without\n");
                        printf("         errors with the drive at its new capacity.\n\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Restoring the max LBA is not supported by this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to restore the max LBA!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (LOW_CURRENT_SPINUP_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Set Low Current Spinup\n");
            }
            bool sctMethodSupported = is_SCT_Low_Current_Spinup_Supported(&deviceList[deviceIter]);
            switch (set_Low_Current_Spin_Up(&deviceList[deviceIter], sctMethodSupported, LOW_CURRENT_SPINUP_STATE))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully ");
                    if (LOW_CURRENT_SPINUP_STATE == 1)
                    {
                        printf("Enabled");
                    }
                    else if (LOW_CURRENT_SPINUP_STATE == 3)
                    {
                        printf("Enabled Ultra");
                    }
                    else
                    {
                        printf("Disabled");
                    }
                    printf(" Low Current Spinup!\nA power cycle is required to complete this change.\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (LOW_CURRENT_SPINUP_STATE == 3)
                    {
                        printf("Setting Ultra Low Current Spinup not supported on this device\n");
                    }
                    else
                    {
                        printf("Setting Low Current Spinup not supported on this device\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the Low Current Spinup feature!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (PUIS_FEATURE_FLAG)
        {
            printf("\nPlease switch use of PUIS options to openSeaChest_PowerControl.\n");
            printf("These options will be removed from openSeaChest_Configure in a future release.\n");
            puisInfo info;
            safe_memset(&info, sizeof(puisInfo), 0, sizeof(puisInfo));
            eReturnValues puisInfoRet = get_PUIS_Info(&deviceList[deviceIter], &info);
            if (PUIS_FEATURE_SPINUP_FLAG)
            {
                if (info.puisEnabled)
                {
                    switch (puis_Spinup(&deviceList[deviceIter]))
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("\nSuccessfully performed PUIS spinup command\n");
                            printf("\nHint:Use --checkPowerMode option to check the new Power State.\n\n");
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("PUIS spinup command is not supported on this device.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to perform the PUIS spinup command\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    if (info.puisSupported)
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("PUIS feature is not enabled. Nothing to do.\n");
                        }
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("PUIS spinup command is not supported on this device.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    }
                }
            }
            if (PUIS_FEATURE_INFO_FLAG)
            {
                switch (puisInfoRet)
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("===PUIS Info===\n");
                        if (info.puisSupported)
                        {
                            if (info.puisEnabled)
                            {
                                printf("\tPUIS is supported and enabled\n");
                            }
                            else
                            {
                                printf("\tPUIS is supported\n");
                            }
                            if (info.spinupCommandRequired)
                            {
                                printf("\tSpin-Up command is required for medium access.\n");
                            }
                            else
                            {
                                printf("\tAutomatic spin-up as needed for medium access.\n");
                            }
                        }
                        else
                        {
                            printf("\tPUIS is not supported on this device.\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("PUIS is not available on this device type!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to get PUIS info from this device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            if (PUIS_STATE_VALID_FLAG)
            {
                switch (enable_Disable_PUIS_Feature(&deviceList[deviceIter], PUIS_FEATURE_STATE_FLAG))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (PUIS_FEATURE_STATE_FLAG)
                        {
                            printf("PUIS feature successfully enabled!\n");
                        }
                        else
                        {
                            printf("PUIS feature successfully disabled!\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (PUIS_FEATURE_STATE_FLAG)
                        {
                            printf("Enabling PUIS feature not supported on this device.\n");
                        }
                        else
                        {
                            printf("Disabling PUIS feature not supported on this device.\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (PUIS_FEATURE_STATE_FLAG)
                        {
                            printf("Failed to enable PUIS feature!\n");
                        }
                        else
                        {
                            printf("Failed to disable PUIS feature!\n");
                            printf("If PUIS is enabled with a jumper, it cannot be disabled with this command!\n");
                            printf("Remove the PUIS jumper to disable the feature in this case.\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (FREE_FALL_FLAG)
        {
            if (FREE_FALL_DISABLE)
            {
                switch (disable_Free_Fall_Control_Feature(&deviceList[deviceIter]))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Free Fall Control feature successfully disabled!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Disabling Free Fall Control feature not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to disable Free Fall Control feature!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                switch (set_Free_Fall_Control_Sensitivity(&deviceList[deviceIter], FREE_FALL_SENSITIVITY))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (FREE_FALL_SENSITIVITY == 0)
                        {
                            printf("Free Fall Control feature successfully set to vendor's recommended value!\n");
                        }
                        else
                        {
                            printf("Free Fall Control feature successfully set to %" PRIu8 "!\n",
                                   FREE_FALL_SENSITIVITY);
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Setting Free Fall Control sensitivity not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to set Free Fall Control sensitivity!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (FREE_FALL_INFO)
        {
            uint16_t sensitivity = UINT16_C(0);
            switch (get_Current_Free_Fall_Control_Sensitivity(&deviceList[deviceIter], &sensitivity))
            {
            case SUCCESS:
                if (sensitivity == UINT16_MAX)
                {
                    printf("Free Fall control feature is supported, but not enabled.\n");
                }
                else if (sensitivity == 0)
                {
                    printf("Free Fall control sensitivity is set to the vendor's recommended value.\n");
                }
                else
                {
                    printf("Free Fall control sensitivity is set to %" PRIu16 " of 255.\n", sensitivity);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Free Fall control feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get Free Fall control information.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (SCSI_MP_RESET_OP)
        {
            switch (scsi_Update_Mode_Page(&deviceList[deviceIter], SCSI_MP_RESET_PAGE_NUMBER,
                                          SCSI_MP_RESET_SUBPAGE_NUMBER, UPDATE_SCSI_MP_RESET_TO_DEFAULT))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully reset mode page!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf(
                        "Mode page not supported or resetting mode page to defaults not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying reset the specified mode page\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_MP_RESTORE_OP)
        {
            switch (scsi_Update_Mode_Page(&deviceList[deviceIter], SCSI_MP_RESTORE_PAGE_NUMBER,
                                          SCSI_MP_RESTORE_SUBPAGE_NUMBER, UPDATE_SCSI_MP_RESTORE_TO_SAVED))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully restored mode page to saved values!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Mode page not supported or restoring mode page to saved values not supported on this "
                           "device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying restore the specified mode page to its saved values\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_MP_SAVE_OP)
        {
            switch (scsi_Update_Mode_Page(&deviceList[deviceIter], SCSI_MP_SAVE_PAGE_NUMBER,
                                          SCSI_MP_SAVE_SUBPAGE_NUMBER, UPDATE_SCSI_MP_SAVE_CURRENT))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully saved mode page!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Mode page not supported or saving mode page not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying save the specified mode page\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_RESET_LP_OP)
        {
            eReturnValues resetLPResult = SUCCESS;
            if (SCSI_RESET_LP_LPC > LPC_DEFAULT_CUMULATIVE_VALUES)
            {
                // requesting to reset all
                for (SCSI_RESET_LP_LPC = LPC_THRESHOLD_VALUES; SCSI_RESET_LP_LPC <= LPC_DEFAULT_CUMULATIVE_VALUES;
                     ++SCSI_RESET_LP_LPC)
                {
                    eReturnValues resetLPCommandRet =
                        reset_SCSI_Log_Page(&deviceList[deviceIter], C_CAST(eScsiLogPageControl, SCSI_RESET_LP_LPC),
                                            SCSI_RESET_LP_PAGE_NUMBER, SCSI_RESET_LP_SUBPAGE_NUMBER, !VOLATILE_FLAG);
                    if (SUCCESS != resetLPCommandRet) // this is to catch if any LPC reset value creates an error
                    {
                        resetLPResult = resetLPCommandRet;
                    }
                }
            }
            else
            {
                // reset just the specified information
                resetLPResult =
                    reset_SCSI_Log_Page(&deviceList[deviceIter], C_CAST(eScsiLogPageControl, SCSI_RESET_LP_LPC),
                                        SCSI_RESET_LP_PAGE_NUMBER, SCSI_RESET_LP_SUBPAGE_NUMBER, !VOLATILE_FLAG);
            }
            switch (resetLPResult)
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully reset the log page!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Log page reset not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case BAD_PARAMETER:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Resetting a specific log page is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying reset the log page\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_SET_MP_OP)
        {
            if (safe_strlen(SCSI_SET_MP_FILENAME) > 0) // file was given to be used to set the MP
            {
                // need to open the file that was passed, and convert it to an array.
                // skip all newlines, spaces, underscores, dashes, slashes, etc. We should only be finding hex bytes
                // inside this buffer (no H's or 0x's either)
                secureFileInfo* modePageFile =
                    secure_Open_File(SCSI_SET_MP_FILENAME, "r", M_NULLPTR, M_NULLPTR, M_NULLPTR);
                if (modePageFile && modePageFile->error == SEC_FILE_SUCCESS)
                {
                    // first, figure out the length of the file...this will be useful to help us allocate a big enough
                    // buffer for the data
                    if (modePageFile->fileSize > 0)
                    {
                        size_t fileLength = modePageFile->fileSize +
                                            1; // add 1 so that we have a null terminator once we read in the file.
                        uint8_t* modePageBuffer = C_CAST(
                            uint8_t*,
                            safe_calloc_aligned(
                                fileLength, sizeof(uint8_t),
                                deviceList[deviceIter]
                                    .os_info.minimumAlignment)); // this will allocate more than enough memory for us to
                                                                 // read the file...it's extra and that's ok.
                        char* fileBuf = M_REINTERPRET_CAST(char*, safe_calloc(fileLength, sizeof(char)));
                        if (modePageBuffer && fileBuf)
                        {
                            // read the file
                            size_t modeBytesRead = SIZE_T_C(0);
                            if (secure_Read_File(modePageFile, fileBuf, fileLength, sizeof(char), fileLength - 1,
                                                 &modeBytesRead) == SEC_FILE_SUCCESS &&
                                modeBytesRead == fileLength - 1)
                            {
                                // parse the file
                                rsize_t     filebuflen = safe_strlen(fileBuf);
                                char*       saveptr    = M_NULLPTR;
                                const char* delimiters = " \n\r-_\\/|\t:;";
                                char*       token      = safe_String_Token(fileBuf, &filebuflen, delimiters,
                                                                           &saveptr); // add more to the delimiter list as needed
                                if (token)
                                {
                                    bool     invalidCharacterOrMissingSeparator = false;
                                    uint16_t modeBufferElementCount             = UINT16_C(0);
                                    do
                                    {
                                        if (safe_strlen(token) > 2)
                                        {
                                            invalidCharacterOrMissingSeparator = true;
                                            break;
                                        }
                                        if (strpbrk(
                                                token,
                                                "ghijklmnopqrstuvwxyzGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()+={[}]\"'<>?,.`~"))
                                        {
                                            invalidCharacterOrMissingSeparator = true;
                                            break;
                                        }
                                        // not an invalid character or a missing separator, so convert the string to an
                                        // array value.
                                        unsigned long temp = 0UL;
                                        if (0 == safe_strtoul(&temp, token, M_NULLPTR, BASE_16_HEX))
                                        {
                                            modePageBuffer[modeBufferElementCount] = C_CAST(uint8_t, temp);
                                        }
                                        else
                                        {
                                            invalidCharacterOrMissingSeparator = true;
                                            break;
                                        }
                                        ++modeBufferElementCount;
                                        token = safe_String_Token(M_NULLPTR, &filebuflen, delimiters, &saveptr);
                                    } while (token);
                                    if (!invalidCharacterOrMissingSeparator)
                                    {
                                        // file is read, send the change
                                        switch (scsi_Set_Mode_Page(&deviceList[deviceIter], modePageBuffer,
                                                                   modeBufferElementCount, !VOLATILE_FLAG))
                                        {
                                        case SUCCESS:
                                            if (VERBOSITY_QUIET < toolVerbosity)
                                            {
                                                printf("Successfully set SCSI mode page!\n");
                                                if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                                {
                                                    printf("NOTE: This command may have affected more than 1 logical "
                                                           "unit\n");
                                                }
                                            }
                                            break;
                                        case NOT_SUPPORTED:
                                            if (VERBOSITY_QUIET < toolVerbosity)
                                            {
                                                printf("Unable to change the requested values in the mode page. These "
                                                       "may not be changable or are an invalid combination.\n");
                                            }
                                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                                            break;
                                        default:
                                            if (VERBOSITY_QUIET < toolVerbosity)
                                            {
                                                printf("Failed to set the mode page changes that were requested.\n");
                                            }
                                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("An error occurred while trying to parse the file. Please check the "
                                                   "file format and make sure no invalid characters are provided.\n");
                                        }
                                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                    }
                                }
                                else
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("An error occurred while trying to parse the file. Please check the "
                                               "file format.\n");
                                    }
                                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                }
                            }
                            else
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Error reading contents of mode page file!\n");
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            }
                            safe_free_aligned(&modePageBuffer);
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Unable to allocate memory to read the file. Cannot set the mode page.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                        safe_free_aligned(&modePageBuffer);
                        safe_free(&fileBuf);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Unable to allocate memory to read the file. Cannot set the mode page.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    if (SEC_FILE_SUCCESS != secure_Close_File(modePageFile))
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            perror("Fatal error while closing mode page file\n");
                        }
                    }
                }
                else
                {
                    if (modePageFile->error == SEC_FILE_INSECURE_PATH)
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            print_Insecure_Path_Utility_Message();
                        }
                        exitCode = UTIL_EXIT_INSECURE_PATH;
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Unable to read the file with the mode page data. Cannot set the mode page.\n");
                        }
                        exitCode = UTIL_EXIT_CANNOT_OPEN_FILE;
                    }
                }
                free_Secure_File_Info(&modePageFile);
            }
            else
            {
                // inputs were parsed, now we need to turn the inputs into what the user wants to change.
                // first, figure out how large the mode page is, then read all it's bytes...then start going through and
                // changing things as the user requests (the hard part)
                uint32_t rawModePageSize = UINT32_C(0);
                if (SUCCESS == get_SCSI_Mode_Page_Size(&deviceList[deviceIter], MPC_CURRENT_VALUES,
                                                       SCSI_SET_MP_PAGE_NUMBER, SCSI_SET_MP_SUBPAGE_NUMBER,
                                                       &rawModePageSize))
                {
                    uint8_t* rawmodePageBuffer =
                        M_REINTERPRET_CAST(uint8_t*, safe_calloc(rawModePageSize, sizeof(uint8_t)));
                    if (rawmodePageBuffer)
                    {
                        bool usedSizeByteCmd = false;
                        if (SUCCESS == get_SCSI_Mode_Page(&deviceList[deviceIter], MPC_CURRENT_VALUES,
                                                          SCSI_SET_MP_PAGE_NUMBER, SCSI_SET_MP_SUBPAGE_NUMBER,
                                                          M_NULLPTR, M_NULLPTR, true, rawmodePageBuffer,
                                                          rawModePageSize, M_NULLPTR, &usedSizeByteCmd))
                        {
                            uint32_t modeHeaderLen =
                                usedSizeByteCmd ? MODE_PARAMETER_HEADER_6_LEN : MODE_PARAMETER_HEADER_10_LEN;
                            uint32_t blockDescriptorLength =
                                usedSizeByteCmd ? rawmodePageBuffer[2]
                                                : M_BytesTo2ByteValue(rawmodePageBuffer[6], rawmodePageBuffer[7]);
                            uint32_t dataLengthAdjustment = modeHeaderLen + blockDescriptorLength;
                            uint32_t modePageSize         = rawModePageSize - dataLengthAdjustment;
                            uint8_t* modePageBuffer       = rawmodePageBuffer + dataLengthAdjustment;
                            // TODO: some of this code can probably be simplified a bit more than it currently is.
                            if (blockDescriptorLength > 0)
                            {
                                // Before going too far, clear the block descriptor to zeroes to make sure we are
                                // clearly communicating that no changes are requested.
                                safe_memset(&rawmodePageBuffer[modeHeaderLen], rawModePageSize - modeHeaderLen, 0,
                                            blockDescriptorLength);
                            }
                            // now we have the data, we can begin modifying the field requested.
                            if (SCSI_SET_MP_FIELD_LEN_BITS % BITSPERBYTE)
                            {
                                // not a multi-bye aligned field. Ex: 12 bits or 3 bits, etc
                                // check if the number of bits is greater than a byte or not and if the starting bit anf
                                // field width would break across byte boundaries
                                if (SCSI_SET_MP_FIELD_LEN_BITS > BITSPERBYTE ||
                                    (C_CAST(int, SCSI_SET_MP_BIT) - C_CAST(int, SCSI_SET_MP_FIELD_LEN_BITS)) < 0)
                                {
                                    // setting bits within multiple bytes, but not necessarily full bytes!
                                    uint8_t remainingBits = SCSI_SET_MP_FIELD_LEN_BITS;
                                    uint8_t highUnalignedBits =
                                        UINT8_C(0);                        // always least significant bits in this byte
                                    uint8_t lowUnalignedBits = UINT8_C(0); // always most significant bits in this byte
                                    if (SCSI_SET_MP_BIT != 7)
                                    {
                                        highUnalignedBits = SCSI_SET_MP_BIT + 1;
                                        remainingBits -= highUnalignedBits;
                                    }
                                    // check how many full bytes worth of bits we'll be setting.
                                    uint8_t fullBytesToSet = remainingBits / BITSPERBYTE;
                                    remainingBits -= C_CAST(uint8_t, fullBytesToSet * BITSPERBYTE);
                                    lowUnalignedBits = remainingBits;
                                    // now we know how we need to set things, so lets start at the end (lsb) and work up
                                    // from there. as we set the necessary bits, we will shift the original value to the
                                    // right to make it easy to set each piece of the bits.
                                    remainingBits = SCSI_SET_MP_FIELD_LEN_BITS; // resetting this to help keep track as
                                                                                // we shift through the bits.
                                    uint16_t offset = SCSI_SET_MP_BYTE + fullBytesToSet;
                                    if (lowUnalignedBits > 0)
                                    {
                                        ++offset; // add one to the offset since these bits are on another byte past the
                                                  // starting offset and any full bytes we need to set
                                        // need to create a mask and take the lowest bits that we need and place then in
                                        // this byte starting at bit 7
                                        uint8_t mask = get_bit_range_uint8(UINT8_MAX, 7, 7 - (lowUnalignedBits - 1))
                                                       << (7 - lowUnalignedBits + 1);
                                        // clear the requested bits first
                                        modePageBuffer[offset] &= C_CAST(uint8_t, ~(mask));
                                        // now set them as requested
                                        modePageBuffer[offset] |= C_CAST(
                                            uint8_t, (mask & (SCSI_SET_MP_FIELD_VALUE << (7 - lowUnalignedBits + 1))));
                                        // bits are set, decrease the offset for the next operation
                                        --offset;
                                        SCSI_SET_MP_FIELD_VALUE >>= lowUnalignedBits;
                                    }
                                    if (fullBytesToSet > 0)
                                    {
                                        for (uint8_t byteCnt = UINT8_C(0); byteCnt < fullBytesToSet;
                                             ++byteCnt, SCSI_SET_MP_FIELD_VALUE >>= BITSPERBYTE, --offset)
                                        {
                                            modePageBuffer[offset] = M_Byte0(SCSI_SET_MP_FIELD_VALUE);
                                        }
                                    }
                                    if (highUnalignedBits > 0)
                                    {
                                        // need to create a mask and take the highest bits (only ones remaining at this
                                        // point) that we need and place then in this byte starting at bit 0
                                        uint8_t mask =
                                            get_bit_range_uint8(UINT8_MAX, (highUnalignedBits - 1),
                                                                (highUnalignedBits - 1) - (highUnalignedBits - 1))
                                            << ((highUnalignedBits - 1) - highUnalignedBits + 1);
                                        // clear the requested bits first
                                        modePageBuffer[SCSI_SET_MP_BYTE] &= C_CAST(uint8_t, ~(mask));
                                        // now set them as requested
                                        modePageBuffer[SCSI_SET_MP_BYTE] |= C_CAST(
                                            uint8_t, (mask & (SCSI_SET_MP_FIELD_VALUE
                                                              << ((highUnalignedBits - 1) - highUnalignedBits + 1))));
                                    }
                                }
                                else
                                {
                                    // setting bits within a single byte.
                                    uint8_t mask =
                                        get_bit_range_uint8(UINT8_MAX, SCSI_SET_MP_BIT,
                                                            SCSI_SET_MP_BIT - (SCSI_SET_MP_FIELD_LEN_BITS - 1))
                                        << (SCSI_SET_MP_BIT - SCSI_SET_MP_FIELD_LEN_BITS + 1);
                                    // clear the requested bits first
                                    modePageBuffer[SCSI_SET_MP_BYTE] &= C_CAST(uint8_t, ~(mask));
                                    // now set them as requested
                                    modePageBuffer[SCSI_SET_MP_BYTE] |= C_CAST(
                                        uint8_t, (mask & (SCSI_SET_MP_FIELD_VALUE
                                                          << (SCSI_SET_MP_BIT - SCSI_SET_MP_FIELD_LEN_BITS + 1))));
                                }
                            }
                            else
                            {
                                // set full bytes to the value requested.
                                uint16_t fieldWidthBytes = SCSI_SET_MP_FIELD_LEN_BITS / BITSPERBYTE;
                                uint8_t  byteNumber      = UINT8_C(0);
                                while (fieldWidthBytes >= 1)
                                {
                                    modePageBuffer[SCSI_SET_MP_BYTE + (fieldWidthBytes - 1)] = C_CAST(
                                        uint8_t, (C_CAST(uint64_t, M_ByteN(byteNumber)) & SCSI_SET_MP_FIELD_VALUE) >>
                                                     (BITSPERBYTE * byteNumber));
                                    --fieldWidthBytes;
                                    ++byteNumber;
                                }
                            }
                            // buffer is ready to send to the drive!
                            switch (scsi_Set_Mode_Page(&deviceList[deviceIter], modePageBuffer,
                                                       C_CAST(uint16_t, modePageSize), !VOLATILE_FLAG))
                            {
                            case SUCCESS:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Successfully set SCSI mode page!\n");
                                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                    {
                                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                                    }
                                }
                                break;
                            case NOT_SUPPORTED:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Unable to change the requested values in the mode page. These may not be "
                                           "changeable or are an invalid combination.\n");
                                }
                                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                                break;
                            default:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Failed to set the mode page changes that were requested.\n");
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                break;
                            }
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Unable to read the requested mode page...it may not be supported.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        }
                        safe_free(&rawmodePageBuffer);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Unable to allocate memory to modify mode page.\n");
                        }
                        exitCode = UTIL_EXIT_NOT_ENOUGH_RESOURCES;
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Unable to determine length of the requested mode page...it may not be supported.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
            }
        }

        if (WRV_INFO)
        {
            wrvInfo info;
            safe_memset(&info, sizeof(wrvInfo), 0, sizeof(wrvInfo));
            switch (get_Write_Read_Verify_Info(&deviceList[deviceIter], &info))
            {
            case SUCCESS:
                print_Write_Read_Verify_Info(&info);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Write-Read-Verify feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get Write-Read-Verify feature infomation.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (WRV_FLAG)
        {
            if (WRV_DISABLE)
            {
                switch (disable_Write_Read_Verify(&deviceList[deviceIter]))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Successfully disabled Write-Read-Verify feature\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Write-Read-Verify feature is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to disable Write-Read-Verify feature.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                switch (set_Write_Read_Verify(&deviceList[deviceIter], WRV_ALL, WRV_VENDOR, WRV_USER_VALUE))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Successfully enabled Write-Read-Verify feature\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Write-Read-Verify feature is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to enable Write-Read-Verify feature.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }
        // At this point, close the device handle since it is no longer needed. Do not put any further IO below this.
        close_Device(&deviceList[deviceIter]);
    }
    free_device_list(&DEVICE_LIST);
    if (getDevsRet != SUCCESS && skippedDevices == DEVICE_LIST_COUNT)
    {
        switch (getDevsRet)
        {
        case WARN_NOT_ALL_DEVICES_ENUMERATED:
            // Different exit code needed? Not entirely sure if this is the best choice here - TJE
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            break;
        case PERMISSION_DENIED:
            exitCode = UTIL_EXIT_NEED_ELEVATED_PRIVILEGES;
            break;
        default:
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            break;
        }
    }
    else if (skippedDevices == DEVICE_LIST_COUNT)
    {
        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
    }
    exit(exitCode);
}

//-----------------------------------------------------------------------------
//
//  Utility_usage()
//
//! \brief   Description:  Dump the utility usage information
//
//  Entry:
//!   \param NONE
//!
//  Exit:
//!   \return VOID
//
//-----------------------------------------------------------------------------
void utility_Usage(bool shortUsage)
{
    // everything needs a help option right?
    printf("Usage\n");
    printf("=====\n");
    printf("\t %s [-d %s] {arguments} {options}\n\n", util_name, deviceHandleName);

    printf("\nExamples\n");
    printf("========\n");
    // example usage
    printf("\t%s --%s\n", util_name, SCAN_LONG_OPT_STRING);
    printf("\t%s -d %s -%c\n", util_name, deviceHandleExample, DEVICE_INFO_SHORT_OPT);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SAT_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LOWLEVEL_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 2\n", util_name, deviceHandleExample, SET_PHY_SPEED_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 3 --%s 1\n", util_name, deviceHandleExample, SET_PHY_SPEED_LONG_OPT_STRING,
           SET_PHY_SAS_PHY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enable\n", util_name, deviceHandleExample, READ_LOOK_AHEAD_LONG_OPT_STRING);
    printf("\t%s -d %s --%s info\n", util_name, deviceHandleExample, NV_CACHE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s disable\n", util_name, deviceHandleExample, WRITE_CACHE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s default\n", util_name, deviceHandleExample, SCT_WRITE_CACHE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enable\n", util_name, deviceHandleExample, SCT_WRITE_CACHE_REORDER_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0\n", util_name, deviceHandleExample, FREE_FALL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s low\n", util_name, deviceHandleExample, LOW_CURRENT_SPINUP_LONG_OPT_STRING);
    printf("\t%s -d %s --%s disable\n", util_name, deviceHandleExample, PUIS_FEATURE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enable\n", util_name, deviceHandleExample, SSC_FEATURE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s info\n", util_name, deviceHandleExample, SET_READY_LED_LONG_OPT_STRING);
    printf("\t%s -d %s --%s on\n", util_name, deviceHandleExample, SET_READY_LED_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 5s\n", util_name, deviceHandleExample, SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0 --%s\n", util_name, deviceHandleExample, SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING,
           VOLATILE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s all --%s 06h\n", util_name, deviceHandleExample, SCSI_RESET_LP_LONG_OPT_STRING,
           SCSI_RESET_LP_PAGE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s cumulative --%s 02h --%s\n", util_name, deviceHandleExample, SCSI_RESET_LP_LONG_OPT_STRING,
           SCSI_RESET_LP_PAGE_LONG_OPT_STRING, VOLATILE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0Ah\n", util_name, deviceHandleExample, SCSI_SHOW_MP_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0Ah --%s saved\n", util_name, deviceHandleExample, SCSI_SHOW_MP_LONG_OPT_STRING,
           SCSI_SHOW_MP_MPC_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0Ah --%s classic\n", util_name, deviceHandleExample, SCSI_SHOW_MP_LONG_OPT_STRING,
           SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 3Fh-FFh\n", util_name, deviceHandleExample, SCSI_MP_RESET_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 3Fh-FFh\n", util_name, deviceHandleExample, SCSI_MP_SAVE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 3Fh-FFh\n", util_name, deviceHandleExample, SCSI_MP_RESTORE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 08:2:2:1=0\n", util_name, deviceHandleExample, SCSI_SET_MP_LONG_OPT_STRING);
    printf("\t%s -d %s --%s file=modePageToChange.txt\n", util_name, deviceHandleExample, SCSI_SET_MP_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 134217728\n", util_name, deviceHandleExample, PROVISION_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 134217728 --%s\n", util_name, deviceHandleExample, SET_MAX_LBA_LONG_OPT_STRING,
           CHANGE_ID_STRING_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, CAPACITY_MODEL_NUMBER_MAPPING_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, ATA_DCO_IDENTIFY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, ATA_DCO_RESTORE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, ATA_DCO_FREEZE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 134217728 --%s %s --%s hpa,puis,wrv\n", util_name, deviceHandleExample,
           ATA_DCO_SETMAXLBA_LONG_OPT_STRING, ATA_DCO_SETMAXMODE_LONG_OPT_STRING, ATA_DCO_MODE_UDMA4,
           ATA_DCO_DISABLE_FEEATURES_LONG_OPT_STRING);
    // return codes
    printf("\nReturn codes\n");
    printf("============\n");
    print_SeaChest_Util_Exit_Codes(0, M_NULLPTR, util_name);

    // utility options - alphabetized
    printf("\nUtility Options\n");
    printf("===============\n");
#if defined(ENABLE_CSMI)
    print_CSMI_Force_Flags_Help(shortUsage);
    print_CSMI_Verbose_Help(shortUsage);
#endif
    print_Echo_Command_Line_Help(shortUsage);
    print_Enable_Legacy_USB_Passthrough_Help(shortUsage);
    print_Force_ATA_Help(shortUsage);
    print_Force_ATA_DMA_Help(shortUsage);
    print_Force_ATA_PIO_Help(shortUsage);
    print_Force_ATA_UDMA_Help(shortUsage);
    print_Force_SCSI_Help(shortUsage);
    print_Help_Help(shortUsage);
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_No_Banner_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    // the test options
    printf("\nUtility Arguments\n");
    printf("=================\n");
    // Common (across utilities) - alphabetized
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Low_Level_Info_Help(shortUsage);
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    print_Fast_Discovery_Help(shortUsage);
    // utility tests/operations go here - alphabetized
    print_Capacity_Model_Number_Mapping_Help(shortUsage);
    print_Change_Id_String_Help(shortUsage);
    print_Phy_Speed_Help(shortUsage);
    print_Read_Look_Ahead_Help(shortUsage);
    print_Restore_Max_LBA_Help(shortUsage);
    print_Set_Max_LBA_Help(shortUsage);
    print_Set_Timestamp_Help(shortUsage);
    print_Write_Cache_Help(shortUsage);

    // SATA Only Options
    printf("\n\tSATA Only:\n\t========\n");
    print_DCO_FreezeLock_Help(shortUsage);
    print_DCO_Identify_Help(shortUsage);
    print_DCO_Restore_Help(shortUsage);
    print_DCO_Set_Max_LBA_Help(shortUsage);
    print_DCO_Set_Max_Mode_Help(shortUsage);
    print_DCO_Disable_Features_Help(shortUsage);
    print_Free_Fall_Help(shortUsage);
    print_Low_Current_Spinup_Help(shortUsage);
    print_PUIS_Feature_Help(shortUsage);
    print_Set_SSC_Help(shortUsage);
    print_SCT_Error_Recovery_Read_Help(shortUsage);
    print_SCT_Write_Cache_Help(shortUsage);
    print_SCT_Write_Cache_Reordering_Help(shortUsage);
    print_SCT_Error_Recovery_Write_Help(shortUsage);
    print_WRV_Help(shortUsage);

    // SAS Only Options
    printf("\n\tSAS Only:\n\t========\n");
    print_NV_Cache_Bit_Help(shortUsage);
    print_Set_Ready_LED_Help(shortUsage);
    print_SAS_Phy_Help(shortUsage);
    print_SCSI_Reset_LP_Help(shortUsage);
    print_SCSI_Reset_LP_Page_Help(shortUsage);
    print_SCSI_MP_Reset_Help(shortUsage);
    print_SCSI_MP_Restore_Help(shortUsage);
    print_SCSI_MP_Save_Help(shortUsage);
    print_Set_SCSI_MP_Help(shortUsage);
    print_Show_SCSI_MP_Output_Mode_Help(shortUsage);
    print_SCSI_Show_MP_Help(shortUsage);
    print_SCSI_Show_MP_Control_Help(shortUsage);

    // data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    // utility data destructive tests/operations go here
    print_Provision_Help(shortUsage);
}
