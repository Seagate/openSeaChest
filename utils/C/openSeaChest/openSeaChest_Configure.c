//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2021 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
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
#include "EULA.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "drive_info.h"
#include "set_max_lba.h"
#include "trim_unmap.h"
#include "smart.h"
#include "logs.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_Configure";
const char *buildVersion = "2.0.5";

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
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    //tool specific
    SET_MAX_LBA_VARS
    RESTORE_MAX_LBA_VAR
    SET_PHY_SPEED_VARS
    SET_PHY_SAS_PHY_IDENTIFIER_VAR
    SET_READY_LED_VARS
    READ_LOOK_AHEAD_VARS
    NV_CACHE_VARS
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

    SCSI_MP_RESET_VARS
    SCSI_MP_RESTORE_VARS
    SCSI_MP_SAVE_VARS
    SCSI_SHOW_MP_VARS
    SCSI_RESET_LP_VARS
    SCSI_SET_MP_VARS

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
        NV_CACHE_LONG_OPT,
        WRITE_CACHE_LONG_OPT,
        LOW_CURRENT_SPINUP_LONG_OPT,
        SCT_WRITE_CACHE_LONG_OPT,
        SCT_WRITE_CACHE_REORDER_LONG_OPT,
        PUIS_FEATURE_LONG_OPT,
        SSC_FEATURE_LONG_OPT,
        SCT_ERROR_RECOVERY_CONTROL_LONG_OPTS,
        FREE_FALL_LONG_OPT,
        SCSI_MP_RESET_LONG_OPT,
        SCSI_MP_RESTORE_LONG_OPT,
        SCSI_MP_SAVE_LONG_OPT,
        SCSI_SHOW_MP_LONG_OPTS,
        SCSI_RESET_LP_LONG_OPTS,
        SCSI_SET_MP_LONG_OPT,
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
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, SET_MAX_LBA_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_MAX_LBA_LONG_OPT_STRING))) == 0)
            {
                if (get_And_Validate_Integer_Input((const char *)optarg, &SET_MAX_LBA_VALUE))
                {
                    SET_MAX_LBA_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SET_MAX_LBA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
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
            else if (strncmp(longopts[optionIndex].name, NV_CACHE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(NV_CACHE_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    NV_CACHE_INFO = true;
                }
                else
                {
                    NV_CACHE_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        NV_CACHE_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        NV_CACHE_SETTING = false;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(NV_CACHE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
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
                if (get_And_Validate_Integer_Input((const char *)optarg, &SET_MAX_LBA_VALUE))
                {
                    SET_MAX_LBA_FLAG = true;
                    //now, based on the new MaxLBA, set the TRIM/UNMAP start flag to get rid of the LBAs that will not be above the new maxLBA (the range will be set later)
                    TRIM_UNMAP_START_FLAG = SET_MAX_LBA_VALUE + 1;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PROVISION_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, LOW_CURRENT_SPINUP_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(LOW_CURRENT_SPINUP_LONG_OPT_STRING))) == 0)
            {
                LOW_CURRENT_SPINUP_FLAG = true;
                if (strcmp(optarg, "low") == 0)
                {
                    LOW_CURRENT_SPINUP_STATE = 1;
                }
                else if (strcmp(optarg, "ultra") == 0)
                {
                    LOW_CURRENT_SPINUP_STATE = 3;
                }
                else if (strcmp(optarg, "disable") == 0)
                {
                    LOW_CURRENT_SPINUP_STATE = 2;
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
                    if (get_And_Validate_Integer_Input(optarg, &value))
                    {
                        print_Error_In_Cmd_Line_Args(FREE_FALL_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    if (value > UINT8_MAX)
                    {
                        print_Error_In_Cmd_Line_Args(FREE_FALL_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    FREE_FALL_FLAG = true;
                    FREE_FALL_SENSITIVITY = (uint8_t)value;
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_MP_RESET_LONG_OPT_STRING) == 0)
            {
                SCSI_MP_RESET_OP = true;
                char * token = strtok(optarg, "-");
                uint8_t count = 0;
                bool errorInCL = false;
                while (token && !errorInCL && count < 2)
                {
                    uint64_t value = 0;
                    if (get_And_Validate_Integer_Input(token, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_MP_RESET_PAGE_NUMBER = (uint8_t)value;
                            if (value > MP_RETURN_ALL_PAGES)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_MP_RESET_SUBPAGE_NUMBER = (uint8_t)value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = strtok(NULL, "-");
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_MP_RESET_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_MP_RESTORE_LONG_OPT_STRING) == 0)
            {
                SCSI_MP_RESTORE_OP = true;
                char * token = strtok(optarg, "-");
                uint8_t count = 0;
                bool errorInCL = false;
                while (token && !errorInCL && count < 2)
                {
                    uint64_t value = 0;
                    if (get_And_Validate_Integer_Input(token, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_MP_RESTORE_PAGE_NUMBER = (uint8_t)value;
                            if (value > MP_RETURN_ALL_PAGES)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_MP_RESTORE_SUBPAGE_NUMBER = (uint8_t)value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = strtok(NULL, "-");
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_MP_RESTORE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_MP_SAVE_LONG_OPT_STRING) == 0)
            {
                SCSI_MP_SAVE_OP = true;
                char * token = strtok(optarg, "-");
                uint8_t count = 0;
                bool errorInCL = false;
                while (token && !errorInCL && count < 2)
                {
                    uint64_t value = 0;
                    if (get_And_Validate_Integer_Input(token, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_MP_SAVE_PAGE_NUMBER = (uint8_t)value;
                            if (value > MP_RETURN_ALL_PAGES)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_MP_SAVE_SUBPAGE_NUMBER = (uint8_t)value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = strtok(NULL, "-");
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_MP_SAVE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_SHOW_MP_LONG_OPT_STRING) == 0)
            {
                SCSI_SHOW_MP_OP = true;
                char * token = strtok(optarg, "-");
                uint8_t count = 0;
                bool errorInCL = false;
                while (token && !errorInCL && count < 2)
                {
                    uint64_t value = 0;
                    if (get_And_Validate_Integer_Input(token, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_SHOW_MP_PAGE_NUMBER = (uint8_t)value;
                            if (value > MP_RETURN_ALL_PAGES)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_SHOW_MP_SUBPAGE_NUMBER = (uint8_t)value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = strtok(NULL, "-");
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_SHOW_MP_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_SET_MP_LONG_OPT_STRING) == 0)
            {
                SCSI_SET_MP_OP = true;
                //first check if they are specifying a file!
                if (strncmp(optarg, "file", 4) == 0)
                {
                    //format is file=filename.txt
                    int sscanfRes = sscanf(optarg, "file=%s", SCSI_SET_MP_FILENAME);
                    if (sscanfRes < 1 || sscanfRes == EOF)
                    {
                        print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    else
                    {
                        if (!os_File_Exists(SCSI_SET_MP_FILENAME))
                        {
                            //TODO: print file open error instead???
                            print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_CANNOT_OPEN_FILE);
                        }
                        //else we open the file later to use
                    }
                }
                else
                {
                    //formatted as mp[-sp]:byte:highbit:fieldWidth=value
                    #define PARSE_MP_PAGE_AND_SUBPAGE_LENGTH 8
                    char pageAndSubpage[PARSE_MP_PAGE_AND_SUBPAGE_LENGTH] = { 0 };
                    char *token = strtok(optarg, ":=");
                    uint8_t tokenCounter = 0;
                    while (token && tokenCounter < 5)
                    {
                        //go through each string and convert it from a string into a value we can use in this tool
                        //start with page and subpage
                        switch (tokenCounter)
                        {
                        case 0://page-subpage
                        {
                            snprintf(pageAndSubpage, PARSE_MP_PAGE_AND_SUBPAGE_LENGTH, "%s", token);
                            //parse later outside this loop. If we use strtok again in here, we'll break the way the parsing works... :(
                        }
                        break;
                        case 1://byte
                            SCSI_SET_MP_BYTE = (uint16_t)atoi(token);
                            break;
                        case 2://bit
                            if (atoi(token) > 7)
                            {
                                print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                            SCSI_SET_MP_BIT = (uint8_t)atoi(token);
                            break;
                        case 3://field width
                            if (atoi(token) > 64)
                            {
                                print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                            SCSI_SET_MP_FIELD_LEN_BITS = (uint8_t)atoi(token);
                            break;
                        case 4://value
                            if (!get_And_Validate_Integer_Input(token, &SCSI_SET_MP_FIELD_VALUE))
                            {
                                print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                            }
                            break;
                        default:
                            //shouldn't get here!!! throw an error?!
                            break;
                        }
                        ++tokenCounter;
                        token = strtok(NULL, ":=");
                    }
                    char *pagetoken = strtok(pageAndSubpage, "-");
                    if (pagetoken)
                    {
                        SCSI_SET_MP_PAGE_NUMBER = (uint8_t)strtoul(pagetoken, NULL, 16);
                        pagetoken = strtok(NULL, "-");
                        if (pagetoken)
                        {
                            SCSI_SET_MP_SUBPAGE_NUMBER = (uint8_t)strtoul(pagetoken, NULL, 16);
                        }
                    }
                    else //should this be an error condition since strtok failed?
                    {
                        //no subpage
                        SCSI_SET_MP_PAGE_NUMBER = (uint8_t)strtoul(pageAndSubpage, NULL, 16);
                        SCSI_SET_MP_SUBPAGE_NUMBER = 0;
                    }
                    if (SCSI_SET_MP_PAGE_NUMBER > 0x3F)
                    {
                        print_Error_In_Cmd_Line_Args(SCSI_SET_MP_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_SHOW_MP_MPC_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "all") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = 4;//bigger than possible for an actual MPC value, so we can filter below off of this
                }
                else if (strcmp(optarg, "current") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = MPC_CURRENT_VALUES;
                }
                else if (strcmp(optarg, "default") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = MPC_DEFAULT_VALUES;
                }
                else if (strcmp(optarg, "saved") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = MPC_SAVED_VALUES;
                }
                else if (strcmp(optarg, "changeable") == 0)
                {
                    SCSI_SHOW_MP_MPC_VALUE = MPC_CHANGABLE_VALUES;
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "classic") == 0)
                {
                    SCSI_SHOW_MP_BUFFER_MODE = false;
                }
                else if (strcmp(optarg, "buffer") == 0)
                {
                    SCSI_SHOW_MP_BUFFER_MODE = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_RESET_LP_LONG_OPT_STRING) == 0)
            {
                SCSI_RESET_LP_OP = true;
                if (strcmp(optarg, "all") == 0)
                {
                    SCSI_RESET_LP_LPC = 4;//bigger than possible for an actual LPC value, so we can filter below off of this
                }
                else if (strcmp(optarg, "threshold") == 0)
                {
                    SCSI_RESET_LP_LPC = LPC_THRESHOLD_VALUES;
                }
                else if (strcmp(optarg, "cumulative") == 0)
                {
                    SCSI_RESET_LP_LPC = LPC_CUMULATIVE_VALUES;
                }
                else if (strcmp(optarg, "defThreshold") == 0)
                {
                    SCSI_RESET_LP_LPC = LPC_DEFAULT_THRESHOLD_VALUES;
                }
                else if (strcmp(optarg, "defCumulative") == 0)
                {
                    SCSI_RESET_LP_LPC = LPC_DEFAULT_CUMULATIVE_VALUES;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SCSI_RESET_LP_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SCSI_RESET_LP_PAGE_LONG_OPT_STRING) == 0)
            {
                char * token = strtok(optarg, "-");
                uint8_t count = 0;
                bool errorInCL = false;
                while (token && !errorInCL && count < 2)
                {
                    uint64_t value = 0;
                    if (get_And_Validate_Integer_Input(token, &value))
                    {
                        switch (count)
                        {
                        case 0:
                            SCSI_RESET_LP_PAGE_NUMBER = (uint8_t)value;
                            if (value > 0x3F)
                            {
                                errorInCL = true;
                            }
                            break;
                        case 1:
                            SCSI_RESET_LP_SUBPAGE_NUMBER = (uint8_t)value;
                            break;
                        default:
                            errorInCL = true;
                            break;
                        }
                        ++count;
                        token = strtok(NULL, "-");
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(SCSI_RESET_LP_PAGE_LONG_OPT_STRING, optarg);
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
        //check for other tool specific options here
        || RESTORE_MAX_LBA_FLAG
        || SET_MAX_LBA_FLAG
        || SET_PHY_SPEED_FLAG
        || SET_READY_LED_FLAG
        || READY_LED_INFO_FLAG
        || WRITE_CACHE_FLAG
        || READ_LOOK_AHEAD_FLAG
        || READ_LOOK_AHEAD_INFO
        || NV_CACHE_INFO
        || NV_CACHE_FLAG
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
        || SCSI_MP_RESET_OP
        || SCSI_MP_RESTORE_OP
        || SCSI_MP_SAVE_OP
        || SCSI_SHOW_MP_OP
        || SCSI_RESET_LP_OP
        || SCSI_SET_MP_OP
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
                if (ret == PERMISSION_DENIED || !is_Running_Elevated())
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

        if (SCSI_SHOW_MP_OP)
        {
            if (SCSI_SHOW_MP_MPC_VALUE > 3)
            {
                //showing all MPC values for the page
                show_SCSI_Mode_Page_All(&deviceList[deviceIter], SCSI_SHOW_MP_PAGE_NUMBER, SCSI_SHOW_MP_SUBPAGE_NUMBER, SCSI_SHOW_MP_BUFFER_MODE);
            }
            else
            {
                //show the specific MPC value
                show_SCSI_Mode_Page(&deviceList[deviceIter], SCSI_SHOW_MP_PAGE_NUMBER, SCSI_SHOW_MP_SUBPAGE_NUMBER, SCSI_SHOW_MP_MPC_VALUE, SCSI_SHOW_MP_BUFFER_MODE);
            }
        }


        if (SET_PHY_SPEED_FLAG)
        {
            switch (set_phy_speed(&deviceList[deviceIter], SET_PHY_SPEED_GEN, SET_PHY_ALL_PHYS, SET_PHY_SAS_PHY_IDENTIFIER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set the PHY speed. Please power cycle the device to complete this change.\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Operation not supported by this device.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unable to read ready LED info on this device or this device type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully changed Ready LED behavior!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Changing Ready LED behavior is not supported on this device or this device type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Write cache successfully enabled!\n");
                    }
                    else
                    {
                        printf("Write cache successfully disabled!\n");
                    }
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set SCT error recovery read command timer!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting SCT error recovery read command timer is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set SCT error recovery write command timer!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting SCT error recovery write command timer is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control read timer is set to %" PRIu32 "ms\n", timerValueMilliseconds);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control write timer is set to %" PRIu32 "ms\n", timerValueMilliseconds);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("SCT error recovery control is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
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

        if (NV_CACHE_INFO)
        {
            if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE)
            {
                if (is_NV_Cache_Supported(&deviceList[deviceIter]))
                {
                    if (is_NV_Cache_Enabled(&deviceList[deviceIter]))
                    {
                        printf("Non-Volatile Cache is Enabled (Caching Mode Page NV_DIS Bit is set to 0)\n");
                    }
                    else
                    {
                        printf("Non-Volatile Cache is Disabled (Caching Mode Page NV_DIS Bit is set to 1)\n");
                    }
                }
                else
                {
                    printf("Non-Volatile Cache is not supported on this drive.\n");
                }
            }
            else
            {
                printf("Non-Volatile Cache info is not available on this drive.\n");
            }
        }

        if (NV_CACHE_FLAG)
        {
            switch (scsi_Set_NV_DIS(&deviceList[deviceIter], NV_CACHE_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (NV_CACHE_SETTING)
                    {
                        printf("Non-Volatile Cache successfully enabled!\n");
                    }
                    else
                    {
                        printf("Non-Volatile Cache successfully disabled!\n");
                    }
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (NV_CACHE_SETTING)
                    {
                        printf("Enabling Non-Volatile Cache not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling Non-Volatile Cache not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (NV_CACHE_SETTING)
                    {
                        printf("Failed to enable Non-Volatile Cache!\n");
                    }
                    else
                    {
                        printf("Failed to disable Non-Volatile Cache!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (READ_LOOK_AHEAD_FLAG)
        {
            switch (set_Read_Look_Ahead(&deviceList[deviceIter], READ_LOOK_AHEAD_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Read look-ahead successfully enabled!\n");
                    }
                    else
                    {
                        printf("Read look-ahead successfully disabled!\n");
                    }
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (DATA_ERASE_FLAG)
                {
                    switch (trim_Unmap_Range(&deviceList[deviceIter], localStartLBA, localRange))
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Successfully trimmed/unmapped LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Trim/Unmap is not supported on this drive type.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Failed to trim/unmap LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
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
                        printf("You must add the flag:\n\"%s\" \n", DATA_ERASE_ACCEPT_STRING);
                        printf("to the command line arguments to run a trim/unmap operation.\n\n");
                        printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample, TRIM_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
                    }
                }
            }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An invalid start LBA has been entered. Please enter a valid value.\n");
                }
            }
        }

        if (SET_MAX_LBA_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Setting MaxLBA to %"PRIu64"\n", SET_MAX_LBA_VALUE);
            }
            switch (set_Max_LBA(&deviceList[deviceIter], SET_MAX_LBA_VALUE, false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully set the max LBA to %"PRIu64"\n", SET_MAX_LBA_VALUE);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Setting the max LBA is not supported by this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the max LBA!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (RESTORE_MAX_LBA_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Restoring max LBA\n");
            }
            switch (set_Max_LBA(&deviceList[deviceIter], 0, true))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully restored the max LBA\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Restoring the max LBA is not supported by this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to restore the max LBA!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (LOW_CURRENT_SPINUP_FLAG)
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Set Low Current Spinup\n");
            }
            bool sctMethodSupported = is_SCT_Low_Current_Spinup_Supported(&deviceList[deviceIter]);
            switch (set_Low_Current_Spin_Up(&deviceList[deviceIter], sctMethodSupported, LOW_CURRENT_SPINUP_STATE))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully ");
                    if (LOW_CURRENT_SPINUP_STATE == 1)
                    {
                        printf("Enabled");
                    }
                    else if (LOW_CURRENT_SPINUP_STATE == 3)
                    {
                        printf("Enabled Ultra");
                    }
                    else
                    {
                        printf("Disabled");
                    }
                    printf(" Low Current Spinup!\nA power cycle is required to complete this change.\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (LOW_CURRENT_SPINUP_STATE == 3)
                    {
                        printf("Setting Ultra Low Current Spinup not supported on this device\n");
                    }
                    else
                    {
                        printf("Setting Low Current Spinup not supported on this device\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to set the Low Current Spinup feature!\n");
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
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
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Free Fall Control feature successfully disabled!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Disabling Free Fall Control feature not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
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
                    if (VERBOSITY_QUIET < toolVerbosity)
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
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Setting Free Fall Control sensitivity not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
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
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Free Fall control feature is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get Free Fall control information.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (SCSI_MP_RESET_OP)
        {
            switch (scsi_Update_Mode_Page(&deviceList[deviceIter], SCSI_MP_RESET_PAGE_NUMBER, SCSI_MP_RESET_SUBPAGE_NUMBER, UPDATE_SCSI_MP_RESET_TO_DEFAULT))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully reset mode page!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Mode page not supported or resetting mode page to defaults not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying reset the specified mode page\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_MP_RESTORE_OP)
        {
            switch (scsi_Update_Mode_Page(&deviceList[deviceIter], SCSI_MP_RESTORE_PAGE_NUMBER, SCSI_MP_RESTORE_SUBPAGE_NUMBER, UPDATE_SCSI_MP_RESTORE_TO_SAVED))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully restored mode page to saved values!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Mode page not supported or restoring mode page to saved values not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying restore the specified mode page to its saved values\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_MP_SAVE_OP)
        {
            switch (scsi_Update_Mode_Page(&deviceList[deviceIter], SCSI_MP_SAVE_PAGE_NUMBER, SCSI_MP_SAVE_SUBPAGE_NUMBER, UPDATE_SCSI_MP_SAVE_CURRENT))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully saved mode page!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Mode page not supported or saving mode page not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying save the specified mode page\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_RESET_LP_OP)
        {
            int resetLPResult = SUCCESS;
            if (SCSI_RESET_LP_LPC > LPC_DEFAULT_CUMULATIVE_VALUES)
            {
                //requesting to reset all
                for (SCSI_RESET_LP_LPC = LPC_THRESHOLD_VALUES; SCSI_RESET_LP_LPC <= LPC_DEFAULT_CUMULATIVE_VALUES; ++SCSI_RESET_LP_LPC)
                {
                    int resetLPCommandRet = reset_SCSI_Log_Page(&deviceList[deviceIter], SCSI_RESET_LP_LPC, SCSI_RESET_LP_PAGE_NUMBER, SCSI_RESET_LP_SUBPAGE_NUMBER, !VOLATILE_FLAG);
                    if (SUCCESS != resetLPCommandRet)//this is to catch if any LPC reset value creates an error
                    {
                        resetLPResult = resetLPCommandRet;
                    }
                }
            }
            else
            {
                //reset just the specified information
                resetLPResult = reset_SCSI_Log_Page(&deviceList[deviceIter], SCSI_RESET_LP_LPC, SCSI_RESET_LP_PAGE_NUMBER, SCSI_RESET_LP_SUBPAGE_NUMBER, !VOLATILE_FLAG);
            }
            switch (resetLPResult)
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully reset the log page!\n");
                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                    {
                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Log page reset not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case BAD_PARAMETER:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Resetting a specific log page is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("An Error occurred while trying reset the log page\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SCSI_SET_MP_OP)
        {
            if (strlen(SCSI_SET_MP_FILENAME) > 0)//file was given to be used to set the MP
            {
                //need to open the file that was passed, and convert it to an array.
                //skip all newlines, spaces, underscores, dashes, slashes, etc. We should only be finding hex bytes inside this buffer (no H's or 0x's either)
                FILE *modePageFile = fopen(SCSI_SET_MP_FILENAME, "r");
                if (modePageFile)
                {
                    //first, figure out the length of the file...this will be useful to help us allocate a big enough buffer for the data
                    size_t fileLength = (size_t)get_File_Size(modePageFile) + 1;//add 1 so that we have a null terminator once we read in the file.
                    uint8_t *modePageBuffer = (uint8_t*)calloc_aligned(fileLength, sizeof(uint8_t), deviceList[deviceIter].os_info.minimumAlignment);//this will allocate more than enough memory for us to read the file...it's extra and that's ok.
                    char *fileBuf = (char*)calloc(fileLength, sizeof(char));
                    if (modePageBuffer && fileBuf)
                    {
                        //read the file
                        if ((fileLength - 1) == fread(fileBuf, sizeof(char), (fileLength - 1), modePageFile))//need the -1 since we added an extra 1 space above for a null terminator otherwise this fails. - TJE
                        {
                            //parse the file
                            char *delimiters = " \n\r-_\\/|\t:;";
                            char *token = strtok(fileBuf, delimiters);//add more to the delimiter list as needed
                            if (token)
                            {
                                bool invalidCharacterOrMissingSeparator = false;
                                uint16_t modeBufferElementCount = 0;
                                do
                                {
                                    if (strlen(token) > 2)
                                    {
                                        invalidCharacterOrMissingSeparator = true;
                                        break;
                                    }
                                    if (strpbrk(token, "ghijklmnopqrstuvwxyzGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()+={[}]\"'<>?,.`~"))
                                    {
                                        invalidCharacterOrMissingSeparator = true;
                                        break;
                                    }
                                    //not an invalid character or a missing separator, so convert the string to an array value.
                                    modePageBuffer[modeBufferElementCount] = (uint8_t)strtoul(token, NULL, 16);
                                    ++modeBufferElementCount;
                                    token = strtok(NULL, delimiters);
                                } while (token);
                                if (!invalidCharacterOrMissingSeparator)
                                {
                                    //file is read, send the change
                                    switch (scsi_Set_Mode_Page(&deviceList[deviceIter], modePageBuffer, modeBufferElementCount, !VOLATILE_FLAG))
                                    {
                                    case SUCCESS:
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("Successfully set SCSI mode page!\n");
                                            if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                            {
                                                printf("NOTE: This command may have affected more than 1 logical unit\n");
                                            }
                                        }
                                        break;
                                    case NOT_SUPPORTED:
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("Unable to change the requested values in the mode page. These may not be changable or are an invalid combination.\n");
                                        }
                                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                                        break;
                                    default:
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("Failed to set the mode page changes that were requested.\n");
                                        }
                                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                        break;
                                    }
                                }
                                else
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("An error occurred while trying to parse the file. Please check the file format and make sure no invalid characters are provided.\n");
                                    }
                                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                }
                            }
                            else
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("An error occurred while trying to parse the file. Please check the file format.\n");
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            }
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Error reading contents of mode page file!\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        }
                        safe_Free_aligned(modePageBuffer)
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Unable to allocate memory to read the file. Cannot set the mode page.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    fclose(modePageFile);
                    safe_Free_aligned(modePageBuffer)
                    safe_Free(fileBuf);
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Unable to read the file with the mode page data. Cannot set the mode page.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
            else
            {
                //inputs were parsed, now we need to turn the inputs into what the user wants to change.
                //first, figure out how large the mode page is, then read all it's bytes...then start going through and changing things as the user requests (the hard part)
                uint32_t rawModePageSize = 0;
                if (SUCCESS == get_SCSI_Mode_Page_Size(&deviceList[deviceIter], MPC_CURRENT_VALUES, SCSI_SET_MP_PAGE_NUMBER, SCSI_SET_MP_SUBPAGE_NUMBER, &rawModePageSize))
                {
                    uint8_t *rawmodePageBuffer = (uint8_t*)calloc(rawModePageSize, sizeof(uint8_t));
                    if (rawmodePageBuffer)
                    {
                        bool usedSizeByteCmd = false;
                        if (SUCCESS == get_SCSI_Mode_Page(&deviceList[deviceIter], MPC_CURRENT_VALUES, SCSI_SET_MP_PAGE_NUMBER, SCSI_SET_MP_SUBPAGE_NUMBER, NULL, NULL, true, rawmodePageBuffer, rawModePageSize, NULL, &usedSizeByteCmd))
                        {
                            uint32_t modeHeaderLen = usedSizeByteCmd ? MODE_PARAMETER_HEADER_6_LEN : MODE_PARAMETER_HEADER_10_LEN;
                            uint32_t blockDescriptorLength = (deviceList[deviceIter].drive_info.scsiVersion < SCSI_VERSION_SCSI2) ? SHORT_LBA_BLOCK_DESCRIPTOR_LEN : 0;//the get_SCSI_MP function will only pull a block descriptor on old drives for compatibility using mode sense 6 commands, which is why this check is minimal
                            uint32_t dataLengthAdjustment = modeHeaderLen + blockDescriptorLength;
                            uint32_t modePageSize = rawModePageSize - dataLengthAdjustment;
                            uint8_t* modePageBuffer = rawmodePageBuffer + dataLengthAdjustment;
                            //TODO: some of this code can probably be simplified a bit more than it currently is.
                            //now we have the data, we can begin modifying the field requested.
                            if (SCSI_SET_MP_FIELD_LEN_BITS % BITSPERBYTE)
                            {
                                //not a multi-bye aligned field. Ex: 12 bits or 3 bits, etc
                                //check if the number of bits is greater than a byte or not and if the starting bit anf field width would break across byte boundaries
                                if (SCSI_SET_MP_FIELD_LEN_BITS > BITSPERBYTE || (int)((int)SCSI_SET_MP_BIT - (int)SCSI_SET_MP_FIELD_LEN_BITS) < 0)
                                {
                                    //setting bits within multiple bytes, but not necessarily full bytes!
                                    uint8_t remainingBits = SCSI_SET_MP_FIELD_LEN_BITS;
                                    uint8_t highUnalignedBits = 0;//always least significant bits in this byte
                                    uint8_t lowUnalignedBits = 0;//always most significant bits in this byte
                                    if (SCSI_SET_MP_BIT != 7)
                                    {
                                        highUnalignedBits = SCSI_SET_MP_BIT + 1;
                                        remainingBits -= highUnalignedBits;
                                    }
                                    //check how many full bytes worth of bits we'll be setting.
                                    uint8_t fullBytesToSet = remainingBits / BITSPERBYTE;
                                    remainingBits -= fullBytesToSet * BITSPERBYTE;
                                    lowUnalignedBits = remainingBits;
                                    //now we know how we need to set things, so lets start at the end (lsb) and work up from there.
                                    //as we set the necessary bits, we will shift the original value to the right to make it easy to set each piece of the bits.
                                    remainingBits = SCSI_SET_MP_FIELD_LEN_BITS;//resetting this to help keep track as we shift through the bits.
                                    uint16_t offset = SCSI_SET_MP_BYTE + fullBytesToSet;
                                    if (lowUnalignedBits > 0)
                                    {
                                        ++offset;//add one to the offset since these bits are on another byte past the starting offset and any full bytes we need to set
                                        //need to create a mask and take the lowest bits that we need and place then in this byte starting at bit 7
                                        uint8_t mask = M_GETBITRANGE(UINT8_MAX, 7, 7 - (lowUnalignedBits - 1)) << (7 - lowUnalignedBits + 1);
                                        //clear the requested bits first
                                        modePageBuffer[offset] &= ~(mask);
                                        //now set them as requested
                                        modePageBuffer[offset] |= (mask & (SCSI_SET_MP_FIELD_VALUE << (7 - lowUnalignedBits + 1)));
                                        //bits are set, decrease the offset for the next operation
                                        --offset;
                                        SCSI_SET_MP_FIELD_VALUE >>= lowUnalignedBits;
                                    }
                                    if (fullBytesToSet > 0)
                                    {
                                        for (uint8_t byteCnt = 0; byteCnt < fullBytesToSet; ++byteCnt, SCSI_SET_MP_FIELD_VALUE >>= BITSPERBYTE, --offset)
                                        {
                                            modePageBuffer[offset] = M_Byte0(SCSI_SET_MP_FIELD_VALUE);
                                        }
                                    }
                                    if (highUnalignedBits > 0)
                                    {
                                        //need to create a mask and take the highest bits (only ones remaining at this point) that we need and place then in this byte starting at bit 0
                                        uint8_t mask = M_GETBITRANGE(UINT8_MAX, (highUnalignedBits - 1), (highUnalignedBits - 1) - (highUnalignedBits - 1)) << ((highUnalignedBits - 1) - highUnalignedBits + 1);
                                        //clear the requested bits first
                                        modePageBuffer[SCSI_SET_MP_BYTE] &= ~(mask);
                                        //now set them as requested
                                        modePageBuffer[SCSI_SET_MP_BYTE] |= (mask & (SCSI_SET_MP_FIELD_VALUE << ((highUnalignedBits - 1) - highUnalignedBits + 1)));
                                    }
                                }
                                else
                                {
                                    //setting bits within a single byte.
                                    uint8_t mask = M_GETBITRANGE(UINT8_MAX, SCSI_SET_MP_BIT, SCSI_SET_MP_BIT - (SCSI_SET_MP_FIELD_LEN_BITS - 1)) << (SCSI_SET_MP_BIT - SCSI_SET_MP_FIELD_LEN_BITS + 1);
                                    //clear the requested bits first
                                    modePageBuffer[SCSI_SET_MP_BYTE] &= ~(mask);
                                    //now set them as requested
                                    modePageBuffer[SCSI_SET_MP_BYTE] |= (mask & (SCSI_SET_MP_FIELD_VALUE << (SCSI_SET_MP_BIT - SCSI_SET_MP_FIELD_LEN_BITS + 1)));
                                }
                            }
                            else
                            {
                                //set full bytes to the value requested.
                                uint16_t fieldWidthBytes = SCSI_SET_MP_FIELD_LEN_BITS / BITSPERBYTE;
                                uint8_t byteNumber = 0;
                                while (fieldWidthBytes >= 1)
                                {
                                    modePageBuffer[SCSI_SET_MP_BYTE + (fieldWidthBytes - 1)] = (uint8_t)((M_ByteN(byteNumber) & SCSI_SET_MP_FIELD_VALUE) >> (BITSPERBYTE * byteNumber));
                                    --fieldWidthBytes;
                                    ++byteNumber;
                                }
                            }
                            //buffer is ready to send to the drive!
                            switch (scsi_Set_Mode_Page(&deviceList[deviceIter], modePageBuffer, C_CAST(uint16_t, modePageSize), !VOLATILE_FLAG))
                            {
                            case SUCCESS:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Successfully set SCSI mode page!\n");
                                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                    {
                                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                                    }
                                }
                                break;
                            case NOT_SUPPORTED:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Unable to change the requested values in the mode page. These may not be changeable or are an invalid combination.\n");
                                }
                                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                                break;
                            default:
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Failed to set the mode page changes that were requested.\n");
                                }
                                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                                break;
                            }
                        }
                        else
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Unable to read the requested mode page...it may not be supported.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        }
                        safe_Free(rawmodePageBuffer);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Unable to allocate memory to modify mode page.\n");
                        }
                        exitCode = UTIL_EXIT_NOT_ENOUGH_RESOURCES;
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Unable to determine length of the requested mode page...it may not be supported.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
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
    print_NV_Cache_Bit_Help(shortUsage);
    print_Set_Ready_LED_Help(shortUsage);
    print_SAS_Phy_Help(shortUsage);
    print_SCSI_Reset_LP_Help(shortUsage);
    print_SCSI_Reset_LP_Page_Help(shortUsage);
    print_SCSI_MP_Reset_Help(shortUsage);
    print_SCSI_MP_Restore_Help(shortUsage);
    print_SCSI_MP_Save_Help(shortUsage);
    print_Set_SCSI_MP_Help(shortUsage);
    print_Show_SCSI_MP_Output_Mode_Help(shortUsage);
    print_SCSI_Show_MP_Help(shortUsage);
    print_SCSI_Show_MP_Control_Help(shortUsage);

    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("=========================\n");
    //utility data destructive tests/operations go here
    print_Provision_Help(shortUsage);
}
