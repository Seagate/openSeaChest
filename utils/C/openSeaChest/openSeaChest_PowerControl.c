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
// \file openSeaChest_PowerControl.c Binary command line that performs various power control methods on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "io_utils.h"
#include "memory_safety.h"
#include "string_utils.h"
#include "time_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"

#include "EULA.h"
#include "drive_info.h"
#include "getopt.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "power_control.h"
#include "seagate_operations.h"                  //for power telemetry
#include "vendor/seagate/seagate_common_types.h" //power telemetry type
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_PowerControl";
const char *buildVersion = "3.7.1";

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
    SAT_INFO_VAR
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
    // utility specific functions
    CHECK_POWER_VAR
    SPIN_DOWN_VAR
    EPC_ENABLED_VAR
    TRANSITION_POWER_MODE_VARS
    TRANSITION_POWER_STATE_VAR
    SET_POWER_CONSUMPTION_VARS
    SHOW_POWER_CONSUMPTION_VAR
    SET_APM_LEVEL_VARS
    SHOW_APM_LEVEL_VARS
    SHOW_EPC_SETTINGS_VAR
    DISABLE_APM_VAR
    SEAGATE_POWER_BALANCE_VARS
    SATA_DIPM_VARS
    SATA_DAPS_VARS
    SAS_PARTIAL_VARS
    SAS_SLUMBER_VARS
    SET_PHY_SAS_PHY_IDENTIFIER_VAR
    IDLE_A_POWER_MODE_VARS
    IDLE_B_POWER_MODE_VARS
    IDLE_C_POWER_MODE_VARS
    STANDBY_Z_POWER_MODE_VARS
    STANDBY_Y_POWER_MODE_VARS
    LEGACY_IDLE_POWER_MODE_VARS
    LEGACY_STANDBY_POWER_MODE_VARS
    SHOW_POWER_TELEMETRY_VAR
    REQUEST_POWER_TELEMETRY_MEASUREMENT_VARS
    SHOW_NVM_POWER_STATES_VAR
    PUIS_FEATURE_VARS

