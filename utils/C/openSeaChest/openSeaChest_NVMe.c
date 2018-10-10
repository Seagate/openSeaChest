//
// SeaChest_NVMe.c
//
// Copyright (c) 2014-2018 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// **************************************************************************************
// \file SeaChest_NVMe.c Binary command line that performs various erase methods on a device.

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

////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "SeaChest_NVMe";
const char *buildVersion = "0.0.3";

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
    PARTIAL_DATA_ERASE_VAR
    LICENSE_VAR
    ECHO_COMMAND_LINE_VAR
    SCAN_FLAG_VAR
    SHOW_BANNER_VAR
    SHOW_HELP_VAR
    TEST_UNIT_READY_VAR
    DOWNLOAD_FW_VARS
    CHECK_POWER_VAR
    TRANSITION_POWER_STATE_VAR
    GET_NVME_LOG_VAR
    GET_NVME_TELE_VAR
    NVME_TELE_DATA_AREA_VAR
    FORMAT_UNIT_VARS
    OUTPUT_MODE_VAR
    GET_FEATURES_VAR
    NVME_TEMP_STATS_VAR
    NVME_PCI_STATS_VAR
    MODEL_MATCH_VARS
    FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    //POWER_MODE_VAR
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    EXT_SMART_LOG_VAR1
    CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_VAR

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
        USB_CHILD_INFO_LONG_OPT,
        SCAN_LONG_OPT,
        SCAN_FLAGS_LONG_OPT,
        VERSION_LONG_OPT,
        VERBOSE_LONG_OPT,
        QUIET_LONG_OPT,
        LICENSE_LONG_OPT,
        ECHO_COMMAND_LIN_LONG_OPT,
        TEST_UNIT_READY_LONG_OPT,
        //tool specific options go here
        DOWNLOAD_FW_MODE_LONG_OPT,
        DOWNLOAD_FW_LONG_OPT,
        CHECK_POWER_LONG_OPT,
        TRANSITION_POWER_STATE_LONG_OPT,
        GET_NVME_LOG_LONG_OPT,
        GET_NVME_TELE_LONG_OPT,
        NVME_TELE_DATA_AREA_LONG_OPT,
        FORMAT_UNIT_LONG_OPT,
        CONFIRM_LONG_OPT,
        OUTPUT_MODE_LONG_OPT,
        GET_FEATURES_LONG_OPT,
        EXT_SMART_LOG_LONG_OPT1,
        CLEAR_PCIE_CORRECTABLE_ERRORS_LONG_OPT,
        NVME_TEMP_STATS_LONG_OPT,
        NVME_PCI_STATS_LONG_OPT,
        LONG_OPT_TERMINATOR
    };

    g_verbosity = VERBOSITY_DEFAULT;
    DOWNLOAD_FW_MODE = DL_FW_UNKNOWN;

    atexit(print_Final_newline);

    ////////////////////////
    //  Argument Parsing  //
    ////////////////////////
    if (argc < 2)
    {
        openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
        utility_Usage(true);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    opterr = 0;//hide getopt parsing errors and let us handle them in the '?' case or ':' case
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
            else if (strncmp(longopts[optionIndex].name, DOWNLOAD_FW_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DOWNLOAD_FW_LONG_OPT_STRING))) == 0)
            {
                DOWNLOAD_FW_FLAG = true;
                sscanf(optarg, "%s", DOWNLOAD_FW_FILENAME_FLAG);
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
            else if (strncmp(longopts[optionIndex].name, DOWNLOAD_FW_MODE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DOWNLOAD_FW_MODE_LONG_OPT_STRING))) == 0)
            {
                DOWNLOAD_FW_MODE = DL_FW_UNKNOWN;
                if (strncmp(optarg, "immediate", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_FULL;
                }
                else if (strncmp(optarg, "deferred", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_DEFERRED;
                }
                else if (strncmp(optarg, "activate", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_ACTIVATE;
                }
                else
                {
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    printf("\nerror processing --%s\n\n",longopts[optionIndex].name);
                    printf("Please use -h option to print help\n\n");
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
                    else if (strncmp("smart", optarg, strlen(optarg)) == 0)
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
            else if (strncmp(longopts[optionIndex].name, GET_NVME_TELE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(GET_NVME_TELE_LONG_OPT_STRING))) == 0)
            {
                if (strncmp("HOST", optarg, strlen(optarg)) == 0)
                {
                    GET_NVME_TELE_IDENTIFIER = NVME_LOG_TELEMETRY_HOST;
                }
                else if (strncmp("CTRL", optarg, strlen(optarg)) == 0)
                {
                    GET_NVME_TELE_IDENTIFIER = NVME_LOG_TELEMETRY_CTRL;
                }
                else
                {
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    printf("\nerror processing --%s\n\n",longopts[optionIndex].name);
                    printf("Please use -h option to print help\n\n");
                }
            }
            else if (strncmp(longopts[optionIndex].name, NVME_TELE_DATA_AREA_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(NVME_TELE_DATA_AREA_LONG_OPT_STRING))) == 0)
            {
                //set the telemetry data area
                if (isdigit(optarg[0]))//this will get the valid NVMe telemetry data area
                {
                    NVME_TELE_DATA_AREA = atoi(optarg);
                }
                else
                {
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    printf("\nerror processing --%s\n\n",longopts[optionIndex].name);
                    printf("Please use -h option to print help\n\n");
                }
            }
            else if (strcmp(longopts[optionIndex].name, FORMAT_UNIT_LONG_OPT_STRING) == 0)
            {
                FORMAT_UNIT_FLAG = goTrue;
                if (strcmp(optarg, "current") != 0)
                {
                    //set the sector size
                    FORMAT_SECTOR_SIZE = (uint32_t)atoi(optarg);
                }
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
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("You must specify a device handle\n");
                }
                return UTIL_EXIT_INVALID_DEVICE_HANDLE;
            case VERBOSE_SHORT_OPT:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("You must specify a verbosity level. 0 - 4 are the valid levels\n");
                }
                break;
            case SCAN_FLAGS_SHORT_OPT:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("You must specify which scan options flags you want to use.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Option %c requires an argument\n", optopt);
                }
                utility_Usage(true);
                return exitCode;
            }
            break;
        case DEVICE_SHORT_OPT: //device
            if (0 != parse_Device_Handle_Argument(optarg, &RUN_ON_ALL_DRIVES, &USER_PROVIDED_HANDLE, &DEVICE_LIST_COUNT, &HANDLE_LIST))
            {
                //Free any memory allocated so far, then exit.
                free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
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
                g_verbosity = atoi(optarg);
            }
            break;
        case QUIET_SHORT_OPT: //quiet mode
            g_verbosity = VERBOSITY_QUIET;
            break;
        case SCAN_FLAGS_SHORT_OPT://scan flags
            SCAN_FLAGS_SUBOPT_PARSING;
            break;
        case '?': //unknown option
        case 'h': //help
            SHOW_HELP_FLAG = true;
            openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
            utility_Usage(false);
            exit(UTIL_EXIT_NO_ERROR);
        default:
            break;
        }
    }


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

    if (VERBOSITY_QUIET < g_verbosity)
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
        scan_And_Print_Devs(scanControl, NULL);
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
    else if (DOWNLOAD_FW_FLAG == true)
    {
        //check
        if ( ( DOWNLOAD_FW_MODE == DL_FW_ACTIVATE ) || (DOWNLOAD_FW_MODE == DL_FW_UNKNOWN) )
        {
            printf("\nerror processing command line options\n\n");
            if (DOWNLOAD_FW_MODE == DL_FW_UNKNOWN)
            {
                printf("missing --%s\n\n",DOWNLOAD_FW_MODE_LONG_OPT_STRING);
            }
            if( DOWNLOAD_FW_MODE == DL_FW_ACTIVATE )
            {
                printf("To download AND activate firmware, use 'immediate' option\n\n");
            }
            print_Firmware_Download_Help(false);
            print_NVMe_Firmware_Download_Mode_Help(false);
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
        }
    }

    //print out errors for unknown arguments for remaining args that haven't been processed yet
    for (argIndex = optind; argIndex < argc; argIndex++)
    {
        if (VERBOSITY_QUIET < g_verbosity)
        {
            printf("Invalid argument: %s\n", argv[argIndex]);
        }
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
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Unable to get number of devices\n");
            }
            exit(UTIL_EXIT_OPERATION_FAILURE);
        }
    }
    else if (DEVICE_LIST_COUNT == 0)
    {
        if (VERBOSITY_QUIET < g_verbosity)
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
          || TEST_UNIT_READY_FLAG
          || CHECK_POWER_FLAG
          || (TRANSITION_POWER_STATE_TO >= 0)
          || (GET_NVME_LOG_IDENTIFIER > 0) // Since 0 is Reserved
          || (GET_NVME_TELE_IDENTIFIER > 0)
          || ( DOWNLOAD_FW_MODE == DL_FW_ACTIVATE )
          || (GET_FEATURES_IDENTIFIER >= 0)
          || FORMAT_UNIT_FLAG
          || EXT_SMART_LOG_FLAG1
          || CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_FLAG
          || NVME_TEMP_STATS_FLAG
          || NVME_PCI_STATS_FLAG 
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
        if (VERBOSITY_QUIET < g_verbosity)
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
//  if (FORCE_ATA_PIO_FLAG)
//  {
//      flags |= FORCE_ATA_PIO_ONLY;
//  }
//
//  if (FORCE_ATA_DMA_FLAG)
//  {
//      flags |= FORCE_ATA_DMA_SAT_MODE;
//  }
//
//  if (FORCE_ATA_UDMA_FLAG)
//  {
//      flags |= FORCE_ATA_UDMA_SAT_MODE;
//  }

    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        ret = get_Device_List(DEVICE_LIST, DEVICE_LIST_COUNT * sizeof(tDevice), version, flags);
        if (SUCCESS != ret)
        {
            if (ret == WARN_NOT_ALL_DEVICES_ENUMERATED)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("WARN: Not all devices enumerated correctly\n");
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
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
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Error: Could not open handle to %s\n", HANDLE_LIST[handleIter]);
                }
                free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
            }
        }
    }
    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
    for (uint32_t deviceIter = 0; deviceIter < numberOfDevices; ++deviceIter)
    {
    	if (ONLY_SEAGATE_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                if (VERBOSITY_QUIET < g_verbosity)
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
				if (VERBOSITY_QUIET < g_verbosity)
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
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input firmware revision: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_revision, FW_STRING_FLAG);
                }
                continue;
            }
        }
        
	    //now start looking at what operations are going to be performed and kick them off
	    if (DEVICE_INFO_FLAG)
	    {
	        if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_HUMAN)
	        {
	            if (SUCCESS != print_Drive_Information(&deviceList[deviceIter], SAT_INFO_FLAG))
	            {
	                if (VERBOSITY_QUIET < g_verbosity)
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
	            print_Data_Buffer((uint8_t *)&deviceList[deviceIter].drive_info.IdentifyData.nvme.ctrl,sizeof(nvmeIDCtrl),true);
	            printf("\nNamespace Identify Information:\n");
	            printf("================================\n");
	            print_Data_Buffer((uint8_t *)&deviceList[deviceIter].drive_info.IdentifyData.nvme.ns,sizeof(nvmeIDNameSpaces),true);
	        }
	        else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_BIN)
	        {
	            FILE *		pIdentifyFile = NULL;
	            char identifyFileName[OPENSEA_PATH_MAX] = {0};
	            char * fileNameUsed = &identifyFileName[0];
	            if(SUCCESS == create_And_Open_Log_File(&deviceList[deviceIter], &pIdentifyFile, NULL,\
	                                                    "CTRL_IDENTIFY", "bin", 1, &fileNameUsed) )
	            {
	                fwrite(&deviceList[deviceIter].drive_info.IdentifyData.nvme.ctrl, sizeof(uint8_t), sizeof(nvmeIDNameSpaces), pIdentifyFile);
	                fflush(pIdentifyFile);
	                fclose(pIdentifyFile);
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("Created %s file with Controller Information\n",fileNameUsed);
	                }
	                memset(fileNameUsed,0,OPENSEA_PATH_MAX);
	                if(SUCCESS == create_And_Open_Log_File(&deviceList[deviceIter], &pIdentifyFile, NULL,\
	                                                        "NAMESPACE_IDENTIFY", "bin", 1, &fileNameUsed) )
	                {
	                    fwrite(&deviceList[deviceIter].drive_info.IdentifyData.nvme.ns, sizeof(uint8_t), sizeof(nvmeIDNameSpaces), pIdentifyFile);
	                    fflush(pIdentifyFile);
	                    fclose(pIdentifyFile);    
	                    if (VERBOSITY_QUIET < g_verbosity)
	                    {
	                        printf("Created %s file with Namespace Information\n",fileNameUsed);
	                    }
	                }
	                else
	                {
	                    if (VERBOSITY_QUIET < g_verbosity)
	                    {
	                        printf("ERROR: failed to open file to write namespace information\n");
	                    }
	                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                }
	
	            }
	            else
	            {
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("ERROR: failed to open file to write controller information\n");
	                }
	                exitCode = UTIL_EXIT_OPERATION_FAILURE;
	            }
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
	            nvme_Print_All_Feature_Identifiers(&deviceList[deviceIter], NVME_CURRENT_FEAT_SEL, false );
	        } 
	        else
	        {
	            //Get the feature 
	            if (nvme_Print_Feature_Details(&deviceList[deviceIter], GET_FEATURES_IDENTIFIER, NVME_CURRENT_FEAT_SEL ) != SUCCESS)
	            {
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("ERROR: failed to get details for feature id %d\n",GET_FEATURES_IDENTIFIER);
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
	                memset(&cmdOpts,0, sizeof(nvmeGetLogPageCmdOpts));
	                if (NVME_LOG_ERROR_ID == GET_NVME_LOG_IDENTIFIER)
	                {
	                    size = 32 * size; //Get first 32 entries. 
	                }
	                logBuffer = calloc(size, 1);
	                if (logBuffer != NULL)
	                {
	                    cmdOpts.nsid = NVME_ALL_NAMESPACES;
	                    cmdOpts.addr = (uint64_t)logBuffer;
	                    cmdOpts.dataLen = size;
	                    cmdOpts.lid = GET_NVME_LOG_IDENTIFIER;
	                    if(nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts)==SUCCESS)
	                    {
	                        if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_RAW)
	                        {
	                            printf("Log Page %d Buffer:\n",GET_NVME_LOG_IDENTIFIER);
	                            printf("================================\n");
	                            print_Data_Buffer((uint8_t *)logBuffer,size,true);
	                            printf("================================\n");
	                        }
	                        else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_BIN)
	                        {
	                            FILE * pLogFile = NULL;
	                            char identifyFileName[OPENSEA_PATH_MAX] = {0};
	                            char * fileNameUsed = &identifyFileName[0];
	                            char logName[16];
	                            sprintf(logName, "LOG_PAGE_%d",GET_NVME_LOG_IDENTIFIER);
	                            if(SUCCESS == create_And_Open_Log_File(&deviceList[deviceIter], &pLogFile, NULL,\
	                                                                    logName, "bin", 1, &fileNameUsed) )
	                            {
	                                fwrite(logBuffer, sizeof(uint8_t), size, pLogFile);
	                                fflush(pLogFile);
	                                fclose(pLogFile);
	                                if (VERBOSITY_QUIET < g_verbosity)
	                                {
	                                    printf("Created %s with Log Page %d Information\n",fileNameUsed,GET_NVME_LOG_IDENTIFIER);
	                                }
	                            }
	                            else
	                            {
	                                    if (VERBOSITY_QUIET < g_verbosity)
	                                    {
	                                        printf("ERROR: failed to open file to write Log Page Information\n");
	                                    }
	                                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                            }
	                        }
	                        else
	                        {
	                            if (VERBOSITY_QUIET < g_verbosity)
	                            {
	                                printf("Error: Unknown/Unsupported output mode %d\n", OUTPUT_MODE_IDENTIFIER);
	                            }
	                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                        }
	                    }
	                    else
	                    {
	                        if (VERBOSITY_QUIET < g_verbosity)
	                        {
	                            printf("Error: Could not retrieve Log Page %d\n", GET_NVME_LOG_IDENTIFIER);
	                        }
	                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                    }
	                    safe_Free(logBuffer);
	                }
	                else
	                {
	                    if (VERBOSITY_QUIET < g_verbosity)
	                    {
	                        printf("Couldn't allocate %ld bytes buffer needed for Log Page %d\n", size, GET_NVME_LOG_IDENTIFIER);
	                    }
	                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                }
	            }
	            else
	            {
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("Log Page %d not available at this time. \n",GET_NVME_LOG_IDENTIFIER);
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
	                        if (VERBOSITY_QUIET < g_verbosity)
	                        {
	                            printf("A failure occured while trying to get SMART/Health Information Log\n");
	                        }
	                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                        break;
	                    }
	                break;
	                case NVME_LOG_ERROR_ID:
	                    switch(nvme_Print_ERROR_Log_Page(&deviceList[deviceIter], 0))
	                    {
	                    case SUCCESS:
	                        //nothing to print here since if it was successful, the log will be printed to the screen
	                        break;
	                    default:
	                        if (VERBOSITY_QUIET < g_verbosity)
	                        {
	                            printf("A failure occured while trying to get Error Information Log\n");
	                        }
	                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                        break;
	                    }
	                break;
	                case NVME_LOG_FW_SLOT_ID:
	                    switch(nvme_Print_FWSLOTS_Log_Page(&deviceList[deviceIter]))
	                    {
	                    case SUCCESS:
	                        //nothing to print here since if it was successful, the log will be printed to the screen
	                        break;
	                    default:
	                        if (VERBOSITY_QUIET < g_verbosity)
	                        {
	                            printf("A failure occured while trying to get FW Slot Information Log\n");
	                        }
	                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                        break;
	                    }
	                break;
	                case NVME_LOG_CMD_SPT_EFET_ID:
	                    switch(nvme_Print_CmdSptEfft_Log_Page(&deviceList[deviceIter]))
	                    {
	                    case SUCCESS:
	                        //nothing to print here since if it was successful, the log will be printed to the screen
	                        break;
	                    default:
	                        if (VERBOSITY_QUIET < g_verbosity)
	                        {
	                            printf("A failure occured while trying to get Commands Supported and Effects Information Log\n");
	                        }
	                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                        break;
	                    }
	                break;
	                case NVME_LOG_DEV_SELF_TEST:
	                    switch(nvme_Print_DevSelfTest_Log_Page(&deviceList[deviceIter]))
	                    {
	                    case SUCCESS:
	                        //nothing to print here since if it was successful, the log will be printed to the screen
	                        break;
	                    default:
	                        if (VERBOSITY_QUIET < g_verbosity)
	                        {
	                            printf("A failure occured while trying to get Device Self-test Information Log\n");
	                        }
	                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                        break;
	                    }
	                break;
	                default:
	                    if (VERBOSITY_QUIET < g_verbosity)
	                    {
	                        printf("Log Page %d not available at this time. \n",GET_NVME_LOG_IDENTIFIER);
	                    }
	                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
	                break;
	            }
	        }
	    }

        if (GET_NVME_TELE_IDENTIFIER > 0) //Since 0 is reserved log
        {
            /**
             * Checking for validty of NVME_TELE_DATA_AREA 
             * If it is not in between 1-3 we should exit with error 
             */

            if ((NVME_TELE_DATA_AREA < 1) && (NVME_TELE_DATA_AREA > 3)) 
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("%d is an invalid temetry data area. \n", NVME_TELE_DATA_AREA);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;

            }
            else
            {
                uint64_t size = BLOCK_SIZE; 
                uint8_t * logBuffer = NULL;
                nvmeGetLogPageCmdOpts cmdOpts;
                uint64_t offset = 0;
                uint64_t fullSize;
                int rtnVal;
                nvmeTemetryLogHdr   *teleHdr;

                memset(&cmdOpts,0, sizeof(nvmeGetLogPageCmdOpts));

                logBuffer = calloc(size, 1);

                if (logBuffer != NULL)
                {
                    cmdOpts.nsid = NVME_ALL_NAMESPACES;
                    cmdOpts.addr = (uint64_t)logBuffer;
                    cmdOpts.dataLen = size;
                    cmdOpts.lid = GET_NVME_TELE_IDENTIFIER;
                    cmdOpts.offset = offset;

                    rtnVal = nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts);
                    offset += BLOCK_SIZE;

                    if(rtnVal == SUCCESS)
                    {
                        teleHdr = (nvmeTemetryLogHdr *)logBuffer;
            
#if defined(_DEBUG)
                        printf("Telemetry Data Area 1 : %d \n", teleHdr->teleDataArea1);
                        printf("Telemetry Data Area 2 : %d \n", teleHdr->teleDataArea2);
                        printf("Telemetry Data Area 3 : %d \n", teleHdr->teleDataArea3);
#endif
                                    
                        if(NVME_TELE_DATA_AREA == 1) 
                        {
                            fullSize = offset + BLOCK_SIZE * teleHdr->teleDataArea1;
                        }

                        if(NVME_TELE_DATA_AREA == 2) 
                        {
                            fullSize = offset + BLOCK_SIZE * teleHdr->teleDataArea2;
                        }

                        if(NVME_TELE_DATA_AREA == 3) 
                        {
                            fullSize = offset + BLOCK_SIZE * teleHdr->teleDataArea3;
                        }

                        if ((OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_RAW) || 
                            (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_HUMAN))
                        {
                            printf("Log Page %d Buffer:\n",GET_NVME_TELE_IDENTIFIER);
                            printf("================================\n");
                            print_Data_Buffer((uint8_t *)logBuffer,size,true);

                            while(offset < fullSize) 
                            {
                                memset(logBuffer, 0, size);
                                cmdOpts.nsid = NVME_ALL_NAMESPACES;
                                cmdOpts.addr = (uint64_t)logBuffer;
                                cmdOpts.dataLen = size;
                                cmdOpts.lid = GET_NVME_TELE_IDENTIFIER;
                                cmdOpts.offset = offset;

                                rtnVal = nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts);
                                offset += BLOCK_SIZE;

                                if(rtnVal != SUCCESS) 
                                {
                                    if (VERBOSITY_QUIET < g_verbosity)
                                    {
                                        printf("Error: Could not retrieve Log Page %d for offset %d\n", GET_NVME_TELE_IDENTIFIER, offset - BLOCK_SIZE);
                                    }
                                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                    break;
                                }

                                print_Data_Buffer((uint8_t *)logBuffer,size,true);
                            }
                            printf("================================\n");
                        }
                        else if (OUTPUT_MODE_IDENTIFIER == UTIL_OUTPUT_MODE_BIN)
                        {
                            FILE * pLogFile = NULL;
                            char identifyFileName[OPENSEA_PATH_MAX] = {0};
                            char * fileNameUsed = &identifyFileName[0];
                            char logName[16];
                            sprintf(logName, "LOG_PAGE_%d",GET_NVME_LOG_IDENTIFIER);
                            if(SUCCESS == create_And_Open_Log_File(&deviceList[deviceIter], &pLogFile, NULL,\
                                                                    logName, "bin", 1, &fileNameUsed) )
                            {
                                fwrite(logBuffer, sizeof(uint8_t), size, pLogFile);

                                while(offset < fullSize) 
                                {
                                    memset(logBuffer, 0, size);
                                    cmdOpts.nsid = NVME_ALL_NAMESPACES;
                                    cmdOpts.addr = (uint64_t)logBuffer;
                                    cmdOpts.dataLen = size;
                                    cmdOpts.lid = GET_NVME_TELE_IDENTIFIER;
                                    cmdOpts.offset = offset;

                                    rtnVal = nvme_Get_Log_Page(&deviceList[deviceIter], &cmdOpts);
                                    offset += BLOCK_SIZE;

                                    if(rtnVal != SUCCESS) 
                                    {
                                        if (VERBOSITY_QUIET < g_verbosity)
                                        {
                                            printf("Error: Could not retrieve Log Page %d for offset %d\n", GET_NVME_TELE_IDENTIFIER, offset - BLOCK_SIZE);
                                        }
                                        exitCode = UTIL_EXIT_OPERATION_FAILURE;

                                        break;
                                    }

                                    fwrite(logBuffer, sizeof(uint8_t), size, pLogFile);
                                }

                                fflush(pLogFile);
                                fclose(pLogFile);


                                if (VERBOSITY_QUIET < g_verbosity)
                                {
                                    printf("Created %s with Log Page %d Information\n",fileNameUsed,GET_NVME_LOG_IDENTIFIER);
                                }
                            }
                            else
                            {
                                    if (VERBOSITY_QUIET < g_verbosity)
                                    {
                                        printf("ERROR: failed to open file to write Log Page Information\n");
                                    }
                                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            }
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Error: Unknown/Unsupported output mode %d\n", OUTPUT_MODE_IDENTIFIER);
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Error: Could not retrieve Log Page %d\n", GET_NVME_TELE_IDENTIFIER);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    safe_Free(logBuffer);
                }
                else
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Couldn't allocate %ld bytes buffer needed for Log Page %d\n", size, GET_NVME_LOG_IDENTIFIER);
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }            
        }

        if (NVME_TEMP_STATS_FLAG == goTrue)
        {
            switch(nvme_Print_Temp_Statistics(&deviceList[deviceIter]))
            {
            case SUCCESS:
                //nothing to print here since if it was successful, the log will be printed to the screen
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("A failure occured while trying to get Error Information Log\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (NVME_PCI_STATS_FLAG == goTrue)
        {
            switch(nvme_Print_PCI_Statistics(&deviceList[deviceIter]))
            {
            case SUCCESS:
                //nothing to print here since if it was successful, the log will be printed to the screen
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
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
	
	    if ( (DOWNLOAD_FW_FLAG) || ( DOWNLOAD_FW_MODE == DL_FW_ACTIVATE ) )
	    {
	        FILE *firmwareFilePtr = NULL;
	        //If it is not activate, must be either immediate or deffered. 
	        if (DOWNLOAD_FW_MODE != DL_FW_ACTIVATE)
	        {
	            //open the file and send the download
	            if ((firmwareFilePtr = fopen(DOWNLOAD_FW_FILENAME_FLAG, "rb")) != NULL)
	            {
                    long firmwareFileSize = get_File_Size(firmwareFilePtr);
                    uint8_t *firmwareMem = (uint8_t*)calloc(firmwareFileSize * sizeof(uint8_t), sizeof(uint8_t));
                    fread(firmwareMem, sizeof(uint8_t), firmwareFileSize, firmwareFilePtr);
	                //We always want to do DL_FW_DEFERRED for NVMe
                    memset(&dlOptions, 0, sizeof(firmwareUpdateData));
                    memset(&commandTimer, 0, sizeof(seatimer_t));
                    dlOptions.dlMode = DL_FW_DEFERRED;									
                    dlOptions.segmentSize = 64;
                    dlOptions.firmwareFileMem = firmwareMem;
                    dlOptions.firmwareMemoryLength = firmwareFileSize;
                    start_Timer(&commandTimer);
                    int firmwareDLResult = firmware_Download(&deviceList[deviceIter], &dlOptions);
                    stop_Timer(&commandTimer);
	                switch (firmwareDLResult)
	                {
	                case SUCCESS:
	                    if (VERBOSITY_QUIET < g_verbosity)
	                    {
	                        printf("Firmware Download successful\n");
	                    }
	                    break;
	                case NOT_SUPPORTED:
	                    if (VERBOSITY_QUIET < g_verbosity)
	                    {
	                        printf("Firmware Download not supported\n");
	                    }
	                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
	                    break;
	                default:
	                    if (VERBOSITY_QUIET < g_verbosity)
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
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    perror("fopen");
	                    printf("Couldn't open file %s\n", DOWNLOAD_FW_FILENAME_FLAG);
	                }
	                exitCode = UTIL_EXIT_OPERATION_FAILURE;
	            }
	        }
	        
	        //If it is not deferred, lets activate
	        if ( (exitCode != UTIL_EXIT_OPERATION_FAILURE ) && (DOWNLOAD_FW_MODE != DL_FW_DEFERRED) )
	        {
	            switch (firmware_Download_Activate(&deviceList[deviceIter], 0))//TODO: Add slot numbers. 
	            {
	            case SUCCESS:
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("Firmware Activate/Commit successful\n");
	                }
	                // This is as of today (02/25/2016) Panther reports. 
	                switch(deviceList[deviceIter].os_info.last_error & 0x00FF)
	                {
	                case NVME_FW_DL_REQUIRES_SYS_RST:
	                case NVME_FW_DL_REQUIRES_NVM_RST:
	                case NVME_FW_DL_ON_NEXT_RST:
	                    printf("\n-- Please power cycle the system --\n\n");
	                    break;
	                default:
	                    printf("\n-- WARN: Firmware Activate/Commit returned Status 0x%X --\n\n",deviceList[deviceIter].os_info.last_error & 0x00FF);
	                    break;
	                }
	
	                break;
	            case NOT_SUPPORTED:
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("Firmware Activate not supported\n");
	                }
	                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
	                break;
	            default:
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("Firmware Activate failed\n");
	                }
	                exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                break;
	            }
	        }
	    }
	
	    if (TRANSITION_POWER_STATE_TO >= 0)
	    {
	        switch (transition_Power_State(&deviceList[deviceIter], TRANSITION_POWER_STATE_TO) )
	        {
	        case SUCCESS:
	            if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("\nSuccessfully transitioned to power state %d.\n",TRANSITION_POWER_STATE_TO);
	                printf("\nHint:Use --checkPowerMode option to check the new Power Mode State.\n\n");
	            }
	            break;
	        case NOT_SUPPORTED:
	            if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("Transitioning power modes not allowed on this device\n");
	            }
	            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
	            break;
	        default:
	            if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("ERROR: Could not transition to the new power state %d\n",TRANSITION_POWER_STATE_TO);
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
	            if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("Checking current power mode not supported on this device.\n");
	            }
	            break;
	        default:
	            exitCode = UTIL_EXIT_OPERATION_FAILURE;
	            if (VERBOSITY_QUIET < g_verbosity)
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
	            if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("Extended smart logs not supported.\n");
	            }
	            break;
	        default:
	            exitCode = UTIL_EXIT_OPERATION_FAILURE;
	            if (VERBOSITY_QUIET < g_verbosity)
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
	            if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("Clear Pcie correctable errors not supported.\n");
	            }
	            break;
	        default:
	            exitCode = UTIL_EXIT_OPERATION_FAILURE;
	            if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("Failed fetch Clear Pcie correctable errors.\n");
	            }
	            break;
	        }
	    }
	
	
	    if (FORMAT_UNIT_FLAG)
	    {
	        if (VERBOSITY_QUIET < g_verbosity)
	        {
	            printf("Format Unit\n");
	        }
	        if (DATA_ERASE_FLAG)
	        {
	            if (!(FORMAT_SECTOR_SIZE))
	            {
	                FORMAT_SECTOR_SIZE = (uint32_t)pow((double)2,\
	                                 (double)deviceList[deviceIter].drive_info.IdentifyData.nvme.ns.lbaf[deviceList[deviceIter].drive_info.IdentifyData.nvme.ns.flbas & 0x0F].lbaDS);
	
	            }
	            printf("Formatting using sector size %"PRIu8" \n",FORMAT_SECTOR_SIZE);
	            uint64_t formatFlags = FORMAT_NVME_ERASE_USER_DATA;
	            if (deviceList[deviceIter].drive_info.IdentifyData.nvme.ctrl.fna & BIT2)
	            {
	                formatFlags = FORMAT_NVME_CRYPTO_ERASE;
	            }
	
	            switch (run_NVMe_Format(&deviceList[deviceIter], FORMAT_SECTOR_SIZE, formatFlags) )
	            {
	            case SUCCESS:
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                        printf("Successfully Formated the device.\n");
	                }
	                break;
	            case NOT_SUPPORTED:
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("Format Unit Not Supported.\n");
	                }
	                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
	                break;
	            default:
	                if (VERBOSITY_QUIET < g_verbosity)
	                {
	                    printf("Format Unit Failed.\n");
	                }
	                exitCode = UTIL_EXIT_OPERATION_FAILURE;
	                break;
	            }
	        }
	        else
	        {
	            if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("\n");
	                printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
	                printf("to the command line arguments to run a format unit.\n\n");
	                printf("e.g.: %s -d %s --%s current --confirm %s\n\n", util_name, deviceHandleExample, FORMAT_UNIT_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
	            }
	        }
	    }
	}



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

    //the test options
    printf("Utility arguments\n");
    printf("=================\n");
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Device_Information_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here
    print_Check_Power_Mode_Help(shortUsage);
    print_Transition_Power_State_Help(shortUsage);
    print_Firmware_Download_Help(shortUsage);
    print_NVMe_Firmware_Download_Mode_Help(shortUsage);
    print_Get_Features_Help(shortUsage);
    print_NVMe_Get_Log_Help(shortUsage);
    print_NVMe_Get_Tele_Help(shortUsage);
    print_pcierr_Help(shortUsage);
    print_extSmatLog_Help(shortUsage);
    print_Output_Mode_Help(shortUsage);
    print_NVMe_Temp_Stats_Help(shortUsage);
    print_NVMe_Pci_Stats_Help(shortUsage);

    //data destructive commands
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //utility data destructive tests/operations go here
    print_NVME_Format_Unit_Help(shortUsage);

    //utility options
    printf("\nUtility Options\n");
    printf("===============\n");
    print_Verbose_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_Version_Help(shortUsage, util_name);
    print_License_Help(shortUsage);
    print_Echo_Command_Line_Help(shortUsage);
    print_Help_Help(shortUsage);


}
