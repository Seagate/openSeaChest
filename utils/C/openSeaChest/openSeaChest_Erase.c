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
// \file OpenSeaChest_Erase.c Binary command line that performs various erase methods on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "io_utils.h"
#include "math_utils.h"
#include "memory_safety.h"
#include "pattern_utils.h"
#include "secure_file.h"
#include "sleep.h"
#include "string_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"
#if defined(_WIN32)
#    include "windows_version_detect.h" //for WinPE check
#endif
#include "EULA.h"
#include "ata_helper.h" //for defined ATA security password size of 32bytes
#include "cmds.h"
#include "getopt.h"
#include "openseachest_util_options.h"
#if !defined(DISABLE_TCG_SUPPORT)
#    include "common_TCG.h"
#    include "genkey.h"
#    include "operations.h"
#    include "port_locking.h"
#    include "revert.h"
#    include "revertSP.h"
#endif
#include "ata_Security.h"
#include "drive_info.h"
#include "format.h"
#include "generic_tests.h"
#include "host_erase.h"
#include "platform_helper.h"
#include "sanitize.h"
#include "set_max_lba.h"
#include "trim_unmap.h"
#include "writesame.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name    = "openSeaChest_Erase";
const char* buildVersion = "4.6.0";

typedef enum eSeaChestEraseExitCodesEnum
{
    SEACHEST_ERASE_EXIT_ZERO_VALIDATION_FAILURE = UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE, // Zero validation failure
    SEACHEST_ERASE_EXIT_MAX_ERROR // don't acutally use this, just for memory allocation below
} eSeaChestEraseExitCodes;

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
    POSSIBLE_DATA_ERASE_VAR
    LOW_LEVEL_FORMAT_VAR
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
    PROGRESS_VAR
    DISPLAY_LBA_VAR
    PATTERN_VARS
    MAX_LBA_VARS
    // tool specific command line flags
    // generic erase
    POLL_VAR
#if !defined(DISABLE_TCG_SUPPORT)
    TCG_SID_VARS
    TCG_PSID_VARS
    // tcg revert SP
    TCG_REVERT_SP_VARS
    // tcg revert
    TCG_REVERT_VAR
#endif
    // ata security
    ATA_SECURITY_PASSWORD_MODIFICATIONS_VAR
    ATA_SECURITY_PASSWORD_VARS
    ATA_SECURITY_USING_MASTER_PW_VAR
    ATA_SECURITY_ERASE_OP_VARS
    ATA_SECURITY_FORCE_SAT_VARS
    // sanitize
    SANITIZE_FREEZE_VAR
    SANITIZE_ANTIFREEZE_VAR
    SANITIZE_UTIL_VARS
    SANITIZE_AUSE_VAR
    SANITIZE_IPBP_VAR
    SANITIZE_OVERWRITE_PASSES_VAR
    ZONE_NO_RESET_VAR
    NO_DEALLOCATE_AFTER_ERASE_VAR //NVMe Sanitize option only at this time.
    // write same
    WRITE_SAME_UTIL_VARS
    // tcgGenkey
    // bool                tcgGenKey                   = false;
    TRIM_UNMAP_VARS
    OVERWRITE_VARS
    FORMAT_UNIT_VARS
    FAST_FORMAT_VAR
    SHOW_ERASE_SUPPORT_VAR
    PERFORM_FASTEST_ERASE_VAR
    NVM_FORMAT_VARS
    NVM_FORMAT_OPTION_VARS
    // time flags
    HOURS_TIME_VAR
    MINUTES_TIME_VAR
    SECONDS_TIME_VAR
    ZERO_VERIFY_VARS
