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
// \file openSeaChest_Firmware.c command line that performs various firmware/microcode download on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "io_utils.h"
#include "math_utils.h"
#include "memory_safety.h"
#include "precision_timer.h"
#include "secure_file.h"
#include "sleep.h"
#include "string_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"
#if defined(_WIN32)
#    include "windows_version_detect.h" //for WinAPI checks and functions
#endif

#include <stdbool.h>
#include <time.h>
#if defined(__unix__) || defined(__APPLE__) // using this definition because linux and unix compilers both define this.
                                            // Apple does not define this, which is why it has it's own definition
#    include <unistd.h>
#endif
#include "EULA.h"
#include "drive_info.h"
#include "firmware_download.h"
#include "getopt.h"
#include "openseachest_util_options.h"
#include "operations.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name    = "openSeaChest_Firmware";
const char* buildVersion = "4.2.0";

typedef enum eSeaChestFirmwareExitCodesEnum
{
    SEACHEST_FIRMWARE_EXIT_FIRMWARE_DOWNLOAD_COMPLETE =
        UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE,         // Segmented, full, or deferred + activate completed
    SEACHEST_FIRMWARE_EXIT_DEFERRED_DOWNLOAD_COMPLETED, // deferred update complete. need to power cycle the drive
    SEACHEST_FIRMWARE_EXIT_DEFERRED_CODE_ACTIVATED,     // activate command sent to the drive
    SEACHEST_FIRMWARE_EXIT_NO_MATCH_FOUND,              // config file only right now
    SEACHEST_FIRMWARE_EXIT_MN_MATCH_FW_MISMATCH,        // config file only right now
    SEACHEST_FIRMWARE_EXIT_FIRMWARE_HASH_DOESNT_MATCH,
    SEACHEST_FIRMWARE_EXIT_ALREADY_UP_TO_DATE,
    SEACHEST_FIRMWARE_EXIT_MATCH_FOUND,                    // used in dry run mode only
    SEACHEST_FIRMWARE_EXIT_MATCH_FOUND_DEFERRED_SUPPORTED, // used in dry run mode only
    // TODO: add more exit codes here! Don't forget to add the string definition below in the help!
    SEACHEST_FIRMWARE_EXIT_MAX_ERROR // don't acutally use this, just for memory allocation below
} eSeaChestFirmwareExitCodes;

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
    eReturnValues ret      = SUCCESS;
    int           exitCode = UTIL_EXIT_NO_ERROR;
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
    FORCE_DRIVE_TYPE_VARS
    ENABLE_LEGACY_PASSTHROUGH_VAR
    // scan output flags
    SCAN_FLAGS_UTIL_VARS
    // tool specific
    DOWNLOAD_FW_VARS
#if defined(_WIN32)
    WIN10_FLEXIBLE_API_USE_VAR
    WIN10_FWDL_FORCE_PT_VAR
#endif
    FIRMWARE_SLOT_VAR
    MODEL_MATCH_VARS
    FW_MATCH_VARS
    NEW_FW_MATCH_VARS
    CHILD_MODEL_MATCH_VARS
    CHILD_FW_MATCH_VARS
    CHILD_NEW_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    FWDL_SEGMENT_SIZE_VARS
    SHOW_FWDL_SUPPORT_VAR
    ACTIVATE_DEFERRED_FW_VAR
    SWITCH_FW_VAR
    FWDL_IGNORE_FINAL_SEGMENT_STATUS_VAR
    FORCE_NVME_COMMIT_ACTION_VAR
    FORCE_DISABLE_NVME_FW_COMMIT_RESET_VAR

#if defined(ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    LOWLEVEL_INFO_VAR
    SHOW_SCSI_FW_INFO_VAR

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
        USB_CHILD_INFO_LONG_OPT,
        SCAN_LONG_OPT,
        AGRESSIVE_SCAN_LONG_OPT,
        SCAN_FLAGS_LONG_OPT,
        NO_BANNER_OPT,
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
        CHILD_NEW_FW_MATCH_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
