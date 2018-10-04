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
// \file OpenSeaChest_Configure.c command line that performs various Configuration methods on a device.

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
#include "set_max_lba.h"
#include "trim_unmap.h"
#include "set_sector_size.h"
#include "smart.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_Configure";
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
    PARTIAL_DATA_ERASE_VAR
    DATA_ERASE_VAR
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
    //tool specific
    SET_MAX_LBA_VARS
    RESTORE_MAX_LBA_VAR
    SET_PHY_SPEED_VARS
    SET_PHY_SAS_PHY_IDENTIFIER_VAR
    SET_READY_LED_VARS
    READ_LOOK_AHEAD_VARS
    WRITE_CACHE_VARS
    PROVISION_VAR
    TRIM_UNMAP_VARS //need these with provision var
    LOW_CURRENT_SPINUP_VARS
    SCT_WRITE_CACHE_VARS
    SCT_WRITE_CACHE_REORDER_VARS
    VOLATILE_VAR
    PUIS_FEATURE_VARS
    SSC_FEATURE_VARS
#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    SCT_ERROR_RECOVERY_CONTROL_VARS
    FREE_FALL_VARS

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
        CONFIRM_LONG_OPT,
        VOLATILE_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        //tool specific options go here
        SET_MAX_LBA_LONG_OPT,
        PROVISION_LONG_OPT,
        RESTORE_MAX_LBA_LONG_OPT,
        SET_PHY_SPEED_LONG_OPT,
        SET_PHY_SAS_PHY_LONG_OPT,
		SET_READY_LED_LONG_OPTS,
        READ_LOOK_AHEAD_LONG_OPT,
        WRITE_CACHE_LONG_OPT,
        LOW_CURRENT_SPINUP_LONG_OPT,
        SCT_WRITE_CACHE_LONG_OPT,
        SCT_WRITE_CACHE_REORDER_LONG_OPT,
        PUIS_FEATURE_LONG_OPT,
        SSC_FEATURE_LONG_OPT,
        SCT_ERROR_RECOVERY_CONTROL_LONG_OPTS,
        FREE_FALL_LONG_OPT,
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
                if (strlen(optarg) == strlen(PARTIAL_DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, PARTIAL_DATA_ERASE_ACCEPT_STRING, strlen(PARTIAL_DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    PARTIAL_DATA_ERASE_FLAG = true;
                }
                else if (strlen(optarg) == strlen(DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, DATA_ERASE_ACCEPT_STRING, strlen(DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    DATA_ERASE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SET_MAX_LBA_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_MAX_LBA_LONG_OPT_STRING))) == 0)
            {
                SET_MAX_LBA_FLAG = true;
                sscanf(optarg, "%"SCNu64"", &SET_MAX_LBA_VALUE);
            }
            else if (strncmp(longopts[optionIndex].name, SET_PHY_SPEED_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_PHY_SPEED_LONG_OPT_STRING))) == 0)
            {
                SET_PHY_SPEED_FLAG = true;
                SET_PHY_SPEED_GEN = (uint8_t)atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, SET_PHY_SAS_PHY_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_PHY_SAS_PHY_LONG_OPT_STRING))) == 0)
            {
                SET_PHY_ALL_PHYS = false;
                SET_PHY_SAS_PHY_IDENTIFIER = (uint8_t)atoi(optarg);
            }
			else if ((strncmp(longopts[optionIndex].name, SET_READY_LED_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_READY_LED_LONG_OPT_STRING))) == 0))
            {
                if (strcmp(optarg, "default") == 0)
                {
                    SET_READY_LED_FLAG = true;
                    SET_READY_LED_DEFAULT = true;
                }
                else if (strcmp(optarg, "on") == 0)
                {
                    SET_READY_LED_FLAG = true;
                    SET_READY_LED_MODE = true;
                }
                else if (strcmp(optarg, "off") == 0)
                {
                    SET_READY_LED_FLAG = true;
                    SET_READY_LED_MODE = false;
                }
                else if (strcmp(optarg, "info") == 0)
                {
                    READY_LED_INFO_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_READY_LED_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, READ_LOOK_AHEAD_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(READ_LOOK_AHEAD_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    READ_LOOK_AHEAD_INFO = true;
                }
                else
                {
                    READ_LOOK_AHEAD_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        READ_LOOK_AHEAD_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        READ_LOOK_AHEAD_SETTING = false;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(READ_LOOK_AHEAD_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, WRITE_CACHE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(WRITE_CACHE_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    WRITE_CACHE_INFO = true;
                }
                else
                {
                    WRITE_CACHE_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        WRITE_CACHE_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        WRITE_CACHE_SETTING = false;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(WRITE_CACHE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, PROVISION_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(PROVISION_LONG_OPT_STRING))) == 0)
            {
                SET_MAX_LBA_FLAG = true;
                sscanf(optarg, "%"SCNu64"", &SET_MAX_LBA_VALUE);
                //now, based on the new MaxLBA, set the TRIM/UNMAP start flag to get rid of the LBAs that will not be above the new maxLBA (the range will be set later)
                TRIM_UNMAP_START_FLAG = SET_MAX_LBA_VALUE + 1;
            }
            else if (strncmp(longopts[optionIndex].name, LOW_CURRENT_SPINUP_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(LOW_CURRENT_SPINUP_LONG_OPT_STRING))) == 0)
            {
                LOW_CURRENT_SPINUP_FLAG = true;
                if (strcmp(optarg, "enable") == 0)
                {
                    LOW_CURRENT_SPINUP_ENABLE_DISABLE = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    LOW_CURRENT_SPINUP_ENABLE_DISABLE = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(LOW_CURRENT_SPINUP_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCT_WRITE_CACHE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SCT_WRITE_CACHE_INFO = true;
                }
                else
                {
                    SCT_WRITE_CACHE_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        SCT_WRITE_CACHE_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        SCT_WRITE_CACHE_SETTING = false;
                    }
                    else if (strcmp(optarg, "default") == 0)
                    {
                        SCT_WRITE_CACHE_SET_DEFAULT = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCT_WRITE_CACHE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCT_WRITE_CACHE_REORDER_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SCT_WRITE_CACHE_REORDER_INFO = true;
                }
                else
                {
                    SCT_WRITE_CACHE_REORDER_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        SCT_WRITE_CACHE_REORDER_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        SCT_WRITE_CACHE_REORDER_SETTING = false;
                    }
                    else if (strcmp(optarg, "default") == 0)
                    {
                        SCT_WRITE_CACHE_REORDER_SET_DEFAULT = true;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SCT_WRITE_CACHE_REORDER_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, PUIS_FEATURE_LONG_OPT_STRING) == 0)
            {
                PUIS_FEATURE_FLAG = true;
                if (strcmp(optarg, "enable") == 0)
                {
                    PUIS_FEATURE_STATE_FLAG = true;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    PUIS_FEATURE_STATE_FLAG = false;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PUIS_FEATURE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SSC_FEATURE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    GET_SSC_FLAG = true;
                }
                else
                {
                    SET_SSC_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        SSC_MODE = SSC_ENABLED;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        SSC_MODE = SSC_DISABLED;
                    }
                    else if (strcmp(optarg, "default") == 0)
                    {
                        SSC_MODE = SSC_DEFAULT;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(SSC_FEATURE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SCT_ERROR_RECOVERY_CONTROL_READ_INFO = true;
                }
                else
                {
                    uint32_t multiplier = 100;//100 millisecond conversion
                    //first check is a unit is provided.
                    if (strstr(optarg, "ms"))
                    {
                        multiplier = UINT32_C(1);
                    }
                    else if (strstr(optarg, "s"))
                    {
                        multiplier = UINT32_C(1000);
                    }
                    else if (strstr(optarg, "m"))
                    {
                        multiplier = UINT32_C(60000);
                    }
                    else if (strstr(optarg, "h"))
                    {
                        multiplier = UINT32_C(3600000);
                    }
                    SCT_ERROR_RECOVERY_CONTROL_SET_READ_TIMER = true;
                    SCT_ERROR_RECOVERY_CONTROL_READ_TIMER_VALUE = (uint32_t)atoi(optarg) * multiplier;
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO = true;
                }
                else
                {
                    uint32_t multiplier = 100;//100 millisecond conversion
                    //first check is a unit is provided.
                    if (strstr(optarg, "ms"))
                    {
                        multiplier = UINT32_C(1);
                    }
                    else if (strstr(optarg, "s"))
                    {
                        multiplier = UINT32_C(1000);
                    }
                    else if (strstr(optarg, "m"))
                    {
                        multiplier = UINT32_C(60000);
                    }
                    else if (strstr(optarg, "h"))
                    {
                        multiplier = UINT32_C(3600000);
                    }
                    SCT_ERROR_RECOVERY_CONTROL_SET_WRITE_TIMER = true;
                    SCT_ERROR_RECOVERY_CONTROL_WRITE_TIMER_VALUE = (uint32_t)atoi(optarg) * multiplier;
                }
            }
            else if (strcmp(longopts[optionIndex].name, FREE_FALL_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    FREE_FALL_INFO = true;
                }
                else if (strcmp(optarg, "enable") == 0)
                {
                    FREE_FALL_FLAG = true;
                    FREE_FALL_SENSITIVITY = 0;//enable to vendor recommended if this arg is given
                }
                else
                {
                    uint64_t value = 0;
                    //this is a value to read in.
                    if (SUCCESS != get_And_Validate_Integer_Input(optarg, &value))
                    {
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    if (value > UINT8_MAX)
                    {
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    FREE_FALL_FLAG = true;
                    FREE_FALL_SENSITIVITY = (uint8_t)value;
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
        || RESTORE_MAX_LBA_FLAG
        || SET_MAX_LBA_FLAG
        || SET_PHY_SPEED_FLAG
        || SET_READY_LED_FLAG
        || READY_LED_INFO_FLAG
        || WRITE_CACHE_FLAG
        || READ_LOOK_AHEAD_FLAG
        || READ_LOOK_AHEAD_INFO
        || WRITE_CACHE_INFO
        || PROVISION_FLAG
        || LOW_CURRENT_SPINUP_FLAG
        || SCT_WRITE_CACHE_INFO
        || SCT_WRITE_CACHE_FLAG
        || SCT_WRITE_CACHE_REORDER_FLAG
        || SCT_WRITE_CACHE_REORDER_INFO
        || PUIS_FEATURE_FLAG
        || SET_SSC_FLAG
        || GET_SSC_FLAG
        || SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO
        || SCT_ERROR_RECOVERY_CONTROL_READ_INFO
        || SCT_ERROR_RECOVERY_CONTROL_SET_READ_TIMER
        || SCT_ERROR_RECOVERY_CONTROL_SET_WRITE_TIMER
        || FREE_FALL_FLAG
        || FREE_FALL_INFO
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

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (SET_PHY_SPEED_FLAG)
        {
            switch (set_phy_speed(&deviceList[deviceIter], SET_PHY_SPEED_GEN, SET_PHY_ALL_PHYS, SET_PHY_SAS_PHY_IDENTIFIER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully set the PHY speed. Please power cycle the device to complete this change.\n");
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Operation not supported by this device.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to set the PHY speed of the device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_SSC_FLAG)
        {
            switch (set_SSC_Feature_SATA(&deviceList[deviceIter], SSC_MODE))
            {
            case SUCCESS:
                printf("Successfully set SSC feature to ");
                switch (SSC_MODE)
                {
                case SSC_DEFAULT:
                    printf("the drive's default value\n");
                    break;
                case SSC_ENABLED:
                    printf("enabled\n");
                    break;
                case SSC_DISABLED:
                    printf("disabled\n");
                    break;
                }
                printf("Please power cycle the drive to make the change take effect.\n");
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                printf("SSC feature is not supported on this drive.\n");
                break;
            case FAILURE:
            default:
                printf("Failed to set the drive's SSC state.\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (GET_SSC_FLAG)
        {
            switch (get_SSC_Feature_SATA(&deviceList[deviceIter], (eSSCFeatureState*)&SSC_MODE))
            {
            case SUCCESS:
                printf("SSC Feature is ");
                switch (SSC_MODE)
                {
                case SSC_DEFAULT:
                    printf("set to the drive's default value\n");
                    break;
                case SSC_ENABLED:
                    printf("enabled\n");
                    break;
                case SSC_DISABLED:
                    printf("disabled\n");
                    break;
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                printf("SSC feature is not supported on this drive.\n");
                break;
            case FAILURE:
            default:
                printf("Failed to get the drive's SSC state.\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (READY_LED_INFO_FLAG)
        {
            bool readyLEDValue = false;
            switch (get_Ready_LED_State(&deviceList[deviceIter], &readyLEDValue))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (readyLEDValue)
                    {
                        printf("Ready LED is set to \"On\"\n");
                    }
                    else
                    {
                        printf("Ready LED is set to \"Off\"\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Unable to read ready LED info on this device or this device type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to read ready LED info!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_READY_LED_FLAG)
        {
            switch (change_Ready_LED(&deviceList[deviceIter], SET_READY_LED_DEFAULT, SET_READY_LED_MODE))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully changed Ready LED behavior!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Changin Ready LED behavior is not supported on this device or this device type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to change Ready LED behavior!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (WRITE_CACHE_INFO)
        {
            if (is_Write_Cache_Enabled(&deviceList[deviceIter]))
            {
                printf("Write Cache is Enabled\n");
            }
            else
            {
                printf("Write Cache is Disabled\n");
            }
        }

        if (WRITE_CACHE_FLAG)
        {
            switch (set_Write_Cache(&deviceList[deviceIter], WRITE_CACHE_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Write cache successfully enabled!\n");
                    }
                    else
                    {
                        printf("Write cache successfully disabled!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Enabling Write cache not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling Write cache not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Failed to enable Write cache!\n");
                    }
                    else
                    {
                        printf("Failed to disable Write cache!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_WRITE_CACHE_INFO)
        {
            uint16_t sctFlags = 0;
            uint16_t state = 0;
            //TODO: switch to sct_Get_Feature_Control since it is the correct call to use for this info
            if (SUCCESS == sct_Get_Feature_Control(&deviceList[deviceIter], SCT_FEATURE_CONTROL_WRITE_CACHE_STATE, &SCT_WRITE_CACHE_SETTING, &SCT_WRITE_CACHE_SET_DEFAULT, &state, &sctFlags))
            {
                if (state == 0 || state > 0x0003)
                {
                    printf("Unable to retrieve SCT Write Cache Status.\n");
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
                else
                {
                    if (SCT_WRITE_CACHE_SET_DEFAULT)
                    {
                        printf("Write Cache is controlled by set features command (default)");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_SETTING)
                        {
                            printf("Write Cache is Enabled");
                        }
                        else
                        {
                            printf("Write Cache is Disabled");
                        }
                    }
                    if (sctFlags == 0x0001)
                    {
                        printf(" (non-volatile)");
                    }
                    else if (sctFlags == 0x0000)
                    {
                        printf(" (volatile)");
                    }
                    printf("\n");
                }
            }
            else
            {
                printf("Unable to retrieve SCT Write Cache Status.\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SCT_WRITE_CACHE_FLAG)
        {
            switch (sct_Set_Feature_Control(&deviceList[deviceIter], SCT_FEATURE_CONTROL_WRITE_CACHE_STATE, SCT_WRITE_CACHE_SETTING, SCT_WRITE_CACHE_SET_DEFAULT, VOLATILE_FLAG, 0))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (SCT_WRITE_CACHE_SET_DEFAULT)
                    {
                        printf("SCT Write cache successfully restored to defaults!\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_SETTING)
                        {
                            printf("SCT Write cache successfully enabled!\n");
                        }
                        else
                        {
                            printf("SCT Write cache successfully disabled!\n");
                        }
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (SCT_WRITE_CACHE_SET_DEFAULT)
                    {
                        printf("Setting SCT Write cache to defaults is not supported on this device\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_SETTING)
                        {
                            printf("SCT Enabling Write cache not supported on this device.\n");
                        }
                        else
                        {
                            printf("SCT Disabling Write cache not supported on this device.\n");
                        }
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (SCT_WRITE_CACHE_SET_DEFAULT)
                    {
                        printf("Failed to restore SCT Write cache to defaults!\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_SETTING)
                        {
                            printf("SCT Failed to enable Write cache!\n");
                        }
                        else
                        {
                            printf("SCT Failed to disable Write cache!\n");
                        }
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_WRITE_CACHE_REORDER_INFO)
        {
            uint16_t sctFlags = 0;
            uint16_t state = 0;
            //TODO: switch to sct_Get_Feature_Control since it is the correct call to use for this info
            if (SUCCESS == sct_Get_Feature_Control(&deviceList[deviceIter], SCT_FEATURE_CONTROL_WRITE_CACHE_REORDERING, &SCT_WRITE_CACHE_REORDER_SETTING, &SCT_WRITE_CACHE_REORDER_SET_DEFAULT, &state, &sctFlags))
            {
                if (state == 0 || state > 0x0002)
                {
                    printf("Unable to retrieve SCT Write Cache Reordering Status.\n");
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                }
                else
                {
                    
                    if (SCT_WRITE_CACHE_REORDER_SETTING)
                    {
                        printf("Write Cache Reordering is Enabled (default)");
                    }
                    else
                    {
                        printf("Write Cache Reordering is Disabled");
                    }
                    if (sctFlags == 0x0001)
                    {
                        printf(" (non-volatile)");
                    }
                    else if (sctFlags == 0x0000)
                    {
                        printf(" (volatile)");
                    }
                    printf("\n");
                }
            }
            else
            {
                printf("Unable to retrieve SCT Write Cache Reordering Status.\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SCT_WRITE_CACHE_REORDER_FLAG)
        {
            switch (sct_Set_Feature_Control(&deviceList[deviceIter], SCT_FEATURE_CONTROL_WRITE_CACHE_REORDERING, SCT_WRITE_CACHE_REORDER_SETTING, SCT_WRITE_CACHE_REORDER_SET_DEFAULT, VOLATILE_FLAG, 0))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (SCT_WRITE_CACHE_REORDER_SET_DEFAULT)
                    {
                        printf("SCT Write cache Reordering successfully restored to defaults!\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_REORDER_SETTING)
                        {
                            printf("SCT Write cache Reordering successfully enabled!\n");
                        }
                        else
                        {
                            printf("SCT Write cache Reordering successfully disabled!\n");
                        }
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (SCT_WRITE_CACHE_REORDER_SET_DEFAULT)
                    {
                        printf("Setting SCT Write cache Reordering to defaults is not supported on this device\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_REORDER_SETTING)
                        {
                            printf("SCT Enabling Write cache Reordering not supported on this device.\n");
                        }
                        else
                        {
                            printf("SCT Disabling Write cache Reordering not supported on this device.\n");
                        }
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (SCT_WRITE_CACHE_REORDER_SET_DEFAULT)
                    {
                        printf("Failed to restore SCT Write cache Reordering to defaults!\n");
                    }
                    else
                    {
                        if (SCT_WRITE_CACHE_REORDER_SETTING)
                        {
                            printf("SCT Failed to enable Write cache Reordering!\n");
                        }
                        else
                        {
                            printf("SCT Failed to disable Write cache Reordering!\n");
                        }
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_SET_READ_TIMER)
        {
            switch (sct_Set_Command_Timer(&deviceList[deviceIter], SCT_ERC_READ_COMMAND, SCT_ERROR_RECOVERY_CONTROL_READ_TIMER_VALUE))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully set SCT error recovery read command timer!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Setting SCT error recovery read command timer is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to set SCT error recovery read command timer!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_SET_WRITE_TIMER)
        {
            switch (sct_Set_Command_Timer(&deviceList[deviceIter], SCT_ERC_WRITE_COMMAND, SCT_ERROR_RECOVERY_CONTROL_WRITE_TIMER_VALUE))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully set SCT error recovery write command timer!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Setting SCT error recovery write command timer is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to set SCT error recovery write command timer!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_READ_INFO)
        {
            uint32_t timerValueMilliseconds = 0;
            switch (sct_Get_Command_Timer(&deviceList[deviceIter], SCT_ERC_READ_COMMAND, &timerValueMilliseconds))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("SCT error recovery control read timer is set to %" PRIu32 "ms\n", timerValueMilliseconds);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("SCT error recovery control is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to get SCT error recovery read command timer!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO)
        {
            uint32_t timerValueMilliseconds = 0;
            switch (sct_Get_Command_Timer(&deviceList[deviceIter], SCT_ERC_WRITE_COMMAND, &timerValueMilliseconds))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("SCT error recovery control write timer is set to %" PRIu32 "ms\n", timerValueMilliseconds);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("SCT error recovery control is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to get SCT error recovery write command timer!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (READ_LOOK_AHEAD_INFO)
        {
            if (is_Read_Look_Ahead_Enabled(&deviceList[deviceIter]))
            {
                printf("Read Look Ahead is Enabled.\n");
            }
            else
            {
                printf("Read Look Ahead is Disabled.\n");
            }
        }

        if (READ_LOOK_AHEAD_FLAG)
        {
            switch (set_Read_Look_Ahead(&deviceList[deviceIter], READ_LOOK_AHEAD_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Read look-ahead successfully enabled!\n");
                    }
                    else
                    {
                        printf("Read look-ahead successfully disabled!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Enabling read look-ahead not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling read look-ahead not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Failed to enable read look-ahead!\n");
                    }
                    else
                    {
                        printf("Failed to disable read look-ahead!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (PROVISION_FLAG)
        {
            //this means we are performing a TRIM/UNMAP operation, then setting the maxlba. Since we've already set the trim start location, time to set the range
            TRIM_UNMAP_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba - TRIM_UNMAP_START_FLAG;
            RUN_TRIM_UNMAP_FLAG = true;
        }

        if (RUN_TRIM_UNMAP_FLAG)
        {
            uint64_t localStartLBA = TRIM_UNMAP_START_FLAG;
            uint64_t localRange = TRIM_UNMAP_RANGE_FLAG;
            if (localStartLBA != UINT64_MAX)
            {
                if (localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                }
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG == UINT64_MAX)
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

        if (SET_MAX_LBA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Setting MaxLBA to %"PRIu64"\n", SET_MAX_LBA_VALUE);
            }
            switch (set_Max_LBA(&deviceList[deviceIter], SET_MAX_LBA_VALUE, false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully set the max LBA to %"PRIu64"\n", SET_MAX_LBA_VALUE);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Setting the max LBA is not supported by this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to set the max LBA!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (RESTORE_MAX_LBA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Restoring max LBA\n");
            }
            switch (set_Max_LBA(&deviceList[deviceIter], 0, true))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully restored the max LBA\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Restoring the max LBA is not supported by this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to restore the max LBA!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (LOW_CURRENT_SPINUP_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Set Low Current Spinup\n");
            }
            if (LOW_CURRENT_SPINUP_ENABLE_DISABLE)
            {
                ret = enable_Low_Current_Spin_Up(&deviceList[deviceIter]);
            }
            else
            {
                ret = disable_Low_Current_Spin_Up(&deviceList[deviceIter]);
            }
            switch (ret)
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully ");
                    if (LOW_CURRENT_SPINUP_ENABLE_DISABLE)
                    {
                        printf("Enabled");
                    }
                    else
                    {
                        printf("Disabled");
                    }
                    printf(" Low Current Spinup!\nA power cycle is required to complete this change.\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Setting Low Current Spinup not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to ");
                    if (LOW_CURRENT_SPINUP_ENABLE_DISABLE)
                    {
                        printf("Enable\n");
                    }
                    else
                    {
                        printf("Disable\n");
                    }
                    printf(" Low Current Spinup!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        
        if(PUIS_FEATURE_FLAG)
        {
            switch (enable_Disable_PUIS_Feature(&deviceList[deviceIter], PUIS_FEATURE_STATE_FLAG))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (PUIS_FEATURE_STATE_FLAG)
                    {
                        printf("PUIS feature successfully enabled!\n");
                    }
                    else
                    {
                        printf("PUIS feature successfully disabled!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (PUIS_FEATURE_STATE_FLAG)
                    {
                        printf("Enabling PUIS feature not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling PUIS feature not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (PUIS_FEATURE_STATE_FLAG)
                    {
                        printf("Failed to enable PUIS feature!\n");
                    }
                    else
                    {
                        printf("Failed to disable PUIS feature!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (FREE_FALL_FLAG)
        {
            if (FREE_FALL_DISABLE)
            {
                switch (disable_Free_Fall_Control_Feature(&deviceList[deviceIter]))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Free Fall Control feature successfully disabled!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Disabling Free Fall Control feature not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Failed to disable Free Fall Control feature!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                switch (set_Free_Fall_Control_Sensitivity(&deviceList[deviceIter], FREE_FALL_SENSITIVITY))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        if (FREE_FALL_SENSITIVITY == 0)
                        {
                            printf("Free Fall Control feature successfully set to vendor's recommended value!\n");
                        }
                        else
                        {
                            printf("Free Fall Control feature successfully set to %" PRIu8 "!\n", FREE_FALL_SENSITIVITY);
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Setting Free Fall Control sensitivity not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Failed to set Free Fall Control sensitivity!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
        }

        if (FREE_FALL_INFO)
        {
            uint16_t sensitivity = 0;
            switch (get_Current_Free_Fall_Control_Sensitivity(&deviceList[deviceIter], &sensitivity))
            {
            case SUCCESS:
                if (sensitivity == UINT16_MAX)
                {
                    printf("Free Fall control feature is supported, but not enabled.\n");
                }
                else if (sensitivity == 0)
                {
                    printf("Free Fall control sensitivity is set to the vendor's recommended value.\n");
                }
                else
                {
                    printf("Free Fall control sensitivity is set to %" PRIu16 " of 255.\n", sensitivity);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Free Fall control feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to get Free Fall control information.\n");
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
    print_Phy_Speed_Help(shortUsage);
    print_Read_Look_Ahead_Help(shortUsage);
    print_Restore_Max_LBA_Help(shortUsage);
    print_Set_Max_LBA_Help(shortUsage);
    print_Write_Cache_Help(shortUsage);
    
    //SATA Only Options
    printf("\n\tSATA Only:\n\t========\n");
    print_Free_Fall_Help(shortUsage);
    print_Low_Current_Spinup_Help(shortUsage);
    print_PUIS_Feature_Help(shortUsage);
    print_Set_SSC_Help(shortUsage);
    print_SCT_Error_Recovery_Read_Help(shortUsage);
    print_SCT_Write_Cache_Help(shortUsage);
    print_SCT_Write_Cache_Reordering_Help(shortUsage);
    print_SCT_Error_Recovery_Write_Help(shortUsage);

    //SAS Only Options
    printf("\n\tSAS Only:\n\t========\n");
	print_Set_Ready_LED_Help(shortUsage);
    print_SAS_Phy_Help(shortUsage);

    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //utility data destructive tests/operations go here
    print_Provision_Help(shortUsage);
}