#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    HIDE_LBA_COUNTER_VAR
    LOWLEVEL_INFO_VAR
    ERASE_RESTORE_MAX_VAR
    REFRESH_FILE_SYSTEMS_VAR

        // clang-format on

        int args        = 0;
    int     argIndex    = 0;
    int     optionIndex = 0;

    struct option longopts[] =
    {
        // common command line options
        DEVICE_LONG_OPT,
        HELP_LONG_OPT,
        DEVICE_INFO_LONG_OPT,
        SAT_INFO_LONG_OPT,
        USB_CHILD_INFO_LONG_OPT,
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
        POLL_LONG_OPT,
        CONFIRM_LONG_OPT,
        PROGRESS_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined(ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        // time vars
        HOURS_TIME_LONG_OPT,
        MINUTES_TIME_LONG_OPT,
        SECONDS_TIME_LONG_OPT,
        LOWLEVEL_INFO_LONG_OPT,
        // tool specific command line options --These should probably be cleaned up to macros like the ones above and
        // remove the short options and replace with numbers.
        OVERWRITE_LONG_OPTS,
        TRIM_LONG_OPTS,
        UNMAP_LONG_OPTS,
#if !defined(DISABLE_TCG_SUPPORT)
        TCG_REVERT_SP_LONG_OPT,
        TCG_REVERT_LONG_OPT,
        TCG_SID_LONG_OPT,
        TCG_PSID_LONG_OPT,
#endif
        SANITIZE_FREEZE_LONG_OPT,
        SANITIZE_ANTIFREEZE_LONG_OPT,
        SANITIZE_LONG_OPT,
        SANITIZE_AUSE_LONG_OPT,
        SANITIZE_IPBP_LONG_OPT,
        SANITIZE_OVERWRITE_PASSES_LONG_OPT,
        ZONE_NO_RESET_LONG_OPT,
        NO_DEALLOCATE_AFTER_ERASE_LONG_OPT,
        WRITE_SAME_LONG_OPTS,
        SHOW_ERASE_SUPPORT_LONG_OPT,
        PERFORM_FASTEST_ERASE_LONG_OPT,
        FORMAT_UNIT_LONG_OPT,
        FAST_FORMAT_LONG_OPT,
        DISPLAY_LBA_LONG_OPT,
        PATTERN_LONG_OPT,
        HIDE_LBA_COUNTER_LONG_OPT,
        ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT,
        ATA_SECURITY_PASSWORD_LONG_OPT,
        ATA_SECURITY_USING_MASTER_PW_LONG_OPT,
        ATA_SECURITY_ERASE_OP_LONG_OPT,
        ATA_SECURITY_FORCE_SAT_LONG_OPT,
        NVM_FORMAT_LONG_OPT,
        NVM_FORMAT_OPTIONS_LONG_OPTS,
        ZERO_VERIFY_LONG_OPT,
        ERASE_RESTORE_MAX_PREP_LONG_OPT,
        REFRESH_FILE_SYSTEMS_LONG_OPT,
        LONG_OPT_TERMINATOR
    };

    eVerbosityLevels toolVerbosity = VERBOSITY_DEFAULT;

#if defined(UEFI_C_SOURCE)
    // NOTE: This is a BSD function used to ensure the program name is set correctly for warning or error functions.
    //       This is not necessary on most modern systems other than UEFI.
    //       This is not used in linux so that we don't depend on libbsd
    //       Update the above #define check if we port to another OS that needs this to be done.
    setprogname(util_name);
#endif

    ////////////////////////
    //  Argument Parsing  //
    ////////////////////////
    if (argc < 2)
    {
        openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
        utility_Usage(true);
        printf("\n");
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    // get options we know we need
    while (1) // changed to while 1 in order to parse multiple options when longs options are involved
    {
        args = getopt_long(argc, argv, "d:hisSF:Vv:qr:R:E:e:w:", longopts, &optionIndex);
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
                else if (strcmp(optarg, LOW_LEVEL_FORMAT_ACCEPT_STRING) == 0)
                {
                    LOW_LEVEL_FORMAT_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, TRIM_RANGE_LONG_OPT_STRING) == 0 ||
                     strcmp(longopts[optionIndex].name, UNMAP_RANGE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                           &TRIM_UNMAP_RANGE_FLAG))
                {
                    if (strcmp(longopts[optionIndex].name, TRIM_RANGE_LONG_OPT_STRING) == 0)
                    {
                        print_Error_In_Cmd_Line_Args(TRIM_RANGE_LONG_OPT_STRING, optarg);
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(UNMAP_RANGE_LONG_OPT_STRING, optarg);
                    }
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, TRIM_LONG_OPT_STRING) == 0 ||
                     strcmp(longopts[optionIndex].name, UNMAP_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &TRIM_UNMAP_START_FLAG))
                {
                    RUN_TRIM_UNMAP_FLAG = true;
                }
                else
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA         = true;
                        RUN_TRIM_UNMAP_FLAG = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA   = true;
                        RUN_TRIM_UNMAP_FLAG = true;
                    }
                    else
                    {
                        if (strcmp(longopts[optionIndex].name, TRIM_LONG_OPT_STRING) == 0)
                        {
                            print_Error_In_Cmd_Line_Args(TRIM_LONG_OPT_STRING, optarg);
                        }
                        else
                        {
                            print_Error_In_Cmd_Line_Args(UNMAP_LONG_OPT_STRING, optarg);
                        }
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, OVERWRITE_RANGE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                           &OVERWRITE_RANGE_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(OVERWRITE_RANGE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, OVERWRITE_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &OVERWRITE_START_FLAG))
                {
                    RUN_OVERWRITE_FLAG = true;
                }
                else
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA        = true;
                        RUN_OVERWRITE_FLAG = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA  = true;
                        RUN_OVERWRITE_FLAG = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(OVERWRITE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, HOURS_TIME_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &HOURS_TIME_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(HOURS_TIME_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, MINUTES_TIME_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint16(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &MINUTES_TIME_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(MINUTES_TIME_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SECONDS_TIME_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint32(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &SECONDS_TIME_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(SECONDS_TIME_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, WRITE_SAME_RANGE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                           &WRITE_SAME_RANGE_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(WRITE_SAME_RANGE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, WRITE_SAME_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &WRITE_SAME_START_FLAG))
                {
                    RUN_WRITE_SAME_FLAG = true;
                }
                else
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA         = true;
                        RUN_WRITE_SAME_FLAG = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA   = true;
                        RUN_WRITE_SAME_FLAG = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(WRITE_SAME_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
#if !defined(DISABLE_TCG_SUPPORT)
            else if (strcmp(longopts[optionIndex].name, TCG_SID_LONG_OPT_STRING) == 0)
            {
                snprintf_err_handle(TCG_SID_FLAG, TCG_SID_BUF_LEN, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, TCG_PSID_LONG_OPT_STRING) == 0)
            {
                snprintf_err_handle(TCG_PSID_FLAG, TCG_PSID_BUF_LEN, "%s", optarg);
            }
#endif
            else if (strcmp(longopts[optionIndex].name, FORMAT_UNIT_LONG_OPT_STRING) == 0)
            {
                FORMAT_UNIT_FLAG = true;
                if (strcmp(optarg, "current") != 0)
                {
                    if (!get_And_Validate_Integer_Input_Uint16(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                               &FORMAT_SECTOR_SIZE))
                    {
                        print_Error_In_Cmd_Line_Args(FORMAT_UNIT_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_LONG_OPT_STRING) == 0)
            {
                NVM_FORMAT_FLAG = true;
                if (strcmp(optarg, "current") != 0)
                {
                    if (!get_And_Validate_Integer_Input_Uint32(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
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
                    NVM_FORMAT_SECURE_ERASE = NVM_FMT_SE_CRYPTO;
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
            else if (strcmp(longopts[optionIndex].name, DISPLAY_LBA_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &DISPLAY_LBA_THE_LBA))
                {
                    DISPLAY_LBA_FLAG = true;
                }
                else
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA      = true;
                        DISPLAY_LBA_FLAG = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
                        DISPLAY_LBA_FLAG  = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(DISPLAY_LBA_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, FAST_FORMAT_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_I(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &FAST_FORMAT_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(FAST_FORMAT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_SECURITY_FORCE_SAT_LONG_OPT_STRING) == 0)
            {
                ATA_SECURITY_FORCE_SAT_VALID = true;
                if (strcmp(optarg, "enable") == 0)
                {
                    ATA_SECURITY_FORCE_SAT = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    ATA_SECURITY_FORCE_SAT = false;
                }
                else
                {
                    ATA_SECURITY_FORCE_SAT_VALID = false;
                    // error in command line
                    print_Error_In_Cmd_Line_Args(ATA_SECURITY_FORCE_SAT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_SECURITY_ERASE_OP_LONG_OPT_STRING) == 0)
            {
                ATA_SECURITY_ERASE_OP = true;
                if (strcmp(optarg, "enhanced") == 0)
                {
                    ATA_SECURITY_ERASE_ENHANCED_FLAG = true;
                }
                else if (strcmp(optarg, "normal") == 0)
                {
                    ATA_SECURITY_ERASE_ENHANCED_FLAG = false;
                }
                else
                {
                    ATA_SECURITY_ERASE_OP = false;
                    // error in command line
                    print_Error_In_Cmd_Line_Args(ATA_SECURITY_ERASE_OP_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "master") == 0)
                {
                    ATA_SECURITY_USING_MASTER_PW = true;
                }
                else if (strcmp(optarg, "user") == 0)
                {
                    ATA_SECURITY_USING_MASTER_PW = false;
                }
                else
                {
                    // error in command line
                    print_Error_In_Cmd_Line_Args(ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_SECURITY_PASSWORD_LONG_OPT_STRING) == 0)
            {
                ATA_SECURITY_USER_PROVIDED_PASS = true;
                if (strcmp(optarg, "empty") == 0)
                {
                    // the modification option can change this to all F's if desired instead of zeros.
                    safe_memset(&ATA_SECURITY_PASSWORD[0], 32, 0, ATA_SECURITY_MAX_PW_LENGTH);
                    ATA_SECURITY_PASSWORD_BYTE_COUNT = ATA_SECURITY_MAX_PW_LENGTH;
                }
                else if (strcmp(optarg, "SeaChest") == 0)
                {
                    ATA_SECURITY_PASSWORD_BYTE_COUNT = C_CAST(uint8_t, safe_strlen("SeaChest"));
                    safe_memcpy(ATA_SECURITY_PASSWORD, 32, "SeaChest", ATA_SECURITY_PASSWORD_BYTE_COUNT);
                }
                else
                {
                    // If the user quoted their password when putting on the cmdline, then we can accept spaces.
                    // Otherwise spaces cannot be picked up.
                    if (safe_strlen(optarg) > ATA_SECURITY_MAX_PW_LENGTH)
                    {
                        print_Error_In_Cmd_Line_Args(ATA_SECURITY_PASSWORD_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    // printf("User entered \"%s\" for their password\n", optarg);
                    safe_memcpy(ATA_SECURITY_PASSWORD, 32, optarg,
                                safe_strlen(optarg)); // make sure we don't try copying over a null terminator because
                                                      // we just need to store the 32bytes of characters provided.
                    ATA_SECURITY_PASSWORD_BYTE_COUNT =
                        C_CAST(uint8_t, M_Min(safe_strlen(optarg), ATA_SECURITY_MAX_PW_LENGTH));
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "byteswap") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.byteSwapped = true;
                }
                else if (strcmp(optarg, "zeropad") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.zeroPadded = true;
                    if (ATA_SECURITY_PASSWORD_MODIFICATIONS.spacePadded || ATA_SECURITY_PASSWORD_MODIFICATIONS.fpadded)
                    {
                        // todo: print error saying invalid argument combination.
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
                else if (strcmp(optarg, "spacepad") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.spacePadded = true;
                    if (ATA_SECURITY_PASSWORD_MODIFICATIONS.zeroPadded || ATA_SECURITY_PASSWORD_MODIFICATIONS.fpadded)
                    {
                        // todo: print error saying invalid argument combination.
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
                else if (strcmp(optarg, "fpad") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.fpadded = true;
                    if (ATA_SECURITY_PASSWORD_MODIFICATIONS.spacePadded ||
                        ATA_SECURITY_PASSWORD_MODIFICATIONS.zeroPadded)
                    {
                        // todo: print error saying invalid argument combination.
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
                else if (strcmp(optarg, "leftAlign") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.leftAligned = true;
                    if (ATA_SECURITY_PASSWORD_MODIFICATIONS.rightAligned)
                    {
                        // todo: print error saying invalid argument combination.
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
                else if (strcmp(optarg, "rightAlign") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.rightAligned = true;
                    if (ATA_SECURITY_PASSWORD_MODIFICATIONS.leftAligned)
                    {
                        // todo: print error saying invalid argument combination.
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
                else if (strcmp(optarg, "uppercase") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.forceUppercase = true;
                    if (ATA_SECURITY_PASSWORD_MODIFICATIONS.forceLowercase ||
                        ATA_SECURITY_PASSWORD_MODIFICATIONS.invertCase)
                    {
                        // todo: print error saying invalid argument combination.
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
                else if (strcmp(optarg, "lowercase") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.forceLowercase = true;
                    if (ATA_SECURITY_PASSWORD_MODIFICATIONS.forceUppercase ||
                        ATA_SECURITY_PASSWORD_MODIFICATIONS.invertCase)
                    {
                        // todo: print error saying invalid argument combination.
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
                else if (strcmp(optarg, "invertcase") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.invertCase = true;
                    if (ATA_SECURITY_PASSWORD_MODIFICATIONS.forceLowercase ||
                        ATA_SECURITY_PASSWORD_MODIFICATIONS.forceUppercase)
                    {
                        // todo: print error saying invalid argument combination.
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
#if defined(MD5_PASSWORD_SUPPORTED)
                else if (strcmp(optarg, "md5") == 0)
                {
                    ATA_SECURITY_PASSWORD_MODIFICATIONS.md5Hash = true;
                }
#endif
                // handle other modifications
                // TODO: handle bad combinations of modifications
                else
                {
                    print_Error_In_Cmd_Line_Args(ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING, optarg);
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
            else if (strcmp(longopts[optionIndex].name, PATTERN_LONG_OPT_STRING) == 0)
            {
                PATTERN_FLAG = true;
                if (strcmp("random", optarg) == 0)
                {
                    fill_Random_Pattern_In_Buffer(PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
                }
                else
                {
                    char* colonLocation = strstr(optarg, ":") +
                                          1; // adding 1 to offset just beyond the colon for parsing the remaining data
                    if (colonLocation && strncmp("file:", optarg, 5) == 0)
                    {
                        fileExt         allowedExt[] = {{".bin", false}, {".BIN", false}, {M_NULLPTR, false}};
                        secureFileInfo* fileinfo =
                            secure_Open_File(colonLocation, "rb", allowedExt, M_NULLPTR, M_NULLPTR);
                        if (fileinfo)
                        {
                            if (fileinfo->error == SEC_FILE_SUCCESS)
                            {
                                if (SEC_FILE_SUCCESS != secure_Read_File(fileinfo, PATTERN_BUFFER,
                                                                         PATTERN_BUFFER_LENGTH, sizeof(uint8_t),
                                                                         fileinfo->fileSize, M_NULLPTR))
                                {
                                    printf("Unable to read contents of the file \"%s\" for the pattern.\n",
                                           fileinfo->filename);
                                    if (SEC_FILE_SUCCESS != secure_Close_File(fileinfo))
                                    {
                                        printf("secure file structure could not be closed! This is a fatal error!\n");
                                    }
                                    free_Secure_File_Info(&fileinfo);
                                    exit(UTIL_EXIT_CANNOT_OPEN_FILE);
                                }
                            }
                            else
                            {
                                printf("Unable to open file \"%s\" for pattern\n", colonLocation);
                                exit(UTIL_EXIT_CANNOT_OPEN_FILE);
                            }
                            if (SEC_FILE_SUCCESS != secure_Close_File(fileinfo))
                            {
                                printf("secure file structure could not be closed! This is a fatal error!\n");
                            }
                            free_Secure_File_Info(&fileinfo);
                        }
                        else
                        {
                            printf("Unable to open file \"%s\" for pattern\n", colonLocation);
                            exit(UTIL_EXIT_CANNOT_OPEN_FILE);
                        }
                    }
                    else if (colonLocation && strncmp("increment:", optarg, 10) == 0)
                    {
                        uint8_t incrementStart = UINT8_C(0);
                        if (get_And_Validate_Integer_Input_Uint8(colonLocation, M_NULLPTR, ALLOW_UNIT_NONE,
                                                                 &incrementStart))
                        {
                            fill_Incrementing_Pattern_In_Buffer(incrementStart, PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
                        }
                        else
                        {
                            print_Error_In_Cmd_Line_Args(PATTERN_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    else if (colonLocation && strncmp("repeat:", optarg, 7) == 0)
                    {
                        // if final character is a lower case h, it's an hex pattern
                        if (colonLocation[safe_strlen(colonLocation) - 1] == 'h' && safe_strlen(colonLocation) == 9)
                        {
                            unsigned long temp = 0UL;
                            if (0 == safe_strtoul(&temp, colonLocation, M_NULLPTR, BASE_16_HEX))
                            {
                                uint32_t hexPattern = M_STATIC_CAST(uint32_t, temp);
                                // TODO: add endianness check before byte swap
                                byte_Swap_32(&hexPattern);
                                fill_Hex_Pattern_In_Buffer(hexPattern, PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
                            }
                            else
                            {
                                print_Error_In_Cmd_Line_Args(PATTERN_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        else
                        {
                            fill_ASCII_Pattern_In_Buffer(colonLocation, C_CAST(uint32_t, safe_strlen(colonLocation)),
                                                         PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
                        }
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(PATTERN_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, ZERO_VERIFY_LONG_OPT_STRING) == 0)
            {
                ZERO_VERIFY_FLAG = true;
                if (strcmp(optarg, "full") == 0)
                {
                    ZERO_VERIFY_MODE_FLAG = ZERO_VERIFY_TYPE_FULL;
                }
                else if (strcmp(optarg, "quick") == 0)
                {
                    ZERO_VERIFY_MODE_FLAG = ZERO_VERIFY_TYPE_QUICK;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(ZERO_VERIFY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SANITIZE_OVERWRITE_PASSES_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &SANITIZE_OVERWRITE_PASSES))
                {
                    print_Error_In_Cmd_Line_Args(SANITIZE_OVERWRITE_PASSES_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            break;
        case PROGRESS_SHORT_OPT: // get test progress for a specific test
            PROGRESS_CHAR = optarg;
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
            openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
            utility_Usage(false);
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\n");
            }
            exit(UTIL_EXIT_NO_ERROR);
        case 'e': // sanitize
            if (strcmp(SANITIZE_BLOCK_ERASE_STR, optarg) == 0)
            {
                SANITIZE_RUN_FLAG        = true;
                SANITIZE_RUN_BLOCK_ERASE = true;
            }
            else if (strcmp(SANITIZE_CRYPTO_ERASE_STR, optarg) == 0)
            {
                SANITIZE_RUN_FLAG         = true;
                SANITIZE_RUN_CRYPTO_ERASE = true;
            }
            else if (strcmp(SANITIZE_OVERWRITE_ERASE_STR, optarg) == 0)
            {
                SANITIZE_RUN_FLAG            = true;
                SANITIZE_RUN_OVERWRITE_ERASE = true;
            }
            else if (strcmp("freezelock", optarg) == 0)
            {
                SANITIZE_FREEZE = true;
                printf("Please migrate to using --%s as this will be removed in future versions\n",
                       SANITIZE_FREEZE_LONG_OPT_STRING);
            }
            else if (strcmp("antifreezelock", optarg) == 0)
            {
                SANITIZE_ANTIFREEZE = true;
                printf("Please migrate to using --%s as this will be removed in future versions\n",
                       SANITIZE_ANTIFREEZE_LONG_OPT_STRING);
            }
            else if (strcmp("info", optarg) == 0)
            {
                SANITIZE_INFO_FLAG = true;
            }
            else
            {
                print_Error_In_Cmd_Line_Args(SANITIZE_LONG_OPT_STRING, optarg);
                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
            }
            break;
        default:
            break;
        }
    }

    atexit(print_Final_newline);

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
        openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
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

    // set any ATA security password modifications that we may need.
    if (ATA_SECURITY_USER_PROVIDED_PASS)
    {
        // check if any password modifications were given and apply them here.
        // check if forcing a specific case before we do things that would make it more difficult for opensea-common to
        // modify
        if (ATA_SECURITY_PASSWORD_MODIFICATIONS.forceUppercase)
        {
            // change all to uppercase
            DECLARE_ZERO_INIT_ARRAY(char, thePassword, ATA_SECURITY_MAX_PW_LENGTH + 1);
            safe_memcpy(thePassword, ATA_SECURITY_MAX_PW_LENGTH + 1, ATA_SECURITY_PASSWORD, ATA_SECURITY_MAX_PW_LENGTH);
            convert_String_To_Upper_Case(thePassword);
            safe_memcpy(ATA_SECURITY_PASSWORD, 32, thePassword, ATA_SECURITY_MAX_PW_LENGTH);
            explicit_zeroes(thePassword, ATA_SECURITY_MAX_PW_LENGTH);
        }
        else if (ATA_SECURITY_PASSWORD_MODIFICATIONS.forceLowercase)
        {
            // change all to lowercase
            DECLARE_ZERO_INIT_ARRAY(char, thePassword, ATA_SECURITY_MAX_PW_LENGTH + 1);
            safe_memcpy(thePassword, ATA_SECURITY_MAX_PW_LENGTH + 1, ATA_SECURITY_PASSWORD, ATA_SECURITY_MAX_PW_LENGTH);
            convert_String_To_Lower_Case(thePassword);
            safe_memcpy(ATA_SECURITY_PASSWORD, 32, thePassword, ATA_SECURITY_MAX_PW_LENGTH);
            explicit_zeroes(thePassword, ATA_SECURITY_MAX_PW_LENGTH);
        }
        else if (ATA_SECURITY_PASSWORD_MODIFICATIONS.invertCase)
        {
            // swap case from upper to lower and lower to upper.
            DECLARE_ZERO_INIT_ARRAY(char, thePassword, ATA_SECURITY_MAX_PW_LENGTH + 1);
            safe_memcpy(thePassword, ATA_SECURITY_MAX_PW_LENGTH + 1, ATA_SECURITY_PASSWORD, ATA_SECURITY_MAX_PW_LENGTH);
            convert_String_To_Inverse_Case(thePassword);
            safe_memcpy(ATA_SECURITY_PASSWORD, 32, thePassword, ATA_SECURITY_MAX_PW_LENGTH);
            explicit_zeroes(thePassword, ATA_SECURITY_MAX_PW_LENGTH);
        }
        // check if byteswapping what was entered
        if (ATA_SECURITY_PASSWORD_MODIFICATIONS.byteSwapped)
        {
            for (uint8_t iter = UINT8_C(0); iter < (ATA_SECURITY_MAX_PW_LENGTH - 1); iter += 2)
            {
                uint8_t temp                    = ATA_SECURITY_PASSWORD[iter + 1];
                ATA_SECURITY_PASSWORD[iter + 1] = ATA_SECURITY_PASSWORD[iter];
                ATA_SECURITY_PASSWORD[iter]     = temp;
            }
        }
        // check alignment next.
        if (ATA_SECURITY_PASSWORD_MODIFICATIONS.leftAligned)
        {
            // nothing to do, already aligned this way
        }
        else if (ATA_SECURITY_PASSWORD_MODIFICATIONS.rightAligned)
        {
            // memcpy and memset based on how many characters were provided by the caller.
            safe_memmove(&ATA_SECURITY_PASSWORD[ATA_SECURITY_MAX_PW_LENGTH - ATA_SECURITY_PASSWORD_BYTE_COUNT],
                         ATA_SECURITY_MAX_PW_LENGTH - ATA_SECURITY_PASSWORD_BYTE_COUNT, &ATA_SECURITY_PASSWORD[0],
                         ATA_SECURITY_PASSWORD_BYTE_COUNT);
            safe_memset(&ATA_SECURITY_PASSWORD[0], 32, 0,
                        ATA_SECURITY_MAX_PW_LENGTH - ATA_SECURITY_PASSWORD_BYTE_COUNT);
        }
        // now check if we had padding to add. NOTE: if right aligned, padding mshould be added IN FRONT (left side)
        if (ATA_SECURITY_PASSWORD_MODIFICATIONS.zeroPadded)
        {
            // we already zero pad. nothing to do.
        }
        else if (ATA_SECURITY_PASSWORD_MODIFICATIONS.spacePadded)
        {
            // convert zero padding to spaces. Need to set different bytes based on whether left or right aligned!
            if (ATA_SECURITY_PASSWORD_MODIFICATIONS.rightAligned)
            {
                safe_memset(&ATA_SECURITY_PASSWORD[0], 32, ' ',
                            ATA_SECURITY_MAX_PW_LENGTH - ATA_SECURITY_PASSWORD_BYTE_COUNT);
            }
            else
            {
                safe_memset(&ATA_SECURITY_PASSWORD[ATA_SECURITY_PASSWORD_BYTE_COUNT],
                            32 - ATA_SECURITY_PASSWORD_BYTE_COUNT, ' ',
                            ATA_SECURITY_MAX_PW_LENGTH - ATA_SECURITY_PASSWORD_BYTE_COUNT);
            }
        }
        else if (ATA_SECURITY_PASSWORD_MODIFICATIONS.fpadded)
        {
            if (ATA_SECURITY_PASSWORD_MODIFICATIONS.rightAligned)
            {
                safe_memset(&ATA_SECURITY_PASSWORD[0], 32, UINT8_MAX,
                            ATA_SECURITY_MAX_PW_LENGTH - ATA_SECURITY_PASSWORD_BYTE_COUNT);
            }
            else
            {
                safe_memset(&ATA_SECURITY_PASSWORD[ATA_SECURITY_PASSWORD_BYTE_COUNT],
                            32 - ATA_SECURITY_PASSWORD_BYTE_COUNT, UINT8_MAX,
                            ATA_SECURITY_MAX_PW_LENGTH - ATA_SECURITY_PASSWORD_BYTE_COUNT);
            }
        }
#if defined(MD5_PASSWORD_SUPPORTED)
        if (ATA_SECURITY_PASSWORD_MODIFICATIONS.md5Hash)
        {
            // TODO: call the md5 hash routine in mbedtls
        }
#endif
    }
    else
    {
        // user did not set a password, so we need to set "SeaChest"
        ATA_SECURITY_PASSWORD_BYTE_COUNT = C_CAST(uint8_t, safe_strlen("SeaChest"));
        safe_memcpy(ATA_SECURITY_PASSWORD, 32, "SeaChest", ATA_SECURITY_PASSWORD_BYTE_COUNT);
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
    if (!(DEVICE_INFO_FLAG || TEST_UNIT_READY_FLAG || LOWLEVEL_INFO_FLAG
#if !defined(DISABLE_TCG_SUPPORT)
          || TCG_REVERT_SP_FLAG || TCG_REVERT_FLAG
#endif
          || SANITIZE_RUN_FLAG || SANITIZE_FREEZE || SANITIZE_ANTIFREEZE || SANITIZE_INFO_FLAG || RUN_WRITE_SAME_FLAG ||
          ATA_SECURITY_ERASE_OP || RUN_OVERWRITE_FLAG || RUN_TRIM_UNMAP_FLAG || (PROGRESS_CHAR != M_NULLPTR) ||
          SHOW_ERASE_SUPPORT_FLAG || PERFORM_FASTEST_ERASE_FLAG || FORMAT_UNIT_FLAG || DISPLAY_LBA_FLAG ||
          NVM_FORMAT_FLAG || ZERO_VERIFY_FLAG || ERASE_RESTORE_MAX_PREP || REFRESH_FILE_SYSTEMS))
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

    if (ATA_SECURITY_ERASE_OP || PERFORM_FASTEST_ERASE_FLAG)
    {
        flags |= HANDLE_RECOMMEND_EXCLUSIVE_ACCESS;
    }

    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        for (uint32_t devi = UINT32_C(0); devi < DEVICE_LIST_COUNT; ++devi)
        {
            DEVICE_LIST[devi].deviceVerbosity = toolVerbosity;
        }
        ret = get_Device_List(DEVICE_LIST, DEVICE_LIST_COUNT * sizeof(tDevice), version, flags);
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
                    exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
                }
                else
                {
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
                (ret == FAILURE || ret == PERMISSION_DENIED))
#else
            if ((deviceList[handleIter].os_info.fd == INVALID_HANDLE_VALUE) ||
                (ret == FAILURE || ret == PERMISSION_DENIED))
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
                else
                {
                    exit(UTIL_EXIT_OPERATION_FAILURE);
                }
            }
        }
    }
    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);

    if ((FORMAT_UNIT_FLAG && FAST_FORMAT_FLAG))
    {
        // These options all do a low-level format that has a risk of leaving the drive inoperable if it is interrupted.
        // Warn the user one last time and provide 30 seconds to cancel the operation
        printf("Fast format unit will perform a low-level format that cannot\n");
        printf("be interrupted once started. All background software should be stopped, any filesystems\n");
        printf("that are currently mounted should first be unmounted in order to reduce the risk of\n");
        printf("interruption. Do not attempt these operations on multiple devices at the same time\n");
        printf("to ensure the best possible outcome. Many controllers/drivers/HBAs cannot handle these\n");
        printf("operations running in parallel without issuing a device reset.\n");
        printf("Not all background activities can be stopped. Some are managed by the OS and are not\n");
        printf("configurable. It is recommended that a format change is done from a live/bootable\n");
        printf("environment to reduce the risk of these interuptions. If the OS is unable to complete\n");
        printf("certain commands for it's background polling of the device, it may trigger a device\n");
        printf("reset and interrupt the format, leaving the drive inoperable if it cannot be recovered.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
        printf("\t\tThere is a risk when performing a low-level format/fast format that may\n");
        printf("\t\tmake the drive inoperable if it is reset at any time while it is formatting.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_COLOR_DEFAULT);
        printf("\t\tWARNING: Any interruption to the device while it is formatting may render the\n");
        printf("\t\t         drive inoperable! Use this at your own risk!\n");
        printf("\t\tWARNING: Set sector size may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n");
        printf("\t\tWARNING (SATA): Do not interrupt this operation once it has started or \n");
        printf("\t\t         it may cause the drive to become unusable. Stop all possible background\n");
        printf("\t\t         activity that would attempt to communicate with the device while this\n");
        printf("\t\t         operation is in progress\n");
        printf("\t\tWARNING: It is not recommended to do this on USB as not\n");
        printf("\t\t         all USB adapters can handle a 4k sector size.\n");
        printf("\t\tWARNING: Disable any out-of-band management systems/services/daemons\n");
        printf("\t\t         before using this option. Interruptions can be caused by these\n");
        printf("\t\t         and may prevent completion of a sector size change.\n");
        printf("\t\tWARNING: It is recommended that this operation is done from a bootable environment\n");
        printf("\t\t         (Live USB) to reduce the risk of OS background activities running and\n");
        printf("\t\t         triggering a device reset while reformating the drive.\n\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
        printf("If you wish to cancel this operation, press CTRL-C now to exit the software.\n");
        // count down timer must go here
        for (int8_t counter = INT8_C(30); counter >= 0; --counter)
        {
            printf("\r%2d", counter);
            delay_Seconds(UINT32_C(1));
        }
        printf("\n");
    }

    for (uint32_t deviceIter = UINT32_C(0); deviceIter < DEVICE_LIST_COUNT; ++deviceIter)
    {
        bool eraseCompleted                    = false;
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

        if (SHOW_ERASE_SUPPORT_FLAG)
        {
            uint32_t    overwriteTimeMinutes = UINT32_C(0);
            eraseMethod eraseMethodList[MAX_SUPPORTED_ERASE_METHODS];
            safe_memset(&eraseMethodList, sizeof(eraseMethod) * MAX_SUPPORTED_ERASE_METHODS, 0,
                        sizeof(eraseMethod) * MAX_SUPPORTED_ERASE_METHODS);
#ifdef DISABLE_TCG_SUPPORT
            // TODO: Check the return method.
            get_Supported_Erase_Methods(&deviceList[deviceIter], eraseMethodList, &overwriteTimeMinutes);
#else //! DISABLE_TCG_SUPPORT
      // get_Supported_Erase_Methods(&selectedDevice, &eraseMethodList);//switch this to method with TCG support later -
      // TJE
            get_Supported_Erase_Methods_With_TCG(&deviceList[deviceIter], eraseMethodList, &overwriteTimeMinutes);
#endif // DISABLE_TCG_SUPPORT
            print_Supported_Erase_Methods(&deviceList[deviceIter], eraseMethodList, &overwriteTimeMinutes);
        }

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (DISPLAY_LBA_FLAG)
        {
            uint8_t* displaySector = M_REINTERPRET_CAST(
                uint8_t*, safe_calloc_aligned(deviceList[deviceIter].drive_info.deviceBlockSize, sizeof(uint8_t),
                                              deviceList[deviceIter].os_info.minimumAlignment));
            if (!displaySector)
            {
                perror("Could not allocate memory to read LBA.");
                exit(UTIL_EXIT_OPERATION_FAILURE);
            }
            if (USE_MAX_LBA)
            {
                DISPLAY_LBA_THE_LBA = deviceList[deviceIter].drive_info.deviceMaxLba;
            }
            else if (USE_CHILD_MAX_LBA)
            {
                DISPLAY_LBA_THE_LBA = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
            }
            if (SUCCESS == read_LBA(&deviceList[deviceIter], DISPLAY_LBA_THE_LBA, false, displaySector,
                                    deviceList[deviceIter].drive_info.deviceBlockSize))
            {
                printf("\nContents of LBA %" PRIu64 ":\n", DISPLAY_LBA_THE_LBA);
                print_Data_Buffer(displaySector, deviceList[deviceIter].drive_info.deviceBlockSize, false);
            }
            else
            {
                printf("Error Reading LBA %" PRIu64 " for display\n", DISPLAY_LBA_THE_LBA);
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
            safe_free_aligned(&displaySector);
        }

        // check for fastest erase flag first since it may be used to set other flags for other erase methods
        if (PERFORM_FASTEST_ERASE_FLAG)
        {
            // check that we were given the confirmation for erasing all data
            if (DATA_ERASE_FLAG)
            {
                eraseMethod eraseMethodList[MAX_SUPPORTED_ERASE_METHODS];
#ifdef DISABLE_TCG_SUPPORT
                // TODO: Check the return method.
                get_Supported_Erase_Methods(&deviceList[deviceIter], eraseMethodList, M_NULLPTR);
#else
                // get_Supported_Erase_Methods(&selectedDevice, &eraseMethodList);//switch this to method with TCG
                // support later - TJE
                get_Supported_Erase_Methods_With_TCG(&deviceList[deviceIter], eraseMethodList, M_NULLPTR);
#endif

                switch (eraseMethodList[0].eraseIdentifier)
                {
#if !defined(DISABLE_TCG_SUPPORT)
                case ERASE_TCG_REVERT:
                    TCG_REVERT_FLAG = true;
                    break;
                case ERASE_TCG_REVERT_SP:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("\nFastest Erase is RevertSP. Please use the --%s option with the drive PSID to perform "
                               "this erase.\n",
                               TCG_REVERT_SP_LONG_OPT_STRING);
                    }
                    break;
#endif
                case ERASE_SANITIZE_OVERWRITE:
                    SANITIZE_RUN_FLAG            = true;
                    SANITIZE_RUN_OVERWRITE_ERASE = true;
                    break;
                case ERASE_SANITIZE_BLOCK:
                    SANITIZE_RUN_FLAG        = true;
                    SANITIZE_RUN_BLOCK_ERASE = true;
                    break;
                case ERASE_SANITIZE_CRYPTO:
                    SANITIZE_RUN_FLAG         = true;
                    SANITIZE_RUN_CRYPTO_ERASE = true;
                    break;
                case ERASE_NVM_FORMAT_USER_SECURE_ERASE:
                    NVM_FORMAT_FLAG         = true;
                    NVM_FORMAT_SECURE_ERASE = NVM_FMT_SE_USER_DATA;
                    break;
                case ERASE_NVM_FORMAT_CRYPTO_SECURE_ERASE:
                    NVM_FORMAT_FLAG         = true;
                    NVM_FORMAT_SECURE_ERASE = NVM_FMT_SE_CRYPTO;
                    break;
                case ERASE_ATA_SECURITY_ENHANCED:
                    ATA_SECURITY_ERASE_OP            = true;
                    ATA_SECURITY_ERASE_ENHANCED_FLAG = true;
                    break;
                case ERASE_ATA_SECURITY_NORMAL:
                    ATA_SECURITY_ERASE_OP            = true;
                    ATA_SECURITY_ERASE_ENHANCED_FLAG = false;
                    break;
                case ERASE_WRITE_SAME:
                    RUN_WRITE_SAME_FLAG   = true;
                    WRITE_SAME_START_FLAG = 0;
                    WRITE_SAME_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba;
                    break;
                case ERASE_OVERWRITE:
                    RUN_OVERWRITE_FLAG   = true;
                    OVERWRITE_START_FLAG = 0;
                    OVERWRITE_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba;
                    break;
                case ERASE_FORMAT_UNIT:
                    FORMAT_UNIT_FLAG = goTrue;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("\nAn error occured while trying to determine best possible erase. No erase will be "
                               "performed.\n");
                    }
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to perform the quickest erase.\n\n");
                    printf("e.g.: %s -d %s --%s --confirm %s\n\n", util_name, deviceHandleExample,
                           PERFORM_FASTEST_ERASE_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (ERASE_RESTORE_MAX_PREP)
        {
            bool doNotContinueToErase = true;
            bool scsiAtaInSync        = false;
            switch (restore_Max_LBA_For_Erase(&deviceList[deviceIter]))
            {
            case SUCCESS:
                doNotContinueToErase = false; // successfully restored so continuing onwards to erase is fine.
                scsiAtaInSync        = is_Max_LBA_In_Sync_With_Adapter_Or_Driver(&deviceList[deviceIter], false);
                fill_Drive_Info_Data(&deviceList[deviceIter]); // refresh stale data
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    double mCapacity = 0;
                    double capacity  = 0;
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
                    printf("Successfully restored maxLBA to highest possible user addressable LBA!\n");
                    printf("New Drive Capacity (%s/%s): %0.02f/%0.02f\n", mCapUnit, capUnit, mCapacity, capacity);
                    if (!scsiAtaInSync)
                    {
                        printf("\nWARNING: The adapter/driver/bridge is not in sync with the capacity change!\n");
                        printf("         If using a drive managed erase, this will not be an issue since the\n");
                        printf("         drive firmware will handle this change properly.\n");
                        printf("         If performing a manual overwrite, such as --%s, then a power cycle\n",
                               OVERWRITE_LONG_OPT_STRING);
                        printf("         is strongly recommended to make sure all writes complete without error.\n");
                    }
                }
                break;
            case DEVICE_ACCESS_DENIED:
            case FROZEN:
            case ABORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Drive aborted the command to restore max LBA. The device may already be at maxLBA,\n");
                    printf("the restore command may have been blocked, the feature may be locked/frozen\n");
                    printf("or some other unknown reason caused the abort.\n");
                    printf("Try power cycling the drive/system and try again or try a different system\n");
                    printf("or method of attaching the drive to run this command.\n");
                    printf("When a feature is \"frozen\" the drive must be power cycled to clear this condition.\n");
                    printf("Some systems will issue the freeze commands on boot which is why changing which system\n");
                    printf("is used or how the drive is attached to the system can get around this issue.\n");
                    printf("If the device supports the HPA security extension feature, then changes to HPA may be\n");
                    printf("blocked by the password set by this feature. You must either unlock the HPA security\n");
                    printf("feature, or power cycle the drive to remove the password and lock.\n");
                    printf("If you think that the device is already at maxLBA or want to proceed to erase anyways,\n");
                    printf("remove the --%s option from the command line and try again.\n",
                           ERASE_RESTORE_MAX_PREP_LONG_OPT_STRING);
                    printf("Erase will not be started while this is failing.\n\n");
                }
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
                break;
            case POWER_CYCLE_REQUIRED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("The device requires a power cycle before the max LBA can be restored all the way.\n");
                    printf("Please power cycle the device, then run the --%s option again.\n",
                           ERASE_RESTORE_MAX_PREP_LONG_OPT_STRING);
                }
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Restoring maxLBA does not appear to be supported on this device.\n");
                    printf("If you believe this is an error, try changing how the device is\n");
                    printf("attached to the system (move from USB to SATA or from SAS HBA to\n");
                    printf("the motherboard) and try again.\n");
                    printf("If this does not work, try another system.\n");
                    printf("If you think that the device is already at maxLBA or want to proceed to erase anyways,\n");
                    printf("remove the --%s option from the command line and try again.\n",
                           ERASE_RESTORE_MAX_PREP_LONG_OPT_STRING);
                    printf("Erase will not be started while this is failing.\n\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case FAILURE:
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to restore max LBA. The device may already be at maxLBA, the restore\n");
                    printf("command may have been blocked, or some other unknown reason caused the failure.\n");
                    printf("Try power cycling the drive/system and try again or try a different system\n");
                    printf("or method of attaching the drive to run this command.\n");
                    printf("If you think that the device is already at maxLBA or want to proceed to erase anyways,\n");
                    printf("remove the --%s option from the command line and try again.\n",
                           ERASE_RESTORE_MAX_PREP_LONG_OPT_STRING);
                    printf("Erase will not be started while this is failing.\n\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
            if (doNotContinueToErase)
            {
                // continue the loop to any additional drives since they may pass
                continue;
            }
        }

#ifdef DISABLE_TCG_SUPPORT

#else
        if (TCG_REVERT_SP_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nRevertSP\n");
            }
            if (DATA_ERASE_FLAG)
            {
                writeAfterErase writeReq;
                safe_memset(&writeReq, sizeof(writeAfterErase), 0, sizeof(writeAfterErase));
                if (safe_strlen(TCG_PSID_FLAG) == 0)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("\tPSID missing. Please use --%s to provide it. PSID must be 32 characters long.\n",
                               TCG_PSID_LONG_OPT_STRING);
                    }
                    return UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                }
                else if (safe_strlen(TCG_PSID_FLAG) < 32)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("\tPSID too short. PSID must be 32 characters long.\n");
                    }
                    return UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                }
                switch (revert_SP(&deviceList[deviceIter], TCG_PSID_FLAG))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("RevertSP Successful!\n");
                        if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                        {
                            printf("NOTE: This command may have affected more than 1 logical unit\n");
                        }
                        if (writeReq.cryptoErase > WAEREQ_READ_COMPLETES_GOOD_STATUS)
                        {
                            printf("ADVISORY: This device requires a write to all LBAs after a crypto erase!\n");
                            printf("          Attempting to read any LBA will result in a failure until it\n");
                            printf("          has been written with new data!\n\n");
                        }
                    }
                    eraseCompleted = true;
                    if (writeReq.cryptoErase > WAEREQ_READ_COMPLETES_GOOD_STATUS)
                    {
                        eraseCompleted = false; // turn this off otherwise the function to update file systems outputs
                                                // errors to the screen since it tries to read the device.
                    }
                    eraseCompleted = true;
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("RevertSP is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("RevertSP Failure!\n");
                        printf("\tThis may fail for a few reasons. Please double check the PSID that was provided.\n");
                        printf(
                            "\tOn Seagate drives, PSIDs are 32 digits long, all uppercase, and uses zeros and ones\n");
                        printf("\tbut do NOT use O's and I's.\n");
                        printf("\tAdditionally, it is possible to exhaust the number of attempts the device allows.\n");
                        printf("\tSeagate drives have this set to 5 attempts. Once this is exhausted, a full power\n");
                        printf("\tcycle of the device is required before you can try again.\n");
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
                    printf("to the command line arguments to run a revertSP.\n\n");
                    printf("e.g.: %s -d %s --%s PUTTHIRTYTWOCHARACTERPSIDHERE --confirm %s\n\n", util_name,
                           deviceHandleExample, TCG_REVERT_SP_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }
        if (TCG_REVERT_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nRevert\n");
            }
            if (DATA_ERASE_FLAG)
            {
                writeAfterErase writeReq;
                eRevertAuthority authority = REVERT_AUTHORITY_MSID;
                char* passwordToUse = M_NULLPTR;
                safe_memset(&writeReq, sizeof(writeAfterErase), 0, sizeof(writeAfterErase));
                if (safe_strlen(TCG_PSID_FLAG) || safe_strlen(TCG_SID_FLAG))
                {
                    // user is providing SID or PSID to use.
                    if (safe_strlen(TCG_PSID_FLAG) > 0 && safe_strlen(TCG_PSID_FLAG) < 32)
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("\tPSID too short. PSID must be 32 characters long.\n");
                        }
                        return UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    }
                    else if (safe_strlen(TCG_PSID_FLAG) == 32)
                    {
                        authority = REVERT_AUTHORITY_PSID;
                        passwordToUse = TCG_PSID_FLAG;
                    }
                    else if (safe_strlen(TCG_SID_FLAG) > 0)
                    {
                        authority = REVERT_AUTHORITY_SID;
                        passwordToUse = TCG_SID_FLAG;
                    }
                    else
                    {
                        return UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    }
                }
                bool didEraseHappen = false;
                switch (revert(&deviceList[deviceIter], authority, passwordToUse,
                               &didEraseHappen)) // Only supporting revert on adminSP at this time.
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Revert Successful!\n");
                        if (!didEraseHappen)
                        {
                            printf("\tNOTE: Because the lockingSP was not activated, the user data may not have been "
                                   "erased.\n");
                            printf("\t      Run a cryptographic erase, such as Sanitize cryptoerase to ensure data was "
                                   "completely erased.\n\n");
                        }
                        if (writeReq.cryptoErase > WAEREQ_READ_COMPLETES_GOOD_STATUS)
                        {
                            printf("ADVISORY: This device requires a write to all LBAs after a crypto erase!\n");
                            printf("          Attempting to read any LBA will result in a failure until it\n");
                            printf("          has been written with new data!\n\n");
                        }
                        if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                        {
                            printf("NOTE: This command may have affected more than 1 logical unit\n");
                        }
                    }
                    if (didEraseHappen)
                    {
                        eraseCompleted = true;
                    }
                    if (writeReq.cryptoErase > WAEREQ_READ_COMPLETES_GOOD_STATUS)
                    {
                        eraseCompleted = false; // turn this off otherwise the function to update file systems outputs
                                                // errors to the screen since it tries to read the device.
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Revert is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Revert Failure!\n");
                        printf(
                            "\tThis may fail for a few reasons. Please double check the PSID/SID that was provided.\n");
                        printf(
                            "\tOn Seagate drives, PSIDs are 32 digits long, all uppercase, and uses zeros and ones\n");
                        printf("\tbut do NOT use O's and I's.\n");
                        printf("\tAdditionally, it is possible to exhaust the number of attempts the device allows.\n");
                        printf("\tSeagate drives have this set to 5 attempts. Once this is exhausted, a full power\n");
                        printf("\tcycle of the device is required before you can try again.\n");
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
                    printf("to the command line arguments to run a revert.\n\n");
                    printf("e.g.: %s -d %s --%s --confirm %s\n\n", util_name, deviceHandleExample,
                           TCG_REVERT_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }
#endif

        if (ZERO_VERIFY_FLAG)
        {
            ret = zero_Verify_Test(&deviceList[deviceIter], C_CAST(eZeroVerifyTestType, ZERO_VERIFY_MODE_FLAG),
                                   HIDE_LBA_COUNTER);
            switch (ret)
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Zero Validation Test passed.\n");
                }
                break;
            case VALIDATION_FAILURE:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Zero Validation failed.\n");
                }
                exitCode = SEACHEST_ERASE_EXIT_ZERO_VALIDATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Operation failure.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SANITIZE_FREEZE)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Sending Sanitize Freeze Lock Command.\n");
            }
            switch (sanitize_Freezelock(&deviceList[deviceIter]))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Sanitize Freezelock passed!\n");
                }
                break;
            case FAILURE:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Sanitize Freezelock failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Sanitize Freezelock not supported.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case IN_PROGRESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A Sanitize operation is already in progress, freezelock cannot be completed.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case DEVICE_ACCESS_DENIED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Access Denied while attempting Sanitize. Please make sure security has unlocked the drive "
                           "and try again.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unknown error in sanitize command.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (SANITIZE_ANTIFREEZE)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Sending Sanitize Anti Freeze Lock Command.\n");
            }
            switch (sanitize_Anti_Freezelock(&deviceList[deviceIter]))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Sanitize Anti Freezelock passed!\n");
                }
                break;
            case FAILURE:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Sanitize Anti Freezelock failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Sanitize Anti Freezelock not supported.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case IN_PROGRESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A Sanitize operation is already in progress, anti freezelock cannot be completed.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case DEVICE_ACCESS_DENIED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Access Denied while attempting Sanitize. Please make sure security has unlocked the drive "
                           "and try again.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unknown error in sanitize command.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SANITIZE_INFO_FLAG)
        {
            sanitizeFeaturesSupported sanitizeOptions;
            safe_memset(&sanitizeOptions, sizeof(sanitizeFeaturesSupported), 0, sizeof(sanitizeFeaturesSupported));
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nSanitize Info\n");
            }
            if (SUCCESS == get_Sanitize_Device_Features(&deviceList[deviceIter], &sanitizeOptions))
            {
                if (sanitizeOptions.sanitizeCmdEnabled)
                {
                    printf("\tThe following sanitize commands are supported:\n");
                    if (sanitizeOptions.crypto)
                    {
                        printf("\t\tCrypto Erase\t--%s cryptoerase\n", SANITIZE_LONG_OPT_STRING);
                    }
                    if (sanitizeOptions.writeAfterCryptoErase >= WAEREQ_MEDIUM_ERROR_OTHER_ASC)
                    {
                        if (sanitizeOptions.writeAfterCryptoErase == WAEREQ_PI_FORMATTED_MAY_REQUIRE_OVERWRITE)
                        {
                            printf("\t\t\tWrite after crypto erase may be required due to PI formatting! Reads may "
                                   "return an error until written!\n");
                        }
                        else
                        {
                            printf(
                                "\t\t\tWrite after crypto erase required! Reads will return an error until written!\n");
                        }
                    }
                    if (sanitizeOptions.blockErase)
                    {
                        printf("\t\tBlock Erase\t--%s blockerase\n", SANITIZE_LONG_OPT_STRING);
                    }
                    if (sanitizeOptions.writeAfterBlockErase >= WAEREQ_MEDIUM_ERROR_OTHER_ASC)
                    {
                        if (sanitizeOptions.writeAfterCryptoErase == WAEREQ_PI_FORMATTED_MAY_REQUIRE_OVERWRITE)
                        {
                            printf("\t\t\tWrite after block erase may be required due to PI formatting! Reads may "
                                   "return an error until written!\n");
                        }
                        else
                        {
                            printf(
                                "\t\t\tWrite after block erase required! Reads will return an error until written!\n");
                        }
                    }
                    if (sanitizeOptions.overwrite)
                    {
                        printf("\t\tOverwrite\t--%s overwrite\n", SANITIZE_LONG_OPT_STRING);
                        printf("\t\t\tMaximum overwrite passes: %" PRIu8 "\n", sanitizeOptions.maximumOverwritePasses);
                    }
                    if (sanitizeOptions.freezelock)
                    {
                        printf("\t\tFreezelock\t--%s\n", SANITIZE_FREEZE_LONG_OPT_STRING);
                    }
                    if (sanitizeOptions.antiFreezeLock)
                    {
                        printf("\t\tAntifreezelock\t--%s\n", SANITIZE_ANTIFREEZE_LONG_OPT_STRING);
                    }
                    if (sanitizeOptions.noDeallocateInhibited)
                    {
                        printf("\t\tNo Deallocate Inhibited by controller.\n");
                        switch (sanitizeOptions.responseMode)
                        {
                        case NO_DEALLOC_RESPONSE_INV:
                            break;
                        case NO_DEALLOC_RESPONSE_WARNING:
                            printf("\t\t\tSanitize command will be accepted but warn about deallocation when no "
                                   "deallocate is specified.\n");
                            break;
                        case NO_DEALLOC_RESPONSE_ERROR:
                            printf("\t\t\tSanitize command will be aborted with an error when no deallocate is "
                                   "specified.\n");
                            break;
                        }
                    }
                    switch (sanitizeOptions.nodmmas)
                    {
                    case NODMMAS_NOT_DEFINED:
                        break;
                    case NODMMAS_NOT_ADDITIONALLY_MODIFIED_AFTER_SANITIZE:
                        printf("\t\tMedia does not have additional modifications after sanitize completes.\n");
                        break;
                    case NODMMAS_MEDIA_MODIFIED_AFTER_SANITIZE:
                        printf("\t\tMedia is modified after sanitize completes.\n");
                        break;
                    case NODMMAS_RESERVED:
                        break;
                    }
                }
                else
                {
                    printf("\tSanitize commands are not supported by this device.\n");
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\tSanitize Features not supported.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SANITIZE_RUN_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nSanitize\n");
            }
            if (DATA_ERASE_FLAG)
            {
                bool                     sanitizeCommandRun = false;
                sanitizeOperationOptions sanitizeOp;
                safe_memset(&sanitizeOp, sizeof(sanitizeOperationOptions), 0, sizeof(sanitizeOperationOptions));
                sanitizeOp.version                                     = SANITIZE_OPERATION_OPTIONS_VERSION;
                sanitizeOp.size                                        = sizeof(sanitizeOperationOptions);
                sanitizeOp.pollForProgress                             = POLL_FLAG;
                sanitizeOp.commonOptions.allowUnrestrictedSanitizeExit = SANITIZE_AUSE;
                // NOTE: The next two options are in a union to mean the same thing
                //       It is likely that a user will specify one for NVMe devices and one for SATA/SAS
                //       So if this is specified, keep the one set to true.
                sanitizeOp.commonOptions.zoneNoReset = ZONE_NO_RESET;
                if (!sanitizeOp.commonOptions.noDeallocate)
                {
                    sanitizeOp.commonOptions.noDeallocate = NO_DEALLOCATE_AFTER_ERASE;
                }
                if (SANITIZE_RUN_BLOCK_ERASE)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Starting Sanitize Block Erase\n");
                    }
                    sanitizeOp.sanitizeEraseOperation = BLOCK_ERASE;
                }
                if (SANITIZE_RUN_CRYPTO_ERASE)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Starting Sanitize Crypto Scramble\n");
                    }
                    sanitizeOp.sanitizeEraseOperation = CRYPTO_ERASE;
                }
                if (SANITIZE_RUN_OVERWRITE_ERASE)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Starting Sanitize Overwrite Erase\n");
                    }
                    sanitizeOp.sanitizeEraseOperation                      = OVERWRITE_ERASE;
                    sanitizeOp.overwriteOptions.invertPatternBetweenPasses = SANITIZE_IPBP;
                    sanitizeOp.overwriteOptions.numberOfPasses             = SANITIZE_OVERWRITE_PASSES;
                    if (PATTERN_FLAG)
                    {
                        sanitizeOp.overwriteOptions.pattern =
                            M_BytesTo4ByteValue(patternBuffer[3], patternBuffer[2], patternBuffer[1], patternBuffer[0]);
                    }
                }
                // get the support information before starting sanitize to add warnings about write after block erase or
                // write after crypto erase.
                writeAfterErase writeReq;
                safe_memset(&writeReq, sizeof(writeAfterErase), 0, sizeof(writeAfterErase));
                is_Write_After_Erase_Required(&deviceList[deviceIter], &writeReq);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("If Sanitize appears to be taking much longer than expected, the system or HBA\n");
                    printf("may not be handling the sanitize in progress state correctly. If this happens\n");
                    printf("the next best thing to do is leave the drive powered on with the interface\n");
                    printf("cable(SATA / SAS HBA cable) disconnected. Sanitize will continue running\n");
                    printf("while the drive is powered onand the system will no longer be able to\n");
                    printf("interrupt or slow down the sanitize operation.\n");
                    printf("NOTE: On an HDD, the approximate overwrite time is 1.5-2 hours per terabyte\n");
                    printf("      of the native capacity of the drive. Reduced size for maxLBA does not\n");
                    printf("      reduce the time sanitize takes to overwrite the drive.\n");
                    printf("      On an SSD, the overwrite time will be much faster and varies depending\n");
                    printf("      on the capabilities of the SSD controller.\n\n");
                    if (SANITIZE_RUN_BLOCK_ERASE && writeReq.blockErase > WAEREQ_READ_COMPLETES_GOOD_STATUS)
                    {
                        if (writeReq.blockErase == WAEREQ_PI_FORMATTED_MAY_REQUIRE_OVERWRITE)
                        {
                            printf("ADVISORY: This device may require a write to all LBAs after a crypto erase!\n");
                            printf(
                                "          PI bytes may be invalid and reading them results in logical block guard\n");
                            printf("          check failures until a logical block has been written with new data.\n");
                            printf("          Attempting to read any LBA will result in a failure until it\n");
                            printf("          has been written with new data!\n\n");
                        }
                        else
                        {
                            printf("ADVISORY: This device requires a write to all LBAs after a block erase!\n");
                            printf("          Attempting to read any LBA will result in a failure until it\n");
                            printf("          has been written with new data!\n\n");
                        }
                    }
                    if (SANITIZE_RUN_CRYPTO_ERASE && writeReq.cryptoErase > WAEREQ_READ_COMPLETES_GOOD_STATUS)
                    {
                        if (writeReq.blockErase == WAEREQ_PI_FORMATTED_MAY_REQUIRE_OVERWRITE)
                        {
                            printf("ADVISORY: This device may require a write to all LBAs after a crypto erase!\n");
                            printf("          PI bytes may be scrambled and reading them results in logical block "
                                   "guard\n");
                            printf("          check failures until a logical block has been written with new data.\n");
                            printf("          Attempting to read any LBA will result in a failure until it\n");
                            printf("          has been written with new data!\n\n");
                        }
                        else
                        {
                            printf("ADVISORY: This device requires a write to all LBAs after a crypto erase!\n");
                            printf("          Attempting to read any LBA will result in a failure until it\n");
                            printf("          has been written with new data!\n\n");
                        }
                    }
                }
                switch (run_Sanitize_Operation2(&deviceList[deviceIter], sanitizeOp))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (POLL_FLAG)
                        {
                            printf("Sanitize command passed!\n");
                        }
                        else
                        {
                            printf("Sanitize command has been started. Use the --%s sanitize option\n",
                                   PROGRESS_LONG_OPT_STRING);
                            printf("to check for progress.\n");
                        }
                        if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                        {
                            printf("NOTE: This command may have affected more than 1 logical unit\n");
                        }
                        if (SANITIZE_RUN_BLOCK_ERASE && writeReq.blockErase > WAEREQ_READ_COMPLETES_GOOD_STATUS)
                        {
                            printf("ADVISORY: This device requires a write to all LBAs after a block erase!\n");
                            printf("          Attempting to read any LBA will result in a failure until it\n");
                            printf("          has been written with new data!\n\n");
                        }
                        if (SANITIZE_RUN_CRYPTO_ERASE && writeReq.cryptoErase > WAEREQ_READ_COMPLETES_GOOD_STATUS)
                        {
                            printf("ADVISORY: This device requires a write to all LBAs after a crypto erase!\n");
                            printf("          Attempting to read any LBA will result in a failure until it\n");
                            printf("          has been written with new data!\n\n");
                        }
                    }
                    if (sanitizeCommandRun && POLL_FLAG)
                    {
                        eraseCompleted = true;
                        if ((SANITIZE_RUN_BLOCK_ERASE && writeReq.blockErase > WAEREQ_READ_COMPLETES_GOOD_STATUS) ||
                            (SANITIZE_RUN_CRYPTO_ERASE && writeReq.cryptoErase > WAEREQ_READ_COMPLETES_GOOD_STATUS))
                        {
                            eraseCompleted = false; // turn this off otherwise the function to update file systems
                                                    // outputs errors to the screen since it tries to read the device.
                        }
                    }
                    break;
                case FAILURE:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Sanitize command failed!\n");
#if defined(_WIN32)
                        if (!is_Windows_PE())
                        {
                            printf("\tNote: Windows 8 and higher block sanitize commands.\n");
                            printf("\t      Starting in Windows 11 or Windows 10 21H2, NVMe\n");
                            printf("\t      devices can issue Sanitize Block or Crypto erases\n");
                            printf("\t      but no other device types or sanitize operations\n");
                            printf("\t      are permitted.\n");
                            printf("\t      The Windows PE/RE environments allow all sanitize operations.\n");
                        }
#endif //_WIN32
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                case IN_PROGRESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("A sanitize command is already in progress.\n");
                    }
                    break;
                case FROZEN:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Cannot run sanitize operation because drive is sanitize frozen.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Sanitize command not supported by the device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                case OS_PASSTHROUGH_FAILURE:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Sanitize command was blocked by the OS.\n");
#if defined(_WIN32)
                        if (!is_Windows_PE())
                        {
                            printf("\tNote: Windows 8 and higher block sanitize commands.\n");
                            printf("\t      Starting in Windows 11 or Windows 10 21H2, NVMe\n");
                            printf("\t      devices can issue Sanitize Block or Crypto erases\n");
                            printf("\t      but no other device types or sanitize operations\n");
                            printf("\t      are permitted.\n");
                            printf("\t      The Windows PE/RE environments allow all sanitize operations.\n");
                        }
#endif //_WIN32
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                case DEVICE_ACCESS_DENIED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Access Denied while attempting Sanitize. Please make sure security has unlocked the "
                               "drive and try again.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Unknown error in sanitize command.\n");
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
                    printf("to the command line arguments to run a sanitize operation.\n\n");
                    printf("e.g.: %s -d %s --%s blockerase --confirm %s\n\n", util_name, deviceHandleExample,
                           SANITIZE_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (FORMAT_UNIT_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Format Unit\n");
            }
            if ((FAST_FORMAT_FLAG > 0 && LOW_LEVEL_FORMAT_FLAG) || DATA_ERASE_FLAG)
            {
                bool                    currentBlockSize = true;
                runFormatUnitParameters formatUnitParameters;
                safe_memset(&formatUnitParameters, sizeof(runFormatUnitParameters), 0, sizeof(runFormatUnitParameters));
                if (FORMAT_SECTOR_SIZE)
                {
                    currentBlockSize = false;
                }
                formatUnitParameters.formatType = C_CAST(eFormatType, FAST_FORMAT_FLAG);
                if (FAST_FORMAT_FLAG > 0)
                {
                    formatUnitParameters.disableImmediate =
                        true; // for fast format, we want to hold the bus busy until it is done.
                }
                formatUnitParameters.currentBlockSize     = currentBlockSize;
                formatUnitParameters.newBlockSize         = FORMAT_SECTOR_SIZE;
                formatUnitParameters.gList                = M_NULLPTR;
                formatUnitParameters.glistSize            = 0;
                formatUnitParameters.completeList         = false;
                formatUnitParameters.disablePrimaryList   = false;
                formatUnitParameters.disableCertification = false;
                formatUnitParameters.stopOnListError      = false;
                formatUnitParameters.defaultFormat = true; // This is true unless we need to write a pattern since we
                                                           // aren't setting any of the other format option flags!!
                formatUnitParameters.protectionType = deviceList[deviceIter].drive_info.currentProtectionType;
                formatUnitParameters.protectionIntervalExponent = deviceList[deviceIter].drive_info.piExponent;
                if (PATTERN_FLAG)
                {
                    formatUnitParameters.pattern       = PATTERN_BUFFER;
                    formatUnitParameters.patternLength = deviceList[deviceIter].drive_info.deviceBlockSize;
                    formatUnitParameters.defaultFormat = false; // This is true unless we need to write a pattern!!
                }
                formatUnitParameters.securityInitialize = false;
                eReturnValues formatRet                 = UNKNOWN;
                os_Lock_Device(&deviceList[deviceIter]);
                os_Unmount_File_Systems_On_Device(&deviceList[deviceIter]);
                if (PATTERN_FLAG)
                {
                    formatRet = run_Format_Unit(&deviceList[deviceIter], formatUnitParameters, POLL_FLAG);
                }
                else
                {
                    formatRet = run_Format_Unit(&deviceList[deviceIter], formatUnitParameters, POLL_FLAG);
                }
                os_Unlock_Device(&deviceList[deviceIter]);
                switch (formatRet)
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (POLL_FLAG)
                        {
                            printf("Format Unit was Successful!\n");
                        }
                        else
                        {
                            printf("Format Unit was started Successfully!\n");
                            printf("Use --%s format to check for progress.\n", PROGRESS_LONG_OPT_STRING);
                        }
                        if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                        {
                            printf("NOTE: This command may have affected more than 1 logical unit\n");
                        }
                    }
                    if (POLL_FLAG)
                    {
                        eraseCompleted = true;
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Format Unit Not Supported!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                case DEVICE_ACCESS_DENIED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Access Denied while attempting Format Unit. Please make sure security has unlocked the "
                               "drive and try again.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Format Unit Failed!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (FAST_FORMAT_FLAG > 0)
                    {
                        printf("\n");
                        printf("You must add the flag:\n\"%s\" \n", LOW_LEVEL_FORMAT_ACCEPT_STRING);
                        printf("to the command line arguments to run a format unit.\n\n");
                        printf("e.g.: %s -d %s --%s current --%s 1 --confirm %s\n\n", util_name, deviceHandleExample,
                               FORMAT_UNIT_LONG_OPT_STRING, FAST_FORMAT_LONG_OPT_STRING,
                               LOW_LEVEL_FORMAT_ACCEPT_STRING);
                        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
                        printf("\t\tThere is an additional risk when performing a low-level fast format that may\n");
                        printf("\t\tmake the drive inoperable if it is reset at any time while it is formatting.\n");
                        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_COLOR_DEFAULT);
                        printf("\t\tWARNING: Any interruption to the device while it is formatting may render the\n");
                        printf("\t\t         drive inoperable! Use this at your own risk!\n");
                        printf("\t\tWARNING: Set sector size may affect all LUNs/namespaces for devices\n");
                        printf("\t\t         with multiple logical units or namespaces.\n");
                        printf("\t\tWARNING: Disable any out-of-band management systems/services/daemons\n");
                        printf("\t\t         before using this option. Interruptions can be caused by these\n");
                        printf("\t\t         and may prevent completion of a sector size change.\n");
                        printf(
                            "\t\tWARNING: It is recommended that this operation is done from a bootable environment\n");
                        printf("\t\t         (Live USB) to reduce the risk of OS background activities running and\n");
                        printf("\t\t         triggering a device reset while reformating the drive.\n\n");
                        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        printf("\n");
                        printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                        printf("to the command line arguments to run a format unit.\n\n");
                        printf("e.g.: %s -d %s --%s current --confirm %s\n\n", util_name, deviceHandleExample,
                               FORMAT_UNIT_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                    }
                }
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
                else if (/*NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM >= 0 && */ NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM < 16)
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
                    if (POLL_FLAG)
                    {
                        eraseCompleted = true;
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
                case DEVICE_ACCESS_DENIED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Access Denied while attempting NVM Format. Please make sure security has unlocked the "
                               "drive and try again.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
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

        if (ATA_SECURITY_ERASE_OP)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nATA Security Erase\n");
            }
            if (DATA_ERASE_FLAG)
            {
                ataSecurityPassword ataPassword;
                safe_memset(&ataPassword, sizeof(ataSecurityPassword), 0, sizeof(ataSecurityPassword));
                ataPassword.passwordType = ATA_SECURITY_USING_MASTER_PW;
                safe_memcpy(ataPassword.password, ATA_SECURITY_MAX_PW_LENGTH, ATA_SECURITY_PASSWORD,
                            ATA_SECURITY_PASSWORD_BYTE_COUNT); // ATA_SECURITY_PASSWORD_BYTE_COUNT shouldn't ever be
                                                               // > 32. Should be caught above.
                ataPassword.passwordLength = ATA_SECURITY_PASSWORD_BYTE_COUNT;
                if (ZONE_NO_RESET == goTrue)
                {
                    ataPassword.zacSecurityOption = ATA_ZAC_ERASE_FULL_ZONES;
                }
                else
                {
                    ataPassword.zacSecurityOption = ATA_ZAC_ERASE_EMPTY_ZONES;
                }
                eATASecurityEraseType ataSecureEraseType = ATA_SECURITY_ERASE_STANDARD_ERASE;
                if (ATA_SECURITY_ERASE_ENHANCED_FLAG)
                {
                    ataSecureEraseType = ATA_SECURITY_ERASE_ENHANCED_ERASE;
                }
                switch (run_ATA_Security_Erase(&deviceList[deviceIter], ataSecureEraseType, ataPassword,
                                               ATA_SECURITY_FORCE_SAT_VALID, ATA_SECURITY_FORCE_SAT))
                {
                case SUCCESS:
                    eraseCompleted = true;
                    break;
                case NOT_SUPPORTED:
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                case FROZEN:
                    exitCode = UTIL_EXIT_OPERATION_ABORTED;
                    break;
                default:
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
                    printf("to the command line arguments to run a secure erase.\n\n");
                    printf("e.g.: %s -d %s --%s normal --confirm %s\n\n", util_name, deviceHandleExample,
                           ATA_SECURITY_ERASE_OP_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (RUN_WRITE_SAME_FLAG)
        {
            uint64_t localStartLBA = WRITE_SAME_START_FLAG;
            uint64_t localRange    = WRITE_SAME_RANGE_FLAG;
            if (USE_MAX_LBA)
            {
                localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                if (WRITE_SAME_RANGE_FLAG == 0 || WRITE_SAME_RANGE_FLAG > 1)
                {
                    localRange = 1;
                }
            }
            else if (USE_CHILD_MAX_LBA)
            {
                localStartLBA = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
                if (WRITE_SAME_RANGE_FLAG == 0 || WRITE_SAME_RANGE_FLAG > 1)
                {
                    localRange = 1;
                }
            }
            if (localStartLBA != UINT64_MAX)
            {
                if (localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                }
                if (WRITE_SAME_RANGE_FLAG == 0 || WRITE_SAME_RANGE_FLAG == UINT64_MAX ||
                    (localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
                }
                if (DATA_ERASE_FLAG)
                {
                    eReturnValues writeSameRet = UNKNOWN;
                    // NOTE: Changed to automatically setting poll, since write same on SATA is EASILY interrupted by
                    // anything other than reading the SCT status log...which is a pain since
                    //       the OS may issue commands when opening the handle and there is not a way to easily handle
                    //       this on scanning the device. So polling should help make sure nothing else goes on while
                    //       write same is running. - TJE
                    os_Lock_Device(&deviceList[deviceIter]);
                    if (localStartLBA == 0)
                    {
                        // only unmount when touching boot sectors
                        os_Unmount_File_Systems_On_Device(&deviceList[deviceIter]);
                    }
                    if (PATTERN_FLAG)
                    {
                        writeSameRet = writesame(&deviceList[deviceIter], localStartLBA, localRange, true,
                                                 PATTERN_BUFFER, deviceList[deviceIter].drive_info.deviceBlockSize);
                    }
                    else
                    {
                        writeSameRet =
                            writesame(&deviceList[deviceIter], localStartLBA, localRange, true, M_NULLPTR, 0);
                    }
                    os_Unlock_Device(&deviceList[deviceIter]);
                    // now we need to send the erase
                    switch (writeSameRet)
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Successfully erased LBAs %" PRIu64 " to %" PRIu64 " using write same\n",
                                   localStartLBA, localStartLBA + localRange - 1);
                        }
                        eraseCompleted = true;
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Write same is not supported on this device, or the range is larger than the device "
                                   "supports.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to erase LBAs %" PRIu64 " to %" PRIu64 " with write same\n", localStartLBA,
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
                        printf("to the command line arguments to run a writesame operation.\n\n");
                        printf("e.g.: %s -d %s --writeSame 0 --writeSameRange 4096 --confirm %s\n\n", util_name,
                               deviceHandleExample, DATA_ERASE_ACCEPT_STRING);
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

        if (RUN_TRIM_UNMAP_FLAG)
        {
            uint64_t localStartLBA = TRIM_UNMAP_START_FLAG;
            uint64_t localRange    = TRIM_UNMAP_RANGE_FLAG;
            if (USE_MAX_LBA)
            {
                localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG > 1)
                {
                    localRange = 1;
                }
            }
            else if (USE_CHILD_MAX_LBA)
            {
                localStartLBA = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG > 1)
                {
                    localRange = 1;
                }
            }
            if (localStartLBA != UINT64_MAX)
            {
                if (localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                }
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG == UINT64_MAX ||
                    (localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba)
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
                        eraseCompleted = true;
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
                        printf("You must add the flag:\n\"%s\" \n", POSSIBLE_DATA_ERASE_ACCEPT_STRING);
                        printf("to the command line arguments to run a trim/unmap operation.\n\n");
                        printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample,
                               TRIM_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, POSSIBLE_DATA_ERASE_ACCEPT_STRING);
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

        if (RUN_OVERWRITE_FLAG)
        {
            if (DATA_ERASE_FLAG)
            {
                // check the time
                uint64_t overwriteSeconds = C_CAST(uint64_t, SECONDS_TIME_FLAG) +
                                            (C_CAST(uint64_t, MINUTES_TIME_FLAG) * UINT64_C(60)) +
                                            (C_CAST(uint64_t, HOURS_TIME_FLAG) * UINT64_C(3600));
                // determine if it's timed or a range
                if (overwriteSeconds == 0)
                {
                    eReturnValues overwriteRet  = UNKNOWN;
                    uint64_t      localStartLBA = OVERWRITE_START_FLAG;
                    uint64_t      localRange    = OVERWRITE_RANGE_FLAG;
                    if (USE_MAX_LBA)
                    {
                        localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                        if (OVERWRITE_RANGE_FLAG == 0 || OVERWRITE_RANGE_FLAG > 1)
                        {
                            localRange = 1;
                        }
                    }
                    else if (USE_CHILD_MAX_LBA)
                    {
                        localStartLBA = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
                        if (OVERWRITE_RANGE_FLAG == 0 || OVERWRITE_RANGE_FLAG > 1)
                        {
                            localRange = 1;
                        }
                    }
                    if (localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
                    {
                        localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                    }
                    // range based overwrite
                    if ((localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba ||
                        localRange == UINT64_MAX || localRange == 0)
                    {
                        localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
                    }
                    if (PATTERN_FLAG)
                    {
                        overwriteRet = erase_Range(&deviceList[deviceIter], localStartLBA, localStartLBA + localRange,
                                                   PATTERN_BUFFER, deviceList[deviceIter].drive_info.deviceBlockSize,
                                                   HIDE_LBA_COUNTER);
                    }
                    else
                    {
                        overwriteRet = erase_Range(&deviceList[deviceIter], localStartLBA, localStartLBA + localRange,
                                                   M_NULLPTR, 0, HIDE_LBA_COUNTER);
                    }
                    switch (overwriteRet)
                    {
                    case SUCCESS:
                        exitCode = 0;
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Successfully overwrote LBAs %" PRIu64 " to %" PRIu64 "\n", localStartLBA,
                                   localStartLBA + localRange - 1);
                        }
                        eraseCompleted = true;
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Erase Range is not supported on this drive type at this time.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to erase LBAs %" PRIu64 " to %" PRIu64 "\n", localStartLBA,
                                   localStartLBA + localRange - 1);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    if (overwriteSeconds > 0)
                    {
                        eReturnValues overwriteRet = UNKNOWN;
                        if (PATTERN_FLAG)
                        {
                            overwriteRet = erase_Time(&deviceList[deviceIter], OVERWRITE_START_FLAG, overwriteSeconds,
                                                      PATTERN_BUFFER, deviceList[deviceIter].drive_info.deviceBlockSize,
                                                      HIDE_LBA_COUNTER);
                        }
                        else
                        {
                            overwriteRet = erase_Time(&deviceList[deviceIter], OVERWRITE_START_FLAG, overwriteSeconds,
                                                      M_NULLPTR, 0, HIDE_LBA_COUNTER);
                        }
                        switch (overwriteRet)
                        {
                        case SUCCESS:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Successfully overwrote LBAs!\n");
                            }
                            eraseCompleted = true;
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Overwrite Time is not supported on this drive type at this time.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Failed to overwrite for the entered amount of time.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("You must specify a time to perform an overwrite for.\n");
                        }
                        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    }
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to run an overwrite operation.\n\n");
                    printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample,
                           OVERWRITE_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
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
            if (strcmp(progressTest, "SANITIZE") == 0 || strcmp(progressTest, "BLOCKERASE") == 0 ||
                strcmp(progressTest, "CRYPTOERASE") == 0 || strcmp(progressTest, "OVERWRITEERASE") == 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting Sanitize progress.\n");
                }
                result = show_Sanitize_Progress(&deviceList[deviceIter]);
            }
            else if (strcmp(progressTest, "FORMAT") == 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting Format Unit Progress.\n");
                }
                result = show_Format_Unit_Progress(&deviceList[deviceIter]);
            }
            else if (strcmp(progressTest, "NVMFORMAT") == 0)
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
        if (eraseCompleted || REFRESH_FILE_SYSTEMS)
        {
            // update the FS cache since just about all actions in here will need this if they do not already handle it
            // internally.
            eReturnValues fsret = os_Update_File_System_Cache(&deviceList[deviceIter]);
            if (REFRESH_FILE_SYSTEMS)
            {
                switch (fsret)
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Successfully refreshed the filesystems for the OS!\n");
                    }
                    eraseCompleted = true;
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Refreshing the filesystems is not supported in this OS.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to refresh the OS file systems.\n");
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
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_ERASE_SUPPORT_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s\n", util_name, deviceHandleExample, PERFORM_FASTEST_ERASE_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 1000 --%s 2000\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING,
           OVERWRITE_RANGE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0 --%s 1\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING,
           HOURS_TIME_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0 --%s repeat:04ABCDEFh\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING,
           PATTERN_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0 --%s\n", util_name, deviceHandleExample, WRITE_SAME_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 1000 --%s 2000 --%s\n", util_name, deviceHandleExample, WRITE_SAME_LONG_OPT_STRING,
           WRITE_SAME_RANGE_LONG_OPT_STRING, POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --sanitize overwrite --%s\n", util_name, deviceHandleExample,
           POLL_LONG_OPT_STRING); // random, file:, increment, repeat
    printf("\t%s -d %s --sanitize overwrite --%s --%s random\n", util_name, deviceHandleExample, POLL_LONG_OPT_STRING,
           PATTERN_LONG_OPT_STRING);
    printf("\t%s -d %s --sanitize cryptoerase --%s\n", util_name, deviceHandleExample, POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enhanced\n", util_name, deviceHandleExample, ATA_SECURITY_ERASE_OP_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enhanced --%s AutoATAWindowsString12345678901 --%s user\n", util_name, deviceHandleExample,
           ATA_SECURITY_ERASE_OP_LONG_OPT_STRING, ATA_SECURITY_PASSWORD_LONG_OPT_STRING,
           ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0\n", util_name, deviceHandleExample, TRIM_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 1000 --%s 2000\n", util_name, deviceHandleExample, TRIM_LONG_OPT_STRING,
           TRIM_RANGE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s\n", util_name, deviceHandleExample, FORMAT_UNIT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s file:path/to/myFile.bin\n", util_name, deviceHandleExample,
           FORMAT_UNIT_LONG_OPT_STRING, POLL_LONG_OPT_STRING, PATTERN_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 4096 --%s\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s user\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING, NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s 1\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING, NVM_FORMAT_PI_TYPE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, ERASE_RESTORE_MAX_PREP_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s 0\n", util_name, deviceHandleExample, ERASE_RESTORE_MAX_PREP_LONG_OPT_STRING,
           OVERWRITE_LONG_OPT_STRING);
    // return codes
    printf("\nReturn codes\n");
    printf("============\n");
    int totalErrorCodes = SEACHEST_ERASE_EXIT_MAX_ERROR - SEACHEST_ERASE_EXIT_ZERO_VALIDATION_FAILURE;
    ptrToolSpecificxitCode seachestEraseExitCodes = M_REINTERPRET_CAST(
        ptrToolSpecificxitCode, safe_calloc(int_to_sizet(totalErrorCodes), sizeof(toolSpecificxitCode)));
    // now set up all the exit codes and their meanings
    if (seachestEraseExitCodes)
    {
        for (int exitIter = UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE; exitIter < SEACHEST_ERASE_EXIT_MAX_ERROR;
             ++exitIter)
        {
            seachestEraseExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCode = exitIter;
            switch (exitIter)
            {
            case SEACHEST_ERASE_EXIT_ZERO_VALIDATION_FAILURE:
                snprintf_err_handle(seachestEraseExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Zero Validation Failure");
                break;
                // add more exit codes here!
            default: // We shouldn't ever hit the default case!
                break;
            }
        }
    }
    print_SeaChest_Util_Exit_Codes(totalErrorCodes, seachestEraseExitCodes, util_name);

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
    print_Hide_LBA_Counter_Help(shortUsage);
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
    print_Display_LBA_Help(shortUsage);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Low_Level_Info_Help(shortUsage);
    print_Poll_Help(shortUsage);
    print_Progress_Help(shortUsage, "sanitize | format | nvmformat");
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    // utility tests/operations go here - alphabetized
    // multiple interfaces
    print_Fast_Discovery_Help(shortUsage);
    print_Time_Hours_Help(shortUsage);
    print_Time_Minutes_Help(shortUsage);
#if !defined(DISABLE_TCG_SUPPORT)
    print_TCG_PSID_Help(shortUsage);
#endif
    print_Time_Seconds_Help(shortUsage);
    print_Erase_Restore_Max_Prep_Help(shortUsage);
    print_Refresh_Filesystems_Help(shortUsage);
    print_Show_Supported_Erase_Modes_Help(shortUsage);
#if !defined(DISABLE_TCG_SUPPORT)
    print_TCG_SID_Help(shortUsage);
#endif
    print_Zero_Verify_Help(shortUsage);

    // SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_ATA_Security_Force_SAT_Security_Protocol_Help(shortUsage);
    print_ATA_Security_Password_Help(shortUsage);
    print_ATA_Security_Password_Type_Help(shortUsage);
    print_ATA_Security_Password_Modifications_Help(shortUsage);
    // SAS Only Options
    // printf("\n\tSAS Only:\n\t=========\n");

    // data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    printf("Data sanitization capabilities:\n");
    printf("\tRecommendation - Restore the MaxLBA of the device prior to any erase in\n");
    printf("\t                 order to allow the drive to erase all user addressable\n");
    printf("\t                 sectors. For ATA devices this means restoring \n");
    printf("\t                 HPA + DCO / AMAC to restore the maxLBA.\n");
    printf("\t                 Restoring the MaxLBA also allows full verification of\n");
    printf("\t                 all user addressable space on the device without a\n");
    printf("\t                 limitation from a lower maxLBA.\n");
    printf("\tClear - Logical techniques are applied to all addressable storage\n");
    printf("\t        locations, protecting against simple, non-invasive data\n");
    printf("\t        recovery techniques.\n");
    printf("\tClear, Possible Purge - Cryptographic erase is a purge if the vendor\n");
    printf("\t        implementation meets the requirements in IEEE 2883-2022.\n");
    printf("\tPurge - Logical techniques that target user data, overprovisioning,\n");
    printf("\t        unused space, and bad blocks rendering data recovery infeasible\n");
    printf("\t        even with state-of-the-art laboratory techniques.\n");
    printf("This utility does not support clear/purge verification yet. All labels are\n");
    printf("written according to the expectation that the device firmware will meet\n");
    printf("these capabilities as defined in the appropriate standards from T10, T13,\n");
    printf("SATA - IO, and NVMexpress.\n");
    printf("=========================\n");
    // multiple interfaces
    print_Sanitize_AUSE_Help(shortUsage);
    print_Sanitize_Overwrite_Invert_Help(shortUsage);
    print_Overwrite_Help(shortUsage);
    print_Overwrite_Range_Help(shortUsage);
    print_Sanitize_Overwrite_Passes_Help(shortUsage);
    print_Pattern_Help(shortUsage);
    print_Perform_Quickest_Erase_Help(shortUsage);
#if !defined(DISABLE_TCG_SUPPORT)
    print_Revert_Help(shortUsage);
    print_RevertSP_Help(shortUsage);
#endif
    print_Sanitize_Help(shortUsage, util_name);
    print_Trim_Unmap_Help(shortUsage);
    print_Trim_Unmap_Range_Help(shortUsage);
    print_Writesame_Help(shortUsage);
    print_Writesame_Range_Help(shortUsage);
    print_Zone_No_Reset_Help(shortUsage);
    // SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_ATA_Security_Erase_Help(shortUsage, "SeaChest");
    print_Sanitize_Anti_Freeze_Help(shortUsage);
    print_Sanitize_Freeze_Help(shortUsage);
    // SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");
    print_Fast_Format_Help(shortUsage);
    print_Format_Unit_Help(shortUsage);
    printf("\n\tNVMe Only:\n\t=========\n");
    print_Sanitize_No_Deallocate_Help(shortUsage);
    print_NVM_Format_Metadata_Setting_Help(shortUsage);
    print_NVM_Format_Metadata_Size_Help(shortUsage);
    print_NVM_Format_NSID_Help(shortUsage);
    print_NVM_Format_PI_Type_Help(shortUsage);
    print_NVM_Format_PIL_Help(shortUsage);
    print_NVM_Format_Secure_Erase_Help(shortUsage);
    print_NVM_Format_Help(shortUsage);
}
