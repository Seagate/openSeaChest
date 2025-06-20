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
// \file openSeaChest_Format.c Binary command line that performs various format methods on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "io_utils.h"
#include "memory_safety.h"
#include "pattern_utils.h"
#include "secure_file.h"
#include "sleep.h"
#include "string_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"

#include "EULA.h"
#include "depopulate.h"
#include "drive_info.h"
#include "format.h"
#include "getopt.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "seagate_operations.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name    = "openSeaChest_Format";
const char* buildVersion = "3.3.2";

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
    // POSSIBLE_DATA_ERASE_VAR
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
    FORCE_VAR
    // scan output flags
    SCAN_FLAGS_UTIL_VARS
    FORMAT_UNIT_VARS
    FAST_FORMAT_VAR
    FORMAT_UNIT_OPTION_FLAGS
    PATTERN_VARS
    DISPLAY_LBA_VAR
    PROGRESS_VAR
    MAX_LBA_VARS
    POLL_VAR
    SHOW_FORMAT_STATUS_LOG_VAR
    SHOW_SUPPORTED_FORMATS_VAR
    SET_SECTOR_SIZE_VARS
    SHOW_PHYSICAL_ELEMENT_STATUS_VAR
    REMOVE_PHYSICAL_ELEMENT_VAR
    REPOPULATE_ELEMENTS_VAR
    DEPOP_MAX_LBA_VAR
    NVM_FORMAT_VARS
    NVM_FORMAT_OPTION_VARS
#if defined(ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
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
        FORCE_LONG_OPT,
