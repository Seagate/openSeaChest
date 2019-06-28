//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2018 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
//
// \file openSeaChest_SMART.c command line that performs various SMART methods on a device.

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
#include "common.h"
#include "EULA.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "smart.h"
#include "dst.h"
#include "drive_info.h"
#include "seagate_operations.h"
#include "defect.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_SMART";
const char *buildVersion = "1.12.2";

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
    /////////////////
    //  Variables  //
    /////////////////
    //common utility variables
    int                 ret = SUCCESS;
    eUtilExitCodes      exitCode = UTIL_EXIT_NO_ERROR;
    DEVICE_UTIL_VARS
    DEVICE_INFO_VAR
    SAT_INFO_VAR
    LICENSE_VAR
    ECHO_COMMAND_LINE_VAR
    SCAN_FLAG_VAR
    AGRESSIVE_SCAN_FLAG_VAR
    SHOW_BANNER_VAR
    SHOW_HELP_VAR
    TEST_UNIT_READY_VAR
    SAT_12_BYTE_CDBS_VAR
    MODEL_MATCH_VARS
    FW_MATCH_VARS
    CHILD_MODEL_MATCH_VARS
    CHILD_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    FORCE_DRIVE_TYPE_VARS
    ENABLE_LEGACY_PASSTHROUGH_VAR
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    //optional common util options
    POLL_VAR
    PROGRESS_VAR
    LONG_TEST_VAR
    SINGLE_SECTOR_DATA_ERASE_VAR
    //tool specific variables
    SMART_CHECK_VAR
    SMART_ATTRIBUTES_VARS
    SHORT_DST_VAR
    LONG_DST_VAR
    ABORT_DST_VAR
    IGNORE_OPERATION_TIMEOUT_VAR
    CAPTIVE_FOREGROUND_VAR
    ABORT_IDD_VAR
    IDD_TEST_VARS
    DST_AND_CLEAN_VAR
    SMART_FEATURE_VARS
    SMART_ATTR_AUTOSAVE_FEATURE_VARS
    SMART_INFO_VAR
    SMART_AUTO_OFFLINE_FEATURE_VARS
    SHOW_DST_LOG_VAR
    CONVEYANCE_DST_VAR
    SET_MRIE_MODE_VARS
    ERROR_LIMIT_VAR
    SCSI_DEFECTS_VARS
    SHOW_SMART_ERROR_LOG_VARS
    SMART_ERROR_LOG_FORMAT_VAR
