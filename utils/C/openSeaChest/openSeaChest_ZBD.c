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
// \file openSeaChest_ZBD.c Binary command line that performs various ZAC/ZBC based commands.

//////////////////////
//  Included files  //
//////////////////////
#include "common.h"
#include <ctype.h>
#if defined (__unix__) || defined(__APPLE__) //using this definition because linux and unix compilers both define this. Apple does not define this, which is why it has it's own definition
#include <unistd.h>
#endif
#include "getopt.h"
#include "EULA.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "drive_info.h"
#include "zoned_operations.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_ZBD";
const char *buildVersion = "2.3.0";

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
    int exitCode = UTIL_EXIT_NO_ERROR;
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
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    //tool specific
    ZONE_ID_VARS
    REPORT_ZONES_VARS
    ZONE_MANAGEMENT_VARS
    MAX_ZONES_VAR

#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
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
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
        LOWLEVEL_INFO_LONG_OPT,
        //tool specific options go here
        ZONE_ID_LONG_OPT,
        REPORT_ZONES_LONG_OPT,
        CLOSE_ZONE_LONG_OPT,
        FINISH_ZONE_LONG_OPT,
        OPEN_ZONE_LONG_OPT,
        RESET_WP_LONG_OPT,
        MAX_ZONES_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
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
            if (strncmp(longopts[optionIndex].name, ZONE_ID_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(ZONE_ID_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "all") == 0)
                {
                    ZONE_ID_ALL_FLAG = true;
                }
                else
                {
                    if (!get_And_Validate_Integer_Input(C_CAST(const char *, optarg), &ZONE_ID_FLAG))
                    {
                        print_Error_In_Cmd_Line_Args(ZONE_ID_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, MAX_ZONES_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(MAX_ZONES_LONG_OPT_STRING))) == 0)
            {
                uint64_t temp = 0;
                if (get_And_Validate_Integer_Input(C_CAST(const char *, optarg), &temp))
                {
                    MAX_ZONES_FLAG = C_CAST(uint32_t, temp);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(MAX_ZONES_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, REPORT_ZONES_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(REPORT_ZONES_LONG_OPT_STRING))) == 0)
            {
                REPORT_ZONES_FLAG = true;
                if (strcmp(optarg, "all") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_ALL_ZONES;
                }
                else if (strcmp(optarg, "empty") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_EMPTY_ZONES;
                }
                else if (strcmp(optarg, "implicitOpen") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_IMPLICIT_OPEN_ZONES;
                }
                else if (strcmp(optarg, "explicitOpen") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_EXPLICIT_OPEN_ZONES;
                }
                else if (strcmp(optarg, "closed") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_CLOSED_ZONES;
                }
                else if (strcmp(optarg, "full") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_FULL_ZONES;
                }
                else if (strcmp(optarg, "readOnly") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_READ_ONLY_ZONES;
                }
                else if (strcmp(optarg, "offline") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_OFFLINE_ZONES;
                }
                else if (strcmp(optarg, "resetRecommended") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_ZONES_WITH_RESET_SET_TO_ONE;
                }
                else if (strcmp(optarg, "nonSeqResourceAvailable") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_ZONES_WITH_NON_SEQ_SET_TO_ONE;
                }
                else if (strcmp(optarg, "allNonWP") == 0)
                {
                    REPORT_ZONES_REPORTING_MODE_FLAG = ZONE_REPORT_LIST_ALL_ZONES_THAT_ARE_NOT_WRITE_POINTERS;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(REPORT_ZONES_LONG_OPT_STRING, optarg);
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
        if (AGRESSIVE_SCAN_FLAG)
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
        eDiscoveryOptions flags = 0;
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
	|| (FORCE_SCSI_FLAG && FORCE_NVME_FLAG)
	|| (FORCE_ATA_FLAG && FORCE_NVME_FLAG)
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
        || LOWLEVEL_INFO_FLAG
        //check for other tool specific options here
        || REPORT_ZONES_FLAG
        || CLOSE_ZONE_FLAG
        || FINISH_ZONE_FLAG
        || OPEN_ZONE_FLAG
        || RESET_WP_FLAG
        ))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    eDiscoveryOptions flags = 0;
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

    if (TEST_UNIT_READY_FLAG)
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

        if (deviceList[deviceIter].drive_info.zonedType == ZONED_TYPE_NOT_ZONED)
        {
            if (toolVerbosity > VERBOSITY_QUIET)
            {
                printf("This tool can only be run on Zoned (SMR) devices.\n");
            }
            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            continue;
        }

        if (CLOSE_ZONE_FLAG)
        {
            if (ZONE_ID_FLAG == UINT64_MAX && !ZONE_ID_ALL_FLAG)
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("You must use the --%s option to specify a zone ID.\n", ZONE_ID_LONG_OPT_STRING);
                }
            }
            else
            {
                if (ZONE_ID_ALL_FLAG)
                {
                    ZONE_ID_FLAG = 0;
                }
                switch (close_Zone(&deviceList[deviceIter], ZONE_ID_ALL_FLAG, ZONE_ID_FLAG, 0))
                {
                case SUCCESS:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        if (ZONE_ID_ALL_FLAG)
                        {
                            printf("Successfully closed all zones!\n");
                        }
                        else
                        {
                            printf("Successfully closed zone %"PRIu64"\n", ZONE_ID_FLAG);
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Closing zone(s) not supported\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Closing zone(s) failed\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }
        if (FINISH_ZONE_FLAG)
        {
            if (ZONE_ID_FLAG == UINT64_MAX && !ZONE_ID_ALL_FLAG)
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("You must use the --%s option to specify a zone ID.\n", ZONE_ID_LONG_OPT_STRING);
                }
            }
            else
            {
                if (ZONE_ID_ALL_FLAG)
                {
                    ZONE_ID_FLAG = 0;
                }
                switch (finish_Zone(&deviceList[deviceIter], ZONE_ID_ALL_FLAG, ZONE_ID_FLAG, 0))
                {
                case SUCCESS:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        if (ZONE_ID_ALL_FLAG)
                        {
                            printf("Successfully finished all zones!\n");
                        }
                        else
                        {
                            printf("Successfully finished zone %"PRIu64"\n", ZONE_ID_FLAG);
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Finishing zone(s) not supported\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Finishing zone(s) failed\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }
        if (OPEN_ZONE_FLAG)
        {
            if (ZONE_ID_FLAG == UINT64_MAX && !ZONE_ID_ALL_FLAG)
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("You must use the --%s option to specify a zone ID.\n", ZONE_ID_LONG_OPT_STRING);
                }
            }
            else
            {
                if (ZONE_ID_ALL_FLAG)
                {
                    ZONE_ID_FLAG = 0;
                }
                switch (open_Zone(&deviceList[deviceIter], ZONE_ID_ALL_FLAG, ZONE_ID_FLAG, 0))
                {
                case SUCCESS:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        if (ZONE_ID_ALL_FLAG)
                        {
                            printf("Successfully opened all zones!\n");
                        }
                        else
                        {
                            printf("Successfully opened zone %"PRIu64"\n", ZONE_ID_FLAG);
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Opening zone(s) not supported\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Opening zone(s) failed\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }
        if (RESET_WP_FLAG)
        {
            if (ZONE_ID_FLAG == UINT64_MAX && !ZONE_ID_ALL_FLAG)
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("You must use the --%s option to specify a zone ID.\n", ZONE_ID_LONG_OPT_STRING);
                }
            }
            else
            {
                if (ZONE_ID_ALL_FLAG)
                {
                    ZONE_ID_FLAG = 0;
                }
                switch (reset_Write_Pointer(&deviceList[deviceIter], ZONE_ID_ALL_FLAG, ZONE_ID_FLAG, 0))
                {
                case SUCCESS:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        if (ZONE_ID_ALL_FLAG)
                        {
                            printf("Successfully reset all write pointers!\n");
                        }
                        else
                        {
                            printf("Successfully reset write pointer for zone %"PRIu64"\n", ZONE_ID_FLAG);
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Resetting write pointer(s) not supported\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Resetting write pointer(s) failed\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }
        if (REPORT_ZONES_FLAG)
        {
            uint32_t numberOfZones = 0;
            if (ZONE_ID_FLAG == UINT64_MAX)
            {
                ZONE_ID_FLAG = 0;
            }
            if (SUCCESS == get_Number_Of_Zones(&deviceList[deviceIter], REPORT_ZONES_REPORTING_MODE_FLAG, ZONE_ID_FLAG, &numberOfZones))
            {
                numberOfZones = M_Min(MAX_ZONES_FLAG, numberOfZones);
                ptrZoneDescriptor zoneDescriptors = C_CAST(ptrZoneDescriptor, calloc(numberOfZones, sizeof(zoneDescriptor)));
                if (!zoneDescriptors)
                {
                    perror("cannot allocate memory for zone descriptors");
                    exit(UTIL_EXIT_OPERATION_FAILURE);
                }
                int reportRet = get_Zone_Descriptors(&deviceList[deviceIter], REPORT_ZONES_REPORTING_MODE_FLAG, ZONE_ID_FLAG, numberOfZones, zoneDescriptors);
                switch (reportRet)
                {
                case SUCCESS:
                    print_Zone_Descriptors(REPORT_ZONES_REPORTING_MODE_FLAG, numberOfZones, zoneDescriptors);
                    break;
                case NOT_SUPPORTED:
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    printf("Reporting zones is not supported on this device.\n");
                    break;
                default:
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    printf("Failed to get zones to report!\n");
                    break;
                }
                safe_Free(zoneDescriptors);
            }
            else
            {
                printf("Unable to get number of zones.\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
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
    printf("\t%s -d %s --%s all\n", util_name, deviceHandleExample, REPORT_ZONES_LONG_OPT_STRING);
    printf("\t%s -d %s --%s empty --%s 256\n", util_name, deviceHandleExample, REPORT_ZONES_LONG_OPT_STRING, MAX_ZONES_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s 16\n", util_name, deviceHandleExample, OPEN_ZONE_LONG_OPT_STRING, ZONE_ID_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s 16\n", util_name, deviceHandleExample, FINISH_ZONE_LONG_OPT_STRING, ZONE_ID_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s 16\n", util_name, deviceHandleExample, CLOSE_ZONE_LONG_OPT_STRING, ZONE_ID_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s 16\n", util_name, deviceHandleExample, RESET_WP_LONG_OPT_STRING, ZONE_ID_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s all\n", util_name, deviceHandleExample, RESET_WP_LONG_OPT_STRING, ZONE_ID_LONG_OPT_STRING);

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
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_No_Banner_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    //the test options
    printf("\nUtility Arguments\n");
    printf("=================\n");
    //Common (across utilities) - alphabetized
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Low_Level_Info_Help(shortUsage);
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    print_Fast_Discovery_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    print_Close_Zone_Help(shortUsage);
    print_Finish_Zone_Help(shortUsage);
    print_Max_Zones_Help(shortUsage);
    print_Open_Zone_Help(shortUsage);
    print_Report_Zones_Help(shortUsage);
    print_Reset_Write_Pointer_Zone_Help(shortUsage);
    print_Zone_ID_Help(shortUsage);
}
