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
// \file openSeaChest_Defect.c CLI to create, view, and manage defects

//////////////////////
//  Included files  //
//////////////////////
#include "common.h"
#include <ctype.h>
#if defined (__unix__) || defined(__APPLE__) //using this definition because linux and unix compilers both define this. Apple does not define this, which is why it has it's own definition
#include <unistd.h>
#endif
#include "getopt.h"
#include "EULA.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "drive_info.h"
#include "dst.h"
#include "smart.h"
#include "seagate_operations.h"
#include "defect.h"

////////////////////////
//  Global Variables  //
////////////////////////
const char* util_name = "openSeaChest_Defect";
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
int main(int argc, char* argv[])
{
    /////////////////
    //  Variables  //
    /////////////////
    //common utility variables
    int                 ret = SUCCESS;
    int      exitCode = UTIL_EXIT_NO_ERROR;
    DEVICE_UTIL_VARS
    DEVICE_INFO_VAR
    SAT_INFO_VAR
    //DATA_ERASE_VAR
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
    SINGLE_SECTOR_DATA_ERASE_VAR
    ENABLE_LEGACY_PASSTHROUGH_VAR
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    DST_AND_CLEAN_VAR
    ERROR_LIMIT_VAR
#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif

    CHECK_PENDING_LIST_COUNT_VARS
    CHECK_GROWN_LIST_COUNT_VARS
    SHOW_PENDING_LIST_VAR
    CREATE_UNCORRECTABLE_VAR
    CREATE_UNCORRECTABLE_LBA_VAR
    UNCORRECTABLE_RANGE_VAR
    RANDOM_UNCORRECTABLES_VAR
    READ_UNCORRECTABLES_VAR
    FLAG_UNCORRECTABLES_VAR
    CORRUPT_LBA_VAR
    CORRUPT_LBA_RANGE_VAR
    CORRUPT_RANDOM_LBAS_VAR
    BYTES_TO_CORRUPT_VAR

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
        CONFIRM_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        //tool specific options go here
        ERROR_LIMIT_LONG_OPT,
        DST_AND_CLEAN_LONG_OPT,
        CHECK_PENDING_LIST_COUNT_LONG_OPT,
        CHECK_GROWN_LIST_COUNT_LONG_OPT,
        SHOW_PENDING_LIST_LONG_OPT,
        CREATE_UNCORRECTABLE_LONG_OPT,
        UNCORRECTABLE_RANGE_LONG_OPT,
        RANDOM_UNCORRECTABLES_LONG_OPT,
        DISABLE_READ_UNCORRECTABLES_LONG_OPT,
        FLAG_UNCORRECTABLE_LONG_OPT,
        CORRUPT_LBA_LONG_OPT,
        CORRUPT_LBA_RANGE_LONG_OPT,
        CORRUPT_RANDOM_LBAS_LONG_OPT,
        BYTES_TO_CORRUPT_LONG_OPT,
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
            if (strncmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CONFIRM_LONG_OPT_STRING))) == 0)
            {
                if (strlen(optarg) == strlen(SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING, strlen(SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    SINGLE_SECTOR_DATA_ERASE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CONFIRM_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, ERROR_LIMIT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(ERROR_LIMIT_LONG_OPT_STRING))) == 0)
            {
                ERROR_LIMIT_FLAG = C_CAST(uint16_t, atoi(optarg));
                if (strchr(optarg, 'L') || strchr(optarg, 'l'))
                {
                    ERROR_LIMIT_LOGICAL_COUNT = true;
                }
            }
            else if (strncmp(longopts[optionIndex].name, CHECK_PENDING_LIST_COUNT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHECK_PENDING_LIST_COUNT_LONG_OPT_STRING))) == 0)
            {
                CHECK_PENDING_LIST_COUNT_FLAG = true;
                CHECK_PENDING_LIST_COUNT_VALUE = C_CAST(uint32_t, atoi(optarg));
                //see if they want physical or logical sectors
                if (strchr(optarg, 'L') || strchr(optarg, 'l'))
                {
                    CHECK_PENDING_LIST_COUNT_LOGICAL_FLAG = true;
                }
            }
            else if (strncmp(longopts[optionIndex].name, CHECK_GROWN_LIST_COUNT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHECK_GROWN_LIST_COUNT_LONG_OPT_STRING))) == 0)
            {
                CHECK_GROWN_LIST_COUNT_FLAG = true;
                CHECK_GROWN_LIST_COUNT_VALUE = C_CAST(uint32_t, atoi(optarg));
                //see if they want physical or logical sectors
                if (strchr(optarg, 'L') || strchr(optarg, 'l'))
                {
                    CHECK_GROWN_LIST_COUNT_LOGICAL_FLAG = true;
                }
            }
            else if (strncmp(longopts[optionIndex].name, CREATE_UNCORRECTABLE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CREATE_UNCORRECTABLE_LONG_OPT_STRING))) == 0)
            {
                if (get_And_Validate_Integer_Input(C_CAST(const char*, optarg), &CREATE_UNCORRECTABLE_LBA_FLAG))
                {
                    CREATE_UNCORRECTABLE_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CREATE_UNCORRECTABLE_LONG_OPT_STRING, C_CAST(const char*, optarg));
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, UNCORRECTABLE_RANGE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(UNCORRECTABLE_RANGE_LONG_OPT_STRING))) == 0)
            {
                if (!get_And_Validate_Integer_Input(C_CAST(const char*, optarg), &UNCORRECTABLE_RANGE_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(UNCORRECTABLE_RANGE_LONG_OPT_STRING, C_CAST(const char*, optarg));
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, RANDOM_UNCORRECTABLES_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(RANDOM_UNCORRECTABLES_LONG_OPT_STRING))) == 0)
            {
                uint64_t temp = 0;
                if (get_And_Validate_Integer_Input(C_CAST(const char*, optarg), &temp))
                {
                    RANDOM_UNCORRECTABLES_FLAG = C_CAST(uint16_t, temp);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(RANDOM_UNCORRECTABLES_LONG_OPT_STRING, C_CAST(const char*, optarg));
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, DISABLE_READ_UNCORRECTABLES_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DISABLE_READ_UNCORRECTABLES_LONG_OPT_STRING))) == 0)
            {
                READ_UNCORRECTABLES_FLAG = false;
            }
            else if (strncmp(longopts[optionIndex].name, CORRUPT_LBA_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CORRUPT_LBA_LONG_OPT_STRING))) == 0)
            {
                if (get_And_Validate_Integer_Input(C_CAST(const char*, optarg), &CORRUPT_LBA_LBA))
                {
                    CORRUPT_LBA_FLAG = true;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CORRUPT_LBA_LONG_OPT_STRING, C_CAST(const char*, optarg));
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, CORRUPT_LBA_RANGE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CORRUPT_LBA_RANGE_LONG_OPT_STRING))) == 0)
            {
                if (!get_And_Validate_Integer_Input(C_CAST(const char*, optarg), &CORRUPT_LBA_RANGE_FLAG))
                {
                    print_Error_In_Cmd_Line_Args(CORRUPT_LBA_RANGE_LONG_OPT_STRING, C_CAST(const char*, optarg));
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, CORRUPT_RANDOM_LBAS_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CORRUPT_RANDOM_LBAS_LONG_OPT_STRING))) == 0)
            {
                uint64_t temp = 0;
                if (get_And_Validate_Integer_Input(C_CAST(const char*, optarg), &temp))
                {
                    CORRUPT_RANDOM_LBAS = true;
                    CORRUPT_RANDOM_LBAS_COUNT = C_CAST(uint16_t, temp);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(CORRUPT_RANDOM_LBAS_LONG_OPT_STRING, C_CAST(const char*, optarg));
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, BYTES_TO_CORRUPT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(BYTES_TO_CORRUPT_LONG_OPT_STRING))) == 0)
            {
                uint64_t temp = 0;
                if (get_And_Validate_Integer_Input(C_CAST(const char*, optarg), &temp))
                {
                    BYTES_TO_CORRUPT_FLAG = true;
                    BYTES_TO_CORRUPT_VAL = C_CAST(uint16_t, temp);
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(BYTES_TO_CORRUPT_LONG_OPT_STRING, C_CAST(const char*, optarg));
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

    //From this point on, we need elevated privileges so we can talk to the drive(s)
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
        || DST_AND_CLEAN_FLAG
        || CHECK_PENDING_LIST_COUNT_FLAG
        || CHECK_GROWN_LIST_COUNT_FLAG
        || SHOW_PENDING_LIST
        || CREATE_UNCORRECTABLE_FLAG
        || UNCORRECTABLE_RANGE_FLAG > 1
        || RANDOM_UNCORRECTABLES_FLAG
        || CORRUPT_LBA_FLAG
        || CORRUPT_RANDOM_LBAS
        ))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    uint64_t flags = 0;
    DEVICE_LIST = C_CAST(tDevice*, calloc(DEVICE_LIST_COUNT, sizeof(tDevice)));
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

    if (FAST_DISCOVERY_FLAG)
    {
        flags = FAST_SCAN;
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
#elif !defined(_WIN32)
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

        if (CHECK_PENDING_LIST_COUNT_FLAG)
        {
            uint32_t driveReportedPListCount = 0;
            uint32_t pendingErrorLimit = CHECK_PENDING_LIST_COUNT_VALUE;
            switch (get_Pending_List_Count(&deviceList[deviceIter], &driveReportedPListCount))
            {
            case SUCCESS://got a valid count, so compare it to what the user wants to check
                if (!CHECK_PENDING_LIST_COUNT_LOGICAL_FLAG)
                {
                    //convert the count to a physical block count..integer division shouldn't be a problem right now...but this may need revisiting.-TJE
                    driveReportedPListCount /= deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize;
                }
                if (driveReportedPListCount > pendingErrorLimit)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Error: drive has more pending errors than the provided limit!\n");
                        printf("\tDrive reports %"PRIu32", limit is %"PRIu32" ", driveReportedPListCount, pendingErrorLimit);
                        if (CHECK_PENDING_LIST_COUNT_LOGICAL_FLAG)
                        {
                            printf("Logical Blocks\n");
                        }
                        else
                        {
                            printf("Physical Blocks\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Drive has fewer pending errors than the provided limit.\n");
                        printf("\tDrive reports %"PRIu32", limit is %"PRIu32" ", driveReportedPListCount, pendingErrorLimit);
                        if (CHECK_PENDING_LIST_COUNT_LOGICAL_FLAG)
                        {
                            printf("Logical Blocks\n");
                        }
                        else
                        {
                            printf("Physical Blocks\n");
                        }
                    }
                }
                break;
            case NOT_SUPPORTED://cannot get the count on this drive because it is not supported.
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Pending list count on this drive is not supported.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default://something went wrong
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get Pending list count.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (CHECK_GROWN_LIST_COUNT_FLAG)
        {
            uint32_t driveReportedGListCount = 0;
            uint32_t grownErrorLimit = CHECK_GROWN_LIST_COUNT_VALUE;
            switch (get_Grown_List_Count(&deviceList[deviceIter], &driveReportedGListCount))
            {
            case SUCCESS://got a valid count, so compare it to what the user wants to check
                if (!CHECK_GROWN_LIST_COUNT_LOGICAL_FLAG)
                {
                    //convert the count to a physical block count..integer division shouldn't be a problem right now...but this may need revisiting.-TJE
                    driveReportedGListCount /= deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize;
                }
                if (driveReportedGListCount > grownErrorLimit)
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Error: drive has more grown errors than the provided limit!\n");
                        printf("\tDrive reports %"PRIu32", limit is %"PRIu32" ", driveReportedGListCount, grownErrorLimit);
                        if (CHECK_GROWN_LIST_COUNT_LOGICAL_FLAG)
                        {
                            printf("Logical Blocks\n");
                        }
                        else
                        {
                            printf("Physical Blocks\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                }
                else
                {
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Drive has fewer grown errors than the provided limit.\n");
                        printf("\tDrive reports %"PRIu32", limit is %"PRIu32" ", driveReportedGListCount, grownErrorLimit);
                        if (CHECK_GROWN_LIST_COUNT_LOGICAL_FLAG)
                        {
                            printf("Logical Blocks\n");
                        }
                        else
                        {
                            printf("Physical Blocks\n");
                        }
                    }
                }
                break;
            case NOT_SUPPORTED://cannot get the count on this drive because it is not supported.
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Grown list count on this drive is not supported.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default://something went wrong
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Failed to get Grown list count.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHOW_PENDING_LIST)
        {
            //extern int get_LBAs_From_Pending_List(tDevice *device, ptrPendingDefect defectList, uint32_t *numberOfDefects);
            uint32_t numberOfPendingLBAs = 0;
            ptrPendingDefect pendingLBAs = C_CAST(ptrPendingDefect, malloc(MAX_PLIST_ENTRIES * sizeof(pendingDefect)));
            if (pendingLBAs)
            {
                switch (get_LBAs_From_Pending_List(&deviceList[deviceIter], pendingLBAs, &numberOfPendingLBAs))
                {
                case SUCCESS:
                    show_Pending_List(pendingLBAs, numberOfPendingLBAs);
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Pending list is not supported on this device.\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Failed to read pending list\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
                safe_Free(pendingLBAs);
            }
            else
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("Unable to allocate memory for pending list\n");
                }
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
            }
        }

        if (DST_AND_CLEAN_FLAG)
        {
            if (SINGLE_SECTOR_DATA_ERASE_FLAG)
            {
                if (VERBOSITY_QUIET < toolVerbosity)
                {
                    printf("DST And Clean\n");
                }
                if (ERROR_LIMIT_LOGICAL_COUNT)
                {
                    ERROR_LIMIT_FLAG *= C_CAST(uint16_t, deviceList[deviceIter].drive_info.devicePhyBlockSize / deviceList[deviceIter].drive_info.deviceBlockSize);
                }
                switch (run_DST_And_Clean(&deviceList[deviceIter], ERROR_LIMIT_FLAG, NULL, NULL, NULL, NULL))
                {
                case UNKNOWN:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("Unknown Error occurred while trying to start DST and Clean\n");
                    }
                    break;
                case SUCCESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DST and Clean completed successfully\n");
                    }
                    break;
                case IN_PROGRESS:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("A self test is currently in progress. Please wait for it to finish before starting DST and Clean\n");
                    }
                    break;
                case ABORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DST And Clean was aborted!\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_ABORTED;
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DST and Clean is not supported on this device\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < toolVerbosity)
                    {
                        printf("DST and Clean Failed!\n");
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
                    printf("You must add the flag:\n\"%s\" \n", SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to run a dst and clean operation.\n\n");
                    printf("e.g.: %s -d %s --%s --confirm %s\n\n", util_name, deviceHandleExample, DST_AND_CLEAN_LONG_OPT_STRING, SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (CREATE_UNCORRECTABLE_FLAG && CREATE_UNCORRECTABLE_LBA_FLAG != UINT64_MAX)
        {
            if (UNCORRECTABLE_RANGE_FLAG > 0 && UNCORRECTABLE_RANGE_FLAG < deviceList[deviceIter].drive_info.deviceMaxLba)
            {
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    if (!FLAG_UNCORRECTABLES_FLAG)
                    {
                        printf("Creating uncorrectable errors at LBA %"PRIu64" for a range of %"PRIu64" LBAs\n.", CREATE_UNCORRECTABLE_LBA_FLAG, UNCORRECTABLE_RANGE_FLAG);
                    }
                    else
                    {
                        printf("Flagging uncorrectable errors at LBA %"PRIu64" for a range of %"PRIu64" LBAs\n.", CREATE_UNCORRECTABLE_LBA_FLAG, UNCORRECTABLE_RANGE_FLAG);
                    }
                }
                int uncorrectableRet = UNKNOWN;
                if (!FLAG_UNCORRECTABLES_FLAG)
                {
                    uncorrectableRet = create_Uncorrectables(&deviceList[deviceIter], CREATE_UNCORRECTABLE_LBA_FLAG, UNCORRECTABLE_RANGE_FLAG, READ_UNCORRECTABLES_FLAG, NULL, NULL);
                }
                else
                {
                    uncorrectableRet = flag_Uncorrectables(&deviceList[deviceIter], CREATE_UNCORRECTABLE_LBA_FLAG, UNCORRECTABLE_RANGE_FLAG, NULL, NULL);
                }
                switch (uncorrectableRet)
                {
                case SUCCESS:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        if (FLAG_UNCORRECTABLES_FLAG)
                        {
                            printf("Successfully flagged uncorrectable errors on the device.\n");
                        }
                        else
                        {
                            printf("Successfully created uncorrectable errors on the device.\n");
                        }
                    }
                    break;
                case NOT_SUPPORTED:
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        if (FLAG_UNCORRECTABLES_FLAG)
                        {
                            printf("Flagging uncorrectable errors is not supported by this device at this time.\n");
                        }
                        else
                        {
                            printf("Creating uncorrectable errors is not supported by this device at this time.\n");
                        }
                    }
                    break;
                default:
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        if (FLAG_UNCORRECTABLES_FLAG)
                        {
                            printf("Failed to flag uncorrectable errors on the device.\n");
                        }
                        else
                        {
                            printf("Failed to create uncorrectable errors on the device.\n");
                        }
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("The range specified is invalid. Please enter a range that does not exceed the MaxLBA of the device from the specified starting LBA.\n");
                }
            }
        }
        else if (CREATE_UNCORRECTABLE_FLAG)
        {
            if (UNCORRECTABLE_RANGE_FLAG > 1)
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    printf("You must used the --createUncorrectable option with an LBA to provide a starting point for the range provided.\n");
                }
            }
        }

        if (RANDOM_UNCORRECTABLES_FLAG > 0)
        {
            if (toolVerbosity > VERBOSITY_QUIET)
            {
                if (FLAG_UNCORRECTABLES_FLAG)
                {
                    printf("Flagging %"PRIu16" random uncorrectables on the device\n", RANDOM_UNCORRECTABLES_FLAG);
                }
                else
                {
                    printf("Creating %"PRIu16" random uncorrectables on the device\n", RANDOM_UNCORRECTABLES_FLAG);
                }
            }
            switch (create_Random_Uncorrectables(&deviceList[deviceIter], RANDOM_UNCORRECTABLES_FLAG, READ_UNCORRECTABLES_FLAG, FLAG_UNCORRECTABLES_FLAG, NULL, NULL))
            {
            case SUCCESS:
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    if (FLAG_UNCORRECTABLES_FLAG)
                    {
                        printf("Successfully flagged random uncorrectable errors on the device.\n");
                    }
                    else
                    {
                        printf("Successfully created random uncorrectable errors on the device.\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    if (FLAG_UNCORRECTABLES_FLAG)
                    {
                        printf("Flagging random uncorrectable errors is not supported by this device at this time.\n");
                    }
                    else
                    {
                        printf("Creating random uncorrectable errors is not supported by this device at this time.\n");
                    }
                }
                break;
            default:
                if (toolVerbosity > VERBOSITY_QUIET)
                {
                    if (FLAG_UNCORRECTABLES_FLAG)
                    {
                        printf("Failed to flag random uncorrectable errors on the device.\n");
                    }
                    else
                    {
                        printf("Failed to create random uncorrectable errors on the device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (CORRUPT_LBA_FLAG)
        {
            if (is_Read_Long_Write_Long_Supported(&deviceList[deviceIter]))
            {
                bool bytesToCorruptSet = true;
                if (!BYTES_TO_CORRUPT_FLAG)
                {
                    //this means the user didn't enter anything, so we need to use a default value.
                    //The defaults here were defined internally and it's not clear if they are product specific or not.
                    if (deviceList[deviceIter].drive_info.devicePhyBlockSize >= 512 && deviceList[deviceIter].drive_info.devicePhyBlockSize <= 1024)//trying to handle drives that may be formatted to 520, etc sizes
                    {
                        BYTES_TO_CORRUPT_VAL = 62;
                    }
                    else if (deviceList[deviceIter].drive_info.devicePhyBlockSize >= 4096 && deviceList[deviceIter].drive_info.devicePhyBlockSize <= 8192)
                    {
                        BYTES_TO_CORRUPT_VAL = 360;
                    }
                    else
                    {
                        exitCode = UTIL_EXIT_OPERATION_ABORTED;
                        bytesToCorruptSet = false;
                        printf("ERROR: Unable to determine default number of bytes to corrupt. Please specify it with the --%s option.\n", BYTES_TO_CORRUPT_LONG_OPT_STRING);
                    }
                }
                if (bytesToCorruptSet)
                {
                    //create the correctable error condition
                    switch (corrupt_LBAs(&deviceList[deviceIter], CORRUPT_LBA_LBA, CORRUPT_LBA_RANGE_FLAG, READ_UNCORRECTABLES_FLAG, BYTES_TO_CORRUPT_VAL, NULL, NULL))
                    {
                    case SUCCESS:
                        if (toolVerbosity > VERBOSITY_QUIET)
                        {
                            printf("Successfully corrupted %" PRIu16" bytes on the device at LBA %" PRIu64 " to LBA %" PRIu64 ".\n", BYTES_TO_CORRUPT_VAL, CORRUPT_LBA_LBA, CORRUPT_LBA_LBA + CORRUPT_LBA_RANGE_FLAG);
                        }
                        break;
                    case NOT_SUPPORTED:
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        if (toolVerbosity > VERBOSITY_QUIET)
                        {
                            printf("Creating correctable errors is not supported by this device at this time.\n");
                        }
                        break;
                    default:
                        if (toolVerbosity > VERBOSITY_QUIET)
                        {
                            printf("Failed to create correctable errors on the device.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
            }
            else
            {
                printf("Creating correctable errors is not supported on this drive.\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (CORRUPT_RANDOM_LBAS && CORRUPT_RANDOM_LBAS_COUNT > 0)
        {
            if (is_Read_Long_Write_Long_Supported(&deviceList[deviceIter]))
            {
                bool bytesToCorruptSet = true;
                if (!BYTES_TO_CORRUPT_FLAG)
                {
                    //this means the user didn't enter anything, so we need to use a default value.
                    //The defaults here were defined internally and it's not clear if they are product specific or not.
                    if (deviceList[deviceIter].drive_info.devicePhyBlockSize >= 512 && deviceList[deviceIter].drive_info.devicePhyBlockSize <= 1024)//trying to handle drives that may be formatted to 520, etc sizes
                    {
                        BYTES_TO_CORRUPT_VAL = 62;
                    }
                    else if (deviceList[deviceIter].drive_info.devicePhyBlockSize >= 4096 && deviceList[deviceIter].drive_info.devicePhyBlockSize <= 8192)
                    {
                        BYTES_TO_CORRUPT_VAL = 360;
                    }
                    else
                    {
                        exitCode = UTIL_EXIT_OPERATION_ABORTED;
                        bytesToCorruptSet = false;
                        printf("ERROR: Unable to determine default number of bytes to corrupt. Please specify it with the --%s option.\n", BYTES_TO_CORRUPT_LONG_OPT_STRING);
                    }
                }
                if (bytesToCorruptSet)
                {
                    if (toolVerbosity > VERBOSITY_QUIET)
                    {
                        printf("Corrupting %"PRIu16" random LBAs on the device\n", CORRUPT_RANDOM_LBAS_COUNT);
                    }
                    switch (corrupt_Random_LBAs(&deviceList[deviceIter], CORRUPT_RANDOM_LBAS_COUNT, READ_UNCORRECTABLES_FLAG, BYTES_TO_CORRUPT_VAL, NULL, NULL))
                    {
                    case SUCCESS:
                        if (toolVerbosity > VERBOSITY_QUIET)
                        {
                            printf("Successfully corrupted %" PRIu16 "random LBAs on the device.\n", CORRUPT_RANDOM_LBAS_COUNT);
                        }
                        break;
                    case NOT_SUPPORTED:
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        if (toolVerbosity > VERBOSITY_QUIET)
                        {
                            printf("Corrupting random LBAs is not supported by this device at this time.\n");
                        }
                        break;
                    default:
                        if (toolVerbosity > VERBOSITY_QUIET)
                        {
                            printf("Failed to corrupt random LBAs on the device.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
            }
            else
            {
                printf("Creating correctable errors is not supported on this drive.\n");
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
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
    print_Fast_Discovery_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    print_Error_Limit_Help(shortUsage);
    //multiple interfaces
    print_Check_Grown_List_Help(shortUsage);
    print_Check_Pending_List_Help(shortUsage);
    print_Show_Pending_List_Help(shortUsage);

    //SATA Only Options
    //printf("\n\tSATA Only:\n\n");

    //SAS Only Options
    //printf("\n\tSAS Only:\n\n");


    //data destructive commands - alphabetized
    printf("Data Destructive Commands (Seagate only)\n");
    printf("========================================\n");
    //utility data destructive tests/operations go here
    print_Bytes_To_Corrupt_Help(shortUsage);
    print_DST_And_Clean_Help(shortUsage);
    print_Corrupt_LBA_Help(shortUsage);
    print_Corrupt_Random_LBAs_Help(shortUsage);
    print_Corrupt_Range_Help(shortUsage);
    print_Disable_Read_Uncorrectables_Help(shortUsage);
    print_Flag_Uncorrectable_Help(shortUsage);
    print_Create_Uncorrectable_Help(shortUsage);
    print_Random_Uncorrectable_Help(shortUsage);
    print_Uncorrectable_Range_Help(shortUsage);
}
