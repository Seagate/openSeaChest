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
// \file openSeaChest_PowerControl.c Binary command line that performs various power control methods on a device.

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
#include "power_control.h"
#include "drive_info.h"

////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_PowerControl";
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
    //utility specific functions
    CHECK_POWER_VAR
    SPIN_DOWN_VAR
    ENABLE_POWER_MODE_VAR
    DISABLE_POWER_MODE_VAR
    DEFAULT_POWER_MODE_VAR
	EPC_ENABLED_VAR
    POWER_MODE_VAR
    POWER_MODE_TIMER_VARS
    CHANGE_POWER_MODE_VAR
    TRANSITION_POWER_MODE_VAR
    SET_POWER_CONSUMPTION_VARS
    SHOW_POWER_CONSUMPTION_VAR
    SET_APM_LEVEL_VARS
    SHOW_APM_LEVEL_VARS
    SHOW_EPC_SETTINGS_VAR
    DISABLE_APM_VAR
    SEAGATE_POWER_BALANCE_VARS
    SATA_DIPM_VARS
    SATA_DAPS_VARS
    STANDBY_VAR
    SLEEP_VAR
    IDLE_VAR
    IDLE_UNLOAD_VAR
    ACTIVE_VAR

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
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        //tool specific options go here
        CHECK_POWER_LONG_OPT,
        SPIN_DOWN_LONG_OPT,
        ENABLE_POWER_MODE_LONG_OPT,
        DISABLE_POWER_MODE_LONG_OPT,
        DEFAULT_POWER_MODE_LONG_OPT,
        POWER_MODE_LONG_OPT,
        EPC_ENABLED_LONG_OPT,
        POWER_MODE_TIMER_LONG_OPT,
        CHANGE_POWER_MODE_LONG_OPT,
        TRANSITION_POWER_MODE_LONG_OPT,
        SHOW_POWER_CONSUMPTION_LONG_OPT,
        SET_POWER_CONSUMPTION_LONG_OPT,
        SET_APM_LEVEL_LONG_OPT,
        SHOW_APM_LEVEL_LONG_OPT,
        SHOW_EPC_SETTINGS_LONG_OPT,
        DISABLE_APM_LONG_OPT,
        SEAGATE_POWER_BALANCE_LONG_OPT,
        SATA_DIPM_LONG_OPT,
        SATA_DAPS_LONG_OPT,
        STANDBY_LONG_OPT,
        SLEEP_LONG_OPT,
        IDLE_LONG_OPT,
        IDLE_UNLOAD_LONG_OPT,
        ACTIVE_LONG_OPT,
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
        args = getopt_long(argc, argv, "d:hisSF:Vv:q", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        //printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:

            if (strncmp(longopts[optionIndex].name, EPC_ENABLED_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(EPC_ENABLED_LONG_OPT_STRING))) == 0)
            {
                if (strncmp("enable", optarg, strlen(optarg)) == 0)
                {
                    EPC_ENABLED_IDENTIFIER = ENABLE_EPC;
                }
                else if (strncmp("disable", optarg, strlen(optarg)) == 0)
                {
                    EPC_ENABLED_IDENTIFIER = DISABLE_EPC;
                }
				else
				{
					print_Error_In_Cmd_Line_Args(EPC_ENABLED_LONG_OPT_STRING, optarg);
					exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
				}
            }
            //parse long options that have no short option and required arguments here
            else if (strncmp(longopts[optionIndex].name, POWER_MODE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(POWER_MODE_LONG_OPT_STRING))) == 0)
            {
                //set the power mode
                if (isdigit(optarg[0]))//this will get the valid NVMe power levels
                {
                    POWER_MODE_IDENTIFIER = atoi(optarg);
                }
                else
                {
                    if (strncmp("active", optarg, strlen(optarg)) == 0)
                    {
                        POWER_MODE_IDENTIFIER = PWR_CND_ACTIVE;
                    }
                    else if (strncmp("idle_a", optarg, strlen(optarg)) == 0)
                    {
                        POWER_MODE_IDENTIFIER = PWR_CND_IDLE_A;
                    }
                    else if (strncmp("idle_b", optarg, strlen(optarg)) == 0)
                    {
                        POWER_MODE_IDENTIFIER = PWR_CND_IDLE_B;
                    }
                    else if (strncmp("idle_c", optarg, strlen(optarg)) == 0)
                    {
                        POWER_MODE_IDENTIFIER = PWR_CND_IDLE_C;
                    }
                    else if (strncmp("standby_z", optarg, strlen(optarg)) == 0)
                    {
                        POWER_MODE_IDENTIFIER = PWR_CND_STANDBY_Z;
                    }
                    else if (strncmp("standby_y", optarg, strlen(optarg)) == 0)
                    {
                        POWER_MODE_IDENTIFIER = PWR_CND_STANDBY_Y;
                    }
                    else if (strncmp("all", optarg, strlen(optarg)) == 0)
                    {
                        POWER_MODE_IDENTIFIER = PWR_CND_ALL;
                    }
					else
					{
						print_Error_In_Cmd_Line_Args(POWER_MODE_LONG_OPT_STRING, optarg);
						exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
					}
                }
            }
            else if (strncmp(longopts[optionIndex].name, POWER_MODE_TIMER_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(POWER_MODE_TIMER_LONG_OPT_STRING))) == 0)
            {
                //set the timer value
                POWER_MODE_TIMER = atoi(optarg);
                POWER_MODE_TIMER_VALID = true;
            }
            else if (strncmp(longopts[optionIndex].name, SET_POWER_CONSUMPTION_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_POWER_CONSUMPTION_LONG_OPT_STRING))) == 0)
            {
                //set the flag for making the power consimption change
                SET_POWER_CONSUMPTION_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    SET_POWER_CONSUMPTION_DEFAULT_FLAG = true;
                }
                else if (strcmp(optarg, "highest") == 0)
                {
                    SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE = 1;
                }
                else if (strcmp(optarg, "intermediate") == 0)
                {
                    SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE = 2;
                }
                else if (strcmp(optarg, "lowest") == 0)
                {
                    SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE = 3;
                }
                else
                {
                    //set the value to change to (watts)
                    //use the lf specifier otherwise it thinks it's a standard float and you won't get the right value.
                    sscanf(optarg, "%lf", &SET_POWER_CONSUMPTION_WATTS_VALUE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SET_APM_LEVEL_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_APM_LEVEL_LONG_OPT_STRING))) == 0)
            {
                SET_APM_LEVEL_FLAG = true;
                SET_APM_LEVEL_VALUE_FLAG = (uint8_t)atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, SEAGATE_POWER_BALANCE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SEAGATE_POWER_BALANCE_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SEAGATE_POWER_BALANCE_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    SEAGATE_POWER_BALANCE_FLAG = true;
                    SEAGATE_POWER_BALANCE_ENABLE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SEAGATE_POWER_BALANCE_FLAG = true;
                    SEAGATE_POWER_BALANCE_ENABLE_FLAG = false;
                }
				else
				{
					print_Error_In_Cmd_Line_Args(SEAGATE_POWER_BALANCE_LONG_OPT_STRING, optarg);
					exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
				}
            }
            else if (strncmp(longopts[optionIndex].name, SATA_DIPM_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SATA_DIPM_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SATA_DIPM_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    SATA_DIPM_FLAG = true;
                    SATA_DIPM_ENABLE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SATA_DIPM_FLAG = true;
                    SATA_DIPM_ENABLE_FLAG = false;
                }
				else
				{
					print_Error_In_Cmd_Line_Args(SATA_DIPM_LONG_OPT_STRING, optarg);
					exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
				}
            }
            else if (strncmp(longopts[optionIndex].name, SATA_DAPS_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SATA_DAPS_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SATA_DAPS_INFO_FLAG = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    SATA_DAPS_FLAG = true;
                    SATA_DAPS_ENABLE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    SATA_DAPS_FLAG = true;
                    SATA_DAPS_ENABLE_FLAG = false;
                }
				else
				{
					print_Error_In_Cmd_Line_Args(SATA_DAPS_LONG_OPT_STRING, optarg);
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
        //check for other tool specific options here
        || CHECK_POWER_FLAG
        || SPIN_DOWN_FLAG
        || CHANGE_POWER_MODE_FLAG
        || TRANSITION_POWER_MODE_FLAG
        || SHOW_POWER_CONSUMPTION_FLAG
        || SET_POWER_CONSUMPTION_FLAG
        || (EPC_ENABLED_IDENTIFIER != ENABLE_EPC_NOT_SET)
        || SET_APM_LEVEL_FLAG
        || SHOW_APM_LEVEL_FLAG
        || SHOW_EPC_SETTINGS_FLAG
        || DISABLE_APM_FLAG
        || SEAGATE_POWER_BALANCE_FLAG
        || SEAGATE_POWER_BALANCE_INFO_FLAG
        || SATA_DIPM_INFO_FLAG
        || SATA_DIPM_FLAG
        || SATA_DAPS_INFO_FLAG
        || SATA_DAPS_FLAG
        || STANDBY_FLAG
        || SLEEP_FLAG
        || IDLE_FLAG
        || IDLE_UNLOAD_FLAG
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

    if (TEST_UNIT_READY_FLAG || CHECK_POWER_FLAG || TRANSITION_POWER_MODE_FLAG || SPIN_DOWN_FLAG || STANDBY_FLAG || IDLE_FLAG || IDLE_UNLOAD_FLAG || SLEEP_FLAG)
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

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (EPC_ENABLED_IDENTIFIER != ENABLE_EPC_NOT_SET)
        {
            switch (enable_Disable_EPC_Feature(&deviceList[deviceIter], EPC_ENABLED_IDENTIFIER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (EPC_ENABLED_IDENTIFIER == ENABLE_EPC)
                    {
                        printf("Successfully Enabled EPC Feature Set on %s.\n", deviceList[deviceIter].os_info.name);
                    }
                    else
                    {
                        printf("Successfully Disabled EPC Feature Set on %s.\n", deviceList[deviceIter].os_info.name);
                    }
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("EPC Feature is not supported by this device %s.\n", deviceList[deviceIter].os_info.name);
                }
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to send EPC command to %s.\n", deviceList[deviceIter].os_info.name);
                    printf("EPC Feature set might not be supported.\n");
                    printf("Or EPC Feature might already be in the desired state.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (SPIN_DOWN_FLAG)
        {
            switch (spin_down_drive(&deviceList[deviceIter], false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully sent command to spin down device. Please wait 15 seconds for it to finish spinning down.\n");
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Spin down not supported by this device.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to spin down device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (IDLE_FLAG)
        {
            switch (transition_To_Idle(&deviceList[deviceIter], false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully tansitioned to idle state.\n");
                    printf("\nHint:Use --checkPowerMode option to check the new Power Mode State.\n\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Transitioning to idle not allowed on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Could not transition to idle state\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (IDLE_UNLOAD_FLAG)
        {
            switch (transition_To_Idle(&deviceList[deviceIter], true))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully tansitioned to idle (unload) state.\n");
                    printf("\nHint:Use --checkPowerMode option to check the new Power Mode State.\n\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Transitioning to idle (unload) not allowed on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Could not transition to idle (unload) state\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (STANDBY_FLAG)
        {
            switch (transition_To_Standby(&deviceList[deviceIter]))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully tansitioned to standby state.\nPlease give device a few seconds to transition.\n");
                    printf("\nHint:Use --checkPowerMode option to check the new Power Mode State.\n\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Transitioning to standby not allowed on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Could not transition to standby state\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SLEEP_FLAG)
        {
            switch (transition_To_Sleep(&deviceList[deviceIter]))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully tansitioned to sleep state.\nPlease give device a few seconds to transition.\n");
                    printf("\nNOTE: drive will now require a reset to wake.\tThe system may not be able to rediscover it without a reboot.\n\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Transitioning to sleep not allowed on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Could not transition to sleep state\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (ACTIVE_FLAG)
        {
            switch (transition_To_Active(&deviceList[deviceIter]))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully tansitioned to active state.\nPlease give device a few seconds to transition.\n");
                    printf("\nHint:Use --checkPowerMode option to check the new Power Mode State.\n\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Transitioning to active not allowed on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Could not transition to active state\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (TRANSITION_POWER_MODE_FLAG)
        {
            if (POWER_MODE_IDENTIFIER != PWR_CND_NOT_SET && POWER_MODE_IDENTIFIER != PWR_CND_ALL)
            {
                switch (transition_Power_State(&deviceList[deviceIter], POWER_MODE_IDENTIFIER))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("\nPower State Transition Successful.\nPlease give device a few seconds to transition.\n");
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
                        printf("ERROR: Could not transition to the new power state\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("You must specify a power mode identifier\n");
                }
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
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

        if (CHANGE_POWER_MODE_FLAG)
        {
            //first check that we got enable, disable, or defaults (but not a combination of any of these options)
            if (ENABLE_POWER_MODE_FLAG || DISABLE_POWER_MODE_FLAG || DEFAULT_POWER_MODE_FLAG)
            {
                //make sure they didn't pass a combination of these flags since we can't make sense of that
                if (ENABLE_POWER_MODE_FLAG != DISABLE_POWER_MODE_FLAG || DEFAULT_POWER_MODE_FLAG)
                {
                    if (POWER_MODE_IDENTIFIER != PWR_CND_NOT_SET)
                    {
                        if (ENABLE_POWER_MODE_FLAG)
                        {
                            ret = set_Device_Power_Mode(&deviceList[deviceIter], DEFAULT_POWER_MODE_FLAG, true, POWER_MODE_IDENTIFIER, POWER_MODE_TIMER, POWER_MODE_TIMER_VALID);
                        }
                        else
                        {
                            ret = set_Device_Power_Mode(&deviceList[deviceIter], DEFAULT_POWER_MODE_FLAG, false, POWER_MODE_IDENTIFIER, POWER_MODE_TIMER, POWER_MODE_TIMER_VALID);
                        }
                        switch (ret)
                        {
                        case SUCCESS:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Successfully changed power mode settings.\n");
                            }
                            break;
                        case NOT_SUPPORTED:
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Changing power settings is not supported on this drive.\n");
                            }
                            break;
                        default:
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("ERROR: Could not change power mode settings.\n");
                            }
                            break;
                        }
                    }
                    else
                    {
                        //invalid power mode
                        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("ERROR: Invalid power mode entered.\n");
                        }
                    }
                }
                else
                {
                    //only specify enable or disable but not both
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: You can only specify enableMode or disableMode, not both.\n");
                    }
                }
            }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                //you must specify enable disable or defaults
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: You must specify enableMode, disableMode, or defaultMode.\n");
                }
            }
        }

        if (SHOW_EPC_SETTINGS_FLAG)
        {
            epcSettings deviceEPCSettings;
            memset(&deviceEPCSettings, 0, sizeof(epcSettings));
            switch(get_EPC_Settings(&deviceList[deviceIter], &deviceEPCSettings))
            {
            case SUCCESS:
                print_EPC_Settings(&deviceList[deviceIter], &deviceEPCSettings);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Showing EPC Settings not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to get the EPC settings.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHOW_POWER_CONSUMPTION_FLAG)
        {
            powerConsumptionIdentifiers identifiers;
			memset(&identifiers, 0, sizeof(powerConsumptionIdentifiers));
            switch (get_Power_Consumption_Identifiers(&deviceList[deviceIter], &identifiers))
            {
            case SUCCESS:
                print_Power_Consumption_Identifiers(&identifiers);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Power Consumption information not available on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to read power consumption information.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_POWER_CONSUMPTION_FLAG)
        {
            int pcRet = SUCCESS;
            if (SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE == PC_ACTIVE_LEVEL_IDENTIFIER)
            {
                pcRet = map_Watt_Value_To_Power_Consumption_Identifier(&deviceList[deviceIter], SET_POWER_CONSUMPTION_WATTS_VALUE, &SET_POWER_CONSUMPTION_VALUE);
            }
            if (pcRet == SUCCESS)
            {
                switch (set_Power_Consumption(&deviceList[deviceIter], SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE, SET_POWER_CONSUMPTION_VALUE, SET_POWER_CONSUMPTION_DEFAULT_FLAG))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Successfully set power consumption value for device!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Setting power consumption is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("An Error occurred while trying to read power consumption information.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An invalid or unsupported value was entered for power consumption level.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }
        
        if(SET_APM_LEVEL_FLAG)
        {
            switch(set_APM_Level(&deviceList[deviceIter], SET_APM_LEVEL_VALUE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set APM Level!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting APM Level is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to set the APM Level.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (DISABLE_APM_FLAG)
        {
            switch (enable_Disable_APM_Feature(&deviceList[deviceIter], false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully disabled APM feature!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Disabling APM is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to disable APM.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        
        if(SHOW_APM_LEVEL_FLAG)
        {
            switch(get_APM_Level(&deviceList[deviceIter], &SHOW_APM_LEVEL_VALUE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Current APM Level is %"PRIu8" (", SHOW_APM_LEVEL_VALUE_FLAG);
                    if(SHOW_APM_LEVEL_VALUE_FLAG == 0x01)
                    {
                        printf("Minimum power consumption with Standby mode");
                    }
                    else if(SHOW_APM_LEVEL_VALUE_FLAG >= 0x02 && SHOW_APM_LEVEL_VALUE_FLAG <= 0x7F)
                    {
                        printf("Intermediate power management with Standby mode");
                    }
                    else if(SHOW_APM_LEVEL_VALUE_FLAG == 0x80)
                    {
                        printf("Minimum power consumption without Standby mode");
                    }
                    else if(SHOW_APM_LEVEL_VALUE_FLAG >= 0x81 && SHOW_APM_LEVEL_VALUE_FLAG <= 0xFD)
                    {
                        printf("Intermediate power management without Standby mode");
                    }
                    else if(SHOW_APM_LEVEL_VALUE_FLAG == 0xFE)
                    {
                        printf("Maximum Performance");
                    }
                    else
                    {
                        printf("Reserved");
                    }
                    printf(")\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Showing APM Level is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying to get the APM Level.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SEAGATE_POWER_BALANCE_FLAG)
        {
            switch (seagate_Set_Power_Balance(&deviceList[deviceIter], SEAGATE_POWER_BALANCE_ENABLE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SEAGATE_POWER_BALANCE_ENABLE_FLAG)
                    {
                        printf("Successfully enabled Seagate Power Balance!\n");
                    }
                    else
                    {
                        printf("Successfully disabled Seagate Power Balance!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting Seagate Power Balance feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the Seagate Power Balance feature!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SEAGATE_POWER_BALANCE_INFO_FLAG)
        {
            bool powerBalanceSupported = false, powerBalanceEnabled = false;
            switch (seagate_Get_Power_Balance(&deviceList[deviceIter], &powerBalanceSupported, &powerBalanceEnabled))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Seagate Power Balance: ");
                    if (powerBalanceSupported)
                    {
                        if (powerBalanceEnabled)
                        {
                            printf("Enabled\n");
                        }
                        else
                        {
                            printf("Disabled\n");
                        }
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting Seagate Power Balance info is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get the Seagate Power Balance info!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        
        if (SATA_DIPM_FLAG)
        {
            switch (sata_Set_Device_Initiated_Interface_Power_State_Transitions(&deviceList[deviceIter], SATA_DIPM_ENABLE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SATA_DIPM_ENABLE_FLAG)
                    {
                        printf("Successfully enabled Device Initiated Power Management (DIPM)!\n");
                    }
                    else
                    {
                        printf("Successfully disabled Device Initiated Power Management (DIPM)!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting DIPM feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the DIPM feature!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (SATA_DIPM_INFO_FLAG)
        {
            bool dipmSupported = false, dipmEnabled = false;
            switch (sata_Get_Device_Initiated_Interface_Power_State_Transitions(&deviceList[deviceIter], &dipmSupported, &dipmEnabled))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Device Initiated Power Management (DIPM): ");
                    if (dipmSupported)
                    {
                        if (dipmEnabled)
                        {
                            printf("Enabled\n");
                        }
                        else
                        {
                            printf("Disabled\n");
                        }
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting DIPM info is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get the DIPM info!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }        
        if (SATA_DAPS_FLAG)
        {
            switch (sata_Set_Device_Automatic_Partioan_To_Slumber_Transtisions(&deviceList[deviceIter], SATA_DAPS_ENABLE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (SATA_DAPS_ENABLE_FLAG)
                    {
                        printf("Successfully enabled Device Automatic Partial To Slumber Transitions (DAPS)!\n");
                    }
                    else
                    {
                        printf("Successfully disabled Device Automatic Partial To Slumber Transitions (DAPS)!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting DAPS feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the DAPS feature!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (SATA_DAPS_INFO_FLAG)
        {
            bool dapsSupported = false, dapsEnabled = false;
            switch (sata_Get_Device_Automatic_Partioan_To_Slumber_Transtisions(&deviceList[deviceIter], &dapsSupported, &dapsEnabled))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Device Automatic Partial To Slumber Transitions (DAPS): ");
                    if (dapsSupported)
                    {
                        if (dapsEnabled)
                        {
                            printf("Enabled\n");
                        }
                        else
                        {
                            printf("Disabled\n");
                        }
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting DAPS info is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get the DAPS info!\n");
                }
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
    print_Active_Help(shortUsage);
    print_Change_Power_Help(shortUsage);
    print_Check_Power_Mode_Help(shortUsage);
    print_Default_Power_Mode_Help(shortUsage);
    print_Disable_Power_Mode_Help(shortUsage);
    print_EnableDisableEPC_Help(shortUsage);
    print_Enable_Power_Mode_Help(shortUsage);
    print_Idle_Help(shortUsage);
    print_Idle_Unload_Help(shortUsage);
    print_Power_Mode_Help(shortUsage);
    print_Show_EPC_Settings_Help(shortUsage);
    print_Sleep_Help(shortUsage);
    print_Spindown_Help(shortUsage);
    print_Standby_Help(shortUsage);
    print_Timer_Mode_Help(shortUsage);
    print_Transition_Power_Help(shortUsage);
    //SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_Seagate_Power_Balance_Help(shortUsage);
    print_Disable_APM_Help(shortUsage);
    print_DAPS_Help(shortUsage);
    print_DIPM_Help(shortUsage);
    print_Set_APM_Level_Help(shortUsage);
    print_Show_APM_Level_Help(shortUsage);
    //SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");
    print_Set_Power_Consumption_Help(shortUsage);
    print_Show_Power_Consumption_Help(shortUsage);
}
