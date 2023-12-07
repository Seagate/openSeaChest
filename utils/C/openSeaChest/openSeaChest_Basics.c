//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2022 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
//
//
// \file OpenSeaChest_Basics.c command line that performs various basic functions on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common.h"
#include <ctype.h>
#if defined (__unix__) || defined(__APPLE__) //using this definition because linux and unix compilers both define this. Apple does not define this, which is why it has it's own definition
#include <unistd.h>
#endif
#include "getopt.h"
#include "common.h"
#include "EULA.h"
#include "openseachest_util_options.h"
#include "drive_info.h"
#include "dst.h"
#include "smart.h"
#include "power_control.h"
#include "set_max_lba.h"
#include "firmware_download.h"
#include "seagate_operations.h"
#include "operations.h"
#include "host_erase.h"
#include "trim_unmap.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_Basics";

const char *buildVersion = "3.5.4";

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
int main(int argc, char *argv[])
{
    /////////////////
    //  Variables  //
    /////////////////
    //common utility variables
    int                 ret = SUCCESS;
    eUtilExitCodes      exitCode = UTIL_EXIT_NO_ERROR;
    DEVICE_UTIL_VARS
    DEVICE_INFO_VAR
    SAT_INFO_VAR
    DATA_ERASE_VAR
    POSSIBLE_DATA_ERASE_VAR
    LICENSE_VAR
    ECHO_COMMAND_LINE_VAR
    SCAN_FLAG_VAR
    AGRESSIVE_SCAN_FLAG_VAR
    SHOW_BANNER_VAR
    NO_BANNER_VAR
    SHOW_HELP_VAR
    TEST_UNIT_READY_VAR
    FAST_DISCOVERY_VAR
    MODEL_MATCH_VARS
    FW_MATCH_VARS
    CHILD_MODEL_MATCH_VARS
    CHILD_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    POLL_VAR
    PROGRESS_VAR
    FORCE_DRIVE_TYPE_VARS
    ENABLE_LEGACY_PASSTHROUGH_VAR
    MAX_LBA_VARS
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    DISPLAY_LBA_VAR
    //utility specific
    SMART_CHECK_VAR
    SHORT_DST_VAR
    SMART_ATTRIBUTES_VARS
    ABORT_DST_VAR
    IGNORE_OPERATION_TIMEOUT_VAR
    CHECK_POWER_VAR
    SPIN_DOWN_VAR
    DOWNLOAD_FW_VARS
    SET_MAX_LBA_VARS
    RESTORE_MAX_LBA_VAR
    SET_PHY_SAS_PHY_IDENTIFIER_VAR
    SET_PHY_SPEED_VARS
    SET_READY_LED_VARS
    READ_LOOK_AHEAD_VARS
    WRITE_CACHE_VARS
    TRIM_UNMAP_VARS
    OVERWRITE_VARS
    HOURS_TIME_VAR
    MINUTES_TIME_VAR
    SECONDS_TIME_VAR
    PROVISION_VAR
    ACTIVATE_DEFERRED_FW_VAR
#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    HIDE_LBA_COUNTER_VAR
    FWDL_IGNORE_FINAL_SEGMENT_STATUS_VAR
    SHOW_CONCURRENT_RANGES_VAR
    LOWLEVEL_INFO_VAR

    int  args = 0;
    int argIndex = 0;
    int optionIndex = 0;

    struct option longopts[] = {
        //common command line options
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
        LOWLEVEL_INFO_LONG_OPT,
        //tool specific options go here
        SMART_CHECK_LONG_OPT,
        SHORT_DST_LONG_OPT,
        SMART_ATTRIBUTES_LONG_OPT,
        ABORT_DST_LONG_OPT,
        IGNORE_OPERATION_TIMEOUT_LONG_OPT,
        CHECK_POWER_LONG_OPT,
        SPIN_DOWN_LONG_OPT,
        DOWNLOAD_FW_LONG_OPT,
        DOWNLOAD_FW_MODE_LONG_OPT,
        SET_MAX_LBA_LONG_OPT,
        PROVISION_LONG_OPT,
        RESTORE_MAX_LBA_LONG_OPT,
        SET_PHY_SPEED_LONG_OPT,
        SET_PHY_SAS_PHY_LONG_OPT,
        SET_READY_LED_LONG_OPTS,
        READ_LOOK_AHEAD_LONG_OPT,
        WRITE_CACHE_LONG_OPT,
        OVERWRITE_LONG_OPTS,
        TRIM_LONG_OPTS,
        UNMAP_LONG_OPTS,
        HOURS_TIME_LONG_OPT,
        MINUTES_TIME_LONG_OPT,
        SECONDS_TIME_LONG_OPT,
        DISPLAY_LBA_LONG_OPT,
        ACTIVATE_DEFERRED_FW_LONG_OPT,
        HIDE_LBA_COUNTER_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        FWDL_IGNORE_FINAL_SEGMENT_STATUS_LONG_OPT,
        SHOW_CONCURRENT_RANGES_LONG_OPT,
        LONG_OPT_TERMINATOR
    };

    eVerbosityLevels toolVerbosity = VERBOSITY_DEFAULT;

#if defined (UEFI_C_SOURCE)
    //NOTE: This is a BSD function used to ensure the program name is set correctly for warning or error functions.
    //      This is not necessary on most modern systems other than UEFI. 
    //      This is not used in linux so that we don't depend on libbsd
    //      Update the above #define check if we port to another OS that needs this to be done.
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
    //get options we know we need
    while (1) //changed to while 1 in order to parse multiple options when longs options are involved
    {
        args = getopt_long(argc, argv, "d:hisSF:Vv:q%:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        //printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:
            //parse long options that have no short option and required arguments here
            if (strcmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING) == 0)
            {
                if (strlen(optarg) == strlen(DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, DATA_ERASE_ACCEPT_STRING, strlen(DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    DATA_ERASE_FLAG = true;
                }
                else if (strlen(optarg) == strlen(POSSIBLE_DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, POSSIBLE_DATA_ERASE_ACCEPT_STRING, strlen(POSSIBLE_DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    POSSIBLE_DATA_ERASE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, TRIM_RANGE_LONG_OPT_STRING) == 0 || strcmp(longopts[optionIndex].name, UNMAP_RANGE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input(C_CAST(const char *, optarg), &TRIM_UNMAP_RANGE_FLAG))
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
            else if (strcmp(longopts[optionIndex].name, TRIM_LONG_OPT_STRING) == 0 || strcmp(longopts[optionIndex].name, UNMAP_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input(C_CAST(const char *, optarg), &TRIM_UNMAP_START_FLAG))
                {
                    RUN_TRIM_UNMAP_FLAG = true;
                }
                else
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA = true;
                        RUN_TRIM_UNMAP_FLAG = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
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
                if (!get_And_Validate_Integer_Input(C_CAST(const char *, optarg), &OVERWRITE_RANGE_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(OVERWRITE_RANGE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, OVERWRITE_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input(C_CAST(const char *, optarg), &OVERWRITE_START_FLAG))
                {
                    RUN_OVERWRITE_FLAG = true;
                }
                else
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA = true;
                        RUN_OVERWRITE_FLAG = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
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
                HOURS_TIME_FLAG = C_CAST(uint8_t, atoi(optarg));
            }
            else if (strcmp(longopts[optionIndex].name, MINUTES_TIME_LONG_OPT_STRING) == 0)
            {
                MINUTES_TIME_FLAG = C_CAST(uint16_t, atoi(optarg));
            }
            else if (strcmp(longopts[optionIndex].name, SECONDS_TIME_LONG_OPT_STRING) == 0)
            {
                SECONDS_TIME_FLAG = C_CAST(uint32_t, atoi(optarg));
            }
            else if (strncmp(longopts[optionIndex].name, DOWNLOAD_FW_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DOWNLOAD_FW_LONG_OPT_STRING))) == 0)
            {
                int scanRet = sscanf(optarg, FIRMWARE_FILE_NAME_MAX_LEN_FORMAT_STR, DOWNLOAD_FW_FILENAME_FLAG);
                if (scanRet > 0 && scanRet != EOF)
                {
                    DOWNLOAD_FW_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(DOWNLOAD_FW_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, DOWNLOAD_FW_MODE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DOWNLOAD_FW_MODE_LONG_OPT_STRING))) == 0)
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
                //TODO: deferredselect and a way to get events: POA, HRA, and VSA
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
            else if (strncmp(longopts[optionIndex].name, SET_MAX_LBA_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_MAX_LBA_LONG_OPT_STRING))) == 0)
            {
                int scanRet = sscanf(optarg, "%" SCNu64, &SET_MAX_LBA_VALUE);
                if (scanRet > 0 && scanRet != EOF)
                {
                    SET_MAX_LBA_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_MAX_LBA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SET_PHY_SPEED_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_PHY_SPEED_LONG_OPT_STRING))) == 0)
            {
                SET_PHY_SPEED_FLAG = true;
                SET_PHY_SPEED_GEN = C_CAST(uint8_t, atoi(optarg));
            }
            else if (strncmp(longopts[optionIndex].name, SET_PHY_SAS_PHY_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_PHY_SAS_PHY_LONG_OPT_STRING))) == 0)
            {
                SET_PHY_ALL_PHYS = false;
                SET_PHY_SAS_PHY_IDENTIFIER = C_CAST(uint8_t, atoi(optarg));
            }
            else if ((strncmp(longopts[optionIndex].name, SET_READY_LED_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_READY_LED_LONG_OPT_STRING))) == 0) ||
                (strncmp(longopts[optionIndex].name, SET_PIN_11_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_PIN_11_LONG_OPT_STRING))) == 0)
                )
            {
                if (strcmp(optarg, "default") == 0)
                {
                    SET_READY_LED_FLAG = true;
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
            else if (strncmp(longopts[optionIndex].name, READ_LOOK_AHEAD_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(READ_LOOK_AHEAD_LONG_OPT_STRING))) == 0)
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
            else if (strncmp(longopts[optionIndex].name, WRITE_CACHE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(WRITE_CACHE_LONG_OPT_STRING))) == 0)
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
            else if (strncmp(longopts[optionIndex].name, PROVISION_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(PROVISION_LONG_OPT_STRING))) == 0)
            {
                if (get_And_Validate_Integer_Input(C_CAST(const char *, optarg), &SET_MAX_LBA_VALUE))
                {
                    SET_MAX_LBA_FLAG = true;
                    //now, based on the new MaxLBA, set the TRIM/UNMAP start flag to get rid of the LBAs that will not be above the new maxLBA (the range will be set later)
                    TRIM_UNMAP_START_FLAG = SET_MAX_LBA_VALUE + 1;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PROVISION_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SMART_ATTRIBUTES_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SMART_ATTRIBUTES_LONG_OPT_STRING))) == 0)
            {
                SMART_ATTRIBUTES_FLAG = true;
                if (strcmp(optarg, "raw") == 0)
                {
                    SMART_ATTRIBUTES_MODE_FLAG = SMART_ATTR_OUTPUT_RAW;
                }
                else if (strcmp(optarg, "analyzed") == 0)
                {
                    SMART_ATTRIBUTES_MODE_FLAG = SMART_ATTR_OUTPUT_ANALYZED;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SMART_ATTRIBUTES_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(MODEL_MATCH_LONG_OPT_STRING))) == 0)
            {
                MODEL_MATCH_FLAG = true;
                snprintf(MODEL_STRING_FLAG, MODEL_STRING_LENGTH, "%s", optarg);
            }
            else if (strncmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                FW_MATCH_FLAG = true;
                snprintf(FW_STRING_FLAG, FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_MODEL_MATCH_LONG_OPT_STRING))) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                snprintf(CHILD_MODEL_STRING_FLAG, CHILD_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                CHILD_FW_MATCH_FLAG = true;
                snprintf(CHILD_FW_STRING_FLAG, CHILD_FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, DISPLAY_LBA_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input(C_CAST(const char *, optarg), &DISPLAY_LBA_THE_LBA))
                {
                    DISPLAY_LBA_FLAG = true;
                }
                else
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA = true;
                        DISPLAY_LBA_FLAG = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
                        DISPLAY_LBA_FLAG = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(DISPLAY_LBA_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            break;
        case ':'://missing required argument
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            switch (optopt)
            {
            case 0:
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
        case DEVICE_SHORT_OPT: //device
            if (0 != parse_Device_Handle_Argument(optarg, &RUN_ON_ALL_DRIVES, &USER_PROVIDED_HANDLE, &DEVICE_LIST_COUNT, &HANDLE_LIST))
            {
                //Free any memory allocated so far, then exit.
                free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\n");
                }
                exit(255);
            }
            break;
        case DEVICE_INFO_SHORT_OPT: //device information
            DEVICE_INFO_FLAG = true;
            break;
        case SCAN_SHORT_OPT: //scan
            SCAN_FLAG = true;
            break;
        case AGRESSIVE_SCAN_SHORT_OPT:
            AGRESSIVE_SCAN_FLAG = true;
            break;
        case VERSION_SHORT_OPT:
            SHOW_BANNER_FLAG = true;
            break;
        case VERBOSE_SHORT_OPT: //verbose
            if (optarg != NULL)
            {
                toolVerbosity = atoi(optarg);
            }
            break;
        case QUIET_SHORT_OPT: //quiet mode
            toolVerbosity = VERBOSITY_QUIET;
            break;
        case SCAN_FLAGS_SHORT_OPT://scan flags
            get_Scan_Flags(&SCAN_FLAGS, optarg);
            break;
        case '?': //unknown option
            printf("%s: Unable to parse %s command line option\nPlease use --%s for more information.\n", util_name, argv[optind - 1], HELP_LONG_OPT_STRING);
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\n");
            }
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
        case 'h': //help
            SHOW_HELP_FLAG = true;
            openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
            utility_Usage(false);
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\n");
            }
            exit(UTIL_EXIT_NO_ERROR);
        case PROGRESS_SHORT_OPT://progress [test]
            PROGRESS_CHAR = strdup(optarg);
            break;
        default:
            break;
        }
    }

    atexit(print_Final_newline);

    if (ECHO_COMMAND_LINE_FLAG)
    {
        int commandLineIter = 1;//start at 1 as starting at 0 means printing the directory info+ SeaChest.exe (or ./SeaChest)
        for (commandLineIter = 1; commandLineIter < argc; commandLineIter++)
        {
            if (strncmp(argv[commandLineIter], "--echoCommandLine", strlen(argv[commandLineIter])) == 0)
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
        utility_Full_Version_Info(util_name, buildVersion, OPENSEA_TRANSPORT_MAJOR_VERSION, OPENSEA_TRANSPORT_MINOR_VERSION, OPENSEA_TRANSPORT_PATCH_VERSION, OPENSEA_COMMON_VERSION, OPENSEA_OPERATION_VERSION);
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
        if(AGRESSIVE_SCAN_FLAG)
        {
            scanControl |= AGRESSIVE_SCAN;
        }
        #if defined (__linux__)
        if (SCAN_FLAGS.scanSD)
        {
            scanControl |= SD_HANDLES;
        }
        if (SCAN_FLAGS.scanSDandSG)
        {
            scanControl |= SG_TO_SD;
        }
        #endif
        //set the drive types to show (if none are set, the lower level code assumes we need to show everything)
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
        //set the interface types to show (if none are set, the lower level code assumes we need to show everything)
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
#if defined (ENABLE_CSMI)
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
        scan_And_Print_Devs(scanControl, NULL, toolVerbosity);
    }
    // Add to this if list anything that is suppose to be independent.
    // e.g. you can't say enumerate & then pull logs in the same command line.
    // SIMPLE IS BEAUTIFUL
    if (SCAN_FLAG || AGRESSIVE_SCAN_FLAG || SHOW_BANNER_FLAG || LICENSE_FLAG || SHOW_HELP_FLAG)
    {
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_NO_ERROR);
    }

    //print out errors for unknown arguments for remaining args that haven't been processed yet
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
        uint64_t flags = 0;
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
            printf("You must specify one or more target devices with the --%s option to run this command.\n", DEVICE_LONG_OPT_STRING);
            utility_Usage(true);
            printf("Use -h option for detailed description\n\n");
        }
        exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
    }

    if ((FORCE_SCSI_FLAG && FORCE_ATA_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_SCSI_FLAG && (FORCE_ATA_PIO_FLAG || FORCE_ATA_DMA_FLAG || FORCE_ATA_UDMA_FLAG))//We may need to remove this. At least when software SAT gets used. (currently only Windows ATA passthrough and FreeBSD ATA passthrough)
        )
    {
        printf("\nError: Only one force flag can be used at a time.\n");
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    //need to make sure this is set when we are asked for SAT Info
    if (SAT_INFO_FLAG)
    {
        DEVICE_INFO_FLAG = goTrue;
    }
    //check that we were given at least one test to perform...if not, show the help and exit
    if (!(DEVICE_INFO_FLAG
        || TEST_UNIT_READY_FLAG
        || SMART_CHECK_FLAG
        || SHORT_DST_FLAG
        || SMART_ATTRIBUTES_FLAG
        || ABORT_DST_FLAG
        || CHECK_POWER_FLAG
        || SPIN_DOWN_FLAG
        || DOWNLOAD_FW_FLAG
        || ACTIVATE_DEFERRED_FW_FLAG
        || RESTORE_MAX_LBA_FLAG
        || SET_MAX_LBA_FLAG
        || SET_PHY_SPEED_FLAG
        || SET_READY_LED_FLAG
        || READY_LED_INFO_FLAG
        || WRITE_CACHE_FLAG
        || READ_LOOK_AHEAD_FLAG
        || READ_LOOK_AHEAD_INFO
        || WRITE_CACHE_INFO
        || PROVISION_FLAG
        || RUN_OVERWRITE_FLAG
        || RUN_TRIM_UNMAP_FLAG
        || (PROGRESS_CHAR != NULL)
        || DISPLAY_LBA_FLAG
        || SHOW_CONCURRENT_RANGES
        || LOWLEVEL_INFO_FLAG
        //check for other tool specific options here
        ))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    uint64_t flags = 0;
    DEVICE_LIST = C_CAST(tDevice*, calloc(DEVICE_LIST_COUNT, sizeof(tDevice)));
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
    memset(&version, 0, sizeof(versionBlock));
    version.version = DEVICE_BLOCK_VERSION;
    version.size = sizeof(tDevice);

    if (TEST_UNIT_READY_FLAG || CHECK_POWER_FLAG)
    {
        flags = DO_NOT_WAKE_DRIVE;
    }

    if (FAST_DISCOVERY_FLAG)
    {
        flags = FAST_SCAN;
    }

    //set flags that can be passed down in get device regarding forcing specific ATA modes.
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
        //TODO? check for this flag ENABLE_LEGACY_PASSTHROUGH_FLAG. Not sure it is needed here and may not be desirable.
        for (uint32_t devi = 0; devi < DEVICE_LIST_COUNT; ++devi)
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
        for (uint32_t handleIter = 0; handleIter < DEVICE_LIST_COUNT; ++handleIter)
        {
            /*Initializing is necessary*/
            deviceList[handleIter].sanity.size = sizeof(tDevice);
            deviceList[handleIter].sanity.version = DEVICE_BLOCK_VERSION;
#if defined (UEFI_C_SOURCE)
            deviceList[handleIter].os_info.fd = NULL;
#elif  !defined(_WIN32)
            deviceList[handleIter].os_info.fd = -1;
#if defined(VMK_CROSS_COMP)
            deviceList[handleIter].os_info.nvmeFd = NULL;
#endif
#else
            deviceList[handleIter].os_info.fd = INVALID_HANDLE_VALUE;
#endif
            deviceList[handleIter].dFlags = flags;

            deviceList[handleIter].deviceVerbosity = toolVerbosity;

            if (ENABLE_LEGACY_PASSTHROUGH_FLAG)
            {
                deviceList[handleIter].drive_info.ata_Options.enableLegacyPassthroughDetectionThroughTrialAndError = true;
            }

            /*get the device for the specified handle*/
#if defined(_DEBUG)
            printf("Attempting to open handle \"%s\"\n", HANDLE_LIST[handleIter]);
#endif
            ret = get_Device(HANDLE_LIST[handleIter], &deviceList[handleIter]);
#if !defined(_WIN32)
#if !defined(VMK_CROSS_COMP)
            if ((deviceList[handleIter].os_info.fd < 0) ||
#else
            if (((deviceList[handleIter].os_info.fd < 0) &&
                 (deviceList[handleIter].os_info.nvmeFd == NULL)) ||
#endif
            (ret == FAILURE || ret == PERMISSION_DENIED))
#else
            if ((deviceList[handleIter].os_info.fd == INVALID_HANDLE_VALUE) || (ret == FAILURE || ret == PERMISSION_DENIED))
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
    for (uint32_t deviceIter = 0; deviceIter < DEVICE_LIST_COUNT; ++deviceIter)
    {
        deviceList[deviceIter].deviceVerbosity = toolVerbosity;
        if (ONLY_SEAGATE_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                /*if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) is not a Seagate drive.\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification);
                }*/
                continue;
            }
        }

        //check for model number match
        if (MODEL_MATCH_FLAG)
        {
            if (strstr(deviceList[deviceIter].drive_info.product_identification, MODEL_STRING_FLAG) == NULL)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) does not match the input model number: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification, MODEL_STRING_FLAG);
                }
                continue;
            }
        }
        //check for fw match
        if (FW_MATCH_FLAG)
        {
            if (strcmp(FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision))
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input firmware revision: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_revision, FW_STRING_FLAG);
                }
                continue;
            }
        }

        //check for child model number match
        if (CHILD_MODEL_MATCH_FLAG)
        {
            if (strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN) == 0 || strstr(deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG) == NULL)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) does not match the input child model number: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG);
                }
                continue;
            }
        }
        //check for child fw match
        if (CHILD_FW_MATCH_FLAG)
        {
            if (strcmp(CHILD_FW_STRING_FLAG, deviceList[deviceIter].drive_info.bridge_info.childDriveFW))
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input child firmware revision: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.bridge_info.childDriveFW, CHILD_FW_STRING_FLAG);
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

        if (FORCE_ATA_PIO_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\tAttempting to force ATA Drive commands in PIO Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaSupported = false;
            deviceList[deviceIter].drive_info.ata_Options.dmaMode = ATA_DMA_MODE_NO_DMA;
            deviceList[deviceIter].drive_info.ata_Options.downloadMicrocodeDMASupported = false;
            deviceList[deviceIter].drive_info.ata_Options.readBufferDMASupported = false;
            deviceList[deviceIter].drive_info.ata_Options.readLogWriteLogDMASupported = false;
            deviceList[deviceIter].drive_info.ata_Options.writeBufferDMASupported = false;
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
            printf("\n%s - %s - %s - %s - %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification, deviceList[deviceIter].drive_info.serialNumber, deviceList[deviceIter].drive_info.product_revision, print_drive_type(&deviceList[deviceIter]));
        }

        //now start looking at what operations are going to be performed and kick them off
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

        if (SHOW_CONCURRENT_RANGES)
        {
            concurrentRanges ranges;
            memset(&ranges, 0, sizeof(concurrentRanges));
            ranges.size = sizeof(concurrentRanges);
            ranges.version = CONCURRENT_RANGES_VERSION;
            switch (get_Concurrent_Positioning_Ranges(&deviceList[deviceIter], &ranges))
            {
            case SUCCESS:
                print_Concurrent_Positioning_Ranges(&ranges);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Concurrent positioning ranges are not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to read the concurrent positioning ranges.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (DISPLAY_LBA_FLAG)
        {
            uint8_t *displaySector = C_CAST(uint8_t*, calloc_aligned(deviceList[deviceIter].drive_info.deviceBlockSize, sizeof(uint8_t), deviceList[deviceIter].os_info.minimumAlignment));
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
            if (SUCCESS == read_LBA(&deviceList[deviceIter], DISPLAY_LBA_THE_LBA, false, displaySector, deviceList[deviceIter].drive_info.deviceBlockSize))
            {
                printf("\nContents of LBA %"PRIu64":\n", DISPLAY_LBA_THE_LBA);
                print_Data_Buffer(displaySector, deviceList[deviceIter].drive_info.deviceBlockSize, true);
            }
            else
            {
                printf("Error Reading LBA %"PRIu64" for display\n", DISPLAY_LBA_THE_LBA);
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
            safe_Free_aligned(displaySector)
        }

        if (SPIN_DOWN_FLAG)
        {
            switch (spin_down_drive(&deviceList[deviceIter], false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully sent command to spin down device. Please wait 15 seconds for it to finish spinning down.\n");
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
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to spin down device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        //this option must come after --transition power so that these two options can be combined on the command line and produce the correct end result
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

        if (SMART_CHECK_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("SMART Check\n");
            }
            ret = run_SMART_Check(&deviceList[deviceIter], NULL);
            if (FAILURE == ret)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SMART has been tripped!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
            else if (SUCCESS == ret)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SMART Check Passed!\n");
                }
            }
            else if (IN_PROGRESS == ret)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SMART Warning condition detected!\n");
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unable to run SMART Check!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SMART_ATTRIBUTES_FLAG)
        {
            switch (print_SMART_Attributes(&deviceList[deviceIter], SMART_ATTRIBUTES_MODE_FLAG))
            {
            case SUCCESS:
                //nothing to print here since if it was successful, the attributes will be printed to the screen
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Showing SMART attributes is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A failure occurred while trying to get SMART attributes\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (ABORT_DST_FLAG)
        {
            int abortResult = UNKNOWN;
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Aborting DST\n");
            }
            abortResult = abort_DST(&deviceList[deviceIter]);
            switch (abortResult)
            {
            case UNKNOWN:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unknown Error occurred while trying to abort DST\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case SUCCESS:
            case ABORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully aborted DST.\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Aborting DST is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Abort DST Failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHORT_DST_FLAG)
        {
            int32_t DSTResult = UNKNOWN;
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Short DST\n");
            }
            DSTResult = run_DST(&deviceList[deviceIter], 1, POLL_FLAG, false, IGNORE_OPERATION_TIMEOUT);
            switch (DSTResult)
            {
            case UNKNOWN:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unknown Error occurred while trying to start DST\n");
                }
                break;
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (POLL_FLAG)
                    {
                        printf("Short DST Passed!\n");
                    }
                    else
                    {
                        printf("Short DST started!\n");
                        printf("use --progress dst -d %s to monitor Drive Self Test progress\n", deviceHandleExample);
                        printf("use --abortDST -d %s to stop Drive Self Test\n", deviceHandleExample);
                    }
                }
                break;
            case IN_PROGRESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A self test is currently in progress.\n");
                }
                break;
            case ABORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Short DST was aborted! You can add the --%s flag to allow DST to continue\n", IGNORE_OPERATION_TIMEOUT_LONG_OPT_STRING);
                    printf("running despite taking longer than expected.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Short DST is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Short DST Failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (DOWNLOAD_FW_FLAG)
        {
            FILE* firmwareFilePtr = NULL;
            bool fileOpenedSuccessfully = true;//assume true in case of activate command
            if (DOWNLOAD_FW_MODE != DL_FW_ACTIVATE)
            {
                //open the file and send the download
                if ((firmwareFilePtr = fopen(DOWNLOAD_FW_FILENAME_FLAG, "rb")) == NULL)
                {
                    fileOpenedSuccessfully = false;
                }
            }
            if (DOWNLOAD_FW_MODE == DL_FW_ACTIVATE)
            {
                //this shouldn't fall into this code path anymore...
                fileOpenedSuccessfully = false;
            }
            if (fileOpenedSuccessfully)
            {
                size_t firmwareFileSize = C_CAST(size_t, get_File_Size(firmwareFilePtr));
                uint8_t* firmwareMem = C_CAST(uint8_t*, calloc_aligned(firmwareFileSize, sizeof(uint8_t), deviceList[deviceIter].os_info.minimumAlignment));
                if (firmwareMem)
                {
                    if (firmwareFileSize == fread(firmwareMem, sizeof(uint8_t), firmwareFileSize, firmwareFilePtr))
                    {
                        firmwareUpdateData dlOptions;
                        memset(&dlOptions, 0, sizeof(firmwareUpdateData));
                        dlOptions.size = sizeof(firmwareUpdateData);
                        dlOptions.version = FIRMWARE_UPDATE_DATA_VERSION;
                        dlOptions.dlMode = DOWNLOAD_FW_MODE;
                        dlOptions.segmentSize = 0;
                        dlOptions.ignoreStatusOfFinalSegment = M_ToBool(FWDL_IGNORE_FINAL_SEGMENT_STATUS_FLAG);
                        dlOptions.firmwareFileMem = firmwareMem;
                        dlOptions.firmwareMemoryLength = C_CAST(uint32_t, firmwareFileSize);//firmware files shouldn't be larger than a few MBs for a LONG time
                        dlOptions.firmwareSlot = 0;
                        ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                        switch (ret)
                        {
                        case SUCCESS:
                        case POWER_CYCLE_REQUIRED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Firmware Download successful\n");
                            }
                            if (ret == POWER_CYCLE_REQUIRED)
                            {
                                printf("The Operating system has reported that a power cycle is required to complete the firmware update\n");
                            }
                            if (DOWNLOAD_FW_MODE == DL_FW_DEFERRED)
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Firmware download complete. Reboot or run the --%s command to finish installing the firmware.\n", ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
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
                                    printf("New firmware version is %s\n", deviceList[deviceIter].drive_info.product_revision);
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
                    safe_Free_aligned(firmwareMem)
                }
                else
                {
                    perror("failed to allocate memory");
                    exit(255);
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    perror("fopen");
                    printf("Couldn't open file %s\n", DOWNLOAD_FW_FILENAME_FLAG);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (ACTIVATE_DEFERRED_FW_FLAG)
        {
            supportedDLModes supportedFWDLModes;
            memset(&supportedFWDLModes, 0, sizeof(supportedDLModes));
            supportedFWDLModes.size = sizeof(supportedDLModes);
            supportedFWDLModes.version = SUPPORTED_FWDL_MODES_VERSION;
            get_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes);
            if (supportedFWDLModes.deferred || supportedFWDLModes.scsiInfoPossiblyIncomplete)
            {
                firmwareUpdateData dlOptions;
                memset(&dlOptions, 0, sizeof(firmwareUpdateData));
                dlOptions.size = sizeof(firmwareUpdateData);
                dlOptions.version = FIRMWARE_UPDATE_DATA_VERSION;
                dlOptions.dlMode = FWDL_UPDATE_MODE_ACTIVATE;
                dlOptions.segmentSize = 0;
                dlOptions.firmwareFileMem = NULL;
                dlOptions.firmwareMemoryLength = 0;
                dlOptions.firmwareSlot = 0;
                dlOptions.ignoreStatusOfFinalSegment = false;//NOTE: This flag is not needed or used on products that support deferred download today.
                if (DOWNLOAD_FW_FLAG)
                {
                    //delay a second as this can help if running a download immediately followed by activate-TJE
                    delay_Seconds(1);
                }
                ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                switch (ret)
                {
                case SUCCESS:
                case POWER_CYCLE_REQUIRED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Firmware activation successful\n");
                        if (ret == POWER_CYCLE_REQUIRED)
                        {
                            printf("The Operating system has reported that a power cycle is required to complete the firmware update\n");
                        }
                        else
                        {
                            fill_Drive_Info_Data(&deviceList[deviceIter]);
                            printf("New firmware version is %s\n", deviceList[deviceIter].drive_info.product_revision);
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
                        printf("Firmware activate not supported\n");
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

        if (SET_PHY_SPEED_FLAG)
        {
            switch (set_phy_speed(&deviceList[deviceIter], SET_PHY_SPEED_GEN, SET_PHY_ALL_PHYS, SET_PHY_SAS_PHY_IDENTIFIER))
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
                exitCode = 0;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully changed ready LED behavior!\n");
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
            //this means we are performing a TRIM/UNMAP operation, then setting the maxlba. Since we've already set the trim start location, time to set the range
            TRIM_UNMAP_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba - TRIM_UNMAP_START_FLAG;
            RUN_TRIM_UNMAP_FLAG = true;
        }

        if (RUN_TRIM_UNMAP_FLAG)
        {
            uint64_t localStartLBA = TRIM_UNMAP_START_FLAG;
            uint64_t localRange = TRIM_UNMAP_RANGE_FLAG;
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
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG == UINT64_MAX || (localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba)
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
                            printf("Successfully trimmed/unmapped LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
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
                            printf("Failed to trim/unmap LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
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
                        printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample, TRIM_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, POSSIBLE_DATA_ERASE_ACCEPT_STRING);
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
                //check the time
                uint64_t overwriteSeconds = SECONDS_TIME_FLAG + (MINUTES_TIME_FLAG * 60) + (HOURS_TIME_FLAG * 3600);
                //determine if it's timed or a range
                if (overwriteSeconds == 0)
                {
                    int overwriteRet = UNKNOWN;
                    uint64_t localStartLBA = OVERWRITE_START_FLAG;
                    uint64_t localRange = OVERWRITE_RANGE_FLAG;
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
                    //range based overwrite
                    if ((localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba || localRange == UINT64_MAX || localRange == 0)
                    {
                        localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
                    }
                    overwriteRet = erase_Range(&deviceList[deviceIter], localStartLBA, localStartLBA + localRange, NULL, 0, HIDE_LBA_COUNTER);
                    switch (overwriteRet)
                    {
                    case SUCCESS:
                        exitCode = 0;
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Successfully overwrote LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
                        }
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
                            printf("Failed to erase LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    if (overwriteSeconds > 0)
                    {
                        int overwriteRet = UNKNOWN;
                        overwriteRet = erase_Time(&deviceList[deviceIter], OVERWRITE_START_FLAG, overwriteSeconds, NULL, 0, HIDE_LBA_COUNTER);
                        switch (overwriteRet)
                        {
                        case SUCCESS:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Successfully overwrote LBAs!\n");
                            }
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
                    printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (SET_MAX_LBA_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Setting MaxLBA to %"PRIu64"\n", SET_MAX_LBA_VALUE);
            }
            switch (set_Max_LBA(&deviceList[deviceIter], SET_MAX_LBA_VALUE, false))
            {
            case SUCCESS:
                fill_Drive_Info_Data(&deviceList[deviceIter]);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    double mCapacity = 0, capacity = 0;
                    char mCapUnits[UNIT_STRING_LENGTH] = { 0 }, capUnits[UNIT_STRING_LENGTH] = { 0 };
                    char* mCapUnit = &mCapUnits[0], * capUnit = &capUnits[0];
                    mCapacity = C_CAST(double, deviceList[deviceIter].drive_info.deviceMaxLba * deviceList[deviceIter].drive_info.deviceBlockSize);
                    capacity = mCapacity;
                    metric_Unit_Convert(&mCapacity, &mCapUnit);
                    capacity_Unit_Convert(&capacity, &capUnit);
                    printf("Successfully set the max LBA to %" PRIu64 "\n", SET_MAX_LBA_VALUE);
                    printf("New Drive Capacity (%s/%s): %0.02f/%0.02f\n", mCapUnit, capUnit, mCapacity, capacity);
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
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Restoring max LBA\n");
            }
            switch (set_Max_LBA(&deviceList[deviceIter], 0, true))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    double mCapacity = 0, capacity = 0;
                    char mCapUnits[UNIT_STRING_LENGTH] = { 0 }, capUnits[UNIT_STRING_LENGTH] = { 0 };
                    char* mCapUnit = &mCapUnits[0], * capUnit = &capUnits[0];
                    mCapacity = C_CAST(double, deviceList[deviceIter].drive_info.deviceMaxLba * deviceList[deviceIter].drive_info.deviceBlockSize);
                    capacity = mCapacity;
                    metric_Unit_Convert(&mCapacity, &mCapUnit);
                    capacity_Unit_Convert(&capacity, &capUnit);
                    printf("Successfully restored the max LBA\n");
                    printf("New Drive Capacity (%s/%s): %0.02f/%0.02f\n", mCapUnit, capUnit, mCapacity, capacity);
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

        if (PROGRESS_CHAR != NULL)
        {
            int result = UNKNOWN;
            //first take whatever was entered in progressTest and convert it to uppercase to do fewer string comparisons
            convert_String_To_Upper_Case(progressTest);
            //do some string comparisons to figure out what we are checking for progress on
            if (strcmp(progressTest, "DST") == 0 ||
                strcmp(progressTest, "SHORTDST") == 0 ||
                strcmp(progressTest, "LONGDST") == 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting DST progress.\n");
                }
                result = print_DST_Progress(&deviceList[deviceIter]);
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unknown test \"%s\" for requested progress\n", progressTest);
                }
            }
            switch (result)
            {
            case UNKNOWN:
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                break;
            case SUCCESS:
            case IN_PROGRESS:
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
        //At this point, close the device handle since it is no longer needed. Do not put any further IO below this.
        close_Device(&deviceList[deviceIter]);
    }
    safe_Free(DEVICE_LIST);
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
    //everything needs a help option right?
    printf("Usage\n");
    printf("=====\n");
    printf("\t %s [-d %s] {arguments} {options}\n\n", util_name, deviceHandleName);

    printf("Examples\n");
    printf("========\n");
    //example usage
    printf("\t%s --%s\n", util_name, SCAN_LONG_OPT_STRING);
    printf("\t%s -d %s -%c\n", util_name, deviceHandleExample, DEVICE_INFO_SHORT_OPT);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SAT_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LOWLEVEL_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SMART_CHECK_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s\n", util_name, deviceHandleExample, SHORT_DST_LONG_OPT_STRING, POLL_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, ABORT_DST_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, CHECK_POWER_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SPIN_DOWN_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, TEST_UNIT_READY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s hybrid\n", util_name, deviceHandleExample, SMART_ATTRIBUTES_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_CONCURRENT_RANGES_LONG_OPT_STRING);
    printf("\t%s -d %s --%s info\n", util_name, deviceHandleExample, SET_READY_LED_LONG_OPT_STRING);
    printf("\t%s -d %s --%s on\n", util_name, deviceHandleExample, SET_READY_LED_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 2\n", util_name, deviceHandleExample, SET_PHY_SPEED_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 3 --%s 1\n", util_name, deviceHandleExample, SET_PHY_SPEED_LONG_OPT_STRING, SET_PHY_SAS_PHY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s info\n", util_name, deviceHandleExample, READ_LOOK_AHEAD_LONG_OPT_STRING);
    printf("\t%s -d %s --%s enable\n", util_name, deviceHandleExample, READ_LOOK_AHEAD_LONG_OPT_STRING);
    printf("\t%s -d %s --%s info\n", util_name, deviceHandleExample, WRITE_CACHE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s disable\n", util_name, deviceHandleExample, WRITE_CACHE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s firmwareFile.bin\n", util_name, deviceHandleExample, DOWNLOAD_FW_LONG_OPT_STRING);
    printf("\t%s -d %s --%s firmwareFile.bin --%s deferred --%s\n", util_name, deviceHandleExample, DOWNLOAD_FW_LONG_OPT_STRING, DOWNLOAD_FW_MODE_LONG_OPT_STRING, ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 1000\n", util_name, deviceHandleExample, DISPLAY_LBA_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 1000 --%s 2000\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING, OVERWRITE_RANGE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0 --%s 1\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING, HOURS_TIME_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0\n", util_name, deviceHandleExample, TRIM_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 1000 --%s 2000\n", util_name, deviceHandleExample, TRIM_LONG_OPT_STRING, TRIM_RANGE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 134217728\n", util_name, deviceHandleExample, PROVISION_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 134217728\n", util_name, deviceHandleExample, SET_MAX_LBA_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, RESTORE_MAX_LBA_LONG_OPT_STRING);
    //return codes
    printf("\nReturn codes\n");
    printf("============\n");
    print_SeaChest_Util_Exit_Codes(0, NULL, util_name);

    //utility options - alphabetized
    printf("\nUtility Options\n");
    printf("===============\n");
#if defined (ENABLE_CSMI)
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
    print_Time_Hours_Help(shortUsage);
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_Time_Minutes_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_No_Time_Limit_Help(shortUsage);
    print_No_Banner_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_Time_Seconds_Help(shortUsage);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    //the test options
    printf("\nUtility Arguments\n");
    printf("=================\n");
    //Common (across utilities) - alphabetized
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Agressive_Scan_Help(shortUsage);
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Device_Information_Help(shortUsage);
    print_Low_Level_Info_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    print_Fast_Discovery_Help(shortUsage);
    print_Check_Power_Mode_Help(shortUsage);
    print_Display_LBA_Help(shortUsage);
    print_Firmware_Activate_Help(shortUsage);
    print_Firmware_Download_Help(shortUsage);
    print_Firmware_Download_Mode_Help(shortUsage);
    print_FWDL_Ignore_Final_Segment_Help(shortUsage);
    print_Short_DST_Help(shortUsage);
    print_Poll_Help(shortUsage);
    print_Progress_Help(shortUsage, "dst");
    print_Abort_DST_Help(shortUsage);
    print_Phy_Speed_Help(shortUsage);
    print_Read_Look_Ahead_Help(shortUsage);
    print_Restore_Max_LBA_Help(shortUsage);
    print_Set_Max_LBA_Help(shortUsage);
    print_Show_Concurrent_Position_Ranges_Help(shortUsage);
    print_SMART_Check_Help(shortUsage);
    print_Spindown_Help(shortUsage);
    print_Write_Cache_Help(shortUsage);

    //SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_SMART_Attributes_Help(shortUsage);

    //SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");
    print_Set_Ready_LED_Help(shortUsage);
    print_SAS_Phy_Help(shortUsage);


    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //utility data destructive tests/operations go here
    print_Overwrite_Help(shortUsage);
    print_Overwrite_Range_Help(shortUsage);
    print_Provision_Help(shortUsage);
    print_Trim_Unmap_Help(shortUsage);
    print_Trim_Unmap_Range_Help(shortUsage);
}
