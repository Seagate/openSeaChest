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
// \file openSeaChest_Logs.c Binary command line that performs various logs related operations.


//////////////////////
//  Included files  //
//////////////////////
#include "common.h"
#include <ctype.h>
#if defined (__unix__) || defined(__APPLE__) //using this definition because linux and unix compilers both define this. Apple does not define this, which is why it has it's own definition
#include <unistd.h>
#include <getopt.h>
#elif defined (_WIN32)
#include "getopt.h"
#else
#error "OS Not Defined or known"
#endif
#include "common_public.h"
#include "EULA.h"
#include "operations.h"
//include the seachest util options for the device option, drive info option and a few other things that are needed.
#include "openseachest_util_options.h"
#include "logs.h"
#include "drive_info.h"

////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_Logs";
const char *buildVersion = "2.1.0";

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
    OUTPUTPATH_VAR
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
    //add tool specific flags here
    LIST_LOGS_VAR
    LIST_ERROR_HISTORY_VAR
    GENERIC_LOG_VAR
    GENERIC_LOG_SUBPAGE_VAR
    GENERIC_ERROR_HISTORY_VARS
    PULL_LOG_MODE_VARS
    FARM_VAR
    DST_LOG_VAR
    IDENTIFY_DEVICE_DATA_LOG_VAR
    SATA_PHY_COUNTERS_LOG_VAR
    DEVICE_STATS_LOG_VAR
    INFORMATIONAL_EXCEPTIONS_VAR

