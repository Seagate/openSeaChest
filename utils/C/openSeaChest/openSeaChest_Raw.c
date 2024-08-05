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
// \file openSeaChest_Raw.c Binary command line that sends raw TFRs or CDBs

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "type_conversion.h"
#include "memory_safety.h"
#include "string_utils.h"
#include "io_utils.h"
#include "unit_conversion.h"
#include "getopt.h"
#include "EULA.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "drive_info.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name = "openSeaChest_Raw";
const char* buildVersion = "0.9.0";

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
int main(int argc, char *argv[])
{
    /////////////////
    //  Variables  //
    /////////////////
    //common utility variables
    eReturnValues ret = SUCCESS;
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
    //tool specific flags
    RAW_TFR_SIZE_VAR
    RAW_TFR_VARS
    RAW_TFR_PROTOCOL_VAR
    RAW_TFR_XFER_LENGTH_LOCATION_VAR
    RAW_TFR_CHECK_CONDITION_VAR
    RAW_DATA_LEN_VARS
    RAW_DATA_DIRECTION_VAR
    RAW_TIMEOUT_VAR
    RAW_OUTPUT_FILE_VARS
    RAW_INPUT_FILE_VARS
    RAW_INPUT_FILE_OFFSET_VAR
    RAW_TFR_BYTE_BLOCK_VAR
    RAW_CDB_LEN_VAR
    RAW_CDB_ARRAY_VAR
#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif

    int args = 0;
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
        NO_BANNER_OPT,
        AGRESSIVE_SCAN_LONG_OPT,
        SCAN_FLAGS_LONG_OPT,
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
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        //tool specific options go here
        RAW_CDB_LEN_LONG_OPT,
        RAW_CDB_ARRAY_LONG_OPT,
        RAW_TFR_SIZE_LONG_OPT,
        RAW_TFR_REGITER_LONG_OPTS,
        RAW_TFR_PROTOCOL_LONG_OPT,
        RAW_TFR_XFER_LENGTH_LOCATION_LONG_OPT,
        RAW_TFR_CHECK_CONDITION_LONG_OPT,
        RAW_TFR_BYTE_BLOCK_LONG_OPT,
        RAW_DATA_LEN_LONG_OPT,
        RAW_DATA_DIRECTION_LONG_OPT,
        RAW_TIMEOUT_LONG_OPT,
        RAW_OUTPUT_FILE_LONG_OPT,
        RAW_INPUT_FILE_LONG_OPT,
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
            if (strcmp(longopts[optionIndex].name, RAW_CDB_ARRAY_LONG_OPT_STRING) == 0)
            {
                char* saveptr = M_NULLPTR;
                rsize_t duplen = 0;
                char* dupoptarg = strdup(optarg);
                char* token = M_NULLPTR;
                uint8_t count = 0;
                bool errorInCL = false;
                if (dupoptarg)
                {
                    duplen = safe_strlen(dupoptarg);
                    token = common_String_Token(dupoptarg, &duplen, ",", &saveptr);
                }
                else
                {
                    errorInCL = true;
                }
                while (token && !errorInCL && count < UINT8_MAX)
                {
                    uint8_t value = 0;
                    if (get_And_Validate_Integer_Input_Uint8(token, M_NULLPTR, ALLOW_UNIT_NONE, &value))
                    {
                        RAW_CDB_ARRAY[count] = value;
                        ++count;
                        token = common_String_Token(M_NULLPTR, &duplen, ",", &saveptr);
                    }
                    else
                    {
                        errorInCL = true;
                    }
                }
                safe_Free(C_CAST(void**, &dupoptarg));
                if (errorInCL)
                {
                    print_Error_In_Cmd_Line_Args(RAW_CDB_ARRAY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_CDB_LEN_LONG_OPT_STRING) == 0)
            {
                //set the cdblength
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_CDB_LEN_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(RAW_CDB_LEN_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_SIZE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "complete") == 0)
                {
                    RAW_TFR_SIZE_FLAG = 0xFF;
                }
                else
                {
                    if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_SIZE_FLAG))
                    {
                        print_Error_In_Cmd_Line_Args(RAW_TFR_SIZE_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_COMMAND_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_COMMAND))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_COMMAND_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_FEATURE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_FEATURE))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_FEATURE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_FEATURE_EXT_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_FEATURE_EXT))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_FEATURE_EXT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_FEATURE_FULL_LONG_OPT_STRING) == 0)
            {
                uint16_t fullfeat = 0;
                if (!get_And_Validate_Integer_Input_Uint16(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &fullfeat))
                {
                    RAW_TFR_FEATURE = M_Byte0(fullfeat);
                    RAW_TFR_FEATURE_EXT = M_Byte1(fullfeat);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_FEATURE_FULL_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_LBA_LOW_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_LBA_LOW))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_LBA_LOW_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_LBA_MID_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_LBA_MID))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_LBA_MID_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_LBA_HIGH_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_LBA_HIGH))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_LBA_HIGH_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_LBA_LOW_EXT_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_LBA_LOW_EXT))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_LBA_LOW_EXT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_LBA_MID_EXT_LONG_OPT_STRING) == 0)
            {
            if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_LBA_MID_EXT))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_LBA_MID_EXT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_LBA_HIGH_EXT_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_LBA_HIGH_EXT))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_LBA_HIGH_EXT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_LBA_FULL_LONG_OPT_STRING) == 0)
            {
                uint64_t fullLBA = 0;
                if (get_And_Validate_Integer_Input_Uint64(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &fullLBA) && fullLBA <= MAX_48_BIT_LBA)
                {
                    RAW_TFR_LBA_LOW = M_Byte0(fullLBA);
                    RAW_TFR_LBA_MID = M_Byte1(fullLBA);
                    RAW_TFR_LBA_HIGH = M_Byte2(fullLBA);
                    RAW_TFR_LBA_LOW_EXT = M_Byte3(fullLBA);
                    RAW_TFR_LBA_MID_EXT = M_Byte4(fullLBA);
                    RAW_TFR_LBA_HIGH_EXT = M_Byte5(fullLBA);
                    //TODO: On a 28bit command, we may need to set the lower nibble of device/head for the high bits of the LBA value
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_LBA_FULL_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_DEVICE_HEAD_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_DEVICE_HEAD))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_DEVICE_HEAD_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_LBA_MODE_BIT_LONG_OPT_STRING) == 0)
            {
                RAW_TFR_DEVICE_HEAD |= LBA_MODE_BIT;
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_SECTOR_COUNT_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_SECTOR_COUNT))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_SECTOR_COUNT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_SECTOR_COUNT_EXT_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_SECTOR_COUNT_EXT))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_SECTOR_COUNT_EXT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_SECTOR_COUNT_FULL_LONG_OPT_STRING) == 0)
            {
                uint16_t fullseccnt = 0;
                if (get_And_Validate_Integer_Input_Uint16(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &fullseccnt))
                {
                    RAW_TFR_SECTOR_COUNT = M_Byte0(fullseccnt);
                    RAW_TFR_SECTOR_COUNT_EXT = M_Byte1(fullseccnt);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_SECTOR_COUNT_FULL_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_ICC_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_ICC))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_ICC_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_AUX1_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_AUX1))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_AUX1_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_AUX2_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_AUX2))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_AUX2_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_AUX3_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_AUX3))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_AUX3_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_AUX4_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TFR_AUX4))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_AUX4_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_AUX_FULL_LONG_OPT_STRING) == 0)
            {
                uint32_t fullaux = 0;
                if (get_And_Validate_Integer_Input_Uint32(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &fullaux))
                {
                    RAW_TFR_AUX1 = M_Byte0(fullaux);
                    RAW_TFR_AUX2 = M_Byte1(fullaux);
                    RAW_TFR_AUX3 = M_Byte2(fullaux);
                    RAW_TFR_AUX4 = M_Byte3(fullaux);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_AUX4_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_PROTOCOL_LONG_OPT_STRING) == 0)
            {
                //set the protocol
                if (strcmp(optarg, "pio") == 0 || strcmp(optarg, "PIO") == 0)
                {
                    RAW_TFR_PROTOCOL = C_CAST(int, ATA_PROTOCOL_PIO);
                }
                else if (strcmp(optarg, "dma") == 0 || strcmp(optarg, "DMA") == 0)
                {
                    RAW_TFR_PROTOCOL = C_CAST(int, ATA_PROTOCOL_DMA);
                }
                else if (strcmp(optarg, "udma") == 0 || strcmp(optarg, "UDMA") == 0)
                {
                    RAW_TFR_PROTOCOL = C_CAST(int, ATA_PROTOCOL_UDMA);
                }
                else if (strcmp(optarg, "fpdma") == 0 || strcmp(optarg, "FPDMA") == 0 || strcmp(optarg, "ncq") == 0 || strcmp(optarg, "NCQ") == 0)
                {
                    RAW_TFR_PROTOCOL = C_CAST(int, ATA_PROTOCOL_DMA_FPDMA);
                }
                else if (strcmp(optarg, "nodata") == 0 || strcmp(optarg, "NODATA") == 0)
                {
                    RAW_TFR_PROTOCOL = C_CAST(int, ATA_PROTOCOL_NO_DATA);
                }
                else if (strcmp(optarg, "reset") == 0 || strcmp(optarg, "RESET") == 0)
                {
                    RAW_TFR_PROTOCOL = C_CAST(int, ATA_PROTOCOL_DEV_RESET);
                }
                else if (strcmp(optarg, "dmaque") == 0 || strcmp(optarg, "DMAQUE") == 0)
                {
                    RAW_TFR_PROTOCOL = C_CAST(int, ATA_PROTOCOL_DMA_QUE);
                }
                else if (strcmp(optarg, "diag") == 0 || strcmp(optarg, "DIAG") == 0)
                {
                    RAW_TFR_PROTOCOL = C_CAST(int, ATA_PROTOCOL_DEV_DIAG);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_PROTOCOL_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_XFER_LENGTH_LOCATION_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "sectorCount") == 0)
                {
                    RAW_TFR_XFER_LENGTH_LOCATION = ATA_PT_LEN_SECTOR_COUNT;
                }
                else if (strcmp(optarg, "feature") == 0)
                {
                    RAW_TFR_XFER_LENGTH_LOCATION = ATA_PT_LEN_FEATURES_REGISTER;
                }
                else if (strcmp(optarg, "tpsiu") == 0)
                {
                    RAW_TFR_XFER_LENGTH_LOCATION = ATA_PT_LEN_TPSIU;
                }
                else if (strcmp(optarg, "nodata") == 0)
                {
                    RAW_TFR_XFER_LENGTH_LOCATION = ATA_PT_LEN_NO_DATA;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_XFER_LENGTH_LOCATION_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TFR_BYTE_BLOCK_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "512") == 0)
                {
                    RAW_TFR_BYTE_BLOCK = 512;
                }
                else if (strcmp(optarg, "logical") == 0)
                {
                    RAW_TFR_BYTE_BLOCK = UINT8_MAX;
                }
                else if (strcmp(optarg, "bytes") == 0)
                {
                    RAW_TFR_BYTE_BLOCK = 1;
                }
                else if (strcmp(optarg, "nodata") == 0)
                {
                    RAW_TFR_BYTE_BLOCK = 0;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_TFR_BYTE_BLOCK_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_DATA_LEN_LONG_OPT_STRING) == 0)
            {
                char* unit = M_NULLPTR;
                if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_DATASIZE, &RAW_DATA_LEN_FLAG))
                {
                    //Check to see if any units were specified, otherwise assume LBAs
                    uint32_t multiplier = 1;
                    if (unit)
                    {
                        if (strcmp(unit, "") == 0)
                        {
                            RAW_DATA_LEN_ADJUST_BY_BLOCKS_FLAG = true;
                        }
                        else if (strcmp(unit, "BLOCKS") == 0 || strcmp(unit, "SECTORS") == 0)
                        {
                            //they specified blocks. For log transfers this means a number of 512B sectors
                            multiplier = LEGACY_DRIVE_SEC_SIZE;
                        }
                        else if (strcmp(unit, "KB") == 0)
                        {
                            multiplier = UINT32_C(1000);
                        }
                        else if (strcmp(unit, "KiB") == 0)
                        {
                            multiplier = UINT32_C(1024);
                        }
                        else if (strcmp(unit, "MB") == 0)
                        {
                            multiplier = UINT32_C(1000000);
                        }
                        else if (strcmp(unit, "MiB") == 0)
                        {
                            multiplier = UINT32_C(1048576);
                        }
                        else if (strcmp(unit, "GB") == 0)
                        {
                            multiplier = UINT32_C(1000000000);
                        }
                        else if (strcmp(unit, "GiB") == 0)
                        {
                            multiplier = UINT32_C(1073741824);
                        }
                        else
                        {
                            print_Error_In_Cmd_Line_Args(RAW_DATA_LEN_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    RAW_DATA_LEN_FLAG *= multiplier;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_DATA_LEN_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_DATA_DIRECTION_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "in") == 0)
                {
                    RAW_DATA_DIRECTION_FLAG = XFER_DATA_IN;
                }
                else if (strcmp(optarg, "out") == 0)
                {
                    RAW_DATA_DIRECTION_FLAG = XFER_DATA_OUT;
                }
                else if (strcmp(optarg, "none") == 0)
                {
                    RAW_DATA_DIRECTION_FLAG = XFER_NO_DATA;
                }
                //all other inputs are invalid for now
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_DATA_DIRECTION_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_TIMEOUT_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint32(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &RAW_TIMEOUT_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(RAW_TIMEOUT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, RAW_OUTPUT_FILE_LONG_OPT_STRING) == 0)
            {
                //open the file later
                RAW_OUTPUT_FILE_NAME_FLAG = optarg;
            }
            else if (strcmp(longopts[optionIndex].name, RAW_INPUT_FILE_LONG_OPT_STRING) == 0)
            {
                //open the file later
                RAW_INPUT_FILE_NAME_FLAG = optarg;
            }
            else if (strcmp(longopts[optionIndex].name, RAW_INPUT_FILE_OFFSET_LONG_OPT_STRING) == 0)
            {
                //set the offset to read the file at
                //set the raw data length - but check the units first!
                char* unit = M_NULLPTR;
                if (get_And_Validate_Integer_Input_Int64(optarg, &unit, ALLOW_UNIT_DATASIZE, &RAW_INPUT_FILE_OFFSET_FLAG))
                {
                    int64_t multiplier = 1;
                    if (strstr(optarg, "BLOCKS"))
                    {
                        //the user specified the number as a number of logical blocks, so adjust this after we know the device logical block size
                        RAW_INPUT_OFFSET_ADJUST_BY_BLOCKS_FLAG = true;
                    }
                    else if (strstr(optarg, "KB"))
                    {
                        multiplier = 1000;
                    }
                    else if (strstr(optarg, "KiB"))
                    {
                        multiplier = 1024;
                    }
                    else if (strstr(optarg, "MB"))
                    {
                        multiplier = 1000000;
                    }
                    else if (strstr(optarg, "MiB"))
                    {
                        multiplier = 1048576;
                    }
                    else if (strstr(optarg, "GB"))
                    {
                        multiplier = 1000000000;
                    }
                    else if (strstr(optarg, "GiB"))
                    {
                        multiplier = 1073741824;
                    }
                    else
                    {
                        print_Error_In_Cmd_Line_Args(RAW_INPUT_FILE_OFFSET_LONG_OPT_STRING, optarg);
                        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                    }
                    RAW_INPUT_FILE_OFFSET_FLAG = RAW_INPUT_FILE_OFFSET_FLAG * multiplier;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RAW_INPUT_FILE_OFFSET_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING) == 0)
            {
                MODEL_MATCH_FLAG = true;
                snprintf(MODEL_STRING_FLAG, MODEL_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING) == 0)
            {
                FW_MATCH_FLAG = true;
                snprintf(FW_STRING_FLAG, FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                snprintf(CHILD_MODEL_STRING_FLAG, CHILD_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING) == 0)
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
            if (!set_Verbosity_From_String(optarg, &toolVerbosity))
            {
                print_Error_In_Cmd_Line_Args_Short_Opt(VERBOSE_SHORT_OPT, optarg);
                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
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
            if (strcmp(argv[commandLineIter], "--echoCommandLine") == 0)
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
        scan_And_Print_Devs(scanControl, toolVerbosity);
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

    if ((RAW_TFR_COMMAND || RAW_TFR_SIZE_FLAG > 0) && (RAW_CDB_LEN_FLAG > 0 && !is_Empty(RAW_CDB_ARRAY, UINT8_MAX)))
    {
        printf("\nError: CDB or TFR. Both were specified, but only one is allowed at a time by this utility.\n");
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    //check that we were given at least one test to perform...if not, show the help and exit
    if (!(DEVICE_INFO_FLAG
        || TEST_UNIT_READY_FLAG
        //check for other tool specific options here
        || (RAW_TFR_COMMAND || RAW_TFR_SIZE_FLAG > 0)
        || (RAW_CDB_LEN_FLAG > 0 && !is_Empty(RAW_CDB_ARRAY, UINT8_MAX))
        ))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    uint64_t flags = 0;
    DEVICE_LIST = C_CAST(tDevice*, safe_calloc(DEVICE_LIST_COUNT, sizeof(tDevice)));
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

    if (!(DEVICE_INFO_FLAG || TEST_UNIT_READY_FLAG))
    {
        flags = OPEN_HANDLE_ONLY;
    }

    if (TEST_UNIT_READY_FLAG)
    {
        flags = DO_NOT_WAKE_DRIVE;
    }

    if (FAST_DISCOVERY_FLAG)
    {
        flags = FAST_SCAN;
    }

    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        
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
            deviceList[handleIter].os_info.fd = M_NULLPTR;
#elif !defined(_WIN32)
            deviceList[handleIter].os_info.fd = -1;
#if defined(VMK_CROSS_COMP)
            deviceList[handleIter].os_info.nvmeFd = M_NULLPTR;
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
                 (deviceList[handleIter].os_info.nvmeFd == M_NULLPTR)) ||
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
            if (strstr(deviceList[deviceIter].drive_info.product_identification, MODEL_STRING_FLAG) == M_NULLPTR)
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
            if (safe_strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN) == 0 || strstr(deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG) == M_NULLPTR)
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

        //TODO: hard and soft reset flags that will issue the SAT command to the device? If that fails try OS APIs?

        //validate that we were given all the info we needed..starting with a CDB
        if (RAW_CDB_LEN_FLAG > 0 || !is_Empty(RAW_CDB_ARRAY, UINT8_MAX))
        {
            //now check that we were given a valid CDB length
            if (RAW_CDB_LEN_FLAG > 0)
            {
                //check that a data direction was set
                if (RAW_DATA_DIRECTION_FLAG != -1)
                {
                    bool dataLenValidForTransferDir = true;
                    if (RAW_DATA_LEN_FLAG == 0 && RAW_DATA_DIRECTION_FLAG != XFER_NO_DATA)
                    {
                        dataLenValidForTransferDir = false;
                    }
                    if (dataLenValidForTransferDir)
                    {
                        bool showSenseData = true;//set to false upon successful completion only
                        uint8_t* dataBuffer = M_NULLPTR;//will be allocated shortly
                        uint32_t allocatedDataLength = 0;
                        const char* fileAccessMode = M_NULLPTR;
                        //now based on the data direction we need to allocate memory
                        switch (RAW_DATA_DIRECTION_FLAG)
                        {
                        case XFER_NO_DATA:
                            //nothing needs to be allocated...no data
                            break;
                        case XFER_DATA_IN:
                            //allocate memory. Also, check for an output file to open...if there isn't one, that's ok since we'll just print to the screen
                            if (!RAW_OUTPUT_FILE_NAME_FLAG)
                            {
                                //not a critical failure, just display a warning that the data won't be saved
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("WARNING: An output file was not specified, so the returned data will not be saved.\n");
                                }
                            }
                            //no "else" needed
                            if (RAW_DATA_LEN_ADJUST_BY_BLOCKS_FLAG)
                            {
                                //allocate based on logical block size
                                allocatedDataLength = deviceList[deviceIter].drive_info.deviceBlockSize * RAW_DATA_LEN_FLAG;

                            }
                            else
                            {
                                //allocate based on the data size the user entered
                                allocatedDataLength = RAW_DATA_LEN_FLAG;
                            }
                            dataBuffer = C_CAST(uint8_t*, safe_calloc_aligned(allocatedDataLength, sizeof(uint8_t), deviceList[deviceIter].os_info.minimumAlignment));
                            if (!dataBuffer)
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("ERROR: Failed to allocate memory for data in command!\n");
                                }
                                exit(UTIL_EXIT_OPERATION_FAILURE);
                            }
                            break;
                        case XFER_DATA_OUT:
                            //allocate memory. Also, check for an output file to open...if there isn't one, we need to return an error
                            if (!RAW_INPUT_FILE_NAME_FLAG)
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("ERROR: An input file is required for a data out command!\n");
                                }
                                exit(UTIL_EXIT_OPERATION_FAILURE);
                            }
                            else
                            {
                                int64_t fileOffset = RAW_INPUT_FILE_OFFSET_FLAG;
                                if (RAW_INPUT_OFFSET_ADJUST_BY_BLOCKS_FLAG)
                                {
                                    fileOffset = deviceList[deviceIter].drive_info.deviceBlockSize * RAW_INPUT_FILE_OFFSET_FLAG;
                                }
                                if (RAW_DATA_LEN_ADJUST_BY_BLOCKS_FLAG)
                                {
                                    //allocate based on logical block size
                                    allocatedDataLength = deviceList[deviceIter].drive_info.deviceBlockSize * RAW_DATA_LEN_FLAG;
                                }
                                else
                                {
                                    //allocate based on the data size the user entered
                                    allocatedDataLength = RAW_DATA_LEN_FLAG;
                                }
                                dataBuffer = C_CAST(uint8_t*, safe_calloc_aligned(allocatedDataLength, sizeof(uint8_t), deviceList[deviceIter].os_info.minimumAlignment));
                                if (!dataBuffer)
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("ERROR: Failed to allocate memory for data in command!\n");
                                    }
                                    exit(UTIL_EXIT_OPERATION_FAILURE);
                                }
                                if (RAW_INPUT_FILE_NAME_FLAG)
                                {
                                    fileAccessMode = "rb";
                                    RAW_INPUT_FILE_FLAG = secure_Open_File(RAW_INPUT_FILE_NAME_FLAG, fileAccessMode, M_NULLPTR, M_NULLPTR, M_NULLPTR);
                                    if (!RAW_INPUT_FILE_FLAG)
                                    {
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("ERROR: Failed to open file for reading data to send to drive!\n");
                                        }
                                        safe_Free_aligned(C_CAST(void**, &dataBuffer));
                                        exit(UTIL_EXIT_OPERATION_FAILURE);
                                    }
                                    eUtilExitCodes inputfilexit = UTIL_EXIT_NO_ERROR;
                                    do
                                    {
                                        if (SEC_FILE_SUCCESS != secure_Seek_File(RAW_INPUT_FILE_FLAG, fileOffset, 0))
                                        {
                                            if (VERBOSITY_QUIET < toolVerbosity)
                                            {
                                                printf("ERROR: Failed to seek to specified offset in file!\n");
                                            }
                                            inputfilexit = UTIL_EXIT_OPERATION_FAILURE;
                                            break;
                                        }
                                        //now copy this data to the data buffer before we send the command
                                        if (SEC_FILE_SUCCESS != secure_Read_File(RAW_INPUT_FILE_FLAG, dataBuffer, allocatedDataLength, sizeof(uint8_t), allocatedDataLength, M_NULLPTR))
                                        {
                                            if (VERBOSITY_QUIET < toolVerbosity)
                                            {
                                                printf("ERROR: Failed to read file for datalen specified to send to drive!\n");
                                            }
                                            inputfilexit = UTIL_EXIT_OPERATION_FAILURE;
                                            break;
                                        }
                                    }
                                    while (0);

                                    //close the file now that we are done reading it
                                    if (SEC_FILE_SUCCESS != secure_Close_File(RAW_INPUT_FILE_FLAG))
                                    {
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("ERROR: Unable to close handle to input file!\n");
                                        }
                                        inputfilexit = UTIL_EXIT_OPERATION_FAILURE;
                                    }
                                    free_Secure_File_Info(&RAW_INPUT_FILE_FLAG);
                                    if (inputfilexit != UTIL_EXIT_NO_ERROR)
                                    {
                                        safe_Free_aligned(C_CAST(void**, &dataBuffer));
                                        exit(C_CAST(int, inputfilexit));
                                    }
                                }
                            }
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("ERROR: Unknown Data Direction. Utility Failure!\n");
                            }
                            exit(UTIL_EXIT_OPERATION_FAILURE);
                            break;
                        }
                        //try issuing the command now
                        switch (scsi_Send_Cdb(&deviceList[deviceIter], RAW_CDB_ARRAY, C_CAST(eCDBLen, RAW_CDB_LEN_FLAG), dataBuffer, allocatedDataLength, C_CAST(eDataTransferDirection, RAW_DATA_DIRECTION_FLAG), deviceList[deviceIter].drive_info.lastCommandSenseData, SPC3_SENSE_LEN, RAW_TIMEOUT_FLAG))
                        {
                        case IN_PROGRESS://separate case so we can save the sense data
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Command completed with an in progress status\n");
                            }
                            //check for an output file to open and save the data to if, if it's a data in transfer
                            if (RAW_OUTPUT_FILE_NAME_FLAG)
                            {
                                fileAccessMode = "ab";
                                RAW_OUTPUT_FILE_FLAG = secure_Open_File(RAW_OUTPUT_FILE_NAME_FLAG, fileAccessMode, M_NULLPTR, M_NULLPTR, M_NULLPTR);
                                if (!RAW_OUTPUT_FILE_FLAG)
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("ERROR: Failed to open/create file for saving returned data!\n");
                                    }
                                    exit(UTIL_EXIT_OPERATION_FAILURE);
                                }
                            }
                            break;
                        case SUCCESS:
                            showSenseData = false;//no need to show the sense data or save it...
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Command completed with good status\n");
                            }
                            //check for an output file to open and save the data to if, if it's a data in transfer
                            if (RAW_OUTPUT_FILE_NAME_FLAG && RAW_DATA_DIRECTION_FLAG == XFER_DATA_IN)
                            {
                                fileAccessMode = "ab";
                                RAW_OUTPUT_FILE_FLAG = secure_Open_File(RAW_OUTPUT_FILE_NAME_FLAG, fileAccessMode, M_NULLPTR, M_NULLPTR, M_NULLPTR);
                                if (!RAW_OUTPUT_FILE_FLAG)
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("ERROR: Failed to open/create file for saving returned data!\n");
                                    }
                                    exit(UTIL_EXIT_OPERATION_FAILURE);
                                }
                            }
                            break;
                        case NOT_SUPPORTED:
                        case OS_PASSTHROUGH_FAILURE:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Operation not supported by low level driver or HBA.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        case FAILURE:
                        case COMMAND_FAILURE:
                        case ABORTED:
                        case FROZEN:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Command completed with a failing status. See sense data.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            //check for an output file to open and save the data to if, if it's a data in transfer
                            if (RAW_OUTPUT_FILE_NAME_FLAG && RAW_DATA_DIRECTION_FLAG != XFER_DATA_IN)
                            {
                                fileAccessMode = "ab";
                                RAW_OUTPUT_FILE_FLAG = secure_Open_File(RAW_OUTPUT_FILE_NAME_FLAG, fileAccessMode, M_NULLPTR, M_NULLPTR, M_NULLPTR);
                                if (!RAW_OUTPUT_FILE_FLAG)
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("ERROR: Failed to open/create file for saving returned sense data!\n");
                                    }
                                    exit(UTIL_EXIT_OPERATION_FAILURE);
                                }
                            }
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("An unknown internal error occured and cannot be recovered. Sense data not available.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                        //if there is an outputfile, 
                        if (RAW_OUTPUT_FILE_FLAG)
                        {
                            if (RAW_DATA_DIRECTION_FLAG == XFER_DATA_IN)
                            {
                                if (SEC_FILE_SUCCESS != secure_Write_File(RAW_OUTPUT_FILE_FLAG, dataBuffer, allocatedDataLength, sizeof(uint8_t), allocatedDataLength, M_NULLPTR))
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("Error writing data to a file!\n");
                                    }
                                    exitCode = UTIL_EXIT_ERROR_WRITING_FILE;
                                }
                            }
                            else
                            {
                                //save the sense data
                                if (SEC_FILE_SUCCESS != secure_Write_File(RAW_OUTPUT_FILE_FLAG, deviceList[deviceIter].drive_info.lastCommandSenseData, SPC3_SENSE_LEN, sizeof(uint8_t), SPC3_SENSE_LEN, M_NULLPTR))
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("Error writing sense data to a file!\n");
                                    }
                                    exitCode = UTIL_EXIT_ERROR_WRITING_FILE;
                                }
                            }
                            if (RAW_OUTPUT_FILE_FLAG && SEC_FILE_SUCCESS != secure_Flush_File(RAW_OUTPUT_FILE_FLAG))
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Error flushing data!\n");
                                }
                                exitCode = UTIL_EXIT_ERROR_WRITING_FILE;
                            }
                            if (RAW_OUTPUT_FILE_FLAG)
                            {
                                //close the file now that we are done reading it
                                if (SEC_FILE_SUCCESS != secure_Close_File(RAW_OUTPUT_FILE_FLAG))
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("ERROR: Unable to close handle to output file!\n");
                                    }
                                }
                                free_Secure_File_Info(&RAW_OUTPUT_FILE_FLAG);
                            }
                        }

                        //show the returned data on data in commands when verbosity is less than 3
                        if (RAW_DATA_DIRECTION_FLAG == XFER_DATA_IN && toolVerbosity < VERBOSITY_BUFFERS)
                        {
                            printf("\nReturned Data:\n");
                            print_Data_Buffer(dataBuffer, allocatedDataLength, true);
                        }

                        //show the sense data if the flag is set and verbosity is less than 3
                        if (toolVerbosity < VERBOSITY_BUFFERS && showSenseData)
                        {
                            printf("\nSense Data:\n");
                            print_Data_Buffer(deviceList[deviceIter].drive_info.lastCommandSenseData, SPC3_SENSE_LEN, true);
                        }
                        safe_Free_aligned(C_CAST(void**, &dataBuffer));
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("ERROR: Data Length is required for data in and data out commands!\n");
                        }
                        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: Data Direction was not given!\n");
                    }
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: CDB Length not given!\n");
                }
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            }
        }

        //perform some sort of validation to see that we have some command to send...then build it and send it.
        if (RAW_TFR_SIZE_FLAG != 0 && RAW_TFR_PROTOCOL != -1 && RAW_TFR_XFER_LENGTH_LOCATION != -1 && RAW_TFR_BYTE_BLOCK != -1)
        {
            ataPassthroughCommand passthroughCommand;
            memset(&passthroughCommand, 0, sizeof(ataPassthroughCommand));
            passthroughCommand.tfr.CommandStatus = RAW_TFR_COMMAND;
            passthroughCommand.tfr.ErrorFeature = RAW_TFR_FEATURE;
            passthroughCommand.tfr.Feature48 = RAW_TFR_FEATURE_EXT;
            passthroughCommand.tfr.LbaLow = RAW_TFR_LBA_LOW;
            passthroughCommand.tfr.LbaMid = RAW_TFR_LBA_MID;
            passthroughCommand.tfr.LbaHi = RAW_TFR_LBA_HIGH;
            passthroughCommand.tfr.LbaLow48 = RAW_TFR_LBA_LOW_EXT;
            passthroughCommand.tfr.LbaMid48 = RAW_TFR_LBA_MID_EXT;
            passthroughCommand.tfr.LbaHi48 = RAW_TFR_LBA_HIGH_EXT;
            passthroughCommand.tfr.DeviceHead = RAW_TFR_DEVICE_HEAD;
            passthroughCommand.tfr.SectorCount = RAW_TFR_SECTOR_COUNT;
            passthroughCommand.tfr.SectorCount48 = RAW_TFR_SECTOR_COUNT_EXT;
            passthroughCommand.tfr.icc = RAW_TFR_ICC;
            passthroughCommand.tfr.aux1 = RAW_TFR_AUX1;
            passthroughCommand.tfr.aux2 = RAW_TFR_AUX2;
            passthroughCommand.tfr.aux3 = RAW_TFR_AUX3;
            passthroughCommand.tfr.aux4 = RAW_TFR_AUX4;

            //set check condition
            passthroughCommand.forceCheckConditionBit = RAW_TFR_CHECK_CONDITION;

            //set the protocol
            passthroughCommand.commadProtocol = C_CAST(eAtaProtocol, RAW_TFR_PROTOCOL);

            //set xfer direction
            passthroughCommand.commandDirection = C_CAST(eDataTransferDirection, RAW_DATA_DIRECTION_FLAG);

            //set where the length is in the command
            passthroughCommand.ataCommandLengthLocation = C_CAST(eATAPassthroughLength, RAW_TFR_XFER_LENGTH_LOCATION);

            //check if we are doing a transfer based on logical blocks
            switch (RAW_TFR_BYTE_BLOCK)
            {
            case 0:
                passthroughCommand.ataTransferBlocks = ATA_PT_NO_DATA_TRANSFER;
                break;
            case 1:
                passthroughCommand.ataTransferBlocks = ATA_PT_NUMBER_OF_BYTES;
                break;
            case UINT8_MAX:
                passthroughCommand.ataTransferBlocks = ATA_PT_LOGICAL_SECTOR_SIZE;
                break;
            case 512:
            default:
                passthroughCommand.ataTransferBlocks = ATA_PT_512B_BLOCKS;
                break;
            }

            //set the command size
            switch (RAW_TFR_SIZE_FLAG)
            {
            case 28:
                passthroughCommand.commandType = ATA_CMD_TYPE_TASKFILE;
                break;
            case 48:
                passthroughCommand.commandType = ATA_CMD_TYPE_EXTENDED_TASKFILE;
                break;
            case 0xFF:
                passthroughCommand.commandType = ATA_CMD_TYPE_COMPLETE_TASKFILE;
                break;
            default:
                passthroughCommand.commandType = ATA_CMD_TYPE_UNKNOWN;
                break;
            }

            if (passthroughCommand.commandType != ATA_CMD_TYPE_COMPLETE_TASKFILE && (passthroughCommand.tfr.icc || passthroughCommand.tfr.aux1 || passthroughCommand.tfr.aux2 || passthroughCommand.tfr.aux3 || passthroughCommand.tfr.aux4))
            {
                //If ANY of these registers are set, we MUST use a complete taskfile
                passthroughCommand.commandType = ATA_CMD_TYPE_COMPLETE_TASKFILE;
            }

            //set the timeout
            passthroughCommand.timeout = RAW_TIMEOUT_FLAG;

            //perform some validation based on the things we've filled in so far before we issue the command.
            //check that a data direction was set
            if (RAW_DATA_DIRECTION_FLAG != -1)
            {
                bool dataLenValidForTransferDir = true;
                if (RAW_DATA_LEN_FLAG == 0 && RAW_DATA_DIRECTION_FLAG != XFER_NO_DATA)
                {
                    dataLenValidForTransferDir = false;
                }
                if (dataLenValidForTransferDir)
                {
                    bool showSenseData = true;//set to false upon successfil completion only
                    uint8_t* dataBuffer = M_NULLPTR;//will be allocated shortly
                    uint32_t allocatedDataLength = 0;
                    const char* fileAccessMode = M_NULLPTR;
                    //now based on the data direction we need to allocate memory
                    switch (RAW_DATA_DIRECTION_FLAG)
                    {
                    case XFER_NO_DATA:
                        //nothing needs to be allocated...no data
                        //TODO: check that all the non-data flags are set the same!
                        break;
                    case XFER_DATA_IN:
                        //allocate memory. Also, check for an output file to open...if there isn't one, that's ok since we'll just print to the screen
                        if (!RAW_OUTPUT_FILE_NAME_FLAG)
                        {
                            //not a critical failure, just display a warning that the data won't be saved
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("WARNING: An output file was not specified, so the returned data will not be saved.\n");
                            }
                        }
                        //no "else" needed
                        if (RAW_DATA_LEN_ADJUST_BY_BLOCKS_FLAG)
                        {
                            //allocate based on logical block size
                            allocatedDataLength = deviceList[deviceIter].drive_info.deviceBlockSize * RAW_DATA_LEN_FLAG;

                        }
                        else
                        {
                            //allocate based on the data size the user entered
                            allocatedDataLength = RAW_DATA_LEN_FLAG;
                        }
                        dataBuffer = C_CAST(uint8_t*, safe_calloc_aligned(allocatedDataLength, sizeof(uint8_t), deviceList[deviceIter].os_info.minimumAlignment));
                        if (!dataBuffer)
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("ERROR: Failed to allocate memory for data in command!\n");
                            }
                            exit(UTIL_EXIT_OPERATION_FAILURE);
                        }
                        break;
                    case XFER_DATA_OUT:
                        //allocate memory. Also, check for an output file to open...if there isn't one, we need to return an error
                        if (!RAW_INPUT_FILE_NAME_FLAG)
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("ERROR: An input file is required for a data out command!\n");
                            }
                            exit(UTIL_EXIT_OPERATION_FAILURE);
                        }
                        else
                        {
                            int64_t fileOffset = RAW_INPUT_FILE_OFFSET_FLAG;
                            if (RAW_INPUT_OFFSET_ADJUST_BY_BLOCKS_FLAG)
                            {
                                fileOffset = deviceList[deviceIter].drive_info.deviceBlockSize * RAW_INPUT_FILE_OFFSET_FLAG;
                            }
                            if (RAW_DATA_LEN_ADJUST_BY_BLOCKS_FLAG)
                            {
                                //allocate based on logical block size
                                allocatedDataLength = deviceList[deviceIter].drive_info.deviceBlockSize * RAW_DATA_LEN_FLAG;
                            }
                            else
                            {
                                //allocate based on the data size the user entered
                                allocatedDataLength = RAW_DATA_LEN_FLAG;
                            }
                            dataBuffer = C_CAST(uint8_t*, safe_calloc_aligned(allocatedDataLength, sizeof(uint8_t), deviceList[deviceIter].os_info.minimumAlignment));
                            if (!dataBuffer)
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("ERROR: Failed to allocate memory for data in command!\n");
                                }
                                exit(UTIL_EXIT_OPERATION_FAILURE);
                            }
                            if (RAW_INPUT_FILE_NAME_FLAG)
                            {
                                fileAccessMode = "rb";
                                RAW_INPUT_FILE_FLAG = secure_Open_File(RAW_INPUT_FILE_NAME_FLAG, fileAccessMode, M_NULLPTR, M_NULLPTR, M_NULLPTR);
                                if (!RAW_INPUT_FILE_FLAG)
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("ERROR: Failed to open file for reading data to send to drive!\n");
                                    }
                                    safe_Free_aligned(C_CAST(void**, &dataBuffer));
                                    exit(UTIL_EXIT_OPERATION_FAILURE);
                                }
                                eUtilExitCodes inputfilexit = UTIL_EXIT_NO_ERROR;
                                do
                                {
                                    if (SEC_FILE_SUCCESS != secure_Seek_File(RAW_INPUT_FILE_FLAG, fileOffset, 0))
                                    {
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("ERROR: Failed to seek to specified offset in file!\n");
                                        }
                                        inputfilexit = UTIL_EXIT_OPERATION_FAILURE;
                                        break;
                                    }
                                    //now copy this data to the data buffer before we send the command
                                    if (SEC_FILE_SUCCESS != secure_Read_File(RAW_INPUT_FILE_FLAG, dataBuffer, allocatedDataLength, sizeof(uint8_t), allocatedDataLength, M_NULLPTR))
                                    {
                                        if (VERBOSITY_QUIET < toolVerbosity)
                                        {
                                            printf("ERROR: Failed to read file for datalen specified to send to drive!\n");
                                        }
                                        inputfilexit = UTIL_EXIT_OPERATION_FAILURE;
                                        break;
                                    }
                                } while (0);

                                //close the file now that we are done reading it
                                if (SEC_FILE_SUCCESS != secure_Close_File(RAW_INPUT_FILE_FLAG))
                                {
                                    if (VERBOSITY_QUIET < toolVerbosity)
                                    {
                                        printf("ERROR: Unable to close handle to input file!\n");
                                    }
                                    inputfilexit = UTIL_EXIT_OPERATION_FAILURE;
                                }
                                free_Secure_File_Info(&RAW_INPUT_FILE_FLAG);
                                if (inputfilexit != UTIL_EXIT_NO_ERROR)
                                {
                                    safe_Free_aligned(C_CAST(void**, &dataBuffer));
                                    exit(C_CAST(int, inputfilexit));
                                }
                            }
                        }
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("ERROR: Unknown Data Direction. Utility Failure!\n");
                        }
                        exit(UTIL_EXIT_OPERATION_FAILURE);
                        break;
                    }
                    passthroughCommand.ptrData = dataBuffer;
                    passthroughCommand.dataSize = allocatedDataLength;
                    //try issuing the command now
                    switch (ata_Passthrough_Command(&deviceList[deviceIter], &passthroughCommand))
                    {
                    case IN_PROGRESS://separate case so we can save the sense data
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Command completed with an in progress status\n");
                        }
                        //check for an output file to open and save the data to if, if it's a data in transfer
                        if (RAW_OUTPUT_FILE_NAME_FLAG)
                        {
                            fileAccessMode = "ab";
                            RAW_OUTPUT_FILE_FLAG = secure_Open_File(RAW_OUTPUT_FILE_NAME_FLAG, fileAccessMode, M_NULLPTR, M_NULLPTR, M_NULLPTR);
                            if (!RAW_OUTPUT_FILE_FLAG)
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("ERROR: Failed to open/create file for saving returned data!\n");
                                }
                                exit(UTIL_EXIT_OPERATION_FAILURE);
                            }
                        }
                        break;
                    case SUCCESS:
                        showSenseData = false;//no need to show the sense data or save it...
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Command completed with good status\n");
                        }
                        //check for an output file to open and save the data to if, if it's a data in transfer
                        if (RAW_OUTPUT_FILE_NAME_FLAG && RAW_DATA_DIRECTION_FLAG == XFER_DATA_IN)
                        {
                            fileAccessMode = "ab";
                            RAW_OUTPUT_FILE_FLAG = secure_Open_File(RAW_OUTPUT_FILE_NAME_FLAG, fileAccessMode, M_NULLPTR, M_NULLPTR, M_NULLPTR);
                            if (!RAW_OUTPUT_FILE_FLAG)
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("ERROR: Failed to open/create file for saving returned data!\n");
                                }
                                exit(UTIL_EXIT_OPERATION_FAILURE);
                            }
                        }
                        break;
                    case NOT_SUPPORTED:
                    case OS_PASSTHROUGH_FAILURE:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Operation not supported by low level driver or HBA.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    case FAILURE:
                    case COMMAND_FAILURE:
                    case ABORTED:
                    case FROZEN:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Command completed with a failing status. See sense data.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        //check for an output file to open and save the data to if, if it's a data in transfer
                        if (RAW_OUTPUT_FILE_NAME_FLAG && RAW_DATA_DIRECTION_FLAG != XFER_DATA_IN)
                        {
                            fileAccessMode = "ab";
                            RAW_OUTPUT_FILE_FLAG = secure_Open_File(RAW_OUTPUT_FILE_NAME_FLAG, fileAccessMode, M_NULLPTR, M_NULLPTR, M_NULLPTR);
                            if (!RAW_OUTPUT_FILE_FLAG)
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("ERROR: Failed to open/create file for saving returned sense data!\n");
                                }
                                exit(UTIL_EXIT_OPERATION_FAILURE);
                            }
                        }
                        break;
                    default:
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("An unknown internal error occured and cannot be recovered. Sense data not available.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    //if there is an outputfile, 
                    if (RAW_OUTPUT_FILE_FLAG)
                    {
                        if (RAW_DATA_DIRECTION_FLAG == XFER_DATA_IN)
                        {
                            if (SEC_FILE_SUCCESS != secure_Write_File(RAW_OUTPUT_FILE_FLAG, dataBuffer, allocatedDataLength, sizeof(uint8_t), allocatedDataLength, M_NULLPTR))
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Error writing data to a file!\n");
                                }
                                exitCode = UTIL_EXIT_ERROR_WRITING_FILE;
                            }
                        }
                        else
                        {
                            //save the sense data
                            if (SEC_FILE_SUCCESS != secure_Write_File(RAW_OUTPUT_FILE_FLAG, deviceList[deviceIter].drive_info.lastCommandSenseData, SPC3_SENSE_LEN, sizeof(uint8_t), SPC3_SENSE_LEN, M_NULLPTR))
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Error writing sense data to a file!\n");
                                }
                                exitCode = UTIL_EXIT_ERROR_WRITING_FILE;
                            }
                        }
                        if (RAW_OUTPUT_FILE_FLAG && SEC_FILE_SUCCESS != secure_Flush_File(RAW_OUTPUT_FILE_FLAG))
                        {
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Error flushing data!\n");
                            }
                            exitCode = UTIL_EXIT_ERROR_WRITING_FILE;
                        }
                        if (RAW_OUTPUT_FILE_FLAG)
                        {
                            //close the file now that we are done reading it
                            if (SEC_FILE_SUCCESS != secure_Close_File(RAW_OUTPUT_FILE_FLAG))
                            {
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("ERROR: Unable to close handle to output file!\n");
                                }
                            }
                            free_Secure_File_Info(&RAW_OUTPUT_FILE_FLAG);
                        }
                    }


                    //show the returned data on data in commands when verbosity is less than 3
                    if (RAW_DATA_DIRECTION_FLAG == XFER_DATA_IN && toolVerbosity < VERBOSITY_BUFFERS)
                    {
                        printf("\nReturned Data:\n");
                        print_Data_Buffer(dataBuffer, allocatedDataLength, true);
                    }

                    //show the sense data if the flag is set and verbosity is less than 3
                    if (toolVerbosity < VERBOSITY_BUFFERS && showSenseData)
                    {
                        printf("\nRTFRs:\n");
                        printf("\t[Error] = %02" PRIX8 "h\n", passthroughCommand.rtfr.error);
                        if (passthroughCommand.commandType == ATA_CMD_TYPE_EXTENDED_TASKFILE || passthroughCommand.commandType == ATA_CMD_TYPE_COMPLETE_TASKFILE)
                        {
                            printf("\t[Count Ext] = %02" PRIX8 "h\n", passthroughCommand.rtfr.secCntExt);
                        }
                        printf("\t[Count] = %02" PRIX8 "h\n", passthroughCommand.rtfr.secCnt);
                        if (passthroughCommand.commandType == ATA_CMD_TYPE_EXTENDED_TASKFILE || passthroughCommand.commandType == ATA_CMD_TYPE_COMPLETE_TASKFILE)
                        {
                            printf("\t[LBA Lo Ext] = %02" PRIX8 "h\n", passthroughCommand.rtfr.lbaLowExt);
                        }
                        printf("\t[LBA Lo] = %02" PRIX8 "h\n", passthroughCommand.rtfr.lbaLow);
                        if (passthroughCommand.commandType == ATA_CMD_TYPE_EXTENDED_TASKFILE || passthroughCommand.commandType == ATA_CMD_TYPE_COMPLETE_TASKFILE)
                        {
                            printf("\t[LBA Mid Ext] = %02" PRIX8 "h\n", passthroughCommand.rtfr.lbaMidExt);
                        }
                        printf("\t[LBA Mid] = %02" PRIX8 "h\n", passthroughCommand.rtfr.lbaMid);
                        if (passthroughCommand.commandType == ATA_CMD_TYPE_EXTENDED_TASKFILE || passthroughCommand.commandType == ATA_CMD_TYPE_COMPLETE_TASKFILE)
                        {
                            printf("\t[LBA Hi Ext] = %02" PRIX8 "h\n", passthroughCommand.rtfr.lbaHiExt);
                        }
                        printf("\t[LBA Hi] = %02" PRIX8 "h\n", passthroughCommand.rtfr.lbaHi);
                        printf("\t[Device] = %02" PRIX8 "h\n", passthroughCommand.rtfr.device);
                        printf("\t[Status] = %02" PRIX8 "h\n", passthroughCommand.rtfr.status);
                        printf("\n");
                        printf("\nSense Data:\n");
                        print_Data_Buffer(deviceList[deviceIter].drive_info.lastCommandSenseData, SPC3_SENSE_LEN, true);
                    }
                    safe_Free_aligned(C_CAST(void**, &dataBuffer));
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("ERROR: Data Length is required for data in and data out commands!\n");
                    }
                    exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("ERROR: Data Direction was not given!\n");
                }
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            }
        }
        else
        {
            //try to tell the user which flags they missed setting
            if (RAW_TFR_SIZE_FLAG == 0)
            {
                printf("You must add the --%s [28 | 48 | complete] command line option\n", RAW_TFR_SIZE_LONG_OPT_STRING);
            }
            if (RAW_TFR_PROTOCOL == -1)
            {
                printf("You must add the --%s [pio | dma | udma | fpdma | ncq | nodata | diag | dmaque | reset] command line option\n", RAW_TFR_PROTOCOL_LONG_OPT_STRING);
            }
            if (RAW_TFR_XFER_LENGTH_LOCATION == -1)
            {
                printf("You must add the --%s [sectorCount | feature | tpsiu | nodata] command line option\n", RAW_TFR_XFER_LENGTH_LOCATION_LONG_OPT_STRING);
            }
            if (RAW_TFR_BYTE_BLOCK == -1)
            {
                printf("You must add the --%s [512 | logical | bytes | nodata] command line option\n", RAW_TFR_BYTE_BLOCK_LONG_OPT_STRING);
            }
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
        }
        //At this point, close the device handle since it is no longer needed. Do not put any further IO below this.
        close_Device(&deviceList[deviceIter]);
    }
    safe_Free(C_CAST(void**, &DEVICE_LIST));
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
    printf("\t%s --%s\n", util_name, SCAN_LONG_OPT_STRING);
    printf("\t%s -d %s -%c\n", util_name, deviceHandleExample, DEVICE_INFO_SHORT_OPT);
    printf("\tIdentify device:\n");
    printf("\t\t-d %s --dataDir in  --dataLen 512  --outputFile ID_dev.bin --tfrByteBlock 512  --tfrProtocol pio  --tfrSize 28  --command ECh --tfrXferLengthReg sectorCount --sectorCount 1\n", deviceHandleExample);
    //return codes
    printf("\nReturn codes\n");
    printf("============\n");
    print_SeaChest_Util_Exit_Codes(0, M_NULLPTR, util_name);

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
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    //multiple interfaces
    print_Fast_Discovery_Help(shortUsage);
    print_Raw_Data_Direction_Help(shortUsage);
    print_Raw_Data_Length_Help(shortUsage);
    print_Raw_Input_File_Help(shortUsage);
    print_Raw_Input_File_Offset_Help(shortUsage);
    print_Raw_Output_File_Help(shortUsage);
    print_Raw_Timeout_Help(shortUsage);

    //SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_Raw_TFR_AUX1_Help(shortUsage);
    print_Raw_TFR_AUX2_Help(shortUsage);
    print_Raw_TFR_AUX3_Help(shortUsage);
    print_Raw_TFR_AUX4_Help(shortUsage);
    print_Raw_TFR_Aux_Full_Help(shortUsage);
    print_Raw_TFR_Command_Help(shortUsage);
    print_Raw_TFR_Device_Head_Help(shortUsage);
    print_Raw_TFR_Feature_Help(shortUsage);
    print_Raw_TFR_Feature_Ext_Help(shortUsage);
    print_Raw_TFR_Feature_Full_Help(shortUsage);
    print_Raw_TFR_ICC_Help(shortUsage);
    print_Raw_TFR_LBA_Full_Help(shortUsage);
    print_Raw_TFR_LBA_High_Help(shortUsage);
    print_Raw_TFR_LBA_High_Ext_Help(shortUsage);
    print_Raw_TFR_LBA_Low_Help(shortUsage);
    print_Raw_TFR_LBA_Low_Ext_Help(shortUsage);
    print_Raw_TFR_LBA_Mode_Help(shortUsage);
    print_Raw_TFR_LBA_Mid_Help(shortUsage);
    print_Raw_TFR_LBA_Mid_Ext_Help(shortUsage);
    print_Raw_TFR_Sector_Count_Help(shortUsage);
    print_Raw_TFR_Sector_Count_Ext_Help(shortUsage);
    print_Raw_TFR_Sector_Count_Full_Help(shortUsage);
    print_Raw_TFR_Byte_Block_Help(shortUsage);
    print_Raw_TFR_Protocol_Help(shortUsage);
    print_Raw_TFR_Check_Condition_Help(shortUsage);
    print_Raw_TFR_Size_Help(shortUsage);
    print_Raw_TFR_XFer_Length_Register_Help(shortUsage);

    //SAS/SCSI Only
    printf("\n\tSAS Only:\n\t=========\n");
    print_Raw_CDB_Help(shortUsage);
    print_Raw_CDB_Length_Help(shortUsage);
}
