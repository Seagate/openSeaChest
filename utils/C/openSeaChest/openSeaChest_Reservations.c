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
// \file openSeaChest_Sample.c Binary command line that performs various <sample> methods on a device.

//////////////////////
//  Included files  //
//////////////////////
#include "common_types.h"
#include "io_utils.h"
#include "memory_safety.h"
#include "string_utils.h"
#include "type_conversion.h"
#include "unit_conversion.h"

#include "EULA.h"
#include "drive_info.h"
#include "getopt.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "reservations.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name    = "openSeaChest_Reservations";
const char* buildVersion = "0.4.1";

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
    DEVICE_INFO_VAR
    SAT_INFO_VAR
    // DATA_ERASE_VAR
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

#if defined(ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif

    SHOW_RESERVATION_CAPABILITIES_VAR
    SHOW_FULL_RESERVATION_INFO_VAR
    SHOW_REGISTRATION_KEYS_VAR
    SHOW_RESERVATIONS_VAR
    PERSISTENT_RESERVATION_KEY_VARS
    PERSISTENT_RESERVATION_TYPE_VARS
    PERSISTENT_RESERVATION_ATP_VAR
    PERSISTENT_RESERVATION_PTPL_VAR
    PERSISTENT_RESERVATION_REGISTER_VAR
    PERSISTENT_RESERVATION_UNREGISTER_VAR
    PERSISTENT_RESERVATION_REGISTER_I_VAR
    PERSISTENT_RESERVATION_RESERVE_VAR
    PERSISTENT_RESERVATION_RELEASE_VAR
    PERSISTENT_RESERVATION_CLEAR_VAR
    PERSISTENT_RESERVATION_PREEMPT_VARS
    PERSISTENT_RESERVATION_PREEMPT_ABORT_VAR
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
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
        LOWLEVEL_INFO_LONG_OPT,