#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    LOG_TRANSFER_LENGTH_BYTES_VAR

    int  args = 0;
    int argIndex = 0;
    int optionIndex = 0;

    //add -- options to this structure DO NOT ADD OPTIONAL ARGUMENTS! Optional arguments are a GNU extension and are not supported in Unix or some compilers- TJE
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
        OUTPUTPATH_LONG_OPT,
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
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        //Utility specific options
        LIST_LOGS_LONG_OPT,
        LIST_ERROR_HISTORY_LONG_OPT,
        GENERIC_LOG_LONG_OPT,
        GENERIC_LOG_SUBPAGE_LONG_OPT,
        GENERIC_ERROR_HISTORY_LONG_OPT,
        PULL_LOG_MODE_LONG_OPT,
        FARM_LONG_OPT,
        DST_LOG_LONG_OPT,//standard spec log
        DEVICE_STATS_LOG_LONG_OPT,//standard spec log
        IDENTIFY_DEVICE_DATA_LOG_LONG_OPT,//standard ATA spec log
        SATA_PHY_COUNTERS_LONG_OPT,//standard ATA spec log
        INFROMATIONAL_EXCEPTIONS_LONG_OPT,
        LOG_TRANSFER_LENGTH_LONG_OPT,
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
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    //get options we know we need
    while (1) //changed to while 1 in order to parse multiple options when longs options are involved
    {
        args = getopt_long(argc, argv, "d:hisSF:Vv:qr:R:E:e:w:c:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        //printf("Parsing args <%u> %s\n", args, logslongopts[optionIndex].name);
        switch (args)
        {
        case 0:
            if (strncmp(longopts[optionIndex].name, GENERIC_LOG_LONG_OPT_STRING, strlen(GENERIC_LOG_LONG_OPT_STRING)) == 0)
            {
                uint64_t temp = 0;
                if ( get_And_Validate_Integer_Input((const char *) optarg, &temp) )
                {
                    GENERIC_LOG_PULL_FLAG = true;
                    GENERIC_LOG_DATA_SET = C_CAST(uint8_t, temp);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(GENERIC_LOG_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, GENERIC_LOG_SUBPAGE_LONG_OPT_STRING, strlen(GENERIC_LOG_SUBPAGE_LONG_OPT_STRING)) == 0)
            {
                uint64_t temp = 0;
                if ( get_And_Validate_Integer_Input((const char *) optarg, &temp) )
                {
                    //no need to do anything...this option requires that the page is also given
                    GENERIC_LOG_SUBPAGE_DATA_SET = C_CAST(uint8_t, temp);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(GENERIC_LOG_SUBPAGE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, GENERIC_ERROR_HISTORY_LONG_OPT_STRING, strlen(GENERIC_ERROR_HISTORY_LONG_OPT_STRING)) == 0)
            {
                if ( get_And_Validate_Integer_Input((const char *) optarg, &GENERIC_ERROR_HISTORY_BUFFER_ID) )
                {
                    GENERIC_ERROR_HISTORY_PULL_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(GENERIC_ERROR_HISTORY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, LOG_TRANSFER_LENGTH_LONG_OPT_STRING, strlen(LOG_TRANSFER_LENGTH_LONG_OPT_STRING)) == 0)
            {
                //set the raw data length - but check the units first!
                uint64_t multiplier = 1;
                uint64_t optargInt = C_CAST(uint64_t, atoll(optarg));
                if (strstr(optarg, "BLOCKS") || strstr(optarg, "SECTORS"))
                {
                    //they specified blocks. For log transfers this means a number of 512B sectors
                    multiplier = LEGACY_DRIVE_SEC_SIZE;
                }
                else if (strstr(optarg, "KB"))
                {
                    multiplier = UINT64_C(1000);
                }
                else if (strstr(optarg, "KiB"))
                {
                    multiplier = UINT64_C(1024);
                }
                else if (strstr(optarg, "MB"))
                {
                    multiplier = UINT64_C(1000000);
                }
                else if (strstr(optarg, "MiB"))
                {
                    multiplier = UINT64_C(1048576);
                }
                else if (strstr(optarg, "GB"))
                {
                    multiplier = UINT64_C(1000000000);
                }
                else if (strstr(optarg, "GiB"))
                {
                    multiplier = UINT64_C(1073741824);
                }
                else if (strstr(optarg, "TB"))
                {
                    multiplier = UINT64_C(1000000000000);
                }
                else if (strstr(optarg, "TiB"))
                {
                    multiplier = UINT64_C(1099511627776);
                }
                LOG_TRANSFER_LENGTH_BYTES = C_CAST(uint32_t, optargInt * multiplier);
            }
            else if (strncmp(longopts[optionIndex].name, PATH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), 9)) == 0)
            {
                OUTPUTPATH_PARSE
                if (!os_Directory_Exists(OUTPUTPATH_FLAG))
                {
                    printf("Err: --outputPath %s does not exist\n", OUTPUTPATH_FLAG);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }

            }
            else if (strncmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING, strlen(MODEL_MATCH_LONG_OPT_STRING)) == 0)
            {
                MODEL_MATCH_FLAG = true;
                snprintf(MODEL_STRING_FLAG, MODEL_STRING_LENGTH, "%s", optarg);
            }
            else if (strncmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING, strlen(FW_MATCH_LONG_OPT_STRING)) == 0)
            {
                FW_MATCH_FLAG = true;
                snprintf(FW_STRING_FLAG, FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING, strlen(CHILD_MODEL_MATCH_LONG_OPT_STRING)) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                snprintf(CHILD_MODEL_STRING_FLAG, CHILD_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING, strlen(CHILD_FW_MATCH_LONG_OPT_STRING)) == 0)
            {
                CHILD_FW_MATCH_FLAG = true;
                snprintf(CHILD_FW_STRING_FLAG, CHILD_FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strncmp(longopts[optionIndex].name, PULL_LOG_MODE_LONG_OPT_STRING, strlen(PULL_LOG_MODE_LONG_OPT_STRING)) == 0)
            {
                if (strcmp(optarg, "raw") == 0)
                {
                    PULL_LOG_MODE = PULL_LOG_RAW_MODE;
                }
                else if (strcmp(optarg, "bin") == 0)
                {
                    PULL_LOG_MODE = PULL_LOG_BIN_FILE_MODE;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PULL_LOG_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            //parse long options that have required args here.
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
        print_EULA_To_Screen(false, false);
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
        || LIST_LOGS_FLAG
        || LIST_ERROR_HISTORY_FLAG
        || GENERIC_LOG_PULL_FLAG
        || GENERIC_ERROR_HISTORY_PULL_FLAG
        || FARM_PULL_FLAG
        || DST_LOG_FLAG
        || IDENTIFY_DEVICE_DATA_LOG_FLAG
        || SATA_PHY_COUNTERS_LOG_FLAG
        || DEVICE_STATS_LOG_FLAG
        || INFORMATIONAL_EXCEPTIONS_FLAG
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

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
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
                    printf("\nListing supported logs is not supported for device %s.\n", \
                        deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to list logs for device %s\n", \
                        deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (LIST_ERROR_HISTORY_FLAG)
        {
            switch (print_Supported_SCSI_Error_History_Buffer_IDs(&deviceList[deviceIter], 0))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nListing supported error history buffer IDs is not supported for device %s.\n", \
                        deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to list error history buffer IDs for device %s\n", \
                        deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (GENERIC_LOG_PULL_FLAG)
        {
            switch (pull_Generic_Log(&deviceList[deviceIter], GENERIC_LOG_DATA_SET, GENERIC_LOG_SUBPAGE_DATA_SET, PULL_LOG_MODE, OUTPUTPATH_FLAG, LOG_TRANSFER_LENGTH_BYTES, 0))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && GENERIC_LOG_SUBPAGE_DATA_SET != 0)
                    {
                        printf("\nSuccessfully pulled Log %" PRIu8 ", subpage %" PRIu8 " from %s\n", GENERIC_LOG_DATA_SET, GENERIC_LOG_SUBPAGE_DATA_SET, deviceList[deviceIter].drive_info.serialNumber);
                    }
                    else
                    {
                        printf("\nSuccessfully pulled Log %" PRIu8 " from %s\n", GENERIC_LOG_DATA_SET, deviceList[deviceIter].drive_info.serialNumber);
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && GENERIC_LOG_SUBPAGE_DATA_SET != 0)
                    {
                        printf("\nLog %" PRIu8 ", subpage %" PRIu8 " not supported by %s\n", GENERIC_LOG_DATA_SET, GENERIC_LOG_SUBPAGE_DATA_SET, deviceList[deviceIter].drive_info.serialNumber);
                    }
                    else
                    {
                        printf("\nLog %" PRIu8 " not supported by %s\n",\
                            GENERIC_LOG_DATA_SET, deviceList[deviceIter].drive_info.serialNumber);
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case MEMORY_FAILURE:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && GENERIC_LOG_SUBPAGE_DATA_SET != 0)
                    {
                        printf("\nFailed to allocate memory for log %" PRIu8 ", subpage %" PRIu8 "\n", GENERIC_LOG_DATA_SET, GENERIC_LOG_SUBPAGE_DATA_SET);
                    }
                    else
                    {
                        printf("\nFailed to allocate memory for log %" PRIu8 "\n", GENERIC_LOG_DATA_SET);
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && GENERIC_LOG_SUBPAGE_DATA_SET != 0)
                    {
                        printf("\nFailed to pull log %" PRIu8 ", subpage %" PRIu8 " from %s\n", GENERIC_LOG_DATA_SET, GENERIC_LOG_SUBPAGE_DATA_SET, deviceList[deviceIter].drive_info.serialNumber);
                    }
                    else
                    {
                        printf("\nFailed to pull log %" PRIu8 " from %s\n", GENERIC_LOG_DATA_SET, deviceList[deviceIter].drive_info.serialNumber);
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (GENERIC_ERROR_HISTORY_PULL_FLAG)
        {
            switch (pull_Generic_Error_History(&deviceList[deviceIter], C_CAST(uint8_t, GENERIC_ERROR_HISTORY_BUFFER_ID), PULL_LOG_MODE, OUTPUTPATH_FLAG, LOG_TRANSFER_LENGTH_BYTES, 0))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully pulled buffer ID %" PRIu64 " from %s\n", GENERIC_ERROR_HISTORY_BUFFER_ID, deviceList[deviceIter].drive_info.serialNumber);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nBuffer ID %" PRIu64 " not supported by %s\n", GENERIC_ERROR_HISTORY_BUFFER_ID, deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case MEMORY_FAILURE:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to allocate memory for buffer ID %"PRIu64"\n", GENERIC_ERROR_HISTORY_BUFFER_ID);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to pull buffer ID %" PRIu64 " from %s\n", GENERIC_ERROR_HISTORY_BUFFER_ID, deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (FARM_PULL_FLAG)
        {
            switch (pull_FARM_Log(&deviceList[deviceIter], OUTPUTPATH_FLAG, LOG_TRANSFER_LENGTH_BYTES, 0, SEAGATE_ATA_LOG_FIELD_ACCESSIBLE_RELIABILITY_METRICS, 0))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully pulled FARM log\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("FARM log not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to pull FARM log\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (DST_LOG_FLAG)
        {
            switch (get_DST_Log(&deviceList[deviceIter], OUTPUTPATH_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Self Test Log pulled successfully from device!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Self Test Log not supported by this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to pull Self Test Log from this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (IDENTIFY_DEVICE_DATA_LOG_FLAG)
        {
            switch (get_Identify_Device_Data_Log(&deviceList[deviceIter], OUTPUTPATH_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Identify Device Data Log pulled successfully from device!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Identify Device Data Log not supported by this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to pull Identify Device Data Log from this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (SATA_PHY_COUNTERS_LOG_FLAG)
        {
            switch (get_SATA_Phy_Event_Counters_Log(&deviceList[deviceIter], OUTPUTPATH_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SATA Phy Event Counters Log pulled successfully from device!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SATA Phy Event Counters Log not supported by this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to pull SATA Phy Event Counters Log from this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (DEVICE_STATS_LOG_FLAG)
        {
            switch (get_Device_Statistics_Log(&deviceList[deviceIter], OUTPUTPATH_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Device Statistics Log pulled successfully from device!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Device Statistics Log not supported by this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to pull Device Statistics Log from this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (INFORMATIONAL_EXCEPTIONS_FLAG)
        {
            switch (pull_SCSI_Informational_Exceptions_Log(&deviceList[deviceIter], OUTPUTPATH_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCSI Informational Exceptions Log pulled successfully from device!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCSI Informational Exceptions Log not supported by this device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to pull SCSI Informational Exceptions Log from this device!\n");
                }
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
    printf("\t%s --scan\n", util_name);
    printf("\t%s -d %s -i\n", util_name, deviceHandleExample);
    printf("\t%s -d %s --farm --outputPath logs\n", util_name, deviceHandleExample);
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
    print_OutputPath_Help(shortUsage);
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
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    //multiple interfaces
	print_Fast_Discovery_Help(shortUsage);
    printf("\n");
    print_Pull_Device_Statistics_Log_Help(shortUsage);
    print_FARM_Log_Help(shortUsage);
    print_Supported_Logs_Help(shortUsage);
    print_Log_Mode_Help(shortUsage);
    print_Log_Transfer_Length_Help(shortUsage);
    print_Pull_Generic_Logs_Help(shortUsage);
    print_Pull_Self_Test_Results_Log_Help(shortUsage);

    //SATA Only Options
    printf("\n\tSATA Only:\n\n");
    print_Pull_Identify_Device_Data_Log_Help(shortUsage);
    print_Pull_SATA_Phy_Event_Counters_Log_Help(shortUsage);
    //SAS Only Options
    printf("\n\tSAS Only:\n\n");
    print_Supported_Error_History_Help(shortUsage);
    print_Pull_Generic_Error_History_Help(shortUsage);
    print_Pull_Informational_Exceptions_Log_Help(shortUsage);
    print_Pull_Generic_Logs_Subpage_Help(shortUsage);

    
}
