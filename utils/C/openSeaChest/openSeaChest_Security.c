// SPDX-License-Identifier: MPL-2.0
//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2024 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
//
// \file openSeaChest_Security.c Binary command line that performs various TCG security methods on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "io_utils.h"
#include "math_utils.h"
#include "memory_safety.h"
#include "string_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"

#include "EULA.h"
#include "ata_helper.h" //for defined ATA security password size of 32bytes
#include "drive_info.h"
#include "getopt.h"
#include "openseachest_util_options.h"
#include "operations.h"
#if !defined(DISABLE_TCG_SUPPORT)
#    include "disable_data_locking.h"
#    include "port_locking.h"
#    include "revert.h"
#    include "revertSP.h"
#    include "tcg_drive_info.h"
#endif
#include "ata_Security.h"
#include "generic_tests.h"

// Uncomment this if we want the command line option to set an ATA security password.
// This is currently removed because we may not be able to unlock it behind some devices due to poor implementation on
// SAT with ATA security active #define ENABLE_ATA_SET_PASSWORD

////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name    = "openSeaChest_Security";
const char* buildVersion = "3.4.1";

typedef enum eSeaChestSecurityExitCodesEnum
{
    SEACHEST_SECURITY_EXIT_ZERO_VALIDATION_FAILURE = UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE, // Zero validation failure
    SEACHEST_SECURITY_EXIT_MAX_ERROR // don't acutally use this, just for memory allocation below
} eSeaChestSecurityExitCodes;

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
    MODEL_MATCH_VARS
    FW_MATCH_VARS
    CHILD_MODEL_MATCH_VARS
    CHILD_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    FORCE_DRIVE_TYPE_VARS
    ENABLE_LEGACY_PASSTHROUGH_VAR
    MAX_LBA_VARS
    // scan output flags
    SCAN_FLAGS_UTIL_VARS
    DISPLAY_LBA_VAR
    HIDE_LBA_COUNTER_VAR
    ZERO_VERIFY_VARS
#if !defined(DISABLE_TCG_SUPPORT)
    TCG_DEVICE_INFO_VAR
    TCG_SID_VARS
    TCG_PSID_VARS
    TCG_REVERT_SP_VARS
    TCG_REVERT_VAR
    FWDL_PORT_VARS
    IEEE1667_PORT_VARS
    DISABLE_DATA_LOCKING_VAR
    SHOW_LOCKED_REGIONS_VAR
#endif
#if defined(ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif

    // ATA Security
    ATA_SECURITY_INFO_OP_VAR
    ATA_SECURITY_PASSWORD_MODIFICATIONS_VAR
    ATA_SECURITY_PASSWORD_VARS
    ATA_SECURITY_USING_MASTER_PW_VAR
    ATA_SECURITY_ERASE_OP_VARS
#if defined ENABLE_ATA_SET_PASSWORD
    ATA_SECURITY_MASTER_PW_CAPABILITY_VAR
    ATA_SECURITY_MASTER_PW_ID_VAR
    ATA_SECURITY_SET_PASSWORD_OP_VAR
#endif // ENABLE_ATA_SET_PASSWORD
    ATA_SECURITY_FORCE_SAT_VARS
    ATA_SECURITY_UNLOCK_OP_VAR
    ATA_SECURITY_DISABLE_OP_VAR
    ATA_SECURITY_FREEZELOCK_OP_VAR
    LOWLEVEL_INFO_VAR

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
        CONFIRM_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
        LOWLEVEL_INFO_LONG_OPT,
    // tool specific options go here
#if !defined(DISABLE_TCG_SUPPORT)
        TCG_DEVICE_INFO_LONG_OPT,
        TCG_REVERT_SP_LONG_OPT,
        TCG_REVERT_LONG_OPT,
        FWDL_PORT_LONG_OPT,
        IEEE1667_PORT_LONG_OPT,
        TCG_SID_LONG_OPT,
        TCG_PSID_LONG_OPT,
        DISABLE_DATA_LOCKING_LONG_OPT,
        DISPLAY_LBA_LONG_OPT,
        SHOW_LOCKED_REGIONS_LONG_OPT,