#if defined(ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        SHOW_RESERVATION_CAPABILITIES_LONG_OPT,
        SHOW_FULL_RESERVATION_INFO_LONG_OPT,
        SHOW_REGISTRATION_KEYS_LONG_OPT,
        SHOW_RESERVATIONS_LONG_OPT,
        PERSISTENT_RESERVATION_KEY_LONG_OPT,
        PERSISTENT_RESERVATION_TYPE_LONG_OPT,
        PERSISTENT_RESERVATION_ATP_LONG_OPT,
        PERSISTENT_RESERVATION_PTPL_LONG_OPT,
        PERSISTENT_RESERVATION_REGISTER_LONG_OPT,
        PERSISTENT_RESERVATION_UNREGISTER_LONG_OPT,
        PERSISTENT_RESERVATION_REGISTER_I_LONG_OPT,
        PERSISTENT_RESERVATION_RESERVE_LONG_OPT,
        PERSISTENT_RESERVATION_RELEASE_LONG_OPT,
        PERSISTENT_RESERVATION_CLEAR_LONG_OPT,
        PERSISTENT_RESERVATION_PREEMPT_LONG_OPT,
        PERSISTENT_RESERVATION_PREEMPT_ABORT_LONG_OPT,
        // tool specific options go here
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
            if (strcmp(longopts[optionIndex].name, PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &PERSISTENT_RESERVATION_KEY))
                {
                    PERSISTENT_RESREVATION_KEY_VALID = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, PERSISTENT_RESERVATION_TYPE_LONG_OPT_STRING) == 0)
            {
                if (strcmp(optarg, "wrex") == 0)
                {
                    PERSISTENT_RESREVATION_TYPE_VALID = true;
                    PERSISTENT_RESERVATION_TYPE       = RES_TYPE_WRITE_EXCLUSIVE;
                }
                else if (strcmp(optarg, "ex") == 0)
                {
                    PERSISTENT_RESREVATION_TYPE_VALID = true;
                    PERSISTENT_RESERVATION_TYPE       = RES_TYPE_EXCLUSIVE_ACCESS;
                }
                else if (strcmp(optarg, "wrexro") == 0)
                {
                    PERSISTENT_RESREVATION_TYPE_VALID = true;
                    PERSISTENT_RESERVATION_TYPE       = RES_TYPE_WRITE_EXCLUSIVE_REGISTRANTS_ONLY;
                }
                else if (strcmp(optarg, "exro") == 0)
                {
                    PERSISTENT_RESREVATION_TYPE_VALID = true;
                    PERSISTENT_RESERVATION_TYPE       = RES_TYPE_EXCLUSIVE_ACCESS_REGISTRANTS_ONLY;
                }
                else if (strcmp(optarg, "wrexar") == 0)
                {
                    PERSISTENT_RESREVATION_TYPE_VALID = true;
                    PERSISTENT_RESERVATION_TYPE       = RES_TYPE_EXCLUSIVE_ACCESS_ALL_REGISTRANTS;
                }
                else if (strcmp(optarg, "exar") == 0)
                {
                    PERSISTENT_RESREVATION_TYPE_VALID = true;
                    PERSISTENT_RESERVATION_TYPE       = RES_TYPE_EXCLUSIVE_ACCESS_ALL_REGISTRANTS;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PERSISTENT_RESERVATION_TYPE_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strcmp(longopts[optionIndex].name, PERSISTENT_RESERVATION_PREEMPT_LONG_OPT_STRING) == 0)
            {
                if (get_And_Validate_Integer_Input_Uint64(optarg, M_NULLPTR, ALLOW_UNIT_NONE,
                                                          &PERSISTENT_RESERVATION_PREEMPT_KEY))
                {
                    PERSISTENT_RESERVATION_PREEMPT = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PERSISTENT_RESERVATION_PREEMPT_LONG_OPT_STRING, optarg);
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
    if (!(DEVICE_INFO_FLAG || TEST_UNIT_READY_FLAG ||
          LOWLEVEL_INFO_FLAG
          // check for other tool specific options here
          || SHOW_RESERVATION_CAPABILITIES || SHOW_FULL_RESERVATION_INFO || SHOW_REGISTRATION_KEYS ||
          SHOW_RESERVATIONS || PERSISTENT_RESERVATION_REGISTER || PERSISTENT_RESERVATION_UNREGISTER ||
          PERSISTENT_RESERVATION_RESERVE || PERSISTENT_RESERVATION_RELEASE || PERSISTENT_RESERVATION_CLEAR ||
          PERSISTENT_RESERVATION_PREEMPT))
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
            printf("\n%s - %s - %s - %s - %s\n", deviceList[deviceIter].os_info.name,
                   deviceList[deviceIter].drive_info.product_identification,
                   deviceList[deviceIter].drive_info.serialNumber, deviceList[deviceIter].drive_info.product_revision,
                   print_drive_type(&deviceList[deviceIter]));
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

        bool prSupported = is_Persistent_Reservations_Supported(&deviceList[deviceIter]);

        // Util specific options here
        if (SHOW_RESERVATION_CAPABILITIES)
        {
            if (prSupported)
            {
                persistentReservationCapabilities prCapabilities;
                safe_memset(&prCapabilities, sizeof(persistentReservationCapabilities), 0,
                            sizeof(persistentReservationCapabilities));
                prCapabilities.size    = sizeof(persistentReservationCapabilities);
                prCapabilities.version = PERSISTENT_RESERVATION_CAPABILITIES_VERSION;
                switch (get_Persistent_Reservations_Capabilities(&deviceList[deviceIter], &prCapabilities))
                {
                case SUCCESS:
                    show_Persistent_Reservations_Capabilities(&prCapabilities);
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("This device does not support the ability to report persistent reservation "
                               "capabilities.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to read persistent reservation capabilities.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SHOW_FULL_RESERVATION_INFO)
        {
            if (prSupported)
            {
                uint16_t               fullReservationsCount    = UINT16_C(0);
                size_t                 fullReservationsDataSize = SIZE_T_C(0);
                ptrFullReservationInfo fullInfo                 = M_NULLPTR;
                switch (get_Full_Status_Key_Count(&deviceList[deviceIter], &fullReservationsCount))
                {
                case SUCCESS:
                    fullReservationsDataSize =
                        sizeof(fullReservationInfo) + (sizeof(fullReservationKeyInfo) * fullReservationsCount);
                    fullInfo = M_REINTERPRET_CAST(ptrFullReservationInfo,
                                                  safe_calloc(fullReservationsDataSize, sizeof(uint8_t)));
                    if (fullInfo)
                    {
                        fullInfo->size    = fullReservationsDataSize;
                        fullInfo->version = FULL_RESERVATION_INFO_VERSION;
                        switch (get_Full_Status(&deviceList[deviceIter], fullReservationsCount, fullInfo))
                        {
                        case SUCCESS:
                            show_Full_Status(fullInfo);
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Full reservation status not available on this device.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Failed to read full reservation status.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                        safe_free_full_reservation_info(&fullInfo);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Error allocating memory to read full reservation information.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Full reservation information is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to read full reservation information.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SHOW_REGISTRATION_KEYS)
        {
            if (prSupported)
            {
                uint16_t                registrationKeyCount     = UINT16_C(0);
                size_t                  registrationKeysDataSize = SIZE_T_C(0);
                ptrRegistrationKeysData registrationKeys         = M_NULLPTR;
                switch (get_Registration_Key_Count(&deviceList[deviceIter], &registrationKeyCount))
                {
                case SUCCESS:
                    // allocate memory to read them
                    registrationKeysDataSize = sizeof(registrationKeysData) + (sizeof(uint64_t) * registrationKeyCount);
                    registrationKeys         = M_REINTERPRET_CAST(ptrRegistrationKeysData,
                                                                  safe_calloc(registrationKeysDataSize, sizeof(uint8_t)));
                    if (registrationKeys)
                    {
                        registrationKeys->size    = registrationKeysDataSize;
                        registrationKeys->version = REGISTRATION_KEY_DATA_VERSION;
                        switch (get_Registration_Keys(&deviceList[deviceIter], registrationKeyCount, registrationKeys))
                        {
                        case SUCCESS:
                            show_Registration_Keys(registrationKeys);
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Reporting registration keys is not supported on this device.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Failed to read registration keys\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                        safe_free_registration_key_data(&registrationKeys);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Error allocating memory to read registration keys.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Reporting registration keys is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to read registration keys\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SHOW_RESERVATIONS)
        {
            if (prSupported)
            {
                uint16_t            reservationKeyCount  = UINT16_C(0);
                size_t              reservationsDataSize = SIZE_T_C(0);
                ptrReservationsData reservations         = M_NULLPTR;
                switch (get_Reservation_Count(&deviceList[deviceIter], &reservationKeyCount))
                {
                case SUCCESS:
                    // allocate memory to read them
                    reservationsDataSize = sizeof(reservationsData) + (sizeof(reservationInfo) * reservationKeyCount);
                    reservations =
                        M_REINTERPRET_CAST(ptrReservationsData, safe_calloc(reservationsDataSize, sizeof(uint8_t)));
                    if (reservations)
                    {
                        reservations->size    = reservationsDataSize;
                        reservations->version = RESERVATION_DATA_VERSION;
                        switch (get_Reservations(&deviceList[deviceIter], reservationKeyCount, reservations))
                        {
                        case SUCCESS:
                            show_Reservations(reservations);
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Reporting reservations is not supported on this device.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < toolVerbosity)
                            {
                                printf("Failed to read reservations\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                        safe_free_reservation_data(&reservations);
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < toolVerbosity)
                        {
                            printf("Error allocating memory to read reservations.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Reporting reservations is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to read reservations\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        // A key must be provided for all of these options...
        if (!PERSISTENT_RESREVATION_KEY_VALID &&
            (PERSISTENT_RESERVATION_REGISTER || PERSISTENT_RESERVATION_UNREGISTER || PERSISTENT_RESERVATION_RESERVE ||
             PERSISTENT_RESERVATION_RELEASE || PERSISTENT_RESERVATION_CLEAR || PERSISTENT_RESERVATION_PREEMPT))
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Persistent Reservations are not supported on this device\n");
            }
            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            continue;
        }

        // persistent reservation type must be provided for these options.
        if (!PERSISTENT_RESREVATION_TYPE_VALID &&
            (PERSISTENT_RESERVATION_RESERVE || PERSISTENT_RESERVATION_RELEASE || PERSISTENT_RESERVATION_PREEMPT))
        {
            if (VERBOSITY_QUIET < toolVerbosity)
            {
                printf("Persistent Reservations type must be specified for reserve, release, or preempt actions.\n");
            }
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            continue;
        }

        if (PERSISTENT_RESERVATION_REGISTER)
        {
            if (prSupported)
            {
                switch (register_Key(&deviceList[deviceIter], PERSISTENT_RESERVATION_KEY,
                                     M_ToBool(PERSISTENT_RESERVATION_ATP), M_ToBool(PERSISTENT_RESERVATION_PTPL),
                                     M_ToBool(PERSISTENT_RESERVATION_REGISTER_I)))
                {
                case SUCCESS:
                    break;
                case NOT_SUPPORTED: // this should only really show up at this point if an invalid field was specified
                                    // for the device since we already check if reservations are supported above.-TJE
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("An unsupported option was provided while trying to register the provided key.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to register provided key and registration options.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (PERSISTENT_RESERVATION_UNREGISTER)
        {
            if (prSupported)
            {
                switch (unregister_Key(&deviceList[deviceIter], PERSISTENT_RESERVATION_KEY))
                {
                case SUCCESS:
                    break;
                case NOT_SUPPORTED: // this should only really show up at this point if an invalid field was specified
                                    // for the device since we already check if reservations are supported above.-TJE
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Invalid key specified. Cannot unregister this key.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to unregister provided key\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (PERSISTENT_RESERVATION_RESERVE)
        {
            if (prSupported)
            {
                switch (acquire_Reservation(&deviceList[deviceIter], PERSISTENT_RESERVATION_KEY,
                                            C_CAST(eReservationType, PERSISTENT_RESERVATION_TYPE)))
                {
                case SUCCESS:
                    break;
                case NOT_SUPPORTED: // this should only really show up at this point if an invalid field was specified
                                    // for the device since we already check if reservations are supported above.-TJE
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf(
                            "Invalid key or reservation type specified. Cannot acquire reservation for the device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to acquire a reservation for the device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (PERSISTENT_RESERVATION_RELEASE)
        {
            if (prSupported)
            {
                switch (release_Reservation(&deviceList[deviceIter], PERSISTENT_RESERVATION_KEY,
                                            C_CAST(eReservationType, PERSISTENT_RESERVATION_TYPE)))
                {
                case SUCCESS:
                    break;
                case NOT_SUPPORTED: // this should only really show up at this point if an invalid field was specified
                                    // for the device since we already check if reservations are supported above.-TJE
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf(
                            "Invalid key or reservation type specified. Cannot release reservation for the device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to release a reservation for the device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (PERSISTENT_RESERVATION_CLEAR)
        {
            if (prSupported)
            {
                switch (clear_Reservations(&deviceList[deviceIter], PERSISTENT_RESERVATION_KEY))
                {
                case SUCCESS:
                    break;
                case NOT_SUPPORTED: // this should only really show up at this point if an invalid field was specified
                                    // for the device since we already check if reservations are supported above.-TJE
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Invalid key specified. Cannot clear reservations for the device.\n");
                        printf("Make sure this key has been registered first, then try again.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to clear reservations for the device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (PERSISTENT_RESERVATION_PREEMPT)
        {
            if (prSupported)
            {
                switch (preempt_Reservation(&deviceList[deviceIter], PERSISTENT_RESERVATION_KEY,
                                            PERSISTENT_RESERVATION_PREEMPT_KEY,
                                            M_ToBool(PERSISTENT_RESERVATION_PREEMPT_ABORT),
                                            C_CAST(eReservationType, PERSISTENT_RESERVATION_TYPE)))
                {
                case SUCCESS:
                    break;
                case NOT_SUPPORTED: // this should only really show up at this point if an invalid field was specified
                                    // for the device since we already check if reservations are supported above.-TJE
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Invalid option specified while trying to preempt a reservation.\n");
                        printf("Check the provided key and preempt key. Some devices may not support\n");
                        printf("the option to preempt-abort a reservation.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to preempt a reservation for the device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Persistent Reservations are not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
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
    printf("NOTE: This utility only supports persistent reservations.\n");
    printf("      SCSI 2 reservations are not supported.\n\n");

    printf("\nExamples\n");
    printf("========\n");
    // example usage
    printf("\t%s --%s\n", util_name, SCAN_LONG_OPT_STRING);
    printf("\t%s -d %s -%c\n", util_name, deviceHandleExample, DEVICE_INFO_SHORT_OPT);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SAT_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LOWLEVEL_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_RESERVATION_CAPABILITIES_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_FULL_RESERVATION_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_REGISTRATION_KEYS_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SHOW_RESERVATIONS_LONG_OPT_STRING);
    // TODO: examples of creating a reservation, clearing, aborting, preempting, releasing

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
    // multiple interfaces

    print_Persistent_Reservations_All_Target_Ports_Help(shortUsage);
    print_Persistent_Reservations_Clear_Help(shortUsage);
    print_Persistent_Reservations_Key_Help(shortUsage);
    print_Persistent_Reservations_Persist_Through_Power_Loss_Help(shortUsage);
    print_Persistent_Reservations_Preempt_Help(shortUsage);
    print_Persistent_Reservations_Preempt_Abort_Help(shortUsage);
    print_Persistent_Reservations_Register_Help(shortUsage);
    print_Persistent_Reservations_Register_Ignore_Help(shortUsage);
    print_Persistent_Reservations_Release_Help(shortUsage);
    print_Persistent_Reservations_Reserve_Help(shortUsage);
    print_Persistent_Reservations_Type_Help(shortUsage);
    print_Persistent_Reservations_Unregister_Help(shortUsage);
    print_Show_Full_Reservation_Info(shortUsage);
    print_Show_Registration_Keys(shortUsage);
    print_Show_Reservations(shortUsage);
    print_Show_Reservation_Capabilities(shortUsage);

    // data destructive commands - alphabetized
    // printf("\nData Destructive Commands\n");
    // printf("=========================\n");
    // utility data destructive tests/operations go here
}