#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif

    int8_t args = 0;
    uint8_t argIndex = 0;
    int32_t optionIndex = 0;

    //add -- options to this structure DO NOT ADD OPTIONAL ARGUMENTS! Optional arguments are a GNU extension and are not supported in Unix or some compilers- TJE
    struct option longopts[] = {
        //common command line options
        DEVICE_LONG_OPT,
        HELP_LONG_OPT,
        DEVICE_INFO_LONG_OPT,
        SAT_INFO_LONG_OPT,
        USB_CHILD_INFO_LONG_OPT,
        SCAN_LONG_OPT,
        AGRESSIVE_SCAN_LONG_OPT,
        SCAN_FLAGS_LONG_OPT,
        VERSION_LONG_OPT,
        VERBOSE_LONG_OPT,
        QUIET_LONG_OPT,
        LICENSE_LONG_OPT,
        ECHO_COMMAND_LIN_LONG_OPT,
        TEST_UNIT_READY_LONG_OPT,
        SAT_12_BYTE_CDBS_LONG_OPT,
        ONLY_SEAGATE_LONG_OPT,
        MODEL_MATCH_LONG_OPT,
        FW_MATCH_LONG_OPT,
        CHILD_MODEL_MATCH_LONG_OPT,
        CHILD_FW_MATCH_LONG_OPT,
        POLL_LONG_OPT,
        PROGRESS_LONG_OPT,
        CONFIRM_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
        //tool specific options go here
        SMART_CHECK_LONG_OPT,
        SMART_ATTRIBUTES_LONG_OPT,
        SHORT_DST_LONG_OPT,
        LONG_DST_LONG_OPT,
        ABORT_DST_LONG_OPT,
        IGNORE_OPERATION_TIMEOUT_LONG_OPT,
        CAPTIVE_FOREGROUND_LONG_OPTS,
        ABORT_IDD_LONG_OPT,
        IDD_TEST_LONG_OPT,
        ERROR_LIMIT_LONG_OPT,
        DST_AND_CLEAN_LONG_OPT,
        SMART_FEATURE_LONG_OPT,
        SMART_ATTR_AUTOSAVE_FEATURE_LONG_OPT,
        SMART_INFO_LONG_OPT,
        SMART_AUTO_OFFLINE_FEATURE_LONG_OPT,
        SHOW_DST_LOG_LONG_OPT,
        CONVEYANCE_DST_LONG_OPT,
        SET_MRIE_MODE_LONG_OPT,
        SCSI_DEFECTS_LONG_OPTS,
        SHOW_SMART_ERROR_LOG_LONG_OPT,
        SMART_ERROR_LOG_FORMAT_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        LONG_OPT_TERMINATOR
    };

    eVerbosityLevels toolVerbosity = VERBOSITY_DEFAULT;

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
        args = getopt_long(argc, argv, "d:hisSF:Vv:qr:R:E:e:w:%:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        //printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:
            //parse long options that have no short option and required arguments here
            if (strncmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CONFIRM_LONG_OPT_STRING))) == 0)
            {
                if (strlen(optarg) == strlen(LONG_TEST_ACCEPT_STRING) && strncmp(optarg, LONG_TEST_ACCEPT_STRING, strlen(LONG_TEST_ACCEPT_STRING)) == 0)
                {
                    LONG_TEST_FLAG = true;
                }
                else if (strlen(optarg) == strlen(SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING, strlen(SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    SINGLE_SECTOR_DATA_ERASE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, IDD_TEST_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(IDD_TEST_LONG_OPT_STRING))) == 0)
            {
                RUN_IDD_FLAG = true;
                if (strcmp(optarg, "short") == 0)
                {
                    IDD_TEST_FLAG = SEAGATE_IDD_SHORT;
                }
                else if (strcmp(optarg, "long") == 0)
                {
                    IDD_TEST_FLAG = SEAGATE_IDD_LONG;
                }
                else
                {
                    //read in the argument as a hex value instead of an integer
                    uint32_t iddTestNumber = 0;
                    sscanf(optarg, "%"SCNx32, &iddTestNumber);
                    switch (iddTestNumber)
                    {
                    case 0x70:
                        IDD_TEST_FLAG = SEAGATE_IDD_SHORT;
                        break;
                    case 0x71:
                        IDD_TEST_FLAG = SEAGATE_IDD_LONG;
                        break;
                    default:
                        print_Error_In_Cmd_Line_Args(IDD_TEST_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, ERROR_LIMIT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(ERROR_LIMIT_LONG_OPT_STRING))) == 0)
            {
                ERROR_LIMIT_FLAG = (uint16_t)atoi(optarg);
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
            else if (strncmp(longopts[optionIndex].name, SMART_FEATURE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SMART_FEATURE_LONG_OPT_STRING))) == 0)
            {
                SMART_FEATURE_FLAG = true;
                if(strcmp(optarg, "enable") == 0)
                {
                    SMART_FEATURE_STATE_FLAG = true;
                }
                else if(strcmp(optarg, "disable") == 0)
                {
                    SMART_FEATURE_STATE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SMART_FEATURE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SET_MRIE_MODE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_MRIE_MODE_LONG_OPT_STRING))) == 0)
            {
                SET_MRIE_MODE_FLAG = true;
                if(strcmp(optarg, "default") == 0)
                {
                    SET_MRIE_MODE_DEFAULT = true;
                }
                else
                {
                    SET_MRIE_MODE_VALUE = (uint8_t)atoi(optarg);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SMART_ATTR_AUTOSAVE_FEATURE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SMART_ATTR_AUTOSAVE_FEATURE_LONG_OPT_STRING))) == 0)
            {
                SMART_ATTR_AUTOSAVE_FEATURE_FLAG = true;
                if(strcmp(optarg, "enable") == 0)
                {
                    SMART_ATTR_AUTOSAVE_FEATURE_STATE_FLAG = true;
                }
                else if(strcmp(optarg, "disable") == 0)
                {
                    SMART_ATTR_AUTOSAVE_FEATURE_STATE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SMART_ATTR_AUTOSAVE_FEATURE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SMART_AUTO_OFFLINE_FEATURE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SMART_AUTO_OFFLINE_FEATURE_LONG_OPT_STRING))) == 0)
            {
                SMART_AUTO_OFFLINE_FEATURE_FLAG = true;
                if (strcmp(optarg, "enable") == 0)
                {
                    SMART_AUTO_OFFLINE_FEATURE_STATE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SMART_AUTO_OFFLINE_FEATURE_STATE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SMART_AUTO_OFFLINE_FEATURE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SCSI_DEFECTS_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SCSI_DEFECTS_LONG_OPT_STRING))) == 0)
            {
                uint8_t counter = 0;
                SCSI_DEFECTS_FLAG = true;
                while (counter < strlen(optarg))
                {
                    if (optarg[counter] == 'p' || optarg[counter] == 'P')
                    {
                        SCSI_DEFECTS_PRIMARY_LIST = true;
                    }
                    else if (optarg[counter] == 'g' || optarg[counter] == 'G')
                    {
                        SCSI_DEFECTS_GROWN_LIST = true;
                    }
                    ++counter;
                }
                if (!SCSI_DEFECTS_PRIMARY_LIST && !SCSI_DEFECTS_GROWN_LIST)
                {
                    printf("\n Error in option --%s. You must specify showing primary (p) or grown (g) defects or both\n", SCSI_DEFECTS_LONG_OPT_STRING);
                    printf("Use -h option to view command line help\n");
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING))) == 0)
            {
                //check for integer value or string that specifies the correct mode.
                if (strlen(optarg) == 1 && isdigit(optarg[0]))
                {
                    SCSI_DEFECTS_DESCRIPTOR_MODE = atoi(optarg);
                }
                else
                {
                    if (strcmp("shortBlock", optarg) == 0)
                    {
                        SCSI_DEFECTS_DESCRIPTOR_MODE = 0;
                    }
                    else if (strcmp("longBlock", optarg) == 0)
                    {
                        SCSI_DEFECTS_DESCRIPTOR_MODE = 3;
                    }
                    else if (strcmp("xbfi", optarg) == 0)
                    {
                        SCSI_DEFECTS_DESCRIPTOR_MODE = 1;
                    }
                    else if (strcmp("xchs", optarg) == 0)
                    {
                        SCSI_DEFECTS_DESCRIPTOR_MODE = 2;
                    }
                    else if (strcmp("bfi", optarg) == 0)
                    {
                        SCSI_DEFECTS_DESCRIPTOR_MODE = 4;
                    }
                    else if (strcmp("chs", optarg) == 0)
                    {
                        SCSI_DEFECTS_DESCRIPTOR_MODE = 5;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, SHOW_SMART_ERROR_LOG_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SHOW_SMART_ERROR_LOG_LONG_OPT_STRING))) == 0)
            {
                SHOW_SMART_ERROR_LOG_FLAG = true;
                if (strcmp("summary", optarg) == 0)
                {
                    SHOW_SMART_ERROR_LOG_MODE = 0;
                }
                else if (strcmp("comprehensive", optarg) == 0 || strcmp("extComprehensive", optarg) == 0)
                {
                    SHOW_SMART_ERROR_LOG_MODE = 1;
                }
                else
                {
                    SHOW_SMART_ERROR_LOG_FLAG = false;
                    print_Error_In_Cmd_Line_Args(SHOW_SMART_ERROR_LOG_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SMART_ERROR_LOG_FORMAT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SMART_ERROR_LOG_FORMAT_LONG_OPT_STRING))) == 0)
            {
                if (strcmp("raw", optarg) == 0 || strcmp("generic", optarg) == 0)
                {
                    SMART_ERROR_LOG_FORMAT_FLAG = true;
                }
                else if (strcmp("detailed", optarg) == 0)
                {
                    SMART_ERROR_LOG_FORMAT_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SMART_ERROR_LOG_FORMAT_LONG_OPT_STRING, optarg);
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
            else if (strncmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_MODEL_MATCH_LONG_OPT_STRING))) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                strncpy(CHILD_MODEL_STRING_FLAG, optarg, M_Min(40, strlen(optarg)));
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                CHILD_FW_MATCH_FLAG = true;
                strncpy(CHILD_FW_STRING_FLAG, optarg, M_Min(9, strlen(optarg)));
            }
            break;
        case ':'://missing required argument
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            switch (optopt)
            {
            case 0:
                //check long options for missing arguments
                if (strncmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CONFIRM_LONG_OPT_STRING))) == 0)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("You must add a confirmation string to the confirm option\n");
                    }
                }
                else if (strncmp(longopts[optionIndex].name, IDD_TEST_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(IDD_TEST_LONG_OPT_STRING))) == 0)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("You must add a a test type to run for the idd option. Valid tests are short or long\n");
                    }
                }
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
            case PROGRESS_SHORT_OPT:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must specify a test to get progress for.\n");
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
        case PROGRESS_SHORT_OPT://progress [test]
            PROGRESS_CHAR = strdup(optarg);
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

    if (SCAN_FLAG || AGRESSIVE_SCAN_FLAG)
    {
        unsigned int scanControl = DEFAULT_SCAN;
        if (AGRESSIVE_SCAN_FLAG)
        {
            scanControl |= AGRESSIVE_SCAN;
        }
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
        if (SAT_12_BYTE_CDBS_FLAG)
        {
            scanControl |= SAT_12_BYTE;
        }
#if defined (ENABLE_CSMI)
        if (scanIgnoreCSMI)
        {
            scanControl |= IGNORE_CSMI;
        }
        if (scanAllowDuplicateDevices)
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
    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        uint64_t flags = 0;
#if defined (ENABLE_CSMI)
        flags |= GET_DEVICE_FUNCS_IGNORE_CSMI;
#endif
        if (SUCCESS != get_Device_Count(&DEVICE_LIST_COUNT, flags))
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
        || SMART_ATTRIBUTES_FLAG
        || SHORT_DST_FLAG
        || LONG_DST_FLAG
        || ABORT_DST_FLAG
        || ABORT_IDD_FLAG
        || (PROGRESS_CHAR != NULL)
        || RUN_IDD_FLAG
        || DST_AND_CLEAN_FLAG
        || SMART_FEATURE_FLAG
        || SMART_ATTR_AUTOSAVE_FEATURE_FLAG
        || SMART_INFO_FLAG
        || SMART_AUTO_OFFLINE_FEATURE_FLAG
        || SHOW_DST_LOG_FLAG
        || CONVEYANCE_DST_FLAG
        || SET_MRIE_MODE_FLAG
        || SCSI_DEFECTS_FLAG
        || SHOW_SMART_ERROR_LOG_FLAG
        //check for other tool specific options here
        ))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    uint64_t flags = 0;
    DEVICE_LIST = (tDevice*)calloc(DEVICE_LIST_COUNT * sizeof(tDevice), sizeof(tDevice));
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