#if defined(ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        LOWLEVEL_INFO_LONG_OPT,
        // tool specific options go here
        DOWNLOAD_FW_LONG_OPT,
        DOWNLOAD_FW_MODE_LONG_OPT,
        NEW_FW_MATCH_LONG_OPT,
        FWDL_SEGMENT_SIZE_LONG_OPT,
        SHOW_FWDL_SUPPORT_LONG_OPT,
        ACTIVATE_DEFERRED_FW_LONG_OPT,
        SWITCH_FW_LONG_OPT,
        FIRMWARE_SLOT_BUFFER_ID_LONG_OPT,
#if defined(_WIN32)
        WIN10_FLEXIBLE_API_USE_LONG_OPT,
        WIN10_FWDL_FORCE_PT_LONG_OPT,
#endif
        FWDL_IGNORE_FINAL_SEGMENT_STATUS_LONG_OPT,
        FORCE_NVME_COMMIT_ACTION_LONG_OPT,
        FORCE_DISABLE_NVME_FW_COMMIT_RESET_LONG_OPT,
        LONG_OPT_TERMINATOR
    };

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
        openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
        utility_Usage(true);
        printf("\n");
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    // get options we know we need
    while (1) // changed to while 1 in order to parse multiple options when longs options are involved
    {
        args = getopt_long(argc, argv, "d:hisSF:Vv:q%:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        // printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:
            // parse long options that have no short option and required arguments here
            if (strcmp(longopts[optionIndex].name, DOWNLOAD_FW_LONG_OPT_STRING) == 0)
            {
                int res = snprintf_err_handle(DOWNLOAD_FW_FILENAME_FLAG, FIRMWARE_FILE_NAME_MAX_LEN, "%s", optarg);
                if (res > 0 && res <= FIRMWARE_FILE_NAME_MAX_LEN)
                {
                    DOWNLOAD_FW_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(DOWNLOAD_FW_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, DOWNLOAD_FW_MODE_LONG_OPT_STRING) == 0)
            {
                DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_AUTOMATIC;
                if (strcmp(optarg, "immediate") == 0 || strcmp(optarg, "full") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_FULL;
                }
                else if (strcmp(optarg, "segmented") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_SEGMENTED;
                }
                else if (strcmp(optarg, "deferred") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_DEFERRED;
                }
                // TODO: deferredselect and a way to get events: POA, HRA, and VSA
                else if (strcmp(optarg, "deferred+activate") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_DEFERRED_PLUS_ACTIVATE;
                }
                else if (strcmp(optarg, "auto") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_AUTOMATIC;
                }
                else if (strcmp(optarg, "temp") == 0)
                {
                    DOWNLOAD_FW_MODE = FWDL_UPDATE_MODE_TEMP;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(DOWNLOAD_FW_MODE_LONG_OPT_STRING, optarg);
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
            else if (strcmp(longopts[optionIndex].name, NEW_FW_MATCH_LONG_OPT_STRING) == 0)
            {
                NEW_FW_MATCH_FLAG = true;
                snprintf_err_handle(NEW_FW_STRING_FLAG, NEW_FW_MATCH_STRING_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, CHILD_NEW_FW_MATCH_LONG_OPT_STRING) == 0)
            {
                CHILD_NEW_FW_MATCH_FLAG = true;
                snprintf_err_handle(CHILD_NEW_FW_STRING_FLAG, CHILD_NEW_FW_STRING_MATCH_LENGTH, "%s", optarg);
            }
            else if (strcmp(longopts[optionIndex].name, FWDL_SEGMENT_SIZE_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint16(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &FWDL_SEGMENT_SIZE_FLAG))
                {
                    FWDL_SEGMENT_SIZE_FROM_USER = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(FWDL_SEGMENT_SIZE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, FIRMWARE_SLOT_LONG_OPT_STRING) == 0 ||
                     strcmp(longopts[optionIndex].name, FIRMWARE_BUFFER_ID_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE, &FIRMWARE_SLOT_FLAG) ||
                    FIRMWARE_SLOT_FLAG > 7)
                {
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("FirmwareSlot/FwBuffer ID must be between 0 and 7\n");
                    }
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, FORCE_NVME_COMMIT_ACTION_LONG_OPT_STRING) == 0)
            {
                if (!get_And_Validate_Integer_Input_Uint8(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &FORCE_NVME_COMMIT_ACTION))
                {
                    print_Error_In_Cmd_Line_Args(FORCE_NVME_COMMIT_ACTION_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            break;
        case ':': // missing required argument
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            switch (optopt)
            {
            case 0:
                // check long options for missing arguments
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
        openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
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
    if (!(DEVICE_INFO_FLAG || TEST_UNIT_READY_FLAG ||
          LOWLEVEL_INFO_FLAG
          // check for other tool specific options here
          || DOWNLOAD_FW_FLAG || SHOW_FWDL_SUPPORT_INFO_FLAG || ACTIVATE_DEFERRED_FW_FLAG || SWITCH_FW_FLAG))
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

    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {

        for (uint32_t devi = UINT32_C(0); devi < DEVICE_LIST_COUNT; ++devi)
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
                (ret == FAILURE || ret == PERMISSION_DENIED))
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
                continue;
            }
        }

        // check for fw already loaded
        if (NEW_FW_MATCH_FLAG)
        {
            if (strcmp(NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision) == 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This drive already has firmware revision: %s\n", deviceList[deviceIter].os_info.name,
                           deviceList[deviceIter].drive_info.product_revision);
                }
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
                continue;
            }
        }
        // check for child fw already loaded
        if (CHILD_NEW_FW_MATCH_FLAG)
        {
            if (strcmp(CHILD_NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.bridge_info.childDriveFW) == 0)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("%s - This child drive already has firmware revision: %s\n",
                           deviceList[deviceIter].os_info.name,
                           deviceList[deviceIter].drive_info.bridge_info.childDriveFW);
                }
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

        if (VERBOSITY_QUIET < toolVerbosity)
        {
            printf("\n%s - %s - %s - %s - %s\n", deviceList[deviceIter].os_info.name,
                   deviceList[deviceIter].drive_info.product_identification,
                   deviceList[deviceIter].drive_info.serialNumber, deviceList[deviceIter].drive_info.product_revision,
                   print_drive_type(&deviceList[deviceIter]));
        }

#if defined(_WIN32) && WINVER >= SEA_WIN32_WINNT_WIN10
        if (WIN10_FLEXIBLE_API_USE_FLAG)
        {
            deviceList[deviceIter].os_info.fwdlIOsupport.allowFlexibleUseOfAPI = true;
        }

        if (WIN10_FWDL_FORCE_PT_FLAG)
        {
            deviceList[deviceIter].os_info.fwdlIOsupport.fwdlIOSupported =
                false; // turn off the Win10 API support to force passthrough mode.
        }

#endif

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

        if (SHOW_FWDL_SUPPORT_INFO_FLAG)
        {
            supportedDLModes supportedFWDLModes;
            safe_memset(&supportedFWDLModes, sizeof(supportedDLModes), 0, sizeof(supportedDLModes));
            supportedFWDLModes.size    = sizeof(supportedDLModes);
            supportedFWDLModes.version = SUPPORTED_FWDL_MODES_VERSION;
            switch (get_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes))
            {
            case SUCCESS:
                show_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes);
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Getting supported download modes not supported on this drive\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get supported download modes from the drive\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHOW_SCSI_FW_INFO_FLAG)
        {
            // these live under at least seadragon for now...-TJE
            seagateSCSIFWNumbers fwNumbers;
            safe_memset(&fwNumbers, sizeof(seagateSCSIFWNumbers), 0, sizeof(seagateSCSIFWNumbers));
            switch (get_Seagate_SCSI_Firmware_Numbers(&deviceList[deviceIter], &fwNumbers))
            {
            case SUCCESS:
                printf("\n===SCSI Firmware Numbers===\n");
                printf(" SCSI Firmware Release Number: %s\n", fwNumbers.scsiFirmwareReleaseNumber);
                printf(" Servo Firmware Release Number: %s\n", fwNumbers.servoFirmwareReleaseNumber);
                printf(" SAP Block Point Numbers: %s\n", fwNumbers.sapBlockPointNumbers);
                printf(" Servo Firmware Release Date: %s\n", fwNumbers.servoFirmmwareReleaseDate);
                printf(" Servo ROM Release Date: %s\n", fwNumbers.servoRomReleaseDate);
                printf(" SAP Firmware Release Number: %s\n", fwNumbers.sapFirmwareReleaseNumber);
                printf(" SAP Firmware Release Date: %s\n", fwNumbers.sapFirmwareReleaseDate);
                printf(" SAP Firmware Release Year: %s\n", fwNumbers.sapFirmwareReleaseYear);
                printf(" SAP Manufacturing Key: %s\n", fwNumbers.sapManufacturingKey);
                printf(" Servo Firmware Product Family and Product Family Member IDs: %s\n",
                       fwNumbers.servoFirmwareProductFamilyAndProductFamilyMemberIDs);
                break;
            case NOT_SUPPORTED:
                printf("SCSI Firmware Numbers not supported by this device\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                printf("Failed to retrieve SCSI Firmware Numbers from this device\n");
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

#if defined(_WIN32)
        if (deviceList[deviceIter].drive_info.drive_type == NVME_DRIVE &&
            (FORCE_NVME_COMMIT_ACTION != 0xFF || FORCE_DISABLE_NVME_FW_COMMIT_RESET))
        {
            // Check for open fabrics as this is possible there as well as USB adapters. Some will be able to support
            // this
            if (!(deviceList[deviceIter].os_info.openFabricsNVMePassthroughSupported ||
                  deviceList[deviceIter].drive_info.passThroughHacks.passthroughType == NVME_PASSTHROUGH_JMICRON ||
                  deviceList[deviceIter].drive_info.passThroughHacks.passthroughType == NVME_PASSTHROUGH_ASMEDIA))
            {
                printf("\nERROR: Forcing specific commit actions or disabling resets is not possible in Windows on "
                       "this device.\n");
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                continue;
            }
        }
#endif

        if (DOWNLOAD_FW_FLAG)
        {
            secureFileInfo* fwfile = secure_Open_File(DOWNLOAD_FW_FILENAME_FLAG, "rb", M_NULLPTR, M_NULLPTR, M_NULLPTR);
            if (fwfile && fwfile->error == SEC_FILE_SUCCESS)
            {
                uint8_t* firmwareMem =
                    M_REINTERPRET_CAST(uint8_t*, safe_calloc_aligned(fwfile->fileSize, sizeof(uint8_t),
                                                                     deviceList[deviceIter].os_info.minimumAlignment));
                if (firmwareMem)
                {
                    if (SEC_FILE_SUCCESS == secure_Read_File(fwfile, firmwareMem, fwfile->fileSize, sizeof(uint8_t),
                                                             fwfile->fileSize, M_NULLPTR))
                    {
                        firmwareUpdateData dlOptions;
                        DECLARE_SEATIMER(commandTimer);
                        safe_memset(&dlOptions, sizeof(firmwareUpdateData), 0, sizeof(firmwareUpdateData));
                        dlOptions.size    = sizeof(firmwareUpdateData);
                        dlOptions.version = FIRMWARE_UPDATE_DATA_VERSION;
                        dlOptions.dlMode  = C_CAST(eFirmwareUpdateMode, DOWNLOAD_FW_MODE);
                        if (FWDL_SEGMENT_SIZE_FROM_USER)
                        {
                            dlOptions.segmentSize = FWDL_SEGMENT_SIZE_FLAG;
                        }
                        else
                        {
                            dlOptions.segmentSize = 0;
                        }
                        dlOptions.ignoreStatusOfFinalSegment = M_ToBool(FWDL_IGNORE_FINAL_SEGMENT_STATUS_FLAG);
                        dlOptions.firmwareFileMem            = firmwareMem;
                        dlOptions.firmwareMemoryLength       = C_CAST(
                                  uint32_t,
                                  fwfile->fileSize); // firmware files shouldn't be larger than a few MBs for a LONG time
                        dlOptions.firmwareSlot = FIRMWARE_SLOT_FLAG;
                        if (FORCE_NVME_COMMIT_ACTION != 0xFF)
                        {
                            // forcing a specific commit action
                            dlOptions.forceCommitAction      = FORCE_NVME_COMMIT_ACTION;
                            dlOptions.forceCommitActionValid = true;
                        }
                        if (FORCE_DISABLE_NVME_FW_COMMIT_RESET)
                        {
                            // disabling the reset after an NVMe commit
                            dlOptions.disableResetAfterCommit = true;
                        }
                        start_Timer(&commandTimer);
                        ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                        stop_Timer(&commandTimer);
                        switch (ret)
                        {
                        case SUCCESS:
                        case POWER_CYCLE_REQUIRED:
                            exitCode = C_CAST(eUtilExitCodes, SEACHEST_FIRMWARE_EXIT_FIRMWARE_DOWNLOAD_COMPLETE);
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Firmware Download successful\n");
                                printf("Firmware Download time");
                                print_Time(get_Nano_Seconds(commandTimer));
                                printf("Average time/segment ");
                                print_Time(dlOptions.avgSegmentDlTime);
                                if (ret != POWER_CYCLE_REQUIRED && dlOptions.activateFWTime > 0)
                                {
                                    printf("Activate Time         ");
                                    print_Time(dlOptions.activateFWTime);
                                }
                            }
                            if (ret == POWER_CYCLE_REQUIRED)
                            {
                                printf("The Operating system has reported that a power cycle is required to complete "
                                       "the firmware update\n");
                            }
                            if (DOWNLOAD_FW_MODE == FWDL_UPDATE_MODE_DEFERRED)
                            {
                                exitCode = C_CAST(eUtilExitCodes, SEACHEST_FIRMWARE_EXIT_DEFERRED_DOWNLOAD_COMPLETED);
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    printf("Firmware download complete. Reboot or run the --%s command to finish "
                                           "installing the firmware.\n",
                                           ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
                                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                    {
                                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                                    }
                                }
                            }
                            else
                            {
                                fill_Drive_Info_Data(&deviceList[deviceIter]);
                                if (VERBOSITY_QUIET < toolVerbosity)
                                {
                                    if (NEW_FW_MATCH_FLAG)
                                    {
                                        if (strcmp(NEW_FW_STRING_FLAG,
                                                   deviceList[deviceIter].drive_info.product_revision) == 0)
                                        {
                                            printf("Successfully validated firmware after download!\n");
                                            printf("New firmware version is %s\n",
                                                   deviceList[deviceIter].drive_info.product_revision);
                                        }
                                        else
                                        {
                                            printf("Unable to verify firmware after download!, expected %s, but found "
                                                   "%s\n",
                                                   NEW_FW_STRING_FLAG,
                                                   deviceList[deviceIter].drive_info.product_revision);
                                        }
                                    }
                                    else
                                    {
                                        printf("New firmware version is %s\n",
                                               deviceList[deviceIter].drive_info.product_revision);
                                    }
                                    if (deviceList[deviceIter].drive_info.numberOfLUs > 1)
                                    {
                                        printf("NOTE: This command may have affected more than 1 logical unit\n");
                                    }
                                }
                            }
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Firmware Download not supported\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Firmware Download failed\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Error reading contents of firmware file!\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    safe_free_aligned(&firmwareMem);
                }
                else
                {
                    perror("failed to allocate memory");
                    if (fwfile)
                    {
                        if (SEC_FILE_SUCCESS != secure_Close_File(fwfile))
                        {
                            printf("Error attempting to close file!\n");
                        }
                        free_Secure_File_Info(&fwfile);
                    }
                    exit(255);
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Couldn't open file %s\n", DOWNLOAD_FW_FILENAME_FLAG);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
            if (fwfile)
            {
                if (SEC_FILE_SUCCESS != secure_Close_File(fwfile))
                {
                    printf("Error attempting to close file!\n");
                }
                free_Secure_File_Info(&fwfile);
            }
        }

        if (ACTIVATE_DEFERRED_FW_FLAG || SWITCH_FW_FLAG)
        {
            supportedDLModes supportedFWDLModes;
            safe_memset(&supportedFWDLModes, sizeof(supportedDLModes), 0, sizeof(supportedDLModes));
            supportedFWDLModes.size    = sizeof(supportedDLModes);
            supportedFWDLModes.version = SUPPORTED_FWDL_MODES_VERSION;
            get_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes);
            if (supportedFWDLModes.deferred || supportedFWDLModes.scsiInfoPossiblyIncomplete)
            {
                firmwareUpdateData dlOptions;
                DECLARE_SEATIMER(commandTimer);
                safe_memset(&dlOptions, sizeof(firmwareUpdateData), 0, sizeof(firmwareUpdateData));
                dlOptions.size                 = sizeof(firmwareUpdateData);
                dlOptions.version              = FIRMWARE_UPDATE_DATA_VERSION;
                dlOptions.dlMode               = FWDL_UPDATE_MODE_ACTIVATE;
                dlOptions.segmentSize          = 0;
                dlOptions.firmwareFileMem      = M_NULLPTR;
                dlOptions.firmwareMemoryLength = 0;
                dlOptions.firmwareSlot         = FIRMWARE_SLOT_FLAG;
                if (SWITCH_FW_FLAG)
                {
                    dlOptions.existingFirmwareImage = true;
                }
                dlOptions.ignoreStatusOfFinalSegment =
                    false; // NOTE: This flag is not needed or used on products that support deferred download today.
                if (FORCE_NVME_COMMIT_ACTION != 0xFF)
                {
                    // forcing a specific commit action
                    dlOptions.forceCommitAction      = FORCE_NVME_COMMIT_ACTION;
                    dlOptions.forceCommitActionValid = true;
                }
                if (FORCE_DISABLE_NVME_FW_COMMIT_RESET)
                {
                    // disabling the reset after an NVMe commit
                    dlOptions.disableResetAfterCommit = true;
                }
                if (DOWNLOAD_FW_FLAG)
                {
                    // delay a second as this can help if running a download immediately followed by activate-TJE
                    delay_Seconds(1);
                }
                start_Timer(&commandTimer);
                ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                stop_Timer(&commandTimer);
                switch (ret)
                {
                case SUCCESS:
                case POWER_CYCLE_REQUIRED:
                    exitCode = C_CAST(eUtilExitCodes, SEACHEST_FIRMWARE_EXIT_DEFERRED_CODE_ACTIVATED);
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Firmware activation successful\n");
                        if (ret == POWER_CYCLE_REQUIRED)
                        {
                            printf("The Operating system has reported that a power cycle is required to complete the "
                                   "firmware update\n");
                        }
                        else
                        {
                            fill_Drive_Info_Data(&deviceList[deviceIter]);
                            if (NEW_FW_MATCH_FLAG)
                            {
                                if (strcmp(NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision) == 0)
                                {
                                    printf("Successfully validated firmware after download!\n");
                                    printf("New firmware version is %s\n",
                                           deviceList[deviceIter].drive_info.product_revision);
                                }
                                else
                                {
                                    printf("Unable to verify firmware after download!, expected %s, but found %s\n",
                                           NEW_FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision);
                                }
                            }
                            else
                            {
                                printf("New firmware version is %s\n",
                                       deviceList[deviceIter].drive_info.product_revision);
                            }
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
                        if (SWITCH_FW_FLAG && FIRMWARE_SLOT_FLAG == 0)
                        {
                            printf("You must specify a valid slot number when switching firmware images.\n");
                        }
                        else
                        {
                            printf("Firmware activate not supported\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Firmware activation failed\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("This drive does not support the activate command.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }
        // At this point, close the device handle since it is no longer needed. Do not put any further IO below this.
        close_Device(&deviceList[deviceIter]);
    }
    free_device_list(&DEVICE_LIST);
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

    printf("Examples\n");
    printf("========\n");
    // example usage
    printf("\t%s --%s\n", util_name, SCAN_LONG_OPT_STRING);
    printf("\t%s -d %s -%c\n", util_name, deviceHandleExample, DEVICE_INFO_SHORT_OPT);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SAT_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LOWLEVEL_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_FWDL_SUPPORT_LONG_OPT_STRING);
    printf("\tUpdating firmware:\n");
    printf("\t%s -d %s --%s file.bin\n", util_name, deviceHandleExample, DOWNLOAD_FW_LONG_OPT_STRING);
    printf("\tUpdating firmware with deferred download and activating:\n");
    printf("\t%s -d %s --%s file.bin --%s deferred --%s\n", util_name, deviceHandleExample, DOWNLOAD_FW_LONG_OPT_STRING,
           DOWNLOAD_FW_MODE_LONG_OPT_STRING, ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
    printf("\tUpdating firmware and specifying a firmware slot (NVMe)\n");
    printf("\t%s -d %s --%s file.bin --%s deferred\n", util_name, deviceHandleExample, DOWNLOAD_FW_LONG_OPT_STRING,
           DOWNLOAD_FW_MODE_LONG_OPT_STRING);
    printf("\t  +\n");
    printf("\t%s -d %s --%s --%s 2\n", util_name, deviceHandleExample, ACTIVATE_DEFERRED_FW_LONG_OPT_STRING,
           FIRMWARE_SLOT_LONG_OPT_STRING);
    // return codes
    printf("\nReturn codes\n");
    printf("============\n");
    // SEACHEST_FIRMWARE_EXIT_MAX_ERROR - SEACHEST_FIRMWARE_EXIT_FIRMWARE_DOWNLOAD_COMPLETE
    int totalErrorCodes = SEACHEST_FIRMWARE_EXIT_MAX_ERROR - SEACHEST_FIRMWARE_EXIT_FIRMWARE_DOWNLOAD_COMPLETE;
    ptrToolSpecificxitCode seachestFirmwareExitCodes = M_REINTERPRET_CAST(
        ptrToolSpecificxitCode, safe_calloc(int_to_sizet(totalErrorCodes), sizeof(toolSpecificxitCode)));
    // now set up all the exit codes and their meanings
    if (seachestFirmwareExitCodes)
    {
        for (int exitIter = UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE; exitIter < SEACHEST_FIRMWARE_EXIT_MAX_ERROR;
             ++exitIter)
        {
            seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCode = exitIter;
            switch (exitIter)
            {
            case SEACHEST_FIRMWARE_EXIT_FIRMWARE_DOWNLOAD_COMPLETE:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Firmware Download Complete");
                break;
            case SEACHEST_FIRMWARE_EXIT_DEFERRED_DOWNLOAD_COMPLETED:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Deferred Firmware Download Complete");
                break;
            case SEACHEST_FIRMWARE_EXIT_DEFERRED_CODE_ACTIVATED:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Deferred Code Activated");
                break;
            case SEACHEST_FIRMWARE_EXIT_NO_MATCH_FOUND:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "No Drive or Firmware match found");
                break;
            case SEACHEST_FIRMWARE_EXIT_MN_MATCH_FW_MISMATCH:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Model number matched, but Firmware mismatched");
                break;
            case SEACHEST_FIRMWARE_EXIT_FIRMWARE_HASH_DOESNT_MATCH:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Firmware File Hash Error");
                break;
            case SEACHEST_FIRMWARE_EXIT_ALREADY_UP_TO_DATE:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Firmware Already up to date");
                break;
            case SEACHEST_FIRMWARE_EXIT_MATCH_FOUND:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH, "Firmware Match Found for update");
                break;
            case SEACHEST_FIRMWARE_EXIT_MATCH_FOUND_DEFERRED_SUPPORTED:
                snprintf_err_handle(seachestFirmwareExitCodes[exitIter - UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE].exitCodeString,
                         TOOL_EXIT_CODE_STRING_MAX_LENGTH,
                         "Firmware Match Found for update - deferred update supported");
                break;
                // add more exit codes here!
            default: // We shouldn't ever hit the default case!
                break;
            }
        }
    }
    print_SeaChest_Util_Exit_Codes(totalErrorCodes, seachestFirmwareExitCodes, util_name);

    // utility options - alphabetized
    printf("Utility Options\n");
    printf("===============\n");
#if defined(ENABLE_CSMI)
    print_CSMI_Force_Flags_Help(shortUsage);
    print_CSMI_Verbose_Help(shortUsage);
#endif
#if defined(_WIN32)
    print_FWDL_Allow_Flexible_Win10_API_Use_Help(shortUsage);
    print_FWDL_Force_Win_Passthrough_Help(shortUsage);
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
    print_New_Firmware_Revision_Match_Help(shortUsage);
    print_No_Banner_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
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
    print_Fast_Discovery_Help(shortUsage);
    // utility tests/operations go here - alphabetized
    print_Firmware_Activate_Help(shortUsage);
    print_Firmware_Download_Help(shortUsage);
    print_Firmware_Download_Mode_Help(shortUsage);
    print_Firmware_Slot_Buffer_ID_Help(shortUsage);
    print_show_FWDL_Support_Help(shortUsage);
    print_FWDL_Ignore_Final_Segment_Help(shortUsage);
    print_FWDL_Segment_Size_Help(shortUsage);
    print_Firmware_Switch_Help(shortUsage);
}