#if defined(ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    LOWLEVEL_INFO_VAR
    VOLATILE_VAR

    int args        = 0;
    int argIndex    = 0;
    int optionIndex = 0;

    struct option longopts[] =
    {
        // common command line options
        DEVICE_LONG_OPT,
        HELP_LONG_OPT,
        DEVICE_INFO_LONG_OPT,
        SAT_INFO_LONG_OPT,
        
        SCAN_LONG_OPT,
        NO_BANNER_OPT,
        AGRESSIVE_SCAN_LONG_OPT,
        SCAN_FLAGS_LONG_OPT,
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
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined(ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        LOWLEVEL_INFO_LONG_OPT,
        // tool specific options go here
        CHECK_POWER_LONG_OPT,
        SHOW_NVM_POWER_STATES_LONG_OPT,
        SPIN_DOWN_LONG_OPT,
        EPC_ENABLED_LONG_OPT,
        TRANSITION_POWER_MODE_LONG_OPT,
        SHOW_POWER_CONSUMPTION_LONG_OPT,
        SET_POWER_CONSUMPTION_LONG_OPT,
        SET_APM_LEVEL_LONG_OPT,
        SHOW_APM_LEVEL_LONG_OPT,
        SHOW_EPC_SETTINGS_LONG_OPT,
        DISABLE_APM_LONG_OPT,
        SEAGATE_POWER_BALANCE_LONG_OPT,
        SATA_DIPM_LONG_OPT,
        SATA_DAPS_LONG_OPT,
        SAS_PARTIAL_LONG_OPT,
        SAS_SLUMBER_LONG_OPT,
        SET_PHY_SAS_PHY_LONG_OPT,
        IDLE_A_LONG_OPT,
        IDLE_B_LONG_OPT,
        IDLE_C_LONG_OPT,
        STANDBY_Z_LONG_OPT,
        STANDBY_Y_LONG_OPT,
        LEGACY_IDLE_LONG_OPT,
        LEGACY_STANDBY_LONG_OPT,
        TRANSITION_POWER_STATE_LONG_OPT,
        SHOW_POWER_TELEMETRY_LONG_OPT,
        REQUEST_POWER_TELEMETRY_MEASUREMENT_OPTIONS,
        PUIS_FEATURE_LONG_OPT,
        VOLATILE_LONG_OPT,
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
        args = getopt_long(argc, argv, "d:hisSF:Vv:q", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        // printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:

            if (strcmp(longopts[optionIndex].name, EPC_ENABLED_LONG_OPT_STRING) == 0)
            {
                if (strcmp("enable", optarg) == 0)
                {
                    EPC_ENABLED_IDENTIFIER = ENABLE_EPC;
                }
                else if (strcmp("disable", optarg) == 0)
                {
                    EPC_ENABLED_IDENTIFIER = DISABLE_EPC;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(EPC_ENABLED_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, TRANSITION_POWER_STATE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Int32(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &TRANSITION_POWER_STATE_TO))
                {
                    print_Error_In_Cmd_Line_Args(TRANSITION_POWER_STATE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            // parse long options that have no short option and required arguments here
            else if (strcmp(longopts[optionIndex].name, TRANSITION_POWER_MODE_LONG_OPT_STRING) == 0)
            {
                TRANSITION_POWER_MODE_FLAG = true;
                // set the power mode
                if (strcmp(POWER_STATE_ACTIVE_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_ACTIVE;
                }
                else if (strcmp(POWER_STATE_IDLE_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_IDLE;
                }
                else if (strcmp(POWER_STATE_IDLE_UNLOAD_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_IDLE_UNLOAD;
                }
                else if (strcmp(POWER_STATE_STANDBY_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_STANDBY;
                }
                else if (strcmp(POWER_STATE_IDLE_A_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_IDLE_A;
                }
                else if (strcmp(POWER_STATE_IDLE_B_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_IDLE_B;
                }
                else if (strcmp(POWER_STATE_IDLE_C_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_IDLE_C;
                }
                else if (strcmp(POWER_STATE_STANDBY_Y_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_STANDBY_Y;
                }
                else if (strcmp(POWER_STATE_STANDBY_Z_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_STANDBY_Z;
                }
                else if (strcmp(POWER_STATE_SLEEP_STRING, optarg) == 0)
                {
                    TRANSITION_POWER_MODE_TO_POWER_MODE = PWR_CND_SLEEP;
                }
                else
                {
                    TRANSITION_POWER_MODE_FLAG = false;
                    print_Error_In_Cmd_Line_Args(TRANSITION_POWER_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SET_POWER_CONSUMPTION_LONG_OPT_STRING) == 0)
            {
                // set the flag for making the power consumption change
                SET_POWER_CONSUMPTION_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    SET_POWER_CONSUMPTION_DEFAULT_FLAG = true;
                }
                else if (strcmp(optarg, "highest") == 0)
                {
                    SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE = 1;
                }
                else if (strcmp(optarg, "intermediate") == 0)
                {
                    SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE = 2;
                }
                else if (strcmp(optarg, "lowest") == 0)
                {
                    SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE = 3;
                }
                else
                {
                    if (!get_And_Validate_Double_Input(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                       &SET_POWER_CONSUMPTION_WATTS_VALUE))
                    {
                        print_Error_In_Cmd_Line_Args(SET_POWER_CONSUMPTION_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SET_APM_LEVEL_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &SET_APM_LEVEL_VALUE_FLAG))
                {
                    SET_APM_LEVEL_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_APM_LEVEL_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SEAGATE_POWER_BALANCE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SEAGATE_POWER_BALANCE_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    SEAGATE_POWER_BALANCE_FLAG        = true;
                    SEAGATE_POWER_BALANCE_ENABLE_FLAG = true;
                    POWER_BALANCE_MODE                = POWER_BAL_ENABLE;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SEAGATE_POWER_BALANCE_FLAG        = true;
                    SEAGATE_POWER_BALANCE_ENABLE_FLAG = false;
                    POWER_BALANCE_MODE                = POWER_BAL_DISABLE;
                }
                else if (strcmp(optarg, "limited") == 0)
                {
                    SEAGATE_POWER_BALANCE_FLAG         = true;
                    SEAGATE_POWER_BALANCE_LIMITED_FLAG = true;
                    POWER_BALANCE_MODE                 = POWER_BAL_LIMITED;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SEAGATE_POWER_BALANCE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SATA_DIPM_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SATA_DIPM_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    SATA_DIPM_FLAG        = true;
                    SATA_DIPM_ENABLE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SATA_DIPM_FLAG        = true;
                    SATA_DIPM_ENABLE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SATA_DIPM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SATA_DAPS_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SATA_DAPS_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    SATA_DAPS_FLAG        = true;
                    SATA_DAPS_ENABLE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SATA_DAPS_FLAG        = true;
                    SATA_DAPS_ENABLE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SATA_DAPS_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SAS_PARTIAL_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SAS_PARTIAL_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    SAS_PARTIAL_FLAG        = true;
                    SAS_PARTIAL_ENABLE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SAS_PARTIAL_FLAG        = true;
                    SAS_PARTIAL_ENABLE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SAS_PARTIAL_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SAS_SLUMBER_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SAS_SLUMBER_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    SAS_SLUMBER_FLAG        = true;
                    SAS_SLUMBER_ENABLE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SAS_SLUMBER_FLAG        = true;
                    SAS_SLUMBER_ENABLE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SAS_SLUMBER_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SET_PHY_SAS_PHY_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &SET_PHY_SAS_PHY_IDENTIFIER))
                {
                    print_Error_In_Cmd_Line_Args(SET_PHY_SAS_PHY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, IDLE_A_LONG_OPT_STRING) == 0)
            {
                IDLE_A_POWER_MODE_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    IDLE_A_STATE = POWER_MODE_STATE_DEFAULT;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    IDLE_A_STATE = POWER_MODE_STATE_ENABLE;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    IDLE_A_STATE = POWER_MODE_STATE_DISABLE;
                }
                else
                {
                    char* unit = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME, &IDLE_A_POWER_MODE_TIMER))
                    {
                        if (unit)
                        {
                            // convert to milliseconds
                            if (strcmp(unit, "") == 0 || strcmp(unit, "ms") == 0)
                            {
                                // nothing to do as no unit specified or already set milliseconds value
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                IDLE_A_POWER_MODE_TIMER *= UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                IDLE_A_POWER_MODE_TIMER *= UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                IDLE_A_POWER_MODE_TIMER *= UINT32_C(3600000);
                            }
                            else
                            {
                                // invalid unit
                                print_Error_In_Cmd_Line_Args(IDLE_A_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        // the set function expects time in 100 millisecond units
                        // Take the input millisecond time and divide it by 100
                        IDLE_A_TIMER_VALID = true;
                        IDLE_A_POWER_MODE_TIMER /= 100;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(IDLE_A_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, IDLE_B_LONG_OPT_STRING) == 0)
            {
                IDLE_B_POWER_MODE_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    IDLE_B_STATE = POWER_MODE_STATE_DEFAULT;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    IDLE_B_STATE = POWER_MODE_STATE_ENABLE;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    IDLE_B_STATE = POWER_MODE_STATE_DISABLE;
                }
                else
                {
                    char* unit = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME, &IDLE_B_POWER_MODE_TIMER))
                    {
                        if (unit)
                        {
                            // convert to milliseconds
                            if (strcmp(unit, "") == 0 || strcmp(unit, "ms") == 0)
                            {
                                // nothing to do as no unit specified or already set milliseconds value
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                IDLE_B_POWER_MODE_TIMER *= UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                IDLE_B_POWER_MODE_TIMER *= UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                IDLE_B_POWER_MODE_TIMER *= UINT32_C(3600000);
                            }
                            else
                            {
                                // invalid unit
                                print_Error_In_Cmd_Line_Args(IDLE_B_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        // the set function expects time in 100 millisecond units
                        // Take the input millisecond time and divide it by 100
                        IDLE_B_TIMER_VALID = true;
                        IDLE_B_POWER_MODE_TIMER /= 100;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(IDLE_B_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, IDLE_C_LONG_OPT_STRING) == 0)
            {
                IDLE_C_POWER_MODE_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    IDLE_C_STATE = POWER_MODE_STATE_DEFAULT;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    IDLE_C_STATE = POWER_MODE_STATE_ENABLE;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    IDLE_C_STATE = POWER_MODE_STATE_DISABLE;
                }
                else
                {
                    char* unit = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME, &IDLE_C_POWER_MODE_TIMER))
                    {
                        if (unit)
                        {
                            // convert to milliseconds
                            if (strcmp(unit, "") == 0 || strcmp(unit, "ms") == 0)
                            {
                                // nothing to do as no unit specified or already set milliseconds value
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                IDLE_C_POWER_MODE_TIMER *= UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                IDLE_C_POWER_MODE_TIMER *= UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                IDLE_C_POWER_MODE_TIMER *= UINT32_C(3600000);
                            }
                            else
                            {
                                // invalid unit
                                print_Error_In_Cmd_Line_Args(IDLE_C_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        // the set function expects time in 100 millisecond units
                        // Take the input millisecond time and divide it by 100
                        IDLE_C_TIMER_VALID = true;
                        IDLE_C_POWER_MODE_TIMER /= 100;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(IDLE_C_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, STANDBY_Z_LONG_OPT_STRING) == 0)
            {
                STANDBY_Z_POWER_MODE_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    STANDBY_Z_STATE = POWER_MODE_STATE_DEFAULT;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    STANDBY_Z_STATE = POWER_MODE_STATE_ENABLE;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    STANDBY_Z_STATE = POWER_MODE_STATE_DISABLE;
                }
                else
                {
                    char* unit = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME,
                                                              &STANDBY_Z_POWER_MODE_TIMER))
                    {
                        if (unit)
                        {
                            // convert to milliseconds
                            if (strcmp(unit, "") == 0 || strcmp(unit, "ms") == 0)
                            {
                                // nothing to do as no unit specified or already set milliseconds value
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                STANDBY_Z_POWER_MODE_TIMER *= UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                STANDBY_Z_POWER_MODE_TIMER *= UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                STANDBY_Z_POWER_MODE_TIMER *= UINT32_C(3600000);
                            }
                            else
                            {
                                // invalid unit
                                print_Error_In_Cmd_Line_Args(STANDBY_Z_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        // the set function expects time in 100 millisecond units
                        // Take the input millisecond time and divide it by 100
                        STANDBY_Z_TIMER_VALID = true;
                        STANDBY_Z_POWER_MODE_TIMER /= 100;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(STANDBY_Z_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, STANDBY_Y_LONG_OPT_STRING) == 0)
            {
                STANDBY_Y_POWER_MODE_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    STANDBY_Y_STATE = POWER_MODE_STATE_DEFAULT;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    STANDBY_Y_STATE = POWER_MODE_STATE_ENABLE;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    STANDBY_Y_STATE = POWER_MODE_STATE_DISABLE;
                }
                else
                {
                    char* unit = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME,
                                                              &STANDBY_Y_POWER_MODE_TIMER))
                    {
                        if (unit)
                        {
                            // convert to milliseconds
                            if (strcmp(unit, "") == 0 || strcmp(unit, "ms") == 0)
                            {
                                // nothing to do as no unit specified or already set milliseconds value
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                STANDBY_Y_POWER_MODE_TIMER *= UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                STANDBY_Y_POWER_MODE_TIMER *= UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                STANDBY_Y_POWER_MODE_TIMER *= UINT32_C(3600000);
                            }
                            else
                            {
                                // invalid unit
                                print_Error_In_Cmd_Line_Args(STANDBY_Y_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        // the set function expects time in 100 millisecond units
                        // Take the input millisecond time and divide it by 100
                        STANDBY_Y_TIMER_VALID = true;
                        STANDBY_Y_POWER_MODE_TIMER /= 100;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(STANDBY_Y_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, LEGACY_IDLE_LONG_OPT_STRING) == 0)
            {
                LEGACY_IDLE_POWER_MODE_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    LEGACY_IDLE_STATE = POWER_MODE_STATE_DEFAULT;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    LEGACY_IDLE_STATE = POWER_MODE_STATE_ENABLE;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    LEGACY_IDLE_STATE = POWER_MODE_STATE_DISABLE;
                }
                else
                {
                    char* unit = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME,
                                                              &LEGACY_IDLE_POWER_MODE_TIMER))
                    {
                        if (unit)
                        {
                            // convert to milliseconds
                            if (strcmp(unit, "") == 0 || strcmp(unit, "ms") == 0)
                            {
                                // nothing to do as no unit specified or already set milliseconds value
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                LEGACY_IDLE_POWER_MODE_TIMER *= UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                LEGACY_IDLE_POWER_MODE_TIMER *= UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                LEGACY_IDLE_POWER_MODE_TIMER *= UINT32_C(3600000);
                            }
                            else
                            {
                                // invalid unit
                                print_Error_In_Cmd_Line_Args(LEGACY_IDLE_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        // the set function expects time in 100 millisecond units
                        // Take the input millisecond time and divide it by 100
                        LEGACY_IDLE_TIMER_VALID = true;
                        LEGACY_IDLE_POWER_MODE_TIMER /= 100;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(LEGACY_IDLE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, LEGACY_STANDBY_LONG_OPT_STRING) == 0)
            {
                LEGACY_STANDBY_POWER_MODE_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    LEGACY_STANDBY_STATE = POWER_MODE_STATE_DEFAULT;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    LEGACY_STANDBY_STATE = POWER_MODE_STATE_ENABLE;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    LEGACY_STANDBY_STATE = POWER_MODE_STATE_DISABLE;
                }
                else
                {
                    char* unit = M_NULLPTR;
                    if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_TIME,
                                                              &LEGACY_STANDBY_POWER_MODE_TIMER))
                    {
                        if (unit)
                        {
                            // convert to milliseconds
                            if (strcmp(unit, "") == 0 || strcmp(unit, "ms") == 0)
                            {
                                // nothing to do as no unit specified or already set milliseconds value
                            }
                            else if (strcmp(unit, "s") == 0)
                            {
                                LEGACY_STANDBY_POWER_MODE_TIMER *= UINT32_C(1000);
                            }
                            else if (strcmp(unit, "m") == 0)
                            {
                                LEGACY_STANDBY_POWER_MODE_TIMER *= UINT32_C(60000);
                            }
                            else if (strcmp(unit, "h") == 0)
                            {
                                LEGACY_STANDBY_POWER_MODE_TIMER *= UINT32_C(3600000);
                            }
                            else
                            {
                                // invalid unit
                                print_Error_In_Cmd_Line_Args(LEGACY_STANDBY_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        // the set function expects time in 100 millisecond units
                        // Take the input millisecond time and divide it by 100
                        LEGACY_STANDBY_TIMER_VALID = true;
                        LEGACY_STANDBY_POWER_MODE_TIMER /= 100;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(LEGACY_STANDBY_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, REQUEST_POWER_TELEMETRY_MEASUREMENT_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint16(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &REQUEST_POWER_TELEMETRY_MEASUREMENT_TIME_SECONDS))
                {
                    REQUEST_POWER_TELEMETRY_MEASUREMENT_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(REQUEST_POWER_TELEMETRY_MEASUREMENT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE_LONG_OPT_STRING) == 0)
            {
                if (strcmp("all", optarg) == 0)
                {
                    REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE = 0;
                }
                else if (strcmp("5", optarg) == 0)
                {
                    REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE = 5;
                }
                else if (strcmp("12", optarg) == 0)
                {
                    REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE = 12;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
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
          || CHECK_POWER_FLAG || SPIN_DOWN_FLAG || TRANSITION_POWER_MODE_FLAG || SHOW_POWER_CONSUMPTION_FLAG ||
          SET_POWER_CONSUMPTION_FLAG || (EPC_ENABLED_IDENTIFIER != ENABLE_EPC_NOT_SET) || SET_APM_LEVEL_FLAG ||
          SHOW_APM_LEVEL_FLAG || SHOW_EPC_SETTINGS_FLAG || DISABLE_APM_FLAG || SEAGATE_POWER_BALANCE_FLAG ||
          SEAGATE_POWER_BALANCE_INFO_FLAG || SATA_DIPM_INFO_FLAG || SATA_DIPM_FLAG || SATA_DAPS_INFO_FLAG ||
          SATA_DAPS_FLAG || SAS_PARTIAL_FLAG || SAS_PARTIAL_INFO_FLAG || SAS_SLUMBER_FLAG || SAS_SLUMBER_INFO_FLAG ||
          IDLE_A_POWER_MODE_FLAG || IDLE_B_POWER_MODE_FLAG || IDLE_C_POWER_MODE_FLAG || STANDBY_Z_POWER_MODE_FLAG ||
          STANDBY_Y_POWER_MODE_FLAG || LEGACY_IDLE_POWER_MODE_FLAG || LEGACY_STANDBY_POWER_MODE_FLAG ||
          (TRANSITION_POWER_STATE_TO >= 0) || SHOW_POWER_TELEMETRY_FLAG || REQUEST_POWER_TELEMETRY_MEASUREMENT_FLAG ||
          SHOW_NVM_POWER_STATES || PUIS_FEATURE_FLAG))
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

    if (TEST_UNIT_READY_FLAG || CHECK_POWER_FLAG || TRANSITION_POWER_MODE_FLAG || SPIN_DOWN_FLAG ||
        (TRANSITION_POWER_STATE_TO >= 0))
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
        ret        = getDevsRet;
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

        if (EPC_ENABLED_IDENTIFIER != ENABLE_EPC_NOT_SET)
        {
            switch (enable_Disable_EPC_Feature(&deviceList[deviceIter], EPC_ENABLED_IDENTIFIER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (EPC_ENABLED_IDENTIFIER == ENABLE_EPC)
                    {
                        printf("Successfully Enabled EPC Feature Set on %s.\n", deviceList[deviceIter].os_info.name);
                    }
                    else
                    {
                        printf("Successfully Disabled EPC Feature Set on %s.\n", deviceList[deviceIter].os_info.name);
                    }
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
                    printf("EPC Feature is not supported by this device %s.\n", deviceList[deviceIter].os_info.name);
                }
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to send EPC command to %s.\n", deviceList[deviceIter].os_info.name);
                    printf("EPC Feature set might not be supported.\n");
                    printf("Or EPC Feature might already be in the desired state.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (SPIN_DOWN_FLAG)
        {
            switch (spin_down_drive(&deviceList[deviceIter], false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully sent command to spin down device. Please wait 15 seconds for it to finish "
                           "spinning down.\n");
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
                    printf("Spin down not supported by this device.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to spin down device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (TRANSITION_POWER_MODE_FLAG)
        {
            if (TRANSITION_POWER_MODE_TO_POWER_MODE != PWR_CND_NOT_SET &&
                TRANSITION_POWER_MODE_TO_POWER_MODE != PWR_CND_ALL)
            {
                switch (transition_Power_State(&deviceList[deviceIter], TRANSITION_POWER_MODE_TO_POWER_MODE))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf(
                            "\nPower Mode Transition Successful.\nPlease give device a few seconds to transition.\n");
                        printf("\nHint:Use --checkPowerMode option to check the new Power Mode.\n\n");
                        if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                        {
                            printf("NOTE: This command may have affected more than 1 logical unit\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Transitioning power modes not allowed on this device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: Could not transition to the new power state\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must specify a power mode identifier\n");
                }
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            }
        }

        if (TRANSITION_POWER_STATE_TO >= 0)
        {
            switch (transition_NVM_Power_State(&deviceList[deviceIter], C_CAST(uint8_t, TRANSITION_POWER_STATE_TO)))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully transitioned to power state %" PRIi32 ".\n", TRANSITION_POWER_STATE_TO);
                    printf("\nHint:Use --checkPowerMode option to check the new Power State.\n\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Transitioning power states not allowed on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Could not transition to the new power state %" PRIi32 "\n",
                           TRANSITION_POWER_STATE_TO);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (PUIS_FEATURE_FLAG && PUIS_FEATURE_SPINUP_FLAG)
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

        if (SHOW_NVM_POWER_STATES)
        {
            nvmeSupportedPowerStates ps;
            safe_memset(&ps, sizeof(nvmeSupportedPowerStates), 0, sizeof(nvmeSupportedPowerStates));
            switch (get_NVMe_Power_States(&deviceList[deviceIter], &ps))
            {
            case SUCCESS:
                print_NVM_Power_States(&ps);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Showing NVM power states is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Could not read NVM power states from the device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        // this option must come after --transition power so that these two options can be combined on the command line
        // and produce the correct end result
        if (CHECK_POWER_FLAG)
        {
            switch (print_Current_Power_Mode(&deviceList[deviceIter]))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Checking current power mode not supported on this device.\n");
                }
                break;
            default:
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed checking current power mode.\n");
                }
                break;
            }
        }

        if (IDLE_A_POWER_MODE_FLAG || IDLE_B_POWER_MODE_FLAG || IDLE_C_POWER_MODE_FLAG || STANDBY_Z_POWER_MODE_FLAG ||
            STANDBY_Y_POWER_MODE_FLAG)
        {
            // setup the structure and send these changes to the drive.
            powerConditionTimers powerTimers;
            safe_memset(&powerTimers, sizeof(powerConditionTimers), 0, sizeof(powerConditionTimers));
            if (IDLE_A_POWER_MODE_FLAG)
            {
                powerTimers.idle_a.powerConditionValid = true;
                if (IDLE_A_TIMER_VALID)
                {
                    powerTimers.idle_a.timerValid                          = IDLE_A_TIMER_VALID;
                    powerTimers.idle_a.timerInHundredMillisecondIncrements = IDLE_A_POWER_MODE_TIMER;
                    // If a timer was requested to change, also make sure it is set to enabled. This is a fair
                    // assumption if the timer is being changed.
                    powerTimers.idle_a.enable      = true;
                    powerTimers.idle_a.enableValid = true;
                }
                else
                {
                    switch (IDLE_A_STATE)
                    {
                    case POWER_MODE_STATE_ENABLE:
                        powerTimers.idle_a.enable      = true;
                        powerTimers.idle_a.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DISABLE:
                        powerTimers.idle_a.enable      = false;
                        powerTimers.idle_a.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DEFAULT:
                        powerTimers.idle_a.restoreToDefault = true;
                        break;
                    default:
                        // This shouldn't happen, but turn the power condition to an invalid state and continue.
                        // TODO: Should this throw an error instead???
                        powerTimers.idle_a.powerConditionValid = false;
                        break;
                    }
                }
            }
            if (IDLE_B_POWER_MODE_FLAG)
            {
                powerTimers.idle_b.powerConditionValid = true;
                if (IDLE_B_TIMER_VALID)
                {
                    powerTimers.idle_b.timerValid                          = IDLE_B_TIMER_VALID;
                    powerTimers.idle_b.timerInHundredMillisecondIncrements = IDLE_B_POWER_MODE_TIMER;
                    // If a timer was requested to change, also make sure it is set to enabled. This is a fair
                    // assumption if the timer is being changed.
                    powerTimers.idle_b.enable      = true;
                    powerTimers.idle_b.enableValid = true;
                }
                else
                {
                    switch (IDLE_B_STATE)
                    {
                    case POWER_MODE_STATE_ENABLE:
                        powerTimers.idle_b.enable      = true;
                        powerTimers.idle_b.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DISABLE:
                        powerTimers.idle_b.enable      = false;
                        powerTimers.idle_b.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DEFAULT:
                        powerTimers.idle_b.restoreToDefault = true;
                        break;
                    default:
                        // This shouldn't happen, but turn the power condition to an invalid state and continue.
                        // TODO: Should this throw an error instead???
                        powerTimers.idle_b.powerConditionValid = false;
                        break;
                    }
                }
            }
            if (IDLE_C_POWER_MODE_FLAG)
            {
                powerTimers.idle_c.powerConditionValid = true;
                if (IDLE_C_TIMER_VALID)
                {
                    powerTimers.idle_c.timerValid                          = IDLE_C_TIMER_VALID;
                    powerTimers.idle_c.timerInHundredMillisecondIncrements = IDLE_C_POWER_MODE_TIMER;
                    // If a timer was requested to change, also make sure it is set to enabled. This is a fair
                    // assumption if the timer is being changed.
                    powerTimers.idle_c.enable      = true;
                    powerTimers.idle_c.enableValid = true;
                }
                else
                {
                    switch (IDLE_C_STATE)
                    {
                    case POWER_MODE_STATE_ENABLE:
                        powerTimers.idle_c.enable      = true;
                        powerTimers.idle_c.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DISABLE:
                        powerTimers.idle_c.enable      = false;
                        powerTimers.idle_c.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DEFAULT:
                        powerTimers.idle_c.restoreToDefault = true;
                        break;
                    default:
                        // This shouldn't happen, but turn the power condition to an invalid state and continue.
                        // TODO: Should this throw an error instead???
                        powerTimers.idle_c.powerConditionValid = false;
                        break;
                    }
                }
            }
            if (STANDBY_Z_POWER_MODE_FLAG)
            {
                powerTimers.standby_z.powerConditionValid = true;
                if (STANDBY_Z_TIMER_VALID)
                {
                    powerTimers.standby_z.timerValid                          = STANDBY_Z_TIMER_VALID;
                    powerTimers.standby_z.timerInHundredMillisecondIncrements = STANDBY_Z_POWER_MODE_TIMER;
                    // If a timer was requested to change, also make sure it is set to enabled. This is a fair
                    // assumption if the timer is being changed.
                    powerTimers.standby_z.enable      = true;
                    powerTimers.standby_z.enableValid = true;
                }
                else
                {
                    switch (STANDBY_Z_STATE)
                    {
                    case POWER_MODE_STATE_ENABLE:
                        powerTimers.standby_z.enable      = true;
                        powerTimers.standby_z.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DISABLE:
                        powerTimers.standby_z.enable      = false;
                        powerTimers.standby_z.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DEFAULT:
                        powerTimers.standby_z.restoreToDefault = true;
                        break;
                    default:
                        // This shouldn't happen, but turn the power condition to an invalid state and continue.
                        // TODO: Should this throw an error instead???
                        powerTimers.standby_z.powerConditionValid = false;
                        break;
                    }
                }
            }
            if (STANDBY_Y_POWER_MODE_FLAG)
            {
                powerTimers.standby_y.powerConditionValid = true;
                if (STANDBY_Y_TIMER_VALID)
                {
                    powerTimers.standby_y.timerValid                          = STANDBY_Y_TIMER_VALID;
                    powerTimers.standby_y.timerInHundredMillisecondIncrements = STANDBY_Y_POWER_MODE_TIMER;
                    // If a timer was requested to change, also make sure it is set to enabled. This is a fair
                    // assumption if the timer is being changed.
                    powerTimers.standby_y.enable      = true;
                    powerTimers.standby_y.enableValid = true;
                }
                else
                {
                    switch (STANDBY_Y_STATE)
                    {
                    case POWER_MODE_STATE_ENABLE:
                        powerTimers.standby_y.enable      = true;
                        powerTimers.standby_y.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DISABLE:
                        powerTimers.standby_y.enable      = false;
                        powerTimers.standby_y.enableValid = true;
                        break;
                    case POWER_MODE_STATE_DEFAULT:
                        powerTimers.standby_y.restoreToDefault = true;
                        break;
                    default:
                        // This shouldn't happen, but turn the power condition to an invalid state and continue.
                        // TODO: Should this throw an error instead???
                        powerTimers.standby_y.powerConditionValid = false;
                        break;
                    }
                }
            }
            // At this point, all power timers should be configured, so we can issue the command to the drive
            switch (set_EPC_Power_Conditions(&deviceList[deviceIter],
                                             false /*reset all should only be done if intending to also change CCF and
                                                      PM_BG_Precedence which is not yet supported in this tool*/
                                             ,
                                             &powerTimers, !VOLATILE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully configured the requested EPC settings.\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Configuring EPC Settings is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to configure the EPC settings.\n");
                    printf("NOTE: Some settings may have been changed, but at least one failed.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (LEGACY_IDLE_POWER_MODE_FLAG || LEGACY_STANDBY_POWER_MODE_FLAG)
        {
            if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && LEGACY_IDLE_POWER_MODE_FLAG &&
                LEGACY_STANDBY_POWER_MODE_FLAG)
            {
                // SCSI drive can set both of these timers at the same time
                powerConditionSettings standbyTimer, idleTimer;
                uint8_t                restoreCount = UINT8_C(0);
                safe_memset(&standbyTimer, sizeof(powerConditionSettings), 0, sizeof(powerConditionSettings));
                safe_memset(&idleTimer, sizeof(powerConditionSettings), 0, sizeof(powerConditionSettings));
                switch (LEGACY_STANDBY_STATE)
                {
                case POWER_MODE_STATE_ENABLE:
                    standbyTimer.enable      = true;
                    standbyTimer.enableValid = true;
                    break;
                case POWER_MODE_STATE_DISABLE:
                    standbyTimer.enable      = false;
                    standbyTimer.enableValid = true;
                    break;
                case POWER_MODE_STATE_DEFAULT:
                    standbyTimer.restoreToDefault = true;
                    ++restoreCount;
                    break;
                default:
                    // This shouldn't happen, but turn the power condition to an invalid state and continue.
                    // TODO: Should this throw an error instead???
                    standbyTimer.powerConditionValid = false;
                    break;
                }
                if (LEGACY_STANDBY_TIMER_VALID)
                {
                    standbyTimer.enable                              = false;
                    standbyTimer.enableValid                         = true;
                    standbyTimer.timerInHundredMillisecondIncrements = LEGACY_STANDBY_POWER_MODE_TIMER;
                }
                switch (LEGACY_IDLE_STATE)
                {
                case POWER_MODE_STATE_ENABLE:
                    idleTimer.enable      = true;
                    idleTimer.enableValid = true;
                    break;
                case POWER_MODE_STATE_DISABLE:
                    idleTimer.enable      = false;
                    idleTimer.enableValid = true;
                    break;
                case POWER_MODE_STATE_DEFAULT:
                    idleTimer.restoreToDefault = true;
                    ++restoreCount;
                    break;
                default:
                    // This shouldn't happen, but turn the power condition to an invalid state and continue.
                    // TODO: Should this throw an error instead???
                    idleTimer.powerConditionValid = false;
                    break;
                }
                if (LEGACY_IDLE_TIMER_VALID)
                {
                    idleTimer.enable                              = false;
                    idleTimer.enableValid                         = true;
                    idleTimer.timerInHundredMillisecondIncrements = LEGACY_IDLE_POWER_MODE_TIMER;
                }

                switch (scsi_Set_Legacy_Power_Conditions(&deviceList[deviceIter], restoreCount == 2 ? true : false,
                                                         &standbyTimer, &idleTimer, !VOLATILE_FLAG))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Successfully configured the requested idle and standby settings.\n");
                        if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                        {
                            printf("NOTE: This command may have affected more than 1 logical unit\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Configuring standby and idle settings is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("An Error occurred while trying to configure the standby and idle settings.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
#define LEGACY_POWER_MODE_CHANGE_STR_LEN 24
                // need to check if SCSI since ATA won't allow some of these.
                // deviceList[deviceIter].drive_info.drive_type
                if (LEGACY_IDLE_POWER_MODE_FLAG)
                {
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
                    {
                        if (LEGACY_IDLE_TIMER_VALID)
                        {
                            switch (set_Idle_Timer(&deviceList[deviceIter], LEGACY_IDLE_POWER_MODE_TIMER, false,
                                                   !VOLATILE_FLAG))
                            {
                            case SUCCESS:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Successfully configured the idle timer.\n");
                                }
                                break;
                            case NOT_SUPPORTED:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Configuring idle timer is not supported on this device.\n");
                                }
                                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                                break;
                            default:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("An Error occurred while trying to configure the idle timer.\n");
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                break;
                            }
                        }
                        else
                        {
                            eReturnValues idleRet = SUCCESS;
                            DECLARE_ZERO_INIT_ARRAY(char, modeChangeStrSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN);
                            DECLARE_ZERO_INIT_ARRAY(char, modeChangeStrNotSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN);
                            switch (LEGACY_IDLE_STATE)
                            {
                            case POWER_MODE_STATE_ENABLE:
                                idleRet = scsi_Set_Idle_Timer_State(&deviceList[deviceIter], true, !VOLATILE_FLAG);
                                snprintf_err_handle(modeChangeStrSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN, "enabled");
                                snprintf_err_handle(modeChangeStrNotSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN,
                                                    "enabling the");
                                break;
                            case POWER_MODE_STATE_DISABLE:
                                idleRet = scsi_Set_Idle_Timer_State(&deviceList[deviceIter], false, !VOLATILE_FLAG);
                                snprintf_err_handle(modeChangeStrSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN, "disable");
                                snprintf_err_handle(modeChangeStrNotSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN,
                                                    "disabling the");
                                break;
                            case POWER_MODE_STATE_DEFAULT:
                                idleRet = set_Idle_Timer(&deviceList[deviceIter], 0, true, !VOLATILE_FLAG);
                                snprintf_err_handle(modeChangeStrSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN,
                                                    "restored defaults");
                                snprintf_err_handle(modeChangeStrNotSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN,
                                                    "restoring the default");
                                break;
                            default:
                                break;
                            }
                            switch (idleRet)
                            {
                            case SUCCESS:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Successfully %s the requested idle settings.\n", modeChangeStrSuccess);
                                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                    {
                                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                                    }
                                }
                                break;
                            case NOT_SUPPORTED:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("%s idle settings is not supported on this device.\n",
                                           modeChangeStrNotSuccess);
                                }
                                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                                break;
                            default:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("An Error occurred while %s idle settings.\n", modeChangeStrNotSuccess);
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                break;
                            }
                        }
                    }
                    else
                    {
                        // not supported
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Configuring idle settings is not supported on this device.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    }
                }
                if (LEGACY_STANDBY_POWER_MODE_FLAG)
                {
                    if (LEGACY_STANDBY_TIMER_VALID)
                    {
                        switch (set_Standby_Timer(&deviceList[deviceIter], LEGACY_STANDBY_POWER_MODE_TIMER, false,
                                                  !VOLATILE_FLAG))
                        {
                        case SUCCESS:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Successfully configured the requested standby timer.\n");
                                if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                {
                                    printf("NOTE: This command may have affected more than 1 logical unit\n");
                                }
                            }
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Configuring standby timer is not supported on this device.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("An Error occurred while trying to configure the standby timer.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
                        {
                            eReturnValues standbyRet = SUCCESS;
                            DECLARE_ZERO_INIT_ARRAY(char, modeChangeStrSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN);
                            DECLARE_ZERO_INIT_ARRAY(char, modeChangeStrNotSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN);
                            switch (LEGACY_STANDBY_STATE)
                            {
                            case POWER_MODE_STATE_ENABLE:
                                standbyRet =
                                    scsi_Set_Standby_Timer_State(&deviceList[deviceIter], true, !VOLATILE_FLAG);
                                snprintf_err_handle(modeChangeStrSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN, "enabled");
                                snprintf_err_handle(modeChangeStrNotSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN,
                                                    "enabling the");
                                break;
                            case POWER_MODE_STATE_DISABLE:
                                standbyRet =
                                    scsi_Set_Standby_Timer_State(&deviceList[deviceIter], false, !VOLATILE_FLAG);
                                snprintf_err_handle(modeChangeStrSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN, "disable");
                                snprintf_err_handle(modeChangeStrNotSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN,
                                                    "disabling the");
                                break;
                            case POWER_MODE_STATE_DEFAULT:
                                standbyRet = set_Standby_Timer(&deviceList[deviceIter], 0, true, !VOLATILE_FLAG);
                                snprintf_err_handle(modeChangeStrSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN,
                                                    "restored defaults");
                                snprintf_err_handle(modeChangeStrNotSuccess, LEGACY_POWER_MODE_CHANGE_STR_LEN,
                                                    "restoring the default");
                                break;
                            default:
                                break;
                            }
                            switch (standbyRet)
                            {
                            case SUCCESS:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Successfully %s the requested standby settings.\n", modeChangeStrSuccess);
                                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                    {
                                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                                    }
                                }
                                break;
                            case NOT_SUPPORTED:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("%s standby settings is not supported on this device.\n",
                                           modeChangeStrNotSuccess);
                                }
                                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                                break;
                            default:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("An Error occurred while trying to %s standby settings.\n",
                                           modeChangeStrNotSuccess);
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                break;
                            }
                        }
                        else
                        {
                            // not supported
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Enabling, disabling, or restoring default standby settings is not supported on "
                                       "this device.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        }
                    }
                }
            }
        }

        if (PUIS_FEATURE_FLAG && !PUIS_FEATURE_SPINUP_FLAG)
        {
            // handle all other PUIS feature options as spinup is handled above with other powerstate transitioning
            // commands.
            if (PUIS_FEATURE_INFO_FLAG)
            {
                puisInfo info;
                safe_memset(&info, sizeof(puisInfo), 0, sizeof(puisInfo));
                switch (get_PUIS_Info(&deviceList[deviceIter], &info))
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

        if (SHOW_EPC_SETTINGS_FLAG)
        {
            epcSettings deviceEPCSettings;
            safe_memset(&deviceEPCSettings, sizeof(epcSettings), 0, sizeof(epcSettings));
            switch (get_EPC_Settings(&deviceList[deviceIter], &deviceEPCSettings))
            {
            case SUCCESS:
                print_EPC_Settings(&deviceList[deviceIter], &deviceEPCSettings);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Showing EPC Settings not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to get the EPC settings.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHOW_POWER_CONSUMPTION_FLAG)
        {
            powerConsumptionIdentifiers identifiers;
            safe_memset(&identifiers, sizeof(powerConsumptionIdentifiers), 0, sizeof(powerConsumptionIdentifiers));
            switch (get_Power_Consumption_Identifiers(&deviceList[deviceIter], &identifiers))
            {
            case SUCCESS:
                print_Power_Consumption_Identifiers(&identifiers);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Power Consumption information not available on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to read power consumption information.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_POWER_CONSUMPTION_FLAG)
        {
            eReturnValues pcRet = SUCCESS;
            if (SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE == PC_ACTIVE_LEVEL_IDENTIFIER)
            {
                pcRet = map_Watt_Value_To_Power_Consumption_Identifier(
                    &deviceList[deviceIter], SET_POWER_CONSUMPTION_WATTS_VALUE, &SET_POWER_CONSUMPTION_VALUE);
            }
            if (pcRet == SUCCESS)
            {
                switch (set_Power_Consumption(&deviceList[deviceIter], SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE,
                                              SET_POWER_CONSUMPTION_VALUE, SET_POWER_CONSUMPTION_DEFAULT_FLAG))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Successfully set power consumption value for device!\n");
                        if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                        {
                            printf("NOTE: This command may have affected more than 1 logical unit\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Setting power consumption is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("An Error occurred while trying to read power consumption information.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An invalid or unsupported value was entered for power consumption level.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SET_APM_LEVEL_FLAG)
        {
            switch (set_APM_Level(&deviceList[deviceIter], SET_APM_LEVEL_VALUE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set APM Level!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting APM Level is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to set the APM Level.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (DISABLE_APM_FLAG)
        {
            switch (enable_Disable_APM_Feature(&deviceList[deviceIter], false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully disabled APM feature!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Disabling APM is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to disable APM.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHOW_APM_LEVEL_FLAG)
        {
            switch (get_APM_Level(&deviceList[deviceIter], &SHOW_APM_LEVEL_VALUE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Current APM Level is %" PRIu8 " (", SHOW_APM_LEVEL_VALUE_FLAG);
                    if (SHOW_APM_LEVEL_VALUE_FLAG == 0x01)
                    {
                        printf("Minimum power consumption with Standby mode");
                    }
                    else if (SHOW_APM_LEVEL_VALUE_FLAG >= 0x02 && SHOW_APM_LEVEL_VALUE_FLAG <= 0x7F)
                    {
                        printf("Intermediate power management with Standby mode");
                    }
                    else if (SHOW_APM_LEVEL_VALUE_FLAG == 0x80)
                    {
                        printf("Minimum power consumption without Standby mode");
                    }
                    else if (SHOW_APM_LEVEL_VALUE_FLAG >= 0x81 && SHOW_APM_LEVEL_VALUE_FLAG <= 0xFD)
                    {
                        printf("Intermediate power management without Standby mode");
                    }
                    else if (SHOW_APM_LEVEL_VALUE_FLAG == 0xFE)
                    {
                        printf("Maximum Performance");
                    }
                    else
                    {
                        printf("Reserved");
                    }
                    printf(")\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Showing APM Level is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to get the APM Level.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SEAGATE_POWER_BALANCE_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Power Balance is a feture unique to Seagate drives and is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
            else
            {
                switch (seagate_Set_Power_Balance(&deviceList[deviceIter], POWER_BALANCE_MODE))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (SEAGATE_POWER_BALANCE_ENABLE_FLAG)
                        {
                            printf("Successfully enabled Seagate Power Balance!\n");
                        }
                        else if (SEAGATE_POWER_BALANCE_LIMITED_FLAG)
                        {
                            printf("Successfully limited Seagate Power Balance!\n");
                        }
                        else
                        {
                            printf("Successfully disabled Seagate Power Balance!\n");
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
                        printf("Setting Seagate Power Balance feature is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to set the Seagate Power Balance feature!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (SEAGATE_POWER_BALANCE_INFO_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Power Balance is a feture unique to Seagate drives and is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
            else
            {
                bool powerBalanceSupported = false;
                bool powerBalanceEnabled   = false;
                switch (
                    seagate_Get_Power_Balance(&deviceList[deviceIter], &powerBalanceSupported, &powerBalanceEnabled))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Seagate Power Balance: ");
                        if (powerBalanceSupported)
                        {
                            if (powerBalanceEnabled)
                            {
                                printf("Enabled\n");
                            }
                            else
                            {
                                printf("Disabled\n");
                            }
                        }
                        else
                        {
                            printf("Not Supported\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Getting Seagate Power Balance info is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to get the Seagate Power Balance info!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (SATA_DIPM_FLAG)
        {
            switch (sata_Set_Device_Initiated_Interface_Power_State_Transitions(&deviceList[deviceIter],
                                                                                SATA_DIPM_ENABLE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SATA_DIPM_ENABLE_FLAG)
                    {
                        printf("Successfully enabled Device Initiated Power Management (DIPM)!\n");
                    }
                    else
                    {
                        printf("Successfully disabled Device Initiated Power Management (DIPM)!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting DIPM feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the DIPM feature!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (SATA_DIPM_INFO_FLAG)
        {
            bool dipmSupported = false;
            bool dipmEnabled   = false;
            switch (sata_Get_Device_Initiated_Interface_Power_State_Transitions(&deviceList[deviceIter], &dipmSupported,
                                                                                &dipmEnabled))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Device Initiated Power Management (DIPM): ");
                    if (dipmSupported)
                    {
                        if (dipmEnabled)
                        {
                            printf("Enabled\n");
                        }
                        else
                        {
                            printf("Disabled\n");
                        }
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting DIPM info is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get the DIPM info!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SATA_DAPS_FLAG)
        {
            switch (sata_Set_Device_Automatic_Partial_To_Slumber_Transtisions(&deviceList[deviceIter],
                                                                               SATA_DAPS_ENABLE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SATA_DAPS_ENABLE_FLAG)
                    {
                        printf("Successfully enabled Device Automatic Partial To Slumber Transitions (DAPS)!\n");
                    }
                    else
                    {
                        printf("Successfully disabled Device Automatic Partial To Slumber Transitions (DAPS)!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting DAPS feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the DAPS feature!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SATA_DAPS_INFO_FLAG)
        {
            bool dapsSupported = false;
            bool dapsEnabled   = false;
            switch (sata_Get_Device_Automatic_Partial_To_Slumber_Transtisions(&deviceList[deviceIter], &dapsSupported,
                                                                               &dapsEnabled))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Device Automatic Partial To Slumber Transitions (DAPS): ");
                    if (dapsSupported)
                    {
                        if (dapsEnabled)
                        {
                            printf("Enabled\n");
                        }
                        else
                        {
                            printf("Disabled\n");
                        }
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting DAPS info is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get the DAPS info!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SAS_PARTIAL_FLAG || SAS_SLUMBER_FLAG)
        {
#define SEACHEST_POWERCONTROL_PARTIAL_SLUMBER_STRING_LENGTH 40
            DECLARE_ZERO_INIT_ARRAY(char, partialSlumberString, SEACHEST_POWERCONTROL_PARTIAL_SLUMBER_STRING_LENGTH);
            if (SAS_PARTIAL_FLAG && SAS_SLUMBER_FLAG)
            {
                snprintf_err_handle(partialSlumberString, SEACHEST_POWERCONTROL_PARTIAL_SLUMBER_STRING_LENGTH,
                         "SAS Partial & Slumber");
            }
            else if (SAS_PARTIAL_FLAG)
            {
                snprintf_err_handle(partialSlumberString, SEACHEST_POWERCONTROL_PARTIAL_SLUMBER_STRING_LENGTH, "SAS Partial");
            }
            else if (SAS_SLUMBER_FLAG)
            {
                snprintf_err_handle(partialSlumberString, SEACHEST_POWERCONTROL_PARTIAL_SLUMBER_STRING_LENGTH, "Slumber");
            }
            switch (scsi_Set_Partial_Slumber(
                &deviceList[deviceIter], SAS_PARTIAL_ENABLE_FLAG, SAS_SLUMBER_ENABLE_FLAG, SAS_PARTIAL_FLAG,
                SAS_SLUMBER_FLAG, SET_PHY_SAS_PHY_IDENTIFIER == 0xFF ? true : false, SET_PHY_SAS_PHY_IDENTIFIER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully changed %s\n", partialSlumberString);
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting %s is not supported on this device.\n", partialSlumberString);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the %s feature!\n", partialSlumberString);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SAS_PARTIAL_INFO_FLAG || SAS_SLUMBER_INFO_FLAG)
        {
            eReturnValues result      = SUCCESS;
            uint8_t       phyListSize = UINT8_C(1);
            if (SET_PHY_SAS_PHY_IDENTIFIER == 0xFF)
            {
                result = get_SAS_Enhanced_Phy_Control_Number_Of_Phys(&deviceList[deviceIter], &phyListSize);
            }
            if (SUCCESS == result)
            {
                ptrSasEnhPhyControl phyData =
                    M_REINTERPRET_CAST(ptrSasEnhPhyControl, safe_calloc(phyListSize, sizeof(sasEnhPhyControl)));
                if (phyData)
                {
                    // get the information needed, then show it
                    result = get_SAS_Enhanced_Phy_Control_Partial_Slumber_Settings(
                        &deviceList[deviceIter], SET_PHY_SAS_PHY_IDENTIFIER == 0xFF ? true : false,
                        SET_PHY_SAS_PHY_IDENTIFIER, phyData, phyListSize * sizeof(sasEnhPhyControl));
                    switch (result)
                    {
                    case SUCCESS:
                        show_SAS_Enh_Phy_Control_Partial_Slumber(phyData, phyListSize * sizeof(sasEnhPhyControl),
                                                                 SAS_PARTIAL_INFO_FLAG, SAS_SLUMBER_INFO_FLAG);
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("SAS Enhanced phy control is not supported on this device. Partial and Slumber are "
                                   "not supported.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to read the SAS Enhanced phy control mode page for Partial/Slumber "
                                   "settings!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    safe_free_sasEnhPhyControl(&phyData);
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Error allocating memory before showing SAS Partial/Slumber info!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get the phy count before showing SAS Partial/Slumber info!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (REQUEST_POWER_TELEMETRY_MEASUREMENT_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf(
                        "Power Telemetry is a feture unique to Seagate drives and is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
            else
            {
                if (is_Seagate_Power_Telemetry_Feature_Supported(&deviceList[deviceIter]))
                {
                    switch (request_Power_Measurement(
                        &deviceList[deviceIter], REQUEST_POWER_TELEMETRY_MEASUREMENT_TIME_SECONDS,
                        C_CAST(ePowerTelemetryMeasurementOptions, REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE)))
                    {
                    case SUCCESS:
                        // show a time when the measurement is expected to be complete?
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Successfully requested a power measurement.\n");
                            time_t currentTime = time(M_NULLPTR);
                            time_t futureTime =
                                get_Future_Date_And_Time(currentTime, REQUEST_POWER_TELEMETRY_MEASUREMENT_TIME_SECONDS);
                            DECLARE_ZERO_INIT_ARRAY(char, timeFormat, TIME_STRING_LENGTH);
                            printf("\n\tCurrent Time: %s\n",
                                   get_Current_Time_String(C_CAST(const time_t*, &currentTime), timeFormat,
                                                           TIME_STRING_LENGTH));
                            safe_memset(timeFormat, TIME_STRING_LENGTH, 0, TIME_STRING_LENGTH);
                            printf("\tEstimated completion Time : %s",
                                   get_Current_Time_String(C_CAST(const time_t*, &futureTime), timeFormat,
                                                           TIME_STRING_LENGTH));
                        }
                        break;
                    case NOT_SUPPORTED: // unlikely since we checked for support first
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Requesting a power measurement is not supported on this device.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to request a power measurement!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    // power telemetry is not supported
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Seagate Power Telemetry is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
            }
        }

        if (SHOW_POWER_TELEMETRY_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf(
                        "Power Telemetry is a feture unique to Seagate drives and is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
            else
            {
                if (is_Seagate_Power_Telemetry_Feature_Supported(&deviceList[deviceIter]))
                {
                    seagatePwrTelemetry pwrTelData;
                    safe_memset(&pwrTelData, sizeof(seagatePwrTelemetry), 0, sizeof(seagatePwrTelemetry));
                    switch (get_Power_Telemetry_Data(&deviceList[deviceIter], &pwrTelData))
                    {
                    case SUCCESS:
                        // show it
                        show_Power_Telemetry_Data(&pwrTelData);
                        break;
                    case NOT_SUPPORTED: // unlikely to happen due to outside guard
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Getting a power telemetry data is not supported on this device.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to get a power telemetry data!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    // power telemetry is not supported
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Seagate Power Telemetry is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
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

    printf("Examples\n");
    printf("========\n");
    // example usage
    printf("\t%s --%s\n", util_name, SCAN_LONG_OPT_STRING);
    printf("\t%s -d %s -%c\n", util_name, deviceHandleExample, DEVICE_INFO_SHORT_OPT);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SAT_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LOWLEVEL_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, CHECK_POWER_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_EPC_SETTINGS_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 5000\n", util_name, deviceHandleExample, IDLE_A_LONG_OPT_STRING);
    printf("\t%s -d %s --%s disable\n", util_name, deviceHandleExample, IDLE_B_LONG_OPT_STRING);
    printf("\t%s -d %s --%s standby_z\n", util_name, deviceHandleExample, TRANSITION_POWER_MODE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SPIN_DOWN_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 1\n", util_name, deviceHandleExample, TRANSITION_POWER_STATE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_NVM_POWER_STATES_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 5000 --%s 30000 --%s enable --%s disable\n", util_name, deviceHandleExample,
           IDLE_A_LONG_OPT_STRING, STANDBY_Z_LONG_OPT_STRING, IDLE_B_LONG_OPT_STRING, IDLE_C_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_POWER_TELEMETRY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 120 --%s 12\n", util_name, deviceHandleExample,
           REQUEST_POWER_TELEMETRY_MEASUREMENT_LONG_OPT_STRING,
           REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enable\n", util_name, deviceHandleExample, EPC_ENABLED_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 15000\n", util_name, deviceHandleExample, LEGACY_STANDBY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 5000\n", util_name, deviceHandleExample, LEGACY_IDLE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_APM_LEVEL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 128\n", util_name, deviceHandleExample, SET_APM_LEVEL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_POWER_CONSUMPTION_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 12\n", util_name, deviceHandleExample, SET_POWER_CONSUMPTION_LONG_OPT_STRING);
    printf("\t%s -d %s --%s lowest\n", util_name, deviceHandleExample, SET_POWER_CONSUMPTION_LONG_OPT_STRING);
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
    print_Check_Power_Mode_Help(shortUsage);
    print_EnableDisableEPC_Help(shortUsage);
    print_Idle_A_Help(shortUsage);
    print_Idle_B_Help(shortUsage);
    print_Idle_C_Help(shortUsage);
    print_Request_Power_Measurement_Mode_Help(shortUsage);
    print_Request_Power_Measurement_Help(shortUsage);
    print_Seagate_Power_Balance_Help(shortUsage);
    print_Show_EPC_Settings_Help(shortUsage);
    print_Show_Power_Telemetry_Help(shortUsage);
    print_Spindown_Help(shortUsage);
    print_Legacy_Standby_Help(shortUsage);
    print_Standby_Y_Help(shortUsage);
    print_Standby_Z_Help(shortUsage);
    print_Transition_Power_Help(shortUsage);

    // SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_Disable_APM_Help(shortUsage);
    print_DAPS_Help(shortUsage);
    print_DIPM_Help(shortUsage);
    print_PUIS_Feature_Help(shortUsage);
    print_Set_APM_Level_Help(shortUsage);
    print_Show_APM_Level_Help(shortUsage);
    // SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");
    print_Legacy_Idle_Help(shortUsage);
    print_SAS_Phy_Help(shortUsage);
    print_SAS_Phy_Partial_Help(shortUsage);
    print_SAS_Phy_Slumber_Help(shortUsage);
    print_Set_Power_Consumption_Help(shortUsage);
    print_Show_Power_Consumption_Help(shortUsage);
    // NVMe Only
    printf("\n\tNVMe Only:\n\t=========\n");
    print_Show_NVM_Power_States_Help(shortUsage);
    print_Transition_Power_State_Help(shortUsage);
}
