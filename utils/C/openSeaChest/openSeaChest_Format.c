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
// \file openSeaChest_Format.c Binary command line that performs various format methods on a device.

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
#include "format_unit.h"
#include "set_sector_size.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_FormatUnit";
const char *buildVersion = "1.2.0";

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
    DATA_ERASE_VAR
    PARTIAL_DATA_ERASE_VAR
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

    FORMAT_UNIT_VARS
    FAST_FORMAT_VAR
    FORMAT_UNIT_OPTION_FLAGS
    PATTERN_VARS
    DISPLAY_LBA_VAR
    PROGRESS_VAR
    MAX_LBA_VARS
    POLL_VAR
    SHOW_FORMAT_STATUS_LOG_VAR
    SHOW_SUPPORTED_PROTECTION_TYPES_VAR
    SHOW_SUPPORTED_SECTOR_SIZES_VAR
    SET_SECTOR_SIZE_VARS

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
        CONFIRM_LONG_OPT,
        POLL_LONG_OPT,
        PROGRESS_LONG_OPT,
        FORMAT_UNIT_LONG_OPT,
        FAST_FORMAT_LONG_OPT,
        DISPLAY_LBA_LONG_OPT,
        PATTERN_LONG_OPT,
        FORMAT_UNIT_ADDITIONAL_OPTIONS,
        SHOW_FORMAT_STATUS_LOG_LONG_OPT,
        SHOW_SUPPORTED_PROTECTION_TYPES_LONG_OPT,
        SET_SECTOR_SIZE_LONG_OPT,
        SHOW_SUPPORTED_SECTOR_SIZES_LONG_OPT,
        LONG_OPT_TERMINATOR
    };

    g_verbosity = VERBOSITY_DEFAULT;

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
                else if (strlen(optarg) == strlen(PARTIAL_DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, PARTIAL_DATA_ERASE_ACCEPT_STRING, strlen(PARTIAL_DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    PARTIAL_DATA_ERASE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
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
            else if (strcmp(longopts[optionIndex].name, FORMAT_UNIT_LONG_OPT_STRING) == 0)
            {
                FORMAT_UNIT_FLAG = true;
                if (strcmp(optarg, "current") != 0)
                {
                    //set the sector size
                    FORMAT_SECTOR_SIZE = (uint32_t)atoi(optarg);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SET_SECTOR_SIZE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_SECTOR_SIZE_LONG_OPT_STRING))) == 0)
            {
                SET_SECTOR_SIZE_FLAG = true;
                SET_SECTOR_SIZE_SIZE = (uint32_t)atoi(optarg);
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
            else if (strcmp(longopts[optionIndex].name, FAST_FORMAT_LONG_OPT_STRING) == 0)
            {
                FAST_FORMAT_FLAG = (eFormatType)atoi(optarg);
            }
            else if (strcmp(longopts[optionIndex].name, FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT_STRING) == 0)
            {
                FORMAT_UNIT_PROTECTION_TYPE = (uint8_t)atoi(optarg);
                FORMAT_UNIT_PROECTION_TYPE_FROM_USER = true;
            }
            else if (strcmp(longopts[optionIndex].name, FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT_LONG_OPT_STRING) == 0)
            {
                FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT = (uint8_t)atoi(optarg);
                FORMAT_UNIT_PROECTION_INTERVAL_EXPONENT_FROM_USER = true;
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
                    char *colonLocation = strstr(optarg, ":") + 1;//adding 1 to offset just beyond the colon for parsing the remaining data
                    if (strncmp("file:", optarg, 5) == 0)
                    {
                        FILE *patternFile = NULL;
                        char *filename = (char*)calloc(strlen(colonLocation) + 1, sizeof(char));
                        if (!filename)
                        {
                            exit(UTIL_EXIT_CANNOT_OPEN_FILE);
                        }
                        strcpy(filename, colonLocation);
                        //open file
                        if (NULL == (patternFile = fopen(filename, "rb")))
                        {
                            printf("Unable to open file \"%s\" for pattern\n", filename);
                            exit(UTIL_EXIT_CANNOT_OPEN_FILE);
                        }
                        //read contents into buffer
                        fread(PATTERN_BUFFER, sizeof(uint8_t), M_Min(PATTERN_BUFFER_LENGTH, get_File_Size(patternFile)), patternFile);
                        //close file
                        fclose(patternFile);
                        safe_Free(filename);
                    }
                    else if (strncmp("increment:", optarg, 10) == 0)
                    {
                        uint8_t incrementStart = (uint8_t)atoi(colonLocation);
                        fill_Incrementing_Pattern_In_Buffer(incrementStart, PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
                    }
                    else if (strncmp("repeat:", optarg, 7) == 0)
                    {
                        //if final character is a lower case h, it's an hex pattern
                        if (colonLocation[strlen(colonLocation) - 1] == 'h' && strlen(colonLocation) == 9)
                        {
                            uint32_t hexPattern = (uint32_t)strtol(colonLocation, NULL, 16);
                            //TODO: add endianness check before byte swap
                            byte_Swap_32(&hexPattern);
                            fill_Hex_Pattern_In_Buffer(hexPattern, PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
                        }
                        else
                        {
                            fill_ASCII_Pattern_In_Buffer(colonLocation, (uint32_t)strlen(colonLocation), PATTERN_BUFFER, PATTERN_BUFFER_LENGTH);
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
		case AGRESSIVE_SCAN_SHORT_OPT:
			AGRESSIVE_SCAN_FLAG = true;
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
        case PROGRESS_SHORT_OPT: //get test progress for a specific test
            PROGRESS_CHAR = optarg;
            break;
        case QUIET_SHORT_OPT: //quiet mode
            g_verbosity = VERBOSITY_QUIET;
            break;
        case SCAN_FLAGS_SHORT_OPT://scan flags
            SCAN_FLAGS_SUBOPT_PARSING;
            break;
        case '?': //unknown option
            printf("%s: Unable to parse %s command line option\nPlease use --%s for more information.\n", util_name, argv[optind - 1], HELP_LONG_OPT_STRING);
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
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

	if (SCAN_FLAG || AGRESSIVE_SCAN_FLAG)
    {
        unsigned int scanControl = DEFAULT_SCAN;
		if(AGRESSIVE_SCAN_FLAG)
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
        scan_And_Print_Devs(scanControl, NULL);
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
        if (VERBOSITY_QUIET < g_verbosity)
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
        || FORMAT_UNIT_FLAG
        || DISPLAY_LBA_FLAG
        || (PROGRESS_CHAR != NULL)
        || SHOW_FORMAT_STATUS_LOG_FLAG
        || SHOW_SUPPORTED_PROTECTION_TYPES_FLAG
        || SET_SECTOR_SIZE_FLAG
        || SHOW_SUPPORTED_SECTOR_SIZES_FLAG
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
#else
            deviceList[handleIter].os_info.fd = INVALID_HANDLE_VALUE;
#endif
            deviceList[handleIter].dFlags = flags;

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
            if ((deviceList[handleIter].os_info.fd < 0) || (ret == FAILURE || ret == PERMISSION_DENIED))
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
                /*if (VERBOSITY_QUIET < g_verbosity)
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
            if (strncmp(MODEL_STRING_FLAG, deviceList[deviceIter].drive_info.product_identification, M_Min(strlen(MODEL_STRING_FLAG), strlen(deviceList[deviceIter].drive_info.product_identification))))
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

        //check for child model number match
        if (CHILD_MODEL_MATCH_FLAG)
        {
            if (strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN) == 0 || strncmp(CHILD_MODEL_STRING_FLAG, deviceList[deviceIter].drive_info.bridge_info.childDriveMN, M_Min(strlen(CHILD_MODEL_STRING_FLAG), strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN))))
            {
                if (VERBOSITY_QUIET < g_verbosity)
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
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input child firmware revision: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.bridge_info.childDriveFW, CHILD_FW_STRING_FLAG);
                }
                continue;
            }
        }

        if (FORCE_SCSI_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tForcing SCSI Drive\n");
            }
            deviceList[deviceIter].drive_info.drive_type = SCSI_DRIVE;
        }

        if (FORCE_ATA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tForcing ATA Drive\n");
            }
            deviceList[deviceIter].drive_info.drive_type = ATA_DRIVE;
        }

        if (FORCE_ATA_PIO_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
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
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tAttempting to force ATA Drive commands in DMA Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaMode = ATA_DMA_MODE_DMA;
        }

        if (FORCE_ATA_UDMA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tAttempting to force ATA Drive commands in UDMA Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaMode = ATA_DMA_MODE_UDMA;
        }

        if (VERBOSITY_QUIET < g_verbosity)
        {
            printf("\n%s - %s - %s - %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification, deviceList[deviceIter].drive_info.serialNumber, print_drive_type(&deviceList[deviceIter]));
        }

        //now start looking at what operations are going to be performed and kick them off
        if (DEVICE_INFO_FLAG)
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

        if (TEST_UNIT_READY_FLAG)
        {
            scsiStatus returnedStatus = { 0 };
            ret = scsi_Test_Unit_Ready(&deviceList[deviceIter], &returnedStatus);
            if ((ret == SUCCESS) && (returnedStatus.senseKey == SENSE_KEY_NO_ERROR))
            {
                printf("READY\n");
            }
            else
            {
                eVerbosityLevels tempVerbosity = g_verbosity;
                printf("NOT READY\n");
                g_verbosity = VERBOSITY_COMMAND_NAMES;//the function below will print out a sense data translation, but only it we are at this verbosity or higher which is why it's set before this call.
                check_Sense_Key_ASC_ASCQ_And_FRU(&deviceList[deviceIter], returnedStatus.senseKey, returnedStatus.acq, returnedStatus.ascq, returnedStatus.fru);
                g_verbosity = tempVerbosity;//restore it back to what it was now that this is done.
            }
        }

        if (DISPLAY_LBA_FLAG)
        {
            uint8_t *displaySector = (uint8_t*)calloc(deviceList[deviceIter].drive_info.deviceBlockSize * sizeof(uint8_t), sizeof(uint8_t));
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
            safe_Free(displaySector);
        }

        //Show supported protection types
        if (SHOW_SUPPORTED_PROTECTION_TYPES_FLAG)
        {
            protectionSupport supportedProtectionTypes;
            memset(&supportedProtectionTypes, 0, sizeof(protectionSupport));
            switch (get_Supported_Protection_Types(&deviceList[deviceIter], &supportedProtectionTypes))
            {
            case SUCCESS:
                show_Supported_Protection_Types(&supportedProtectionTypes);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Protection types are not supported on this drive\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to read supported protection types\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        //show supported sector sizes and protection types VPD page
        if (SHOW_SUPPORTED_SECTOR_SIZES_FLAG)
        {
            switch (show_Supported_Sector_Sizes(&deviceList[deviceIter]))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to get supported sector sizes from device!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        //Show format status log
        if (SHOW_FORMAT_STATUS_LOG_FLAG)
        {
            formatStatus formatStatusInfo;
            memset(&formatStatusInfo, 0, sizeof(formatStatus));
            switch (get_Format_Status(&deviceList[deviceIter], &formatStatusInfo))
            {
            case SUCCESS:
                show_Format_Status_Log(&formatStatusInfo);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("The format status log is not supportd on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to read the format status log.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
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
                bool currentBlockSize = true;
                runFormatUnitParameters formatUnitParameters;
                memset(&formatUnitParameters, 0, sizeof(runFormatUnitParameters));
                if (FORMAT_SECTOR_SIZE)
                {
                    currentBlockSize = false;
                }
                formatUnitParameters.formatType = FAST_FORMAT_FLAG;
                formatUnitParameters.currentBlockSize = currentBlockSize;
                formatUnitParameters.newBlockSize = FORMAT_SECTOR_SIZE;
                formatUnitParameters.gList = NULL;
                formatUnitParameters.glistSize = 0;
                formatUnitParameters.completeList = FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG;
                formatUnitParameters.defaultFormat = true;//Setting this to ture as the default UNLESS one of the format option flags is given!!! - TJE
                formatUnitParameters.disablePrimaryList = FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG;
                formatUnitParameters.disableCertification = FORMAT_UNIT_DISABLE_CERTIFICATION;
                formatUnitParameters.stopOnListError = FORMAT_UNIT_STOP_ON_LIST_ERROR;
                if (PATTERN_FLAG)
                {
                    formatUnitParameters.pattern = PATTERN_BUFFER;
                    formatUnitParameters.patternLength = deviceList[deviceIter].drive_info.deviceBlockSize;
                }
                if (formatUnitParameters.disablePrimaryList || formatUnitParameters.disableCertification || formatUnitParameters.stopOnListError || PATTERN_FLAG)
                {
                    //format options are being specified, so we need to turn this off!!! Code refactor would probably be better in the operations lib, but this is OK for now.
                    formatUnitParameters.defaultFormat = false;
                }
                //formatUnitParameters.defaultFormat = FORMAT_UNIT_DEFAULT_FORMAT;//change to true when a user wants a default format...This is basically obsolete now due to the above code, but left in place in case someone wants to try some weird bit combinations
                formatUnitParameters.disableImmediate = FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE;
                //Set the same protection information as we discovered first.
                formatUnitParameters.protectionType = deviceList[deviceIter].drive_info.currentProtectionType;
                formatUnitParameters.protectionIntervalExponent = deviceList[deviceIter].drive_info.piExponent;
                //override protection info if we were asked to.
                if (FORMAT_UNIT_PROECTION_TYPE_FROM_USER)
                {
                    formatUnitParameters.protectionType = FORMAT_UNIT_PROTECTION_TYPE;
                    if (formatUnitParameters.protectionType < 2)
                    {
                        //clearing the exponent value since it is only valid for PI 3.
                        formatUnitParameters.protectionIntervalExponent = 0;
                    }
                }
                if (FORMAT_UNIT_PROECTION_INTERVAL_EXPONENT_FROM_USER)
                {
                    formatUnitParameters.protectionIntervalExponent = FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT;
                }
                formatUnitParameters.securityInitialize = false;
                int formatRet = run_Format_Unit(&deviceList[deviceIter], formatUnitParameters, POLL_FLAG);
                switch (formatRet)
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < g_verbosity)
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
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Format Unit Not Supported or invalid option combination provided!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Format Unit Failed!\n");
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

        if (SET_SECTOR_SIZE_FLAG)
        {
            if (DATA_ERASE_FLAG)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Set Sector Size to %"PRIu32"\n", SET_SECTOR_SIZE_SIZE);
                }
                switch (set_Sector_Configuration(&deviceList[deviceIter], SET_SECTOR_SIZE_SIZE))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Successfully set sector size to %"PRIu32"\n", SET_SECTOR_SIZE_SIZE);
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Setting sector size not supported on this device\n");
                        if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
                        {
                            printf("For SCSI Drives, try a format unit operation\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Failed to set sector size!\n");
                        if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
                        {
                            printf("For SCSI Drives, try a format unit operation\n");
                        }
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
                    printf("to the command line arguments to run a set sector size operation.\n\n");
                    printf("e.g.: %s -d %s --%s 4096 --%s %s\n\n", util_name, deviceHandleExample, SET_SECTOR_SIZE_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }
        
        if (PROGRESS_CHAR != NULL)
        {
            int result = UNKNOWN;
            //first take whatever was entered in progressTest and convert it to uppercase to do fewer string comparisons
            convert_String_To_Upper_Case(progressTest);
            //do some string comparisons to figure out what we are checking for progress on
            if (strcmp(progressTest, "FORMAT") == 0)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Getting Format Unit Progress.\n");
                }
                result = show_Format_Unit_Progress(&deviceList[deviceIter]);
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
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
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_SAT_12_Byte_CDB_Help(shortUsage);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    //the test options
    printf("\nUtility arguments\n");
    printf("=================\n");
    //Common (across utilities) - alphabetized
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Display_LBA_Help(shortUsage);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Poll_Help(shortUsage);
    print_Progress_Help(shortUsage, "format");
    print_Scan_Help(shortUsage, deviceHandleExample);
	print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    //multiple interfaces
    print_Set_Sector_Size_Help(shortUsage);
    print_Show_Supported_Sector_Sizes_Help(shortUsage);
    //SATA Only Options
    //printf("\n\tSATA Only:\n\n");

    //SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");
    print_Show_Format_Status_Log_Help(shortUsage);
    print_Show_Protection_Types_Supported_Help(shortUsage);

    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //utility data destructive tests/operations go here
    print_Pattern_Help(shortUsage);
    printf("\n\tSAS Only:\n\t=========\n");
    //print_Format_Default_Format_Help(shortUsage);
    print_Format_Disable_Certification_Help(shortUsage);
    print_Format_Disable_Primary_List_Help(shortUsage);
    print_Format_Discard_Grown_Defect_List_Help(shortUsage);
    print_Format_Disable_Immediate_Response_Help(shortUsage);
    print_Format_Protection_Interval_Exponent_Help(shortUsage);
    print_Format_Protection_Type_Help(shortUsage);
    print_Fast_Format_Help(shortUsage);
    print_Format_Unit_Help(shortUsage);
    print_Format_Security_Initialize_Help(shortUsage);
    print_Format_Stop_On_List_Error_Help(shortUsage);
}
