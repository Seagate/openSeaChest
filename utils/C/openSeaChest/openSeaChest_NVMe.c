//
// openSeaChest_NVMe.c
//
// Copyright (c) 2014-2018 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
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
#include <stdio.h>
#include <ctype.h>
#if defined (__unix__) || defined(__APPLE__) //using this definition because linux and unix compilers both define this. Apple does not define this, which is why it has it's own definition
#include <unistd.h>
#include <getopt.h>
#elif defined (_WIN32)
#include "getopt.h"
#else
#error "OS Not Defined or known"
#endif
#include <math.h>
#include "common.h"
#include "EULA.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "drive_info.h"
#include "firmware_download.h"
#include "power_control.h"
#include "smart.h"
#include "logs.h"
#include "nvme_operations.h"
#include "format.h"

////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_NVMe";
const char *buildVersion = "1.1.0";

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
int32_t main(int argc, char *argv[])
{
    eUtilExitCodes      exitCode = UTIL_EXIT_NO_ERROR;
#if !defined(DISABLE_NVME_PASSTHROUGH)
    /////////////////
    //  Variables  //
    /////////////////
    //common utility variables
    int                 ret = SUCCESS;
    DEVICE_UTIL_VARS
    DEVICE_INFO_VAR
    SAT_INFO_VAR
    DATA_ERASE_VAR
    LICENSE_VAR
    ECHO_COMMAND_LINE_VAR
    SCAN_FLAG_VAR
    SHOW_BANNER_VAR
    SHOW_HELP_VAR
    TEST_UNIT_READY_VAR
    DOWNLOAD_FW_VARS
    ACTIVATE_DEFERRED_FW_VAR
    SWITCH_FW_VAR
    FWDL_SEGMENT_SIZE_VARS
    FW_MATCH_VARS
    FIRMWARE_SLOT_VAR
    NEW_FW_MATCH_VARS
    CHECK_POWER_VAR
    TRANSITION_POWER_STATE_VAR
    GET_NVME_LOG_VAR
    GET_TELEMETRY_VAR
    TELEMETRY_DATA_AREA_VAR
    OUTPUT_MODE_VAR
    GET_FEATURES_VAR
    NVME_TEMP_STATS_VAR
    NVME_PCI_STATS_VAR
    MODEL_MATCH_VARS
    //CHILD_MODEL_MATCH_VARS
    //CHILD_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    //POWER_MODE_VAR
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    EXT_SMART_LOG_VAR1
    CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_VAR
    NVM_FORMAT_VARS
    NVM_FORMAT_OPTION_VARS
    POLL_VAR
    PROGRESS_VAR
    SHOW_SUPPORTED_FORMATS_VAR

    int8_t  args = 0;
    uint8_t argIndex = 0;
    int32_t optionIndex = 0;

    firmwareUpdateData dlOptions;
    seatimer_t commandTimer;

    //add -- options to this structure DO NOT ADD OPTIONAL ARGUMENTS! Optional arguments are a GNU extension and are not supported in Unix or some compilers- TJE
    struct option longopts[] = {
        //common command line options
        DEVICE_LONG_OPT,
        HELP_LONG_OPT,
        DEVICE_INFO_LONG_OPT,
        SAT_INFO_LONG_OPT,
        //USB_CHILD_INFO_LONG_OPT,
        SCAN_LONG_OPT,
        SCAN_FLAGS_LONG_OPT,
        VERSION_LONG_OPT,
        VERBOSE_LONG_OPT,
        QUIET_LONG_OPT,
        LICENSE_LONG_OPT,
        ECHO_COMMAND_LIN_LONG_OPT,
        TEST_UNIT_READY_LONG_OPT,
        POLL_LONG_OPT,
        PROGRESS_LONG_OPT,
        //tool specific options go here
        DOWNLOAD_FW_MODE_LONG_OPT,
        DOWNLOAD_FW_LONG_OPT,
        NEW_FW_MATCH_LONG_OPT,
        FWDL_SEGMENT_SIZE_LONG_OPT,
        ACTIVATE_DEFERRED_FW_LONG_OPT,
        SWITCH_FW_LONG_OPT,
        FIRMWARE_SLOT_BUFFER_ID_LONG_OPT,
        CHECK_POWER_LONG_OPT,
        TRANSITION_POWER_STATE_LONG_OPT,
        GET_NVME_LOG_LONG_OPT,
        GET_TELEMETRY_LONG_OPT,
        TELEMETRY_DATA_AREA_LONG_OPT,
        CONFIRM_LONG_OPT,
        OUTPUT_MODE_LONG_OPT,
        GET_FEATURES_LONG_OPT,
        EXT_SMART_LOG_LONG_OPT1,
        MODEL_MATCH_LONG_OPT,
        FW_MATCH_LONG_OPT,
//      CHILD_MODEL_MATCH_LONG_OPT,
//      CHILD_FW_MATCH_LONG_OPT,
        CLEAR_PCIE_CORRECTABLE_ERRORS_LONG_OPT,
        NVME_TEMP_STATS_LONG_OPT,
        NVME_PCI_STATS_LONG_OPT,
        SHOW_SUPPORTED_FORMATS_LONG_OPT,
        NVM_FORMAT_LONG_OPT,
        NVM_FORMAT_OPTIONS_LONG_OPTS,
        LONG_OPT_TERMINATOR
    };

    eVerbosityLevels toolVerbosity = VERBOSITY_DEFAULT;
    DOWNLOAD_FW_MODE = DL_FW_UNKNOWN;

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
        args = getopt_long(argc, argv, "d:hisF:Vv:q%:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        //printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:
            //parse long options that have no short option and required arguments here
            //parse long options that have no short option and required arguments here
            if (strcmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING) == 0)
            {
                if (strlen(optarg) == strlen(DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, DATA_ERASE_ACCEPT_STRING, strlen(DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    DATA_ERASE_FLAG = true;
                }
            }
            else if (strncmp(longopts[optionIndex].name, OUTPUT_MODE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(OUTPUT_MODE_LONG_OPT_STRING))) == 0)
            {
                if (strncmp(optarg, "raw", strlen(optarg)) == 0)
                {
                    OUTPUT_MODE_IDENTIFIER = UTIL_OUTPUT_MODE_RAW;
                }
                else if (strncmp(optarg, "binary", strlen(optarg)) == 0)
                {
                    OUTPUT_MODE_IDENTIFIER = UTIL_OUTPUT_MODE_BIN;
                }
                else
                {
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    printf("\nerror processing --%s\n\n",longopts[optionIndex].name);
                    printf("Please use -h option to print help\n\n");
                }

            }
            else if (strncmp(longopts[optionIndex].name, GET_FEATURES_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(GET_FEATURES_LONG_OPT_STRING))) == 0)
            {
                if (isdigit(optarg[0]))
                {
                    GET_FEATURES_IDENTIFIER = atoi(optarg);
                }
                else if (strncmp(optarg, "help", strlen(optarg)) == 0)
                {
                    GET_FEATURES_IDENTIFIER = 0;
                }
                else if (strncmp(optarg, "list", strlen(optarg)) == 0)
                {
                    GET_FEATURES_IDENTIFIER = 0x0E;//Using a reserved value - Revisit later
                }
                else
                {
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    printf("\nerror processing --%s\n\n",longopts[optionIndex].name);
                    printf("Please use -h option to print help\n\n");
                }

            }
            else if (strncmp(longopts[optionIndex].name, NVME_TEMP_STATS_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(NVME_TEMP_STATS_LONG_OPT_STRING))) == 0)
            {
                NVME_TEMP_STATS_FLAG = goTrue;
            }
            else if (strncmp(longopts[optionIndex].name, NVME_PCI_STATS_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(NVME_PCI_STATS_LONG_OPT_STRING))) == 0)
            {
                NVME_PCI_STATS_FLAG = goTrue;
            }
            else if (strncmp(longopts[optionIndex].name, DOWNLOAD_FW_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DOWNLOAD_FW_LONG_OPT_STRING))) == 0)
            {
                DOWNLOAD_FW_FLAG = true;
                sscanf(optarg, "%s", DOWNLOAD_FW_FILENAME_FLAG);
            }
            else if (strncmp(longopts[optionIndex].name, DOWNLOAD_FW_MODE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DOWNLOAD_FW_MODE_LONG_OPT_STRING))) == 0)
            {
                USER_SET_DOWNLOAD_MODE = true;
                DOWNLOAD_FW_MODE = DL_FW_SEGMENTED;
                if (strncmp(optarg, "immediate", strlen(optarg)) == 0 || strncmp(optarg, "full", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_FULL;
                }
                else if (strncmp(optarg, "segmented", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_SEGMENTED;
                }
                else if (strncmp(optarg, "deferred", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_DEFERRED;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(DOWNLOAD_FW_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, FWDL_SEGMENT_SIZE_LONG_OPT_STRING) == 0)
            {
                FWDL_SEGMENT_SIZE_FROM_USER = true;
                FWDL_SEGMENT_SIZE_FLAG = (uint16_t)atoi(optarg);
            }
            else if (strcmp(longopts[optionIndex].name, FIRMWARE_SLOT_LONG_OPT_STRING) == 0 || strcmp(longopts[optionIndex].name, FIRMWARE_BUFFER_ID_LONG_OPT_STRING) == 0)
            {
                FIRMWARE_SLOT_FLAG = (uint8_t)atoi(optarg);
                if (FIRMWARE_SLOT_FLAG > 7)
                {
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("FirmwareSlot/FwBuffer ID must be between 0 and 7\n");
                    }
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, TRANSITION_POWER_STATE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(TRANSITION_POWER_STATE_LONG_OPT_STRING))) == 0)
            {
                //set the timer value
                TRANSITION_POWER_STATE_TO = atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, GET_NVME_LOG_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(GET_NVME_LOG_LONG_OPT_STRING))) == 0)
            {
                //set the power mode
                if (isdigit(optarg[0]))//this will get the valid NVMe power levels
                {
                    GET_NVME_LOG_IDENTIFIER = atoi(optarg);
                }
                else
                {
                    if (strncmp("error", optarg, strlen(optarg)) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_ERROR_ID;
                    }
                    else if (strncmp("smart", optarg, strlen(optarg)) == 0 || strncmp("SMART", optarg, strlen(optarg)) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_SMART_ID;
                    }
                    else if (strncmp("fwSlots", optarg, strlen(optarg)) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_FW_SLOT_ID;
                    }
                    else if (strncmp("suppCmds", optarg, strlen(optarg)) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_CMD_SPT_EFET_ID;
                    }
                    else if (strncmp("selfTest", optarg, strlen(optarg)) == 0)
                    {
                        GET_NVME_LOG_IDENTIFIER = NVME_LOG_DEV_SELF_TEST;
                    }
                    else
                    {
                        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                        printf("\nerror processing --%s\n\n",longopts[optionIndex].name);
                        printf("Please use -h option to print help\n\n");
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, GET_TELEMETRY_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(GET_TELEMETRY_LONG_OPT_STRING))) == 0)
            {
                if (strncmp("host", optarg, strlen(optarg)) == 0)
                {
                    GET_TELEMETRY_IDENTIFIER = NVME_LOG_TELEMETRY_HOST;
                }
                else if (strncmp("ctrl", optarg, strlen(optarg)) == 0)
                {
                    GET_TELEMETRY_IDENTIFIER = NVME_LOG_TELEMETRY_CTRL;
                }
                else
                {
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    printf("\nerror processing --%s\n\n",longopts[optionIndex].name);
                    printf("Please use -h option to print help\n\n");
                }
            }
            else if (strncmp(longopts[optionIndex].name, TELEMETRY_DATA_AREA_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(TELEMETRY_DATA_AREA_LONG_OPT_STRING))) == 0)
            {
                //set the telemetry data area
                if (isdigit(optarg[0]))//this will get the valid NVMe telemetry data area
                {
                    TELEMETRY_DATA_AREA = atoi(optarg);
                }
                else
                {
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    printf("\nerror processing --%s\n\n",longopts[optionIndex].name);
                    printf("Please use -h option to print help\n\n");
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_LONG_OPT_STRING) == 0)
            {
                NVM_FORMAT_FLAG = true;
                if (strcmp(optarg, "current") != 0)
                {
                    //set the sector size
                    NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM = (uint32_t)atoi(optarg);
                }
            }
            else if (strcmp(longopts[optionIndex].name, NVM_FORMAT_NSID_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "current") == 0)
                {
                    NVM_FORMAT_NSID = 0;//detect this below and insert the correct NSID for the current handle
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
                NVM_FORMAT_PI_TYPE = (uint8_t)atoi(optarg);
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
                NVM_FORMAT_METADATA_SIZE = (uint32_t)atoi(optarg);
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
            else if (strncmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(MODEL_MATCH_LONG_OPT_STRING))) == 0)
            {
                MODEL_MATCH_FLAG = true;
                strncpy(MODEL_STRING_FLAG, optarg, M_Min(40, strlen(optarg)));
            }
            else if (strncmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                FW_MATCH_FLAG = true;
                strncpy(FW_STRING_FLAG, optarg, M_Min(9, strlen(optarg)));
            }
            else if (strncmp(longopts[optionIndex].name, NEW_FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(NEW_FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                NEW_FW_MATCH_FLAG = true;
                strncpy(NEW_FW_STRING_FLAG, optarg, M_Min(9, strlen(optarg)));
            }
//          else if (strncmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_MODEL_MATCH_LONG_OPT_STRING))) == 0)
//          {
//              CHILD_MODEL_MATCH_FLAG = true;
//              strncpy(CHILD_MODEL_STRING_FLAG, optarg, M_Min(40, strlen(optarg)));
//          }
//          else if (strncmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_FW_MATCH_LONG_OPT_STRING))) == 0)
//          {
//              CHILD_FW_MATCH_FLAG = true;
//              strncpy(CHILD_FW_STRING_FLAG, optarg, M_Min(9, strlen(optarg)));
//          }
            break;
        case ':'://missing required argument
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            switch (optopt)
            {
            case 0:
                    //check long options for missing arguments
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
        case VERSION_SHORT_OPT:
            SHOW_BANNER_FLAG = true;
            break;
        case VERBOSE_SHORT_OPT: //verbose
            if (optarg != NULL)
            {
                toolVerbosity = atoi(optarg);
            }
            break;
        case PROGRESS_SHORT_OPT: //get test progress for a specific test
            PROGRESS_CHAR = optarg;
            break;
        case QUIET_SHORT_OPT: //quiet mode
            toolVerbosity = VERBOSITY_QUIET;
            break;
        case SCAN_FLAGS_SHORT_OPT://scan flags
            SCAN_FLAGS_SUBOPT_PARSING;
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
        default:
            break;
        }
    }

    atexit(print_Final_newline);

    if (ECHO_COMMAND_LINE_FLAG)
    {
        uint64_t commandLineIter = 1;//start at 1 as starting at 0 means printing the directory info+ SeaChest.exe (or ./SeaChest)
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

    if (VERBOSITY_QUIET < toolVerbosity)
    {
        openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
    }

    if (SHOW_BANNER_FLAG)
    {
        utility_Full_Version_Info(util_name, buildVersion, OPENSEA_TRANSPORT_MAJOR_VERSION, OPENSEA_TRANSPORT_MINOR_VERSION, OPENSEA_TRANSPORT_PATCH_VERSION, OPENSEA_COMMON_VERSION, OPENSEA_OPERATION_VERSION);
    }

    if (LICENSE_FLAG)
    {
        print_EULA_To_Screen(false, false);
    }

    if (SCAN_FLAG)
    {
        if (!is_Running_Elevated())
        {
            print_Elevated_Privileges_Text();
            exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
        }
        unsigned int scanControl = DEFAULT_SCAN;
        #if defined (__linux__)
        if (scanSD)
        {
            scanControl |= SD_HANDLES;
        }
        if (scanSDandSG)
        {
            scanControl |= SG_TO_SD;
        }
        #endif
        //set the drive types to show (if none are set, the lower level code assumes we need to show everything)
        if (scanATA)
        {
            scanControl |= ATA_DRIVES;
        }
        if (scanUSB)
        {
            scanControl |= USB_DRIVES;
        }
        if (scanSCSI)
        {
            scanControl |= SCSI_DRIVES;
        }
        if (scanNVMe)
        {
            scanControl |= NVME_DRIVES;
        }
        if (scanRAID)
        {
            scanControl |= RAID_DRIVES;
        }
        //set the interface types to show (if none are set, the lower level code assumes we need to show everything)
        if (scanInterfaceATA)
        {
            scanControl |= IDE_INTERFACE_DRIVES;
        }
        if (scanInterfaceUSB)
        {
            scanControl |= USB_INTERFACE_DRIVES;
        }
        if (scanInterfaceSCSI)
        {
            scanControl |= SCSI_INTERFACE_DRIVES;
        }
        if (scanInterfaceNVMe)
        {
            scanControl |= NVME_INTERFACE_DRIVES;
        }
        scan_And_Print_Devs(scanControl, NULL, toolVerbosity);
    }
    // Add to this if list anything that is suppose to be independent.
    // e.g. you can't say enumerate & then pull logs in the same command line.
    // SIMPLE IS BEAUTIFUL
    if (SCAN_FLAG || SHOW_BANNER_FLAG || LICENSE_FLAG || SHOW_HELP_FLAG)
    {
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_NO_ERROR);
    }
    else if (exitCode == UTIL_EXIT_ERROR_IN_COMMAND_LINE)
    {
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
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
        exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
    }

    //the following flags imply running on all drives.
    if ((MODEL_MATCH_FLAG || FW_MATCH_FLAG || ONLY_SEAGATE_FLAG) && !RUN_ON_ALL_DRIVES)
    {
        RUN_ON_ALL_DRIVES = true;
    }
    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        if (SUCCESS != get_Device_Count(&numberOfDevices, 0))
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Unable to get number of devices\n");
            }
            exit(UTIL_EXIT_OPERATION_FAILURE);
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

    //check that we were given at least one test to perform...if not, set that we are dumping device information so we at least do something
    if (!(DEVICE_INFO_FLAG
          || DOWNLOAD_FW_FLAG
          || ACTIVATE_DEFERRED_FW_FLAG
          || SWITCH_FW_FLAG
          || TEST_UNIT_READY_FLAG
          || CHECK_POWER_FLAG
          || (TRANSITION_POWER_STATE_TO >= 0)
          || (GET_NVME_LOG_IDENTIFIER > 0) // Since 0 is Reserved
          || (GET_TELEMETRY_IDENTIFIER > 0)
          || (GET_FEATURES_IDENTIFIER >= 0)
          || EXT_SMART_LOG_FLAG1
          || CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_FLAG
          || NVME_TEMP_STATS_FLAG
          || NVME_PCI_STATS_FLAG
          || SHOW_SUPPORTED_FORMATS_FLAG
          || NVM_FORMAT_FLAG
        //check for other tool specific options here
        ))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    uint64_t flags = 0;
    DEVICE_LIST = (tDevice*)calloc(DEVICE_LIST_COUNT, sizeof(tDevice));
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

    if (TEST_UNIT_READY_FLAG)
    {
        flags = DO_NOT_WAKE_DRIVE;
    }

    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
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
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unable to get device list\n");
                }
                exit(UTIL_EXIT_OPERATION_FAILURE);
            }
        }
    }
    else
    {
        /*need to go through the handle list and attempt to open each handle.*/
        for (uint16_t handleIter = 0; handleIter < DEVICE_LIST_COUNT; ++handleIter)
        {
            /*Initializing is necessary*/
            deviceList[handleIter].sanity.size = sizeof(tDevice);
            deviceList[handleIter].sanity.version = DEVICE_BLOCK_VERSION;
#if defined (UEFI_C_SOURCE)
            deviceList[handleIter].os_info.fd = NULL;
#elif !defined(_WIN32)
            deviceList[handleIter].os_info.fd = -1;
#if defined(VMK_CROSS_COMP)
            deviceList[handleIter].os_info.nvmeFd = NULL;
#endif
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
                exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) is not a Seagate drive.\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification);
                }
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

        if (deviceList[deviceIter].drive_info.drive_type != NVME_DRIVE)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("%s - Not an NVMe device, skipping\n", deviceList[deviceIter].os_info.name);
            }
            continue;
        }

        if (VERBOSITY_QUIET < toolVerbosity)
        {
            printf("\n%s - %s - %s - %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification, deviceList[deviceIter].drive_info.serialNumber, print_drive_type(&deviceList[deviceIter]));
        }

        //now start looking at what operations are going to be performed and kick them off
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
                print_Data_Buffer((uint8_t *)&deviceList[deviceIter].drive_info.IdentifyData.nvme.ctrl, sizeof(nvmeIDCtrl), true);
                printf("\nNamespace Identify Information:\n");
                printf("================================\n");
                print_Data_Buffer((uint8_t *)&deviceList[deviceIter].drive_info.IdentifyData.nvme.ns, sizeof(nvmeIDNameSpaces), true);
            }
            else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_BIN)
            {
                FILE *      pIdentifyFile = NULL;
                char identifyFileName[OPENSEA_PATH_MAX] = { 0 };
                char * fileNameUsed = &identifyFileName[0];
                if (SUCCESS == create_And_Open_Log_File(&deviceList[deviceIter], &pIdentifyFile, NULL, \
                    "CTRL_IDENTIFY", "bin", 1, &fileNameUsed))
                {
                    fwrite(&deviceList[deviceIter].drive_info.IdentifyData.nvme.ctrl, sizeof(uint8_t), sizeof(nvmeIDNameSpaces), pIdentifyFile);
                    fflush(pIdentifyFile);
                    fclose(pIdentifyFile);
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Created %s file with Controller Information\n", fileNameUsed);
                    }
                    memset(fileNameUsed, 0, OPENSEA_PATH_MAX);
                    if (SUCCESS == create_And_Open_Log_File(&deviceList[deviceIter], &pIdentifyFile, NULL, \
                        "NAMESPACE_IDENTIFY", "bin", 1, &fileNameUsed))
                    {
                        fwrite(&deviceList[deviceIter].drive_info.IdentifyData.nvme.ns, sizeof(uint8_t), sizeof(nvmeIDNameSpaces), pIdentifyFile);
                        fflush(pIdentifyFile);
                        fclose(pIdentifyFile);
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Created %s file with Namespace Information\n", fileNameUsed);
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

                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: failed to open file to write controller information\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
        }

        if (SHOW_SUPPORTED_FORMATS_FLAG)
        {
            uint32_t numberOfSectorSizes = get_Number_Of_Supported_Sector_Sizes(&deviceList[deviceIter]);
            uint32_t memSize = sizeof(supportedFormats) + sizeof(sectorSize) * numberOfSectorSizes;
            ptrSupportedFormats formats = (ptrSupportedFormats)malloc(memSize);
            if (formats)
            {
                memset(formats, 0, memSize);
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
                safe_Free(formats);
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

        if (GET_FEATURES_IDENTIFIER >= 0)
        {
            if (GET_FEATURES_IDENTIFIER == 0) //help
            {
                nvme_Print_Feature_Identifiers_Help();
            }
            else if (GET_FEATURES_IDENTIFIER == 0x0E) //List them all
            {
                nvme_Print_All_Feature_Identifiers(&deviceList[deviceIter], NVME_CURRENT_FEAT_SEL, false);
            }
            else
            {
                //Get the feature
                if (nvme_Print_Feature_Details(&deviceList[deviceIter], GET_FEATURES_IDENTIFIER, NVME_CURRENT_FEAT_SEL) != SUCCESS)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: failed to get details for feature id %d\n", GET_FEATURES_IDENTIFIER);
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
        }

        if (GET_NVME_LOG_IDENTIFIER > 0) //Since 0 is reserved log
        {
            if (OUTPUT_MODE_IDENTIFIER != UTIL_OUTPUT_MODE_HUMAN)
            {
                uint64_t size = 0;
                uint8_t * logBuffer = NULL;
                nvmeGetLogPageCmdOpts cmdOpts;
                if ((nvme_Get_Log_Size(GET_NVME_LOG_IDENTIFIER, &size) == SUCCESS) && size)
                {
                    memset(&cmdOpts, 0, sizeof(nvmeGetLogPageCmdOpts));
                    if (NVME_LOG_ERROR_ID == GET_NVME_LOG_IDENTIFIER)
                    {
                        size = 32 * size; //Get first 32 entries.
                    }
                    logBuffer = (uint8_t *)calloc((size_t)size, sizeof(uint8_t));
                    if (logBuffer != NULL)
                    {
                        cmdOpts.nsid = NVME_ALL_NAMESPACES;
                        cmdOpts.addr = logBuffer;
                        cmdOpts.dataLen = (uint32_t)size;
                        cmdOpts.lid = GET_NVME_LOG_IDENTIFIER;
                        if (nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts) == SUCCESS)
                        {
                            if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_RAW)
                            {
                                printf("Log Page %d Buffer:\n", GET_NVME_LOG_IDENTIFIER);
                                printf("================================\n");
                                print_Data_Buffer((uint8_t *)logBuffer, (uint32_t)size, true);
                                printf("================================\n");
                            }
                            else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_BIN)
                            {
                                FILE * pLogFile = NULL;
                                char identifyFileName[OPENSEA_PATH_MAX] = { 0 };
                                char * fileNameUsed = &identifyFileName[0];
                                char logName[16];
                                sprintf(logName, "LOG_PAGE_%d", GET_NVME_LOG_IDENTIFIER);
                                if (SUCCESS == create_And_Open_Log_File(&deviceList[deviceIter], &pLogFile, NULL, \
                                    logName, "bin", 1, &fileNameUsed))
                                {
                                    fwrite(logBuffer, sizeof(uint8_t), (size_t)size, pLogFile);
                                    fflush(pLogFile);
                                    fclose(pLogFile);
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("Created %s with Log Page %" PRId32 " Information\n", fileNameUsed, GET_NVME_LOG_IDENTIFIER);
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
                            }
                            else
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Error: Unknown/Unsupported output mode %" PRId32 "\n", OUTPUT_MODE_IDENTIFIER);
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            }
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Error: Could not retrieve Log Page %" PRId32 "\n", GET_NVME_LOG_IDENTIFIER);
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                        safe_Free(logBuffer);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Couldn't allocate %" PRIu64 " bytes buffer needed for Log Page %" PRId32 "\n", size, GET_NVME_LOG_IDENTIFIER);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Log Page %d not available at this time. \n", GET_NVME_LOG_IDENTIFIER);
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
            }
            else //Human Readable.
            {
                switch (GET_NVME_LOG_IDENTIFIER)
                {
                case NVME_LOG_SMART_ID:
                    switch (print_SMART_Attributes(&deviceList[deviceIter], SMART_ATTR_OUTPUT_RAW))
                    {
                    case SUCCESS:
                        //nothing to print here since if it was successful, the log will be printed to the screen
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
                        //nothing to print here since if it was successful, the log will be printed to the screen
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
                        //nothing to print here since if it was successful, the log will be printed to the screen
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
                        //nothing to print here since if it was successful, the log will be printed to the screen
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("A failure occured while trying to get Commands Supported and Effects Information Log\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    break;
                case NVME_LOG_DEV_SELF_TEST:
                    switch (nvme_Print_DevSelfTest_Log_Page(&deviceList[deviceIter]))
                    {
                    case SUCCESS:
                        //nothing to print here since if it was successful, the log will be printed to the screen
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

        if (GET_TELEMETRY_IDENTIFIER > 0) //Since 0 is reserved log
        {
            /**
             * Checking for validty of TELEMETRY_DATA_AREA
             * If it is not in between 1-3 we should exit with error
             */

            if ((TELEMETRY_DATA_AREA < 1) && (TELEMETRY_DATA_AREA > 3))
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%d is an invalid temetry data area. \n", TELEMETRY_DATA_AREA);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;

            }
            else
            {
                uint32_t size = BLOCK_SIZE;
                uint8_t * logBuffer = NULL;
                nvmeGetLogPageCmdOpts cmdOpts;
                uint64_t offset = 0;
                uint64_t fullSize = 0;
                int rtnVal;
                nvmeTemetryLogHdr   *teleHdr;

                memset(&cmdOpts, 0, sizeof(nvmeGetLogPageCmdOpts));

                logBuffer = (uint8_t*)calloc(size, sizeof(uint8_t));

                if (logBuffer != NULL)
                {
                    cmdOpts.nsid = NVME_ALL_NAMESPACES;
                    cmdOpts.addr = logBuffer;
                    cmdOpts.dataLen = size;
                    cmdOpts.lid = GET_TELEMETRY_IDENTIFIER;
                    cmdOpts.offset = offset;

                    rtnVal = nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts);
                    offset += BLOCK_SIZE;

                    if (rtnVal == SUCCESS)
                    {
                        teleHdr = (nvmeTemetryLogHdr *)logBuffer;

#if defined(_DEBUG)
                        printf("Telemetry Data Area 1 : %d \n", teleHdr->teleDataArea1);
                        printf("Telemetry Data Area 2 : %d \n", teleHdr->teleDataArea2);
                        printf("Telemetry Data Area 3 : %d \n", teleHdr->teleDataArea3);
#endif

                        if (TELEMETRY_DATA_AREA == 1)
                        {
                            fullSize = offset + BLOCK_SIZE * teleHdr->teleDataArea1;
                        }

                        if (TELEMETRY_DATA_AREA == 2)
                        {
                            fullSize = offset + BLOCK_SIZE * teleHdr->teleDataArea2;
                        }

                        if (TELEMETRY_DATA_AREA == 3)
                        {
                            fullSize = offset + BLOCK_SIZE * teleHdr->teleDataArea3;
                        }

                        if ((OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_RAW) ||
                            (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_HUMAN))
                        {
                            printf("Log Page %d Buffer:\n", GET_TELEMETRY_IDENTIFIER);
                            printf("================================\n");
                            print_Data_Buffer((uint8_t *)logBuffer, size, true);

                            while (offset < fullSize)
                            {
                                memset(logBuffer, 0, size);
                                cmdOpts.nsid = NVME_ALL_NAMESPACES;
                                cmdOpts.addr = logBuffer;
                                cmdOpts.dataLen = size;
                                cmdOpts.lid = GET_TELEMETRY_IDENTIFIER;
                                cmdOpts.offset = offset;

                                rtnVal = nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts);
                                offset += BLOCK_SIZE;

                                if (rtnVal != SUCCESS)
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("Error: Could not retrieve Log Page %" PRId32 " for offset %" PRIu64 "\n", GET_TELEMETRY_IDENTIFIER, offset - BLOCK_SIZE);
                                    }
                                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                    break;
                                }

                                print_Data_Buffer((uint8_t *)logBuffer, size, true);
                            }
                            printf("================================\n");
                        }
                        else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_BIN)
                        {
                            FILE * pLogFile = NULL;
                            char identifyFileName[OPENSEA_PATH_MAX] = { 0 };
                            char * fileNameUsed = &identifyFileName[0];
                            char logName[16];
                            sprintf(logName, "LOG_PAGE_%d", GET_NVME_LOG_IDENTIFIER);
                            if (SUCCESS == create_And_Open_Log_File(&deviceList[deviceIter], &pLogFile, NULL, \
                                logName, "bin", 1, &fileNameUsed))
                            {
                                fwrite(logBuffer, sizeof(uint8_t), size, pLogFile);

                                while (offset < fullSize)
                                {
                                    memset(logBuffer, 0, size);
                                    cmdOpts.nsid = NVME_ALL_NAMESPACES;
                                    cmdOpts.addr = logBuffer;
                                    cmdOpts.dataLen = size;
                                    cmdOpts.lid = GET_TELEMETRY_IDENTIFIER;
                                    cmdOpts.offset = offset;

                                    rtnVal = nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts);
                                    offset += BLOCK_SIZE;

                                    if (rtnVal != SUCCESS)
                                    {
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("Error: Could not retrieve Log Page %" PRId32 " for offset %" PRIu64 "\n", GET_TELEMETRY_IDENTIFIER, offset - BLOCK_SIZE);
                                        }
                                        exitCode = UTIL_EXIT_OPERATION_FAILURE;

                                        break;
                                    }

                                    fwrite(logBuffer, sizeof(uint8_t), size, pLogFile);
                                }

                                fflush(pLogFile);
                                fclose(pLogFile);


                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Created %s with Log Page %d Information\n", fileNameUsed, GET_NVME_LOG_IDENTIFIER);
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
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Error: Could not retrieve Log Page %d\n", GET_TELEMETRY_IDENTIFIER);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    safe_Free(logBuffer);
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Couldn't allocate %" PRIu32 " bytes buffer needed for Log Page %" PRId32 "\n", size, GET_NVME_LOG_IDENTIFIER);
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
        }

        if (NVME_TEMP_STATS_FLAG)
        {
            switch (nvme_Print_Temp_Statistics(&deviceList[deviceIter]))
            {
            case SUCCESS:
                //nothing to print here since if it was successful, the log will be printed to the screen
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A failure occured while trying to get Error Information Log\n");
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
                //nothing to print here since if it was successful, the log will be printed to the screen
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A failure occured while trying to get Error Information Log\n");
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
            switch (transition_Power_State(&deviceList[deviceIter], TRANSITION_POWER_STATE_TO))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully transitioned to power state %d.\n", TRANSITION_POWER_STATE_TO);
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
                    printf("ERROR: Could not transition to the new power state %d\n", TRANSITION_POWER_STATE_TO);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
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


        //this option must come after --transition power so that these two options can be combined on the command line and produce the correct end result
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
            FILE *firmwareFilePtr = NULL;
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
                long firmwareFileSize = get_File_Size(firmwareFilePtr);
                uint8_t *firmwareMem = (uint8_t*)calloc(firmwareFileSize, sizeof(uint8_t));
                if (firmwareMem)
                {
                    supportedDLModes supportedFWDLModes;
                    memset(&supportedFWDLModes, 0, sizeof(supportedDLModes));
                    if (SUCCESS == get_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes))
                    {
                        if (!USER_SET_DOWNLOAD_MODE)
                        {
                            DOWNLOAD_FW_MODE = supportedFWDLModes.recommendedDownloadMode;
                        }
                    }
                    fread(firmwareMem, sizeof(uint8_t), firmwareFileSize, firmwareFilePtr);

                    memset(&dlOptions, 0, sizeof(firmwareUpdateData));
                    memset(&commandTimer, 0, sizeof(seatimer_t));
                    dlOptions.dlMode = DOWNLOAD_FW_MODE;
                    if (FWDL_SEGMENT_SIZE_FROM_USER)
                    {
                        dlOptions.segmentSize = FWDL_SEGMENT_SIZE_FLAG;
                    }
                    else
                    {
                        dlOptions.segmentSize = 0;
                    }
                    dlOptions.firmwareFileMem = firmwareMem;
                    dlOptions.firmwareMemoryLength = firmwareFileSize;
                    dlOptions.firmwareSlot = FIRMWARE_SLOT_FLAG;
                    start_Timer(&commandTimer);
                    ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                    stop_Timer(&commandTimer);
                    switch (ret)
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Firmware Download successful\n");
                            printf("Firmware Download time");
                            print_Time(get_Nano_Seconds(commandTimer));
                            printf("Average time/segment ");
                            print_Time(dlOptions.avgSegmentDlTime);
                            if (DOWNLOAD_FW_MODE != DL_FW_DEFERRED)
                            {
                                printf("Activate Time         ");
                                print_Time(dlOptions.activateFWTime);
                            }
                        }
                        if (DOWNLOAD_FW_MODE == DL_FW_DEFERRED)
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Firmware download complete. Reboot or run the --%s command to finish installing the firmware.\n", ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
                            }
                        }
                        else if (supportedFWDLModes.seagateDeferredPowerCycleActivate && DOWNLOAD_FW_MODE == DL_FW_SEGMENTED)
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("This drive requires a full power cycle to activate the new code.\n");
                            }
                        }
                        else
                        {
                            fill_Drive_Info_Data(&deviceList[deviceIter]);
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                if (NEW_FW_MATCH_FLAG)
                                {
                                    if (strcmp(NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision) == 0)
                                    {
                                        printf("Successfully validated firmware after download!\n");
                                        printf("New firmware version is %s\n", deviceList[deviceIter].drive_info.product_revision);
                                    }
                                    else
                                    {
                                        printf("Unable to verify firmware after download!, expected %s, but found %s\n", NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision);
                                    }
                                }
                                else
                                {
                                    printf("New firmware version is %s\n", deviceList[deviceIter].drive_info.product_revision);
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
                    safe_Free(firmwareMem);
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

        if (ACTIVATE_DEFERRED_FW_FLAG || SWITCH_FW_FLAG)
        {
            supportedDLModes supportedFWDLModes;
            memset(&supportedFWDLModes, 0, sizeof(supportedDLModes));
            get_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes);
            if (supportedFWDLModes.deferred || supportedFWDLModes.scsiInfoPossiblyIncomplete)
            {
                memset(&dlOptions, 0, sizeof(firmwareUpdateData));
                memset(&commandTimer, 0, sizeof(seatimer_t));
                dlOptions.dlMode = DL_FW_ACTIVATE;
                dlOptions.segmentSize = 0;
                dlOptions.firmwareFileMem = NULL;
                dlOptions.firmwareMemoryLength = 0;
                dlOptions.firmwareSlot = FIRMWARE_SLOT_FLAG;
                if (SWITCH_FW_FLAG)
                {
                    dlOptions.existingFirmwareImage = true;
                }
                start_Timer(&commandTimer);
                ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                stop_Timer(&commandTimer);
                switch (ret)
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Firmware activation successful\n");
                        fill_Drive_Info_Data(&deviceList[deviceIter]);
                        if (NEW_FW_MATCH_FLAG)
                        {
                            if (strcmp(NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision) == 0)
                            {
                                printf("Successfully validated firmware after download!\n");
                                printf("New firmware version is %s\n", deviceList[deviceIter].drive_info.product_revision);
                            }
                            else
                            {
                                printf("Unable to verify firmware after download!, expected %s, but found %s\n", NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision);
                            }
                        }
                        else
                        {
                            printf("New firmware version is %s\n", deviceList[deviceIter].drive_info.product_revision);
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
                memset(&nvmformatParameters, 0, sizeof(runNVMFormatParameters));
                if (NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM >= 16 && NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM <= 512)
                {
                    nvmformatParameters.formatNumberProvided = false;
                    nvmformatParameters.newSize.currentBlockSize = true;
                }
                else if (NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM >= 0 && NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM < 16)
                {
                    nvmformatParameters.formatNumberProvided = true;
                    nvmformatParameters.formatNumber = NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM;
                }
                else
                {
                    nvmformatParameters.formatNumberProvided = false;
                    nvmformatParameters.newSize.currentBlockSize = false;
                    nvmformatParameters.newSize.newBlockSize = NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM;
                }
                if (NVM_FORMAT_METADATA_SIZE != UINT32_MAX && !nvmformatParameters.formatNumberProvided)
                {
                    nvmformatParameters.newSize.changeMetadataSize = true;
                    nvmformatParameters.newSize.metadataSize = (uint16_t)NVM_FORMAT_METADATA_SIZE;
                }
                if (NVM_FORMAT_NSID != UINT32_MAX)
                {
                    nvmformatParameters.currentNamespace = true;
                }
                nvmformatParameters.secureEraseSettings = NVM_FORMAT_SECURE_ERASE;
                //PI
                switch (NVM_FORMAT_PI_TYPE)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                    nvmformatParameters.changeProtectionType = true;
                    nvmformatParameters.protectionType = NVM_FORMAT_PI_TYPE;
                    break;
                default:
                    break;
                }
                //PIL
                switch (NVM_FORMAT_PI_LOCATION)
                {
                case 0:
                    nvmformatParameters.protectionLocation.valid = true;
                    nvmformatParameters.protectionLocation.first8Bytes = true;
                    break;
                case 1:
                    nvmformatParameters.protectionLocation.valid = true;
                    nvmformatParameters.protectionLocation.first8Bytes = false;
                    break;
                default:
                    break;
                }
                //metadata settings
                switch (NVM_FORMAT_METADATA_SETTING)
                {
                case 0:
                    nvmformatParameters.metadataSettings.valid = true;
                    nvmformatParameters.metadataSettings.metadataAsExtendedLBA = true;
                    break;
                case 1:
                    nvmformatParameters.metadataSettings.valid = true;
                    nvmformatParameters.metadataSettings.metadataAsExtendedLBA = false;
                    break;
                default:
                    break;
                }
                int formatRet = run_NVMe_Format(&deviceList[deviceIter], nvmformatParameters, POLL_FLAG);
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
                    printf("e.g.: %s -d %s --%s current --confirm %s\n\n", util_name, deviceHandleExample, NVM_FORMAT_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (PROGRESS_CHAR != NULL)
        {
            int result = UNKNOWN;
            //first take whatever was entered in progressTest and convert it to uppercase to do fewer string comparisons
            convert_String_To_Upper_Case(progressTest);
            //do some string comparisons to figure out what we are checking for progress on
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
        //At this point, close the device handle since it is no longer needed. Do not put any further IO below this.
        close_Device(&deviceList[deviceIter]);
    }
    safe_Free(DEVICE_LIST);
#endif //DISABLE_NVME_PASSTHROUGH
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
    printf("\t%s --scan\n", util_name);
    printf("\t%s -d %s -i\n", util_name, deviceHandleExample);
    //return codes
    printf("Return codes\n");
    printf("============\n");
    print_SeaChest_Util_Exit_Codes(0, NULL, util_name);

    //utility options
    printf("\nUtility Options\n");
    printf("===============\n");
    print_Echo_Command_Line_Help(shortUsage);
    print_Help_Help(shortUsage);
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_New_Firmware_Revision_Match_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    //the test options
    printf("Utility Arguments\n");
    printf("=================\n");
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Device_Information_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here
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
    print_Firmware_Revision_Match_Help(shortUsage);
    print_pcierr_Help(shortUsage);
    print_extSmatLog_Help(shortUsage);
    print_Output_Mode_Help(shortUsage);
    print_NVMe_Temp_Stats_Help(shortUsage);
    print_NVMe_Pci_Stats_Help(shortUsage);
    print_Show_Supported_Formats_Help(shortUsage);

    //data destructive commands
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //utility data destructive tests/operations go here
    print_NVM_Format_Metadata_Setting_Help(shortUsage);
    print_NVM_Format_Metadata_Size_Help(shortUsage);
    print_NVM_Format_NSID_Help(shortUsage);
    print_NVM_Format_PI_Type_Help(shortUsage);
    print_NVM_Format_PIL_Help(shortUsage);
    print_NVM_Format_Secure_Erase_Help(shortUsage);
    print_NVM_Format_Help(shortUsage);
}