#if defined (ENABLE_CSMI)
    if (CSMI_VERBOSE_FLAG)
    {
        flags |= CSMI_FLAG_VERBOSE;//This may be temporary....or it may last longer, but for now this highest bit is what we'll set for this option...
    }
    if (CSMI_FORCE_USE_PORT_FLAG)
    {
        flags |= CSMI_FLAG_USE_PORT;
    }
    if (CSMI_FORCE_IGNORE_PORT_FLAG)
    {
        flags |= CSMI_FLAG_IGNORE_PORT;
    }
#endif

    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        //TODO? check for this flag ENABLE_LEGACY_PASSTHROUGH_FLAG. Not sure it is needed here and may not be desirable.
#if defined (ENABLE_CSMI)
        flags |= GET_DEVICE_FUNCS_IGNORE_CSMI;//TODO: Remove this flag so that CSMI devices can be part of running on all drives. This is not allowed now because of issues with running the same operation on the same drive with both PD? and SCSI?:? handles.
#endif
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
#if !defined(_WIN32)
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
                /*if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) is not a Seagate drive.\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification);
                }*/
                continue;
            }
        }

        if (SAT_12_BYTE_CDBS_FLAG)
        {
            //set SAT12 for this device if requested
            deviceList[deviceIter].drive_info.ata_Options.use12ByteSATCDBs = true;
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
            printf("\n%s - %s - %s - %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification, deviceList[deviceIter].drive_info.serialNumber, print_drive_type(&deviceList[deviceIter]));
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

        if (SMART_INFO_FLAG)
        {
            smartFeatureInfo smartData;
            memset(&smartData, 0, sizeof(smartFeatureInfo));
            switch (get_SMART_Info(&deviceList[deviceIter], &smartData))
            {
            case SUCCESS:
                print_SMART_Info(&deviceList[deviceIter], &smartData);
                break;
            case WARN_INVALID_CHECKSUM:
                printf("Error: Device returned SMART data with an invalid checksum\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case NOT_SUPPORTED:
                printf("SMART Information is not supported on this device\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                printf("Failed to get SMART Information from device\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_DEFECTS_FLAG)
        {
            ptrSCSIDefectList defects = NULL;
            switch (get_SCSI_Defect_List(&deviceList[deviceIter], SCSI_DEFECTS_DESCRIPTOR_MODE, SCSI_DEFECTS_GROWN_LIST, SCSI_DEFECTS_PRIMARY_LIST, &defects))
            {
            case SUCCESS:
                print_SCSI_Defect_List(defects);
                free_Defect_List(&defects);
                break;
            case NOT_SUPPORTED:
                printf("Reading Defects not supported on this device or unsupported defect list format was given.\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                printf("Failed to retrieve SCSI defect list from this device\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (SMART_CHECK_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("SMART Check\n");
            }
            smartTripInfo tripInfo;
            memset(&tripInfo, 0, sizeof(smartTripInfo));
            ret = run_SMART_Check(&deviceList[deviceIter], &tripInfo);
            if (FAILURE == ret)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SMART has been tripped!\n");
                    if (tripInfo.reasonStringLength > 0)
                    {
                        printf("\t%s\n", tripInfo.reasonString);
                    }
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
                    printf("A failure occured while trying to get SMART attributes\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHOW_SMART_ERROR_LOG_FLAG)
        {
            switch (SHOW_SMART_ERROR_LOG_MODE)
            {
            case 0://summary
            {
                summarySMARTErrorLog sumErrorLog;
                memset(&sumErrorLog, 0, sizeof(summarySMARTErrorLog));
                switch (get_ATA_Summary_SMART_Error_Log(&deviceList[deviceIter], &sumErrorLog))
                {
                case SUCCESS:
                    print_ATA_Summary_SMART_Error_Log(&sumErrorLog, SMART_ERROR_LOG_FORMAT_FLAG);
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("SMART Summary Error log is not supported on this device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to read the SMART Summary Error log!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            break;
            case 1://(ext) comprehensive
            {
                comprehensiveSMARTErrorLog compErrorLog;
                memset(&compErrorLog, 0, sizeof(comprehensiveSMARTErrorLog));
                switch (get_ATA_Comprehensive_SMART_Error_Log(&deviceList[deviceIter], &compErrorLog, false /*force reading SMART comprehensive log is turned off right now*/))
                {
                case SUCCESS:
                    print_ATA_Comprehensive_SMART_Error_Log(&compErrorLog, SMART_ERROR_LOG_FORMAT_FLAG);
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("SMART (Ext) Comprehensive Error log is not supported on this device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to read the SMART (Ext) Comprehensive Error log!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
                break;
            default://error
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unknown SMART Error Log specified!\n");
                }
                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
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

        if (ABORT_IDD_FLAG)
        {
            int abortResult = UNKNOWN;
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Aborting IDD\n");
            }
            abortResult = abort_DST(&deviceList[deviceIter]);//calls into the same code to do the abort - TJE
            switch (abortResult)
            {
            case UNKNOWN:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unknown Error occurred while trying to abort IDD\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case SUCCESS:
            case ABORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully aborted IDD.\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Aborting IDD is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Abort IDD Failed!\n");
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
            DSTResult = run_DST(&deviceList[deviceIter], DST_TYPE_SHORT, POLL_FLAG, CAPTIVE_FOREGROUND_FLAG, IGNORE_OPERATION_TIMEOUT);
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
                    if (POLL_FLAG || CAPTIVE_FOREGROUND_FLAG)
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

        if (CONVEYANCE_DST_FLAG)
        {
            int32_t DSTResult = UNKNOWN;
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Conveyance DST\n");
            }
            DSTResult = run_DST(&deviceList[deviceIter], DST_TYPE_CONVEYENCE, POLL_FLAG, CAPTIVE_FOREGROUND_FLAG, IGNORE_OPERATION_TIMEOUT);
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
                    if (POLL_FLAG || CAPTIVE_FOREGROUND_FLAG)
                    {
                        printf("Conveyance DST Passed!\n");
                    }
                    else
                    {
                        printf("Conveyance DST started!\n");
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
                    printf("Conveyance DST was aborted! You can add the --%s flag to allow DST to continue\n", IGNORE_OPERATION_TIMEOUT_LONG_OPT_STRING);
                    printf("running despite taking longer than expected.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Conveyance DST is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Conveyance DST Failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (LONG_DST_FLAG)
        {
            int32_t DSTResult = UNKNOWN;
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Long DST\n");
                uint8_t hours = 0;
                uint8_t minutes = 0;
                if (SUCCESS == get_Long_DST_Time(&deviceList[deviceIter], &hours, &minutes))
                {
                    printf("Drive reported long DST time as ");
                    if (hours > 0)
                    {
                        printf("%"PRIu8" hour", hours);
                        if (hours > 1)
                        {
                            printf("s ");
                        }
                        else
                        {
                            printf(" ");
                        }
                    }
                    printf("%"PRIu8" minute", minutes);
                    if (minutes > 1)
                    {
                        printf("s");
                    }
                    printf("\n");
                }
                else
                {
                    printf("Drive does not report how long this test will take.\n\n");
                }
            }
            if (!LONG_TEST_FLAG)
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must add the flag:\n\"%s\" \n", LONG_TEST_ACCEPT_STRING);
                    printf("to the command line arguments to run the long DST.\n\n");
                    printf("e.g.: %s -d %s --longDST --confirm %s\n\n", util_name, deviceHandleExample, LONG_TEST_ACCEPT_STRING);
                }
            }
            else
            {
                DSTResult = run_DST(&deviceList[deviceIter], DST_TYPE_LONG, POLL_FLAG, CAPTIVE_FOREGROUND_FLAG, IGNORE_OPERATION_TIMEOUT);
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
                        if (POLL_FLAG || CAPTIVE_FOREGROUND_FLAG)
                        {
                            printf("Long DST Passed!\n");
                        }
                        else
                        {
                            printf("Long DST started!\n");
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
                        printf("Long DST was aborted! You can add the --%s flag to allow DST to continue\n", IGNORE_OPERATION_TIMEOUT_LONG_OPT_STRING);
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
                        printf("Long DST Failed!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (RUN_IDD_FLAG)
        {
            int32_t IDDResult = UNKNOWN;
            if (is_Seagate_Family(&deviceList[deviceIter]) == SEAGATE)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    uint64_t iddTimeSeconds = 0;
                    uint8_t hours = 0, minutes = 0, seconds = 0;
                    get_Approximate_IDD_Time(&deviceList[deviceIter], IDD_TEST_FLAG, &iddTimeSeconds);
                    if (iddTimeSeconds == UINT64_MAX)
                    {
                        printf("A time estimate is not available for this IDD operation");
                    }
                    else
                    {
                        printf("The In Drive Diagnostics (IDD) test will take approximately ");
                        convert_Seconds_To_Displayable_Time(iddTimeSeconds, NULL, NULL, &hours, &minutes, &seconds);
                        print_Time_To_Screen(NULL, NULL, &hours, &minutes, &seconds);
                    }
                    printf("\n");
                }
                IDDResult = run_IDD(&deviceList[deviceIter], IDD_TEST_FLAG, POLL_FLAG, CAPTIVE_FOREGROUND_FLAG);
                switch (IDDResult)
                {
                case UNKNOWN:
                    //IDD was not run
                    break;
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (POLL_FLAG || IDD_TEST_FLAG == SEAGATE_IDD_SHORT || CAPTIVE_FOREGROUND_FLAG)//short test is run in captive mode, so polling doesn't make sense
                        {
                            printf("IDD - ");
                            switch(IDD_TEST_FLAG)
                            {
                            case SEAGATE_IDD_SHORT:
                                printf("short");
                                break;
                            case SEAGATE_IDD_LONG:
                                printf("long");
                                break;
                            default:
                                printf("unknown");
                                break;
                            }
                            printf(" - completed without error!\n");
                        }
                        else
                        {
                            printf("IDD - ");
                            switch(IDD_TEST_FLAG)
                            {
                            case SEAGATE_IDD_SHORT:
                                printf("short");
                                break;
                            case SEAGATE_IDD_LONG:
                                printf("long");
                                break;
                            default:
                                printf("unknown");
                                break;
                            }
                            printf(" - has been started.\n");
                            printf("use --progress idd -d %s to monitor IDD progress\n", deviceHandleExample);
                            printf("use --abortIDD -d %s to stop IDD\n", deviceHandleExample);
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
                        printf("IDD aborted!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_ABORTED;
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && CAPTIVE_FOREGROUND_FLAG)
                        {
                            printf("Captive/foreground mode not supported on this IDD test on this drive.\n");
                        }
                        else
                        {
                            printf("IDD not supported\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("IDD Failed!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("IDD is only supported on Seagate Drives.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (DST_AND_CLEAN_FLAG)
        {
            if (SINGLE_SECTOR_DATA_ERASE_FLAG)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DST And Clean\n");
                }
                switch (run_DST_And_Clean(&deviceList[deviceIter], ERROR_LIMIT_FLAG, NULL, NULL, NULL, NULL))
                {
                case UNKNOWN:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Unknown Error occurred while trying to start DST and Clean\n");
                    }
                    break;
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DST and Clean completed successfully\n");
                    }
                    break;
                case IN_PROGRESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("A self test is currently in progress. Please wait for it to finish before starting DST and Clean\n");
                    }
                    break;
                case ABORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DST And Clean was aborted!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_ABORTED;
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DST and Clean is not supported on this device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DST and Clean Failed!\n");
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
                    printf("You must add the flag:\n\"%s\" \n", SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to run a dst and clean operation.\n\n");
                    printf("e.g.: %s -d %s --%s --confirm %s\n\n", util_name, deviceHandleExample, DST_AND_CLEAN_LONG_OPT_STRING, SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (SHOW_DST_LOG_FLAG)
        {
            dstLogEntries dstEntries;
            memset(&dstEntries, 0, sizeof(dstLogEntries));
            switch (get_DST_Log_Entries(&deviceList[deviceIter], &dstEntries))
            {
            case SUCCESS:
                print_DST_Log_Entries(&dstEntries);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Showing DST Log not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get DST log!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if(SMART_FEATURE_FLAG)
        {
            switch (enable_Disable_SMART_Feature(&deviceList[deviceIter], SMART_FEATURE_STATE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully ");
                    if(SMART_FEATURE_STATE_FLAG)
                    {
                        printf("Enabled");
                    }
                    else
                    {
                        printf("Disabled");
                    }
                    printf(" SMART feature on this device\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Configuring SMART feature is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A failure occurred while trying to configure SMART feature\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_MRIE_MODE_FLAG)
        {
            switch (set_MRIE_Mode(&deviceList[deviceIter], SET_MRIE_MODE_VALUE, SET_MRIE_MODE_DEFAULT))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set MRIE mode to %" PRIu8"\n", SET_MRIE_MODE_VALUE);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Changing MRIE mode not supported or attempted to change to an unsupported mode\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to change MRIE mode.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if(SMART_ATTR_AUTOSAVE_FEATURE_FLAG)
        {
            switch (enable_Disable_SMART_Attribute_Autosave(&deviceList[deviceIter], SMART_ATTR_AUTOSAVE_FEATURE_STATE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully ");
                    if(SMART_ATTR_AUTOSAVE_FEATURE_STATE_FLAG)
                    {
                        printf("Enabled");
                    }
                    else
                    {
                        printf("Disabled");
                    }
                    printf(" SMART attribute auto-save on this device\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Configuring SMART attribute auto-save is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A failure occurred while trying to configure SMART attribute auto-save\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SMART_AUTO_OFFLINE_FEATURE_FLAG)
        {
            switch (enable_Disable_SMART_Auto_Offline(&deviceList[deviceIter], SMART_AUTO_OFFLINE_FEATURE_STATE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully ");
                    if (SMART_AUTO_OFFLINE_FEATURE_STATE_FLAG)
                    {
                        printf("Enabled");
                    }
                    else
                    {
                        printf("Disabled");
                    }
                    printf(" SMART auto-off-line on this device\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Configuring SMART auto-off-line is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("A failure occurred while trying to configure SMART auto-off-line\n");
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
            else if (strcmp(progressTest, "IDD") == 0)
            {
                uint8_t iddStatus = 0;
                char iddStatusString[MAX_DST_STATUS_STRING_LENGTH + 1] = { 0 };
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting IDD progress.\n");
                }
                result = get_IDD_Status(&deviceList[deviceIter], &iddStatus);
                translate_DST_Status_To_String(iddStatus, iddStatusString, false, false);
                printf("%s\n", iddStatusString);
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
    }
    return exitCode;
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
    printf("\nReturn codes\n");
    printf("============\n");
    print_SeaChest_Util_Exit_Codes(0, NULL, util_name);

    //utility options - alphabetized
    printf("Utility Options\n");
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
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_SAT_12_Byte_CDB_Help(shortUsage);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    //the test options
    printf("\nUtility Arguments\n");
    printf("=================\n");
    //Common (across utilities) - alphabetized
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Poll_Help(shortUsage);
    print_Progress_Help(shortUsage, "dst, idd");
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    print_Abort_DST_Help(shortUsage);
    print_Abort_IDD_Help(shortUsage);
    print_Captive_Foreground_Help(shortUsage);
    print_Conveyance_DST_Help(shortUsage);
    print_Error_Limit_Help(shortUsage);
    print_IDD_Help(shortUsage);
    print_Long_DST_Help(shortUsage, commandWindowType);
    print_Short_DST_Help(shortUsage);
    print_Show_DST_Log_Help(shortUsage);
    print_SMART_Check_Help(shortUsage);
    print_SMART_Feature_Help(shortUsage);
    //SATA Only
    printf("\n\tSATA Only:\n\t=========\n");
    print_SMART_Attributes_Help(shortUsage);
    print_SMART_Attribute_Autosave_Help(shortUsage);
    print_SMART_Auto_Offline_Help(shortUsage);
    print_Show_SMART_Error_Log_Help(shortUsage);
    print_SMART_Error_Log_Format_Help(shortUsage);
    print_SMART_Info_Help(shortUsage);

    //SAS Only
    printf("\n\tSAS Only:\n\t=========\n");
    print_SCSI_Defects_Format_Help(shortUsage);
    print_Set_MRIE_Help(shortUsage);
    print_SCSI_Defects_Help(shortUsage);

    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    print_DST_And_Clean_Help(shortUsage);
}