#if defined(ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        LOWLEVEL_INFO_LONG_OPT,
        // tool specific options go here
        CONFIRM_LONG_OPT,
        POLL_LONG_OPT,
        PROGRESS_LONG_OPT,
        FORMAT_UNIT_LONG_OPT,
        FAST_FORMAT_LONG_OPT,
        DISPLAY_LBA_LONG_OPT,
        PATTERN_LONG_OPT,
        FORMAT_UNIT_ADDITIONAL_OPTIONS,
        SHOW_FORMAT_STATUS_LOG_LONG_OPT,
        SET_SECTOR_SIZE_LONG_OPT,
        SHOW_SUPPORTED_FORMATS_LONG_OPT,
        SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT,
        REMOVE_PHYSICAL_ELEMENT_LONG_OPT,
        REPOPULATE_ELEMENTS_LONG_OPT,
        DEPOP_MAX_LBA_LONG_OPT,
        NVM_FORMAT_LONG_OPT,
        NVM_FORMAT_OPTIONS_LONG_OPTS,
        FORCE_LONG_OPT,
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
                /*else if (strcmp(optarg, POSSIBLE_DATA_ERASE_ACCEPT_STRING) == 0)
                {
                    POSSIBLE_DATA_ERASE_FLAG = true;
                }*/
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
            else if (strcmp(longopts[optionIndex].name, SET_SECTOR_SIZE_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint32(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &SET_SECTOR_SIZE_SIZE))
                {
                    SET_SECTOR_SIZE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_SECTOR_SIZE_LONG_OPT_STRING, optarg);
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
            else if (strcmp(longopts[optionIndex].name, FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                         &FORMAT_UNIT_PROTECTION_TYPE))
                {
                    FORMAT_UNIT_PROECTION_TYPE_FROM_USER = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                         &FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT))
                {
                    FORMAT_UNIT_PROECTION_INTERVAL_EXPONENT_FROM_USER = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, FORMAT_UNIT_NEW_MAX_LBA_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                           &FORMAT_UNIT_NEW_MAX_LBA))
                {
                    print_Error_In_Cmd_Line_Args(FORMAT_UNIT_NEW_MAX_LBA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, DEPOP_MAX_LBA_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                           &DEPOP_MAX_LBA_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(DEPOP_MAX_LBA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING) ==
                     0) // REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING
            {
                if (!get_And_Validate_Integer_Input_Uint32(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                           &REMOVE_PHYSICAL_ELEMENT_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
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
            else if (strcmp(longopts[optionIndex].name, PATTERN_LONG_OPT_STRING) == 0)
            {
                PATTERN_FLAG = true;
                if (strcmp("random", optarg) == 0)
                {
                    fill_Random_Pattern_In_Buffer(PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
                }
                else
                {
                    char* colonLocation = strstr(optarg, ":");
                    if (colonLocation)
                    {
                        colonLocation += 1; // adding 1 to offset just beyond the colon for parsing the remaining data
                        if (strncmp("file:", optarg, 5) == 0)
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
                                        if (fileinfo->error == SEC_FILE_END_OF_FILE_REACHED)
                                        {
                                        }
                                        else
                                        {
                                            printf("Unable to read contents of the file \"%s\" for the pattern.\n",
                                                   fileinfo->filename);
                                            if (SEC_FILE_SUCCESS != secure_Close_File(fileinfo))
                                            {
                                                printf("secure file structure could not be closed! This is a fatal "
                                                       "error!\n");
                                            }
                                            free_Secure_File_Info(&fileinfo);
                                            exit(UTIL_EXIT_CANNOT_OPEN_FILE);
                                        }
                                    }
                                }
                                else
                                {
                                    if (fileinfo->error == SEC_FILE_INSECURE_PATH)
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
                                            printf("Unable to open file \"%s\" for pattern\n", colonLocation);
                                        }
                                        exitCode = UTIL_EXIT_CANNOT_OPEN_FILE;
                                    }
                                    exit(exitCode);
                                }
                                if (SEC_FILE_SUCCESS == secure_Close_File(fileinfo))
                                {
                                    free_Secure_File_Info(&fileinfo);
                                }
                                else
                                {
                                    printf("secure file structure could not be closed! This is a fatal error!\n");
                                }
                            }
                            else
                            {
                                printf("Unable to open file \"%s\" for pattern\n", colonLocation);
                                exit(UTIL_EXIT_CANNOT_OPEN_FILE);
                            }
                        }
                        else if (strncmp("increment:", optarg, 10) == 0)
                        {
                            uint8_t incrementStart = UINT8_C(0);
                            if (get_And_Validate_Integer_Input_Uint8(colonLocation, M_NULLPTR, ALLOW_UNIT_NONE,
                                                                     &incrementStart))
                            {
                                fill_Incrementing_Pattern_In_Buffer(incrementStart, PATTERN_BUFFER,
                                                                    PATTERN_BUFFER_LENGTH);
                            }
                            else
                            {
                                print_Error_In_Cmd_Line_Args(PATTERN_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                        }
                        else if (strncmp("repeat:", optarg, 7) == 0)
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
                                fill_ASCII_Pattern_In_Buffer(colonLocation,
                                                             C_CAST(uint32_t, safe_strlen(colonLocation)),
                                                             PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
                            }
                        }
                        else
                        {
                            print_Error_In_Cmd_Line_Args(PATTERN_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(PATTERN_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
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
          || FORMAT_UNIT_FLAG || DISPLAY_LBA_FLAG || (PROGRESS_CHAR != M_NULLPTR) || SHOW_FORMAT_STATUS_LOG_FLAG ||
          SET_SECTOR_SIZE_FLAG || SHOW_SUPPORTED_FORMATS_FLAG || SHOW_PHYSICAL_ELEMENT_STATUS_FLAG ||
          REMOVE_PHYSICAL_ELEMENT_FLAG > 0 || REPOPULATE_ELEMENTS_FLAG || NVM_FORMAT_FLAG))
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

    if (FORMAT_UNIT_FLAG)
    {
        flags |= HANDLE_RECOMMEND_EXCLUSIVE_ACCESS;
    }

    if (SET_SECTOR_SIZE_FLAG || FAST_FORMAT_FLAG || REMOVE_PHYSICAL_ELEMENT_FLAG > 0 || REPOPULATE_ELEMENTS_FLAG)
    {
        flags |= HANDLE_REQUIRE_EXCLUSIVE_ACCESS;
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

    if (SET_SECTOR_SIZE_FLAG || REMOVE_PHYSICAL_ELEMENT_FLAG > 0 || REPOPULATE_ELEMENTS_FLAG ||
        (FORMAT_UNIT_FLAG && FAST_FORMAT_FLAG))
    {
        // These options all do a low-level format that has a risk of leaving the drive inoperable if it is interrupted.
        // Warn the user one last time and provide 30 seconds to cancel the operation
        printf("One or more of the options provided will perform a low-level format that cannot\n");
        printf("be interrupted once started. All background software should be stopped, any filesystems\n");
        printf("that are currently mounted should first be unmounted in order to reduce the risk of\n");
        printf("interruption. Do not attempt these operations on multiple devices at the same time\n");
        printf("to ensure the best possible outcome. Many controllers/drivers/HBAs cannot handle these\n");
        printf("operations running in parallel without issuing a device reset.\n");
        printf("Not all background activities can be stopped. Some are managed by the OS and are not\n");
        printf("configurable. It is recommended that a format change is done from a live/bootable\n");
        printf("environment to reduce the risk of these interruptions. If the OS is unable to complete\n");
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
            flush_stdout();
            delay_Seconds(UINT32_C(1));
        }
        printf("\n");
    }

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

        if (SHOW_SUPPORTED_FORMATS_FLAG)
        {
            uint32_t            numberOfSectorSizes = get_Number_Of_Supported_Sector_Sizes(&deviceList[deviceIter]);
            uint32_t            memSize = sizeof(supportedFormats) + sizeof(sectorSize) * numberOfSectorSizes;
            ptrSupportedFormats formats = M_REINTERPRET_CAST(ptrSupportedFormats, safe_malloc(memSize));

            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\nWARNING: Customer unique firmware may have specific requirements that \n");
                printf("         restrict sector sizes on some products. It may not be possible to format/ \n");
                printf("         fast format to common sizes like 4K or 512B due to these customer requirements.\n\n");
            }
            if (formats)
            {
                safe_memset(formats, memSize, 0, memSize);
                formats->numberOfSectorSizes = numberOfSectorSizes;
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

        // Show format status log
        if (SHOW_FORMAT_STATUS_LOG_FLAG)
        {
            formatStatus formatStatusInfo;
            safe_memset(&formatStatusInfo, sizeof(formatStatus), 0, sizeof(formatStatus));
            switch (get_Format_Status(&deviceList[deviceIter], &formatStatusInfo))
            {
            case SUCCESS:
                show_Format_Status_Log(&formatStatusInfo);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("The format status log is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to read the format status log.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHOW_PHYSICAL_ELEMENT_STATUS_FLAG)
        {
            uint64_t depopTime    = UINT64_C(0);
            bool     depopSupport = is_Depopulation_Feature_Supported(&deviceList[deviceIter], &depopTime);
            if (depopSupport)
            {
                uint32_t numberOfDescriptors = UINT32_C(0);
                get_Number_Of_Descriptors(&deviceList[deviceIter], &numberOfDescriptors);
                if (numberOfDescriptors > 0)
                {
                    ptrPhysicalElement elementList = M_REINTERPRET_CAST(
                        ptrPhysicalElement, safe_malloc(numberOfDescriptors * sizeof(physicalElement)));
                    uint32_t depopElementID = UINT32_C(0);
                    uint16_t maxDepop = UINT16_C(0);
                    uint16_t currentDepop = UINT16_C(0);
                    if (elementList)
                    {
                        safe_memset(elementList, numberOfDescriptors * sizeof(physicalElement), 0,
                                    numberOfDescriptors * sizeof(physicalElement));
                        if (SUCCESS ==
                            get_Physical_Element_Descriptors_2(&deviceList[deviceIter], numberOfDescriptors, &depopElementID, &maxDepop, &currentDepop, elementList))
                        {
                            show_Physical_Element_Descriptors_2(numberOfDescriptors, elementList, depopTime, depopElementID, maxDepop, currentDepop);
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Failed to get physical element status.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Unable to allocate memory for physical element descriptors\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("No physical elements were found on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("The Storage Element Depopulation feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (FORMAT_UNIT_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Format Unit\n");
            }
            if (DATA_ERASE_FLAG || (FAST_FORMAT_FLAG > 0 && LOW_LEVEL_FORMAT_FLAG))
            {
                bool                    currentBlockSize = true;
                runFormatUnitParameters formatUnitParameters;
                safe_memset(&formatUnitParameters, sizeof(runFormatUnitParameters), 0, sizeof(runFormatUnitParameters));
                if (FORMAT_SECTOR_SIZE)
                {
                    currentBlockSize = false;
                }
                formatUnitParameters.formatType       = C_CAST(eFormatType, FAST_FORMAT_FLAG);
                formatUnitParameters.currentBlockSize = currentBlockSize;
                formatUnitParameters.newBlockSize     = FORMAT_SECTOR_SIZE;
                formatUnitParameters.newMaxLBA        = FORMAT_UNIT_NEW_MAX_LBA; // if zero, this is ignored
                formatUnitParameters.gList            = M_NULLPTR;
                formatUnitParameters.glistSize        = 0;
                formatUnitParameters.completeList     = FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG;
                formatUnitParameters.defaultFormat =
                    true; // Setting this to ture as the default UNLESS one of the format option flags is given!!! - TJE
                formatUnitParameters.disablePrimaryList   = FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG;
                formatUnitParameters.disableCertification = FORMAT_UNIT_DISABLE_CERTIFICATION;
                formatUnitParameters.stopOnListError      = FORMAT_UNIT_STOP_ON_LIST_ERROR;
                if (PATTERN_FLAG)
                {
                    formatUnitParameters.pattern       = PATTERN_BUFFER;
                    formatUnitParameters.patternLength = deviceList[deviceIter].drive_info.deviceBlockSize;
                }
                if (formatUnitParameters.disablePrimaryList || formatUnitParameters.disableCertification ||
                    formatUnitParameters.stopOnListError || PATTERN_FLAG)
                {
                    // format options are being specified, so we need to turn this off!!! Code refactor would probably
                    // be better in the operations lib, but this is OK for now.
                    formatUnitParameters.defaultFormat = false;
                }
                // formatUnitParameters.defaultFormat = FORMAT_UNIT_DEFAULT_FORMAT;//change to true when a user wants a
                // default format...This is basically obsolete now due to the above code, but left in place in case
                // someone wants to try some weird bit combinations
                formatUnitParameters.disableImmediate = FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE;
                if (FAST_FORMAT_FLAG > 0)
                {
                    // For a fast format, make the drive hold the bus instead or return immediately for a better overall
                    // result and reduced risk of being interrupted during the format.
                    formatUnitParameters.disableImmediate = true;
                }
                // Set the same protection information as we discovered first.
                formatUnitParameters.changeProtectionType = false;
                // override protection info if we were asked to.
                if (FORMAT_UNIT_PROECTION_TYPE_FROM_USER)
                {
                    formatUnitParameters.changeProtectionType = true;
                    formatUnitParameters.protectionType       = FORMAT_UNIT_PROTECTION_TYPE;
                    if (formatUnitParameters.protectionType < 2)
                    {
                        // clearing the exponent value since it is only valid for PI 3.
                        formatUnitParameters.protectionIntervalExponent = 0;
                    }
                }
                if (FORMAT_UNIT_PROECTION_INTERVAL_EXPONENT_FROM_USER)
                {
                    formatUnitParameters.protectionIntervalExponent = FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT;
                }
                formatUnitParameters.securityInitialize = false;
                eReturnValues formatRet = run_Format_Unit(&deviceList[deviceIter], formatUnitParameters, POLL_FLAG);
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
                        if (FAST_FORMAT_FLAG > 0)
                        {
                            printf("NOTE: After changing the sector size the drive may need to perform additional\n");
                            printf(
                                "      background operations in order to ensure full functionality and reliability.\n");
                            printf("      This background activity may take a long time and will prevent the drive "
                                   "from\n");
                            printf(
                                "      entering power saving modes like idle or standby until these operations have\n");
                            printf("      completed. These operations may take a very long time to complete.\n");
                            printf("      While EPC timers are suspended during this background operation, manual\n");
                            printf("      transitions to lower power states is supported. Manually moving to a lower "
                                   "power\n");
                            printf("      state will pause all background activity until the drive has become activate "
                                   "again\n");
                            printf("      from a command such as a read or write. If forcing a transition\n");
                            printf("      to idle_a, be aware that this power condition keeps the heads above the "
                                   "medium\n");
                            printf("      and is considered a special case that the drive firmware will allow it to "
                                   "continue\n");
                            printf("      these background operations. All EPC timers will be honored once the\n");
                            printf("      background activity is completed.\n\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Format Unit Not Supported or invalid option combination provided!\n");
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
                        printf("\t\t         and may prevent completion of a sector size change.\n\n");
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

        if (SET_SECTOR_SIZE_FLAG)
        {
            if (LOW_LEVEL_FORMAT_FLAG)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Set Sector Size to %" PRIu32 "\n", SET_SECTOR_SIZE_SIZE);
                }
                switch (set_Sector_Configuration_With_Force(&deviceList[deviceIter], SET_SECTOR_SIZE_SIZE, FORCE_FLAG))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Successfully set sector size to %" PRIu32 "\n", SET_SECTOR_SIZE_SIZE);
                        if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                        {
                            printf("NOTE: This command may have affected more than 1 logical unit\n");
                        }
                        printf("NOTE: After changing the sector size the drive may need to perform additional\n");
                        printf("      background operations in order to ensure full functionality and reliability.\n");
                        printf("      This background activity may take a long time and will prevent the drive from\n");
                        printf("      entering power saving modes like idle or standby until these operations have\n");
                        printf("      completed. These operations may take a very long time to complete.\n");
                        printf("      While EPC timers are suspended during this background operation, manual\n");
                        printf(
                            "      transitions to lower power states is supported. Manually moving to a lower power\n");
                        printf("      state will pause all background activity until the drive has become activate "
                               "again\n");
                        printf("      from a command such as a read or write. If forcing a transition\n");
                        printf(
                            "      to idle_a, be aware that this power condition keeps the heads above the medium\n");
                        printf("      and is considered a special case that the drive firmware will allow it to "
                               "continue\n");
                        printf("      these background operations. All EPC timers will be honored once the\n");
                        printf("      background activity is completed.\n\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Setting sector size not supported on this device\n");
                        if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
                        {
                            printf("For SCSI Drives, try a format unit operation\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                case DEVICE_ACCESS_DENIED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Access Denied while attempting Set Sector Size. Please make sure security has unlocked "
                               "the drive and try again.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to set sector size!\n");
                    }
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("For SCSI Drives, try a format unit operation to recover the device\n");
                        }
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
                    printf("You must add the flag:\n\"%s\" \n", LOW_LEVEL_FORMAT_ACCEPT_STRING);
                    printf("to the command line arguments to run a set sector size operation.\n\n");
                    printf("e.g.: %s -d %s --%s 4096 --%s %s\n\n", util_name, deviceHandleExample,
                           SET_SECTOR_SIZE_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, LOW_LEVEL_FORMAT_ACCEPT_STRING);
                    set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
                    printf("\t\tThere is an additional risk when performing a low-level format/fast format that may\n");
                    printf("\t\tmake the drive inoperable if it is reset at any time while it is formatting.\n");
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
                }
            }
        }

        if (REMOVE_PHYSICAL_ELEMENT_FLAG > 0)
        {
            if (LOW_LEVEL_FORMAT_FLAG)
            {
                bool depopSupport = is_Depopulation_Feature_Supported(&deviceList[deviceIter], M_NULLPTR);
                if (depopSupport)
                {
                    switch (perform_Depopulate_Physical_Element(&deviceList[deviceIter], REMOVE_PHYSICAL_ELEMENT_FLAG,
                                                                DEPOP_MAX_LBA_FLAG, POLL_FLAG))
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            if (POLL_FLAG)
                            {
                                printf("Successfully depopulated physical element %" PRIu32 "!\n",
                                       REMOVE_PHYSICAL_ELEMENT_FLAG);
                            }
                            else
                            {
                                printf("Successfully started depopulation for physical element %" PRIu32 "!\n",
                                       REMOVE_PHYSICAL_ELEMENT_FLAG);
                                printf("The device may take a long time before it is ready to accept all commands "
                                       "again.\n");
                                printf("Use \"--%s depop\" or \"--%s\" to check progress.\n", PROGRESS_LONG_OPT_STRING,
                                       SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT_STRING);
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
                            printf("This operation is not supported on this drive.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    case DEVICE_ACCESS_DENIED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Access Denied while attempting to remove physical element. Please make sure "
                                   "security has unlocked the drive and try again.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to depopulate element %" PRIu32 ".\n", REMOVE_PHYSICAL_ELEMENT_FLAG);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    printf("The Storage Element Depopulation feature is not supported on this device.\n");
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", LOW_LEVEL_FORMAT_ACCEPT_STRING);
                    printf("to the command line arguments to run a remove physical element command.\n\n");
                    printf("e.g.: %s -d %s --%s element# --confirm %s\n\n", util_name, deviceHandleExample,
                           REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING, LOW_LEVEL_FORMAT_ACCEPT_STRING);
                    set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
                    printf("\t\tThere is an additional risk when performing a remove physical element as it low-level "
                           "formats\n");
                    printf("\t\tthe drive and may make the drive inoperable if it is reset at any time while it is "
                           "formatting.\n");
                    set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
                }
            }
        }

        if (REPOPULATE_ELEMENTS_FLAG)
        {
            if (LOW_LEVEL_FORMAT_FLAG)
            {
                bool repopSupport = is_Repopulate_Feature_Supported(&deviceList[deviceIter], M_NULLPTR);
                if (repopSupport)
                {
                    switch (perform_Repopulate_Physical_Element(&deviceList[deviceIter], POLL_FLAG))
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            if (POLL_FLAG)
                            {
                                printf("Successfully repopulated all physical elements!\n");
                            }
                            else
                            {
                                printf("Successfully started repopulation.\n");
                                printf("The device may take a long time before it is ready to accept all commands "
                                       "again.\n");
                                printf("Use \"--%s repop\" or \"--%s\" to check progress.\n", PROGRESS_LONG_OPT_STRING,
                                       SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT_STRING);
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
                            printf("This operation is not supported on this drive.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    case DEVICE_ACCESS_DENIED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Access Denied while attempting to repopulate physical elements. Please make sure "
                                   "security has unlocked the drive and try again.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to repopulate physical elements!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    printf("Neither the standard or the Seagate remanufacture feature is supported on this device.\n");
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", LOW_LEVEL_FORMAT_ACCEPT_STRING);
                    printf("to the command line arguments to run a repopulate elements operation.\n\n");
                    printf("e.g.: %s -d %s --%s --confirm %s\n\n", util_name, deviceHandleExample,
                           REPOPULATE_ELEMENTS_LONG_OPT_STRING, LOW_LEVEL_FORMAT_ACCEPT_STRING);
                    set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
                    printf("\t\tThere is an additional risk when performing a repopulate elements as it low-level "
                           "formats\n");
                    printf("\t\tthe drive and may make the drive inoperable if it is reset at any time while it is "
                           "formatting.\n");
                    set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
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

        if (PROGRESS_CHAR != M_NULLPTR)
        {
            eReturnValues result = UNKNOWN;
            // first take whatever was entered in progressTest and convert it to uppercase to do fewer string
            // comparisons
            convert_String_To_Upper_Case(progressTest);
            // do some string comparisons to figure out what we are checking for progress on
            if (strcmp(progressTest, "FORMAT") == 0)
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
            else if (strcmp(progressTest, "DEPOP") == 0 || strcmp(progressTest, "REPOP") == 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting depop/repop Progress.\n");
                }
                result = show_Depop_Repop_Progress(&deviceList[deviceIter]);
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
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 2\n", util_name, deviceHandleExample, REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, REPOPULATE_ELEMENTS_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_SUPPORTED_FORMATS_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_FORMAT_STATUS_LOG_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s\n", util_name, deviceHandleExample, FORMAT_UNIT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s file:path/to/myFile.bin\n", util_name, deviceHandleExample,
           FORMAT_UNIT_LONG_OPT_STRING, POLL_LONG_OPT_STRING, PATTERN_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 4096 --%s 1 --%s\n", util_name, deviceHandleExample, FORMAT_UNIT_LONG_OPT_STRING,
           FAST_FORMAT_LONG_OPT_STRING, POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s --%s --%s\n", util_name, deviceHandleExample, FORMAT_UNIT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING, FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG_LONG_OPT_STRING,
           FORMAT_UNIT_DISABLE_CERTIFICATION_LONG_OPT_STRING, FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s 1 --%s\n", util_name, deviceHandleExample, FORMAT_UNIT_LONG_OPT_STRING,
           FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT_STRING, POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 4096 --%s\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s user\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING, NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s --%s 1\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING,
           POLL_LONG_OPT_STRING, NVM_FORMAT_PI_TYPE_LONG_OPT_STRING);
    // TODO: Format and NVM format with PI

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
    print_Force_Help(shortUsage);
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
    print_Display_LBA_Help(shortUsage);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Low_Level_Info_Help(shortUsage);
    print_Poll_Help(shortUsage);
    print_Progress_Help(shortUsage, "format | nvmformat | depop | repop");
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    print_Fast_Discovery_Help(shortUsage);
    // utility tests/operations go here - alphabetized
    // multiple interfaces
    print_Depop_MaxLBA_Help(shortUsage);
    print_Show_Physical_Element_Status_Help(shortUsage);
    print_Show_Supported_Formats_Help(shortUsage);
    // SATA Only Options
    // printf("\n\tSATA Only:\n\n");

    // SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");
    print_Show_Format_Status_Log_Help(shortUsage);

    // data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    // utility data destructive tests/operations go here
    print_Pattern_Help(shortUsage);
    print_Remove_Physical_Element_Status_Help(shortUsage);
    print_Repopulate_Elements_Help(shortUsage);
    print_Set_Sector_Size_Help(shortUsage);
    printf("\n\tSAS Only:\n\t=========\n");
    // print_Format_Default_Format_Help(shortUsage);
    print_Format_Disable_Certification_Help(shortUsage);
    print_Format_Disable_Primary_List_Help(shortUsage);
    print_Format_Discard_Grown_Defect_List_Help(shortUsage);
    print_Format_Disable_Immediate_Response_Help(shortUsage);
    print_Format_New_Max_LBA_Help(shortUsage);
    print_Format_Protection_Interval_Exponent_Help(shortUsage);
    print_Format_Protection_Type_Help(shortUsage);
    print_Fast_Format_Help(shortUsage);
    print_Format_Unit_Help(shortUsage);
    print_Format_Security_Initialize_Help(shortUsage);
    print_Format_Stop_On_List_Error_Help(shortUsage);
    printf("\n\tNVMe Only:\n\t=========\n");
    print_NVM_Format_Metadata_Setting_Help(shortUsage);
    print_NVM_Format_Metadata_Size_Help(shortUsage);
    print_NVM_Format_NSID_Help(shortUsage);
    print_NVM_Format_PI_Type_Help(shortUsage);
    print_NVM_Format_PIL_Help(shortUsage);
    print_NVM_Format_Secure_Erase_Help(shortUsage);
    print_NVM_Format_Help(shortUsage);
}
