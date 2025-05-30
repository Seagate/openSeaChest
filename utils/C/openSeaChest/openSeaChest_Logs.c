// SPDX-License-Identifier: MPL-2.0
//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2025 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
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
#include "common_types.h"
#include "io_utils.h"
#include "memory_safety.h"
#include "secure_file.h"
#include "string_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"

#include "EULA.h"
#include "common_public.h"
#include "getopt.h"
#include "operations.h"
// include the seachest util options for the device option, drive info option and a few other things that are needed.
#include "drive_info.h"
#include "farm_log.h"
#include "logs.h"
#include "openseachest_util_options.h"
#include "smart.h"

////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name    = "openSeaChest_Logs";
const char* buildVersion = "2.6.0";

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
int main(int argc, char* argv[])
{
    /////////////////
    //  Variables  //
    /////////////////
    // common utility variables
    // clang-format off
    eReturnValues ret      = SUCCESS;
    int           exitCode = UTIL_EXIT_NO_ERROR;
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
    // scan output flags
    SCAN_FLAGS_UTIL_VARS
    // add tool specific flags here
    LIST_LOGS_VAR
    LIST_ERROR_HISTORY_VAR
    GENERIC_LOG_VAR
    GENERIC_LOG_SUBPAGE_VAR
    GENERIC_ERROR_HISTORY_VARS
    PULL_LOG_MODE_VAR
    FARM_VAR
    FARM_COMBINED_VAR
    SATA_FARM_COPY_TYPE_VARS
    DST_LOG_VAR
    IDENTIFY_DEVICE_DATA_LOG_VAR
    SATA_PHY_COUNTERS_LOG_VAR
    DEVICE_STATS_LOG_VAR
    GET_TELEMETRY_VAR
    TELEMETRY_DATA_AREA_VAR
    INFORMATIONAL_EXCEPTIONS_VAR

#if defined(ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    LOG_TRANSFER_LENGTH_BYTES_VAR
    LOG_LENGTH_BYTES_VAR
    LOWLEVEL_INFO_VAR

    int args        = 0;
    int argIndex    = 0;
    int optionIndex = 0;

    struct option longopts[] =
    {
        // common command line options
        DEVICE_LONG_OPT,
        HELP_LONG_OPT,
        DEVICE_INFO_LONG_OPT,
        SAT_INFO_LONG_OPT,
        
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
        LOWLEVEL_INFO_LONG_OPT,
#if defined(ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        // Utility specific options
        LIST_LOGS_LONG_OPT,
        LIST_ERROR_HISTORY_LONG_OPT,
        GENERIC_LOG_LONG_OPT,
        GENERIC_LOG_SUBPAGE_LONG_OPT,
        GENERIC_ERROR_HISTORY_LONG_OPT,
        PULL_LOG_MODE_LONG_OPT,
        FARM_LONG_OPT,
        FARM_COMBINED_LONG_OPT,
        SATA_FARM_COPY_TYPE_LONG_OPT,
		GET_TELEMETRY_LONG_OPT,
		TELEMETRY_DATA_AREA_LONG_OPT,
        DST_LOG_LONG_OPT,                  // standard spec log
        DEVICE_STATS_LOG_LONG_OPT,         // standard spec log
        IDENTIFY_DEVICE_DATA_LOG_LONG_OPT, // standard ATA spec log
        SATA_PHY_COUNTERS_LONG_OPT,        // standard ATA spec log
        INFROMATIONAL_EXCEPTIONS_LONG_OPT,
        LOG_TRANSFER_LENGTH_LONG_OPT,
        LOG_LENGTH_LONG_OPT,
        LONG_OPT_TERMINATOR
    };
    // clang-format on

    eVerbosityLevels toolVerbosity = VERBOSITY_DEFAULT;

#if defined(UEFI_C_SOURCE)
    // NOTE: This is a BSD function used to ensure the program name is set correctly for warning or error functions.
    //       This is not necessary on most modern systems other than UEFI.
    //       This is not used in linux so that we don't depend on libbsd
    //       Update the above #define check if we port to another OS that needs this to be done.
    setprogname(util_name);
#endif

    ////////////////////////
    //  Argument Parsing  //
    ////////////////////////
    if (argc < 2)
    {
        openseachest_utility_Info(util_name, buildVersion);
        utility_Usage(true);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    // get options we know we need
    while (1) // changed to while 1 in order to parse multiple options when longs options are involved
    {
        args = getopt_long(argc, argv, "d:hisSF:Vv:qr:R:E:e:w:c:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        // printf("Parsing args <%u> %s\n", args, longopts[optionIndex].name);
        switch (args)
        {
        case 0:
            if (strcmp(longopts[optionIndex].name, GENERIC_LOG_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint8(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                         &GENERIC_LOG_DATA_SET))
                {
                    GENERIC_LOG_PULL_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(GENERIC_LOG_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, GENERIC_LOG_SUBPAGE_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &GENERIC_LOG_SUBPAGE_DATA_SET))
                {
                    print_Error_In_Cmd_Line_Args(GENERIC_LOG_SUBPAGE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, GENERIC_ERROR_HISTORY_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &GENERIC_ERROR_HISTORY_BUFFER_ID))
                {
                    GENERIC_ERROR_HISTORY_PULL_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(GENERIC_ERROR_HISTORY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, LOG_TRANSFER_LENGTH_LONG_OPT_STRING) == 0)
            {
                char* unit = M_NULLPTR;
                if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_DATASIZE,
                                                          &LOG_TRANSFER_LENGTH_BYTES))
                {
                    // Check to see if any units were specified, otherwise assume LBAs
                    uint32_t multiplier = UINT32_C(1);
                    if (unit)
                    {
                        if (strcmp(unit, "") == 0)
                        {
                            multiplier = 1; // no additional units provided, so do not treat as an error
                        }
                        else if (strcmp(unit, "BLOCKS") == 0 || strcmp(unit, "SECTORS") == 0)
                        {
                            // they specified blocks. For log transfers this means a number of 512B sectors
                            multiplier = LEGACY_DRIVE_SEC_SIZE;
                        }
                        else if (strcmp(unit, "KB") == 0)
                        {
                            multiplier = UINT64_C(1000);
                        }
                        else if (strcmp(unit, "KiB") == 0)
                        {
                            multiplier = UINT64_C(1024);
                        }
                        else if (strcmp(unit, "MB") == 0)
                        {
                            multiplier = UINT64_C(1000000);
                        }
                        else if (strcmp(unit, "MiB") == 0)
                        {
                            multiplier = UINT64_C(1048576);
                        }
                        else
                        {
                            print_Error_In_Cmd_Line_Args(LOG_TRANSFER_LENGTH_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    LOG_TRANSFER_LENGTH_BYTES *= multiplier;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(LOG_TRANSFER_LENGTH_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, LOG_LENGTH_LONG_OPT_STRING) == 0)
            {
                char* unit = M_NULLPTR;
                if (get_And_Validate_Integer_Input_Uint32(optarg, &unit, ALLOW_UNIT_DATASIZE, &LOG_LENGTH_BYTES))
                {
                    // Check to see if any units were specified, otherwise assume LBAs
                    uint32_t multiplier = UINT32_C(1);
                    if (unit)
                    {
                        if (strcmp(unit, "") == 0)
                        {
                            multiplier = 1; // no additional units provided, so do not treat as an error
                        }
                        else if (strcmp(unit, "BLOCKS") == 0 || strcmp(unit, "SECTORS") == 0)
                        {
                            // they specified blocks. For log transfers this means a number of 512B sectors
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
                            print_Error_In_Cmd_Line_Args(LOG_LENGTH_LONG_OPT_STRING, optarg);
                            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                        }
                    }
                    LOG_LENGTH_BYTES *= multiplier;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(LOG_LENGTH_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, TELEMETRY_DATA_AREA_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(C_CAST(const char*, optarg), M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &TELEMETRY_DATA_AREA) ||
                    (TELEMETRY_DATA_AREA > 4 || TELEMETRY_DATA_AREA < 1))
                {
                    print_Error_In_Cmd_Line_Args(TELEMETRY_DATA_AREA_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, PATH_LONG_OPT_STRING) == 0)
            {
                OUTPUTPATH_PARSE
                if (!os_Directory_Exists(OUTPUTPATH_FLAG))
                {
                    printf("Err: --outputPath %s does not exist\n", OUTPUTPATH_FLAG);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING) == 0)
            {
                MODEL_MATCH_FLAG = true;
                snprintf_err_handle(MODEL_STRING_FLAG, MODEL_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING) == 0)
            {
                FW_MATCH_FLAG = true;
                snprintf_err_handle(FW_STRING_FLAG, FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                snprintf_err_handle(CHILD_MODEL_STRING_FLAG, CHILD_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING) == 0)
            {
                CHILD_FW_MATCH_FLAG = true;
                snprintf_err_handle(CHILD_FW_STRING_FLAG, CHILD_FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, GET_TELEMETRY_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "current") == 0 || strcmp(optarg, "host") == 0)
                {
                    GET_TELEMETRY_IDENTIFIER = 1;
                }
                else if (strcmp(optarg, "saved") == 0 || strcmp(optarg, "ctrl") == 0)
                {
                    GET_TELEMETRY_IDENTIFIER = 2;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(GET_TELEMETRY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, PULL_LOG_MODE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "raw") == 0)
                {
                    PULL_LOG_MODE = PULL_LOG_RAW_MODE;
                }
                else if (strcmp(optarg, "bin") == 0)
                {
                    PULL_LOG_MODE = PULL_LOG_BIN_FILE_MODE;
                }
                else if (strcmp(optarg, "pipe") == 0)
                {
                    PULL_LOG_MODE  = PULL_LOG_PIPE_MODE;
                    NO_BANNER_FLAG = true;
                    if (!FARM_PULL_FLAG)
                    {
                        printf("Unsupported pipe feature for this log! \n");
                    }
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PULL_LOG_MODE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, SATA_FARM_COPY_TYPE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "disc") == 0)
                {
                    SATA_FARM_COPY_TYPE_FLAG = SATA_FARM_COPY_TYPE_DISC;
                }
                else if (strcmp(optarg, "flash") == 0)
                {
                    SATA_FARM_COPY_TYPE_FLAG = SATA_FARM_COPY_TYPE_FLASH;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(SATA_FARM_COPY_TYPE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            break;
        case DEVICE_SHORT_OPT: // device
            if (0 != parse_Device_Handle_Argument(optarg, &RUN_ON_ALL_DRIVES, &USER_PROVIDED_HANDLE, &DEVICE_LIST_COUNT,
                                                  &HANDLE_LIST))
            {
                // Free any memory allocated so far, then exit.
                free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\n");
                }
                exit(255);
            }
            break;
        case DEVICE_INFO_SHORT_OPT: // device information
            DEVICE_INFO_FLAG = true;
            break;
        case SCAN_SHORT_OPT: // scan
            SCAN_FLAG = true;
            break;
        case AGRESSIVE_SCAN_SHORT_OPT:
            AGRESSIVE_SCAN_FLAG = true;
            break;
        case VERSION_SHORT_OPT:
            SHOW_BANNER_FLAG = true;
            break;
        case VERBOSE_SHORT_OPT: // verbose
            if (!set_Verbosity_From_String(optarg, &toolVerbosity))
            {
                print_Error_In_Cmd_Line_Args_Short_Opt(VERBOSE_SHORT_OPT, optarg);
                exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
            }
            break;
        case QUIET_SHORT_OPT: // quiet mode
            toolVerbosity = VERBOSITY_QUIET;
            break;
        case SCAN_FLAGS_SHORT_OPT: // scan flags
            get_Scan_Flags(&SCAN_FLAGS, optarg);
            break;
        case '?': // unknown option
            printf("%s: Unable to parse %s command line option\nPlease use --%s for more information.\n", util_name,
                   argv[optind - 1], HELP_LONG_OPT_STRING);
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("\n");
            }
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
        case 'h': // help
            SHOW_HELP_FLAG = true;
            openseachest_utility_Info(util_name, buildVersion);
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
        int commandLineIter =
            1; // start at 1 as starting at 0 means printing the directory info+ SeaChest.exe (or ./SeaChest)
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
        openseachest_utility_Info(util_name, buildVersion);
    }

    if (SHOW_BANNER_FLAG)
    {
        utility_Full_Version_Info(util_name, buildVersion, OPENSEA_TRANSPORT_MAJOR_VERSION,
                                  OPENSEA_TRANSPORT_MINOR_VERSION, OPENSEA_TRANSPORT_PATCH_VERSION,
                                  OPENSEA_COMMON_VERSION, OPENSEA_OPERATION_VERSION);
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
#if defined(__linux__)
        if (SCAN_FLAGS.scanSD)
        {
            scanControl |= SD_HANDLES;
        }
        if (SCAN_FLAGS.scanSDandSG)
        {
            scanControl |= SG_TO_SD;
        }
#endif
        // set the drive types to show (if none are set, the lower level code assumes we need to show everything)
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
        // set the interface types to show (if none are set, the lower level code assumes we need to show everything)
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
#if defined(ENABLE_CSMI)
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

    // print out errors for unknown arguments for remaining args that haven't been processed yet
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
        uint64_t flags = UINT64_C(0);
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
            printf("You must specify one or more target devices with the --%s option to run this command.\n",
                   DEVICE_LONG_OPT_STRING);
            utility_Usage(true);
            printf("Use -h option for detailed description\n\n");
        }
        exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
    }

    if ((FORCE_SCSI_FLAG && FORCE_ATA_FLAG) || (FORCE_SCSI_FLAG && FORCE_NVME_FLAG) ||
        (FORCE_ATA_FLAG && FORCE_NVME_FLAG) || (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG) ||
        (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG) || (FORCE_ATA_PIO_FLAG && FORCE_ATA_UDMA_FLAG) ||
        (FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG) ||
        (FORCE_SCSI_FLAG &&
         (FORCE_ATA_PIO_FLAG || FORCE_ATA_DMA_FLAG ||
          FORCE_ATA_UDMA_FLAG)) // We may need to remove this. At least when software SAT gets used. (currently only
                                // Windows ATA passthrough and FreeBSD ATA passthrough)
    )
    {
        printf("\nError: Only one force flag can be used at a time.\n");
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    // need to make sure this is set when we are asked for SAT Info
    if (SAT_INFO_FLAG)
    {
        DEVICE_INFO_FLAG = goTrue;
    }
    // check that we were given at least one test to perform...if not, show the help and exit
    if (!(DEVICE_INFO_FLAG || TEST_UNIT_READY_FLAG || LOWLEVEL_INFO_FLAG || LIST_LOGS_FLAG || LIST_ERROR_HISTORY_FLAG ||
          GENERIC_LOG_PULL_FLAG || GENERIC_ERROR_HISTORY_PULL_FLAG || FARM_PULL_FLAG || FARM_COMBINED_FLAG || (GET_TELEMETRY_IDENTIFIER > 0) ||
          DST_LOG_FLAG || IDENTIFY_DEVICE_DATA_LOG_FLAG || SATA_PHY_COUNTERS_LOG_FLAG || DEVICE_STATS_LOG_FLAG ||
          INFORMATIONAL_EXCEPTIONS_FLAG))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    uint64_t flags = UINT64_C(0);
    DEVICE_LIST    = M_REINTERPRET_CAST(tDevice*, safe_calloc(DEVICE_LIST_COUNT, sizeof(tDevice)));
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
    safe_memset(&version, sizeof(versionBlock), 0, sizeof(versionBlock));
    version.version = DEVICE_BLOCK_VERSION;
    version.size    = sizeof(tDevice);

    if (TEST_UNIT_READY_FLAG)
    {
        flags = DO_NOT_WAKE_DRIVE;
    }

    if (FAST_DISCOVERY_FLAG)
    {
        flags = FAST_SCAN;
    }

    // set flags that can be passed down in get device regarding forcing specific ATA modes.
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

    eReturnValues getDevsRet = SUCCESS;
    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {

        for (uint32_t devi = UINT32_C(0); devi < DEVICE_LIST_COUNT; ++devi)
        {
            DEVICE_LIST[devi].deviceVerbosity = toolVerbosity;
        }
        getDevsRet = get_Device_List(DEVICE_LIST, DEVICE_LIST_COUNT * sizeof(tDevice), version, flags);
        ret        = getDevsRet;
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
                    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                    exit(UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
                }
                else
                {
                    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                    exit(UTIL_EXIT_OPERATION_FAILURE);
                }
            }
        }
    }
    else
    {
        /*need to go through the handle list and attempt to open each handle.*/
        for (uint32_t handleIter = UINT32_C(0); handleIter < DEVICE_LIST_COUNT; ++handleIter)
        {
            /*Initializing is necessary*/
            deviceList[handleIter].sanity.size    = sizeof(tDevice);
            deviceList[handleIter].sanity.version = DEVICE_BLOCK_VERSION;
#if defined(UEFI_C_SOURCE)
            deviceList[handleIter].os_info.fd = M_NULLPTR;
#elif !defined(_WIN32)
            deviceList[handleIter].os_info.fd     = -1;
#    if defined(VMK_CROSS_COMP)
            deviceList[handleIter].os_info.nvmeFd = M_NULLPTR;
#    endif
#else
            deviceList[handleIter].os_info.fd = INVALID_HANDLE_VALUE;
#endif
            deviceList[handleIter].dFlags = flags;

            deviceList[handleIter].deviceVerbosity = toolVerbosity;

            if (ENABLE_LEGACY_PASSTHROUGH_FLAG)
            {
                deviceList[handleIter].drive_info.ata_Options.enableLegacyPassthroughDetectionThroughTrialAndError =
                    true;
            }

            /*get the device for the specified handle*/
#if defined(_DEBUG)
            printf("Attempting to open handle \"%s\"\n", HANDLE_LIST[handleIter]);
#endif

            ret = get_Device(HANDLE_LIST[handleIter], &deviceList[handleIter]);
#if !defined(_WIN32)
#    if !defined(VMK_CROSS_COMP)
            if ((deviceList[handleIter].os_info.fd < 0) ||
#    else
            if (((deviceList[handleIter].os_info.fd < 0) && (deviceList[handleIter].os_info.nvmeFd == M_NULLPTR)) ||
#    endif
                (ret != SUCCESS))
#else
            if ((deviceList[handleIter].os_info.fd == INVALID_HANDLE_VALUE) ||
                (ret != SUCCESS))
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
                else if (ret == DEVICE_BUSY)
                {
                    exit(UTIL_EXIT_DEVICE_BUSY);
                }
                else if (ret == DEVICE_INVALID)
                {
                    exit(UTIL_EXIT_NO_DEVICE);
                }
                else
                {
                    exit(UTIL_EXIT_OPERATION_FAILURE);
                }
            }
        }
    }
    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
    uint32_t skippedDevices = UINT32_C(0);
    for (uint32_t deviceIter = UINT32_C(0); deviceIter < DEVICE_LIST_COUNT; ++deviceIter)
    {
        deviceList[deviceIter].deviceVerbosity = toolVerbosity;
        if (ONLY_SEAGATE_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                /*if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) is not a Seagate drive.\n", deviceList[deviceIter].os_info.name,
                deviceList[deviceIter].drive_info.product_identification);
                }*/
                ++skippedDevices;
                continue;
            }
        }

        // check for model number match
        if (MODEL_MATCH_FLAG)
        {
            if (strstr(deviceList[deviceIter].drive_info.product_identification, MODEL_STRING_FLAG) == M_NULLPTR)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) does not match the input model number: %s\n",
                           deviceList[deviceIter].os_info.name,
                           deviceList[deviceIter].drive_info.product_identification, MODEL_STRING_FLAG);
                }
                ++skippedDevices;
                continue;
            }
        }
        // check for fw match
        if (FW_MATCH_FLAG)
        {
            if (strcmp(FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision) != 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input firmware revision: %s\n",
                           deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_revision,
                           FW_STRING_FLAG);
                }
                ++skippedDevices;
                continue;
            }
        }
        // check for child model number match
        if (CHILD_MODEL_MATCH_FLAG)
        {
            if (safe_strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN) == 0 ||
                strstr(deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG) ==
                    M_NULLPTR)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive (%s) does not match the input child model number: %s\n",
                           deviceList[deviceIter].os_info.name,
                           deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG);
                }
                ++skippedDevices;
                continue;
            }
        }
        // check for child fw match
        if (CHILD_FW_MATCH_FLAG)
        {
            if (strcmp(CHILD_FW_STRING_FLAG, deviceList[deviceIter].drive_info.bridge_info.childDriveFW) != 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input child firmware revision: %s\n",
                           deviceList[deviceIter].os_info.name,
                           deviceList[deviceIter].drive_info.bridge_info.childDriveFW, CHILD_FW_STRING_FLAG);
                }
                ++skippedDevices;
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
            deviceList[deviceIter].drive_info.ata_Options.dmaSupported                  = false;
            deviceList[deviceIter].drive_info.ata_Options.dmaMode                       = ATA_DMA_MODE_NO_DMA;
            deviceList[deviceIter].drive_info.ata_Options.downloadMicrocodeDMASupported = false;
            deviceList[deviceIter].drive_info.ata_Options.readBufferDMASupported        = false;
            deviceList[deviceIter].drive_info.ata_Options.readLogWriteLogDMASupported   = false;
            deviceList[deviceIter].drive_info.ata_Options.writeBufferDMASupported       = false;
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

        if (deviceList[deviceIter].drive_info.interface_type == UNKNOWN_INTERFACE)
        {
            ++skippedDevices;
            continue;
        }

        if (VERBOSITY_QUIET < toolVerbosity)
        {
            if (PULL_LOG_MODE != PULL_LOG_PIPE_MODE)
            {
                printf("\n%s - %s - %s - %s - %s\n", deviceList[deviceIter].os_info.name,
                       deviceList[deviceIter].drive_info.product_identification,
                       deviceList[deviceIter].drive_info.serialNumber,
                       deviceList[deviceIter].drive_info.product_revision, print_drive_type(&deviceList[deviceIter]));
            }
        }

        // now start looking at what operations are going to be performed and kick them off
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

        if (LIST_LOGS_FLAG)
        {
            switch (print_Supported_Logs(&deviceList[deviceIter], 0))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nListing supported logs is not supported for device %s.\n",
                           deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to list logs for device %s\n", deviceList[deviceIter].drive_info.serialNumber);
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
                    printf("\nListing supported error history buffer IDs is not supported for device %s.\n",
                           deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to list error history buffer IDs for device %s\n",
                           deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (GENERIC_LOG_PULL_FLAG)
        {
            switch (pull_Generic_Log(&deviceList[deviceIter], GENERIC_LOG_DATA_SET, GENERIC_LOG_SUBPAGE_DATA_SET,
                                     PULL_LOG_MODE, OUTPUTPATH_FLAG, LOG_TRANSFER_LENGTH_BYTES, LOG_LENGTH_BYTES))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && GENERIC_LOG_SUBPAGE_DATA_SET != 0)
                    {
                        printf("\nSuccessfully pulled Log %" PRIu8 ", subpage %" PRIu8 " from %s\n",
                               GENERIC_LOG_DATA_SET, GENERIC_LOG_SUBPAGE_DATA_SET,
                               deviceList[deviceIter].drive_info.serialNumber);
                    }
                    else
                    {
                        printf("\nSuccessfully pulled Log %" PRIu8 " from %s\n", GENERIC_LOG_DATA_SET,
                               deviceList[deviceIter].drive_info.serialNumber);
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && GENERIC_LOG_SUBPAGE_DATA_SET != 0)
                    {
                        printf("\nLog %" PRIu8 ", subpage %" PRIu8 " not supported by %s\n", GENERIC_LOG_DATA_SET,
                               GENERIC_LOG_SUBPAGE_DATA_SET, deviceList[deviceIter].drive_info.serialNumber);
                    }
                    else
                    {
                        printf("\nLog %" PRIu8 " not supported by %s\n", GENERIC_LOG_DATA_SET,
                               deviceList[deviceIter].drive_info.serialNumber);
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case MEMORY_FAILURE:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (deviceList[deviceIter].drive_info.drive_type == SCSI_DRIVE && GENERIC_LOG_SUBPAGE_DATA_SET != 0)
                    {
                        printf("\nFailed to allocate memory for log %" PRIu8 ", subpage %" PRIu8 "\n",
                               GENERIC_LOG_DATA_SET, GENERIC_LOG_SUBPAGE_DATA_SET);
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
                        printf("\nFailed to pull log %" PRIu8 ", subpage %" PRIu8 " from %s\n", GENERIC_LOG_DATA_SET,
                               GENERIC_LOG_SUBPAGE_DATA_SET, deviceList[deviceIter].drive_info.serialNumber);
                    }
                    else
                    {
                        printf("\nFailed to pull log %" PRIu8 " from %s\n", GENERIC_LOG_DATA_SET,
                               deviceList[deviceIter].drive_info.serialNumber);
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (GENERIC_ERROR_HISTORY_PULL_FLAG)
        {
            switch (pull_Generic_Error_History(&deviceList[deviceIter],
                                               C_CAST(uint8_t, GENERIC_ERROR_HISTORY_BUFFER_ID), PULL_LOG_MODE,
                                               OUTPUTPATH_FLAG, LOG_TRANSFER_LENGTH_BYTES))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nSuccessfully pulled buffer ID %" PRIu64 " from %s\n", GENERIC_ERROR_HISTORY_BUFFER_ID,
                           deviceList[deviceIter].drive_info.serialNumber);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nBuffer ID %" PRIu64 " not supported by %s\n", GENERIC_ERROR_HISTORY_BUFFER_ID,
                           deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            case MEMORY_FAILURE:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to allocate memory for buffer ID %" PRIu64 "\n", GENERIC_ERROR_HISTORY_BUFFER_ID);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("\nFailed to pull buffer ID %" PRIu64 " from %s\n", GENERIC_ERROR_HISTORY_BUFFER_ID,
                           deviceList[deviceIter].drive_info.serialNumber);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (GET_TELEMETRY_IDENTIFIER > 0)
        {
            if (TELEMETRY_DATA_AREA > TELEMETRY_LOG_MIN_DATA_SET && TELEMETRY_DATA_AREA <= TELEMETRY_LOG_MAX_DATA_SET)
            {
                switch (pull_Telemetry_Log(&deviceList[deviceIter], GET_TELEMETRY_IDENTIFIER == 1 ? true : false,
                                           TELEMETRY_DATA_AREA, true, M_NULLPTR, 0, OUTPUTPATH_FLAG,
                                           LOG_TRANSFER_LENGTH_BYTES))
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Telemetry log pulled successfully from device!\n");
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Telemetry log not supported by this device!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to pull telemetry log from this device!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
            }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf(
                        "You must specify a Telemetry data set.\n\t1 - 3 are valid inputs for the data set on SATA\n");
                    printf("\tOn SAS and NVMe, 1 - 4 are valid inputs for the data set.\n");
                }
            }
        }
        
        if (FARM_PULL_FLAG)
        {
            // PULL_FARM_FACTORY_PAGE_FLAG
            switch (pull_FARM_Log(&deviceList[deviceIter], OUTPUTPATH_FLAG, LOG_TRANSFER_LENGTH_BYTES, 0,
                                  SEAGATE_ATA_LOG_FIELD_ACCESSIBLE_RELIABILITY_METRICS, PULL_LOG_BIN_FILE_MODE,
                                  NAMING_SERIAL_NUMBER_DATE_TIME))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    if (PULL_LOG_MODE != PULL_LOG_PIPE_MODE)
                    {
                        printf("Successfully pulled FARM log\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    // if (0)
                    // {
                    //     printf("Factory FARM Page not supported on this device\n");
                    // }
                    // else
                    {
                        printf("FARM log not supported on this device\n");
                    }
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

        if (FARM_COMBINED_FLAG)
        {
            // PULL FARM Log containing all FARM sub Log pages
            switch (pull_FARM_Combined_Log(&deviceList[deviceIter], OUTPUTPATH_FLAG, LOG_TRANSFER_LENGTH_BYTES,
                                           SATA_FARM_COPY_TYPE_FLAG, NAMING_SERIAL_NUMBER_DATE_TIME))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Successfully pulled FARM Combined log\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("FARM Combined log not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to pull FARM Combined log\n");
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
        // At this point, close the device handle since it is no longer needed. Do not put any further IO below this.
        close_Device(&deviceList[deviceIter]);
    }
    free_device_list(&DEVICE_LIST);
    if (getDevsRet != SUCCESS && skippedDevices == DEVICE_LIST_COUNT)
    {
        switch (getDevsRet)
        {
        case WARN_NOT_ALL_DEVICES_ENUMERATED:
            // Different exit code needed? Not entirely sure if this is the best choice here - TJE
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            break;
        case PERMISSION_DENIED:
            exitCode = UTIL_EXIT_NEED_ELEVATED_PRIVILEGES;
            break;
        default:
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            break;
        }
    }
    else if (skippedDevices == DEVICE_LIST_COUNT)
    {
        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
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
    // everything needs a help option right?
    printf("Usage\n");
    printf("=====\n");
    printf("\t %s [-d %s] {arguments} {options}\n\n", util_name, deviceHandleName);

    printf("\nExamples\n");
    printf("========\n");
    // example usage
    printf("\t%s --%s\n", util_name, SCAN_LONG_OPT_STRING);
    printf("\t%s -d %s -%c\n", util_name, deviceHandleExample, DEVICE_INFO_SHORT_OPT);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SAT_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LOWLEVEL_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s logs --%s pipe\n", util_name, deviceHandleExample, FARM_LONG_OPT_STRING,
           PATH_LONG_OPT_STRING, PULL_LOG_MODE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s --%s 64KiB --%s bin\n", util_name, deviceHandleExample, FARM_COMBINED_LONG_OPT_STRING,
           LOG_TRANSFER_LENGTH_LONG_OPT_STRING, PULL_LOG_MODE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, DEVICE_STATISTICS_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LIST_LOGS_LONG_OPT_STRING);
    printf("\t%s -d %s --%s C6h\n", util_name, deviceHandleExample, GENERIC_LOG_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 0Dh --%s 01h --%s bin\n", util_name, deviceHandleExample, GENERIC_LOG_LONG_OPT_STRING,
           GENERIC_LOG_SUBPAGE_LONG_OPT_STRING, PULL_LOG_MODE_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LIST_ERROR_HISTORY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s 30 --%s bin\n", util_name, deviceHandleExample, GENERIC_ERROR_HISTORY_LONG_OPT_STRING,
           PULL_LOG_MODE_LONG_OPT_STRING);

    printf("\t%s -d %s --%s host\n", util_name, deviceHandleExample, GET_TELEMETRY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s ctrl\n", util_name, deviceHandleExample, GET_TELEMETRY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current\n", util_name, deviceHandleExample, GET_TELEMETRY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s saved\n", util_name, deviceHandleExample, GET_TELEMETRY_LONG_OPT_STRING);
    printf("\t%s -d %s --%s host --%s 2\n", util_name, deviceHandleExample, GET_TELEMETRY_LONG_OPT_STRING,
           TELEMETRY_DATA_AREA_LONG_OPT_STRING);
    printf("\t%s -d %s --%s current --%s 4\n", util_name, deviceHandleExample, GET_TELEMETRY_LONG_OPT_STRING,
           TELEMETRY_DATA_AREA_LONG_OPT_STRING);
    // return codes
    printf("\nReturn codes\n");
    printf("============\n");
    print_SeaChest_Util_Exit_Codes(0, M_NULLPTR, util_name);

    // utility options - alphabetized
    printf("\nUtility Options\n");
    printf("===============\n");
#if defined(ENABLE_CSMI)
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

    // the test options
    printf("\nUtility Arguments\n");
    printf("=================\n");
    // Common (across utilities) - alphabetized
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Device_Information_Help(shortUsage);
    print_Low_Level_Info_Help(shortUsage);
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    // utility tests/operations go here - alphabetized
    // multiple interfaces
    print_Fast_Discovery_Help(shortUsage);
    printf("\n");
    print_Pull_Device_Statistics_Log_Help(shortUsage);
    print_FARM_Log_Help(shortUsage);
    print_FARM_Combined_Log_Help(shortUsage);
	print_Get_Telemetry_Help(shortUsage);
    print_Supported_Logs_Help(shortUsage);
    print_Log_Length_Help(shortUsage);
    print_Log_Mode_Help(shortUsage);
    print_Log_Transfer_Length_Help(shortUsage);
    print_Pull_Generic_Logs_Help(shortUsage);
    print_Pull_Self_Test_Results_Log_Help(shortUsage);
	print_Telemetry_Data_Set_Help(shortUsage);

    // SATA Only Options
    printf("\n\tSATA Only:\n\n");
    print_Pull_Identify_Device_Data_Log_Help(shortUsage);
    print_Sata_FARM_Copy_Type_Flag_Help(shortUsage);
    print_Pull_SATA_Phy_Event_Counters_Log_Help(shortUsage);
    // SAS Only Options
    printf("\n\tSAS Only:\n\n");
    print_Supported_Error_History_Help(shortUsage);
    print_Pull_Generic_Error_History_Help(shortUsage);
    print_Pull_Informational_Exceptions_Log_Help(shortUsage);
    print_Pull_Generic_Logs_Subpage_Help(shortUsage);
}
