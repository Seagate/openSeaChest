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
// \file openSeaChest_PassthroughTest.c CLI tool that attempts to determine what works or doesn't work to talk passthrough to a device

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
#include "drive_info.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_PassthroughTest";
const char *buildVersion = "0.0.1";

////////////////////////////
//  functions to declare  //
////////////////////////////
static void utility_Usage(bool shortUsage);


typedef struct _passthroughTestParams
{
    tDevice *device;//pointer to device to test
    uint16_t skipToStepNumber;//leave this at zero unless skipping ahead to a different part of the test.
    ePassthroughType passthroughType;//Only set this to seomthing ahead of time if non-sat
    bool checkReadWriteSupport;//optional test. Not on by default since most devices won't work properly (unless it's actually a SAS drive that supports a transfer length of zero)
    bool checkAdditionalSCSICommandSupport;//other commands like report supported operations codes, security protocol, get physical element status (SAT5+), etc
    bool forceRetest;//even though the device is already known, retest it.
}passthroughTestParams, *ptrPassthroughTestParams;

int perform_Passthrough_Test(ptrPassthroughTestParams inputs);

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
    //DATA_ERASE_VAR
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

#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif

    int8_t  args = 0;
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
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        //tool specific options go here
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
            if (strncmp(longopts[optionIndex].name, "longOption", M_Min(strlen(longopts[optionIndex].name), strlen("longOption"))) == 0)
            {
                //set flags
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
            printf("You must run with elevated privileges to communicate with devices in the system.\n");
            exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
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

    if (!is_Running_Elevated())
    {
        printf("You must run with elevated privileges to communicate with devices in the system.\n");
        exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
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

    if (passthroughTest)
    {
        flags = OPEN_HANDLE_ONLY;
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
            deviceList[deviceIter].drive_info.passThroughHacks.useA1SATPassthroughWheneverPossible = true;
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

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (passthroughTest)
        {
            //Need to allow a way to skip to certain parts of a test if the device has problems, or hangs, or anything like that.
            
            
        }
    }
    exit(exitCode);
}

int perform_Passthrough_Test(ptrPassthroughTestParams inputs)
{
    int ret = SUCCESS;
    printf("Performing Passthrough test. \n");
    printf("If at any point during the test, a crash, a hang, or the device\n");
    printf("seems to stop responding to any normal request, save all previously\n");
    printf("reported hacks, stop the tool, reset the device, then restart\n");
    printf("the test, skipping to another part of the test to avoid the same issue.\n");
    printf("All discovered hacks will be necessary in order to fully communicate\n");
    printf("with the end target device to the highest capabilities of the adapter or\n");
    printf("bridge performing the translations.\n\n");

    //TODO: Need a way to report when an OS doesn't provide the information necessary to setup hacks based on IDs.

    if (inputs && inputs->device)
    {
        //step 1, check if already in the code database.
        if (setup_Passthrough_Hacks_By_ID(inputs->device) && !inputs->forceRetest)
        {
            printf("This device is already in the database with known passthrough hacks.\n");
            printf("Use the --%s option to force a retest of passthrough hacks already known.\n", "--retestHacks");
        }
        //2. Check what things the device reports for SCSI capabilities, SAT VPD page, etc. Emit warnings for pages that are missing that were expected
        printf("Checking standard SCSI inquiry data, VPD pages, and some mode pages\n")
        printf("to understand device capabilities. Only commands specified in translator\n");
        printf("specifications will be tested.\n");

        //TODO: save information from some pages to compare later to the data retrieved through passthrough
        printf("\tReading Inquiry Data\n");
        char vendorId[T10_VENDOR_ID_LEN + 1] = { 0 };
        char productId[INQ_DATA_PRODUCT_ID_LEN + 1] = { 0 };
        char productRev[INQ_DATA_PRODUCT_REV_LEN + 1] = { 0 };
        char *unitSerialNumber = NULL;//This will be allocated later!
        if (SUCCESS == scsi_Inquiry(device, (uint8_t *)&device->drive_info.scsiVpdData.inquiryData, 96, 0, false, false))
        {
            bool continueTesting = false;
            uint8_t *inqPtr = &device->drive_info.scsiVpdData.inquiryData;
            memset(device->drive_info.serialNumber, 0, sizeof(device->drive_info.serialNumber));
            memset(device->drive_info.T10_vendor_ident, 0, sizeof(device->drive_info.T10_vendor_ident));
            memset(device->drive_info.product_identification, 0, sizeof(device->drive_info.product_identification));
            memset(device->drive_info.product_revision, 0, sizeof(device->drive_info.product_revision));

            //print out peripheral device type
            uint8_t peripheralDeviceType = M_GETBITRANGE(inqPtr[0], 4, 0);
            printf("\t\tPeripheral Device Type: ");
            switch (peripheralDeviceType)
            {
            case PERIPHERAL_DIRECT_ACCESS_BLOCK_DEVICE:
                printf("Direct Access Block Device (SBC)\n");
                continueTesting = true;
                break;
            case PERIPHERAL_SEQUENTIAL_ACCESS_BLOCK_DEVICE:
                printf("Sequential Access Block Device (SSC)\n");
                break;
            case PERIPHERAL_PRINTER_DEVICE:
                printf("Printer Device (SSC)\n");
                break;
            case PERIPHERAL_PROCESSOR_DEVICE:
                printf("Processor Device (SPC)\n");
                break;
            case PERIPHERAL_WRITE_ONCE_DEVICE:
                printf("Write Once Device (SBC)\n");
                break;
            case PERIPHERAL_CD_DVD_DEVICE:
                printf("CD/DVD Device (MMC)\n");
                break;
            case PERIPHERAL_SCANNER_DEVICE:
                printf("Scanner Device (SGC)\n");
                break;
            case PERIPHERAL_OPTICAL_MEMORY_DEVICE:
                printf("Optical Memory Device (SBC)\n");
                break;
            case PERIPHERAL_MEDIUM_CHANGER_DEVICE:
                printf("Medium Changer Device (SMC)\n");
                break;
            case PERIPHERAL_COMMUNICATIONS_DEVICE:
                printf("Communications Device (SSC)\n");
                break;
            case PERIPHERAL_ASC_IT8_1:
            case PERIPHERAL_ASC_IT8_2:
                printf("Graphic arts pre-press Device (ASC IT8)\n");
                break;
            case PERIPHERAL_STORAGE_ARRAY_CONTROLLER_DEVICE:
                printf("Storage Array Controller Device (SCC)\n");
                break;
            case PERIPHERAL_ENCLOSURE_SERVICES_DEVICE:
                printf("Enclosure Services Device (SES)\n");
                break;
            case PERIPHERAL_SIMPLIFIED_DIRECT_ACCESS_DEVICE:
                printf("Simplified Direct Access Block Device (RBC)\n");
                break;
            case PERIPHERAL_OPTICAL_CARD_READER_WRITER_DEVICE:
                printf("Optical Card Reader/Writer Device (OCRW)\n");
                break;
            case PERIPHERAL_BRIDGE_CONTROLLER_COMMANDS:
                printf("Reserved for Bridge Contoller/Expanders\n");
                break;
            case PERIPHERAL_OBJECT_BASED_STORAGE_DEVICE:
                printf("Object-Based Storage Device (OSD)\n");
                break;
            case PERIPHERAL_AUTOMATION_DRIVE_INTERFACE:
                printf("Automation/Drive Interface (ADC)\n");
                break;
            case PERIPHERAL_SECURITY_MANAGER_DEVICE:
                printf("Security Manager Device\n");
                break;
            case PERIPHERAL_HOST_MANAGED_ZONED_BLOCK_DEVICE:
                continueTesting = true;
                printf("Host Managed Zoned Block Device (ZBC)\n");
                break;
            case PERIPHERAL_WELL_KNOWN_LOGICAL_UNIT:
                printf("Well Known Logical Unit\n");
                break;
            case PERIPHERAL_RESERVED3:
            case PERIPHERAL_RESERVED4:
            case PERIPHERAL_RESERVED5:
            case PERIPHERAL_RESERVED6:
            case PERIPHERAL_RESERVED7:
            case PERIPHERAL_RESERVED8:
            case PERIPHERAL_RESERVED9:
            case PERIPHERAL_RESERVED10:
            case PERIPHERAL_RESERVED11:
                printf("Reserved Device Type (%" PRIX8 "h)\n", peripheralDeviceType);
                break;
            case PERIPHERAL_UNKNOWN_OR_NO_DEVICE_TYPE:
            default:
                printf("Unknown or no device type\n");
                break;
            }
            //check rmb bit (show warning if this is set unless this truly is ejectable media, which an HDD is not)
            if (inqPtr[1] & BIT7)
            {
                printf("\t\tRMB bit is set meaning the recording medium is removable. Ex: CD/DVD\n");
            }

            //Report value for version, translated to SCSI/SPC version. Report warnings if below SPC3 since this is when SAT was first released (version1 in 2006)
            printf("\t\tSCSI Version: ");
            device->drive_info.scsiVersion = inqPtr[2];
            switch (inqPtr[2])
            {
            case 0:
                printf("No version compliance reported. This is typical for very old devices...SASI devices\n");
                device->drive_info.scsiVersion = SCSI_VERSION_NO_STANDARD;
//              if (device->drive_info.interface_type != USB_INTERFACE)
//              {
//                  checkForSAT = false; //NOTE: some cheap USB to SATA/PATA adapters will set this version or no version. The only way to work around this, is to make sure the low level for the OS detects it on USB interface and it can be run through the usb_hacks file instead.
//              }
                break;
            case 0x01:
            case 0x81:
                printf("SCSI (1986) - (%" PRIX8 "h)\n", inqPtr[2]);
                device->drive_info.scsiVersion = SCSI_VERSION_SCSI;//changing to 1 for SCSI
//              if (device->drive_info.interface_type != USB_INTERFACE)
//              {
//                  checkForSAT = false;//NOTE: some cheap USB to SATA/PATA adapters will set this version or no version. The only way to work around this, is to make sure the low level for the OS detects it on USB interface and it can be run through the usb_hacks file instead.
//              }
                break;
            case 0x02:
            case 0x80:
            case 0x82:
                printf("SCSI 2 (1993) - (%" PRIX8 "h)\n", inqPtr[2]);
                device->drive_info.scsiVersion = SCSI_VERSION_SCSI2;//changing to 2 for SCSI 2
                break;
            case 0x03:
            case 0x83:
                printf("SPC (1997) - (%" PRIX8 "h)\n", inqPtr[2]);
                device->drive_info.scsiVersion = SCSI_VERSION_SPC;//changing to 3 for SPC
                break;
            case 0x04:
            case 0x84:
                printf("SPC2 (2001) - (%" PRIX8 "h)\n", inqPtr[2]);
                device->drive_info.scsiVersion = SCSI_VERSION_SPC_2;//changing to 4 for SPC2
                break;
            case 0x05://SPC-3
                printf("SPC3 (2005) - (%" PRIX8 "h)\n", inqPtr[2]);
                break;
            case 0x06://SPC-4
                printf("SPC4 (2015) - (%" PRIX8 "h)\n", inqPtr[2]);
                break;
            case 0x07://SPC-5
                printf("SPC5 (2019) - (%" PRIX8 "h)\n", inqPtr[2]);
                break;
            default:
                //convert some versions since old standards broke the version number into ANSI vs ECMA vs ISO standard numbers
                if ((version >= 0x08 && version <= 0x0C) ||
                    (version >= 0x40 && version <= 0x44) ||
                    (version >= 0x48 && version <= 0x4C) ||
                    (version >= 0x80 && version <= 0x84) ||
                    (version >= 0x88 && version <= 0x8C))
                {
                    printf("Unknown or Obsolete version format - (%" PRIX8 "h)\n", inqPtr[2]);
                    //these are obsolete version numbers
                    device->drive_info.scsiVersion = M_GETBITRANGE(version, 3, 0);
                    printf("Treating as - (%" PRIX8 "h) for legacy device compatibility since version is likely ANSI+ECMA+ISO format\n", device->drive_info.scsiVersion);
                }
                else
                {
                    printf("Unknown version - (%" PRIX8 "h)\n", inqPtr[2]);
                }
                break;
            }

            //Check response data format. This should be 2h. If not, then this is a device from before SCSI2 which means nothing else is parsable. 
            //If below 2, report a strong warning that the fields below are all vendor unique and may not be parsable. Ask if this product is older than 1993??? CCS was released in 1986....
            uint8_t responseDataFormat = M_Nibble0(inqPtr[3]);
            uint8_t totalInqLength = inqPtr[4] + 4;
            bool satVersionDescriptorFound = false;
            if (responseDataFormat < 2)
            {
                printf("WARNING: Response data format is set to %" PRIu8 "!\n", responseDataFormat);
                printf("         This means that all product identification is in a vendor specific\n");
                printf("         format and cannot be parsed correctly!\n");
                printf("         This is only expected for OLD SCSI devices. Nothing else should\n");
                printf("         report in this format. Raw output is provided which may be usable\n");
                printf("         if needed for better legacy device support.\n");
                print_Data_Buffer(inqPtr, totalInqLength, true);
            }
            else
            {
                //Get and print T10 vendor - print warning if empty?
                memcpy(vendorId, &inqPtr[8], T10_VENDOR_ID_LEN);
                //Check for printable and non-ASCII characters to warn that these are not supposed to be here!
                for (uint8_t iter = 0; iter < T10_VENDOR_ID_LEN; ++iter)
                {
                    if (!is_ASCII(vendorId[iter]) || !isprint(vendorId[iter]))
                    {
                        vendorId[iter] = ' ';
                        printf("WARNING: Vendor ID contains non-ASCII or non-Printable Characters!\n");
                    }
                }
                memcpy(device->drive_info.T10_vendor_ident, vendorId, T10_VENDOR_ID_LEN);
                if (strlen(vendorId) == 0)
                {
                    printf("WARNING: Vendor ID is empty!\n");
                }
                else
                {
                    printf("Got Vendor ID as %s\n", vendorId);
                }
                //Get and print Product ID - print warning if empty?
                memcpy(productId, &inqPtr[16], INQ_DATA_PRODUCT_ID_LEN);
                //Check for printable and non-ASCII characters to warn that these are not supposed to be here!
                for (uint8_t iter = 0; iter < INQ_DATA_PRODUCT_ID_LEN; ++iter)
                {
                    if (!is_ASCII(productId[iter]) || !isprint(productId[iter]))
                    {
                        productId[iter] = ' ';
                        printf("WARNING: Product ID contains non-ASCII or non-Printable Characters!\n");
                    }
                }
                memcpy(device->drive_info.product_identification, productId, INQ_DATA_PRODUCT_ID_LEN);
                if (strlen(productId) == 0)
                {
                    printf("WARNING: Product ID is empty!\n");
                }
                else
                {
                    printf("Got Product ID as %s\n", productId);
                }
                //get and print product revision - print warning if empty?
                memcpy(productRev, &inqPtr[32], INQ_DATA_PRODUCT_REV_LEN);
                //Check for printable and non-ASCII characters to warn that these are not supposed to be here!
                for (uint8_t iter = 0; iter < INQ_DATA_PRODUCT_REV_LEN; ++iter)
                {
                    if (!is_ASCII(productRev[iter]) || !isprint(productRev[iter]))
                    {
                        productRev[iter] = ' ';
                        printf("WARNING: Product Revision contains non-ASCII or non-Printable Characters!\n");
                    }
                }
                memcpy(device->drive_info.product_revision, productRev, INQ_DATA_PRODUCT_REV_LEN);
                if (strlen(productRev) == 0)
                {
                    printf("WARNING: Product Revision is empty!\n");
                }
                else
                {
                    printf("Got Product Revision as %s\n", productRev);
                }
                //TODO: based on what was reported for these, check passthrough data later to see if the vendor/product were just concatenated together instead of following translation
                //NOTE: Many USB products report something completely different from the encapsulated drive to make it appear as a specific product in the system. This is not abnormal, but may be a light warning.

                //TODO: Check for version descriptors and show what they are
                if (totalInqLength > 36)//checking if inquire data is longer since it was only 36bytes in SCSI2
                {
                    if (totalInqLength > 58)
                    {
                        printf("Checking Version Descriptors:\n");
                        if (version >= 4)
                        {
                            //Version Descriptors 1-8 (SPC2 and up)
                            uint16_t versionDescriptor = 0;
                            uint8_t versionIter = 0;
                            for (; versionIter < 8; versionIter++)
                            {
                                versionDescriptor = 0;
                                versionDescriptor = M_BytesTo2ByteValue(device->drive_info.scsiVpdData.inquiryData[(versionIter * 2) + 58], device->drive_info.scsiVpdData.inquiryData[(versionIter * 2) + 59]);
                                if (versionDescriptor > 0)
                                {
                                    char versionString[20] = { 0 };
                                    printf("\t%" PRIX16 " - ", versionDescriptor);
                                    decypher_SCSI_Version_Descriptors(versionDescriptor, (char*)versionString);
                                    printf("%s\n", versionString);
                                    //TODO: Note when finding SAT or USB, etc to and how it's helpful to figure out what a device supports and how to discover it.
                                }
                                if (is_Standard_Supported(versionDescriptor, STANDARD_CODE_SAT)
                                    || is_Standard_Supported(versionDescriptor, STANDARD_CODE_SAT2)
                                    || is_Standard_Supported(versionDescriptor, STANDARD_CODE_SAT3)
                                    || is_Standard_Supported(versionDescriptor, STANDARD_CODE_SAT4)
                                    //Next version descriptors aren't sat but should only appear on a SAT interface...at least we know they are ATA/ATAPI so it won't hurt to try issuing a command to the drive.
                                    || is_Standard_Supported(versionDescriptor, STANDARD_CODE_ATA_ATAPI6)
                                    || is_Standard_Supported(versionDescriptor, STANDARD_CODE_ATA_ATAPI7)
                                    || is_Standard_Supported(versionDescriptor, STANDARD_CODE_ATA_ATAPI8)
                                    || is_Standard_Supported(versionDescriptor, STANDARD_CODE_ACSx)
                                    )
                                {
                                    //SAT or ATA version found. Should definitely check for SAT VPD page.
                                    satVersionDescriptorFound = true;
                                }
                            }
                        }
                        else
                        {
                            printf("\tThis device reports SPC compliance which is prior to vesrion descriptors being added.\n");
                        }
                    }
                }
                else
                {
                    printf("NOTE: Inquiry data is 36 bytes or less but reports format 2 which should be 96\n");
                    printf("      bytes. In these bytes version descriptors will be reported and can be used\n");
                    printf("      to help better understand device capabilities (starting with SPC2)\n\n");
                }

            }
            //Read capacity. Start with 10, then do 16. Emit warnings when 16 doesn't work and the scsi versions is greater than SPC2
            //If read capacity 10 comes back saying UINT32_MAX for capacity, warn that this mismatches the SCSI version reported.
            printf("Getting Read Capacity data. 10 & 16 byte\n");
            uint8_t readCapacityData[32] = { 0 };
            uint32_t rc10MaxLBA = 0;
            uint32_t rc10BlockLen = 0;
            int readCap10Result = SUCCESS, readCap16Result = SUCCESS;
            if (SUCCESS == (readCap10Result = scsi_Read_Capacity_10(device, readCapacityData, 8)))
            {
                rc10MaxLBA = M_BytesTo4ByteValue(readCapacityData[0], readCapacityData[1], readCapacityData[2], readCapacityData[3]);
                rc10BlockLen = M_BytesTo4ByteValue(readCapacityData[4], readCapacityData[5], readCapacityData[6], readCapacityData[7]);
                printf("\tRead Capacity 10 data:\n");
                printf("\t\tMaxLBA: %" PRIu32 "\n", rc10MaxLBA);
                printf("\t\tBlock Size : %" PRIu32 "\n", rc10BlockLen);
            }
            else
            {
                printf("WARNING: Failed read capacity 10. While this command is superceeded by read capacity 16,\n");
                printf("         supporting it helps legacy system support and software. If not supported, it should\n");
                printf("         at least fail gracefully and report \"Invalid Operation Code\"\n");
            }
            memset(readCapacityData, 0, 32);
            uint64_t rc16MaxLBA = 0;
            uint32_t rc16BlockLen = 0;
            uint64_t rc16PhyBlockLen = 0;
            if (SUCCESS == (readCap16Result = scsi_Read_Capacity_16(device, readCapacityData, 32)))
            {
                rc16MaxLBA = M_BytesTo8ByteValue(readCapacityData[0], readCapacityData[1], readCapacityData[2], readCapacityData[3], readCapacityData[4], readCapacityData[5], readCapacityData[6], readCapacityData[7]);
                rc16BlockLen = M_BytesTo4ByteValue(readCapacityData[8], readCapacityData[9], readCapacityData[10], readCapacityData[11]);
                rc16PhyBlockLen = power_Of_Two(M_Nibble0(readCapacityData[13])) * rc16BlockLen;
                printf("\tRead Capacity 16 data:\n");
                printf("\t\tMaxLBA: %" PRIu64 "\n", rc16MaxLBA);
                printf("\t\tLogical Block Size: %" PRIu32 "\n", rc16BlockLen);
                printf("\t\tPhysical Block Size: %" PRIu64 "\n", rc16PhyBlockLen);
            }
            else
            {
                printf("WARNING: Failed read capacity 16. This should only happen on old legacy SPC & earlier devices.\n");
                printf("         All new devices SHOULD support this command to report PI type (if any) and logical\n");
                printf("         to physical block relationships so that read/write can be properly aligned for the device.\n");
                printf("         this command also reports if logical block provisioning management is enabled and whether\n");
                printf("         or not zeros are reported when reading an unmapped LBA.\n");
            }

            if (readCap16Result == SUCCESS && readCap10Result == SUCCESS)
            {
                if (rc16BlockLen != rc10BlockLen)
                {
                    printf("ERROR: Block length does not match between read capacity commands!\n");
                }
                if (rc10MaxLBA != UINT32_MAX && rc10MaxLBA != rc16MaxLBA)
                {
                    printf("ERROR: Max LBA does not match between read capacity commands!\n");
                }
            }

            printf("Checking VPD page support.\n");

            uint8_t supportedPages[36] = { 0 };
            bool dummiedPages = false;
            if (SUCCESS != scsi_Inquiry(device, supportedPages, 30, SUPPORTED_VPD_PAGES, true, false))
            {
                dummiedPages = true;
                printf("WARNING: Device did not report supported VPD pages.\n");
                printf("         VPD pages communicate other device capabilities\n");
                printf("         and limits which are useful to the host.\n");
                printf("Will dummy up support to see if any pages are supported that are useful.\n");
                memset(supportedPages, 0, 36);
                uint16_t offset = 4;//start of pages to dummy up
                //in here we will set up a fake supported VPD pages buffer so that we try to read the unit serial number page, the SAT page, and device identification page
                supportedPages[0] = 0 << 5;//todo put the proper qualifier in here!
                supportedPages[0] |= peripheralDeviceType;
                //set page code
                supportedPages[1] = 0x00;
                //Don't dummy up this page since it already didn't work
//              supportedPages[offset] = SUPPORTED_VPD_PAGES;
//              ++offset;
                supportedPages[offset] = UNIT_SERIAL_NUMBER;
                ++offset;
                //if (version >= 3)//SPC
                {
                    supportedPages[offset] = DEVICE_IDENTIFICATION;
                    ++offset;
                }
                supportedPages[offset] = EXTENDED_INQUIRY_DATA;
                ++offset;
                if (satVersionDescriptorFound)
                {
                    supportedPages[offset] = ATA_INFORMATION;
                    ++offset;
                }
                //if (version >= 3)//SPC
                {
                    if (peripheralDeviceType == PERIPHERAL_DIRECT_ACCESS_BLOCK_DEVICE || peripheralDeviceType == PERIPHERAL_SIMPLIFIED_DIRECT_ACCESS_DEVICE || peripheralDeviceType == PERIPHERAL_HOST_MANAGED_ZONED_BLOCK_DEVICE)
                    {
                        supportedPages[offset] = BLOCK_LIMITS;
                        ++offset;
                        supportedPages[offset] = BLOCK_DEVICE_CHARACTERISTICS;
                        ++offset;
                    }
                }
                //TODO: dummy up other page support here
                //set page length (n-3)
                supportedPages[2] = M_Byte1(offset - 4);//msb
                supportedPages[3] = M_Byte0(offset - 4);//lsb
            }
            //Now attempt to read VPD pages. - Warn about any missing MANDATORY pages
            //definitely strong warning if SAT VPD page is missing when it was expected. This warning should be emitted later if we do SAT passthrough testing as well...try to trust what the device does report
            //If supported VPD pages is missing, attempt to read unit serial number and device identification and block limits pages anyways...report warnings if ANY of these work, but supported pages was missing.
            uint16_t supportedVPDPagesLength = M_BytesTo2ByteValue(supportedPages[2], supportedPages[3]);
            //TODO: validate peripheral qualifier and peripheral device type on every page with std inquiry data
            for (vpdIter = 0; vpdIter < supportedVPDPagesLength && vpdIter < INQ_RETURN_DATA_LENGTH; vpdIter++)
            {
                bool genericVPDPageReadOutput = true;
                uint8_t *pageToRead = (uint8_t*)calloc_aligned(4, sizeof(uint8_t), device->os_info.minimumAlignment));
                printf("\tFound page ");
                switch (supportedPages[vpdIter])
                {
                case SUPPORTED_VPD_PAGES:
                    printf("%" PRIX8 "h - Supported VPD Pages\n", supportedPages[vpdIter]);
                    break;
                case UNIT_SERIAL_NUMBER:
                    printf("%" PRIX8 "h - Unit Serial Number VPD Page\n", supportedPages[vpdIter]);
                    if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
                    {
                        uint16_t vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
                        if (temp)
                        {
                            pageToRead = temp;
                            if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                            {
                                genericVPDPageReadOutput = false;
                                vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                                //TODO: Also validate the peripheral qualifier and peripheral device type???
                                if (pageToRead[1] != supportedPages[vpdIter])
                                {
                                    printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                                    print_Data_Buffer(pageToRead, vpdPageLength, true);
                                }
                                else
                                {
                                    unitSerialNumber = (char*)calloc(vpdPageLength + 1, sizeof(char));//add 1 for NULL terminator
                                    if (unitSerialNumber)
                                    {
                                        memcpy(unitSerialNumber, &pageToRead[4], vpdPageLength);
                                        for (uint8_t iter = 0; iter < vpdPageLength; ++iter)
                                        {
                                            if (!is_ASCII(unitSerialNumber[iter]) || !isprint(unitSerialNumber[iter]))
                                            {
                                                unitSerialNumber[iter] = ' ';
                                                printf("WARNING: Unit Serial Number contains non-ASCII or non-Printable Characters!\n");
                                            }
                                        }
                                        if (strlen(unitSerialNumber) == 0)
                                        {
                                            printf("WARNING: Unit Serial Number is empty!\n");
                                        }
                                        else
                                        {
                                            printf("\tGot Unit Serial Number as %s\n", unitSerialNumber);
                                        }
                                    }
                                }
                            }
                            else if (!dummiedPages)
                            {
                                printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                                printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                            }
                        }
                        else
                        {
                            printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                        }
                    }
                    else if (!dummiedPages)
                    {
                        printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                        printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
                    }
                    break;
                case IMPLEMENTED_OPERATING_DEFINITIONS:
                    printf("%" PRIX8 "h - Implemented Operating Definitions VPD Page\n", supportedPages[vpdIter]);
                    break;
                case ASCII_IMPLEMENTED_OPERATING_DEFINITION:
                    printf("%" PRIX8 "h - ASCII Implemented Operation Definition VPD Page\n", supportedPages[vpdIter]);
                    break;
                case DEVICE_IDENTIFICATION:
                    printf("%" PRIX8 "h - Device Identification VPD Page\n", supportedPages[vpdIter]);
                    if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
                    {
                        uint16_t vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
                        if (temp)
                        {
                            pageToRead = temp;
                            if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                            {
                                genericVPDPageReadOutput = false;
                                vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                                //TODO: Also validate the peripheral qualifier and peripheral device type???
                                if (pageToRead[1] != supportedPages[vpdIter])
                                {
                                    printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                                    print_Data_Buffer(pageToRead, vpdPageLength, true);
                                }
                                else
                                {
                                    printf("\tReported Designators:\n");
                                    uint16_t counter = 0;
                                    uint16_t designatorLength = 4;//header length and will be update with each one we look at.
                                    bool gotEUI64 = false, gotNAA = false, gotSCSINameString = false, gotUUID = false, gotMD5 = false;
                                    uint16_t md5Offset = 0;
                                    for (uint16_t offset = 4; offset < vpdPageLength; offset += designatorLength + 4, ++counter)
                                    {
                                        uint8_t protocolIdentifier = M_Nibble1(pageToRead[offset]);
                                        uint8_t codeSet = M_Nibble0(pageToRead[offset]);
                                        bool piv = pageToRead[offset + 1] & BIT7 ? true : false;
                                        uint8_t association = M_GETBITRANGE(pageToRead[offset + 1], 5, 4);
                                        uint8_t designatorType = M_Nibble0(pageToRead[offset + 1]);
                                        uint16_t designatorOffset = offset + 4;
                                        designatorLength = pageToRead[offset + 3];
                                        bool isASCII = false;
                                        bool isUTF8 = false;
                                        switch (codeSet)
                                        {
                                        case 1://binary values
                                               //break;
                                        case 2://ASCII printable characters
                                           isASCII = true;
                                           break;
                                        case 3://UTF-8 codes
                                           isUTF8 = true;
                                           break;
                                        case 0://reserved
                                        default://reserved
                                            break;
                                        }
                                        printf("\t\tDesignator %" PRIu16 "\n", counter);
                                        switch (association)
                                        {
                                        case 0://associated with addressed logical unit
                                            printf("\t\t\tAssociated with the addressed logical unit\n");
                                            break;
                                        case 1://associated with the target port that received the command
                                               //check PIV & protocol identifier
                                            printf("\t\t\tAssociated with the target port that received the command\n");
                                            if (piv)
                                            {
                                                switch (protocolIdentifier)
                                                {
                                                case 0:
                                                    printf("\t\t\t    Fibre Channel Protocol for SCSI\n");
                                                    break;
                                                case 1:
                                                    printf("\t\t\t    Parallel SCSI\n");
                                                    break;
                                                case 2:
                                                    printf("\t\t\t    Serial Storage Architecture SCSI-3 Protocol\n");
                                                    break;
                                                case 3:
                                                    printf("\t\t\t    Serial Bus Protocol for IEEE 1394\n");
                                                    break;
                                                case 4:
                                                    printf("\t\t\t    SCSI RDMA Protocol\n");
                                                    break;
                                                case 5:
                                                    printf("\t\t\t    Internet SCSI (iSCSI)\n");
                                                    break;
                                                case 6:
                                                    printf("\t\t\t    SAS Serial SCSI Protocol\n");
                                                    break;
                                                case 7:
                                                    printf("\t\t\t    Automation/Drive Interface Transport Protocol\n");
                                                    break;
                                                case 8:
                                                    printf("\t\t\t    AT Attachment Interface\n");
                                                    break;
                                                case 9:
                                                    printf("\t\t\t    USB Attached SCSI\n");
                                                    break;
                                                case 10:
                                                    printf("\t\t\t    SCSI over PCI Express\n");
                                                    break;
                                                case 0x0f:
                                                    printf("\t\t\t    No specific Protocol\n");
                                                    break;
                                                default:
                                                    printf("\t\t\t    Reserved protocol ID value %" PRIX8 "h\n", protocolIdentifier);
                                                    break;
                                                }
                                            }
                                            break;
                                        case 2://associated with the SCSI target device that contains the addressed logical unit
                                               //check PIV & protocol identifier
                                            printf("\t\t\tAssociated with the SCSI target device that contains the addressed logical unit\n");
                                            if (piv)
                                            {
                                                switch (protocolIdentifier)
                                                {
                                                case 0:
                                                    printf("\t\t\t    Fibre Channel Protocol for SCSI\n");
                                                    break;
                                                case 1:
                                                    printf("\t\t\t    Parallel SCSI\n");
                                                    break;
                                                case 2:
                                                    printf("\t\t\t    Serial Storage Architecture SCSI-3 Protocol\n");
                                                    break;
                                                case 3:
                                                    printf("\t\t\t    Serial Bus Protocol for IEEE 1394\n");
                                                    break;
                                                case 4:
                                                    printf("\t\t\t    SCSI RDMA Protocol\n");
                                                    break;
                                                case 5:
                                                    printf("\t\t\t    Internet SCSI (iSCSI)\n");
                                                    break;
                                                case 6:
                                                    printf("\t\t\t    SAS Serial SCSI Protocol\n");
                                                    break;
                                                case 7:
                                                    printf("\t\t\t    Automation/Drive Interface Transport Protocol\n");
                                                    break;
                                                case 8:
                                                    printf("\t\t\t    AT Attachment Interface\n");
                                                    break;
                                                case 9:
                                                    printf("\t\t\t    USB Attached SCSI\n");
                                                    break;
                                                case 10:
                                                    printf("\t\t\t    SCSI over PCI Express\n");
                                                    break;
                                                case 0x0f:
                                                    printf("\t\t\t    No specific Protocol\n");
                                                    break;
                                                default:
                                                    printf("\t\t\t    Reserved protocol ID value %" PRIX8 "h\n", protocolIdentifier);
                                                    break;
                                                }
                                            }
                                            break;
                                        case 3://reserved
                                            printf("\t\t\tWARNING: Reserved association\n");
                                            break;
                                        }
                                        switch (designatorType)
                                        {
                                        case 0://vendor specific
                                            printf("\t\t\tVendor Specific Designator:\n");
                                            print_Data_Buffer(&pageToRead[offset], designatorLength, true);
                                            break;
                                        case 1://T10 vendor ID based
                                            printf("\t\t\tT10 Vendor ID Based Designator:\n");
                                            if (!isASCII)
                                            {
                                                printf("WARNING: Code set does not mark this as ASCII! This should be ASCII data!!!\n");
                                            }
                                            {
                                                char desVendorID[9] = { 0 };
                                                char *vendorSpecificID = NULL;
                                                memcpy(desVendorID, &pageToRead[designatorOffset], 8);
                                                printf("\t\t\t    T10 Vendor ID: %s\n", desVendorID);
                                                if (designatorLength > 8)
                                                {
                                                    vendorSpecificID = (char*)calloc(designatorLength - 8 + 1, sizeof(char));
                                                    if (vendorSpecificID)
                                                    {
                                                        memcpy(vendorSpecificID, &pageToRead[designatorOffset + 8], designatorLength - 8);
                                                        //TODO: validate that all characters are printable
                                                        printf("\t\t\t    Vendor Specific ID: %s\n", vendorSpecificID);
                                                        safe_Free(vendorSpecificID);
                                                    }
                                                }
                                                else
                                                {
                                                    printf("WARNING: T10 Vendor ID based designator is missing vendor specific identifier!!!\n");
                                                    printf("Recommended method from SPC is to concatenate Product ID and Product Serial number\n");
                                                }
                                            }
                                            break;
                                        case 2://EUI-64 based
                                            gotEUI64 = true;
                                            //TODO: May need to save this to compare for NVMe over USB
                                            printf("\t\t\tEUI-64 Designator:\n");
                                            //3 different formats depending on designator length
                                            switch (designatorLength)
                                            {
                                            case 0x08://EUI64
                                                printf("\t\t\t    EUI64\n");
                                                printf("\t\t\t    IEEE Company ID: ");
                                                for (uint16_t ieeeVIDOffset = designatorOffset; ieeeVIDOffset < (designatorOffset + 3); ++ieeeVIDOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[ieeeVIDOffset]);
                                                }
                                                printf("\n");
                                                printf("\t\t\t    Vendor Specific Extension Identifier: ");
                                                for (uint16_t vuExtIDOffset = designatorOffset + 3; vuExtIDOffset < (designatorOffset + 8); ++vuExtIDOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[vuExtIDOffset]);
                                                }
                                                printf("\n");
                                                break;
                                            case 0x0C://EUI64 - 12 byte
                                                printf("\t\t\t    EUI64 - 12 Byte\n");
                                                printf("\t\t\t    IEEE Company ID: ");
                                                for (uint16_t ieeeVIDOffset = designatorOffset; ieeeVIDOffset < (designatorOffset + 3); ++ieeeVIDOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[ieeeVIDOffset]);
                                                }
                                                printf("\n");
                                                printf("\t\t\t    Vendor Specific Extension Identifier: ");
                                                for (uint16_t vuExtIDOffset = designatorOffset + 3; vuExtIDOffset < (designatorOffset + 8); ++vuExtIDOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[vuExtIDOffset]);
                                                }
                                                printf("\n");
                                                printf("\t\t\t    Directory ID: ");
                                                for (uint16_t dIDOffset = designatorOffset + 8; dIDOffset < (designatorOffset + 12); ++dIDOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[dIDOffset]);
                                                }
                                                printf("\n");
                                                break;
                                            case 0x10://EUI64 - 16 byte
                                                printf("\t\t\t    EUI64 - 16 Byte\n");
                                                printf("\t\t\t    Identifier Extension: ");
                                                for (uint16_t idExtOffset = designatorOffset; idExtOffset < (designatorOffset + 8); ++idExtOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[idExtOffset]);
                                                }
                                                printf("\n");
                                                printf("\t\t\t    IEEE Company ID: ");
                                                for (uint16_t ieeeVIDOffset = designatorOffset + 8; ieeeVIDOffset < (designatorOffset + 11); ++ieeeVIDOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[ieeeVIDOffset]);
                                                }
                                                printf("\n");
                                                printf("\t\t\t    Vendor Specific Extension Identifier: ");
                                                for (uint16_t vuExtIDOffset = designatorOffset + 11; vuExtIDOffset < (designatorOffset + 16); ++vuExtIDOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[vuExtIDOffset]);
                                                }
                                                printf("\n");
                                                break;
                                            default://unknown EUI64 format!
                                                printf("\t\t\t    Unknown EUI64 designator length!\n");
                                                printf("\t\t\t    Unknown data format: ");
                                                //print it out in hex
                                                for (uint16_t euiOffset = designatorOffset; euiOffset < (designatorOffset + designatorLength); ++euiOffset)
                                                {
                                                    printf("%02" PRIX8 , pageToRead[euiOffset]);
                                                }
                                                printf("\n");
                                                break;
                                            }
                                            break;
                                        case 3://NAA
                                            gotNAA = true;
                                            //TODO: May need to save this to compare for SATA over USB
                                            printf("\t\t\tNetwork Address Authority Designator:\n");
                                            //NAA value will be either 2, 4, 5, or 6 which changes length and format
                                            {
                                                uint8_t naa = M_Nibble1(pageToRead[designatorOffset]);
                                                switch (naa)
                                                {
                                                case 2://IEEE Extended - binary only
                                                    printf("\t\t\t    NAA Type 2 - IEEE Extended\n");
                                                    //vendor specific Identifier A
                                                    printf("\t\t\t        Vendor Specific ID A: %01" PRIX8 "%02" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset]), pageToRead[designatorOffset + 1]);
                                                    //IEEE Company ID
                                                    printf("\t\t\t        IEEE Company ID: %02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", pageToRead[designatorOffset + 2], pageToRead[designatorOffset + 3], pageToRead[designatorOffset + 4]);
                                                    //vendor specific Identifier B
                                                    printf("\t\t\t        Vendor Specific ID B: %02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", pageToRead[designatorOffset + 5], pageToRead[designatorOffset + 6], pageToRead[designatorOffset + 7]);
                                                    break;
                                                case 3://Locally Assigned - binary only
                                                    printf("\t\t\t    NAA Type 3 - Locally Assigned\n");
                                                    //Locally administered value
                                                    printf("\t\t\t        Locally Administered Value: %01" PRIX8 , M_Nibble0(pageToRead[designatorOffset]));//print the first nibble, the rest is all individual bytes
                                                    for (uint16_t lavOffset = designatorOffset + 1; lavOffset < (designatorLength + 4); ++lavOffset)
                                                    {
                                                        printf("%02" PRIX8 , pageToRead[lavOffset]);
                                                    }
                                                    printf("\n");
                                                    break;
                                                case 5://IEEE Registered - binary only
                                                    printf("\t\t\t    NAA Type 5 - IEEE Registered\n");
                                                    //IEEE Company ID
                                                    printf("\t\t\t        IEEE Company ID: %01" PRIX8 "%02" PRIX8 "%02" PRIX8 "%01" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset]), pageToRead[designatorOffset + 1], pageToRead[designatorOffset + 2], M_Nibble1(pageToRead[designatorOffset + 3]));
                                                    //Vendor Specific Identifier
                                                    printf("\t\t\t        Vendor Specific ID: %01" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset + 3]), pageToRead[designatorOffset + 4], pageToRead[designatorOffset + 5], pageToRead[designatorOffset + 6], pageToRead[designatorOffset + 7]);
                                                    break;
                                                case 6://IEEE Registered Extended - binary only
                                                    printf("\t\t\t    NAA Type 6 - IEEE Registered Extended\n");
                                                    //IEEE Company ID
                                                    printf("\t\t\t        IEEE Company ID: %01" PRIX8 "%02" PRIX8 "%02" PRIX8 "%01" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset]), pageToRead[designatorOffset + 1], pageToRead[designatorOffset + 2], M_Nibble1(pageToRead[designatorOffset + 3]));
                                                    //Vendor Specific Identifier
                                                    printf("\t\t\t        Vendor Specific ID: %01" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset + 3]), pageToRead[designatorOffset + 4], pageToRead[designatorOffset + 5], pageToRead[designatorOffset + 6], pageToRead[designatorOffset + 7]);
                                                    //Vendor Specific Identifier Extentsion
                                                    printf("\t\t\t        Vendor Specific ID Extension: %02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", pageToRead[designatorOffset + 8], pageToRead[designatorOffset + 9], pageToRead[designatorOffset + 10], pageToRead[designatorOffset + 11], pageToRead[designatorOffset + 12], pageToRead[designatorOffset + 12], pageToRead[designatorOffset + 13], pageToRead[designatorOffset + 14]pageToRead[designatorOffset + 15]);
                                                    break;
                                                default://unknown NAA Type
                                                    printf("\t\t\t    Unknown NAA Type %" PRIX8 "h\n", naa);
                                                    printf("\t\t\t        Value: ");
                                                    for (uint16_t unaaOffset = designatorOffset; unaaOffset < (designatorLength + 4); ++unaaOffset)
                                                    {
                                                        if (codeSet == 2 || codeSet == 3)
                                                        {
                                                            //ASCII or UTF8...this should be OK, but we could get a unprintable UTF8 character this way!
                                                            printf("%c", pageToRead[unaaOffset]);
                                                        }
                                                        else
                                                        {
                                                            printf("%02" PRIX8 , pageToRead[unaaOffset]);
                                                        }
                                                    }
                                                    printf("\n");
                                                    break;
                                                }
                                            }
                                            break;
                                        case 4://relative target port identifier
                                            printf("\t\t\tRelative Target Port Identifier Designator:\n");
                                            if (isASCII || isUTF8)
                                            {
                                                printf("WARNING: Code set is not set properly! This should be set to 1 for binary!\n");
                                            }
                                            else if (association != 1)
                                            {
                                                printf("WARNING: Use of this designator with association not set to 1 is reserved!\n");
                                            }
                                            {
                                                uint16_t relativeTargetPortID = M_BytesTo2ByteValue(pageToRead[designatorOffset + 2], pageToRead[designatorOffset + 3]);
                                                printf("\t\t\t    Relative Target Port ID: %" PRIu16 "\n", relativeTargetPortID);
                                            }
                                            break;
                                        case 5://target port group
                                            printf("\t\t\tTarget Port Group Designator:\n");
                                            if (isASCII || isUTF8)
                                            {
                                                printf("WARNING: Code set is not set properly! This should be set to 1 for binary!\n");
                                            }
                                            else if (association != 1)
                                            {
                                                printf("WARNING: Use of this designator with association not set to 1 is reserved!\n");
                                            }
                                            {
                                                uint16_t targetPortGroup = M_BytesTo2ByteValue(pageToRead[designatorOffset + 2], pageToRead[designatorOffset + 3]);
                                                printf("\t\t\t    Target Port Group: %" PRIu16 "\n", targetPortGroup);
                                            }
                                            break;
                                        case 6://logical unit group
                                            printf("\t\t\tLogical Unit Group Designator:\n");
                                            if (isASCII || isUTF8)
                                            {
                                                printf("WARNING: Code set is not set properly! This should be set to 1 for binary!\n");
                                            }
                                            else if (association != 0)
                                            {
                                                printf("WARNING: Use of this designator with association not set to 0 is reserved!\n");
                                            }
                                            {
                                                uint16_t luGroup = M_BytesTo2ByteValue(pageToRead[designatorOffset + 2], pageToRead[designatorOffset + 3]);
                                                printf("\t\t\t    Logical Unit Group: %" PRIu16 "\n", luGroup);
                                            }
                                            break;
                                        case 7://MD5 logical unit identifier
                                            gotMD5 = true;
                                            //This should basically never be used at this point as all the other unique identifiers should be used instead. This
                                            //should be handled AFTER checking all other designators in order to warn if it is in use while other unique designators are 
                                            //present as SPC defines. 
                                            md5Offset = designatorOffset;
                                            break;
                                        case 8://SCSI name string
                                            gotSCSINameString = true;
                                            printf("\t\t\tSCSI Name String Designator:\n");
                                            if (!isUTF8)
                                            {
                                                printf("WARNING: Code set is not properly set! This should be set to 3 for UTF8!\n");
                                            }
                                            {
                                                char *scsiNameString = NULL;
                                                scsiNameString = (char*)calloc(designatorLength + 1, sizeof(char));
                                                if (scsiNameString)
                                                {
                                                    memcpy(scsiNameString, &pageToRead[designatorOffset], designatorLength);
                                                    //TODO: validate that all characters are UTF8
                                                    printf("\t\t\t    SCSI Name: %s\n", scsiNameString);
                                                    safe_Free(scsiNameString);
                                                }
                                                else
                                                {
                                                    printf("ERROR: failed to allocate memory (%" PRIu16 "B) to read SCSI Name string\n", designatorLength + 1);
                                                }
                                            }
                                            break;
                                        case 9: //protocol specific port identifier
                                            printf("\t\t\tProtocol Specific Port Identifier Designator:\n");
                                            //this is either USB or PCIe information depending on the protocol identifier field
                                            if (association != 0x01)
                                            {
                                                printf("WARNING: Association is not set to 1! This may cause parsing issues!\n");
                                            }
                                            if (!piv)
                                            {
                                                //This should be an error since we can only parse this if the interface is set.
                                                printf("ERROR: Protocol identifier valid bit is not set! Cannot parse this without this bit!\n");
                                            }
                                            else
                                            {
                                                switch (protocolIdentifier)
                                                {
                                                case 9://UAS
                                                    {
                                                        uint8_t deviceAddress = M_GETBITRANGE(pageToRead[designatorOffset], 6, 0);
                                                        uint8_t interfaceNumber = pageToRead[designatorOffset + 2];
                                                        printf("\t\t\t    UAS Target Port Identifier:\n");
                                                        printf("\t\t\t        Device Address: %02" PRIX8 "h\n", deviceAddress);
                                                        printf("\t\t\t        Interface Number: %02" PRIX8 "h\n", interfaceNumber);
                                                    }
                                                    break;
                                                case 0xA://SCSI over PCIe
                                                    {
                                                        uint16_t routingID = M_BytesTo2ByteValue(pageToRead[designatorOffset], pageToRead[designatorOffset + 1]);
                                                        printf("\t\t\t    PCIe Routing Identifier:\n");
                                                        printf("\t\t\t        Routing ID: %04" PRIX16 "h\n", routingID);
                                                    }
                                                    break;
                                                default://unknown or not defined in SPC5 when this was written
                                                    printf("\t\t\t    Unknown Protocol Specific Port Identifier type!\n");
                                                    print_Data_Buffer(&pageToRead[designatorOffset], designatorLength, true);
                                                    break;
                                                }
                                            }
                                            break;
                                        case 0xA://UUID
                                            gotUUID = true;
                                            printf("\t\t\tUUID Designator:\n");
                                            {
                                                uint8_t uuidType = M_Nibble1(pageToRead[designatorOffset]);
                                                switch (uuidType)
                                                {
                                                case 1://locally assigned RFC 4122 UUID
                                                    printf("\t\t\t    Locally Assigned RFC 4122 UUID:\n");
                                                    printf("\t\t\t        ");
                                                    //print bytes 2 - 17
                                                    if (designatorLength < 18)
                                                    {
                                                        printf("WARNING: Designator length should be 18B, but got %" PRIu8 "B\n", designatorLength);
                                                    }
                                                    for (uint16_t uuidOffset = designatorOffset; uuidOffset < (designatorLength + 4); ++uuidOffset)
                                                    {
                                                        printf("%02" PRIX8 , pageToRead[uuidOffset]);
                                                    }
                                                    printf("\n");
                                                    break;
                                                default://reserved or unknown UUID type
                                                    printf("\t\t\t    Unknown UUID type!\n");
                                                    print_Data_Buffer(&pageToRead[designatorOffset], designatorLength, true);
                                                    break;
                                                }
                                            }
                                            break;
                                        default://unknown or reserved type
                                            printf("\t\t\tUnknown Designator Type - %" PRIX8 "h\n", designatorType);
                                            print_Data_Buffer(&pageToRead[designatorOffset], designatorLength, true);
                                            break;
                                        }
                                    }
                                    //checking if we got an MD5 when we shouldn't have!
                                    if (gotMD5 && (gotEUI64 || gotNAA || gotSCSINameString || gotUUID ))
                                    {
                                        printf("WARNING: Device reported MD5 designator while also reporting a\n");
                                        printf("         unique identifier as well. This is not allowed per SPC!\n");
                                    }
                                    else if (gotMD5 && md5Offset)
                                    {
                                        //TODO: Validate the MD5 output by computing it ourselves from other designators that were reported!
                                        //NOTE: Any designators not available are replaced with 8 space characters per spec as input to the MD5 computation
                                        printf("\t\t\tMD5 Logical Unit Designator:\n");
                                        printf("\t\t\t    MD5 Logical Unit Identifier: ");
                                        for (; md5Offset < 16; ++md5Offset)
                                        {
                                            printf("%02" PRIX8 , pageToRead[md5Offset]);
                                        }
                                        printf("\n");
                                    }
                                }
                            }
                            else if (!dummiedPages)
                            {
                                printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                                printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                            }
                        }
                        else
                        {
                            printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                        }
                    }
                    else if (!dummiedPages)
                    {
                        printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                        printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
                    }
                    break;
                case SOFTWARE_INTERFACE_IDENTIFICATION:
                    printf("%" PRIX8 "h - Software Interface Identification VPD Page\n", supportedPages[vpdIter]);
                    break;
                case MANAGEMENT_NETWORK_ADDRESSES:
                    printf("%" PRIX8 "h - Management Network Addresses VPD Page\n", supportedPages[vpdIter]);
                    break;
                case EXTENDED_INQUIRY_DATA://While this page is useful, it is only in SAT4 and only and only if read buffer command is supported. SAT5 adds in support for download microcode modes supported
                    printf("%" PRIX8 "h - Extended Inquiry Data VPD Page\n", supportedPages[vpdIter]);
                    //Thorough parsing of this page and outputting its information in a nice format
                    if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
                    {
                        uint16_t vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
                        if (temp)
                        {
                            pageToRead = temp;
                            if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                            {
                                genericVPDPageReadOutput = false;
                                vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                                //TODO: Also validate the peripheral qualifier and peripheral device type???
                                bool parsePage = true;
                                if (pageToRead[1] != supportedPages[vpdIter])
                                {
                                    printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                                    print_Data_Buffer(pageToRead, vpdPageLength, true);
                                    parsePage = false;
                                }
                                else if (vpdPageLength < 0x003C)
                                {
                                    printf("ERROR: VPD Page length is less than specified in SPC! Expected %" PRIX8 "h, but got %" PRIX8 "h\n", 0x003C, vpdPageLength);
                                }
                                if (parsePage)
                                {
                                    //Need to figure out which things are actually useful to show! Will start with things that are most useful
                                    uint16_t extDSTMinutes = M_BytesTo2ByteValue(pageToRead[10], pageToRead[11]);
                                    if (extDSTMinutes == 0)
                                    {
                                        printf("WARNING: Extended DST time in minutes was reported as zero! This means SCSI DST translation is not available!\n");
                                    }
                                    else
                                    {
                                        printf("\tExtended Self-Test Completion Time (minutes): %" PRIu16 "\n", extDSTMinutes);
                                    }
                                    if (pageToRead[13] == 0)
                                    {
                                        printf("WARNING: Maximum Sense Length not reported. Will assume %u Bytes\n", SPC3_SENSE_LEN);
                                    }
                                    else if (pageToRead[13] > SPC3_SENSE_LEN)
                                    {
                                        printf("WARNING: Maximum Sense Length reported as larger than max allowed by SPC! %u Bytes\n", pageToRead[13]);
                                    }
                                    else
                                    {
                                        printf("\tMaximum supported sense data length: %" PRIu8 " Bytes\n", pageToRead[13]);
                                    }
                                    if (pageToRead[12] & BIT4)
                                    {
                                        //Download Microcode support byte is available
                                        //TODO: Parse this out. Skipping for now since this is so new nothing will be using it yet.
                                    }
                                }
                            }
                            else if (!dummiedPages)
                            {
                                printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                                printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                            }
                        }
                        else
                        {
                            printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                        }
                    }
                    else if (!dummiedPages)
                    {
                        printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                        printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
                    }
                    break;
                case MODE_PAGE_POLICY:
                    printf("%" PRIX8 "h - Mode Page Policy VPD Page\n", supportedPages[vpdIter]);
                    break;
                case SCSI_PORTS:
                    printf("%" PRIX8 "h - SCSI Ports VPD Page\n", supportedPages[vpdIter]);
                    break;
                case ATA_INFORMATION:
                    printf("%" PRIX8 "h - ATA Information VPD Page\n", supportedPages[vpdIter]);
                    if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
                    {
                        uint16_t vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
                        if (temp)
                        {
                            pageToRead = temp;
                            if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                            {
                                genericVPDPageReadOutput = false;
                                vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                                //TODO: Also validate the peripheral qualifier and peripheral device type???
                                bool parsePage = true;
                                if (pageToRead[1] != supportedPages[vpdIter])
                                {
                                    printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                                    print_Data_Buffer(pageToRead, vpdPageLength, true);
                                    parsePage = false;
                                }
                                else if (vpdPageLength < 0x0238)
                                {
                                    printf("ERROR: VPD Page length is less than specified in SAT! Expected %" PRIX8 "h, but got %" PRIX8 "h\n", 0x0238, vpdPageLength);
                                }
                                if (parsePage)
                                {
                                    char satVendor[9] = { 0 };
                                    char satProductID[17] = { 0 };
                                    char satRevision[5] = { 0 };
                                    memcpy(satVendor, &pageToRead[8], 8);
                                    memcpy(satProductID, &pageToRead[16], 16);
                                    memcpy(satRevision, &pageToRead[32], 4);
                                    for (uint8_t iter = 0; iter < 8; ++iter)
                                    {
                                        if (!is_ASCII(satVendor[iter]) || !isprint(satVendor[iter]))
                                        {
                                            satVendor[iter] = ' ';
                                            printf("WARNING: SAT Vendor ID contains non-ASCII or non-Printable Characters!\n");
                                        }
                                    }
                                    memcpy(device->drive_info.bridge_info.t10SATvendorID, satVendor, 8);
                                    if (strlen(satVendor) == 0)
                                    {
                                        printf("WARNING: SAT Vendor ID is empty!\n");
                                    }
                                    else
                                    {
                                        printf("\tGot SAT Vendor ID as %s\n", satVendor);
                                    }
                                    for (uint8_t iter = 0; iter < 16; ++iter)
                                    {
                                        if (!is_ASCII(satProductID[iter]) || !isprint(satProductID[iter]))
                                        {
                                            satProductID[iter] = ' ';
                                            printf("WARNING: SAT Product ID contains non-ASCII or non-Printable Characters!\n");
                                        }
                                    }
                                    memcpy(device->drive_info.bridge_info.SATproductID, satProductID, 16);
                                    if (strlen(satProductID) == 0)
                                    {
                                        printf("WARNING: SAT Product ID is empty!\n");
                                    }
                                    else
                                    {
                                        printf("\tGot SAT Product ID as %s\n", satProductID);
                                    }
                                    for (uint8_t iter = 0; iter < 4; ++iter)
                                    {
                                        if (!is_ASCII(satRevision[iter]) || !isprint(satRevision[iter]))
                                        {
                                            satRevision[iter] = ' ';
                                            printf("WARNING: SAT Product Revision contains non-ASCII or non-Printable Characters!\n");
                                        }
                                    }
                                    memcpy(device->drive_info.bridge_info.SATfwRev, satRevision, 4);
                                    if (strlen(satRevision) == 0)
                                    {
                                        printf("WARNING: SAT Product Revision is empty!\n");
                                    }
                                    else
                                    {
                                        printf("\tGot SAT Product Revision as %s\n", satRevision);
                                    }
                                    //Device signature (starting at byte 36)
                                    uint16_t signatureOffset = 36;
                                    bool isSATA = false;
                                    printf("\tATA Device Signature:\n");
                                    switch (pageToRead[signatureOffset])
                                    {
                                    case 0://PATA
                                        printf("\t\tTransport: PATA\n");
                                        break;
                                    case 34://SATA
                                        printf("\t\tTransport: SATA\n");
                                        isSATA = true;
                                        break;
                                    default://unknown
                                        printf("\t\tTransport: Unknown (%02" PRIX8 "h)\n", pageToRead[signatureOffset]);
                                        break;
                                    }
                                    if (isSATA)
                                    {
                                        printf("\t\tInterrupt: %d\n", pageToRead[signatureOffset + 1] & BIT6 > 0 ? 1 : 0);
                                        printf("\t\tPM Port: %" PRIu8 "\n", M_Nibble0(pageToRead[signatureOffset + 1]));
                                    }
                                    printf("\t\tStatus:       %02"  PRIX8 "h\n", pageToRead[signatureOffset + 2]);
                                    printf("\t\tError:        %02"  PRIX8 "h\n", pageToRead[signatureOffset + 3]);
                                    printf("\t\tLBA (7:0):    %02"  PRIX8 "h\n", pageToRead[signatureOffset + 4]);
                                    printf("\t\tLBA (15:8):   %02"  PRIX8 "h\n", pageToRead[signatureOffset + 5]);
                                    printf("\t\tLBA (23:16):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 6]);
                                    printf("\t\tDevice:       %02"  PRIX8 "h\n", pageToRead[signatureOffset + 7]);
                                    printf("\t\tLBA (31:24):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 8]);
                                    printf("\t\tLBA (39:32):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 9]);
                                    printf("\t\tLBA (47:40):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 10]);
                                    printf("\t\tCount (7:0):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 12]);
                                    printf("\t\tCount (15:8): %02"  PRIX8 "h\n", pageToRead[signatureOffset + 13]);
                                    //command code
                                    printf("\tIdentify Command Code: ")
                                    switch (pageToRead[56])
                                    {
                                    case 0xEC://Identify
                                        printf("Identify (ECh)\n");
                                        break;
                                    case 0xA1://Identify Packet
                                        printf("Identify Packet (A1h)\n");
                                        break;
                                    case 0x2F://Read Log Ext
                                        printf("Read Log Ext (2Fh)\n");
                                        break;
                                    case 0x47://Read Log Ext DMA
                                        printf("Read Log Ext DMA (47h)\n");
                                        break;
                                    default://unknown
                                        printf("Unknown (%02" PRIX8 "h)\n", pageToRead[56]);
                                        break;
                                    }
                                    printf("\tIdentify Data:\n");
                                    print_Data_Buffer(&pageToRead[60], 512, true);
                                }
                            }
                            else if (!dummiedPages)
                            {
                                printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                                printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                            }
                        }
                        else
                        {
                            printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                        }
                    }
                    else if (!dummiedPages)
                    {
                        printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                        printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
                    }
                    break;
                case POWER_CONDITION:
                    printf("%" PRIX8 "h - Power Condition VPD Page\n", supportedPages[vpdIter]);
                    //TODO: Save power condition information to compare to what is reported from reading ATA device logs
                    if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
                    {
                        uint16_t vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
                        if (temp)
                        {
                            pageToRead = temp;
                            if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                            {
                                genericVPDPageReadOutput = false;
                                vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                                //TODO: Also validate the peripheral qualifier and peripheral device type???
                                if (pageToRead[1] != supportedPages[vpdIter])
                                {
                                    printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                                    print_Data_Buffer(pageToRead, vpdPageLength, true);
                                }
                                else if (vpdPageLength < 0x0E)
                                {
                                    printf("ERROR: VPD Page length is less than specified in SPC! Expected %" PRIX8 "h, but got %" PRIX8 "h\n", 0x0E, vpdPageLength);
                                }
                                else
                                {
                                    if (M_BytesTo2ByteValue(pageToRead[8], pageToRead[9]) == UINT16_MAX)
                                    {
                                        printf("\tStopped Condition Recovery Time: >65.534 milliseconds\n", M_BytesTo2ByteValue(pageToRead[8], pageToRead[9]));
                                    }
                                    else
                                    {
                                        printf("\tStopped Condition Recovery Time: %" PRIu16 " milliseconds\n", M_BytesTo2ByteValue(pageToRead[8], pageToRead[9]));
                                    }
                                    if (pageToRead[4] & BIT0)
                                    {
                                        printf("\tStandby Z Supported\n");
                                        printf("\t    Recovery Time: ");
                                        if (M_BytesTo2ByteValue(pageToRead[8], pageToRead[9]) == UINT16_MAX)
                                        {
                                            printf(">65.534 milliseconds\n");
                                        }
                                        else
                                        {
                                            printf("%" PRIu16 " milliseconds\n", M_BytesTo2ByteValue(pageToRead[8], pageToRead[9]));
                                        }
                                    }
                                    if (pageToRead[4] & BIT1)
                                    {
                                        printf("\tStandby Y Supported\n");
                                        if (M_BytesTo2ByteValue(pageToRead[10], pageToRead[11]) == UINT16_MAX)
                                        {
                                            printf(">65.534 milliseconds\n");
                                        }
                                        else
                                        {
                                            printf("%" PRIu16 " milliseconds\n", M_BytesTo2ByteValue(pageToRead[10], pageToRead[11]));
                                        }
                                    }
                                    if (pageToRead[54] & BIT0)
                                    {
                                        printf("\tIdle A Supported\n");
                                        if (M_BytesTo2ByteValue(pageToRead[12], pageToRead[13]) == UINT16_MAX)
                                        {
                                            printf(">65.534 milliseconds\n");
                                        }
                                        else
                                        {
                                            printf("%" PRIu16 " milliseconds\n", M_BytesTo2ByteValue(pageToRead[12], pageToRead[13]));
                                        }
                                    }
                                    if (pageToRead[54] & BIT1)
                                    {
                                        printf("\tIdle B Supported\n");
                                        if (M_BytesTo2ByteValue(pageToRead[14], pageToRead[15]) == UINT16_MAX)
                                        {
                                            printf(">65.534 milliseconds\n");
                                        }
                                        else
                                        {
                                            printf("%" PRIu16 " milliseconds\n", M_BytesTo2ByteValue(pageToRead[14], pageToRead[15]));
                                        }
                                    }
                                    if (pageToRead[54] & BIT2)
                                    {
                                        printf("\tIdle C Supported\n");
                                        if (M_BytesTo2ByteValue(pageToRead[16], pageToRead[17]) == UINT16_MAX)
                                        {
                                            printf(">65.534 milliseconds\n");
                                        }
                                        else
                                        {
                                            printf("%" PRIu16 " milliseconds\n", M_BytesTo2ByteValue(pageToRead[16], pageToRead[17]));
                                        }
                                    }
                                }
                            }
                            else if (!dummiedPages)
                            {
                                printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                                printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                            }
                        }
                        else
                        {
                            printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                        }
                    }
                    else if (!dummiedPages)
                    {
                        printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                        printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
                    }
                    break;
                case DEVICE_CONSTITUENTS:
                    printf("%" PRIX8 "h - Device Constituents VPD Page\n", supportedPages[vpdIter]);
                    break;
                case CFA_PROFILE_INFORMATION:
                    printf("%" PRIX8 "h - CFA Profile Information VPD Page\n", supportedPages[vpdIter]);
                    break;
                case POWER_CONSUMPTION:
                    printf("%" PRIX8 "h - Power Consumption VPD Page\n", supportedPages[vpdIter]);
                    break;
                case THIRD_PARTY_COPY:
                    printf("%" PRIX8 "h - Third Party Copy VPD Page\n", supportedPages[vpdIter]);
                    break;
                case PROTOCOL_SPECIFIC_LU_INFO:
                    printf("%" PRIX8 "h - Protocol Specific Logical Unit Information VPD Page\n", supportedPages[vpdIter]);
                    break;
                case PROTOCOL_SPECIFIC_PORT_INFO:
                    printf("%" PRIX8 "h - Protocol Specific Port Information VPD Page\n", supportedPages[vpdIter]);
                    break;
                case SCSI_FEATURE_SETS:
                    printf("%" PRIX8 "h - SCSI Feature Sets VPD Page\n", supportedPages[vpdIter]);
                    if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
                    {
                        uint16_t vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
                        if (temp)
                        {
                            pageToRead = temp;
                            if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                            {
                                genericVPDPageReadOutput = false;
                                vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                                //TODO: Also validate the peripheral qualifier and peripheral device type???
                                if (pageToRead[1] != supportedPages[vpdIter])
                                {
                                    printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                                    print_Data_Buffer(pageToRead, vpdPageLength, true);
                                }
                                else
                                {
                                    for (uint16_t featIter = 8; featIter < (vpdPageLength + 4); featIter += 2)
                                    {
                                        uint16_t featureSetCode = M_BytesTo2ByteValue(pageToRead[featIter], pageToRead[featIter + 1]);
                                        switch (featureSetCode)
                                        {
                                        case 0x0001:
                                            printf("\tSPC Discovery 2016 Feature Set\n");
                                            printf("\t\tMandatory Commands:\n");
                                            printf("\t\t\tInquiry\n");
                                            printf("\t\t\tMode Sense 10\n");
                                            printf("\t\t\tReport Luns\n");
                                            printf("\t\t\tReport Supported Operation Codes\n");
                                            printf("\t\t\tTest Unit Ready\n");
                                            printf("\t\tMandatory Mode Pages:\n");
                                            printf("\t\t\tControl\n");
                                            printf("\t\t\tControl Extension\n");
                                            printf("\t\tMandatory VPD Pages:\n");
                                            printf("\t\t\tDevice Identification\n");
                                            printf("\t\t\tExtended Inquiry Data\n");
                                            printf("\t\t\tMode Page Policy\n");
                                            printf("\t\t\tSCSI Feature Sets\n");
                                            printf("\t\t\tSupported VPD Pages\n");
                                            break;
                                        case 0x0102: //SBC Base 2010
                                            printf("\tSBC Base 2010 Feature Set\n");
                                            printf("\t\tMandatory Commands:\n");
                                            printf("\t\t\tFormat Unit\n");
                                            printf("\t\t\tInquiry\n");
                                            printf("\t\t\tMode Select 10\n");
                                            printf("\t\t\tMode Sense 10\n");
                                            printf("\t\t\tRead 10\n");
                                            printf("\t\t\tRead Capacity 10\n");
                                            printf("\t\t\tReport Luns\n");
                                            printf("\t\t\tRequest Sense\n");
                                            printf("\t\t\tStart Stop Unit\n");
                                            printf("\t\t\tSynchronize Cache 10\n");
                                            printf("\t\t\tTest Unit Ready\n");
                                            printf("\t\t\tWrite 10\n");
                                            printf("\t\t\tWrite Same 10\n");
                                            printf("\t\tMandatory Mode Pages:\n");
                                            printf("\t\t\tCaching\n");
                                            printf("\t\t\tControl\n");
                                            printf("\t\t\tControl Extension\n");
                                            printf("\t\t\tRead-Write Error Recovery\n");
                                            printf("\t\tMandatory VPD Pages:\n");
                                            printf("\t\t\tATA Information (SAT devices only)\n");
                                            printf("\t\t\tBlock Device Characteristics\n");
                                            printf("\t\t\tDevice Identification\n");
                                            printf("\t\t\tExtended Inquiry Data\n");
                                            printf("\t\t\tMode Page Policy\n");
                                            printf("\t\t\tSCSI Feature Sets\n");
                                            printf("\t\t\tSupported VPD Pages\n");
                                            printf("\t\t\tPower Condition\n");
                                            break;
                                        case 0x0101: //SBC Base 2016
                                            printf("\tSBC Base 2016 Feature Set\n");
                                            printf("\t\tMandatory Commands:\n");
                                            printf("\t\t\tFormat Unit\n");
                                            printf("\t\t\tInquiry\n");
                                            printf("\t\t\tMode Select 10\n");
                                            printf("\t\t\tMode Sense 10\n");
                                            printf("\t\t\tRead 16\n");
                                            printf("\t\t\tRead Capacity 16\n");
                                            printf("\t\t\tReport Luns\n");
                                            printf("\t\t\tReport Supported Operation Codes\n");
                                            printf("\t\t\tReport Supported Task Management Functions\n");
                                            printf("\t\t\tRequest Sense\n");
                                            printf("\t\t\tStart Stop Unit\n");
                                            printf("\t\t\tSynchronize Cache 10\n");
                                            printf("\t\t\tTest Unit Ready\n");
                                            printf("\t\t\tWrite 16\n");
                                            printf("\t\t\tWrite Same 16\n");
                                            printf("\t\tMandatory Mode Pages:\n");
                                            printf("\t\t\tCaching\n");
                                            printf("\t\t\tControl\n");
                                            printf("\t\t\tControl Extension\n");
                                            printf("\t\t\tInformation Exceptions Control\n");
                                            printf("\t\t\tRead-Write Error Recovery\n");
                                            printf("\t\tMandatory VPD Pages:\n");
                                            printf("\t\t\tATA Information (SAT devices only)\n");
                                            printf("\t\t\tBlock Device Characteristics\n");
                                            printf("\t\t\tBlock Limits\n");
                                            printf("\t\t\tDevice Identification\n");
                                            printf("\t\t\tExtended Inquiry Data\n");
                                            printf("\t\t\tMode Page Policy\n");
                                            printf("\t\t\tSCSI Feature Sets\n");
                                            printf("\t\t\tSupported VPD Pages\n");
                                            printf("\t\t\tPower Condition\n");
                                            break;
                                        case 0x0103: //Basic Provisioning 2016
                                            printf("\tBasic Provisioning 2016 Feature Set\n");
                                            printf("\t\tMandatory Commands:\n");
                                            printf("\t\t\tGet LBA Status\n");
                                            printf("\t\t\tRead Capacity 16\n");
                                            printf("\t\t\tUnmap\n");
                                            printf("\t\t\tWrite Same 16\n");
                                            printf("\t\tMandatory VPD Pages:\n");
                                            printf("\t\t\tBlock Limits\n");
                                            printf("\t\t\tLogical Block Provisioning\n");
                                            break;
                                        case 0x0104: //Drive Maintenance 2016
                                            printf("\tDrive Maintenance 2016 Feature Set\n");
                                            printf("\t\tMandatory Commands:\n");
                                            printf("\t\t\tFormat Unit\n");
                                            printf("\t\t\tLog Select\n");
                                            printf("\t\t\tLog Sense\n");
                                            printf("\t\t\tRead Buffer 10\n");
                                            printf("\t\t\tRead Defect Data 12\n");
                                            printf("\t\t\tReasign Blocks\n");
                                            printf("\t\t\tSanitize\n");
                                            printf("\t\t\tSend Diagnostics\n");
                                            printf("\t\t\tReceive Diagnostic Results\n");
                                            printf("\t\t\tWrite Buffer\n");
                                            printf("\t\t\tWrite Long 16\n");
                                            printf("\t\tMandatory VPD Pages:\n");
                                            printf("\t\t\tExtended Inquiry Data\n");
                                            printf("\t\t\tBlock Device Characteristics\n");
                                            printf("\t\t\tPower Consumption\n");
                                            printf("\t\tMandatory Log Pages:\n");
                                            printf("\t\t\tSupported Pages\n");
                                            printf("\t\t\tSupported Pages and Subpages\n");
                                            printf("\t\t\tBackground Scan\n");
                                            printf("\t\t\tInformational Exceptions\n");
                                            printf("\t\t\tNon-Medium Error\n");
                                            printf("\t\t\tNon-volatile Cache\n");
                                            printf("\t\t\tRead Error Counters\n");
                                            printf("\t\t\tSelf-Test Results\n");
                                            printf("\t\t\tSolid State Media\n");
                                            printf("\t\t\tStart-Stop Cycle Counter\n");
                                            printf("\t\t\tTemperature\n");
                                            break;
                                        default:
                                            if (featureSetCode <= 0x00FF)
                                            {
                                                //SPC
                                                printf("\tUnknown SPC Feature Set: %04" PRIX16 "h\n", featureSetCode);
                                            }
                                            else if (featureSetCode > 0x00FF && featureSetCode <= 0x01FF)
                                            {
                                                //SBC
                                                printf("\tUnknown SBC Feature Set: %04" PRIX16 "h\n", featureSetCode);
                                            }
                                            else if (featureSetCode > 0x01FF && featureSetCode <= 0x02FF)
                                            {
                                                //SSC
                                                printf("\tUnknown SSC Feature Set: %04" PRIX16 "h\n", featureSetCode);
                                            }
                                            else if (featureSetCode > 0x02FF && featureSetCode <= 0x03FF)
                                            {
                                                //ZBC
                                                printf("\tUnknown ZBC Feature Set: %04" PRIX16 "h\n", featureSetCode);
                                            }
                                            else
                                            {
                                                //unknown or reserved feature set code
                                                printf("\tUnknown Standard Feature Set: %04" PRIX16 "h\n", featureSetCode);
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            else if (!dummiedPages)
                            {
                                printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                                printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                            }
                        }
                        else
                        {
                            printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                        }
                    }
                    else if (!dummiedPages)
                    {
                        printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                        printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
                    }
                    break;
                case BLOCK_LIMITS:
                    printf("%" PRIX8 "h - Block Limits VPD Page\n", supportedPages[vpdIter]);
                    if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
                    {
                        uint16_t vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
                        if (temp)
                        {
                            pageToRead = temp;
                            if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                            {
                                genericVPDPageReadOutput = false;
                                vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                                //TODO: Also validate the peripheral qualifier and peripheral device type???
                                if (pageToRead[1] != supportedPages[vpdIter])
                                {
                                    printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                                    print_Data_Buffer(pageToRead, vpdPageLength, true);
                                }
                                else if (vpdPageLength < 0x003C)
                                {
                                    printf("ERROR: VPD Page length is less than specified in SBC! Expected %" PRIX8 "h, but got %" PRIX8 "h\n", 0x003C, vpdPageLength);
                                } 
                                else
                                {
                                    if (pageToRead[5] > 0)
                                    {
                                        printf("\tMaximum Compare And Write Length: %" PRIu8 " Logical Blocks\n", pageToRead[5]);
                                    }
                                    printf("\tOptimal Transfer Length Granularity: ");
                                    if (M_BytesTo2ByteValue(pageToRead[6], pageToRead[7]) > 0)
                                    {
                                         printf("%" PRIu16 " Logical Blocks\n", M_BytesTo2ByteValue(pageToRead[6], pageToRead[7]));
                                    }
                                    else
                                    {
                                        printf("Not reported\n");
                                    }
                                    printf("Maximum Transfer Length: ");
                                    if (M_BytesTo4ByteValue(pageToRead[8], pageToRead[9], pageToRead[10], pageToRead[11]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[8], pageToRead[9], pageToRead[10], pageToRead[11]));
                                    }
                                    else
                                    {
                                        printf("Not Reported\n");
                                    }
                                    printf("Optimal Transfer Length: ");
                                    if (M_BytesTo4ByteValue(pageToRead[12], pageToRead[13], pageToRead[14], pageToRead[15]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[12], pageToRead[13], pageToRead[14], pageToRead[15]));
                                    }
                                    else
                                    {
                                        printf("Not Reported\n");
                                    }
                                    printf("Maximum Prefetch Length: ");
                                    if (M_BytesTo4ByteValue(pageToRead[16], pageToRead[17], pageToRead[18], pageToRead[19]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[16], pageToRead[17], pageToRead[18], pageToRead[19]));
                                    }
                                    else
                                    {
                                        printf("Not Reported or prefetch not supported\n");
                                    }
                                    printf("Maximum Unmap LBA Count: ");
                                    if (M_BytesTo4ByteValue(pageToRead[20], pageToRead[21], pageToRead[22], pageToRead[23]) > 0)
                                    {
                                        printf("%" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[20], pageToRead[21], pageToRead[22], pageToRead[23]));
                                    }
                                    else
                                    {
                                        printf("Not Reported\n");
                                    }
                                    printf("Maximum Unmap Block Descriptor Count: ");
                                    if (M_BytesTo4ByteValue(pageToRead[24], pageToRead[25], pageToRead[26], pageToRead[27]) > 0)
                                    {
                                        printf("%" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[24], pageToRead[25], pageToRead[26], pageToRead[27]));
                                    }
                                    else
                                    {
                                        printf("Not Reported\n");
                                    }
                                    printf("Optimal Unmap Granularity: ");
                                    if (M_BytesTo4ByteValue(pageToRead[28], pageToRead[29], pageToRead[30], pageToRead[31]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[28], pageToRead[29], pageToRead[30], pageToRead[31]));
                                    }
                                    else
                                    {
                                        printf("Not Reported\n");
                                    }
                                    if (pageToRead[32] & BIT7)
                                    {
                                        printf("Unmap Granularity Alignment: ");
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[32] ^ BIT7, pageToRead[33], pageToRead[34], pageToRead[35]));
                                    }
                                    printf("Maximum Write Same Length: ");
                                    if (M_BytesTo8ByteValue(pageToRead[36], pageToRead[37], pageToRead[38], pageToRead[39], pageToRead[40], pageToRead[41], pageToRead[42], pageToRead[43]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo8ByteValue(pageToRead[36], pageToRead[37], pageToRead[38], pageToRead[39], pageToRead[40], pageToRead[41], pageToRead[42], pageToRead[43]));
                                    }
                                    else
                                    {
                                        printf("Not Reported or write same not supported\n");
                                    }
                                    if (pageToRead[4] & BIT0)
                                    {
                                        printf("\tWrite same command does not allow number of logical blocks to be zero (write full LBA space the same)\n");
                                    }
                                    printf("Maximum Atomic Transfer Length: ");
                                    if (M_BytesTo4ByteValue(pageToRead[44], pageToRead[45], pageToRead[46], pageToRead[47]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[44], pageToRead[45], pageToRead[46], pageToRead[47]));
                                    }
                                    else
                                    {
                                        printf("Not Reported or write atomic not supported\n");
                                    }
                                    printf("Atomic Alignment: ");
                                    if (M_BytesTo4ByteValue(pageToRead[48], pageToRead[49], pageToRead[50], pageToRead[51]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[48], pageToRead[49], pageToRead[50], pageToRead[51]));
                                    }
                                    else
                                    {
                                        printf("Not Reported or write atomic not supported\n");
                                    }
                                    printf("Atomic Transfer Length Granularity: ");
                                    if (M_BytesTo4ByteValue(pageToRead[52], pageToRead[53], pageToRead[54], pageToRead[55]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[52], pageToRead[53], pageToRead[54], pageToRead[55]));
                                    }
                                    else
                                    {
                                        printf("Not Reported or write atomic not supported\n");
                                    }
                                    printf("Maximum Atomic Transfer Length With Atomic Boundary: ");
                                    if (M_BytesTo4ByteValue(pageToRead[56], pageToRead[57], pageToRead[58], pageToRead[59]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[56], pageToRead[57], pageToRead[58], pageToRead[59]));
                                    }
                                    else
                                    {
                                        printf("Not Reported or write atomic not supported\n");
                                    }
                                    printf("Maximum Atomic Boundary Size: ");
                                    if (M_BytesTo4ByteValue(pageToRead[60], pageToRead[61], pageToRead[62], pageToRead[63]) > 0)
                                    {
                                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[60], pageToRead[61], pageToRead[62], pageToRead[63]));
                                    }
                                    else
                                    {
                                        printf("Not Reported or write atomic not supported\n");
                                    }
                                }
                            }
                            else if (!dummiedPages)
                            {
                                printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                                printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                            }
                        }
                        else
                        {
                            printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                        }
                    }
                    else if (!dummiedPages)
                    {
                        printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                        printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
                    }
                    break;
                case BLOCK_DEVICE_CHARACTERISTICS:
                    printf("%" PRIX8 "h - Block Device Characteristics VPD Page\n", supportedPages[vpdIter]);
                    //TODO: Thorough parsing of this page and outputting its information in a nice format
                    break;
                case LOGICAL_BLOCK_PROVISIONING:
                    printf("%" PRIX8 "h - Logical Block Provisioning VPD Page\n", supportedPages[vpdIter]);
                    //TODO: Thorough parsing of this page and outputting its information in a nice format
                    break;
                case REFERALS:
                    printf("%" PRIX8 "h - Referals VPD Page\n", supportedPages[vpdIter]);
                    break;
                case SUPPORTED_BLOCK_LENGTHS_AND_PROTECTION_TYPES:
                    printf("%" PRIX8 "h - Supported Block Lengths And Protection Types VPD Page\n", supportedPages[vpdIter]);
                    break;
                case BLOCK_DEVICE_CHARACTERISTISCS_EXT:
                    printf("%" PRIX8 "h - Block Device Characteristics Ext VPD Page\n", supportedPages[vpdIter]);
                    //TODO: Thorough parsing of this page and outputting its information in a nice format
                    break;
                case ZONED_BLOCK_DEVICE_CHARACTERISTICS:
                    printf("%" PRIX8 "h - Zoned Block Device Characteristics VPD Page\n", supportedPages[vpdIter]);
                    //TODO: Thorough parsing of this page and outputting its information in a nice format
                    break;
                case BLOCK_LIMITS_EXTENSION:
                    printf("%" PRIX8 "h - Block Limits Extension VPD Page\n", supportedPages[vpdIter]);
                    //TODO: Thorough parsing of this page and outputting its information in a nice format
                    break;
                default:
                    if (supportedPages[vpdIter] >= 0x01 && supportedPages[vpdIter] <= 0x7F)
                    {
                        //ascii information page
                        printf("%" PRIX8 "h - ASCII Information VPD Page\n", supportedPages[vpdIter]);
                    }
                    else if (supportedPages[vpdIter] >= 0xC0 && supportedPages[vpdIter] <= 0xFF)
                    {
                        //vendor specific
                        printf("%" PRIX8 "h - Vendor Specific VPD Page\n", supportedPages[vpdIter]);
                    } 
                    else
                    {
                        //unknown page
                        printf("%" PRIX8 "h - Unknown VPD Page\n", supportedPages[vpdIter]);
                    }
                    break;
                }
                if (genericVPDPageReadOutput)
                {
                    if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
                    {
                        uint16_t vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
                        if (temp)
                        {
                            pageToRead = temp;
                            if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                            {
                                //TODO: Also validate the peripheral qualifier and peripheral device type???
                                if (pageToRead[1] != supportedPages[vpdIter])
                                {
                                    printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                                }
                                print_Data_Buffer(pageToRead, vpdPageLength, true);
                            }
                            else if (!dummiedPages)
                            {
                                printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                                printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                            }
                        }
                        else
                        {
                            printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                        }
                    }
                    else if (!dummiedPages)
                    {
                        printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                        printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
                    }
                }
                safe_Free(pageToRead);
            }
            //Now check for log pages - Warn about any missing MANDATORY pages

            //Now check for mode pages - Warn about any missing MANDATORY pages
            //TODO: check for vendor specific mode page 0? Might help identify translators vs non-translators, but might not.


            if (!continueTesting)
            {
                return NOT_SUPPORTED;
            }
        }
        else
        {
            set_Console_Colors(true, RED);
            printf("Fatal Error: Unable to get standard inquiry data. Unable to proceed with any more testing!\n");
            set_Console_Colors(true, DEFAULT);
            return FAILURE;
        }

        //3. Start checking for SAT passthrough, unless given information to use a different passthrough.
        
        //4. optionally check SCSI read/write CDB support. This will not be on by default since it doesn't actually seem to work on anything other than a proper SAS drive today
        if (inputs->checkReadWriteSupport)
        {
            printf("Checking Read/Write support by setting transfer length to zero!\n");
            printf("This is part of the SBC standards and the device should return success\n");
            printf("without issuing ANY read or write commands if the CDB operation code\n");
            printf("is known!\n\n");
        }
        //5. optionally do a more in depth check for additional SCSI commands like report supported operation codes that would also be useful, or security protocol commands.
        if (inputs->checkAdditionalSCSICommandSupport)
        {
            printf("Checking for additional SCSI CDBs that could be useful.\n");
        }
        //Finally. Display the results and which hacks are necessary

        safe_Free(unitSerialNumber);
    }
    else
    {
        ret = BAD_PARAMETER;
    }
    return ret;
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

    printf("\nExamples\n");
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
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    //multiple interfaces

    //SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");

    //SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");

    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //utility data destructive tests/operations go here
}