#endif
#if defined(ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        // ATA security
        ATA_SECURITY_INFO_OP_LONG_OPT,
        ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT,
        ATA_SECURITY_PASSWORD_LONG_OPT,
        ATA_SECURITY_USING_MASTER_PW_LONG_OPT,
        ATA_SECURITY_ERASE_OP_LONG_OPT,
        ATA_SECURITY_FORCE_SAT_LONG_OPT,
#if defined ENABLE_ATA_SET_PASSWORD
        ATA_SECURITY_SET_PASSWORD_OP_LONG_OPT,
        ATA_SECURITY_MASTER_PW_CAPABILITY_LONG_OPT,
        ATA_SECURITY_MASTER_PW_ID_LONG_OPT,
#endif // ENABLE_ATA_SET_PASSWORD
        ATA_SECURITY_UNLOCK_OP_LONG_OPT,
        ATA_SECURITY_DISABLE_OP_LONG_OPT,
        ATA_SECURITY_FREEZELOCK_OP_LONG_OPT,
        HIDE_LBA_COUNTER_LONG_OPT,
        ZERO_VERIFY_LONG_OPT,
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
            if (strcmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, DATA_ERASE_ACCEPT_STRING) == 0)
                {
                    DATA_ERASE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
#if !defined(DISABLE_TCG_SUPPORT)
            else if (strcmp(longopts[optionIndex].name, FWDL_PORT_LONG_OPT_STRING) == 0)
            {
                FWDL_PORT_FLAG = true;
                if (strcmp(optarg, "lock") == 0)
                {
                    FWDL_PORT_MODE_FLAG = true;
                }
                else if (strcmp(optarg, "unlock") == 0)
                {
                    FWDL_PORT_MODE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(FWDL_PORT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, IEEE1667_PORT_LONG_OPT_STRING) == 0)
            {
                IEEE1667_PORT_FLAG = true;
                if (strcmp(optarg, "lock") == 0)
                {
                    IEEE1667_PORT_MODE_FLAG = true;
                }
                else if (strcmp(optarg, "unlock") == 0)
                {
                    IEEE1667_PORT_MODE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(IEEE1667_PORT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, TCG_SID_LONG_OPT_STRING) == 0)
            {
                snprintf(TCG_SID_FLAG, TCG_SID_BUF_LEN, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, TCG_PSID_LONG_OPT_STRING) == 0)
            {
                snprintf(TCG_PSID_FLAG, TCG_PSID_BUF_LEN, "%s", optarg);
            }
#endif //#if !defined(DISABLE_TCG_SUPPORT)
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
                                M_Min(safe_strlen(optarg),
                                      ATA_SECURITY_MAX_PW_LENGTH)); // make sure we don't try copying over a null
                                                                    // terminator because we just need to store the
                                                                    // 32bytes of characters provided.
                    ATA_SECURITY_PASSWORD_BYTE_COUNT =
                        C_CAST(uint8_t, M_Min(safe_strlen(optarg), ATA_SECURITY_MAX_PW_LENGTH));
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
#if defined ENABLE_ATA_SET_PASSWORD
            else if (strcmp(longopts[optionIndex].name, ATA_SECURITY_MASTER_PW_CAPABILITY_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "high") == 0)
                {
                    ATA_SECURITY_MASTER_PW_CAPABILITY = false;
                }
                else if (strcmp(optarg, "maximum") == 0)
                {
                    ATA_SECURITY_MASTER_PW_CAPABILITY = true;
                }
                else
                {
                    // error in command line
                    print_Error_In_Cmd_Line_Args(ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, ATA_SECURITY_MASTER_PW_ID_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint16(optarg, M_NULLPTR, &ATA_SECURITY_MASTER_PW_ID))
                {
                    print_Error_In_Cmd_Line_Args(ATA_SECURITY_MASTER_PW_ID_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
#endif // ENABLE_ATA_SET_PASSWORD
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
            else if (strcmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING) == 0)
            {
                MODEL_MATCH_FLAG = true;
                snprintf(MODEL_STRING_FLAG, MODEL_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING) == 0)
            {
                FW_MATCH_FLAG = true;
                snprintf(FW_STRING_FLAG, FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                snprintf(CHILD_MODEL_STRING_FLAG, CHILD_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING) == 0)
            {
                CHILD_FW_MATCH_FLAG = true;
                snprintf(CHILD_FW_STRING_FLAG, CHILD_FW_MATCH_STRING_LENGTH, "%s", optarg);
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
            openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
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
            for (uint8_t iter = UINT8_C(0); (iter + 1) < ATA_SECURITY_MAX_PW_LENGTH; iter += 2)
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
            // TODO: call the md5 hash routine or write one with available in public source code that is compatible with
            // MPL 2.0
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

    // From this point on, we need elevated privileges so we can talk to the drive(s)
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
          || TCG_DEVICE_INFO_FLAG
          // check for other tool specific options here
          || TCG_REVERT_FLAG || TCG_REVERT_SP_FLAG || FWDL_PORT_FLAG || IEEE1667_PORT_FLAG ||
          DISABLE_DATA_LOCKING_FLAG || DISPLAY_LBA_FLAG || SHOW_LOCKED_REGIONS
#endif
          || ATA_SECURITY_INFO_OP || ATA_SECURITY_FREEZELOCK_OP || ATA_SECURITY_DISABLE_OP || ATA_SECURITY_UNLOCK_OP
#if defined ENABLE_ATA_SET_PASSWORD
          || ATA_SECURITY_SET_PASSWORD_OP
#endif // ENABLE_ATA_SET_PASSWORD
          || ATA_SECURITY_ERASE_OP || ZERO_VERIFY_FLAG))
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

        if (ATA_SECURITY_INFO_OP)
        {
            bool              satSecSupported = false;
            ataSecurityStatus ataSecurityInfo;
            safe_memset(&ataSecurityInfo, sizeof(ataSecurityStatus), 0, sizeof(ataSecurityStatus));
            if (ATA_SECURITY_FORCE_SAT_VALID)
            {
                satSecSupported = ATA_SECURITY_FORCE_SAT;
            }
            else
            {
                satSecSupported = sat_ATA_Security_Protocol_Supported(&deviceList[deviceIter]);
            }
            get_ATA_Security_Info(&deviceList[deviceIter], &ataSecurityInfo, satSecSupported);
            print_ATA_Security_Info(&ataSecurityInfo, satSecSupported);
        }

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

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
                exitCode = SEACHEST_SECURITY_EXIT_ZERO_VALIDATION_FAILURE;
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
                print_Data_Buffer(displaySector, deviceList[deviceIter].drive_info.deviceBlockSize, true);
            }
            else
            {
                printf("Error Reading LBA %" PRIu64 " for display\n", DISPLAY_LBA_THE_LBA);
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
            safe_free_aligned(&displaySector);
        }

#if !defined(DISABLE_TCG_SUPPORT)
        if (TCG_DEVICE_INFO_FLAG)
        {
            tcgDriveInformation tcgDriveInfoData;
            if (SUCCESS == get_TCG_Device_Info(&deviceList[deviceIter], &tcgDriveInfoData))
            {
                print_TCG_Device_Info(&deviceList[deviceIter], &tcgDriveInfoData);
            }
            else
            {
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("This operation is not supported on this drive\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SHOW_LOCKED_REGIONS)
        {
            switch (show_Locked_Regions(&deviceList[deviceIter]))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("Showing locked regions is not supported on this drive.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("Failed to retrieve locked region info.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (safe_strlen(TCG_SID_FLAG) > 0 && safe_strlen(TCG_SID_FLAG) < 32)
        {
            if (toolVerbosity > VERBOSITY_QUIET)
            {
                printf("Error: SID must be 32 characters long.\n");
            }
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
        }

        if (FWDL_PORT_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nSetting FWDL Port to ");
                if (FWDL_PORT_MODE_FLAG)
                {
                    printf("locked\n");
                }
                else
                {
                    printf("unlocked\n");
                }
            }
            switch (set_Port_State(&deviceList[deviceIter], TCG_PORT_FIRMWARE_DOWNLOAD, FWDL_PORT_MODE_FLAG,
                                   TCG_PORT_AUTHENTICATION_SID, TCG_SID_FLAG, M_NULLPTR))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Set FWDL Port Successful!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Set FWDL Port is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Set FWDL Port Failure!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (IEEE1667_PORT_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nSetting IEEE1667 Port to ");
                if (IEEE1667_PORT_MODE_FLAG)
                {
                    printf("locked\n");
                }
                else
                {
                    printf("unlocked\n");
                }
            }
            // TODO: Seagate HDD and SAS SSD only
            {
                switch (set_Port_State(&deviceList[deviceIter], TCG_PORT_IEEE_1667, IEEE1667_PORT_MODE_FLAG,
                                       TCG_PORT_AUTHENTICATION_SID, TCG_SID_FLAG, M_NULLPTR))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Set IEEE1667 Port Successful!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Set IEEE1667 Port is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Set IEEE1667 Port Failure!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (DISABLE_DATA_LOCKING_FLAG)
        {
            switch (disable_Data_Locking(&deviceList[deviceIter]))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Disable Data Locking Successful!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Disable Data Locking is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Disable Data Locking Failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (TCG_REVERT_SP_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nRevertSP\n");
            }
            if (DATA_ERASE_FLAG)
            {
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
                    }
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
                eRevertAuthority authority     = REVERT_AUTHORITY_MSID;
                char*            passwordToUse = M_NULLPTR;
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
                        authority     = REVERT_AUTHORITY_PSID;
                        passwordToUse = TCG_PSID_FLAG;
                    }
                    else if (safe_strlen(TCG_SID_FLAG) > 0)
                    {
                        authority     = REVERT_AUTHORITY_SID;
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
#endif //#if !defined(DISABLE_TCG_SUPPORT)

        if (ATA_SECURITY_UNLOCK_OP)
        {
            ataSecurityPassword ataPassword;
            safe_memset(&ataPassword, sizeof(ataSecurityPassword), 0, sizeof(ataSecurityPassword));
            ataPassword.passwordType = ATA_SECURITY_USING_MASTER_PW;
            safe_memcpy(ataPassword.password, ATA_SECURITY_MAX_PW_LENGTH, ATA_SECURITY_PASSWORD,
                        ATA_SECURITY_PASSWORD_BYTE_COUNT); // ATA_SECURITY_PASSWORD_BYTE_COUNT shouldn't ever be > 32.
                                                           // Should be caught above.
            ataPassword.passwordLength = ATA_SECURITY_PASSWORD_BYTE_COUNT;
            switch (run_Unlock_ATA_Security(&deviceList[deviceIter], ataPassword, ATA_SECURITY_FORCE_SAT_VALID,
                                            ATA_SECURITY_FORCE_SAT))
            {
            case SUCCESS:
                printf("Successfully unlocked ATA security\n");
                break;
            case NOT_SUPPORTED:
                printf("ATA Security is not supported on this device.\n");
                break;
            case FROZEN:
                printf("Unable to unlock ATA security. Device is frozen.\n");
                break;
            default:
                printf("Failed to unlock ATA security.\n");
                break;
            }
            explicit_zeroes(&ataPassword, sizeof(ataSecurityPassword));
        }
        if (ATA_SECURITY_DISABLE_OP)
        {
            ataSecurityPassword ataPassword;
            safe_memset(&ataPassword, sizeof(ataSecurityPassword), 0, sizeof(ataSecurityPassword));
            ataPassword.passwordType = ATA_SECURITY_USING_MASTER_PW;
            safe_memcpy(ataPassword.password, ATA_SECURITY_MAX_PW_LENGTH, ATA_SECURITY_PASSWORD,
                        ATA_SECURITY_PASSWORD_BYTE_COUNT); // ATA_SECURITY_PASSWORD_BYTE_COUNT shouldn't ever be > 32.
                                                           // Should be caught above.
            ataPassword.passwordLength = ATA_SECURITY_PASSWORD_BYTE_COUNT;
            switch (run_Disable_ATA_Security_Password(&deviceList[deviceIter], ataPassword,
                                                      ATA_SECURITY_FORCE_SAT_VALID, ATA_SECURITY_FORCE_SAT))
            {
            case SUCCESS:
                printf("Successfully disabled ATA security password\n");
                break;
            case NOT_SUPPORTED:
                printf("ATA Security is not supported on this device.\n");
                break;
            case FROZEN:
                printf("Unable to disable ATA security. Device is frozen.\n");
                break;
            default:
                printf("Failed to disable ATA security.\n");
                break;
            }
            explicit_zeroes(&ataPassword, sizeof(ataSecurityPassword));
        }
#if defined ENABLE_ATA_SET_PASSWORD
        if (ATA_SECURITY_SET_PASSWORD_OP)
        {
            ataSecurityPassword ataPassword;
            safe_memset(&ataPassword, sizeof(ataSecurityPassword), 0, sizeof(ataSecurityPassword));
            ataPassword.passwordType       = ATA_SECURITY_USING_MASTER_PW;
            ataPassword.masterCapability   = ATA_SECURITY_MASTER_PW_CAPABILITY;
            ataPassword.masterPWIdentifier = ATA_SECURITY_MASTER_PW_ID;
            safe_memcpy(ataPassword.password, ATA_SECURITY_MAX_PW_LENGTH, ATA_SECURITY_PASSWORD,
                        ATA_SECURITY_PASSWORD_BYTE_COUNT); // ATA_SECURITY_PASSWORD_BYTE_COUNT shouldn't ever be > 32.
                                                           // Should be caught above.
            ataPassword.passwordLength = ATA_SECURITY_PASSWORD_BYTE_COUNT;
            switch (run_Set_ATA_Security_Password(&deviceList[deviceIter], ataPassword, ATA_SECURITY_FORCE_SAT_VALID,
                                                  ATA_SECURITY_FORCE_SAT))
            {
            case SUCCESS:
                printf("Successfully set ATA security password\n");
                break;
            case NOT_SUPPORTED:
                printf("ATA Security is not supported on this device.\n");
                break;
            case FROZEN:
                printf("Unable to set ATA security password. Device is frozen.\n");
                break;
            default:
                printf("Failed to set ATA security.\n");
                break;
            }
            explicit_zeroes(&ataPassword, sizeof(ataSecurityPassword));
        }
#endif // ENABLE_ATA_SET_PASSWORD

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
                ataPassword.passwordLength               = ATA_SECURITY_PASSWORD_BYTE_COUNT;
                eATASecurityEraseType ataSecureEraseType = ATA_SECURITY_ERASE_STANDARD_ERASE;
                if (ATA_SECURITY_ERASE_ENHANCED_FLAG)
                {
                    ataSecureEraseType = ATA_SECURITY_ERASE_ENHANCED_ERASE;
                }
                switch (run_ATA_Security_Erase(&deviceList[deviceIter], ataSecureEraseType, ataPassword,
                                               ATA_SECURITY_FORCE_SAT_VALID, ATA_SECURITY_FORCE_SAT))
                {
                case SUCCESS:
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
                explicit_zeroes(&ataPassword, sizeof(ataSecurityPassword));
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

        if (ATA_SECURITY_FREEZELOCK_OP)
        {
            switch (
                run_Freeze_ATA_Security(&deviceList[deviceIter], ATA_SECURITY_FORCE_SAT_VALID, ATA_SECURITY_FORCE_SAT))
            {
            case SUCCESS:
                printf("Successfully froze ATA security\n");
                break;
            case NOT_SUPPORTED:
                printf("ATA Security is not supported on this device.\n");
                break;
            default:
                printf("Failed to freeze ATA security.\n");
                break;
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

    printf("\nExamples\n");
    printf("========\n");
    // example usage
    printf("\t%s --%s\n", util_name, SCAN_LONG_OPT_STRING);
    printf("\t%s -d %s -%c\n", util_name, deviceHandleExample, DEVICE_INFO_SHORT_OPT);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SAT_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LOWLEVEL_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, ATA_SECURITY_INFO_OP_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enhanced\n", util_name, deviceHandleExample, ATA_SECURITY_ERASE_OP_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enhanced --%s AutoATAWindowsString12345678901 --%s user\n", util_name, deviceHandleExample,
           ATA_SECURITY_ERASE_OP_LONG_OPT_STRING, ATA_SECURITY_PASSWORD_LONG_OPT_STRING,
           ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s AutoATAWindowsString12345678901 --%s user\n", util_name, deviceHandleExample,
           ATA_SECURITY_DISABLE_OP_LONG_OPT_STRING, ATA_SECURITY_PASSWORD_LONG_OPT_STRING,
           ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
#if defined ENABLE_ATA_SET_PASSWORD
    printf("\t%s -d %s --%s --%s AutoATAWindowsString12345678901 --%s user\n", util_name, deviceHandleExample,
           ATA_SECURITY_SET_PASSWORD_OP_LONG_OPT_STRING, ATA_SECURITY_PASSWORD_LONG_OPT_STRING,
           ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
#endif // ENABLE_ATA_SET_PASSWORD

    // return codes
    printf("\nReturn codes\n");
    printf("============\n");
    int totalErrorCodes = SEACHEST_SECURITY_EXIT_MAX_ERROR - SEACHEST_SECURITY_EXIT_ZERO_VALIDATION_FAILURE;
    ptrToolSpecificxitCode seachestSecurityExitCodes = M_REINTERPRET_CAST(
        ptrToolSpecificxitCode, safe_calloc(int_to_sizet(totalErrorCodes), sizeof(toolSpecificxitCode)));
    // now set up all the exit codes and their meanings
    if (seachestSecurityExitCodes)
    {
        for (int exitIter = UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE; exitIter < SEACHEST_SECURITY_EXIT_MAX_ERROR;
             ++exitIter)
        {
            seachestSecurityExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCode = exitIter;
            switch (exitIter)
            {
            case SEACHEST_SECURITY_EXIT_ZERO_VALIDATION_FAILURE:
                snprintf(seachestSecurityExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Zero Validation Failure");
                break;
                // add more exit codes here!
            default: // We shouldn't ever hit the default case!
                break;
            }
        }
    }
    print_SeaChest_Util_Exit_Codes(totalErrorCodes, seachestSecurityExitCodes, util_name);

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
    printf("\nUtility arguments\n");
    printf("=================\n");
    // Common (across utilities) - alphabetized
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Display_LBA_Help(shortUsage);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Low_Level_Info_Help(shortUsage);
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    print_Fast_Discovery_Help(shortUsage);
    // utility tests/operations go here - alphabetized
#if !defined(DISABLE_TCG_SUPPORT)
    print_Disable_Data_Locking_Help(shortUsage);
    print_TCG_PSID_Help(shortUsage);
    print_Set_FWDL_Port_Help(shortUsage);
    print_Set_IEEE1667_Port_Help(shortUsage);
    print_Show_Locked_Regions_Help(shortUsage);
    print_TCG_Info_Help(shortUsage);
    print_TCG_SID_Help(shortUsage);
#endif
    print_Zero_Verify_Help(shortUsage);

    printf("\n\tSATA Only:\n\t=========\n");
    print_ATA_Security_Force_SAT_Security_Protocol_Help(shortUsage);
#if defined ENABLE_ATA_SET_PASSWORD
    print_ATA_Security_Master_Password_Capability_Help(shortUsage);
#endif // ENABLE_ATA_SET_PASSWORD
    print_ATA_Security_Freezelock_Help(shortUsage);
#if defined ENABLE_ATA_SET_PASSWORD
    print_ATA_Security_Master_Password_ID_Help(shortUsage);
#endif // ENABLE_ATA_SET_PASSWORD
    print_ATA_Security_Password_Help(shortUsage);
    print_ATA_Security_Password_Type_Help(shortUsage);
    print_ATA_Security_Password_Modifications_Help(shortUsage);
    print_ATA_Security_Info_Help(shortUsage);
    print_Disable_ATA_Security_Password_Help(shortUsage, util_name);
#if defined ENABLE_ATA_SET_PASSWORD
    print_ATA_Security_Set_Password_Help(shortUsage);
#endif // ENABLE_ATA_SET_PASSWORD
    print_ATA_Security_Unlock_Help(shortUsage);

    // data destructive commands - alphabetized
    printf("Data Destructive Commands (Seagate only)\n");
    printf("========================================\n");
    // utility data destructive tests/operations go here
#if !defined(DISABLE_TCG_SUPPORT)
    print_Revert_Help(shortUsage);
    print_RevertSP_Help(shortUsage);
#endif
    printf("\n\tSATA Only:\n\t=========\n");
    print_ATA_Security_Erase_Help(
        shortUsage, "SeaChest"); // old implementation used the utility name as the password, switching to SeaChest
}
