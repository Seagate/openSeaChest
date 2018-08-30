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
// \file OpenSeaChest_Erase.c Binary command line that performs various erase methods on a device.

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
#include "cmds.h"
#if !defined(DISABLE_TCG_SUPPORT)
#include "common_TCG.h"
#include "revertSP.h"
#include "revert.h"
#include "genkey.h"
#include "port_locking.h"
#endif
#include "operations.h"
#include "host_erase.h"
#include "ata_Security.h"
#include "sanitize.h"
#include "writesame.h"
#include "trim_unmap.h"
#include "drive_info.h"
#include "format_unit.h"
#include "depopulate.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_Erase";
const char *buildVersion = "1.7.4";

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
    PROGRESS_VAR
    DISPLAY_LBA_VAR
    PATTERN_VARS
    MAX_LBA_VARS
    //tool specific command line flags
    //generic erase
    POLL_VAR
    //tcg revert SP
    TCG_REVERT_SP_VARS
    //tcg revert
    TCG_REVERT_VAR
    //ata security
    ATA_SECURITY_ERASE_UTIL_VARS
    ATA_SECURITY_ERASE_DISABLE_PW_UTIL_VARS
    //sanitize
    SANITIZE_UTIL_VARS
    //write same
    WRITE_SAME_UTIL_VARS
    //tcgGenkey
    //bool                tcgGenKey                   = false;
    TRIM_UNMAP_VARS
    OVERWRITE_VARS
    FORMAT_UNIT_VARS
    FAST_FORMAT_VAR
    SHOW_ERASE_SUPPORT_VAR
    PERFORM_FASTEST_ERASE_VAR
    SHOW_PHYSICAL_ELEMENT_STATUS_VAR
    REMOVE_PHYSICAL_ELEMENT_VAR
    FORCE_SEAGATE_DEPOPULATE_COMMANDS_VAR

    //time flags
    HOURS_TIME_VAR
    MINUTES_TIME_VAR
    SECONDS_TIME_VAR

