// SPDX-License-Identifier: MPL-2.0
//
// openSeaChest_NVMe.c
//
// Copyright (c) 2014-2025 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// **************************************************************************************
// \file openSeaChest_NVMe.c Binary command line that performs various erase methods on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "io_utils.h"
#include "math_utils.h"
#include "memory_safety.h"
#include "precision_timer.h"
#include "secure_file.h"
#include "sleep.h"
#include "string_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"

#include "EULA.h"
#include "drive_info.h"
#include "firmware_download.h"
#include "format.h"
#include "getopt.h"
#include "logs.h"
#include "nvme_operations.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "power_control.h"
#include "smart.h"
#include <math.h>

////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name    = "openSeaChest_NVMe";
const char* buildVersion = "3.0.2";

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
    DATA_ERASE_VAR
    LICENSE_VAR
    ECHO_COMMAND_LINE_VAR
    SCAN_FLAG_VAR
    NO_BANNER_VAR
    AGRESSIVE_SCAN_FLAG_VAR
    SHOW_BANNER_VAR
    SHOW_HELP_VAR
    TEST_UNIT_READY_VAR
    FAST_DISCOVERY_VAR
    OUTPUTPATH_VAR
    LOG_TRANSFER_LENGTH_BYTES_VAR
    DOWNLOAD_FW_VARS
    ACTIVATE_DEFERRED_FW_VAR
    SWITCH_FW_VAR
    FWDL_SEGMENT_SIZE_VARS
    FW_MATCH_VARS
    FIRMWARE_SLOT_VAR
    NEW_FW_MATCH_VARS
    CHECK_POWER_VAR
    TRANSITION_POWER_STATE_VAR
    SHOW_NVM_POWER_STATES_VAR
    GET_NVME_LOG_VAR
    GET_TELEMETRY_VAR
    TELEMETRY_DATA_AREA_VAR
    OUTPUT_MODE_VAR
    GET_FEATURES_VARS
    NVME_TEMP_STATS_VAR
    NVME_PCI_STATS_VAR
    MODEL_MATCH_VARS
    // CHILD_MODEL_MATCH_VARS
    // CHILD_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    // POWER_MODE_VAR
    // scan output flags
    SCAN_FLAGS_UTIL_VARS
    EXT_SMART_LOG_VAR1
    CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_VAR
    NVM_FORMAT_VARS
    NVM_FORMAT_OPTION_VARS
    POLL_VAR
    PROGRESS_VAR
    SHOW_SUPPORTED_FORMATS_VAR
    LOWLEVEL_INFO_VAR
    LIST_LOGS_VAR
    FORCE_DRIVE_TYPE_VARS

#if defined(ENABLE_HWRAID_SUPPORT)
    RAID_PHYSICAL_DRIVE
    rDevice; // TODO: This should be a list so that we can talk to more than one raid device at a time!
