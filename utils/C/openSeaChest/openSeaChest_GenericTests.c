//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2020 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
//
// \file openSeaChest_GenericTests.c Binary command line that performs various generic tests on a device.

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
#include "generic_tests.h"
#include "drive_info.h"
#include "buffer_test.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_GenericTests";
const char *buildVersion = "1.10.0";

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
    MODEL_MATCH_VARS
    FW_MATCH_VARS
    CHILD_MODEL_MATCH_VARS
    CHILD_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    FORCE_DRIVE_TYPE_VARS
    ENABLE_LEGACY_PASSTHROUGH_VAR
    MAX_LBA_VARS
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    DISPLAY_LBA_VAR
    //tool specific flags
    GENERIC_TEST_MODE_VAR
    GENERIC_TEST_MODE_FLAG = RWV_COMMAND_READ; //setting this to the enum value to default to read mode...
    SHORT_GENERIC_VAR
    TWO_MINUTE_TEST_VAR
    LONG_GENERIC_VAR
    USER_GENERIC_START_VAR
    USER_GENERIC_RANGE_VAR
    RUN_USER_GENERIC_TEST_VAR
    ERROR_LIMIT_VAR
    RANDOM_READ_TEST_VAR
    BUTTEFFLY_READ_TEST_VAR
    STOP_ON_ERROR_VAR
    REPAIR_AT_END_VAR
    REPAIR_ON_FLY_VAR
    //test time flags
    HOURS_TIME_VAR
    MINUTES_TIME_VAR
    SECONDS_TIME_VAR
    //data erase
    DATA_ERASE_VAR
    OD_MD_ID_TEST_VARS
    HIDE_LBA_COUNTER_VAR
    BUFFER_TEST_VAR
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
        ONLY_SEAGATE_LONG_OPT,
        MODEL_MATCH_LONG_OPT,
        FW_MATCH_LONG_OPT,
        CHILD_MODEL_MATCH_LONG_OPT,
        CHILD_FW_MATCH_LONG_OPT,
        CONFIRM_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        //tool specific options go here
        GENERIC_TEST_LONG_OPT,
        SHORT_GENERIC_LONG_OPT,
        TWO_MINUTE_TEST_LONG_OPT,
        LONG_GENERIC_LONG_OPT,
        USER_GENERIC_START_LONG_OPT,
        USER_GENERIC_RANGE_LONG_OPT,
        OD_MD_ID_TEST_LONG_OPT,
        OD_MD_ID_TEST_RANGE_LONG_OPT,
        ERROR_LIMIT_LONG_OPT,
        RANDOM_READ_TEST_LONG_OPT,
        BUTTERFLY_TEST_LONG_OPT,
        STOP_ON_ERROR_LONG_OPT,
        REPAIR_AT_END_LONG_OPT,
        REPAIR_ON_FLY_LONG_OPT,
        HOURS_TIME_LONG_OPT,
        MINUTES_TIME_LONG_OPT,
        SECONDS_TIME_LONG_OPT,
        DISPLAY_LBA_LONG_OPT,
        HIDE_LBA_COUNTER_LONG_OPT,
        BUFFER_TEST_LONG_OPT,
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
            }
            else if (strncmp(longopts[optionIndex].name, USER_GENERIC_LONG_OPT_START_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(USER_GENERIC_LONG_OPT_START_STRING))) == 0)
            {
                RUN_USER_GENERIC_TEST = true;
                if (0 == sscanf(optarg, "%"SCNu64, &USER_GENERIC_START_FLAG))
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(USER_GENERIC_LONG_OPT_START_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, USER_GENERIC_LONG_OPT_RANGE_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(USER_GENERIC_LONG_OPT_RANGE_STRING))) == 0)
            {
                USER_GENERIC_RANGE_FLAG = (uint64_t)atoll(optarg);
                //Check to see if any units were specified, otherwise assume LBAs
                uint64_t multiplier = 1;
                if (strstr(optarg, "KB"))
                {
                    USER_GENERIC_RANGE_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1000);
                }
                else if (strstr(optarg, "KiB"))
                {
                    USER_GENERIC_RANGE_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1024);
                }
                else if (strstr(optarg, "MB"))
                {
                    USER_GENERIC_RANGE_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1000000);
                }
                else if (strstr(optarg, "MiB"))
                {
                    USER_GENERIC_RANGE_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1048576);
                }
                else if (strstr(optarg, "GB"))
                {
                    USER_GENERIC_RANGE_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1000000000);
                }
                else if (strstr(optarg, "GiB"))
                {
                    USER_GENERIC_RANGE_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1073741824);
                }
                else if (strstr(optarg, "TB"))
                {
                    USER_GENERIC_RANGE_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1000000000000);
                }
                else if (strstr(optarg, "TiB"))
                {
                    USER_GENERIC_RANGE_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1099511627776);
                }
                USER_GENERIC_RANGE_FLAG *= multiplier;//Later we need to adjust this by the logical sector size...currently this is a value in bytes
            }
            else if (strncmp(longopts[optionIndex].name, ERROR_LIMIT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(ERROR_LIMIT_LONG_OPT_STRING))) == 0)
            {
                ERROR_LIMIT_FLAG = (uint16_t)atoi(optarg);
                if(strstr(optarg, "l"))
                {
                    ERROR_LIMIT_LOGICAL_COUNT = true;
                }
            }
            else if (strncmp(longopts[optionIndex].name, HOURS_TIME_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(HOURS_TIME_LONG_OPT_STRING))) == 0)
            {
                HOURS_TIME_FLAG = (uint8_t)atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, MINUTES_TIME_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(MINUTES_TIME_LONG_OPT_STRING))) == 0)
            {
                MINUTES_TIME_FLAG = (uint16_t)atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, SECONDS_TIME_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SECONDS_TIME_LONG_OPT_STRING))) == 0)
            {
                SECONDS_TIME_FLAG = (uint32_t)atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, GENERIC_TEST_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(GENERIC_TEST_LONG_OPT_STRING))) == 0)
            {
                if (strncmp(optarg, "read", M_Min(4, strlen(optarg))) == 0)
                {
                    GENERIC_TEST_MODE_FLAG = RWV_COMMAND_READ;
                }
                else if (strncmp(optarg, "write", M_Min(5, strlen(optarg))) == 0)
                {
                    GENERIC_TEST_MODE_FLAG = RWV_COMMAND_WRITE;
                }
                else if (strncmp(optarg, "verify", M_Min(6, strlen(optarg))) == 0)
                {
                    GENERIC_TEST_MODE_FLAG = RWV_COMMAND_VERIFY;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(GENERIC_TEST_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(MODEL_MATCH_LONG_OPT_STRING))) == 0)
            {
                MODEL_MATCH_FLAG = true;
                strncpy(MODEL_STRING_FLAG, optarg, 40);
            }
            else if (strncmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                FW_MATCH_FLAG = true;
                strncpy(FW_STRING_FLAG, optarg, 8);
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_MODEL_MATCH_LONG_OPT_STRING))) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                strncpy(CHILD_MODEL_STRING_FLAG, optarg, 40);
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                CHILD_FW_MATCH_FLAG = true;
                strncpy(CHILD_FW_STRING_FLAG, optarg, 8);
            }
            else if (strcmp(longopts[optionIndex].name, DISPLAY_LBA_LONG_OPT_STRING) == 0)
            {
                DISPLAY_LBA_FLAG = true;
                if (0 == sscanf(optarg, "%"SCNu64, &DISPLAY_LBA_THE_LBA))
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(DISPLAY_LBA_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, OD_MD_ID_TEST_LONG_OPT_STRING) == 0)
            {
                if (strchr(optarg, 'o') || strchr(optarg, 'O'))
                {
                    PERFORM_OD_TEST = true;
                }
                if (strchr(optarg, 'm') || strchr(optarg, 'M'))
                {
                    PERFORM_MD_TEST = true;
                }
                if (strchr(optarg, 'i') || strchr(optarg, 'I'))
                {
                    PERFORM_ID_TEST = true;
                }
            }
            else if (strcmp(longopts[optionIndex].name, OD_MD_ID_TEST_RANGE_LONG_OPT_STRING) == 0)
            {
                OD_ID_MD_TEST_RANGE = (uint64_t)atoll(optarg);
                //Check to see if any units were specified, otherwise assume LBAs
                uint64_t multiplier = 1;
                if (strstr(optarg, "KB"))
                {
                    OD_MD_ID_TEST_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1000);
                }
                else if (strstr(optarg, "KiB"))
                {
                    OD_MD_ID_TEST_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1024);
                }
                else if (strstr(optarg, "MB"))
                {
                    OD_MD_ID_TEST_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1000000);
                }
                else if (strstr(optarg, "MiB"))
                {
                    OD_MD_ID_TEST_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1048576);
                }
                else if (strstr(optarg, "GB"))
                {
                    OD_MD_ID_TEST_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1000000000);
                }
                else if (strstr(optarg, "GiB"))
                {
                    OD_MD_ID_TEST_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1073741824);
                }
                else if (strstr(optarg, "TB"))
                {
                    OD_MD_ID_TEST_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1000000000000);
                }
                else if (strstr(optarg, "TiB"))
                {
                    OD_MD_ID_TEST_UNITS_SPECIFIED = true;
                    multiplier = UINT64_C(1099511627776);
                }
                OD_ID_MD_TEST_RANGE *= multiplier;//Later we need to adjust this by the logical sector size...currently this is a value in bytes
            }
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
        if (!is_Running_Elevated())
        {
            print_Elevated_Privileges_Text();
        }
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

    if (!is_Running_Elevated())
    {
        print_Elevated_Privileges_Text();
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
        //check for other tool specific options here
        || SHORT_GENERIC_FLAG
        || TWO_MINUTE_TEST_FLAG
        || LONG_GENERIC_FLAG
        || RUN_USER_GENERIC_TEST
        || RANDOM_READ_TEST_FLAG
        || BUTTERFLY_READ_TEST_FLAG
        || DISPLAY_LBA_FLAG
        || (PERFORM_OD_TEST || PERFORM_ID_TEST || PERFORM_MD_TEST)
        || BUFFER_TEST_FLAG
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
        for (uint16_t handleIter = 0; handleIter < DEVICE_LIST_COUNT; ++handleIter)
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
                if(ret == PERMISSION_DENIED || !is_Running_Elevated())
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
            printf("\n%s - %s - %s - %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification, deviceList[deviceIter].drive_info.serialNumber, print_drive_type(&deviceList[deviceIter]));
        }

        //multiple the error limit by the number of logical sectors per physical sector
        ERROR_LIMIT_FLAG *= (deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize);

        //make sure a time was entered, otherwise, set to 1 minute
        if (SECONDS_TIME_FLAG == 0 && MINUTES_TIME_FLAG == 0 && HOURS_TIME_FLAG == 0)
        {
            SECONDS_TIME_FLAG = 60;
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

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (DISPLAY_LBA_FLAG)
        {
            uint8_t *displaySector = (uint8_t*)calloc_aligned(deviceList[deviceIter].drive_info.deviceBlockSize, sizeof(uint8_t), deviceList[deviceIter].os_info.minimumAlignment);
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
            safe_Free_aligned(displaySector);
        }

        if (BUFFER_TEST_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Performing buffer test. This may take a while...\n");
            }
            cableTestResults bufferTestResults;
            switch (perform_Cable_Test(&deviceList[deviceIter], &bufferTestResults))
            {
            case SUCCESS:
                print_Cable_Test_Results(bufferTestResults);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Buffer test is not supported on this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Buffer test failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        //check if the drive is seagate or not. If not seagate, disable the repairs because repairs are only allowed on Seagate Drives
        if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
        {
            if (REPAIR_ON_FLY_FLAG || REPAIR_AT_END_FLAG)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Repairs are only allowed on Seagate Drives.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                continue;
            }
            if (GENERIC_TEST_MODE_FLAG == RWV_COMMAND_WRITE)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Write command testing is only allowed on Seagate Drives.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                continue;
            }
        }

        if (REPAIR_AT_END_FLAG && REPAIR_ON_FLY_FLAG)
        {
            //tell the user that only one repair flag is allowed
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("ERROR: You cannot specify both repairAtEnd and repairOnFly. They are mutually exclusive.\n");
            }
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
        }

        if (GENERIC_TEST_MODE_FLAG == RWV_COMMAND_WRITE && !DATA_ERASE_FLAG)
        {
            //user must provide the confirmation string to enable write testing.
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\n");
                printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                printf("to the command line arguments to use a write operation in a generic test.\n\n");
                printf("e.g.: %s -d %s --shortGeneric --confirm %s\n\n", util_name, deviceHandleExample, DATA_ERASE_ACCEPT_STRING);
            }
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
        }

        if (SHORT_GENERIC_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Starting short generic test.\n");
            }
            switch (short_Generic_Test(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, NULL, NULL, HIDE_LBA_COUNTER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Short generic test completed successfully!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Short generic test is not supported on this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Short generic test failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (TWO_MINUTE_TEST_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Starting two minute generic test.\n");
            }
            switch (two_Minute_Generic_Test(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, NULL, NULL, HIDE_LBA_COUNTER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Two minute generic test completed successfully!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Two minute generic test is not supported on this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Two minute generic test failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (LONG_GENERIC_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Starting long generic test.\n");
            }
            if(ERROR_LIMIT_LOGICAL_COUNT)
            {
                ERROR_LIMIT_FLAG *= deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize;
            }
            switch (long_Generic_Test(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, ERROR_LIMIT_FLAG, STOP_ON_ERROR_FLAG, REPAIR_ON_FLY_FLAG, REPAIR_AT_END_FLAG, NULL, NULL, HIDE_LBA_COUNTER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Long generic test completed successfully!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Long generic test is not supported on this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Long generic test failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (RUN_USER_GENERIC_TEST)
        {
            uint64_t localRange = USER_GENERIC_RANGE_FLAG;
            if (localRange > 0)
            {
                if (USER_GENERIC_RANGE_UNITS_SPECIFIED)
                {
                    //this will convert the byte size to the nearest logical block (rounding up)
                    localRange = ((localRange + deviceList[deviceIter].drive_info.deviceBlockSize) - 1) / deviceList[deviceIter].drive_info.deviceBlockSize;
                }
                if (USE_MAX_LBA)
                {
                    USER_GENERIC_START_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba;
                    if (USER_GENERIC_RANGE_FLAG == 0 || USER_GENERIC_RANGE_FLAG > 1)
                    {
                        localRange = 1;
                    }
                }
                else if (USE_CHILD_MAX_LBA)
                {
                    USER_GENERIC_START_FLAG = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
                    if (USER_GENERIC_RANGE_FLAG == 0 || USER_GENERIC_RANGE_FLAG > 1)
                    {
                        localRange = 1;
                    }
                }
                if (USER_GENERIC_RANGE_FLAG == 0 && USER_GENERIC_START_FLAG == deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localRange = 1;
                }
                else if (USER_GENERIC_RANGE_FLAG == 0 && USER_GENERIC_START_FLAG != UINT64_MAX)
                {
                    localRange = deviceList[deviceIter].drive_info.deviceMaxLba - USER_GENERIC_START_FLAG;
                }
                if (localRange > 0 && USER_GENERIC_START_FLAG != UINT64_MAX)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Starting user generic test starting at LBA %"PRIu64" for the range %"PRIu64"\n", USER_GENERIC_START_FLAG, localRange);
                    }
                    if(ERROR_LIMIT_LOGICAL_COUNT)
                    {
                        ERROR_LIMIT_FLAG *= deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize;
                    }
                    switch (user_Sequential_Test(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, USER_GENERIC_START_FLAG, localRange, ERROR_LIMIT_FLAG, STOP_ON_ERROR_FLAG, REPAIR_ON_FLY_FLAG, REPAIR_AT_END_FLAG, NULL, NULL, HIDE_LBA_COUNTER))
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("User generic test completed successfully!\n");
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("User generic test is not supported on this device!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("User generic test failed!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                }
                else
                {
                    if (USER_GENERIC_RANGE_FLAG == 0)
                    {
                        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("You must enter a valid range!\n");
                        }
                        //you must enter a valid range
                    }
                    else if (USER_GENERIC_START_FLAG == UINT64_MAX)
                    {
                        //you must enter a valid starting LBA
                        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("You must enter a valid starting LBA!\n");
                        }
                    }
                }
            }
            else
            {
                //timed test
                uint64_t timeInSeconds = SECONDS_TIME_FLAG + (MINUTES_TIME_FLAG * 60) + (HOURS_TIME_FLAG * 3600);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    uint8_t days = 0, hours = 0, minutes = 0, seconds = 0;
                    convert_Seconds_To_Displayable_Time(timeInSeconds, NULL, &days, &hours, &minutes, &seconds);
                    printf("Starting user generic timed test at LBA %"PRIu64" for", USER_GENERIC_START_FLAG);
                    print_Time_To_Screen(NULL, &days, &hours, &minutes, &seconds);
                    printf("\n");
                }
                if(ERROR_LIMIT_LOGICAL_COUNT)
                {
                    ERROR_LIMIT_FLAG *= deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize;
                }
                switch (user_Timed_Test(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, USER_GENERIC_START_FLAG, timeInSeconds, ERROR_LIMIT_FLAG, STOP_ON_ERROR_FLAG, REPAIR_ON_FLY_FLAG, REPAIR_AT_END_FLAG, NULL, NULL, HIDE_LBA_COUNTER))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("User generic test completed successfully!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("User generic test is not supported on this device!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("User generic test failed!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
        }

        if (RANDOM_READ_TEST_FLAG)
        {
            time_t randomReadSeconds = SECONDS_TIME_FLAG + (MINUTES_TIME_FLAG * 60) + (HOURS_TIME_FLAG * 3600);
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Starting Random test\n");
            }
            switch (random_Test(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, randomReadSeconds, NULL, NULL, HIDE_LBA_COUNTER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Random test completed successfully!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Random test is not supported on this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Random test failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (BUTTERFLY_READ_TEST_FLAG)
        {
            time_t butterflyTestSeconds = SECONDS_TIME_FLAG + (MINUTES_TIME_FLAG * 60) + (HOURS_TIME_FLAG * 3600);
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Starting Buttefly test.\n");
            }
            switch (butterfly_Test(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, butterflyTestSeconds, NULL, NULL, HIDE_LBA_COUNTER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Butterfly test completed successfully!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Butterfly test is not supported on this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Butterfly test failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (PERFORM_OD_TEST || PERFORM_ID_TEST || PERFORM_MD_TEST)
        {
            uint64_t localRange = OD_ID_MD_TEST_RANGE;
            if (OD_MD_ID_TEST_UNITS_SPECIFIED)
            {
                //this will convert the byte size to the nearest logical block (rounding up)
                localRange = ((localRange + deviceList[deviceIter].drive_info.deviceBlockSize) - 1) / deviceList[deviceIter].drive_info.deviceBlockSize;
            }
            uint64_t OdMdIdTestSeconds = SECONDS_TIME_FLAG + (MINUTES_TIME_FLAG * 60) + (HOURS_TIME_FLAG * 3600);
            if (localRange > 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Starting diameter test\n");
                }
                if(ERROR_LIMIT_LOGICAL_COUNT)
                {
                    ERROR_LIMIT_FLAG *= deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize;
                }
                switch (diameter_Test_Range(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, PERFORM_OD_TEST, PERFORM_MD_TEST, PERFORM_ID_TEST, localRange, ERROR_LIMIT_FLAG, STOP_ON_ERROR_FLAG, REPAIR_ON_FLY_FLAG, REPAIR_AT_END_FLAG, NULL, NULL, HIDE_LBA_COUNTER))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Diameter test completed successfully!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Diameter test is not supported on this device!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Diameter test failed!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else if (OdMdIdTestSeconds > 0)
            {
                //run a timed OD MD ID test
                if(ERROR_LIMIT_LOGICAL_COUNT)
                {
                    ERROR_LIMIT_FLAG *= deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize;
                }
                switch (diameter_Test_Time(&deviceList[deviceIter], GENERIC_TEST_MODE_FLAG, PERFORM_OD_TEST, PERFORM_MD_TEST, PERFORM_ID_TEST, OdMdIdTestSeconds, ERROR_LIMIT_FLAG, STOP_ON_ERROR_FLAG, REPAIR_ON_FLY_FLAG, REPAIR_AT_END_FLAG, HIDE_LBA_COUNTER))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Diameter test completed successfully!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Diameter test is not supported on this device!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Diameter test failed!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must enter a range or a time!\n");
                }
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
    printf("\t%s --scan\n", util_name);
    printf("\t%s -d %s -i\n", util_name, deviceHandleExample);
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
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    //the test options
    printf("\nUtility Arguments\n");
    printf("=================\n");
    // Common(across utilities) - alphabetized
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Display_LBA_Help(shortUsage);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here
    print_Buffer_Test_Help(shortUsage);
    print_Butterfly_Read_Test_Help(shortUsage);
    print_OD_MD_ID_Test_Help(shortUsage);
    print_OD_MD_ID_Test_Range_Help(shortUsage);
    print_Error_Limit_Help(shortUsage);
    print_Generic_Test_Mode_Help(shortUsage);
    print_Time_Hours_Help(shortUsage);
    print_Long_Generic_Help(shortUsage);
    print_Time_Minutes_Help(shortUsage);
    print_Random_Read_Test_Help(shortUsage);
    print_Time_Seconds_Help(shortUsage);
    print_Short_Generic_Help(shortUsage);
    print_Stop_On_Error_Help(shortUsage);
    print_two_Minute_Test_Help(shortUsage);
    print_User_Generic_Start_Help(shortUsage);
    print_User_Generic_Range_Help(shortUsage);

    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //utility data destructive tests/operations go here
    print_Repair_At_End_Help(shortUsage);
    print_Repair_On_Fly_Help(shortUsage);
}