#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    HIDE_LBA_COUNTER_VAR

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
        POLL_LONG_OPT,
        CONFIRM_LONG_OPT,
        PROGRESS_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        //time vars
        HOURS_TIME_LONG_OPT,
        MINUTES_TIME_LONG_OPT,
        SECONDS_TIME_LONG_OPT,
        //tool specific command line options --These should probably be cleaned up to macros like the ones above and remove the short options and replace with numbers.
        OVERWRITE_LONG_OPTS,
        TRIM_LONG_OPTS,
        UNMAP_LONG_OPTS,
        TCG_REVERT_SP_LONG_OPT,
        TCG_REVERT_LONG_OPT,
        { "secureErase",            required_argument,          NULL,           'y'     }, //has a purposely unadvertized short option
        { "disableATASecurityPW",   required_argument,          NULL,           'D'     }, //has a purposely unadvertized short option
        { "sanitize",               required_argument,          NULL,           'e'     }, //has a purposely unadvertized short option
        WRITE_SAME_LONG_OPTS,
        SHOW_ERASE_SUPPORT_LONG_OPT,
        PERFORM_FASTEST_ERASE_LONG_OPT,
        FORMAT_UNIT_LONG_OPT,
        FAST_FORMAT_LONG_OPT,
        DISPLAY_LBA_LONG_OPT,
        PATTERN_LONG_OPT,
        HIDE_LBA_COUNTER_LONG_OPT,
        SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT,
        REMOVE_PHYSICAL_ELEMENT_LONG_OPT,
        FORCE_SEAGATE_DEPOPULATE_COMMANDS_LONG_OPT,
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
        args = getopt_long(argc, argv, "d:hisSF:Vv:qr:R:E:e:w:", longopts, &optionIndex);
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
            else if (strcmp(longopts[optionIndex].name, TRIM_RANGE_LONG_OPT_STRING) == 0 || strcmp(longopts[optionIndex].name, UNMAP_RANGE_LONG_OPT_STRING) == 0)
            {
                sscanf(optarg, "%"SCNu64, &TRIM_UNMAP_RANGE_FLAG);
            }
            else if (strcmp(longopts[optionIndex].name, TRIM_LONG_OPT_STRING) == 0 || strcmp(longopts[optionIndex].name, UNMAP_LONG_OPT_STRING) == 0)
            {
            	RUN_TRIM_UNMAP_FLAG = true;
                sscanf(optarg, "%"SCNu64, &TRIM_UNMAP_START_FLAG);
                if (0 == sscanf(optarg, "%"SCNu64, &TRIM_UNMAP_START_FLAG))
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
                sscanf(optarg, "%"SCNu64, &OVERWRITE_RANGE_FLAG);
            }
            else if (strcmp(longopts[optionIndex].name, OVERWRITE_LONG_OPT_STRING) == 0)
            {
                RUN_OVERWRITE_FLAG = true;
                if (0 == sscanf(optarg, "%"SCNu64, &OVERWRITE_START_FLAG))
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
                        print_Error_In_Cmd_Line_Args(OVERWRITE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, HOURS_TIME_LONG_OPT_STRING) == 0)
            {
                HOURS_TIME_FLAG = (uint8_t)atoi(optarg);
            }
            else if (strcmp(longopts[optionIndex].name, MINUTES_TIME_LONG_OPT_STRING) == 0)
            {
                MINUTES_TIME_FLAG = (uint16_t)atoi(optarg);
            }
            else if (strcmp(longopts[optionIndex].name, SECONDS_TIME_LONG_OPT_STRING) == 0)
            {
                SECONDS_TIME_FLAG = (uint32_t)atoi(optarg);
            }
            else if (strcmp(longopts[optionIndex].name, WRITE_SAME_RANGE_LONG_OPT_STRING) == 0)
            {
                sscanf(optarg, "%"SCNu64, &WRITE_SAME_RANGE_FLAG);
            }
            else if (strcmp(longopts[optionIndex].name, WRITE_SAME_LONG_OPT_STRING) == 0)
            {
                RUN_WRITE_SAME_FLAG = true;
                sscanf(optarg, "%"SCNu64, &WRITE_SAME_START_FLAG);
                if (0 == sscanf(optarg, "%"SCNu64, &WRITE_SAME_START_FLAG))
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
                        print_Error_In_Cmd_Line_Args(WRITE_SAME_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, TCG_REVERT_SP_LONG_OPT_STRING) == 0)
            {
                TCG_REVERT_SP_FLAG = true;
                TCG_REVERT_SP_PSID_FLAG = strdup(optarg);
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
            else if (strcmp(longopts[optionIndex].name, REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING) == 0)//REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING
            {
                REMOVE_PHYSICAL_ELEMENT_FLAG = (uint32_t)atoi(optarg);
            }
            break;
        case PROGRESS_SHORT_OPT: //get test progress for a specific test
            PROGRESS_CHAR = optarg;
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
        case 'D': //disableATASecurityPW [SeaChest | ASCIIPW] [user | master]
            ATA_SECURITY_ERASE_DISABLE_PW_SUBOPT_PARSE
            break;
        case 'e': //sanitize
            SANITIZE_SUBOPT_PARSE
            break;
        case 'y': //secure erase
            ATA_SECURITY_ERASE_SUBOPT_PARSE
            break;
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
        || TCG_REVERT_SP_FLAG
        || TCG_REVERT_FLAG
        || sanitize
        || RUN_WRITE_SAME_FLAG
        || runSecureErase
        || RUN_OVERWRITE_FLAG
        || RUN_TRIM_UNMAP_FLAG
        || disableATAPassword
        || (PROGRESS_CHAR != NULL)
        || SHOW_ERASE_SUPPORT_FLAG
        || PERFORM_FASTEST_ERASE_FLAG
        || FORMAT_UNIT_FLAG
        || DISPLAY_LBA_FLAG
        || SHOW_PHYSICAL_ELEMENT_STATUS_FLAG
        || REMOVE_PHYSICAL_ELEMENT_FLAG > 0
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
#if defined(VMK_CROSS_COMP)
            deviceList[handleIter].os_info.nvmeFd = NULL;
#endif
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
            if ((deviceList[handleIter].os_info.fd < 0) || 
#if defined(VMK_CROSS_COMP)
                (deviceList[handleIter].os_info.nvmeFd == NULL) ||
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

        //check for child model number match
        if (CHILD_MODEL_MATCH_FLAG)
        {
			if (strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN) == 0 || strstr(deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG) == NULL)
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

        if (SHOW_ERASE_SUPPORT_FLAG)
        {
            uint32_t overwriteTimeMinutes = 0;
            eraseMethod eraseMethodList[MAX_SUPPORTED_ERASE_METHODS];
            memset(&eraseMethodList, 0, sizeof(eraseMethod) * MAX_SUPPORTED_ERASE_METHODS);
#ifdef DISABLE_TCG_SUPPORT
            //TODO: Check the return method. 
            get_Supported_Erase_Methods(&deviceList[deviceIter], eraseMethodList, &overwriteTimeMinutes);
#else
            //get_Supported_Erase_Methods(&selectedDevice, &eraseMethodList);//switch this to method with TCG support later - TJE
            get_Supported_Erase_Methods_With_TCG(&deviceList[deviceIter], eraseMethodList, &overwriteTimeMinutes);
#endif
            print_Supported_Erase_Methods(&deviceList[deviceIter], eraseMethodList, &overwriteTimeMinutes);
        }

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
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

        if (SHOW_PHYSICAL_ELEMENT_STATUS_FLAG)
        {
            uint64_t depopTime = 0;
            bool depopSupport = is_Depopulation_Feature_Supported(&deviceList[deviceIter], &depopTime);
            if (depopSupport)
            {
                uint32_t numberOfDescriptors = 0;
                get_Number_Of_Descriptors(&deviceList[deviceIter], &numberOfDescriptors);
                if (numberOfDescriptors > 0)
                {
                    ptrPhysicalElement elementList = (ptrPhysicalElement)malloc(numberOfDescriptors * sizeof(physicalElement));
                    memset(elementList, 0, numberOfDescriptors * sizeof(physicalElement));
                    if (SUCCESS == get_Physical_Element_Descriptors(&deviceList[deviceIter], numberOfDescriptors, elementList))
                    {
                        show_Physical_Element_Descriptors(numberOfDescriptors, elementList, depopTime);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Failed to get physical element status.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("No physical elements were found on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("The Storage Element Depopulation feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        //check for fastest erase flag first since it may be used to set other flags for other erase methods
        if (PERFORM_FASTEST_ERASE_FLAG)
        {
            //check that we were given the confirmation for erasing all data
            if (DATA_ERASE_FLAG)
            {
                eraseMethod eraseMethodList[MAX_SUPPORTED_ERASE_METHODS];
                //set partial data erase flag to true in case setting the flags below require that one instead.
                PARTIAL_DATA_ERASE_FLAG = true;
#ifdef DISABLE_TCG_SUPPORT
                //TODO: Check the return method. 
                get_Supported_Erase_Methods(&deviceList[deviceIter], eraseMethodList, NULL);
#else
                //get_Supported_Erase_Methods(&selectedDevice, &eraseMethodList);//switch this to method with TCG support later - TJE
                get_Supported_Erase_Methods_With_TCG(&deviceList[deviceIter], eraseMethodList, NULL);
#endif                

                switch (eraseMethodList[0].eraseIdentifier)
                {
                case ERASE_TCG_REVERT:
                    TCG_REVERT_FLAG = true;
                    break;
                case ERASE_TCG_REVERT_SP:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("\nFastest Erase is RevertSP. Please use the --%s option with the drive PSID to perform this erase.\n", TCG_REVERT_SP_LONG_OPT_STRING);
                    }
                    break;
                case ERASE_TRIM_UNMAP:
                    RUN_TRIM_UNMAP_FLAG = true;
                    TRIM_UNMAP_START_FLAG = 0;
                    TRIM_UNMAP_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba;
                    break;
                case ERASE_SANITIZE_OVERWRITE:
                    sanitize = true;
                    sanoverwrite = true;
                    break;
                case ERASE_SANITIZE_BLOCK:
                    sanitize = true;
                    sanblockErase = true;
                    break;
                case ERASE_SANITIZE_CRYPTO:
                    sanitize = true;
                    sancryptoErase = true;
                    break;
                case ERASE_ATA_SECURITY_ENHANCED:
                    runSecureErase = true;
                    enhanced = true;
                    break;
                case ERASE_ATA_SECURITY_NORMAL:
                    runSecureErase = true;
                    break;
                case ERASE_WRITE_SAME:
                    RUN_WRITE_SAME_FLAG = true;
                    WRITE_SAME_START_FLAG = 0;
                    WRITE_SAME_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba;
                    break;
                case ERASE_OVERWRITE:
                    RUN_OVERWRITE_FLAG = true;
                    OVERWRITE_START_FLAG = 0;
                    OVERWRITE_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba;
                    break;
                case ERASE_FORMAT_UNIT:
                    FORMAT_UNIT_FLAG = goTrue;
                    break;
                default:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("\nAn error occured while trying to determine best possible erase. No erase will be performed.\n");
                    }
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to perform the quickest erase.\n\n");
                    printf("e.g.: %s -d %s --%s --confirm %s\n\n", util_name, deviceHandleExample, PERFORM_FASTEST_ERASE_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (REMOVE_PHYSICAL_ELEMENT_FLAG > 0)
        {
            if (DATA_ERASE_FLAG)
            {
                bool depopSupport = is_Depopulation_Feature_Supported(&deviceList[deviceIter], NULL);
                if (depopSupport)
                {
                    //TODO: add an option to allow setting a requested MaxLBA? Lets wait to see if a customer wants that option before we add it - TJE
                    switch (depopulate_Physical_Element(&deviceList[deviceIter], REMOVE_PHYSICAL_ELEMENT_FLAG, 0))
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Successfully sent the remove physical element command!\n");
                            printf("The device may take a long time before it is ready to accept all commands again.\n");
                            printf("Use the --%s option to check if the depopulate is still in progress or complete.\n", SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT_STRING);
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("This operation is not supported on this drive or a bad element ID was given.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Failed to depopulate element %" PRIu32".\n", REMOVE_PHYSICAL_ELEMENT_FLAG);
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
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to run a remove physical element command.\n\n");
                    printf("e.g.: %s -d %s --%s element# --confirm %s\n\n", util_name, deviceHandleExample, REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

#ifdef DISABLE_TCG_SUPPORT

#else
        if (TCG_REVERT_SP_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\nRevertSP\n");
            }
            if (DATA_ERASE_FLAG)
            {
                if (strlen(TCG_REVERT_SP_PSID_FLAG) < 32)
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("\tPSID too short. PSID must be 32 characters long.\n");
                    }
                    return UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                }
                switch (revert_SP(&deviceList[deviceIter], TCG_REVERT_SP_PSID_FLAG))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("RevertSP Successful!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("RevertSP is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("RevertSP Failure!\n");
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
                    printf("to the command line arguments to run a revertSP.\n\n");
                    printf("e.g.: %s -d %s --%s PUTTHIRTYTWOCHARACTERPSIDHERE --confirm %s\n\n", util_name, deviceHandleExample, TCG_REVERT_SP_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }
        if (TCG_REVERT_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\nRevert\n");
            }
            if (DATA_ERASE_FLAG)
            {
                switch (revert(&deviceList[deviceIter]))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Revert Successful!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Revert is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Revert Failure!\n");
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
                    printf("to the command line arguments to run a revert.\n\n");
                    printf("e.g.: %s -d %s --%s --confirm %s\n\n", util_name, deviceHandleExample, TCG_REVERT_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }
#endif

        if (sanitize)
        {
            int sanitizeResult = UNKNOWN;
            sanitizeFeaturesSupported sanitizeOptions;
            memset(&sanitizeOptions, 0, sizeof(sanitizeFeaturesSupported));
            sanitizeResult = get_Sanitize_Device_Features(&deviceList[deviceIter], &sanitizeOptions);
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\nSanitize\n");
            }
            if (sanitizeInfo)
            {
                if (SUCCESS == sanitizeResult)
                {
                    if (sanitizeOptions.crypto || sanitizeOptions.blockErase || sanitizeOptions.overwrite)
                    {
                        printf("\tThe following sanitize commands are supported:\n");
                        if (sanitizeOptions.crypto)
                        {
                            printf("\t\tCrypto Erase\t--%s cryptoerase\n", SANITIZE_LONG_OPT_STRING);
                        }
                        if (sanitizeOptions.blockErase)
                        {
                            printf("\t\tBlock Erase\t--%s blockerase\n", SANITIZE_LONG_OPT_STRING);
                        }
                        if (sanitizeOptions.overwrite)
                        {
                            printf("\t\tOverwrite\t--%s overwrite\n", SANITIZE_LONG_OPT_STRING);
                        }
                    }
                    else
                    {
                        printf("\tSanitize commands are not supported by this device.\n");
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("\tSanitize Features not supported.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
            }
            //if sanitize features are supported and the user has input a sanitize command to run, then we can kick them off
            if (sanitizeOptions.sanitizeCmdEnabled)
            {
                if (DATA_ERASE_FLAG && !(sanfreezelock || sanAntiFreezeLock || sanitizeInfo))
                {
                    bool sanitizeCommandRun = false;
                    if (sanblockErase)
                    {
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Starting Sanitize Block Erase\n");
                        }
                        sanitizeResult = run_Sanitize_Operation(&deviceList[deviceIter], SANITIZE_BLOCK_ERASE, POLL_FLAG, NULL, 0);
                        sanitizeCommandRun = true;
                    }
                    if (sancryptoErase)
                    {
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Starting Sanitize Crypto Scramble\n");
                        }
                        sanitizeResult = run_Sanitize_Operation(&deviceList[deviceIter], SANITIZE_CRYPTO_ERASE, POLL_FLAG, NULL, 0);
                        sanitizeCommandRun = true;
                    }
                    if (sanoverwrite)
                    {
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Starting Sanitize Overwrite Erase\n");
                        }
                        sanitizeResult = run_Sanitize_Operation(&deviceList[deviceIter], SANITIZE_OVERWRITE_ERASE, POLL_FLAG, PATTERN_BUFFER, deviceList[deviceIter].drive_info.deviceBlockSize);
                        sanitizeCommandRun = true;
                    }
                    if (sanitizeCommandRun)
                    {
                        switch (sanitizeResult)
                        {
                        case SUCCESS:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                if (POLL_FLAG)
                                {
                                    printf("Sanitize command passed!\n");
                                }
                                else
                                {
                                    printf("Sanitize command has been started. Use the --%s sanitize option\n", PROGRESS_LONG_OPT_STRING);
                                    printf("to check for progress.\n");
                                }
                            }
                            break;
                        case FAILURE:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Sanitize command failed!\n");
                                #if defined (_WIN32)//TODO: handle Win PE somehow when we support WinPE
                                printf("\tNote: Windows 8 and higher block sanitize commands.\n");
                                #endif
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        case IN_PROGRESS:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("A sanitize command is already in progress.\n");
                            }
                            break;
                        case FROZEN:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Cannot run sanitize operation because drive is sanitize frozen.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Sanitize command not supported by the device.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        case OS_PASSTHROUGH_FAILURE:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Sanitize command was blocked by the OS.\n");
                                #if defined (_WIN32)//TODO: handle Win PE somehow when we support WinPE
                                printf("\tNote: Windows 8 and higher block sanitize commands.\n");
                                #endif
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Unknown error in sanitize command.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                    }
                }
                else if (sanfreezelock)
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Sending Sanitize Freeze Lock Command.\n");
                    }
                    sanitizeResult = run_Sanitize_Operation(&deviceList[deviceIter], SANTIZIE_FREEZE_LOCK, false, NULL, 0);
                    switch (sanitizeResult)
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Sanitize Freezelock passed!\n");
                        }
                        break;
                    case FAILURE:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Sanitize Freezelock failed!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Sanitize Freezelock not supported.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    case IN_PROGRESS:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("A Sanitize operation is already in progress, freezelock cannot be completed.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    default:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Unknown error in sanitize command.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else if (sanAntiFreezeLock)
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Sending Sanitize Anti Freeze Lock Command.\n");
                    }
                    sanitizeResult = run_Sanitize_Operation(&deviceList[deviceIter], SANITIZE_ANTI_FREEZE_LOCK, false, NULL, 0);
                    switch (sanitizeResult)
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Sanitize Anti Freezelock passed!\n");
                        }
                        break;
                    case FAILURE:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Sanitize Anti Freezelock failed!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Sanitize Anti Freezelock not supported.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    case IN_PROGRESS:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("A Sanitize operation is already in progress, anti freezelock cannot be completed.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    default:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Unknown error in sanitize command.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else if (!sanitizeInfo)
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("\n");
                        printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                        printf("to the command line arguments to run a sanitize operation.\n\n");
                        printf("e.g.: %s -d %s --%s blockerase --confirm %s\n\n", util_name, deviceHandleExample, SANITIZE_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                    }
                }
            }
            else if (!sanitizeInfo)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("\tSanitize Features not supported.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
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
                formatUnitParameters.completeList = false;
                formatUnitParameters.disablePrimaryList = false;
                formatUnitParameters.disableCertification = false;
                formatUnitParameters.stopOnListError = false;
                formatUnitParameters.defaultFormat = true;//This is true unless we need to write a pattern since we aren't setting any of the other format option flags!!
                formatUnitParameters.protectionType = deviceList[deviceIter].drive_info.currentProtectionType;
                formatUnitParameters.protectionIntervalExponent = deviceList[deviceIter].drive_info.piExponent;
                if (PATTERN_FLAG)
                {
                    formatUnitParameters.pattern = PATTERN_BUFFER;
                    formatUnitParameters.patternLength = deviceList[deviceIter].drive_info.deviceBlockSize;
                    formatUnitParameters.defaultFormat = false;//This is true unless we need to write a pattern!!
                }
                formatUnitParameters.securityInitialize = false;
                int formatRet = UNKNOWN;
                if (PATTERN_FLAG)
                {
                    formatRet = run_Format_Unit(&deviceList[deviceIter], formatUnitParameters, POLL_FLAG);
                }
                else
                {
                    formatRet = run_Format_Unit(&deviceList[deviceIter], formatUnitParameters, POLL_FLAG);
                }
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
                        printf("Format Unit Not Supported!\n");
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

        if (runSecureErase)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Secure Erase\n");
            }
            if (DATA_ERASE_FLAG)
            {
                if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Security erase is only available on ATA drives.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
                else
                {
                    switch (run_ATA_Security_Erase(&deviceList[deviceIter], enhanced, false, "SeaChest", POLL_FLAG))
                    {
                    case SUCCESS:
                        break;
                    case NOT_SUPPORTED:
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    case FROZEN:
                        exitCode = UTIL_EXIT_OPERATION_ABORTED;
                        break;
                    default:
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to run a secure erase.\n\n");
                    printf("e.g.: %s -d %s --secureErase normal --confirm %s\n\n", util_name, deviceHandleExample, DATA_ERASE_ACCEPT_STRING);
                }
            }
        }
        if (RUN_WRITE_SAME_FLAG)
        {
            uint64_t localStartLBA = WRITE_SAME_START_FLAG;
            uint64_t localRange = WRITE_SAME_RANGE_FLAG;
            if (USE_MAX_LBA)
            {
                localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                if (WRITE_SAME_RANGE_FLAG == 0 || WRITE_SAME_RANGE_FLAG > 1)
                {
                    localRange = 1;
                }
            }
            else if (USE_CHILD_MAX_LBA)
            {
                localStartLBA = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
                if (WRITE_SAME_RANGE_FLAG == 0 || WRITE_SAME_RANGE_FLAG > 1)
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
                if (WRITE_SAME_RANGE_FLAG == 0 || WRITE_SAME_RANGE_FLAG == UINT64_MAX || (localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
                }
                if (PARTIAL_DATA_ERASE_FLAG)
                {
		            int writeSameRet = UNKNOWN;
		            if (PATTERN_FLAG)
		            {
		                writeSameRet = writesame(&deviceList[deviceIter], localStartLBA, localRange, POLL_FLAG, PATTERN_BUFFER, deviceList[deviceIter].drive_info.deviceBlockSize);
		            }
		            else
		            {
		                writeSameRet = writesame(&deviceList[deviceIter], localStartLBA, localRange, POLL_FLAG, NULL, 0);
		            }
		            //now we need to send the erase
		            switch (writeSameRet)
	                {
	                case SUCCESS:
	                    if (VERBOSITY_QUIET < g_verbosity)
	                    {
			                if (POLL_FLAG && deviceList[deviceIter].drive_info.drive_type == ATA_DRIVE)
			                {
			                    printf("Successfully erased LBAs %"PRIu64" to %"PRIu64" using write same\n", localStartLBA, localStartLBA + localRange - 1);
			                }
			                else
			                {
			                    printf("Erasing LBAs %"PRIu64" to %"PRIu64" using write same in the background.\n", localStartLBA, localStartLBA + localRange - 1);
			                    if (deviceList[deviceIter].drive_info.drive_type == ATA_DRIVE)
			                    {
			                        printf("\tUse --poll to see progress when using the write same command line option.\n");
			                    }
			                }
			            }
		                break;
		            case NOT_SUPPORTED:
		                if (VERBOSITY_QUIET < g_verbosity)
		                {
		                    printf("Write same is not supported on this device, or the range is larger than the device supports.\n");
		                }
		                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
		                break;
		            default:
		                if (VERBOSITY_QUIET < g_verbosity)
		                {
		                    printf("Failed to erase LBAs %"PRIu64" to %"PRIu64" with write same\n", localStartLBA, localStartLBA + localRange - 1);
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
		                printf("You must add the flag:\n\"%s\" \n", PARTIAL_DATA_ERASE_ACCEPT_STRING);
		                printf("to the command line arguments to run a writesame operation.\n\n");
		                printf("e.g.: %s -d %s --writeSame 0 --writeSameRange 4096 --confirm %s\n\n", util_name, deviceHandleExample, PARTIAL_DATA_ERASE_ACCEPT_STRING);
	                }
	            }
	        }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("An invalid start LBA has been entered. Please enter a valid value.\n");
                }
            }
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
            	if(localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
            	{
            		localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
            	}
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG == UINT64_MAX || (localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba)
		        {
		            localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
		        }
		        if (PARTIAL_DATA_ERASE_FLAG)
		        {
		            switch (trim_Unmap_Range(&deviceList[deviceIter], localStartLBA, localRange))
		            {
		            case SUCCESS:
		                if (VERBOSITY_QUIET < g_verbosity)
		                {
		                    printf("Successfully trimmed/unmapped LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
		                }
		                break;
		            case NOT_SUPPORTED:
		                if (VERBOSITY_QUIET < g_verbosity)
		                {
		                    printf("Trim/Unmap is not supported on this drive type.\n");
		                }
		                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
		                break;
		            default:
		                if (VERBOSITY_QUIET < g_verbosity)
		                {
		                    printf("Failed to trim/unmap LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
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
		                printf("You must add the flag:\n\"%s\" \n", PARTIAL_DATA_ERASE_ACCEPT_STRING);
		                printf("to the command line arguments to run a trim/unmap operation.\n\n");
		                printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample, TRIM_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, PARTIAL_DATA_ERASE_ACCEPT_STRING);
		            }
		        }
            }
            else
            {
            	exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            	if (VERBOSITY_QUIET < g_verbosity)
	            {
	                printf("An invalid start LBA has been entered. Please enter a valid value.\n");
                }
            }
        }

        if (RUN_OVERWRITE_FLAG)
        {
            if (PARTIAL_DATA_ERASE_FLAG)
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
                    if(localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
                    {
                    	localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                    }
                    //range based overwrite
                    if ((localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba || localRange == UINT64_MAX || localRange == 0)
                    {
                        localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
                    }
                    if (PATTERN_FLAG)
                    {
                        overwriteRet = erase_Range(&deviceList[deviceIter], localStartLBA, localStartLBA + localRange, PATTERN_BUFFER, deviceList[deviceIter].drive_info.deviceBlockSize, HIDE_LBA_COUNTER);
                    }
                    else
                    {
                        overwriteRet = erase_Range(&deviceList[deviceIter], localStartLBA, localStartLBA + localRange, NULL, 0, HIDE_LBA_COUNTER);
                    }
                    switch (overwriteRet)
                    {
                    case SUCCESS:
                        exitCode = 0;
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Successfully overwrote LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Erase Range is not supported on this drive type at this time.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < g_verbosity)
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
                        if (PATTERN_FLAG)
                        {
                            overwriteRet = erase_Time(&deviceList[deviceIter], OVERWRITE_START_FLAG, overwriteSeconds, PATTERN_BUFFER, deviceList[deviceIter].drive_info.deviceBlockSize, HIDE_LBA_COUNTER);
                        }
                        else
                        {
                            overwriteRet = erase_Time(&deviceList[deviceIter], OVERWRITE_START_FLAG, overwriteSeconds, NULL, 0, HIDE_LBA_COUNTER);
                        }
                        switch (overwriteRet)
                        {
                        case SUCCESS:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Successfully overwrote LBAs!\n");
                            }
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Overwrite Time is not supported on this drive type at this time.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Failed to overwrite for the entered amount of time.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("You must specify a time to perform an overwrite for.\n");
                        }
                        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    }
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", PARTIAL_DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to run an overwrite operation.\n\n");
                    printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, PARTIAL_DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (disableATAPassword)
        {
            switch (run_Disable_ATA_Security_Password(&deviceList[deviceIter], ATAPassword, atauserMasterPW))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully disabled ATA security password!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Disabling ATA security password is not supported on this device or this device type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to disable ATA security password!\n");
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
            if (strcmp(progressTest, "SANITIZE") == 0 ||
                strcmp(progressTest, "BLOCKERASE") == 0 ||
                strcmp(progressTest, "CRYPTOERASE") == 0 ||
                strcmp(progressTest, "OVERWRITEERASE") == 0
                )
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Getting Sanitize progress.\n");
                }
                result = show_Sanitize_Progress(&deviceList[deviceIter]);
            }
            else if (strcmp(progressTest, "FORMAT") == 0)
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
    print_SAT_12_Byte_CDB_Help(shortUsage);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    //the test options
    printf("\nUtility Arguments\n");
    printf("=================\n");
    //Common (across utilities) - alphabetized
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Display_LBA_Help(shortUsage);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Poll_Help(shortUsage);
    print_Progress_Help(shortUsage, "sanitize, format");
    print_Scan_Help(shortUsage, deviceHandleExample);
	print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    //multiple interfaces
    print_Time_Hours_Help(shortUsage);
    print_Time_Minutes_Help(shortUsage);
    print_Time_Seconds_Help(shortUsage);
    print_Show_Supported_Erase_Modes_Help(shortUsage);
    print_Show_Physical_Element_Status_Help(shortUsage);
    //SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_Disable_ATA_Security_Password_Help(shortUsage, util_name);
    //SAS Only Options
    //printf("\n\tSAS Only:\n\n");

    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //multiple interfaces
    print_Overwrite_Help(shortUsage);
    print_Overwrite_Range_Help(shortUsage);
    print_Pattern_Help(shortUsage);
    print_Perform_Quickest_Erase_Help(shortUsage);
    print_Revert_Help(shortUsage);
    print_RevertSP_Help(shortUsage);
    print_Remove_Physical_Element_Status_Help(shortUsage);
    print_Sanitize_Help(shortUsage, util_name);
    print_Trim_Unmap_Help(shortUsage);
    print_Trim_Unmap_Range_Help(shortUsage);
    print_Writesame_Help(shortUsage);
    print_Writesame_Range_Help(shortUsage);
    //SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_ATA_Security_Erase_Help(shortUsage, util_name);
    //SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");
    print_Fast_Format_Help(shortUsage);
    print_Format_Unit_Help(shortUsage);
}