#endif

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
        // 
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
        POLL_LONG_OPT,
        PROGRESS_LONG_OPT,
        LOWLEVEL_INFO_LONG_OPT,
        // tool specific options go here
        DOWNLOAD_FW_MODE_LONG_OPT,
        DOWNLOAD_FW_LONG_OPT,
        NEW_FW_MATCH_LONG_OPT,
        FWDL_SEGMENT_SIZE_LONG_OPT,
        ACTIVATE_DEFERRED_FW_LONG_OPT,
        SWITCH_FW_LONG_OPT,
        FIRMWARE_SLOT_BUFFER_ID_LONG_OPT,
        CHECK_POWER_LONG_OPT,
        TRANSITION_POWER_STATE_LONG_OPT,
        SHOW_NVM_POWER_STATES_LONG_OPT,
        GET_NVME_LOG_LONG_OPT,
        GET_TELEMETRY_LONG_OPT,
        TELEMETRY_DATA_AREA_LONG_OPT,
        CONFIRM_LONG_OPT,
        OUTPUT_MODE_LONG_OPT,
        GET_FEATURES_LONG_OPT,
        EXT_SMART_LOG_LONG_OPT1,
        MODEL_MATCH_LONG_OPT,
        FW_MATCH_LONG_OPT,
        // CHILD_MODEL_MATCH_LONG_OPT,
        // CHILD_FW_MATCH_LONG_OPT,
        CLEAR_PCIE_CORRECTABLE_ERRORS_LONG_OPT,
        NVME_TEMP_STATS_LONG_OPT,
        NVME_PCI_STATS_LONG_OPT,
        SHOW_SUPPORTED_FORMATS_LONG_OPT,
        NVM_FORMAT_LONG_OPT,
        NVM_FORMAT_OPTIONS_LONG_OPTS,
        LIST_LOGS_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        LONG_OPT_TERMINATOR
    };
    // clang-format on

    eVerbosityLevels toolVerbosity = VERBOSITY_DEFAULT;
    DOWNLOAD_FW_MODE               = DL_FW_UNKNOWN;

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
        args = getopt_long(argc, argv, "d:hisF:Vv:q%:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        // printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:
            // parse long options that have no short option and required arguments here
            // parse long options that have no short option and required arguments here
            if (strcmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, DATA_ERASE_ACCEPT_STRING) == 0)
                {
                    DATA_ERASE_FLAG = true;
                }
            }
            else if (strcmp(longopts[optionIndex].name, OUTPUT_MODE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "raw") == 0)
                {
                    OUTPUT_MODE_IDENTIFIER = UTIL_OUTPUT_MODE_RAW;
                }
                else if (strcmp(optarg, "binary") == 0)
                {
                    OUTPUT_MODE_IDENTIFIER = UTIL_OUTPUT_MODE_BIN;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(OUTPUT_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, PATH_LONG_OPT_STRING) == 0)
            {
                OUTPUTPATH_PARSE
                if (!os_Directory_Exists(OUTPUTPATH_FLAG))
                {
                    printf("Err: --outputPath %s does not exist\n", OUTPUTPATH_FLAG);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, LOG_TRANSFER_LENGTH_LONG_OPT_STRING) == 0)
            {
                char* unit = M_NULLPTR;
                if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_DATASIZE,
                                                          &LOG_TRANSFER_LENGTH_BYTES))
                {
                    // Check to see if any units were specified, otherwise assume LBAs
                    uint32_t multiplier = 1;
                    if (unit)
                    {
                        if (strcmp(unit, "") == 0)
                        {
                            multiplier = 1; // no additional units provided, so do not treat as an error
                        }
                        else if (strcmp(unit, "BLOCKS") == 0 || strcmp(unit, "SECTORS") == 0)
                        {
                            // they specified blocks. For log transfers this means a number of 512B sectors
                            multiplier = LEGACY_DRIVE_SEC_SIZE;
                        }
                        else if (strcmp(unit, "KB") == 0)
                        {
                            multiplier = UINT64_C(1000);
                        }
                        else if (strcmp(unit, "KiB") == 0)
                        {
                            multiplier = UINT64_C(1024);
                        }
                        else if (strcmp(unit, "MB") == 0)
                        {
                            multiplier = UINT64_C(1000000);
                        }
                        else if (strcmp(unit, "MiB") == 0)
                        {
                            multiplier = UINT64_C(1048576);
                        }
                        else
                        {
                            print_Error_In_Cmd_Line_Args(LOG_TRANSFER_LENGTH_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    LOG_TRANSFER_LENGTH_BYTES *= multiplier;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(LOG_TRANSFER_LENGTH_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, GET_FEATURES_LONG_OPT_STRING) == 0)
            {
                GET_FEATURES_FLAG = true;
                if (strcmp(optarg, "help") == 0)
                {
                    nvme_Print_Feature_Identifiers_Help();
                    exit(UTIL_EXIT_NO_ERROR);
                }
                else if (strcmp(optarg, "list") == 0)
                {
                    GET_FEATURES = UINT16_MAX; // set a value higher than is possible to request to know it is invalid
                                               // and requesting the list
                }
                else if (!get_And_Validate_Integer_Input_Uint16(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &GET_FEATURES))
                {
                    print_Error_In_Cmd_Line_Args(GET_FEATURES_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVME_TEMP_STATS_LONG_OPT_STRING) == 0)
            {
                NVME_TEMP_STATS_FLAG = goTrue;
            }
            else if (strcmp(longopts[optionIndex].name, NVME_PCI_STATS_LONG_OPT_STRING) == 0)
            {
                NVME_PCI_STATS_FLAG = goTrue;
            }
            else if (strcmp(longopts[optionIndex].name, DOWNLOAD_FW_LONG_OPT_STRING) == 0)
            {
                int res = snprintf_err_handle(DOWNLOAD_FW_FILENAME_FLAG, FIRMWARE_FILE_NAME_MAX_LEN, "%s", optarg);
                if (res > 0 && res <= FIRMWARE_FILE_NAME_MAX_LEN)
                {
                    DOWNLOAD_FW_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(DOWNLOAD_FW_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, DOWNLOAD_FW_MODE_LONG_OPT_STRING) == 0)
            {
                DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_AUTOMATIC;
                if (strcmp(optarg, "immediate") == 0 || strcmp(optarg, "full") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_FULL;
                }
                else if (strcmp(optarg, "segmented") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_SEGMENTED;
                }
                else if (strcmp(optarg, "deferred") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_DEFERRED;
                }
                // TODO: deferredselect and a way to get events: POA, HRA, and VSA
                else if (strcmp(optarg, "deferred+activate") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_DEFERRED_PLUS_ACTIVATE;
                }
                else if (strcmp(optarg, "auto") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_AUTOMATIC;
                }
                else if (strcmp(optarg, "temp") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_TEMP;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(DOWNLOAD_FW_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, FWDL_SEGMENT_SIZE_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint16(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &FWDL_SEGMENT_SIZE_FLAG))
                {
                    FWDL_SEGMENT_SIZE_FROM_USER = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(FWDL_SEGMENT_SIZE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, FIRMWARE_SLOT_LONG_OPT_STRING) == 0 ||
                     strcmp(longopts[optionIndex].name, FIRMWARE_BUFFER_ID_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &FIRMWARE_SLOT_FLAG) ||
                    FIRMWARE_SLOT_FLAG > 7)
                {
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("FirmwareSlot/FwBuffer ID must be between 0 and 7\n");
                    }
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, TRANSITION_POWER_STATE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Int32(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &TRANSITION_POWER_STATE_TO))
                {
                    print_Error_In_Cmd_Line_Args(TRANSITION_POWER_STATE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, GET_NVME_LOG_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &GET_NVME_LOG_IDENTIFIER))
                {
                    if (strcmp("error", optarg) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_ERROR_ID;
                    }
                    else if (strcasecmp("smart", optarg) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_SMART_ID;
                    }
                    else if (strcmp("fwSlots", optarg) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_FW_SLOT_ID;
                    }
                    else if (strcmp("suppCmds", optarg) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_CMD_SPT_EFET_ID;
                    }
                    else if (strcmp("selfTest", optarg) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_DEV_SELF_TEST_ID;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(GET_NVME_LOG_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, GET_TELEMETRY_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "current") == 0 || strcmp(optarg, "host") == 0)
                {
                    GET_TELEMETRY_IDENTIFIER = 1;
                }
                else if (strcmp(optarg, "saved") == 0 || strcmp(optarg, "ctrl") == 0)
                {
                    GET_TELEMETRY_IDENTIFIER = 2;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(GET_TELEMETRY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, TELEMETRY_DATA_AREA_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &TELEMETRY_DATA_AREA))
                {
                    print_Error_In_Cmd_Line_Args(TELEMETRY_DATA_AREA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_LONG_OPT_STRING) == 0)
            {
                NVM_FORMAT_FLAG = true;
                if (strcmp(optarg, "current") != 0)
                {
                    // set the sector size
                    if (!get_And_Validate_Integer_Input_Uint32(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                               &NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM))
                    {
                        print_Error_In_Cmd_Line_Args(NVM_FORMAT_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_NSID_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "current") == 0)
                {
                    NVM_FORMAT_NSID = 0; // detect this below and insert the correct NSID for the current handle
                }
                else if (strcmp(optarg, "all") == 0)
                {
                    NVM_FORMAT_NSID = UINT32_MAX;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(NVM_FORMAT_NSID_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "none") == 0)
                {
                    NVM_FORMAT_SECURE_ERASE = NVM_FMT_SE_NO_SECURE_ERASE_REQUESTED;
                }
                else if (strcmp(optarg, "user") == 0)
                {
                    NVM_FORMAT_SECURE_ERASE = NVM_FMT_SE_USER_DATA;
                }
                else if (strcmp(optarg, "crypto") == 0)
                {
                    NVM_FORMAT_SECURE_ERASE = NVM_FMT_SE_NO_SECURE_ERASE_REQUESTED;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_PI_TYPE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &NVM_FORMAT_PI_TYPE) ||
                    NVM_FORMAT_PI_TYPE > 3)
                {
                    print_Error_In_Cmd_Line_Args(NVM_FORMAT_PI_TYPE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_PI_LOCATION_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "beginning") == 0)
                {
                    NVM_FORMAT_PI_LOCATION = 0;
                }
                else if (strcmp(optarg, "end") == 0)
                {
                    NVM_FORMAT_PI_LOCATION = 1;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(NVM_FORMAT_PI_LOCATION_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_METADATA_SIZE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint32(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                           &NVM_FORMAT_METADATA_SIZE))
                {
                    print_Error_In_Cmd_Line_Args(NVM_FORMAT_METADATA_SIZE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_METADATA_SETTING_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "xlba") == 0)
                {
                    NVM_FORMAT_METADATA_SETTING = 0;
                }
                else if (strcmp(optarg, "separate") == 0)
                {
                    NVM_FORMAT_METADATA_SETTING = 1;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(NVM_FORMAT_METADATA_SETTING_LONG_OPT_STRING, optarg);
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
            else if (strcmp(longopts[optionIndex].name, NEW_FW_MATCH_LONG_OPT_STRING) == 0)
            {
                NEW_FW_MATCH_FLAG = true;
                snprintf_err_handle(NEW_FW_STRING_FLAG, NEW_FW_MATCH_STRING_LENGTH, "%s", optarg);
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
        case PROGRESS_SHORT_OPT: // get test progress for a specific test
            PROGRESS_CHAR = optarg;
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

    // the following flags imply running on all drives.
    if ((MODEL_MATCH_FLAG || FW_MATCH_FLAG || ONLY_SEAGATE_FLAG) && !RUN_ON_ALL_DRIVES)
    {
        RUN_ON_ALL_DRIVES = true;
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
    // need to make sure this is set when we are asked for SAT Info
    if (SAT_INFO_FLAG)
    {
        DEVICE_INFO_FLAG = goTrue;
    }
    // check that we were given at least one test to perform...if not, set that we are dumping device information so we
    // at least do something
    if (!(DEVICE_INFO_FLAG || LOWLEVEL_INFO_FLAG || DOWNLOAD_FW_FLAG || ACTIVATE_DEFERRED_FW_FLAG || SWITCH_FW_FLAG ||
          TEST_UNIT_READY_FLAG || CHECK_POWER_FLAG || (TRANSITION_POWER_STATE_TO >= 0) || SHOW_NVM_POWER_STATES ||
          (GET_NVME_LOG_IDENTIFIER > 0) // Since 0 is Reserved
          || (GET_TELEMETRY_IDENTIFIER > 0) || (GET_FEATURES_FLAG) || EXT_SMART_LOG_FLAG1 ||
          CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_FLAG || NVME_TEMP_STATS_FLAG || NVME_PCI_STATS_FLAG ||
          SHOW_SUPPORTED_FORMATS_FLAG || NVM_FORMAT_FLAG || PROGRESS_CHAR != M_NULLPTR || LIST_LOGS_FLAG
          // check for other tool specific options here
          ))
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
        if (FORCE_NVME_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\tForcing NVME Drive\n");
            }
            deviceList[deviceIter].drive_info.drive_type = NVME_DRIVE;
        }

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

        if (deviceList[deviceIter].drive_info.drive_type != NVME_DRIVE)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("%s - Not an NVMe device, skipping\n", deviceList[deviceIter].os_info.name);
            }
            ++skippedDevices;
            continue;
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
            if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_HUMAN)
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
            else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_RAW)
            {
                printf("Controller Identify Information:\n");
                printf("================================\n");
                print_Data_Buffer((uint8_t*)&deviceList[deviceIter].drive_info.IdentifyData.nvme.ctrl,
                                  sizeof(nvmeIDCtrl), true);
                printf("\nNamespace Identify Information:\n");
                printf("================================\n");
                print_Data_Buffer((uint8_t*)&deviceList[deviceIter].drive_info.IdentifyData.nvme.ns,
                                  sizeof(nvmeIDNameSpaces), true);
            }
            else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_BIN)
            {
                secureFileInfo* secureFile = M_NULLPTR;
                if (SUCCESS == create_And_Open_Secure_Log_File_Dev_EZ(&deviceList[deviceIter], &secureFile,
                                                                      NAMING_SERIAL_NUMBER_DATE_TIME, M_NULLPTR,
                                                                      "CTRL_IDENTIFY", "bin"))
                {
                    if (SEC_FILE_SUCCESS ==
                        secure_Write_File(secureFile, &deviceList[deviceIter].drive_info.IdentifyData.nvme.ctrl,
                                          NVME_IDENTIFY_DATA_LEN, sizeof(uint8_t), NVME_IDENTIFY_DATA_LEN, M_NULLPTR))
                    {
                        secure_Flush_File(secureFile);
                        if (SEC_FILE_SUCCESS != secure_Close_File(secureFile))
                        {
                            printf("Error closing file!\n");
                        }
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Created %s file with Controller Information\n", secureFile->fullpath);
                        }
                    }
                    else
                    {
                        if (secureFile->error == SEC_FILE_INSECURE_PATH)
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
                                printf("ERROR: failed to open file to write controller identify information\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                    }
                    free_Secure_File_Info(&secureFile);
                    if (SUCCESS == create_And_Open_Secure_Log_File_Dev_EZ(&deviceList[deviceIter], &secureFile,
                                                                          NAMING_SERIAL_NUMBER_DATE_TIME, M_NULLPTR,
                                                                          "NAMESPACE_IDENTIFY", "bin"))
                    {
                        if (SEC_FILE_SUCCESS ==
                            secure_Write_File(secureFile, &deviceList[deviceIter].drive_info.IdentifyData.nvme.ns,
                                              NVME_IDENTIFY_DATA_LEN, sizeof(uint8_t), NVME_IDENTIFY_DATA_LEN,
                                              M_NULLPTR))
                        {
                            secure_Flush_File(secureFile);
                            if (SEC_FILE_SUCCESS != secure_Close_File(secureFile))
                            {
                                printf("Error closing file!\n");
                            }
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Created %s file with Namespace Information\n", secureFile->fullpath);
                            }
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("ERROR: failed to open file to write namespace information\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                        free_Secure_File_Info(&secureFile);
                    }
                    else
                    {
                        if (secureFile->error == SEC_FILE_INSECURE_PATH)
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
                                printf("ERROR: failed to open file to write namespace identify information\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                    }
                    free_Secure_File_Info(&secureFile);
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: failed to open file to write controller information\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
                free_Secure_File_Info(&secureFile);
            }
        }

        if (LOWLEVEL_INFO_FLAG)
        {
            print_Low_Level_Info(&deviceList[deviceIter]);
        }

        if (LIST_LOGS_FLAG)
        {
            switch (print_Supported_Logs(&deviceList[deviceIter], 0))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nListing supported logs is not supported for device %s.\n",
                           deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to list logs for device %s\n", deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (SHOW_SUPPORTED_FORMATS_FLAG)
        {
            uint32_t memSize = C_CAST(uint32_t, sizeof(supportedFormats));
            ptrSupportedFormats formats = M_REINTERPRET_CAST(ptrSupportedFormats, safe_malloc(memSize));
            if (formats != M_NULLPTR)
            {
                safe_memset(formats, memSize, 0, memSize);
                switch (get_Supported_Formats(&deviceList[deviceIter], formats))
                {
                case SUCCESS:
                    show_Supported_Formats(formats);
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Device does not support showing supported formats\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to get supported sector sizes from device!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
                safe_free_supported_formats(&formats);
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unable to allocate memory for supported formats\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (GET_FEATURES_FLAG)
        {
            if (GET_FEATURES > UINT8_MAX) // List them all
            {
                nvme_Print_All_Feature_Identifiers(&deviceList[deviceIter], NVME_CURRENT_FEAT_SEL, false);
            }
            else
            {
                // Get the feature
                switch (nvme_Print_Feature_Details(&deviceList[deviceIter], C_CAST(uint8_t, GET_FEATURES),
                                                   NVME_CURRENT_FEAT_SEL))
                {
                case SUCCESS:
                    break;
                case NOT_SUPPORTED:
                    // This should really only be a "device does not support this", but there are a lot of features we
                    // cannot parse details for at this time, so for now this message is a bit more of a catch-all
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Either the requested feature is not supported by the device, or the\n");
                        printf("utility is unable to show details about the request feature at this time.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: failed to get details for feature id %d\n", GET_FEATURES);
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (GET_NVME_LOG_IDENTIFIER > 0) // Since 0 is reserved log
        {
            if (OUTPUT_MODE_IDENTIFIER != UTIL_OUTPUT_MODE_HUMAN)
            {
                uint64_t              size      = UINT64_C(0);
                uint8_t*              logBuffer = M_NULLPTR;
                nvmeGetLogPageCmdOpts cmdOpts;
                ret = nvme_Get_Log_Size(&deviceList[deviceIter], GET_NVME_LOG_IDENTIFIER, &size);
                if (ret == SUCCESS && size)
                {
                    safe_memset(&cmdOpts, sizeof(nvmeGetLogPageCmdOpts), 0, sizeof(nvmeGetLogPageCmdOpts));
                    if (NVME_LOG_ERROR_ID == GET_NVME_LOG_IDENTIFIER)
                    {
                        size = UINT64_C(32) * size; // Get first 32 entries.
                    }
                    logBuffer = M_REINTERPRET_CAST(uint8_t*, safe_calloc(uint64_to_sizet(size), sizeof(uint8_t)));
                    if (logBuffer != M_NULLPTR)
                    {
                        cmdOpts.nsid    = NVME_ALL_NAMESPACES;
                        cmdOpts.addr    = logBuffer;
                        cmdOpts.dataLen = C_CAST(uint32_t, size);
                        cmdOpts.lid     = GET_NVME_LOG_IDENTIFIER;
                        ret             = nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts);
                        if (ret == SUCCESS)
                        {
                            if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_RAW)
                            {
                                printf("Log Page %d Buffer:\n", GET_NVME_LOG_IDENTIFIER);
                                printf("================================\n");
                                print_Data_Buffer(C_CAST(uint8_t*, logBuffer), C_CAST(uint32_t, size), true);
                                printf("================================\n");
                            }
                            else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_BIN)
                            {
                                secureFileInfo* secureFile = M_NULLPTR;
#define SEACHEST_NVME_LOG_NAME_LENGTH 16
                                DECLARE_ZERO_INIT_ARRAY(char, logName, SEACHEST_NVME_LOG_NAME_LENGTH);
                                snprintf_err_handle(logName, SEACHEST_NVME_LOG_NAME_LENGTH, "LOG_PAGE_%d",
                                         GET_NVME_LOG_IDENTIFIER);
                                if (SUCCESS == create_And_Open_Secure_Log_File_Dev_EZ(
                                                   &deviceList[deviceIter], &secureFile, NAMING_SERIAL_NUMBER_DATE_TIME,
                                                   M_NULLPTR, logName, "bin"))
                                {
                                    if (SEC_FILE_SUCCESS == secure_Write_File(secureFile, logBuffer,
                                                                              uint64_to_sizet(size), sizeof(uint8_t),
                                                                              uint64_to_sizet(size), M_NULLPTR))
                                    {
                                        secure_Flush_File(secureFile);
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("Created %s with Log Page %" PRIu8 " Information\n",
                                                   secureFile->fullpath, GET_NVME_LOG_IDENTIFIER);
                                        }
                                    }
                                    else
                                    {
                                        if (secureFile->error == SEC_FILE_INSECURE_PATH)
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
                                                printf("ERROR: failed to write Log Page Information to file\n");
                                            }
                                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                        }
                                    }
                                    if (SEC_FILE_SUCCESS != secure_Close_File(secureFile))
                                    {
                                        printf("Error closing file!\n");
                                    }
                                }
                                else
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("ERROR: failed to open file to write Log Page Information\n");
                                    }
                                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                }
                                free_Secure_File_Info(&secureFile);
                            }
                            else
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Error: Unknown/Unsupported output mode %d\n", OUTPUT_MODE_IDENTIFIER);
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            }
                        }
                        else if (ret == NOT_SUPPORTED)
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Log Page %" PRIu8 " is not supported. \n", GET_NVME_LOG_IDENTIFIER);
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Error: Could not retrieve Log Page %" PRIu8 "\n", GET_NVME_LOG_IDENTIFIER);
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                        safe_free(&logBuffer);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Couldn't allocate %" PRIu64 " bytes buffer needed for Log Page %" PRIu8 "\n", size,
                                   GET_NVME_LOG_IDENTIFIER);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                }
                else if (ret == NOT_SUPPORTED)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Log Page %" PRIu8 " is not supported. \n", GET_NVME_LOG_IDENTIFIER);
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Log Page %" PRIu8 " not available at this time. \n", GET_NVME_LOG_IDENTIFIER);
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
            else // Human Readable.
            {
                switch (GET_NVME_LOG_IDENTIFIER)
                {
                case NVME_LOG_SMART_ID:
                    switch (show_NVMe_Health(&deviceList[deviceIter]))
                    {
                    case SUCCESS:
                        // nothing to print here since if it was successful, the log will be printed to the screen
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("SMART/Health Information Log not supported\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("A failure occured while trying to get SMART/Health Information Log\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    break;
                case NVME_LOG_ERROR_ID:
                    switch (nvme_Print_ERROR_Log_Page(&deviceList[deviceIter], 0))
                    {
                    case SUCCESS:
                        // nothing to print here since if it was successful, the log will be printed to the screen
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Error Information Log not supported\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("A failure occured while trying to get Error Information Log\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    break;
                case NVME_LOG_FW_SLOT_ID:
                    switch (nvme_Print_FWSLOTS_Log_Page(&deviceList[deviceIter]))
                    {
                    case SUCCESS:
                        // nothing to print here since if it was successful, the log will be printed to the screen
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("FW Slot Information Log not supported\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("A failure occured while trying to get FW Slot Information Log\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    break;
                case NVME_LOG_CMD_SPT_EFET_ID:
                    switch (nvme_Print_CmdSptEfft_Log_Page(&deviceList[deviceIter]))
                    {
                    case SUCCESS:
                        // nothing to print here since if it was successful, the log will be printed to the screen
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Commands Supported and Effects Information Log not supported\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("A failure occured while trying to get Commands Supported and Effects Information "
                                   "Log\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    break;
                case NVME_LOG_DEV_SELF_TEST_ID:
                    switch (nvme_Print_DevSelfTest_Log_Page(&deviceList[deviceIter]))
                    {
                    case SUCCESS:
                        // nothing to print here since if it was successful, the log will be printed to the screen
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Device Self-test Information Log not supported\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("A failure occured while trying to get Device Self-test Information Log\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Log Page %d not available at this time. \n", GET_NVME_LOG_IDENTIFIER);
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                }
            }
        }

        if (GET_TELEMETRY_IDENTIFIER > 0) // Since 0 is reserved log
        {
            if (TELEMETRY_DATA_AREA >= TELEMETRY_LOG_MIN_DATA_SET && TELEMETRY_DATA_AREA <= TELEMETRY_LOG_MAX_DATA_SET)
            {
                switch (pull_Telemetry_Log(&deviceList[deviceIter], GET_TELEMETRY_IDENTIFIER == 1 ? true : false,
                                           TELEMETRY_DATA_AREA, true, M_NULLPTR, 0, OUTPUTPATH_FLAG,
                                           LOG_TRANSFER_LENGTH_BYTES))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Telemetry log pulled successfully from device!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Telemetry log not supported by this device!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to pull telemetry log from this device!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must specify a Telemetry data set.\n\t1 - 4 are valid inputs for the data set on NVME "
                           "drives.\n");
                }
            }
        }

        if (NVME_TEMP_STATS_FLAG)
        {
            switch (nvme_Print_Temp_Statistics(&deviceList[deviceIter]))
            {
            case SUCCESS:
                // nothing to print here since if it was successful, the log will be printed to the screen
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Printing Temperature Statistics data is not supported on this drive type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A failure occured while trying to get Temp Statistics Information Log\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (NVME_PCI_STATS_FLAG)
        {
            switch (nvme_Print_PCI_Statistics(&deviceList[deviceIter]))
            {
            case SUCCESS:
                // nothing to print here since if it was successful, the log will be printed to the screen
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Printing PCI Statistics data is not supported on this drive type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A failure occured while trying to get PCI Statistics Information Log\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (TRANSITION_POWER_STATE_TO >= 0)
        {
            switch (transition_NVM_Power_State(&deviceList[deviceIter], C_CAST(uint8_t, TRANSITION_POWER_STATE_TO)))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully transitioned to power state %" PRIi32 ".\n", TRANSITION_POWER_STATE_TO);
                    printf("\nHint:Use --checkPowerMode option to check the new Power Mode State.\n\n");
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
                    printf("ERROR: Could not transition to the new power state %" PRIi32 "\n",
                           TRANSITION_POWER_STATE_TO);
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

        // this option must come after --transition power so that these two options can be combined on the command line
        // and produce the correct end result
        if (EXT_SMART_LOG_FLAG1)
        {
            switch (get_Ext_Smrt_Log(&deviceList[deviceIter]))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Extended smart logs not supported.\n");
                }
                break;
            default:
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed fetch Extended smart logs.\n");
                }
                break;
            }
        }

        if (CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_FLAG)
        {
            switch (clr_Pcie_Correctable_Errs(&deviceList[deviceIter]))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Clear Pcie correctable errors not supported.\n");
                }
                break;
            default:
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed fetch Clear Pcie correctable errors.\n");
                }
                break;
            }
        }

        if (DOWNLOAD_FW_FLAG)
        {
            secureFileInfo* fwfile = secure_Open_File(DOWNLOAD_FW_FILENAME_FLAG, "rb", M_NULLPTR, M_NULLPTR, M_NULLPTR);
            if (fwfile && fwfile->error == SEC_FILE_SUCCESS)
            {
                uint8_t* firmwareMem =
                    M_REINTERPRET_CAST(uint8_t*, safe_calloc_aligned(fwfile->fileSize, sizeof(uint8_t),
                                                                     deviceList[deviceIter].os_info.minimumAlignment));
                if (firmwareMem)
                {
                    if (SEC_FILE_SUCCESS == secure_Read_File(fwfile, firmwareMem, fwfile->fileSize, sizeof(uint8_t),
                                                             fwfile->fileSize, M_NULLPTR))
                    {
                        firmwareUpdateData dlOptions;
                        DECLARE_SEATIMER(commandTimer);
                        safe_memset(&dlOptions, sizeof(firmwareUpdateData), 0, sizeof(firmwareUpdateData));
                        dlOptions.size    = sizeof(firmwareUpdateData);
                        dlOptions.version = FIRMWARE_UPDATE_DATA_VERSION;
                        dlOptions.dlMode  = C_CAST(eFirmwareUpdateMode, DOWNLOAD_FW_MODE);
                        if (FWDL_SEGMENT_SIZE_FROM_USER)
                        {
                            dlOptions.segmentSize = FWDL_SEGMENT_SIZE_FLAG;
                        }
                        else
                        {
                            dlOptions.segmentSize = 0;
                        }
                        dlOptions.ignoreStatusOfFinalSegment = false;
                        dlOptions.firmwareFileMem            = firmwareMem;
                        dlOptions.firmwareMemoryLength       = C_CAST(
                                  uint32_t,
                                  fwfile->fileSize); // firmware files shouldn't be larger than a few MBs for a LONG time
                        dlOptions.firmwareSlot = FIRMWARE_SLOT_FLAG;
                        start_Timer(&commandTimer);
                        ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                        stop_Timer(&commandTimer);
                        switch (ret)
                        {
                        case SUCCESS:
                        case POWER_CYCLE_REQUIRED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Firmware Download successful\n");
                                printf("Firmware Download time");
                                print_Time(get_Nano_Seconds(commandTimer));
                                printf("Average time/segment ");
                                print_Time(dlOptions.avgSegmentDlTime);
                                if (ret != POWER_CYCLE_REQUIRED && dlOptions.activateFWTime > 0)
                                {
                                    printf("Activate Time         ");
                                    print_Time(dlOptions.activateFWTime);
                                }
                            }
                            if (ret == POWER_CYCLE_REQUIRED)
                            {
                                printf("The Operating system has reported that a power cycle is required to complete "
                                       "the firmware update\n");
                            }
                            if (DOWNLOAD_FW_MODE == FWDL_UPDATE_MODE_DEFERRED)
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Firmware download complete. Reboot or run the --%s command to finish "
                                           "installing the firmware.\n",
                                           ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
                                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                    {
                                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                                    }
                                }
                            }
                            else
                            {
                                fill_Drive_Info_Data(&deviceList[deviceIter]);
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    if (NEW_FW_MATCH_FLAG)
                                    {
                                        if (strcmp(NEW_FW_STRING_FLAG,
                                                   deviceList[deviceIter].drive_info.product_revision) == 0)
                                        {
                                            printf("Successfully validated firmware after download!\n");
                                            printf("New firmware version is %s\n",
                                                   deviceList[deviceIter].drive_info.product_revision);
                                        }
                                        else
                                        {
                                            printf("Unable to verify firmware after download!, expected %s, but found "
                                                   "%s\n",
                                                   NEW_FW_STRING_FLAG,
                                                   deviceList[deviceIter].drive_info.product_revision);
                                        }
                                    }
                                    else
                                    {
                                        printf("New firmware version is %s\n",
                                               deviceList[deviceIter].drive_info.product_revision);
                                    }
                                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                    {
                                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                                    }
                                }
                            }
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Firmware Download not supported\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Firmware Download failed\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Error reading contents of firmware file!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    safe_free_aligned(&firmwareMem);
                }
                else
                {
                    perror("failed to allocate memory");
                    if (fwfile)
                    {
                        if (SEC_FILE_SUCCESS != secure_Close_File(fwfile))
                        {
                            printf("Error attempting to close file!\n");
                        }
                        free_Secure_File_Info(&fwfile);
                    }
                    exit(255);
                }
            }
            else
            {
                if (fwfile->error == SEC_FILE_INSECURE_PATH)
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
                        printf("Couldn't open file %s\n", DOWNLOAD_FW_FILENAME_FLAG);
                    }
                    exitCode = UTIL_EXIT_CANNOT_OPEN_FILE;
                }
            }
            if (fwfile)
            {
                if (SEC_FILE_SUCCESS != secure_Close_File(fwfile))
                {
                    printf("Error attempting to close file!\n");
                }
                free_Secure_File_Info(&fwfile);
            }
        }

        if (ACTIVATE_DEFERRED_FW_FLAG || SWITCH_FW_FLAG)
        {
            supportedDLModes supportedFWDLModes;
            safe_memset(&supportedFWDLModes, sizeof(supportedDLModes), 0, sizeof(supportedDLModes));
            supportedFWDLModes.size    = sizeof(supportedDLModes);
            supportedFWDLModes.version = SUPPORTED_FWDL_MODES_VERSION;
            get_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes);
            if (supportedFWDLModes.deferred || supportedFWDLModes.scsiInfoPossiblyIncomplete)
            {
                firmwareUpdateData dlOptions;
                DECLARE_SEATIMER(commandTimer);
                safe_memset(&dlOptions, sizeof(firmwareUpdateData), 0, sizeof(firmwareUpdateData));
                dlOptions.size                 = sizeof(firmwareUpdateData);
                dlOptions.version              = FIRMWARE_UPDATE_DATA_VERSION;
                dlOptions.dlMode               = FWDL_UPDATE_MODE_ACTIVATE;
                dlOptions.segmentSize          = 0;
                dlOptions.firmwareFileMem      = M_NULLPTR;
                dlOptions.firmwareMemoryLength = 0;
                dlOptions.firmwareSlot         = FIRMWARE_SLOT_FLAG;
                if (SWITCH_FW_FLAG)
                {
                    dlOptions.existingFirmwareImage = true;
                }
                dlOptions.ignoreStatusOfFinalSegment =
                    false; // NOTE: This flag is not needed or used on products that support deferred download today.
                if (DOWNLOAD_FW_FLAG)
                {
                    // delay a second as this can help if running a download immediately followed by activate-TJE
                    delay_Seconds(1);
                }
                start_Timer(&commandTimer);
                ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                stop_Timer(&commandTimer);
                switch (ret)
                {
                case SUCCESS:
                case POWER_CYCLE_REQUIRED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Firmware activation successful\n");
                        if (ret == POWER_CYCLE_REQUIRED)
                        {
                            printf("The Operating system has reported that a power cycle is required to complete the "
                                   "firmware update\n");
                        }
                        else
                        {
                            fill_Drive_Info_Data(&deviceList[deviceIter]);
                            if (NEW_FW_MATCH_FLAG)
                            {
                                if (strcmp(NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision) == 0)
                                {
                                    printf("Successfully validated firmware after download!\n");
                                    printf("New firmware version is %s\n",
                                           deviceList[deviceIter].drive_info.product_revision);
                                }
                                else
                                {
                                    printf("Unable to verify firmware after download!, expected %s, but found %s\n",
                                           NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision);
                                }
                            }
                            else
                            {
                                printf("New firmware version is %s\n",
                                       deviceList[deviceIter].drive_info.product_revision);
                            }
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
                        if (SWITCH_FW_FLAG && FIRMWARE_SLOT_FLAG == 0)
                        {
                            printf("You must specify a valid slot number when switching firmware images.\n");
                        }
                        else
                        {
                            printf("Firmware activate not supported\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Firmware activation failed\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("This drive does not support the activate command.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (NVM_FORMAT_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("NVM Format\n");
            }
            if (DATA_ERASE_FLAG)
            {
                runNVMFormatParameters nvmformatParameters;
                safe_memset(&nvmformatParameters, sizeof(runNVMFormatParameters), 0, sizeof(runNVMFormatParameters));
                if (NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM >= 16 && NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM <= 512)
                {
                    nvmformatParameters.formatNumberProvided     = false;
                    nvmformatParameters.newSize.currentBlockSize = true;
                }
                else if (/* NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM >= 0 && */ NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM < 16)
                {
                    nvmformatParameters.formatNumberProvided = true;
                    nvmformatParameters.formatNumber         = C_CAST(uint8_t, NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM);
                }
                else
                {
                    nvmformatParameters.formatNumberProvided     = false;
                    nvmformatParameters.newSize.currentBlockSize = false;
                    nvmformatParameters.newSize.newBlockSize     = NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM;
                }
                if (NVM_FORMAT_METADATA_SIZE != UINT32_MAX && !nvmformatParameters.formatNumberProvided)
                {
                    nvmformatParameters.newSize.changeMetadataSize = true;
                    nvmformatParameters.newSize.metadataSize       = C_CAST(uint16_t, NVM_FORMAT_METADATA_SIZE);
                }
                if (NVM_FORMAT_NSID != UINT32_MAX)
                {
                    nvmformatParameters.currentNamespace = true;
                }
                nvmformatParameters.secureEraseSettings = NVM_FORMAT_SECURE_ERASE;
                // PI
                switch (NVM_FORMAT_PI_TYPE)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                    nvmformatParameters.changeProtectionType = true;
                    nvmformatParameters.protectionType       = NVM_FORMAT_PI_TYPE;
                    break;
                default:
                    break;
                }
                // PIL
                switch (NVM_FORMAT_PI_LOCATION)
                {
                case 0:
                    nvmformatParameters.protectionLocation.valid       = true;
                    nvmformatParameters.protectionLocation.first8Bytes = true;
                    break;
                case 1:
                    nvmformatParameters.protectionLocation.valid       = true;
                    nvmformatParameters.protectionLocation.first8Bytes = false;
                    break;
                default:
                    break;
                }
                // metadata settings
                switch (NVM_FORMAT_METADATA_SETTING)
                {
                case 0:
                    nvmformatParameters.metadataSettings.valid                 = true;
                    nvmformatParameters.metadataSettings.metadataAsExtendedLBA = true;
                    break;
                case 1:
                    nvmformatParameters.metadataSettings.valid                 = true;
                    nvmformatParameters.metadataSettings.metadataAsExtendedLBA = false;
                    break;
                default:
                    break;
                }
                eReturnValues formatRet = run_NVMe_Format(&deviceList[deviceIter], nvmformatParameters, POLL_FLAG);
                switch (formatRet)
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (POLL_FLAG)
                        {
                            printf("NVM Format was Successful!\n");
                        }
                        else
                        {
                            printf("NVM Format was started Successfully!\n");
                            printf("Use --%s nvmformat to check for progress.\n", PROGRESS_LONG_OPT_STRING);
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("NVM Format Not Supported or invalid option combination provided!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                case OS_COMMAND_NOT_AVAILABLE:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("NVM Format is not supported in this OS\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("NVM Format Failed!\n");
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
                    printf("to the command line arguments to run a nvm format.\n\n");
                    printf("e.g.: %s -d %s --%s current --confirm %s\n\n", util_name, deviceHandleExample,
                           NVM_FORMAT_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (PROGRESS_CHAR != M_NULLPTR)
        {
            eReturnValues result = UNKNOWN;
            // first take whatever was entered in progressTest and convert it to uppercase to do fewer string
            // comparisons
            convert_String_To_Upper_Case(progressTest);
            // do some string comparisons to figure out what we are checking for progress on
            if (strcmp(progressTest, "NVMFORMAT") == 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting NVM Format Progress.\n");
                }
                result = show_Format_Unit_Progress(&deviceList[deviceIter]);
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\"%s\" does not report progress.\n", progressTest);
                }
            }
            switch (result)
            {
            case UNKNOWN:
                break;
            case SUCCESS:
                break;
            case IN_PROGRESS:
                break;
            case ABORTED:
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
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
    printf("\tUpdating firmware:\n");
    printf("\t%s -d %s --%s file.bin\n", util_name, deviceHandleExample, DOWNLOAD_FW_LONG_OPT_STRING);
    printf("\tUpdating firmware with deferred download and activating:\n");
    printf("\t%s -d %s --%s file.bin --%s deferred --%s\n", util_name, deviceHandleExample, DOWNLOAD_FW_LONG_OPT_STRING,
           DOWNLOAD_FW_MODE_LONG_OPT_STRING, ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
    printf("\tUpdating firmware and specifying a firmware slot (NVMe)\n");
    printf("\t%s -d %s --%s file.bin --%s deferred\n", util_name, deviceHandleExample, DOWNLOAD_FW_LONG_OPT_STRING,
           DOWNLOAD_FW_MODE_LONG_OPT_STRING);
    printf("\t  +\n");
    printf("\t%s -d %s --%s --%s 2\n", util_name, deviceHandleExample, ACTIVATE_DEFERRED_FW_LONG_OPT_STRING,
           FIRMWARE_SLOT_LONG_OPT_STRING);
    printf("\tFormatting an NVMe device:\n");
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_SUPPORTED_FORMATS_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 4096 --%s\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s user\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING, NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s 1\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING, NVM_FORMAT_PI_TYPE_LONG_OPT_STRING);
    printf("\tChecking and changing power states:\n");
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, CHECK_POWER_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 1\n", util_name, deviceHandleExample, TRANSITION_POWER_STATE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_NVM_POWER_STATES_LONG_OPT_STRING);
    printf("\tPulling the Telemetry log:\n");
    printf("\t%s -d %s --%s host\n", util_name, deviceHandleExample, GET_TELEMETRY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s host, --%s 2 --%s bin\n", util_name, deviceHandleExample, GET_TELEMETRY_LONG_OPT_STRING,
           TELEMETRY_DATA_AREA_LONG_OPT_STRING, OUTPUT_MODE_LONG_OPT_STRING);

    // return codes
    printf("Return codes\n");
    printf("============\n");
    print_SeaChest_Util_Exit_Codes(0, M_NULLPTR, util_name);

    // utility options
    printf("\nUtility Options\n");
    printf("===============\n");
    print_Echo_Command_Line_Help(shortUsage);
    print_Help_Help(shortUsage);
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_New_Firmware_Revision_Match_Help(shortUsage);
    print_No_Banner_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    // the test options
    printf("Utility Arguments\n");
    printf("=================\n");
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Device_Information_Help(shortUsage);
    print_Low_Level_Info_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    // utility tests/operations go here
    print_Fast_Discovery_Help(shortUsage);
    print_Firmware_Activate_Help(shortUsage);
    print_Check_Power_Mode_Help(shortUsage);
    print_Transition_Power_State_Help(shortUsage);
    print_Firmware_Download_Help(shortUsage);
    print_Firmware_Download_Mode_Help(shortUsage);
    print_Firmware_Slot_Buffer_ID_Help(shortUsage);
    print_FWDL_Segment_Size_Help(shortUsage);
    print_Get_Features_Help(shortUsage);
    print_NVMe_Get_Log_Help(shortUsage);
    print_NVMe_Get_Tele_Help(shortUsage);
    print_Supported_Logs_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_pcierr_Help(shortUsage);
    print_Poll_Help(shortUsage);
    print_Progress_Help(shortUsage, "nvmformat");
    print_extSmatLog_Help(shortUsage);
    print_Output_Mode_Help(shortUsage);
    print_NVMe_Temp_Stats_Help(shortUsage);
    print_NVMe_Pci_Stats_Help(shortUsage);
    print_Show_NVM_Power_States_Help(shortUsage);
    print_Show_Supported_Formats_Help(shortUsage);

    // data destructive commands
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    // utility data destructive tests/operations go here
    print_NVM_Format_Metadata_Setting_Help(shortUsage);
    print_NVM_Format_Metadata_Size_Help(shortUsage);
    print_NVM_Format_NSID_Help(shortUsage);
    print_NVM_Format_PI_Type_Help(shortUsage);
    print_NVM_Format_PIL_Help(shortUsage);
    print_NVM_Format_Secure_Erase_Help(shortUsage);
    print_NVM_Format_Help(shortUsage);
}
