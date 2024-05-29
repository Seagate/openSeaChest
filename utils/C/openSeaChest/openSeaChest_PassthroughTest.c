// SPDX-License-Identifier: MPL-2.0
//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2019-2022 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
//
// \file openSeaChest_PassthroughTest.c CLI tool that attempts to determine what works or doesn't work to talk passthrough to a device

//////////////////////
//  Included files  //
//////////////////////
#include "common.h"
#include <ctype.h>
#if defined (__unix__) || defined(__APPLE__) //using this definition because linux and unix compilers both define this. Apple does not define this, which is why it has it's own definition
#include <unistd.h>
#include "getopt.h"
#elif defined (_WIN32)
#include "getopt.h"
#else
#error "OS Not Defined or known"
#endif
#include "EULA.h"
#include "openseachest_util_options.h"
#include "operations.h"
#include "drive_info.h"
#include "common_public.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_PassthroughTest";
const char *buildVersion = "1.4.1";

////////////////////////////
//  functions to declare  //
////////////////////////////
static void utility_Usage(bool shortUsage);


typedef struct _passthroughTestParams
{
    tDevice *device;//pointer to device to test
    bool suspectedDriveTypeProvidedByUser;
    eDriveType suspectedDriveType;//This should be set by the tool user to help identify things to try and do.
    bool suspectedPassthroughProvidedByUser;
    ePassthroughType suspectedPassthroughType;//Only set this to seomthing ahead of time if non-sat
    bool forceRetest;//even though the device is already known, retest it.
    bool disableTranslatorPassthroughTesting;//Can be used if wanting to disable SAT or VS NVMe passthrough command tests.
    bool allowLegacyATAPTTest;
    bool testPotentiallyDeviceHangingCommands;//extra testing for other commands that are known to cause issues on some devices
    struct
    {
        bool zeroLengthReads;
        bool sctLogWithGPL;//SAT only. Only some devices
        bool returnResponseInforWithoutTdir;//SAT only. Only some devices.
    }hangCommandsToTest;
    //options below here are not supported yet, but should be added
    bool noMultiSectorPIOTest;
    bool noReturnResponseInfoTest;
    bool nocheckConditionTest;

}passthroughTestParams, *ptrPassthroughTestParams;

eReturnValues perform_Passthrough_Test(ptrPassthroughTestParams inputs);

#define HACK_COLOR CONSOLE_COLOR_BLUE
#define LIKELY_HACK_COLOR CONSOLE_COLOR_CYAN
#define WARNING_COLOR CONSOLE_COLOR_YELLOW
#define ERROR_COLOR CONSOLE_COLOR_RED
#define NOTE_COLOR CONSOLE_COLOR_MAGENTA
#define HEADING_COLOR CONSOLE_COLOR_GREEN

//run the passthrough test
#define RUN_PASSTHROUGH_TEST_FLAG passthroughTest
#define RUN_PASSTHROUGH_TEST_VAR int RUN_PASSTHROUGH_TEST_FLAG = 0;
#define RUN_PASSTHROUGH_TEST_LONG_OPT_STRING "runPTTest"
#define RUN_PASSTHROUGH_TEST_LONG_OPT { RUN_PASSTHROUGH_TEST_LONG_OPT_STRING, no_argument, &RUN_PASSTHROUGH_TEST_FLAG, 1 }

//hint that the drive is ATA or NVMe
#define PT_DRIVE_HINT passthroughDriveTypeHint
#define PT_DRIVE_HINT_VAR int PT_DRIVE_HINT = -1;
#define PT_DRIVE_HINT_LONG_OPT_STRING "ptDriveHint"
#define PT_DRIVE_HINT_LONG_OPT { PT_DRIVE_HINT_LONG_OPT_STRING, required_argument, NULL, 0 }

static void print_Drive_Type_Hint_Help(bool shortHelp)
{
    printf("\t--%s [ata | nvme]\n", PT_DRIVE_HINT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option passes a hint to the software that the device being tested is\n");
        printf("\t\tan ATA or NVMe drive so it can appropriately test it.\n");
        printf("\t\tThis option is most useful when also using --ptTypeHint\n");
        printf("\n");
    }
}

//hint that the passthrough is SAT or legacyATA or VU_NVME, etc.
#define PT_PTTYPE_HINT passthroughTypeHint
#define PT_PTTYPE_HINT_VAR int PT_PTTYPE_HINT = -1;
#define PT_PTTYPE_HINT_LONG_OPT_STRING "ptTypeHint"
#define PT_PTTYPE_HINT_LONG_OPT { PT_PTTYPE_HINT_LONG_OPT_STRING, required_argument, NULL, 0 }

static void print_Passthrough_Type_Hint_Help(bool shortHelp)
{
    printf("\t--%s [sat | legacyATA]\n", PT_PTTYPE_HINT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tAdd this option if the device being tested is likely to support one of\n");
        printf("\t\tthe supported passthrough types. This hints to the software to perform\n");
        printf("\t\tadditional testing for these passthroughs if no other support is apparent.\n");
        printf("\t\t    sat - device supports SAT ATA-passthrough commands (12 or 16 byte)\n");
        printf("\t\t    legacyATA - device supports a legacy vendor unique method to passthrough\n");
        printf("\t\t                ATA commands. You must also specify --enableLegacyATAPTTest\n");
        printf("\t\t                in order for these commands to be tested.\n");
        //TODO: allow specifying specific legacy ATA passthrough types
        //TODO: NVMe vendor unique passthrough types
        printf("\n");
    }
}

//this option can be used to disable checking for NVMe or ATA passthrough capabilities.
#define DISABLE_PT_TESTING disablePassthroughTesting
#define DISABLE_PT_TESTING_VAR int DISABLE_PT_TESTING = 0;
#define DISABLE_PT_TESTING_LONG_OPT_STRING "disablePassthroughTesting"
#define DISABLE_PT_TESTING_LONG_OPT { DISABLE_PT_TESTING_LONG_OPT_STRING, no_argument, &DISABLE_PT_TESTING, 1 }

static void print_Disable_PT_Testing_Help(bool shortHelp)
{
    printf("\t--%s\n", DISABLE_PT_TESTING_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tDisables all ATA passthrough testing. Device will only be tested with SCSI commands from\n");
        printf("\t\tSPC and SBC specifications.\n\n");
    }
}
//enable legacy ATA testing
#define ENABLE_LEGACY_ATA_PT_TESTING enableLegacyATAPTTest
#define ENABLE_LEGACY_ATA_PT_TESTING_VAR int ENABLE_LEGACY_ATA_PT_TESTING = 0;
#define ENABLE_LEGACY_ATA_PT_TESTING_LONG_OPT_STRING "enableLegacyATAPTTest"
#define ENABLE_LEGACY_ATA_PT_TESTING_LONG_OPT { ENABLE_LEGACY_ATA_PT_TESTING_LONG_OPT_STRING, no_argument, &ENABLE_LEGACY_ATA_PT_TESTING, 1 }

static void print_Enable_Legacy_ATA_PT_Testing_Help(bool shortHelp)
{
    printf("\t--%s\n", ENABLE_LEGACY_ATA_PT_TESTING_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tAdd this option to the command line to allow sending legacy passthrough CDBs from\n");
        printf("\t\told USB adapters or drivers. By default these are off since these operation codes\n");
        printf("\t\tmay perform unintended actions on devices that don't actually support passthrough.\n");
        printf("\t\tFor example: There is a USB thumbdrive that receives one of these and bricks immediately.\n");
        printf("\t\tOnly add this to the command line if you understand and have high confidence that the\n");
        printf("\t\tdevice you are testing is likely to support one of these passthroughs. Most of these devices\n");
        printf("\t\twill have been created prior to 2006 when the SAT spec was added for a standardized passthrough.\n\n");
    }
}

//enable command tests that may hang the device
#define TEST_ZERO_LENGTH_READS zlr
#define TEST_SCT_STATUS_GPL sctGPL
#define TEST_RETURN_RESPONSE_NO_TDIR rrTdir
#define ENABLE_HANG_COMMANDS_TEST enableHangingCommandsTest
#define ENABLE_HANG_COMMANDS_TEST_VARS\
    int ENABLE_HANG_COMMANDS_TEST = 0;\
    int TEST_ZERO_LENGTH_READS = 0;\
    int TEST_SCT_STATUS_GPL = 0;\
    int TEST_RETURN_RESPONSE_NO_TDIR = 0;
    //Add more here if we run into other commands that hang some devices
#define ENABLE_HANG_COMMANDS_TEST_LONG_OPT_STRING "enableHangCmdsTest"
#define ENABLE_HANG_COMMANDS_TEST_LONG_OPT { ENABLE_HANG_COMMANDS_TEST_LONG_OPT_STRING, required_argument, NULL, 0 }

static void print_Enable_Hang_Commands_Test_Help(bool shortHelp)
{
    printf("\t--%s [all | zlr | sctgpl | rrTdir]\n", ENABLE_HANG_COMMANDS_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThere are some commands that are known to cause some translators\n");
        printf("\t\tto hang when they are received. This option covers some known commands for some devices\n");
        printf("\t\tThis must be enabled manually for a more complete test, but if it hangs the device could cause problems.\n");
        printf("\t\tEach of these tests has a specific hack/quirk related to it, so if the device hangs, then it must be added\n");
        printf("\t\tto the list for higher compatibility. Not all hangs will be able to be detected by the software!\n");
        printf("\t\tPut this option on the command line multiple times to add different combinations of tests.\n");
        printf("\t\t    all - run all known commands that may cause hangs\n");
        printf("\t\t    zlr - do SCSI read commands with zero transfer length\n");
        printf("\t\t    sctgpl - try reading the SCT status log with a GPL read log ext command\n");
        printf("\t\t    rrTdir - in the SAT return response information protocol, run it without setting the tdir bit as the spec allows\n");
        printf("\n");
    }
}

//force retesting the device even if it is already in the internal list
#define FORCE_RETEST forceDeviceRetest
#define FORCE_RETEST_VAR int FORCE_RETEST = 0;
#define FORCE_RETEST_LONG_OPT_STRING "forceRetest"
#define FORCE_RETEST_LONG_OPT { FORCE_RETEST_LONG_OPT_STRING, no_argument, &FORCE_RETEST, 1 }

static void print_Force_Retest_Help(bool shortHelp)
{
    printf("\t--%s\n", FORCE_RETEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to force this utility to retest this device if it reports\n");
        printf("\t\tthat it is already known. This may be useful when testing multiple devices\n");
        printf("\t\tthat use the same chip and are identified, or when trying to troubleshoot\n");
        printf("\t\tcompatibility on another OS.\n\n");
    }
}

static void print_Run_Passthrough_Test_Help(bool shortHelp)
{
    printf("\t--%s\n", RUN_PASSTHROUGH_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to perform the passthrough test.\n");
        printf("\t\tThe passthrough test is an attempt to figure out quirks or\n");
        printf("\t\thacks with different translators in order to make the device\n");
        printf("\t\tmore compatible with the rest of the openSeaChest software.\n");
        printf("\t\tThe default test is done using only what can be determined about\n");
        printf("\t\tthe device from the SCSI reported data. Attempting to passthrough\n");
        printf("\t\tATA or NVMe commands may not be done depending on how the device reports.\n");
        printf("\t\tUsing the other options can help tell this software to test for other\n");
        printf("\t\tbehavior. The other options to control the test are as follows:\n");
        printf("\t\t    --%s\n", PT_DRIVE_HINT_LONG_OPT_STRING);
        printf("\t\t    --%s\n", PT_PTTYPE_HINT_LONG_OPT_STRING);
        printf("\t\t    --%s\n", DISABLE_PT_TESTING_LONG_OPT_STRING);
        printf("\t\t    --%s\n", ENABLE_LEGACY_ATA_PT_TESTING_LONG_OPT_STRING);
        printf("\t\t    --%s\n", ENABLE_HANG_COMMANDS_TEST_LONG_OPT_STRING);
        printf("\t\t    --%s\n", FORCE_RETEST_LONG_OPT_STRING);
        printf("\n");
    }
}

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
    ENABLE_LEGACY_PASSTHROUGH_VAR
    //scan output flags
    SCAN_FLAGS_UTIL_VARS

    RUN_PASSTHROUGH_TEST_VAR
    PT_DRIVE_HINT_VAR
    PT_PTTYPE_HINT_VAR
    DISABLE_PT_TESTING_VAR
    ENABLE_LEGACY_ATA_PT_TESTING_VAR
    ENABLE_HANG_COMMANDS_TEST_VARS
    FORCE_RETEST_VAR

#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    LOWLEVEL_INFO_VAR

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
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        RUN_PASSTHROUGH_TEST_LONG_OPT,
        PT_DRIVE_HINT_LONG_OPT,
        PT_PTTYPE_HINT_LONG_OPT,
        DISABLE_PT_TESTING_LONG_OPT,
        ENABLE_LEGACY_ATA_PT_TESTING_LONG_OPT,
        ENABLE_HANG_COMMANDS_TEST_LONG_OPT,
        FORCE_RETEST_LONG_OPT,
        //tool specific options go here
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
            if (strncmp(longopts[optionIndex].name, PT_DRIVE_HINT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(PT_DRIVE_HINT_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "ata") == 0 || strcmp(optarg, "ATA") == 0 || strcmp(optarg, "sata") == 0 || strcmp(optarg, "SATA") == 0 || strcmp(optarg, "pata") == 0 || strcmp(optarg, "PATA") == 0 || strcmp(optarg, "ide") == 0 || strcmp(optarg, "IDE") == 0)
                {
                    PT_DRIVE_HINT = ATA_DRIVE;
                }
                else if (strcmp(optarg, "nvme") == 0 || strcmp(optarg, "NVME") == 0)
                {
                    PT_DRIVE_HINT = NVME_DRIVE;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(PT_DRIVE_HINT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, PT_PTTYPE_HINT_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(PT_PTTYPE_HINT_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "sat") == 0 || strcmp(optarg, "SAT") == 0)
                {
                    PT_PTTYPE_HINT = ATA_PASSTHROUGH_SAT;
                }
                else if (strcmp(optarg, "legacyATA") == 0) //TODO: add in ability to specify specific legacy ATA passthrough types
                {
                    //This means test all legacy USB ATA translations
                    PT_PTTYPE_HINT = 50;//setting this until I figure out a better way to do this
                }
                //TODO: Add vendor unique NVMe types here. Each one should be unique with some sort of generic value to say "test all of them"
                else
                {
                    print_Error_In_Cmd_Line_Args(PT_DRIVE_HINT_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, ENABLE_HANG_COMMANDS_TEST_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(ENABLE_HANG_COMMANDS_TEST_LONG_OPT_STRING))) == 0)
            {
                ENABLE_HANG_COMMANDS_TEST = 1;
                if (strcmp(optarg, "all") == 0)
                {
                    TEST_ZERO_LENGTH_READS = 1;
                    TEST_SCT_STATUS_GPL = 1;
                    TEST_RETURN_RESPONSE_NO_TDIR = 1;
                }
                else if (strcmp(optarg, "zlr") == 0)
                {
                    TEST_ZERO_LENGTH_READS = 1;
                }
                else if (strcmp(optarg, "sctgpl") == 0)
                {
                    TEST_SCT_STATUS_GPL = 1;
                }
                else if (strcmp(optarg, "rrTdir") == 0)
                {
                    TEST_RETURN_RESPONSE_NO_TDIR = 1;
                }
                else
                {
                    print_Error_In_Cmd_Line_Args(ENABLE_HANG_COMMANDS_TEST_LONG_OPT_STRING, optarg);
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
        || (FORCE_SCSI_FLAG && FORCE_NVME_FLAG)
        || (FORCE_ATA_FLAG && FORCE_NVME_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_SCSI_FLAG && (FORCE_ATA_PIO_FLAG || FORCE_ATA_DMA_FLAG || FORCE_ATA_UDMA_FLAG))//We may need to remove this. At least when software SAT gets used. (currently only Windows ATA passthrough and FreeBSD ATA passthrough)
        )
    {
        set_Console_Colors(true, ERROR_COLOR);
        printf("\nError: Only one force flag can be used at a time.\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
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
        || LOWLEVEL_INFO_FLAG
        //check for other tool specific options here
        || RUN_PASSTHROUGH_TEST_FLAG
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

    if (RUN_PASSTHROUGH_TEST_FLAG)
    {
        flags = OPEN_HANDLE_ONLY;
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

        if (LOWLEVEL_INFO_FLAG)
        {
            print_Low_Level_Info(&deviceList[deviceIter]);
        }

        if (TEST_UNIT_READY_FLAG)
        {
            show_Test_Unit_Ready_Status(&deviceList[deviceIter]);
        }

        if (RUN_PASSTHROUGH_TEST_FLAG)
        {
            //Need to allow a way to skip to certain parts of a test if the device has problems, or hangs, or anything like that.
            passthroughTestParams params;
            memset(&params, 0, sizeof(passthroughTestParams));
            params.device = &deviceList[deviceIter];

            params.allowLegacyATAPTTest = M_ToBool(ENABLE_LEGACY_ATA_PT_TESTING);
            params.testPotentiallyDeviceHangingCommands = M_ToBool(ENABLE_HANG_COMMANDS_TEST);
            if (ENABLE_HANG_COMMANDS_TEST)
            {
                params.hangCommandsToTest.returnResponseInforWithoutTdir = M_ToBool(TEST_RETURN_RESPONSE_NO_TDIR);
                params.hangCommandsToTest.sctLogWithGPL = M_ToBool(TEST_SCT_STATUS_GPL);
                params.hangCommandsToTest.zeroLengthReads = M_ToBool(TEST_ZERO_LENGTH_READS);
            }
            params.forceRetest = M_ToBool(FORCE_RETEST);
            params.disableTranslatorPassthroughTesting = M_ToBool(DISABLE_PT_TESTING);
            if (PT_DRIVE_HINT > 0)
            {
                params.suspectedDriveTypeProvidedByUser = true;
                params.suspectedDriveType = C_CAST(eDriveType, PT_DRIVE_HINT);
            }
            if (PT_PTTYPE_HINT > 0)
            {
                params.suspectedPassthroughProvidedByUser = true;
                //TODO: Handle value of 50 meaning test all legacy methods!
                params.suspectedPassthroughType = C_CAST(ePassthroughType, PT_PTTYPE_HINT);
            }
            perform_Passthrough_Test(&params);

        }
        //At this point, close the device handle since it is no longer needed. Do not put any further IO below this.
        close_Device(&deviceList[deviceIter]);
    }
    safe_Free(DEVICE_LIST);
    exit(exitCode);
}

static eReturnValues return_Response_Extend_Bit_Test(tDevice *device)
{
    eReturnValues ret = NOT_SUPPORTED;
    //Do additional command testing to check if the extend bit is reported correctly or not.
    //This obviously requires a 48bit drive and support for a command such as reading the native max LBA
    if (device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported)
    {
        uint64_t nativeMaxLBA = 0;
        eReturnValues nativeMaxRet = 0;
        bool commandIssued = true;
        //check HPA or AMAC support as those are preferred commands to be used.
        //The values that are returned should be greater than or equal to the current maxLBA of the drive.
        //Also, if the ext registers are 0xFF, then they were not reported correctly.
        if (device->drive_info.IdentifyData.ata.Word082 != 0xFFFF && device->drive_info.IdentifyData.ata.Word082 != 0 && device->drive_info.IdentifyData.ata.Word082 & BIT10)
        {
            //HPA
            nativeMaxRet = ata_Read_Native_Max_Address(device, &nativeMaxLBA, true);
        }
        else if (device->drive_info.IdentifyData.ata.Word119 != 0 && device->drive_info.IdentifyData.ata.Word119 & BIT14 && device->drive_info.IdentifyData.ata.Word119 & BIT8)
        {
            //AMAC
            nativeMaxRet = ata_Get_Native_Max_Address_Ext(device, &nativeMaxLBA);
        }
        else
        {
            //warning that this device doesn't support a featureset that can be used to figure out if the extend bit is reported properly.
            commandIssued = false;
            printf("WARNING: Unable to perform RTFR test for extend bit with return response information.\n");
            printf("         Please use a 48bit drive that supports either the HPA feature or AMAC feature\n");
            return NOT_SUPPORTED;
        }
        if (commandIssued)
        {
            if (nativeMaxRet == SUCCESS)
            {
                //This likely means we got everything, but need to confirm. It could also mean we think we got everything, but we could have missed ext RTFRs too.
                if (nativeMaxLBA >= device->drive_info.bridge_info.childDeviceMaxLba && device->drive_info.lastCommandRTFRs.secCntExt != 0xFF && device->drive_info.lastCommandRTFRs.lbaLowExt != 0xFF && device->drive_info.lastCommandRTFRs.lbaMidExt != 0xFF && device->drive_info.lastCommandRTFRs.lbaHiExt != 0xFF)
                {
                    if (device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit)
                    {
                        set_Console_Colors(true, HACK_COLOR);
                        printf("HACK FOUND: RSIE\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        printf("RTFRs returned correctly and extend bit appears to work properly.\n");
                    }
                    ret = SUCCESS;
                }
                else if (!device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit)
                {
                    //Extend bit may not be reporting correctly!
                    //Try enabling this and retry this test.
                    device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit = true;
                    device->drive_info.passThroughHacks.ataPTHacks.partialRTFRs = false;//Turn this off in case it was set earlier by another test.
                    ret = return_Response_Extend_Bit_Test(device);
                    if (ret != SUCCESS)
                    {
                        device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit = false;
                    }
                    return ret;
                }
                else
                {
                    printf("WARNING: This device is only able to return partial return task file registers.\n");
                    printf("         This means only 28bit commands will get full results. 48bit commands will be partial\n");
                    set_Console_Colors(true, HACK_COLOR);
                    printf("HACK FOUND: PARTRTFR\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    device->drive_info.passThroughHacks.ataPTHacks.partialRTFRs = true;
                }
            }
            else if (nativeMaxRet == WARN_INCOMPLETE_RFTRS)
            {
                //This COULD mean that the RTFRs didn't come back all the way, OR it could mean that the extend bit should be ignored for return response info
                if (!device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit)
                {
                    //Extend bit may not be reporting correctly!
                    //Try enabling this and retry this test.
                    device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit = true;
                    device->drive_info.passThroughHacks.ataPTHacks.partialRTFRs = false;//Turn this off in case it was set earlier by another test.
                    printf("Received warning the RTFRs are incomplete. Will retry ignoring the extend bit to see if this helps.\n");
                    ret = return_Response_Extend_Bit_Test(device);
                    if (ret != SUCCESS)
                    {
                        device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit = false;
                    }
                    return ret;
                }
                else
                {
                    printf("WARNING: This device is only able to return partial return task file registers.\n");
                    printf("         This means only 28bit commands will get full results. 48bit commands will be partial\n");
                    set_Console_Colors(true, HACK_COLOR);
                    printf("HACK FOUND: PARTRTFR\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    device->drive_info.passThroughHacks.ataPTHacks.partialRTFRs = true;
                }
            }
            else
            {
                //Something went really wrong
                set_Console_Colors(true, ERROR_COLOR);
                printf("ERROR: Command failure while trying to perform RTFR return response info test.\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                return FAILURE;
            }
        }
    }
    else
    {
        set_Console_Colors(true, NOTE_COLOR);
        printf("NOTE: This device doesn't support 48bit commands. Cannot test for proper extend bit in\n");
        printf("      return response information test. It is strongly recommended that this adapter is retested\n");
        printf("      with a device that supports 48bit commands so that this can be tested properly\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }
    return ret;
}

static void multi_Sector_PIO_Test_With_Logs(tDevice *device, bool gpl, uint8_t logAddress, uint32_t logSize)
{
    uint8_t *log = C_CAST(uint8_t*, calloc_aligned(logSize, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (log)
    {
        eReturnValues cmdResult = SUCCESS;
        fill_Random_Pattern_In_Buffer(log, logSize);
        if (gpl)
        {
            cmdResult = ata_Write_Log_Ext(device, logAddress, 0, log, logSize, false, false);
        }
        else
        {
            cmdResult = ata_SMART_Write_Log(device, logAddress, log, logSize, false);
        }

        if (cmdResult == SUCCESS)
        {
            //now we need to read and compare if the pattern changed!
            uint8_t *logR = C_CAST(uint8_t*, calloc_aligned(logSize, sizeof(uint8_t), device->os_info.minimumAlignment));
            if (logR)
            {
                if (gpl)
                {
                    cmdResult = ata_Read_Log_Ext(device, logAddress, 0, logR, logSize, false, 0);
                }
                else
                {
                    cmdResult = ata_SMART_Read_Log(device, logAddress, logR, logSize);
                }
                if (cmdResult == SUCCESS)
                {
                    if (memcmp(log, logR, logSize) == 0)
                    {
                        //this worked!!!
                        if (device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
                        {
                            set_Console_Colors(true, HACK_COLOR);
                            printf("HACK FOUND: MMPIO\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        else
                        {
                            printf("Multi-sector PIO commands work properly!\n");
                        }
                    }
                    else
                    {
                        if (device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
                        {
                            device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                            set_Console_Colors(true, HACK_COLOR);
                            printf("HACK FOUND: SPIO\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                            device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
                        }
                        else
                        {
                            if (!device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode && M_Byte1(device->drive_info.IdentifyData.ata.Word047) == 0x80 && M_Byte0(device->drive_info.IdentifyData.ata.Word047) > 0)
                            {
                                printf("Retrying after changing the multiple mode.\n");
                                if (SUCCESS == ata_Set_Multiple_Mode(device, M_Byte0(device->drive_info.IdentifyData.ata.Word047)))
                                {
                                    device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = true;
                                    //recursively call this function and try again
                                    multi_Sector_PIO_Test_With_Logs(device, gpl, logAddress, logSize);
                                }
                                else
                                {
                                    printf("Unable to set the multiple mode. Aborting multi-sector PIO test\n");
                                    device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                                }
                            }
                            else if (device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
                            {
                                device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                                printf("Unable to perform test of multi-sector PIO commands\n");
                            }
                            else
                            {
                                set_Console_Colors(true, HACK_COLOR);
                                printf("HACK FOUND: SPIO\n");
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                                device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
                            }
                        }
                    }
                }
                else
                {
                    //Something went wrong
                    printf("Unable to read back the log that was written!\n");
                    //is this a different error or multisector PIO issue?
                }
                //clear the log back out to zeros
                memset(log, 0, logSize);
                if (gpl)
                {
                    cmdResult = ata_Write_Log_Ext(device, logAddress, 0, log, logSize, false, false);
                }
                else
                {
                    cmdResult = ata_SMART_Write_Log(device, logAddress, log, logSize, false);
                }
                safe_Free_aligned(logR)
            }
            else
            {
                set_Console_Colors(true, ERROR_COLOR);
                printf("FATAL ERROR: Unable to allocate enough memory to finish multi-sector PIO test!\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            //clear the log back to zeros if necessary
        }
        else
        {
            if (!device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode && M_Byte1(device->drive_info.IdentifyData.ata.Word047) == 0x80 && M_Byte0(device->drive_info.IdentifyData.ata.Word047) > 0)
            {
                printf("Retrying after changing the multiple mode.\n");
                if (SUCCESS == ata_Set_Multiple_Mode(device, M_Byte0(device->drive_info.IdentifyData.ata.Word047)))
                {
                    device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = true;
                    //recursively call this function and try again
                    safe_Free_aligned(log)
                        multi_Sector_PIO_Test_With_Logs(device, gpl, logAddress, logSize);
                }
                else
                {
                    printf("Unable to set the multiple mode. Aborting multi-sector PIO test\n");
                    device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                }
            }
            else if (device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
            {
                device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                printf("Unable to perform test of multi-sector PIO commands\n");
            }
            else
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: SPIO\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
            }
        }
        safe_Free_aligned(log)
    }
}

//TODO: Use this function to do a read and write test to compare results in different sectors.
//      will only be needed if unable to use logs method below
// static void multi_Sector_PIO_Test_With_Read_Write(M_ATTR_UNUSED tDevice *device)
// {
//     return;
// }

static void multi_Sector_PIO_Test(tDevice *device, bool smartSupported, bool smartLoggingSupported)
{
    printf("Checking multi-sector PIO command support\n");
    bool foundMultiSectorLogPage = false;
    uint8_t logDir[512] = { 0 };
    //Attempt read log commands first since they aren't touching media
    if (device->drive_info.ata_Options.generalPurposeLoggingSupported)
    {
        if (SUCCESS == ata_Read_Log_Ext(device, ATA_LOG_DIRECTORY, 0, logDir, 512, false, 0))
        {
            uint8_t logAddress = 0;
            //First check the Host log addresses and find an empty one.
            //Attempt to write the whole thing with a pattern or random data, then read it back.
            //This should tell whether all the data made it through or not.
            //If this worked right away, then everything is working as expected and no hacks are necesssary
            //If not, try setting the multiple mode to the highest supported by the drive, then try again and see if that changes the behavior.
            //    If the behavior changed and worked, then we know multisector PIO is possible up to the maximum multiple mode configuration
            for (uint16_t iter = ATA_LOG_HOST_SPECIFIC_80H * 2; iter < 512 && iter <= ATA_LOG_HOST_SPECIFIC_9FH * 2; iter += 2)
            {
                uint16_t logSize = M_BytesTo2ByteValue(logDir[iter + 1], logDir[iter]);
                logAddress = C_CAST(uint8_t, iter / 2);
                if (logSize > 0)
                {
                    uint32_t allocedLogSize = C_CAST(uint32_t, logSize) * UINT32_C(512);
                    uint8_t *log = C_CAST(uint8_t *, calloc_aligned(allocedLogSize, sizeof(uint8_t), device->os_info.minimumAlignment));
                    if (log)
                    {
                        if (SUCCESS == ata_Read_Log_Ext(device, logAddress, 0, log, allocedLogSize, false, 0))
                        {
                            //now check if it's empty so we don't overwrite any data in it.
                            if (is_Empty(log, allocedLogSize))
                            {
                                safe_Free_aligned(log)
                                multi_Sector_PIO_Test_With_Logs(device, true, C_CAST(uint8_t, iter / 2), allocedLogSize);
                                break;
                            }
                            safe_Free_aligned(log)
                        }
                        else
                        {
                            safe_Free_aligned(log)
                            printf("WARNING: Failed to read multi-sector log with PIO commands. Likely a chip not compliant with multisector PIO commands\n");
                            if (!device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode && M_Byte0(device->drive_info.IdentifyData.ata.Word047) > 0)
                            {
                                uint8_t drqBlocksToSet = M_Byte0(device->drive_info.IdentifyData.ata.Word047);
                                uint8_t currentDRQblocks = M_Byte0(device->drive_info.IdentifyData.ata.Word059);//todo, bit 8 is a validity bit
                                if (device->drive_info.IdentifyData.ata.Word059 & BIT8 && currentDRQblocks == drqBlocksToSet)
                                {
                                    //already at the max multiple mode supported, so don't bother setting it, this has already been tested for now.
                                    //Instead, try changing this to a value of 1 to see if changing this to a different value works around the failure seen so far.
                                    drqBlocksToSet = 1;
                                }
                                printf("Retrying after changing the multiple mode.\n");
                                if (SUCCESS == ata_Set_Multiple_Mode(device, drqBlocksToSet))
                                {
                                    device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = true;
                                    //recursively call this function and try again
                                    multi_Sector_PIO_Test(device, smartSupported, smartLoggingSupported);
                                    if (device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    printf("Unable to set the multiple mode. Aborting multi-sector PIO test\n");
                                    //restore the multiple mode back to what the adapter originally configured to be safe that it's in a good state.
                                    ata_Set_Multiple_Mode(device, currentDRQblocks);
                                    device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                                    set_Console_Colors(true, LIKELY_HACK_COLOR);
                                    printf("HACK FOUND: SPIO\n");
                                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                                    device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
                                }
                            }
                            else if (device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
                            {
                                device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                                printf("Unable to perform test of multi-sector PIO commands\n");
                                set_Console_Colors(true, HACK_COLOR);
                                printf("HACK FOUND: SPIO\n");
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                                device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
                                break;
                            }
                            else
                            {
                                set_Console_Colors(true, HACK_COLOR);
                                printf("HACK FOUND: SPIO\n");
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                                device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
                                break;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    else if (smartSupported && smartLoggingSupported)
    {
        if (SUCCESS == ata_SMART_Read_Log(device, ATA_LOG_DIRECTORY, logDir, 512))
        {
            uint8_t logAddress = 0;
            //First check the Host log addresses and find an empty one.
            //Attempt to write the whole thing with a pattern or random data, then read it back.
            //This should tell whether all the data made it through or not.
            //If this worked right away, then everything is working as expected and no hacks are necesssary
            //If not, try setting the multiple mode to the highest supported by the drive, then try again and see if that changes the behavior.
            //    If the behavior changed and worked, then we know multisector PIO is possible up to the maximum multiple mode configuration
            for (uint16_t iter = 0x80 * 2; iter < 512 && iter <= 0x9F * 2; iter += 2)
            {
                uint16_t logSize = M_BytesTo2ByteValue(logDir[iter + 1], logDir[iter]);
                logAddress = C_CAST(uint8_t, iter / 2);
                if (logSize > 0)
                {
                    uint32_t allocedLogSize = C_CAST(uint32_t, logSize) * UINT32_C(512);
                    uint8_t *log = C_CAST(uint8_t *, calloc_aligned(allocedLogSize, sizeof(uint8_t), device->os_info.minimumAlignment));
                    if (log)
                    {
                        if (SUCCESS == ata_SMART_Read_Log(device, logAddress, log, allocedLogSize))
                        {
                            //now check if it's empty so we don't overwrite any data in it.
                            if (is_Empty(log, allocedLogSize))
                            {
                                safe_Free_aligned(log)
                                multi_Sector_PIO_Test_With_Logs(device, false, C_CAST(uint8_t, iter / 2), allocedLogSize);
                                break;
                            }
                            safe_Free_aligned(log)
                        }
                        else
                        {
                            safe_Free_aligned(log)
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Failed to read multi-sector log with PIO commands. Likely a chip not compliant with multisector PIO commands\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                            if (!device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode && M_Byte1(device->drive_info.IdentifyData.ata.Word047) == 0x80 && M_Byte0(device->drive_info.IdentifyData.ata.Word047) > 0)
                            {
                                printf("Retrying after changing the multiple mode.\n");
                                if (SUCCESS == ata_Set_Multiple_Mode(device, M_Byte0(device->drive_info.IdentifyData.ata.Word047)))
                                {
                                    device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = true;
                                    //recursively call this function and try again
                                    multi_Sector_PIO_Test(device, smartSupported, smartLoggingSupported);
                                    if (device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    printf("Unable to set the multiple mode. Aborting multi-sector PIO test\n");
                                    device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                                }
                            }
                            else if (device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
                            {
                                device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode = false;
                                printf("Unable to perform test of multi-sector PIO commands\n");
                                set_Console_Colors(true, HACK_COLOR);
                                printf("HACK FOUND: SPIO\n");
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                                device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
                                break;
                            }
                            else
                            {
                                set_Console_Colors(true, HACK_COLOR);
                                printf("HACK FOUND: SPIO\n");
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                                device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    if (!foundMultiSectorLogPage)//Or should this be the default if allowing data destructive testing flag is given???
    {
        //If these aren't available or there aren't any multi-sector logs, then will need to use read and write PIO commands. Since these are potentially data-damaging, this will need a flag.
        //multi_Sector_PIO_Test_With_Read_Write(device);
    }
    return;
}

static void sat_DMA_UDMA_Protocol_Test(tDevice *device, M_ATTR_UNUSED bool smartSupported, M_ATTR_UNUSED bool smartLoggingSupported)
{
    //Attempt UDMA mode first
    uint64_t lba = 0;
    uint16_t sectors = 1;
    uint32_t dataSize = device->drive_info.bridge_info.childDeviceBlockSize * sectors;
    uint8_t *ptrData = C_CAST(uint8_t *, calloc_aligned(dataSize, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (ptrData)
    {
        bool use48 = device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported;
        if (device->drive_info.passThroughHacks.ataPTHacks.ata28BitOnly)
        {
            use48 = false;
        }
        device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_DMA;
        eReturnValues dmaReadRet = ata_Read_DMA(device, lba, ptrData, sectors, dataSize, use48);
        device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_UDMA;
        eReturnValues udmaReadRet = ata_Read_DMA(device, lba, ptrData, sectors, dataSize, use48);

        if (dmaReadRet != SUCCESS && udmaReadRet != SUCCESS)
        {
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: NDMA\n");//Cannot issue any DMA mode commands.
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            set_Console_Colors(true, ERROR_COLOR);
            printf("ERROR: No possible way to issue a DMA mode command on this device.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_NO_DMA;
        }
        else
        {
            if (udmaReadRet != SUCCESS && dmaReadRet == SUCCESS)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: FDMA\n");//force using DMA mode instead of UDMA
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_DMA;
            }
            else if (udmaReadRet == SUCCESS && dmaReadRet != SUCCESS)
            {
                set_Console_Colors(true, NOTE_COLOR);
                printf("NOTE: Device only supports UDMA mode for passthrough. This is the default for the tool, but may be an issue for some other tools.\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_UDMA;
            }
            else if (udmaReadRet == SUCCESS && dmaReadRet == SUCCESS)
            {
                printf("Both UDMA and DMA protocols are supported.\n");
            }
            else
            {
                set_Console_Colors(true, ERROR_COLOR);
                printf("FATAL ERROR: You shouldn't be seeing this message. Something went horribly wrong\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
        }
        safe_Free_aligned(ptrData)
    }
    else
    {
        set_Console_Colors(true, ERROR_COLOR);
        printf("ERROR: Unable to allocate memory for DMA/UDMA protocol test\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }
    return;
}

static void check_Condition_Bit_Test(tDevice *device, bool smartSupported, bool smartLoggingSupported)
{
    //Test check condition bit
    printf("Testing for check condition bit support on any command\n");
    device->drive_info.passThroughHacks.ataPTHacks.alwaysCheckConditionAvailable = true;
    //try identify first...if this doesn't even work, then we know this doesn't work
    uint8_t identifyData[512] = { 0 };
    eReturnValues satRet = ata_Identify(device, identifyData, 512);
    if ((SUCCESS == satRet || WARN_INVALID_CHECKSUM == satRet) && !is_Empty(identifyData, 512))
    {
        bool testedGPL = false, testedSMART = false;
        bool gotValidRTFRsGPL = false, gotValidRTFRsSMART = false;
        //since this at least worked, try some other commands.
        //Try other things like read log (single page) and read log (multiple pages)...need to try a multipage log from GPL or SMART. Doesn't matter which one.
        if (device->drive_info.ata_Options.generalPurposeLoggingSupported)
        {
            //GPL
            //try reading the GPL directory, then read the first multi-page log available to figure out if multiple pages are OK with the check condition bit
            testedGPL = true;
        }
        if (smartSupported)
        {
            //SMART is available.
            //Try SMART read data
            uint8_t *smartData = C_CAST(uint8_t*, calloc_aligned(512, sizeof(uint8_t), device->os_info.minimumAlignment));
            memset(&device->drive_info.lastCommandRTFRs, 0, sizeof(ataReturnTFRs));
            if (smartData)
            {
                eReturnValues smartRet = ata_SMART_Read_Data(device, smartData, 512);
                if (SUCCESS == smartRet || WARN_INVALID_CHECKSUM == smartRet)
                {
                    //Make sure the registers show successful completion and not zeros
                    if (device->drive_info.lastCommandRTFRs.status != 0 && !(ATA_STATUS_BIT_ERROR & device->drive_info.lastCommandRTFRs.status)
                        &&
                        device->drive_info.lastCommandRTFRs.error == 0)
                    {
                        //check condition seems to work for a single sector
                        gotValidRTFRsSMART = true;
                    }
                }
                memset(&device->drive_info.lastCommandRTFRs, 0, sizeof(ataReturnTFRs));
                //If logging is available, try SMART read log for a multiple page log if possible.
                if (smartLoggingSupported)
                {
                    //Logging is available. Try SMART read log commands for a log page with multiple sectors
                    memset(smartData, 0, 512);
                    smartRet = ata_SMART_Read_Log(device, 0, smartData, 512);
                    if (SUCCESS == smartRet || WARN_INVALID_CHECKSUM == smartRet)
                    {
                        //find a multipage log and attempt to read it...try multiple logs if necessary.
                    }
                }
                //Lastly, try a SMART return status command. This SHOULD
                safe_Free_aligned(smartData)
                testedSMART = true;
            }
        }
        if (!testedGPL && !testedSMART)
        {
            printf("WARNING: Neither GPL or SMART were available to test. Please retest with a drive that supports at least\n");
            printf("         One of these features\n");
            device->drive_info.passThroughHacks.ataPTHacks.alwaysCheckConditionAvailable = false;
        }
        else if (gotValidRTFRsGPL || gotValidRTFRsSMART)
        {
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: CHK\n");//check condition bit is always supported properly
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
        else
        {
            device->drive_info.passThroughHacks.ataPTHacks.alwaysCheckConditionAvailable = false;
            //test if check condition bit works on non-data properly...which generally is true.
            uint8_t pm = 0;
            if (SUCCESS == ata_Check_Power_Mode(device, &pm))
            {
                //need to see status set for success
                if (device->drive_info.lastCommandRTFRs.status != 0 && !(ATA_STATUS_BIT_ERROR & device->drive_info.lastCommandRTFRs.status)
                    &&
                    device->drive_info.lastCommandRTFRs.error == 0)
                {
                    set_Console_Colors(true, HACK_COLOR);
                    printf("HACK FOUND: CHKND\n");//check condition bit is supported for non-data commands only
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    set_Console_Colors(true, HACK_COLOR);
                    printf("HACK FOUND: CHKE\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    device->drive_info.passThroughHacks.ataPTHacks.checkConditionEmpty = true;
                }
            }
        }
    }
    else
    {
        device->drive_info.passThroughHacks.ataPTHacks.alwaysCheckConditionAvailable = false;
        //test a non-data command to make sure check condition bit DOES work
        uint8_t pm = 0;
        if (SUCCESS == ata_Check_Power_Mode(device, &pm))
        {
            //need to see status set for success
            if (device->drive_info.lastCommandRTFRs.status != 0 && !(ATA_STATUS_BIT_ERROR & device->drive_info.lastCommandRTFRs.status)
                &&
                device->drive_info.lastCommandRTFRs.error == 0)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: CHKND\n");//check condition bit is supported for non-data commands only
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
        }
        else
        {
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: NCHK\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            device->drive_info.passThroughHacks.ataPTHacks.disableCheckCondition = true;
        }
    }
}

#include "sat_helper_func.h"
static void return_Response_Info_Test(tDevice *device, M_ATTR_UNUSED bool smartSupported, M_ATTR_UNUSED bool smartLoggingSupported, bool testWithoutTDirAllowed)
{
    printf("Testing for support of Return Response Info protocol\n");
    //Test return response information - TODO: May need to try issuing some command before we do this test.
    device->drive_info.passThroughHacks.ataPTHacks.returnResponseInfoNeedsTDIR = true;//Testing with this on FIRST because it is likely more compatible.
    if (SUCCESS == request_Return_TFRs_From_Device(device, &device->drive_info.lastCommandRTFRs))
    {
        device->drive_info.passThroughHacks.ataPTHacks.returnResponseInfoSupported = true;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: RS\n");//returnResponseInfoSupported
        printf("HACK FOUND: RSTD\n");//returnResponseInfoNeedsTDIR
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }
    else if (testWithoutTDirAllowed)
    {
        device->drive_info.passThroughHacks.ataPTHacks.returnResponseInfoNeedsTDIR = false;
        //TODO: send test unit ready to clear any errors..this may need to loop and retry up to 30 seconds for watchdog timers to kick in and reset the device.
        if (SUCCESS == request_Return_TFRs_From_Device(device, &device->drive_info.lastCommandRTFRs))
        {
            device->drive_info.passThroughHacks.ataPTHacks.returnResponseInfoSupported = true;
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: RS\n");//returnResponseInfoSupported
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);

        }
    }
    if (device->drive_info.passThroughHacks.ataPTHacks.returnResponseInfoSupported)
    {
        //call this function for the test since it can be called recursively to retry the test if it needs to be.
        bool partRTFR = device->drive_info.passThroughHacks.ataPTHacks.partialRTFRs;
        return_Response_Extend_Bit_Test(device);
        if (partRTFR && !device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit)
        {
            device->drive_info.passThroughHacks.ataPTHacks.partialRTFRs = partRTFR;
        }
    }
}

typedef struct _rawDesignator
{
    bool valid;
    uint8_t designatorType;
    uint8_t designator[16];
    uint16_t designatorLength;
}rawDesignator;

typedef struct _scsiDevInfo
{
    //inquiry data
    struct
    {
        char vendorId[T10_VENDOR_ID_LEN + 1];
        char productId[INQ_DATA_PRODUCT_ID_LEN + 1];
        char productRev[INQ_DATA_PRODUCT_REV_LEN + 1];
        uint8_t peripheralDeviceType;
        uint8_t scsiVersion;
        uint8_t responseFormat;//inq response format. Can help understand if above information came from somewhere else or not
        uint16_t versionDescriptors[8];
        bool rmb;
        bool satVersionDescriptorFound;
    }inquiryData;
    //read capacity data - zeros mean we couldn't read one of these pieces of information.
    struct
    {
        uint32_t rc10MaxLBA;
        uint32_t rc10blockSize;
        uint64_t rc16MaxLBA;
        uint32_t rc16blockSize;
        uint32_t rc16physBlockSize;
    }readCapData;
    //vpd page data
    struct
    {
        bool gotUnitSNVPDPage;
        char unitSN[255];
        bool gotDeviceIDVPDPage;
        uint8_t designatorCount;
        rawDesignator designators[8];
        bool gotSATVPDPage;
        bool gotPowerConditionsVPDPage;
        struct
        {
            bool standby_z;
            bool standby_y;
            bool idle_a;
            bool idle_b;
            bool idle_c;
            uint16_t stansbyZTimer;
            uint16_t stansbyYTimer;
            uint16_t idleATimer;
            uint16_t idleBTimer;
            uint16_t idleCTimer;
        }powerConditionData;
        bool gotExInquiryVPDPage;
        uint16_t vpdDSTTimeMinutes;//ext dst from VPD page. If zero, either not reported on this page, or device just doesn't do this translation
        bool gotBlockCharacteristicsVPDPage;
        struct
        {
            uint16_t rotationRate;
            uint8_t productType;
            uint8_t formFactor;
            uint8_t zonedCapabilities;
        }blockCharacteristicsData;
        bool gotBlockLimitsVPDPage;
        struct
        {
            uint32_t maximumXferLen;
            uint32_t optimalXferLen;
        }blockLimitsData;
        //TODO: unmap lengths
        bool gotLogicalBlockProvisioningVPDPage;
        bool unmapSupported;
        //TODO: based on reported VPD pages (mode page policy and/or feature sets, etc), create a list of mode pages that are expected to be supported for the mode page test.
    }vpdData;
    //Log Page data
    struct
    {
        bool readErrorsFound;
        struct
        {
            union {
                uint32_t readRecoveryAttempts;//ATA
                uint64_t totalTimesCorrectionAlgorithmProcessed;//SCSI
            };
            union {
                uint32_t numberOfReportedUncorrectableErrors;//ATA
                uint64_t totalUncorrectedErrors;//SCSI
            };
        }readErrorData;
        bool gotTemperature;
        struct
        {
            uint8_t temperature;
            uint8_t referenceTemperature;
        }temperatureData;
        bool gotStartStop;
        struct
        {
            union {
                uint32_t headLoadEvents;
                uint32_t accumulatedStartStopCycles;
            };
            union {
                uint32_t headLoadEvents2;
                uint32_t accumulatedLoadUnloadCycles;
            };
        }startStopData;
        bool gotSelfTestData;
        struct
        {
            uint32_t totalSelfTestEntries;//Only doing this because it is more complicated to compare the fields with ATA and is not really worth it right now - TJE
        }selfTestData;
        bool gotSolidStateData;
        struct
        {
            uint8_t percentUsedIndicator;
        }solidStateData;
        bool gotBackgroundScan;
        struct
        {
            uint64_t accumulatedPowerOnMinutes;//60 * ATA Power on hours statistic
        }backgroundData;
        bool gotZonedStatistics;
        struct
        {
            //TODO: zoned statistics
            uint8_t reserved;
        }zonedData;
        bool gotGeneralStatistics;
        struct
        {
            uint64_t numberOfReadCommands;
            uint64_t numberOfWriteCommands;
            uint64_t numberOfLogicalBlocksReceived;
            uint64_t numberOfLogicalBlocksTransmitted;
        }generalStatisticsData;
        bool gotPendingDefects;
        struct
        {
            uint32_t pendingDefectsCount;
        }pendingDefectsData;
        bool gotInformationalExceptions;
        struct
        {
            uint8_t asc;
            uint8_t ascq;
            uint8_t mostRecentTemperatureReading;
        }informationalExceptionsData;
    }logData;
    //Mode Page data - all of these are in SAT5 or STNI (NVMe translation) minus control extension for NVMe
    //TODO: only looking for current pages now. Should we also check default, saved, and changable???? - TJE
    struct
    {
        bool gotControlPage;
        struct
        {
            bool d_sense;
            uint16_t extDSTCompletionTimeSeconds;//Seconds
            //TODO: busy timeout period???
        }controlData;
        bool gotControlExtensionPage;//not in NVMe
        struct
        {
            uint8_t maxSenseDataLength;//what is it currently set to here...can also be changed here.
        }controlExtData;
        bool gotReadWriteErrorRecoveryPage;
        struct
        {
            bool awre;//true for everything today. Possibly false if it is an old drive....can tell by checking defaults instead of current values.
            bool arre;//false for ATA, true for NVMe
            uint16_t recoveryTimeLimit;//0 for all ATA devices according to SAT. NVMe will set this to value of a feature.
        }rwErrRecData;
        bool gotCachingPage;
        struct
        {
            bool wce;
            bool dra;//not on NVMe ever.
        }cachingData;
        bool gotInformationalExceptionsControlPage;
        struct
        {
            uint8_t mrie;
            bool ewasc;
            bool dexcpt;
            bool perf;
        }infoExcepData;
        bool gotRigidDiskPage;
        struct
        {
            //Storing everything from this page for now.
            uint32_t numberOfCylinders;
            uint8_t numberOfHeads;
            uint32_t startingCyliderWritePrecompensation;
            uint32_t startingCylinerReducedWriteCurrent;
            uint16_t driveStepRate;
            uint32_t landingZoneCylinder;
            uint8_t rpl;
            uint8_t rotationalOffset;
            uint16_t mediumRotationRate;
        }rigidDiskData;
        bool gotPowerConditionControlPage;//StdSCSI page
        struct
        {
            uint8_t pageLength;//to help detect legacy devices since this used to be a shorter page
            bool standbyY;
            bool standbyZ;
            bool idleA;
            bool idleB;
            bool idleC;
            uint32_t idleATimer;
            uint32_t standbyZTimer;
            //If pagelength is less than 26h, then nothing below here is going to be set
            uint32_t idleBTimer;
            uint32_t idleCTimer;
            uint32_t standbyYTimer;
        }powerConditionCtrlData;
        bool gotPataControlPage;//ATA specific for PATA devices.
        struct
        {
            bool mwd2;
            bool mwd1;
            bool mwd0;
            bool pio4;
            bool pio3;
            bool udma6;
            bool udma5;
            bool udma4;
            bool udma3;
            bool udma2;
            bool udma1;
            bool udma0;
        }pataCtrlData;
        bool gotATAPowerConditionPage;//ATA specific page for APM
        struct
        {
            bool apmp;
            uint8_t apmValue;
        }ataPwrConditionData;
        bool gotVendorUniquePage0;
    }modeData;

}scsiDevInformation, *ptrScsiDevInformation;

static void scsi_VPD_Pages(tDevice *device, ptrScsiDevInformation scsiDevInfo)
{
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n=========================\n");
    printf("Checking VPD page support\n");
    printf("=========================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);

    uint8_t supportedPages[36] = { 0 };
    uint8_t dummiedPageCount = 0;
    bool dummiedPages = false;
    if (SUCCESS != scsi_Inquiry(device, supportedPages, 30, SUPPORTED_VPD_PAGES, true, false))
    {
        dummiedPages = true;
        set_Console_Colors(true, WARNING_COLOR);
        printf("WARNING: Device did not report supported VPD pages.\n");
        printf("         VPD pages communicate other device capabilities\n");
        printf("         and limits which are useful to the host.\n");
        printf("Will dummy up support to see if any pages are supported that are useful.\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        memset(supportedPages, 0, 36);
        uint16_t offset = 4;//start of pages to dummy up
        //in here we will set up a fake supported VPD pages buffer so that we try to read the unit serial number page, the SAT page, and device identification page
        supportedPages[0] = 0 << 5;//todo put the proper qualifier in here!
        supportedPages[0] |= scsiDevInfo->inquiryData.peripheralDeviceType;
        //set page code
        supportedPages[1] = 0x00;
        //Don't dummy up this page since it already didn't work
//              supportedPages[offset] = SUPPORTED_VPD_PAGES;
//              ++offset;
        supportedPages[offset] = UNIT_SERIAL_NUMBER;
        ++offset;
        ++dummiedPageCount;
        //if (version >= 3)//SPC
        {
            supportedPages[offset] = DEVICE_IDENTIFICATION;
            ++offset;
            ++dummiedPageCount;
        }
        supportedPages[offset] = EXTENDED_INQUIRY_DATA;
        ++offset;
        ++dummiedPageCount;
        if (scsiDevInfo->inquiryData.satVersionDescriptorFound)
        {
            supportedPages[offset] = ATA_INFORMATION;
            ++offset;
            ++dummiedPageCount;
        }
        //if (version >= 3)//SPC
        {
            if (scsiDevInfo->inquiryData.peripheralDeviceType == PERIPHERAL_DIRECT_ACCESS_BLOCK_DEVICE || scsiDevInfo->inquiryData.peripheralDeviceType == PERIPHERAL_SIMPLIFIED_DIRECT_ACCESS_DEVICE || scsiDevInfo->inquiryData.peripheralDeviceType == PERIPHERAL_HOST_MANAGED_ZONED_BLOCK_DEVICE)
            {
                supportedPages[offset] = BLOCK_LIMITS;
                ++offset;
                ++dummiedPageCount;
                supportedPages[offset] = BLOCK_DEVICE_CHARACTERISTICS;
                ++offset;
                ++dummiedPageCount;
            }
        }
        //TODO: dummy up other page support here
        //set page length (n-3)
        supportedPages[2] = M_Byte1(offset - 4);//msb
        supportedPages[3] = M_Byte0(offset - 4);//lsb
    }
    //Now attempt to read VPD pages. - Warn about any missing MANDATORY pages
    //definitely strong warning if SAT VPD page is missing when it was expected. This warning should be emitted later if we do SAT passthrough testing as well...try to trust what the device does report
    //If supported VPD pages is missing, attempt to read unit serial number and device identification and block limits pages anyways...report warnings if ANY of these work, but supported pages was missing.
    uint16_t supportedVPDPagesLength = M_BytesTo2ByteValue(supportedPages[2], supportedPages[3]);
    if ((supportedVPDPagesLength + 4) > INQ_RETURN_DATA_LENGTH)
    {
        set_Console_Colors(true, WARNING_COLOR);
        printf("WARNING: Supported VPD pages length seems suspiciously large!\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }
    uint16_t pagesread = 0;
    //TODO: validate peripheral qualifier and peripheral device type on every page with std inquiry data
    for (uint32_t vpdIter = 4; vpdIter < C_CAST(uint32_t, (supportedVPDPagesLength + 4)) && vpdIter < INQ_RETURN_DATA_LENGTH; vpdIter++)
    {
        bool genericVPDPageReadOutput = true;
        bool readVPDPage = false;
        uint8_t *pageToRead = C_CAST(uint8_t*, calloc_aligned(4, sizeof(uint8_t), device->os_info.minimumAlignment));
        uint16_t vpdPageLength = 0;
        printf("\tFound page %" PRIX8 "h\n", supportedPages[vpdIter]);

        if (SUCCESS == scsi_Inquiry(device, pageToRead, 4, supportedPages[vpdIter], true, false))
        {
            vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
            uint8_t* temp = realloc_aligned(pageToRead, 4, vpdPageLength + 4, device->os_info.minimumAlignment);
            if (temp)
            {
                pageToRead = temp;
                if (SUCCESS == scsi_Inquiry(device, pageToRead, vpdPageLength + 4, supportedPages[vpdIter], true, false))
                {
                    genericVPDPageReadOutput = false;
                    vpdPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                    //TODO: Also validate the peripheral qualifier and peripheral device type???
                    if (pageToRead[1] != supportedPages[vpdIter])
                    {
                        set_Console_Colors(true, ERROR_COLOR);
                        printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", supportedPages[vpdIter], pageToRead[1]);
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        readVPDPage = true;
                        ++pagesread;
                    }
                }
                else if (!dummiedPages)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
                    printf("       Attempted to read %" PRIu16 " bytes as reported by device.\n", vpdPageLength);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }
            else
            {
                set_Console_Colors(true, ERROR_COLOR);
                printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page %" PRIX8 "h\n", vpdPageLength, supportedPages[vpdIter]);
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
        }
        else if (!dummiedPages)
        {
            set_Console_Colors(true, ERROR_COLOR);
            printf("ERROR: Failed to read page %" PRIX8 "h. Was reported as supported, but cannot be read.\n", supportedPages[vpdIter]);
            printf("       Attempted to read only first 4 bytes to determine full VPD page size as spec allows.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
        switch (supportedPages[vpdIter])
        {
        case SUPPORTED_VPD_PAGES:
            printf("Supported VPD Pages\n");
            break;
        case UNIT_SERIAL_NUMBER:
            printf("Unit Serial Number VPD Page\n");
            if (readVPDPage)
            {
                if (dummiedPages)
                {
                    set_Console_Colors(true, HACK_COLOR);
                    printf("HACK FOUND: UNA\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    device->drive_info.passThroughHacks.scsiHacks.unitSNAvailable = true;
                }
                char *unitSerialNumber = C_CAST(char *, calloc(vpdPageLength + 1, sizeof(char))); //add 1 for NULL terminator
                if (unitSerialNumber)
                {
                    scsiDevInfo->vpdData.gotUnitSNVPDPage = true;
                    memcpy(unitSerialNumber, &pageToRead[4], vpdPageLength);
                    for (uint16_t iter = 0; iter < vpdPageLength && iter < UINT16_MAX; ++iter)
                    {
                        if (!is_ASCII(unitSerialNumber[iter]) || !isprint(unitSerialNumber[iter]))
                        {
                            unitSerialNumber[iter] = ' ';
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Unit Serial Number contains non-ASCII or non-Printable Characters!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                    }
                    if (strlen(unitSerialNumber) == 0)
                    {
                        set_Console_Colors(true, WARNING_COLOR);
                        printf("WARNING: Unit Serial Number is empty!\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        genericVPDPageReadOutput = false;
                        printf("\tGot Unit Serial Number as %s\n", unitSerialNumber);
                        memcpy(&scsiDevInfo->vpdData.unitSN, unitSerialNumber, M_Min(255, strlen(unitSerialNumber)));
                    }
                    safe_Free(unitSerialNumber);
                }
                else
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: Unable to allocate %" PRIu16 " Bytes for the serial number\n", vpdPageLength + 1);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }
            break;
        case IMPLEMENTED_OPERATING_DEFINITIONS:
            printf("Implemented Operating Definitions VPD Page\n");
            break;
        case ASCII_IMPLEMENTED_OPERATING_DEFINITION:
            printf("ASCII Implemented Operation Definition VPD Page\n");
            break;
        case DEVICE_IDENTIFICATION:
            printf("Device Identification VPD Page\n");
            if (readVPDPage)
            {
                printf("\tReported Designators:\n");
                genericVPDPageReadOutput = false;
                uint16_t counter = 0;
                uint16_t designatorLength = 4;//header length and will be update with each one we look at.
                bool gotEUI64 = false, gotNAA = false, gotSCSINameString = false, gotUUID = false, gotMD5 = false;
                uint16_t md5Offset = 0;
                scsiDevInfo->vpdData.gotDeviceIDVPDPage = true;

                for (uint32_t offset = 4; offset < C_CAST(uint32_t, vpdPageLength + 4) && offset < UINT16_MAX; offset += designatorLength + 4, ++counter)
                {
                    uint8_t protocolIdentifier = M_Nibble1(pageToRead[offset]);
                    uint8_t codeSet = M_Nibble0(pageToRead[offset]);
                    bool piv = M_ToBool(pageToRead[offset + 1] & BIT7);
                    uint8_t association = M_GETBITRANGE(pageToRead[offset + 1], 5, 4);
                    uint8_t designatorType = M_Nibble0(pageToRead[offset + 1]);
                    uint16_t designatorOffset = C_CAST(uint16_t, offset + 4);//This cast to a smaller type should be ok since the offset should never even get to the max uint16_t length. There is a small possibility of ovverflow with the +4, but it is very slim due to how this is reported from a device. This should never really happen. - TJE
                    designatorLength = pageToRead[offset + 3];
                    bool isASCII = false;
                    bool isUTF8 = false;
                    switch (codeSet)
                    {
                    case 1://binary values
                        break;
                    case 2://ASCII printable characters
                        isASCII = true;
                        break;
                    case 3://UTF-8 codes
                        isUTF8 = true;
                        break;
                    case 0://reserved
                    default://reserved
                        break;
                    }
                    //Save the value of the designator to our array to track all reported designators
                    scsiDevInfo->vpdData.designators[scsiDevInfo->vpdData.designatorCount].valid = true;
                    memcpy(&scsiDevInfo->vpdData.designators[scsiDevInfo->vpdData.designatorCount].designator[0], &pageToRead[designatorOffset], M_Min(designatorLength, UINT16_C(16)));
                    scsiDevInfo->vpdData.designators[scsiDevInfo->vpdData.designatorCount].designatorLength = M_Min(designatorLength, UINT16_C(16));
                    scsiDevInfo->vpdData.designators[scsiDevInfo->vpdData.designatorCount].designatorType = designatorType;
                    ++scsiDevInfo->vpdData.designatorCount;

                    printf("\t\tDesignator %" PRIu16 "\n", counter);
                    switch (association)
                    {
                    case 0://associated with addressed logical unit
                        printf("\t\t\tAssociated with the addressed logical unit\n");
                        break;
                    case 1://associated with the target port that received the command
                           //check PIV & protocol identifier
                        printf("\t\t\tAssociated with the target port that received the command\n");
                        if (piv)
                        {
                            switch (protocolIdentifier)
                            {
                            case 0:
                                printf("\t\t\t    Fibre Channel Protocol for SCSI\n");
                                break;
                            case 1:
                                printf("\t\t\t    Parallel SCSI\n");
                                break;
                            case 2:
                                printf("\t\t\t    Serial Storage Architecture SCSI-3 Protocol\n");
                                break;
                            case 3:
                                printf("\t\t\t    Serial Bus Protocol for IEEE 1394\n");
                                break;
                            case 4:
                                printf("\t\t\t    SCSI RDMA Protocol\n");
                                break;
                            case 5:
                                printf("\t\t\t    Internet SCSI (iSCSI)\n");
                                break;
                            case 6:
                                printf("\t\t\t    SAS Serial SCSI Protocol\n");
                                break;
                            case 7:
                                printf("\t\t\t    Automation/Drive Interface Transport Protocol\n");
                                break;
                            case 8:
                                printf("\t\t\t    AT Attachment Interface\n");
                                break;
                            case 9:
                                printf("\t\t\t    USB Attached SCSI\n");
                                break;
                            case 10:
                                printf("\t\t\t    SCSI over PCI Express\n");
                                break;
                            case 0x0f:
                                printf("\t\t\t    No specific Protocol\n");
                                break;
                            default:
                                printf("\t\t\t    Reserved protocol ID value %" PRIX8 "h\n", protocolIdentifier);
                                break;
                            }
                        }
                        break;
                    case 2://associated with the SCSI target device that contains the addressed logical unit
                           //check PIV & protocol identifier
                        printf("\t\t\tAssociated with the SCSI target device that contains the addressed logical unit\n");
                        if (piv)
                        {
                            switch (protocolIdentifier)
                            {
                            case 0:
                                printf("\t\t\t    Fibre Channel Protocol for SCSI\n");
                                break;
                            case 1:
                                printf("\t\t\t    Parallel SCSI\n");
                                break;
                            case 2:
                                printf("\t\t\t    Serial Storage Architecture SCSI-3 Protocol\n");
                                break;
                            case 3:
                                printf("\t\t\t    Serial Bus Protocol for IEEE 1394\n");
                                break;
                            case 4:
                                printf("\t\t\t    SCSI RDMA Protocol\n");
                                break;
                            case 5:
                                printf("\t\t\t    Internet SCSI (iSCSI)\n");
                                break;
                            case 6:
                                printf("\t\t\t    SAS Serial SCSI Protocol\n");
                                break;
                            case 7:
                                printf("\t\t\t    Automation/Drive Interface Transport Protocol\n");
                                break;
                            case 8:
                                printf("\t\t\t    AT Attachment Interface\n");
                                break;
                            case 9:
                                printf("\t\t\t    USB Attached SCSI\n");
                                break;
                            case 10:
                                printf("\t\t\t    SCSI over PCI Express\n");
                                break;
                            case 0x0f:
                                printf("\t\t\t    No specific Protocol\n");
                                break;
                            default:
                                printf("\t\t\t    Reserved protocol ID value %" PRIX8 "h\n", protocolIdentifier);
                                break;
                            }
                        }
                        break;
                    case 3://reserved
                        set_Console_Colors(true, WARNING_COLOR);
                        printf("\t\t\tWARNING: Reserved association\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        break;
                    }
                    switch (designatorType)
                    {
                    case 0://vendor specific
                        printf("\t\t\tVendor Specific Designator:\n");
                        print_Data_Buffer(&pageToRead[offset], designatorLength, true);
                        break;
                    case 1://T10 vendor ID based
                        printf("\t\t\tT10 Vendor ID Based Designator:\n");
                        if (!isASCII)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Code set does not mark this as ASCII! This should be ASCII data!!!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        {
                            char desVendorID[9] = { 0 };
                            char *vendorSpecificID = NULL;
                            memcpy(desVendorID, &pageToRead[designatorOffset], 8);
                            printf("\t\t\t    T10 Vendor ID: %s\n", desVendorID);
                            if (designatorLength > 8)
                            {
                                vendorSpecificID = C_CAST(char*, calloc(designatorLength - 8 + 1, sizeof(char)));
                                if (vendorSpecificID)
                                {
                                    memcpy(vendorSpecificID, &pageToRead[designatorOffset + 8], designatorLength - 8);
                                    //TODO: validate that all characters are printable
                                    printf("\t\t\t    Vendor Specific ID: %s\n", vendorSpecificID);
                                    safe_Free(vendorSpecificID);
                                }
                            }
                            else
                            {
                                set_Console_Colors(true, WARNING_COLOR);
                                printf("WARNING: T10 Vendor ID based designator is missing vendor specific identifier!!!\n");
                                printf("Recommended method from SPC is to concatenate Product ID and Product Serial number\n");
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                            }
                        }
                        break;
                    case 2://EUI-64 based
                        gotEUI64 = true;
                        //TODO: May need to save this to compare for NVMe over USB
                        printf("\t\t\tEUI-64 Designator:\n");
                        //3 different formats depending on designator length
                        switch (designatorLength)
                        {
                        case 0x08://EUI64
                            printf("\t\t\t    EUI64\n");
                            printf("\t\t\t    IEEE Company ID: ");
                            for (uint32_t ieeeVIDOffset = designatorOffset; ieeeVIDOffset < C_CAST(uint32_t, (designatorOffset + 3)); ++ieeeVIDOffset)
                            {
                                printf("%02" PRIX8, pageToRead[ieeeVIDOffset]);
                            }
                            printf("\n");
                            printf("\t\t\t    Vendor Specific Extension Identifier: ");
                            for (uint32_t vuExtIDOffset = designatorOffset + 3; vuExtIDOffset < C_CAST(uint32_t, (designatorOffset + 8)); ++vuExtIDOffset)
                            {
                                printf("%02" PRIX8, pageToRead[vuExtIDOffset]);
                            }
                            printf("\n");
                            break;
                        case 0x0C://EUI64 - 12 byte
                            printf("\t\t\t    EUI64 - 12 Byte\n");
                            printf("\t\t\t    IEEE Company ID: ");
                            for (uint32_t ieeeVIDOffset = designatorOffset; ieeeVIDOffset < C_CAST(uint32_t, (designatorOffset + 3)); ++ieeeVIDOffset)
                            {
                                printf("%02" PRIX8, pageToRead[ieeeVIDOffset]);
                            }
                            printf("\n");
                            printf("\t\t\t    Vendor Specific Extension Identifier: ");
                            for (uint32_t vuExtIDOffset = designatorOffset + 3; vuExtIDOffset < C_CAST(uint32_t, (designatorOffset + 8)); ++vuExtIDOffset)
                            {
                                printf("%02" PRIX8, pageToRead[vuExtIDOffset]);
                            }
                            printf("\n");
                            printf("\t\t\t    Directory ID: ");
                            for (uint32_t dIDOffset = designatorOffset + 8; dIDOffset < C_CAST(uint32_t, (designatorOffset + 12)); ++dIDOffset)
                            {
                                printf("%02" PRIX8, pageToRead[dIDOffset]);
                            }
                            printf("\n");
                            break;
                        case 0x10://EUI64 - 16 byte
                            printf("\t\t\t    EUI64 - 16 Byte\n");
                            printf("\t\t\t    Identifier Extension: ");
                            for (uint32_t idExtOffset = designatorOffset; idExtOffset < C_CAST(uint32_t, (designatorOffset + 8)); ++idExtOffset)
                            {
                                printf("%02" PRIX8, pageToRead[idExtOffset]);
                            }
                            printf("\n");
                            printf("\t\t\t    IEEE Company ID: ");
                            for (uint32_t ieeeVIDOffset = designatorOffset + 8; ieeeVIDOffset < C_CAST(uint32_t, (designatorOffset + 11)); ++ieeeVIDOffset)
                            {
                                printf("%02" PRIX8, pageToRead[ieeeVIDOffset]);
                            }
                            printf("\n");
                            printf("\t\t\t    Vendor Specific Extension Identifier: ");
                            for (uint32_t vuExtIDOffset = designatorOffset + 11; vuExtIDOffset < C_CAST(uint32_t, (designatorOffset + 16)); ++vuExtIDOffset)
                            {
                                printf("%02" PRIX8, pageToRead[vuExtIDOffset]);
                            }
                            printf("\n");
                            break;
                        default://unknown EUI64 format!
                            printf("\t\t\t    Unknown EUI64 designator length!\n");
                            printf("\t\t\t    Unknown data format: ");
                            //print it out in hex
                            for (uint32_t euiOffset = designatorOffset; euiOffset < C_CAST(uint32_t, (designatorOffset + designatorLength)); ++euiOffset)
                            {
                                printf("%02" PRIX8, pageToRead[euiOffset]);
                            }
                            printf("\n");
                            break;
                        }
                        break;
                    case 3://NAA
                        gotNAA = true;
                        //TODO: May need to save this to compare for SATA over USB
                        printf("\t\t\tNetwork Address Authority Designator:\n");
                        //NAA value will be either 2, 4, 5, or 6 which changes length and format
                        {
                            uint8_t naa = M_Nibble1(pageToRead[designatorOffset]);
                            switch (naa)
                            {
                            case 2://IEEE Extended - binary only
                                printf("\t\t\t    NAA Type 2 - IEEE Extended\n");
                                //vendor specific Identifier A
                                printf("\t\t\t        Vendor Specific ID A: %01" PRIX8 "%02" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset]), pageToRead[designatorOffset + 1]);
                                //IEEE Company ID
                                printf("\t\t\t        IEEE Company ID: %02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", pageToRead[designatorOffset + 2], pageToRead[designatorOffset + 3], pageToRead[designatorOffset + 4]);
                                //vendor specific Identifier B
                                printf("\t\t\t        Vendor Specific ID B: %02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", pageToRead[designatorOffset + 5], pageToRead[designatorOffset + 6], pageToRead[designatorOffset + 7]);
                                break;
                            case 3://Locally Assigned - binary only
                                printf("\t\t\t    NAA Type 3 - Locally Assigned\n");
                                //Locally administered value
                                printf("\t\t\t        Locally Administered Value: %01" PRIX8, M_Nibble0(pageToRead[designatorOffset]));//print the first nibble, the rest is all individual bytes
                                for (uint16_t lavOffset = designatorOffset + 1; lavOffset < (designatorLength + 4); ++lavOffset)
                                {
                                    printf("%02" PRIX8, pageToRead[lavOffset]);
                                }
                                printf("\n");
                                break;
                            case 5://IEEE Registered - binary only
                                printf("\t\t\t    NAA Type 5 - IEEE Registered\n");
                                //IEEE Company ID
                                printf("\t\t\t        IEEE Company ID: %01" PRIX8 "%02" PRIX8 "%02" PRIX8 "%01" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset]), pageToRead[designatorOffset + 1], pageToRead[designatorOffset + 2], M_Nibble1(pageToRead[designatorOffset + 3]));
                                //Vendor Specific Identifier
                                printf("\t\t\t        Vendor Specific ID: %01" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset + 3]), pageToRead[designatorOffset + 4], pageToRead[designatorOffset + 5], pageToRead[designatorOffset + 6], pageToRead[designatorOffset + 7]);
                                break;
                            case 6://IEEE Registered Extended - binary only
                                printf("\t\t\t    NAA Type 6 - IEEE Registered Extended\n");
                                //IEEE Company ID
                                printf("\t\t\t        IEEE Company ID: %01" PRIX8 "%02" PRIX8 "%02" PRIX8 "%01" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset]), pageToRead[designatorOffset + 1], pageToRead[designatorOffset + 2], M_Nibble1(pageToRead[designatorOffset + 3]));
                                //Vendor Specific Identifier
                                printf("\t\t\t        Vendor Specific ID: %01" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", M_Nibble0(pageToRead[designatorOffset + 3]), pageToRead[designatorOffset + 4], pageToRead[designatorOffset + 5], pageToRead[designatorOffset + 6], pageToRead[designatorOffset + 7]);
                                //Vendor Specific Identifier Extentsion
                                printf("\t\t\t        Vendor Specific ID Extension: %02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "\n", pageToRead[designatorOffset + 8], pageToRead[designatorOffset + 9], pageToRead[designatorOffset + 10], pageToRead[designatorOffset + 11], pageToRead[designatorOffset + 12], pageToRead[designatorOffset + 13], pageToRead[designatorOffset + 14], pageToRead[designatorOffset + 15]);
                                break;
                            default://unknown NAA Type
                                printf("\t\t\t    Unknown NAA Type %" PRIX8 "h\n", naa);
                                printf("\t\t\t        Value: ");
                                for (uint16_t unaaOffset = designatorOffset; unaaOffset < (designatorLength + 4); ++unaaOffset)
                                {
                                    if (codeSet == 2 || codeSet == 3)
                                    {
                                        //ASCII or UTF8...this should be OK, but we could get a unprintable UTF8 character this way!
                                        printf("%c", pageToRead[unaaOffset]);
                                    }
                                    else
                                    {
                                        printf("%02" PRIX8, pageToRead[unaaOffset]);
                                    }
                                }
                                printf("\n");
                                break;
                            }
                        }
                        break;
                    case 4://relative target port identifier
                        printf("\t\t\tRelative Target Port Identifier Designator:\n");
                        if (isASCII || isUTF8)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Code set is not set properly! This should be set to 1 for binary!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        else if (association != 1)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Use of this designator with association not set to 1 is reserved!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        {
                            uint16_t relativeTargetPortID = M_BytesTo2ByteValue(pageToRead[designatorOffset + 2], pageToRead[designatorOffset + 3]);
                            printf("\t\t\t    Relative Target Port ID: %" PRIu16 "\n", relativeTargetPortID);
                        }
                        break;
                    case 5://target port group
                        printf("\t\t\tTarget Port Group Designator:\n");
                        if (isASCII || isUTF8)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Code set is not set properly! This should be set to 1 for binary!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        else if (association != 1)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Use of this designator with association not set to 1 is reserved!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        {
                            uint16_t targetPortGroup = M_BytesTo2ByteValue(pageToRead[designatorOffset + 2], pageToRead[designatorOffset + 3]);
                            printf("\t\t\t    Target Port Group: %" PRIu16 "\n", targetPortGroup);
                        }
                        break;
                    case 6://logical unit group
                        printf("\t\t\tLogical Unit Group Designator:\n");
                        if (isASCII || isUTF8)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Code set is not set properly! This should be set to 1 for binary!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        else if (association != 0)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Use of this designator with association not set to 0 is reserved!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        {
                            uint16_t luGroup = M_BytesTo2ByteValue(pageToRead[designatorOffset + 2], pageToRead[designatorOffset + 3]);
                            printf("\t\t\t    Logical Unit Group: %" PRIu16 "\n", luGroup);
                        }
                        break;
                    case 7://MD5 logical unit identifier
                        gotMD5 = true;
                        //This should basically never be used at this point as all the other unique identifiers should be used instead. This
                        //should be handled AFTER checking all other designators in order to warn if it is in use while other unique designators are 
                        //present as SPC defines. 
                        md5Offset = designatorOffset;
                        break;
                    case 8://SCSI name string
                        gotSCSINameString = true;
                        printf("\t\t\tSCSI Name String Designator:\n");
                        if (!isUTF8)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Code set is not properly set! This should be set to 3 for UTF8!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        {
                            char *scsiNameString = NULL;
                            scsiNameString = C_CAST(char*, calloc(designatorLength + 1, sizeof(char)));
                            if (scsiNameString)
                            {
                                memcpy(scsiNameString, &pageToRead[designatorOffset], designatorLength);
                                //TODO: validate that all characters are UTF8
                                printf("\t\t\t    SCSI Name: %s\n", scsiNameString);
                                safe_Free(scsiNameString);
                            }
                            else
                            {
                                set_Console_Colors(true, ERROR_COLOR);
                                printf("ERROR: failed to allocate memory (%" PRIu16 "B) to read SCSI Name string\n", designatorLength + 1);
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                            }
                        }
                        break;
                    case 9: //protocol specific port identifier
                        printf("\t\t\tProtocol Specific Port Identifier Designator:\n");
                        //this is either USB or PCIe information depending on the protocol identifier field
                        if (association != 0x01)
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Association is not set to 1! This may cause parsing issues!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        if (!piv)
                        {
                            //This should be an error since we can only parse this if the interface is set.
                            set_Console_Colors(true, ERROR_COLOR);
                            printf("ERROR: Protocol identifier valid bit is not set! Cannot parse this without this bit!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                        else
                        {
                            switch (protocolIdentifier)
                            {
                            case 9://UAS
                            {
                                uint8_t deviceAddress = M_GETBITRANGE(pageToRead[designatorOffset], 6, 0);
                                uint8_t interfaceNumber = pageToRead[designatorOffset + 2];
                                printf("\t\t\t    UAS Target Port Identifier:\n");
                                printf("\t\t\t        Device Address: %02" PRIX8 "h\n", deviceAddress);
                                printf("\t\t\t        Interface Number: %02" PRIX8 "h\n", interfaceNumber);
                            }
                            break;
                            case 0xA://SCSI over PCIe
                            {
                                uint16_t routingID = M_BytesTo2ByteValue(pageToRead[designatorOffset], pageToRead[designatorOffset + 1]);
                                printf("\t\t\t    PCIe Routing Identifier:\n");
                                printf("\t\t\t        Routing ID: %04" PRIX16 "h\n", routingID);
                            }
                            break;
                            default://unknown or not defined in SPC5 when this was written
                                printf("\t\t\t    Unknown Protocol Specific Port Identifier type!\n");
                                print_Data_Buffer(&pageToRead[designatorOffset], designatorLength, true);
                                break;
                            }
                        }
                        break;
                    case 0xA://UUID
                        gotUUID = true;
                        printf("\t\t\tUUID Designator:\n");
                        {
                            uint8_t uuidType = M_Nibble1(pageToRead[designatorOffset]);
                            switch (uuidType)
                            {
                            case 1://locally assigned RFC 4122 UUID
                                printf("\t\t\t    Locally Assigned RFC 4122 UUID:\n");
                                printf("\t\t\t        ");
                                //print bytes 2 - 17
                                if (designatorLength < 18)
                                {
                                    set_Console_Colors(true, WARNING_COLOR);
                                    printf("WARNING: Designator length should be 18B, but got %" PRIu16 "B\n", designatorLength);
                                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                                }
                                for (uint16_t uuidOffset = designatorOffset; uuidOffset < (designatorLength + 4); ++uuidOffset)
                                {
                                    printf("%02" PRIX8, pageToRead[uuidOffset]);
                                }
                                printf("\n");
                                break;
                            default://reserved or unknown UUID type
                                printf("\t\t\t    Unknown UUID type!\n");
                                print_Data_Buffer(&pageToRead[designatorOffset], designatorLength, true);
                                break;
                            }
                        }
                        break;
                    default://unknown or reserved type
                        printf("\t\t\tUnknown Designator Type - %" PRIX8 "h\n", designatorType);
                        print_Data_Buffer(&pageToRead[designatorOffset], designatorLength, true);
                        break;
                    }
                }
                //checking if we got an MD5 when we shouldn't have!
                if (gotMD5 && (gotEUI64 || gotNAA || gotSCSINameString || gotUUID))
                {
                    set_Console_Colors(true, WARNING_COLOR);
                    printf("WARNING: Device reported MD5 designator while also reporting a\n");
                    printf("         unique identifier as well. This is not allowed per SPC!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else if (gotMD5 && md5Offset)
                {
                    //TODO: Validate the MD5 output by computing it ourselves from other designators that were reported!
                    //NOTE: Any designators not available are replaced with 8 space characters per spec as input to the MD5 computation
                    printf("\t\t\tMD5 Logical Unit Designator:\n");
                    printf("\t\t\t    MD5 Logical Unit Identifier: ");
                    for (; md5Offset < 16; ++md5Offset)
                    {
                        printf("%02" PRIX8, pageToRead[md5Offset]);
                    }
                    printf("\n");
                }
            }
            break;
        case SOFTWARE_INTERFACE_IDENTIFICATION:
            printf("Software Interface Identification VPD Page\n");
            break;
        case MANAGEMENT_NETWORK_ADDRESSES:
            printf("Management Network Addresses VPD Page\n");
            break;
        case EXTENDED_INQUIRY_DATA://While this page is useful, it is only in SAT4 and only and only if read buffer command is supported. SAT5 adds in support for download microcode modes supported
            printf("Extended Inquiry Data VPD Page\n");
            //Thorough parsing of this page and outputting its information in a nice format
            if (readVPDPage)
            {
                if (vpdPageLength < 0x003C)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: VPD Page length is less than specified in SPC! Expected %" PRIX16 "h, but got %" PRIX16 "h\n", 0x003C, vpdPageLength);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    genericVPDPageReadOutput = false;
                    scsiDevInfo->vpdData.gotExInquiryVPDPage = true;
                    //Need to figure out which things are actually useful to show! Will start with things that are most useful
                    uint16_t extDSTMinutes = M_BytesTo2ByteValue(pageToRead[10], pageToRead[11]);
                    if (extDSTMinutes == 0)
                    {
                        set_Console_Colors(true, WARNING_COLOR);
                        printf("WARNING: Extended DST time in minutes was reported as zero! This means SCSI DST translation is not available!\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        printf("\tExtended Self-Test Completion Time (minutes): %" PRIu16 "\n", extDSTMinutes);
                        scsiDevInfo->vpdData.vpdDSTTimeMinutes = extDSTMinutes;
                    }
                    if (pageToRead[13] == 0)
                    {
                        set_Console_Colors(true, WARNING_COLOR);
                        printf("WARNING: Maximum Sense Length not reported. Will assume %u Bytes\n", SPC3_SENSE_LEN);
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else if (pageToRead[13] > SPC3_SENSE_LEN)
                    {
                        set_Console_Colors(true, WARNING_COLOR);
                        printf("WARNING: Maximum Sense Length reported as larger than max allowed by SPC! %u Bytes\n", pageToRead[13]);
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        printf("\tMaximum supported sense data length: %" PRIu8 " Bytes\n", pageToRead[13]);
                    }
                    if (pageToRead[12] & BIT4)
                    {
                        //Download Microcode support byte is available
                        //TODO: Parse this out. Skipping for now since this is so new nothing will be using it yet.
                    }
                }
            }
            break;
        case MODE_PAGE_POLICY:
            printf("Mode Page Policy VPD Page\n");
            //TODO: Parse this page to figure out which mode pages are supported by the device so that the mode page checking section can be validated better!
            break;
        case SCSI_PORTS:
            printf("SCSI Ports VPD Page\n");
            break;
        case ATA_INFORMATION:
            printf("ATA Information VPD Page\n");
            if (readVPDPage)
            {
                if (vpdPageLength < 0x0238)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: VPD Page length is less than specified in SAT! Expected %" PRIX16 "h, but got %" PRIX16 "h\n", 0x0238, vpdPageLength);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    char satVendor[9] = { 0 };
                    char satProductID[17] = { 0 };
                    char satRevision[5] = { 0 };
                    memcpy(satVendor, &pageToRead[8], 8);
                    memcpy(satProductID, &pageToRead[16], 16);
                    memcpy(satRevision, &pageToRead[32], 4);
                    genericVPDPageReadOutput = false;
                    scsiDevInfo->vpdData.gotSATVPDPage = true;
                    for (uint8_t iter = 0; iter < 8; ++iter)
                    {
                        if (!is_ASCII(satVendor[iter]) || !isprint(satVendor[iter]))
                        {
                            satVendor[iter] = ' ';
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: SAT Vendor ID contains non-ASCII or non-Printable Characters!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                    }
                    memcpy(device->drive_info.bridge_info.t10SATvendorID, satVendor, 8);
                    if (strlen(satVendor) == 0)
                    {
                        set_Console_Colors(true, WARNING_COLOR);
                        printf("WARNING: SAT Vendor ID is empty!\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        printf("\tGot SAT Vendor ID as %s\n", satVendor);
                    }
                    for (uint8_t iter = 0; iter < 16; ++iter)
                    {
                        if (!is_ASCII(satProductID[iter]) || !isprint(satProductID[iter]))
                        {
                            satProductID[iter] = ' ';
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: SAT Product ID contains non-ASCII or non-Printable Characters!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                    }
                    memcpy(device->drive_info.bridge_info.SATproductID, satProductID, 16);
                    if (strlen(satProductID) == 0)
                    {
                        set_Console_Colors(true, WARNING_COLOR);
                        printf("WARNING: SAT Product ID is empty!\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        printf("\tGot SAT Product ID as %s\n", satProductID);
                    }
                    for (uint8_t iter = 0; iter < 4; ++iter)
                    {
                        if (!is_ASCII(satRevision[iter]) || !isprint(satRevision[iter]))
                        {
                            satRevision[iter] = ' ';
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: SAT Product Revision contains non-ASCII or non-Printable Characters!\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        }
                    }
                    memcpy(device->drive_info.bridge_info.SATfwRev, satRevision, 4);
                    if (strlen(satRevision) == 0)
                    {
                        set_Console_Colors(true, WARNING_COLOR);
                        printf("WARNING: SAT Product Revision is empty!\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    else
                    {
                        printf("\tGot SAT Product Revision as %s\n", satRevision);
                    }
                    //Device signature (starting at byte 36)
                    uint16_t signatureOffset = 36;
                    bool isSATA = false;
                    printf("\tATA Device Signature:\n");
                    switch (pageToRead[signatureOffset])
                    {
                    case 0x00://PATA
                        printf("\t\tTransport: PATA\n");
                        break;
                    case 0x34://SATA
                        printf("\t\tTransport: SATA\n");
                        isSATA = true;
                        break;
                    default://unknown
                        printf("\t\tTransport: Unknown (%02" PRIX8 "h)\n", pageToRead[signatureOffset]);
                        break;
                    }
                    if (isSATA)
                    {
                        printf("\t\tInterrupt: %d\n", (pageToRead[signatureOffset + 1] & BIT6) > 0 ? 1 : 0);
                        printf("\t\tPM Port: %" PRIu8 "\n", M_Nibble0(pageToRead[signatureOffset + 1]));
                    }
                    printf("\t\tStatus:       %02"  PRIX8 "h\n", pageToRead[signatureOffset + 2]);
                    printf("\t\tError:        %02"  PRIX8 "h\n", pageToRead[signatureOffset + 3]);
                    printf("\t\tLBA (7:0):    %02"  PRIX8 "h\n", pageToRead[signatureOffset + 4]);
                    printf("\t\tLBA (15:8):   %02"  PRIX8 "h\n", pageToRead[signatureOffset + 5]);
                    printf("\t\tLBA (23:16):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 6]);
                    printf("\t\tDevice:       %02"  PRIX8 "h\n", pageToRead[signatureOffset + 7]);
                    printf("\t\tLBA (31:24):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 8]);
                    printf("\t\tLBA (39:32):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 9]);
                    printf("\t\tLBA (47:40):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 10]);
                    printf("\t\tCount (7:0):  %02"  PRIX8 "h\n", pageToRead[signatureOffset + 12]);
                    printf("\t\tCount (15:8): %02"  PRIX8 "h\n", pageToRead[signatureOffset + 13]);
                    //command code
                    printf("\tIdentify Command Code: ");
                    switch (pageToRead[56])
                    {
                    case 0xEC://Identify
                        printf("Identify (ECh)\n");
                        break;
                    case 0xA1://Identify Packet
                        printf("Identify Packet (A1h)\n");
                        break;
                    case 0x2F://Read Log Ext
                        printf("Read Log Ext (2Fh)\n");
                        break;
                    case 0x47://Read Log Ext DMA
                        printf("Read Log Ext DMA (47h)\n");
                        break;
                    default://unknown
                        printf("Unknown (%02" PRIX8 "h)\n", pageToRead[56]);
                        break;
                    }
                    printf("\tIdentify Data:\n");
                    print_Data_Buffer(&pageToRead[60], 512, true);
                }
            }
            break;
        case POWER_CONDITION:
            printf("Power Condition VPD Page\n");
            //TODO: Save power condition information to compare to what is reported from reading ATA device logs
            if (readVPDPage)
            {
                if (vpdPageLength < 0x0E)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: VPD Page length is less than specified in SPC! Expected %" PRIX16 "h, but got %" PRIX16 "h\n", 0x0E, vpdPageLength);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    scsiDevInfo->vpdData.gotPowerConditionsVPDPage = true;
                    genericVPDPageReadOutput = false;
                    if (M_BytesTo2ByteValue(pageToRead[8], pageToRead[9]) == UINT16_MAX)
                    {
                        printf("\tStopped Condition Recovery Time: >65.534 milliseconds\n");
                    }
                    else
                    {
                        printf("\tStopped Condition Recovery Time: %" PRIu16 " milliseconds\n", M_BytesTo2ByteValue(pageToRead[8], pageToRead[9]));
                    }
                    if (pageToRead[4] & BIT0)
                    {
                        printf("\tStandby Z Supported\n");
                        printf("\t    Recovery Time: ");
                        scsiDevInfo->vpdData.powerConditionData.standby_z = true;
                        scsiDevInfo->vpdData.powerConditionData.stansbyZTimer = M_BytesTo2ByteValue(pageToRead[8], pageToRead[9]);
                        if (scsiDevInfo->vpdData.powerConditionData.stansbyZTimer == UINT16_MAX)
                        {
                            printf(">65.534 milliseconds\n");
                        }
                        else
                        {
                            printf("%" PRIu16 " milliseconds\n", scsiDevInfo->vpdData.powerConditionData.stansbyZTimer);
                        }
                    }
                    if (pageToRead[4] & BIT1)
                    {
                        printf("\tStandby Y Supported\n");
                        scsiDevInfo->vpdData.powerConditionData.standby_y = true;
                        scsiDevInfo->vpdData.powerConditionData.stansbyYTimer = M_BytesTo2ByteValue(pageToRead[10], pageToRead[11]);
                        if (scsiDevInfo->vpdData.powerConditionData.stansbyYTimer == UINT16_MAX)
                        {
                            printf(">65.534 milliseconds\n");
                        }
                        else
                        {
                            printf("%" PRIu16 " milliseconds\n", scsiDevInfo->vpdData.powerConditionData.stansbyYTimer);
                        }
                    }
                    if (pageToRead[54] & BIT0)
                    {
                        printf("\tIdle A Supported\n");
                        scsiDevInfo->vpdData.powerConditionData.idle_a = true;
                        scsiDevInfo->vpdData.powerConditionData.idleATimer = M_BytesTo2ByteValue(pageToRead[12], pageToRead[13]);
                        if (scsiDevInfo->vpdData.powerConditionData.idleATimer == UINT16_MAX)
                        {
                            printf(">65.534 milliseconds\n");
                        }
                        else
                        {
                            printf("%" PRIu16 " milliseconds\n", scsiDevInfo->vpdData.powerConditionData.idleATimer);
                        }
                    }
                    if (pageToRead[54] & BIT1)
                    {
                        printf("\tIdle B Supported\n");
                        scsiDevInfo->vpdData.powerConditionData.idle_b = true;
                        scsiDevInfo->vpdData.powerConditionData.idleBTimer = M_BytesTo2ByteValue(pageToRead[12], pageToRead[13]);
                        if (scsiDevInfo->vpdData.powerConditionData.idleBTimer == UINT16_MAX)
                        {
                            printf(">65.534 milliseconds\n");
                        }
                        else
                        {
                            printf("%" PRIu16 " milliseconds\n", scsiDevInfo->vpdData.powerConditionData.idleBTimer);
                        }
                    }
                    if (pageToRead[54] & BIT2)
                    {
                        printf("\tIdle C Supported\n");
                        scsiDevInfo->vpdData.powerConditionData.idle_c = true;
                        scsiDevInfo->vpdData.powerConditionData.idleCTimer = M_BytesTo2ByteValue(pageToRead[16], pageToRead[17]);
                        if (scsiDevInfo->vpdData.powerConditionData.idleCTimer == UINT16_MAX)
                        {
                            printf(">65.534 milliseconds\n");
                        }
                        else
                        {
                            printf("%" PRIu16 " milliseconds\n", scsiDevInfo->vpdData.powerConditionData.idleCTimer);
                        }
                    }
                }
            }
            break;
        case DEVICE_CONSTITUENTS:
            printf("Device Constituents VPD Page\n");
            break;
        case CFA_PROFILE_INFORMATION:
            printf("CFA Profile Information VPD Page\n");
            break;
        case POWER_CONSUMPTION:
            printf("Power Consumption VPD Page\n");
            break;
        case THIRD_PARTY_COPY:
            printf("Third Party Copy VPD Page\n");
            break;
        case PROTOCOL_SPECIFIC_LU_INFO:
            printf("Protocol Specific Logical Unit Information VPD Page\n");
            break;
        case PROTOCOL_SPECIFIC_PORT_INFO:
            printf("Protocol Specific Port Information VPD Page\n");
            break;
        case SCSI_FEATURE_SETS:
            printf("SCSI Feature Sets VPD Page\n");
            if (readVPDPage)
            {
                genericVPDPageReadOutput = false;
                for (uint32_t featIter = 8; featIter < C_CAST(uint32_t, (vpdPageLength + 4)) && featIter < UINT16_MAX; featIter += 2)
                {
                    uint16_t featureSetCode = M_BytesTo2ByteValue(pageToRead[featIter], pageToRead[featIter + 1]);
                    switch (featureSetCode)
                    {
                    case 0x0001:
                        printf("\tSPC Discovery 2016 Feature Set\n");
                        printf("\t\tMandatory Commands:\n");
                        printf("\t\t\tInquiry\n");
                        printf("\t\t\tMode Sense 10\n");
                        printf("\t\t\tReport Luns\n");
                        printf("\t\t\tReport Supported Operation Codes\n");
                        printf("\t\t\tTest Unit Ready\n");
                        printf("\t\tMandatory Mode Pages:\n");
                        printf("\t\t\tControl\n");
                        printf("\t\t\tControl Extension\n");
                        printf("\t\tMandatory VPD Pages:\n");
                        printf("\t\t\tDevice Identification\n");
                        printf("\t\t\tExtended Inquiry Data\n");
                        printf("\t\t\tMode Page Policy\n");
                        printf("\t\t\tSCSI Feature Sets\n");
                        printf("\t\t\tSupported VPD Pages\n");
                        break;
                    case 0x0102: //SBC Base 2010
                        printf("\tSBC Base 2010 Feature Set\n");
                        printf("\t\tMandatory Commands:\n");
                        printf("\t\t\tFormat Unit\n");
                        printf("\t\t\tInquiry\n");
                        printf("\t\t\tMode Select 10\n");
                        printf("\t\t\tMode Sense 10\n");
                        printf("\t\t\tRead 10\n");
                        printf("\t\t\tRead Capacity 10\n");
                        printf("\t\t\tReport Luns\n");
                        printf("\t\t\tRequest Sense\n");
                        printf("\t\t\tStart Stop Unit\n");
                        printf("\t\t\tSynchronize Cache 10\n");
                        printf("\t\t\tTest Unit Ready\n");
                        printf("\t\t\tWrite 10\n");
                        printf("\t\t\tWrite Same 10\n");
                        printf("\t\tMandatory Mode Pages:\n");
                        printf("\t\t\tCaching\n");
                        printf("\t\t\tControl\n");
                        printf("\t\t\tControl Extension\n");
                        printf("\t\t\tRead-Write Error Recovery\n");
                        printf("\t\tMandatory VPD Pages:\n");
                        printf("\t\t\tATA Information (SAT devices only)\n");
                        printf("\t\t\tBlock Device Characteristics\n");
                        printf("\t\t\tDevice Identification\n");
                        printf("\t\t\tExtended Inquiry Data\n");
                        printf("\t\t\tMode Page Policy\n");
                        printf("\t\t\tSCSI Feature Sets\n");
                        printf("\t\t\tSupported VPD Pages\n");
                        printf("\t\t\tPower Condition\n");
                        break;
                    case 0x0101: //SBC Base 2016
                        printf("\tSBC Base 2016 Feature Set\n");
                        printf("\t\tMandatory Commands:\n");
                        printf("\t\t\tFormat Unit\n");
                        printf("\t\t\tInquiry\n");
                        printf("\t\t\tMode Select 10\n");
                        printf("\t\t\tMode Sense 10\n");
                        printf("\t\t\tRead 16\n");
                        printf("\t\t\tRead Capacity 16\n");
                        printf("\t\t\tReport Luns\n");
                        printf("\t\t\tReport Supported Operation Codes\n");
                        printf("\t\t\tReport Supported Task Management Functions\n");
                        printf("\t\t\tRequest Sense\n");
                        printf("\t\t\tStart Stop Unit\n");
                        printf("\t\t\tSynchronize Cache 10\n");
                        printf("\t\t\tTest Unit Ready\n");
                        printf("\t\t\tWrite 16\n");
                        printf("\t\t\tWrite Same 16\n");
                        printf("\t\tMandatory Mode Pages:\n");
                        printf("\t\t\tCaching\n");
                        printf("\t\t\tControl\n");
                        printf("\t\t\tControl Extension\n");
                        printf("\t\t\tInformation Exceptions Control\n");
                        printf("\t\t\tRead-Write Error Recovery\n");
                        printf("\t\tMandatory VPD Pages:\n");
                        printf("\t\t\tATA Information (SAT devices only)\n");
                        printf("\t\t\tBlock Device Characteristics\n");
                        printf("\t\t\tBlock Limits\n");
                        printf("\t\t\tDevice Identification\n");
                        printf("\t\t\tExtended Inquiry Data\n");
                        printf("\t\t\tMode Page Policy\n");
                        printf("\t\t\tSCSI Feature Sets\n");
                        printf("\t\t\tSupported VPD Pages\n");
                        printf("\t\t\tPower Condition\n");
                        break;
                    case 0x0103: //Basic Provisioning 2016
                        printf("\tBasic Provisioning 2016 Feature Set\n");
                        printf("\t\tMandatory Commands:\n");
                        printf("\t\t\tGet LBA Status\n");
                        printf("\t\t\tRead Capacity 16\n");
                        printf("\t\t\tUnmap\n");
                        printf("\t\t\tWrite Same 16\n");
                        printf("\t\tMandatory VPD Pages:\n");
                        printf("\t\t\tBlock Limits\n");
                        printf("\t\t\tLogical Block Provisioning\n");
                        break;
                    case 0x0104: //Drive Maintenance 2016
                        printf("\tDrive Maintenance 2016 Feature Set\n");
                        printf("\t\tMandatory Commands:\n");
                        printf("\t\t\tFormat Unit\n");
                        printf("\t\t\tLog Select\n");
                        printf("\t\t\tLog Sense\n");
                        printf("\t\t\tRead Buffer 10\n");
                        printf("\t\t\tRead Defect Data 12\n");
                        printf("\t\t\tReasign Blocks\n");
                        printf("\t\t\tSanitize\n");
                        printf("\t\t\tSend Diagnostics\n");
                        printf("\t\t\tReceive Diagnostic Results\n");
                        printf("\t\t\tWrite Buffer\n");
                        printf("\t\t\tWrite Long 16\n");
                        printf("\t\tMandatory VPD Pages:\n");
                        printf("\t\t\tExtended Inquiry Data\n");
                        printf("\t\t\tBlock Device Characteristics\n");
                        printf("\t\t\tPower Consumption\n");
                        printf("\t\tMandatory Log Pages:\n");
                        printf("\t\t\tSupported Pages\n");
                        printf("\t\t\tSupported Pages and Subpages\n");
                        printf("\t\t\tBackground Scan\n");
                        printf("\t\t\tInformational Exceptions\n");
                        printf("\t\t\tNon-Medium Error\n");
                        printf("\t\t\tNon-volatile Cache\n");
                        printf("\t\t\tRead Error Counters\n");
                        printf("\t\t\tSelf-Test Results\n");
                        printf("\t\t\tSolid State Media\n");
                        printf("\t\t\tStart-Stop Cycle Counter\n");
                        printf("\t\t\tTemperature\n");
                        break;
                    default:
                        if (featureSetCode <= 0x00FF)
                        {
                            //SPC
                            printf("\tUnknown SPC Feature Set: %04" PRIX16 "h\n", featureSetCode);
                        }
                        else if (featureSetCode > 0x00FF && featureSetCode <= 0x01FF)
                        {
                            //SBC
                            printf("\tUnknown SBC Feature Set: %04" PRIX16 "h\n", featureSetCode);
                        }
                        else if (featureSetCode > 0x01FF && featureSetCode <= 0x02FF)
                        {
                            //SSC
                            printf("\tUnknown SSC Feature Set: %04" PRIX16 "h\n", featureSetCode);
                        }
                        else if (featureSetCode > 0x02FF && featureSetCode <= 0x03FF)
                        {
                            //ZBC
                            printf("\tUnknown ZBC Feature Set: %04" PRIX16 "h\n", featureSetCode);
                        }
                        else
                        {
                            //unknown or reserved feature set code
                            printf("\tUnknown Standard Feature Set: %04" PRIX16 "h\n", featureSetCode);
                        }
                        break;
                    }
                }
            }
            break;
        case BLOCK_LIMITS:
            printf("Block Limits VPD Page\n");
            if (readVPDPage)
            {
                if (vpdPageLength < 0x003C)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: VPD Page length is less than specified in SBC! Expected %" PRIX16 "h, but got %" PRIX16 "h\n", 0x003C, vpdPageLength);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    genericVPDPageReadOutput = false;
                    scsiDevInfo->vpdData.gotBlockLimitsVPDPage = true;
                    if (pageToRead[5] > 0)
                    {
                        printf("\tMaximum Compare And Write Length: %" PRIu8 " Logical Blocks\n", pageToRead[5]);
                    }
                    printf("\tOptimal Transfer Length Granularity: ");
                    if (M_BytesTo2ByteValue(pageToRead[6], pageToRead[7]) > 0)
                    {
                        printf("%" PRIu16 " Logical Blocks\n", M_BytesTo2ByteValue(pageToRead[6], pageToRead[7]));
                    }
                    else
                    {
                        printf("Not reported\n");
                    }
                    printf("\tMaximum Transfer Length: ");
                    scsiDevInfo->vpdData.blockLimitsData.maximumXferLen = M_BytesTo4ByteValue(pageToRead[8], pageToRead[9], pageToRead[10], pageToRead[11]);
                    if (scsiDevInfo->vpdData.blockLimitsData.maximumXferLen > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", scsiDevInfo->vpdData.blockLimitsData.maximumXferLen);
                    }
                    else
                    {
                        printf("Not Reported\n");
                    }
                    printf("\tOptimal Transfer Length: ");
                    scsiDevInfo->vpdData.blockLimitsData.optimalXferLen = M_BytesTo4ByteValue(pageToRead[12], pageToRead[13], pageToRead[14], pageToRead[15]);
                    if (scsiDevInfo->vpdData.blockLimitsData.optimalXferLen > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", scsiDevInfo->vpdData.blockLimitsData.optimalXferLen);
                    }
                    else
                    {
                        printf("Not Reported\n");
                    }
                    printf("\tMaximum Prefetch Length: ");
                    if (M_BytesTo4ByteValue(pageToRead[16], pageToRead[17], pageToRead[18], pageToRead[19]) > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[16], pageToRead[17], pageToRead[18], pageToRead[19]));
                    }
                    else
                    {
                        printf("Not Reported or prefetch not supported\n");
                    }
                    printf("\tMaximum Unmap LBA Count: ");
                    if (M_BytesTo4ByteValue(pageToRead[20], pageToRead[21], pageToRead[22], pageToRead[23]) > 0)
                    {
                        printf("%" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[20], pageToRead[21], pageToRead[22], pageToRead[23]));
                    }
                    else
                    {
                        printf("Not Reported\n");
                    }
                    printf("\tMaximum Unmap Block Descriptor Count: ");
                    if (M_BytesTo4ByteValue(pageToRead[24], pageToRead[25], pageToRead[26], pageToRead[27]) > 0)
                    {
                        printf("%" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[24], pageToRead[25], pageToRead[26], pageToRead[27]));
                    }
                    else
                    {
                        printf("Not Reported\n");
                    }
                    printf("\tOptimal Unmap Granularity: ");
                    if (M_BytesTo4ByteValue(pageToRead[28], pageToRead[29], pageToRead[30], pageToRead[31]) > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[28], pageToRead[29], pageToRead[30], pageToRead[31]));
                    }
                    else
                    {
                        printf("Not Reported\n");
                    }
                    if (pageToRead[32] & BIT7)
                    {
                        printf("\tUnmap Granularity Alignment: ");
                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[32] ^ BIT7, pageToRead[33], pageToRead[34], pageToRead[35]));
                    }
                    printf("\tMaximum Write Same Length: ");
                    if (M_BytesTo8ByteValue(pageToRead[36], pageToRead[37], pageToRead[38], pageToRead[39], pageToRead[40], pageToRead[41], pageToRead[42], pageToRead[43]) > 0)
                    {
                        printf("%" PRIu64 " Logical Blocks\n", M_BytesTo8ByteValue(pageToRead[36], pageToRead[37], pageToRead[38], pageToRead[39], pageToRead[40], pageToRead[41], pageToRead[42], pageToRead[43]));
                    }
                    else
                    {
                        printf("Not Reported or write same not supported\n");
                    }
                    if (pageToRead[4] & BIT0)
                    {
                        printf("\tWrite same command does not allow number of logical blocks to be zero (write full LBA space the same)\n");
                    }
                    printf("\tMaximum Atomic Transfer Length: ");
                    if (M_BytesTo4ByteValue(pageToRead[44], pageToRead[45], pageToRead[46], pageToRead[47]) > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[44], pageToRead[45], pageToRead[46], pageToRead[47]));
                    }
                    else
                    {
                        printf("Not Reported or write atomic not supported\n");
                    }
                    printf("\tAtomic Alignment: ");
                    if (M_BytesTo4ByteValue(pageToRead[48], pageToRead[49], pageToRead[50], pageToRead[51]) > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[48], pageToRead[49], pageToRead[50], pageToRead[51]));
                    }
                    else
                    {
                        printf("Not Reported or write atomic not supported\n");
                    }
                    printf("\tAtomic Transfer Length Granularity: ");
                    if (M_BytesTo4ByteValue(pageToRead[52], pageToRead[53], pageToRead[54], pageToRead[55]) > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[52], pageToRead[53], pageToRead[54], pageToRead[55]));
                    }
                    else
                    {
                        printf("Not Reported or write atomic not supported\n");
                    }
                    printf("\tMaximum Atomic Transfer Length With Atomic Boundary: ");
                    if (M_BytesTo4ByteValue(pageToRead[56], pageToRead[57], pageToRead[58], pageToRead[59]) > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[56], pageToRead[57], pageToRead[58], pageToRead[59]));
                    }
                    else
                    {
                        printf("Not Reported or write atomic not supported\n");
                    }
                    printf("\tMaximum Atomic Boundary Size: ");
                    if (M_BytesTo4ByteValue(pageToRead[60], pageToRead[61], pageToRead[62], pageToRead[63]) > 0)
                    {
                        printf("%" PRIu32 " Logical Blocks\n", M_BytesTo4ByteValue(pageToRead[60], pageToRead[61], pageToRead[62], pageToRead[63]));
                    }
                    else
                    {
                        printf("Not Reported or write atomic not supported\n");
                    }
                }
            }
            break;
        case BLOCK_DEVICE_CHARACTERISTICS:
            printf("Block Device Characteristics VPD Page\n");
            if (readVPDPage)
            {
                if (vpdPageLength < 0x003C)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: VPD Page length is less than specified in SBC! Expected %" PRIX16 "h, but got %" PRIX16 "h\n", 0x003C, vpdPageLength);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    scsiDevInfo->vpdData.gotBlockCharacteristicsVPDPage = true;
                    genericVPDPageReadOutput = false;
                    scsiDevInfo->vpdData.blockCharacteristicsData.rotationRate = M_BytesTo2ByteValue(pageToRead[4], pageToRead[5]);
                    printf("\tMedium Rotation Rate: ");
                    if (scsiDevInfo->vpdData.blockCharacteristicsData.rotationRate == 0)
                    {
                        printf("Not Reported\n");
                    }
                    else if (scsiDevInfo->vpdData.blockCharacteristicsData.rotationRate == 1)
                    {
                        printf("Non-rotating medium\n");
                    }
                    else if (scsiDevInfo->vpdData.blockCharacteristicsData.rotationRate >= 0x0401 && scsiDevInfo->vpdData.blockCharacteristicsData.rotationRate <= 0xFFFE)
                    {
                        printf("%" PRIu16 " RPM\n", scsiDevInfo->vpdData.blockCharacteristicsData.rotationRate);
                    }
                    else
                    {
                        set_Console_Colors(true, ERROR_COLOR);
                        printf("ERROR: Reserved value\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                    printf("\tProduct Type: ");
                    scsiDevInfo->vpdData.blockCharacteristicsData.productType = pageToRead[6];
                    switch (scsiDevInfo->vpdData.blockCharacteristicsData.productType)
                    {
                    case 0:
                        printf("Not indicated\n");
                        break;
                    case 1:
                        printf("CFast\n");
                        break;
                    case 2:
                        printf("CompactFlash\n");
                        break;
                    case 3:
                        printf("Memory Stick\n");
                        break;
                    case 4:
                        printf("MultiMediaCard\n");
                        break;
                    case 5:
                        printf("Secure Digital Card\n");
                        break;
                    case 6:
                        printf("XQD\n");
                        break;
                    case 7:
                        printf("Universal Flash Storage\n");
                        break;
                    default:
                        if (scsiDevInfo->vpdData.blockCharacteristicsData.productType >= 8 && scsiDevInfo->vpdData.blockCharacteristicsData.productType <= 0xEF)
                        {
                            printf("Reserved\n");
                        }
                        else
                        {
                            printf("Vendor Specific\n");
                        }
                        break;
                    }
                    //These are write after <block | crypto> erase required fields. Not necessary right now to report.
                    //uint8_t wabereq = M_GETBITRANGE(pageToRead[7], 7, 6);
                    //uint8_t wacereq = M_GETBITRANGE(pageToRead[7], 5, 4);

                    //form factor
                    printf("\tNominal Form Factor: ");
                    scsiDevInfo->vpdData.blockCharacteristicsData.formFactor = M_Nibble0(pageToRead[7]);
                    switch (scsiDevInfo->vpdData.blockCharacteristicsData.formFactor)
                    {
                    case 0:
                        printf("Not Reported\n");
                        break;
                    case 1:
                        printf("5.25\"\n");
                        break;
                    case 2:
                        printf("3.5\"\n");
                        break;
                    case 3:
                        printf("2.5\"\n");
                        break;
                    case 4:
                        printf("1.8\"\n");
                        break;
                    case 5:
                        printf("Less than 1.8\"\n");
                        break;
                    //NOTE: SATA spec has other form factors listed in it now that aren't in SBC as of writing this.
                    case 6:
                        printf("mSATA\n");
                        break;
                    case 7:
                        printf("M.2\n");
                        break;
                    case 8:
                        printf("MicroSSD\n");
                        break;
                    case 9:
                        printf("CFast\n");
                        break;
                    default://all others are reserved
                        printf("Reserved\n");
                        break;
                    }
                    printf("Zoned Capabilities: ");
                    scsiDevInfo->vpdData.blockCharacteristicsData.zonedCapabilities = M_GETBITRANGE(pageToRead[8], 5, 4);
                    switch (scsiDevInfo->vpdData.blockCharacteristicsData.zonedCapabilities)
                    {
                    case 0:
                        printf("Not Reported\n");
                        break;
                    case 1:
                        printf("Host Aware Zoned Block Device\n");
                        break;
                    case 2:
                        printf("Device Managed Zoned Block Device\n");
                        break;
                    default:
                        printf("Reserved\n");
                        break;
                    }
                }
            }
            break;
        case LOGICAL_BLOCK_PROVISIONING:
            printf("Logical Block Provisioning VPD Page\n");
            if (readVPDPage)
            {
                if (vpdPageLength < 0x0004)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: VPD Page length is less than specified in SBC! Expected %" PRIX16 "h, but got %" PRIX16 "h\n", 0x0004, vpdPageLength);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    //Threshold exponent and percentage don't really matter on SCSI to ATA devices.
                    //provisioning type SHOULD be reported especially for SCSI to NVMe
                    printf("\tUnmap Command: ");
                    scsiDevInfo->vpdData.gotLogicalBlockProvisioningVPDPage = true;
                    scsiDevInfo->vpdData.unmapSupported = M_ToBool(pageToRead[5] & BIT7);
                    if (scsiDevInfo->vpdData.unmapSupported)
                    {
                        printf("Supported\n");
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                    printf("\tUnmap Support in Write Same 16: ");
                    if (pageToRead[5] & BIT6)
                    {
                        printf("Supported\n");
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                    printf("\tUnmap Support in Write Same 10: ");
                    if (pageToRead[5] & BIT5)
                    {
                        printf("Supported\n");
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                    printf("\tUnmapped LBA Data: ");
                    uint8_t lbprz = M_GETBITRANGE(pageToRead[5], 4, 2);
                    if (lbprz == 0)
                    {
                        printf("Vendor Specific\n");
                    }
                    else if (lbprz & BIT0)
                    {
                        printf("Zeros\n");
                    }
                    else if (lbprz == 0x2)
                    {
                        printf("Provisioning Initialization Pattern\n");
                    }
                    else
                    {
                        printf("Reserved\n");
                    }
                    printf("\tAnchor Support: ");
                    if (pageToRead[6] & BIT1)
                    {
                        printf("Supported\n");
                    }
                    else
                    {
                        printf("Not Supported\n");
                    }
                    uint8_t provisioningType = M_GETBITRANGE(pageToRead[6], 2, 0);
                    printf("\tProvisioning Type: ");
                    switch (provisioningType)
                    {
                    case 0:
                        printf("Not reported or fully provisioned\n");
                        break;
                    case 1:
                        printf("Resource Provisioned\n");
                        break;
                    case 2:
                        printf("Thin Provisioned\n");
                        break;
                    default:
                        printf("Reserved\n");
                        break;
                    }

//                          if (pageToRead[6] & BIT0)
//                          {
//                              //TODO: If DP bit is set, parse the provisioning group descriptor
//                              //Will be either t10 vendor ID, NAA, or UUID designator type
//                          }
                }
            }
            break;
        case REFERALS:
            printf("Referals VPD Page\n");
            break;
        case SUPPORTED_BLOCK_LENGTHS_AND_PROTECTION_TYPES:
            printf("Supported Block Lengths And Protection Types VPD Page\n");
            break;
        case BLOCK_DEVICE_CHARACTERISTISCS_EXT:
            printf("Block Device Characteristics Ext VPD Page\n");
            //TODO: Thorough parsing of this page and outputting its information in a nice format
            break;
        case ZONED_BLOCK_DEVICE_CHARACTERISTICS:
            printf("Zoned Block Device Characteristics VPD Page\n");
            //TODO: Thorough parsing of this page and outputting its information in a nice format
            break;
        case BLOCK_LIMITS_EXTENSION:
            printf("Block Limits Extension VPD Page\n");
            //TODO: Thorough parsing of this page and outputting its information in a nice format
            break;
        default:
            if (supportedPages[vpdIter] >= 0x01 && supportedPages[vpdIter] <= 0x7F)
            {
                //ascii information page
                printf("ASCII Information VPD Page %02" PRIX8 "h\n", supportedPages[vpdIter]);
            }
            else if (supportedPages[vpdIter] >= 0xC0 /* && supportedPages[vpdIter] <= 0xFF */)
            {
                //vendor specific
                printf("Vendor Specific VPD Page %02" PRIX8 "h\n", supportedPages[vpdIter]);
            }
            else
            {
                //unknown page
                printf("Unknown VPD Page %02" PRIX8 "h\n", supportedPages[vpdIter]);
            }
            break;
        }
        if (genericVPDPageReadOutput && readVPDPage)
        {
            print_Data_Buffer(pageToRead, vpdPageLength, true);
        }
        safe_Free_aligned(pageToRead)
    }
    if (pagesread <= dummiedPageCount && dummiedPages)//less than or equal to 1 because it is possible that the only suppored page is the unit serial number!
    {
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: NVPD\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.scsiHacks.noVPDPages = true;
    }
}

static eReturnValues scsi_Information(tDevice *device, ptrScsiDevInformation scsiDevInfo)
{
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n====================\n");
    printf("Reading Inquiry Data\n");
    printf("====================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);

    if (SUCCESS == scsi_Inquiry(device, (uint8_t *)&device->drive_info.scsiVpdData.inquiryData, 96, 0, false, false))
    {
        bool continueTesting = false;
        uint8_t *inqPtr = &device->drive_info.scsiVpdData.inquiryData[0];
        memset(device->drive_info.serialNumber, 0, sizeof(device->drive_info.serialNumber));
        memset(device->drive_info.T10_vendor_ident, 0, sizeof(device->drive_info.T10_vendor_ident));
        memset(device->drive_info.product_identification, 0, sizeof(device->drive_info.product_identification));
        memset(device->drive_info.product_revision, 0, sizeof(device->drive_info.product_revision));

        //print out peripheral device type
        scsiDevInfo->inquiryData.peripheralDeviceType = M_GETBITRANGE(inqPtr[0], 4, 0);
        printf("\t\tPeripheral Device Type: ");
        switch (scsiDevInfo->inquiryData.peripheralDeviceType)
        {
        case PERIPHERAL_DIRECT_ACCESS_BLOCK_DEVICE:
            printf("Direct Access Block Device (SBC)\n");
            continueTesting = true;
            break;
        case PERIPHERAL_SEQUENTIAL_ACCESS_BLOCK_DEVICE:
            printf("Sequential Access Block Device (SSC)\n");
            break;
        case PERIPHERAL_PRINTER_DEVICE:
            printf("Printer Device (SSC)\n");
            break;
        case PERIPHERAL_PROCESSOR_DEVICE:
            printf("Processor Device (SPC)\n");
            break;
        case PERIPHERAL_WRITE_ONCE_DEVICE:
            printf("Write Once Device (SBC)\n");
            break;
        case PERIPHERAL_CD_DVD_DEVICE:
            printf("CD/DVD Device (MMC)\n");
            break;
        case PERIPHERAL_SCANNER_DEVICE:
            printf("Scanner Device (SGC)\n");
            break;
        case PERIPHERAL_OPTICAL_MEMORY_DEVICE:
            printf("Optical Memory Device (SBC)\n");
            break;
        case PERIPHERAL_MEDIUM_CHANGER_DEVICE:
            printf("Medium Changer Device (SMC)\n");
            break;
        case PERIPHERAL_COMMUNICATIONS_DEVICE:
            printf("Communications Device (SSC)\n");
            break;
        case PERIPHERAL_ASC_IT8_1:
        case PERIPHERAL_ASC_IT8_2:
            printf("Graphic arts pre-press Device (ASC IT8)\n");
            break;
        case PERIPHERAL_STORAGE_ARRAY_CONTROLLER_DEVICE:
            printf("Storage Array Controller Device (SCC)\n");
            break;
        case PERIPHERAL_ENCLOSURE_SERVICES_DEVICE:
            printf("Enclosure Services Device (SES)\n");
            break;
        case PERIPHERAL_SIMPLIFIED_DIRECT_ACCESS_DEVICE:
            printf("Simplified Direct Access Block Device (RBC)\n");
            break;
        case PERIPHERAL_OPTICAL_CARD_READER_WRITER_DEVICE:
            printf("Optical Card Reader/Writer Device (OCRW)\n");
            break;
        case PERIPHERAL_BRIDGE_CONTROLLER_COMMANDS:
            printf("Reserved for Bridge Contoller/Expanders\n");
            break;
        case PERIPHERAL_OBJECT_BASED_STORAGE_DEVICE:
            printf("Object-Based Storage Device (OSD)\n");
            break;
        case PERIPHERAL_AUTOMATION_DRIVE_INTERFACE:
            printf("Automation/Drive Interface (ADC)\n");
            break;
        case PERIPHERAL_SECURITY_MANAGER_DEVICE:
            printf("Security Manager Device\n");
            break;
        case PERIPHERAL_HOST_MANAGED_ZONED_BLOCK_DEVICE:
            continueTesting = true;
            printf("Host Managed Zoned Block Device (ZBC)\n");
            break;
        case PERIPHERAL_WELL_KNOWN_LOGICAL_UNIT:
            printf("Well Known Logical Unit\n");
            break;
        case PERIPHERAL_RESERVED3:
        case PERIPHERAL_RESERVED4:
        case PERIPHERAL_RESERVED5:
        case PERIPHERAL_RESERVED6:
        case PERIPHERAL_RESERVED7:
        case PERIPHERAL_RESERVED8:
        case PERIPHERAL_RESERVED9:
        case PERIPHERAL_RESERVED10:
        case PERIPHERAL_RESERVED11:
            printf("Reserved Device Type (%" PRIX8 "h)\n", scsiDevInfo->inquiryData.peripheralDeviceType);
            break;
        case PERIPHERAL_UNKNOWN_OR_NO_DEVICE_TYPE:
        default:
            printf("Unknown or no device type\n");
            break;
        }
        //check rmb bit (show warning if this is set unless this truly is ejectable media, which an HDD is not)
        if (inqPtr[1] & BIT7)
        {
            printf("\t\tRMB bit is set meaning the recording medium is removable. Ex: CD/DVD\n");
            scsiDevInfo->inquiryData.rmb = true;
        }

        //Report value for version, translated to SCSI/SPC version. Report warnings if below SPC3 since this is when SAT was first released (version1 in 2006)
        printf("\t\tSCSI Version: ");
        device->drive_info.scsiVersion = scsiDevInfo->inquiryData.scsiVersion = inqPtr[2];
        switch (inqPtr[2])
        {
        case 0:
            printf("No version compliance reported. This is typical for very old devices...SASI devices\n");
            device->drive_info.scsiVersion = SCSI_VERSION_NO_STANDARD;
//              if (device->drive_info.interface_type != USB_INTERFACE)
//              {
//                  checkForSAT = false; //NOTE: some cheap USB to SATA/PATA adapters will set this version or no version. The only way to work around this, is to make sure the low level for the OS detects it on USB interface and it can be run through the usb_hacks file instead.
//              }
            break;
        case 0x01:
        case 0x81:
            printf("SCSI (1986) - (%" PRIX8 "h)\n", inqPtr[2]);
            device->drive_info.scsiVersion = SCSI_VERSION_SCSI;//changing to 1 for SCSI
//              if (device->drive_info.interface_type != USB_INTERFACE)
//              {
//                  checkForSAT = false;//NOTE: some cheap USB to SATA/PATA adapters will set this version or no version. The only way to work around this, is to make sure the low level for the OS detects it on USB interface and it can be run through the usb_hacks file instead.
//              }
            break;
        case 0x02:
        case 0x80:
        case 0x82:
            printf("SCSI 2 (1993) - (%" PRIX8 "h)\n", inqPtr[2]);
            device->drive_info.scsiVersion = SCSI_VERSION_SCSI2;//changing to 2 for SCSI 2
            break;
        case 0x03:
        case 0x83:
            printf("SPC (1997) - (%" PRIX8 "h)\n", inqPtr[2]);
            device->drive_info.scsiVersion = SCSI_VERSION_SPC;//changing to 3 for SPC
            break;
        case 0x04:
        case 0x84:
            printf("SPC2 (2001) - (%" PRIX8 "h)\n", inqPtr[2]);
            device->drive_info.scsiVersion = SCSI_VERSION_SPC_2;//changing to 4 for SPC2
            break;
        case 0x05://SPC-3
            printf("SPC3 (2005) - (%" PRIX8 "h)\n", inqPtr[2]);
            break;
        case 0x06://SPC-4
            printf("SPC4 (2015) - (%" PRIX8 "h)\n", inqPtr[2]);
            break;
        case 0x07://SPC-5
            printf("SPC5 (2019) - (%" PRIX8 "h)\n", inqPtr[2]);
            break;
        default:
            //convert some versions since old standards broke the version number into ANSI vs ECMA vs ISO standard numbers
            if ((inqPtr[2] >= 0x08 && inqPtr[2] <= 0x0C) ||
                (inqPtr[2] >= 0x40 && inqPtr[2] <= 0x44) ||
                (inqPtr[2] >= 0x48 && inqPtr[2] <= 0x4C) ||
                (inqPtr[2] >= 0x80 && inqPtr[2] <= 0x84) ||
                (inqPtr[2] >= 0x88 && inqPtr[2] <= 0x8C))
            {
                printf("Unknown or Obsolete version format - (%" PRIX8 "h)\n", inqPtr[2]);
                //these are obsolete version numbers
                device->drive_info.scsiVersion = scsiDevInfo->inquiryData.scsiVersion = M_GETBITRANGE(inqPtr[2], 3, 0);
                printf("Treating as - (%" PRIX8 "h) for legacy device compatibility since version is likely ANSI+ECMA+ISO format\n", device->drive_info.scsiVersion);
            }
            else
            {
                printf("Unknown version - (%" PRIX8 "h)\n", inqPtr[2]);
            }
            break;
        }

        //Check response data format. This should be 2h. If not, then this is a device from before SCSI2 which means nothing else is parsable. 
        //If below 2, report a strong warning that the fields below are all vendor unique and may not be parsable. Ask if this product is older than 1993??? CCS was released in 1986....
        scsiDevInfo->inquiryData.responseFormat = M_Nibble0(inqPtr[3]);
        uint8_t totalInqLength = inqPtr[4] + 4;
        if (scsiDevInfo->inquiryData.responseFormat < 2)
        {
            set_Console_Colors(true, WARNING_COLOR);
            printf("WARNING: Response data format is set to %" PRIu8 "!\n", scsiDevInfo->inquiryData.responseFormat);
            printf("         This means that all product identification is in a vendor specific\n");
            printf("         format and cannot be parsed correctly!\n");
            printf("         This is only expected for OLD SCSI devices. Nothing else should\n");
            printf("         report in this format. Raw output is provided which may be usable\n");
            printf("         if needed for better legacy device support.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: PRESCSI2\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            device->drive_info.passThroughHacks.scsiHacks.preSCSI2InqData = true;
            print_Data_Buffer(inqPtr, totalInqLength, true);
        }
        else
        {
            //Get and print T10 vendor - print warning if empty?
            memcpy(scsiDevInfo->inquiryData.vendorId, &inqPtr[8], T10_VENDOR_ID_LEN);
            //Check for printable and non-ASCII characters to warn that these are not supposed to be here!
            for (uint8_t iter = 0; iter < T10_VENDOR_ID_LEN; ++iter)
            {
                if (!is_ASCII(scsiDevInfo->inquiryData.vendorId[iter]) || !isprint(scsiDevInfo->inquiryData.vendorId[iter]))
                {
                    scsiDevInfo->inquiryData.vendorId[iter] = ' ';
                    set_Console_Colors(true, WARNING_COLOR);
                    printf("WARNING: Vendor ID contains non-ASCII or non-Printable Characters!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }
            memcpy(device->drive_info.T10_vendor_ident, scsiDevInfo->inquiryData.vendorId, T10_VENDOR_ID_LEN);
            if (strlen(scsiDevInfo->inquiryData.vendorId) == 0)
            {
                set_Console_Colors(true, WARNING_COLOR);
                printf("WARNING: Vendor ID is empty!\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            else
            {
                printf("Got Vendor ID as %s\n", scsiDevInfo->inquiryData.vendorId);
            }
            //Get and print Product ID - print warning if empty?
            memcpy(scsiDevInfo->inquiryData.productId, &inqPtr[16], INQ_DATA_PRODUCT_ID_LEN);
            //Check for printable and non-ASCII characters to warn that these are not supposed to be here!
            for (uint8_t iter = 0; iter < INQ_DATA_PRODUCT_ID_LEN; ++iter)
            {
                if (!is_ASCII(scsiDevInfo->inquiryData.productId[iter]) || !isprint(scsiDevInfo->inquiryData.productId[iter]))
                {
                    scsiDevInfo->inquiryData.productId[iter] = ' ';
                    set_Console_Colors(true, WARNING_COLOR);
                    printf("WARNING: Product ID contains non-ASCII or non-Printable Characters!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }
            memcpy(device->drive_info.product_identification, scsiDevInfo->inquiryData.productId, INQ_DATA_PRODUCT_ID_LEN);
            if (strlen(scsiDevInfo->inquiryData.productId) == 0)
            {
                set_Console_Colors(true, WARNING_COLOR);
                printf("WARNING: Product ID is empty!\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            else
            {
                printf("Got Product ID as %s\n", scsiDevInfo->inquiryData.productId);
            }
            //get and print product revision - print warning if empty?
            memcpy(scsiDevInfo->inquiryData.productRev, &inqPtr[32], INQ_DATA_PRODUCT_REV_LEN);
            //Check for printable and non-ASCII characters to warn that these are not supposed to be here!
            for (uint8_t iter = 0; iter < INQ_DATA_PRODUCT_REV_LEN; ++iter)
            {
                if (!is_ASCII(scsiDevInfo->inquiryData.productRev[iter]) || !isprint(scsiDevInfo->inquiryData.productRev[iter]))
                {
                    scsiDevInfo->inquiryData.productRev[iter] = ' ';
                    set_Console_Colors(true, WARNING_COLOR);
                    printf("WARNING: Product Revision contains non-ASCII or non-Printable Characters!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }
            memcpy(device->drive_info.product_revision, scsiDevInfo->inquiryData.productRev, INQ_DATA_PRODUCT_REV_LEN);
            if (strlen(scsiDevInfo->inquiryData.productRev) == 0)
            {
                set_Console_Colors(true, WARNING_COLOR);
                printf("WARNING: Product Revision is empty!\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            else
            {
                printf("Got Product Revision as %s\n", scsiDevInfo->inquiryData.productRev);
            }
            //TODO: based on what was reported for these, check passthrough data later to see if the vendor/product were just concatenated together instead of following translation
            //NOTE: Many USB products report something completely different from the encapsulated drive to make it appear as a specific product in the system. This is not abnormal, but may be a light warning.

            //TODO: Check for version descriptors and show what they are
            if (totalInqLength > 36)//checking if inquire data is longer since it was only 36bytes in SCSI2
            {
                if (totalInqLength > 58)
                {
                    printf("Checking Version Descriptors:\n");
                    if (scsiDevInfo->inquiryData.scsiVersion >= 4)
                    {
                        //Version Descriptors 1-8 (SPC2 and up)
                        uint8_t versionIter = 0;
                        for (; versionIter < 8; versionIter++)
                        {
                            scsiDevInfo->inquiryData.versionDescriptors[versionIter] = 0;
                            scsiDevInfo->inquiryData.versionDescriptors[versionIter] = M_BytesTo2ByteValue(device->drive_info.scsiVpdData.inquiryData[(versionIter * 2) + 58], device->drive_info.scsiVpdData.inquiryData[(versionIter * 2) + 59]);
                            if (scsiDevInfo->inquiryData.versionDescriptors[versionIter] > 0)
                            {
                                char versionString[MAX_VERSION_DESCRIPTOR_STRING_LENGTH] = { 0 };
                                printf("\t%04" PRIX16 " - ", scsiDevInfo->inquiryData.versionDescriptors[versionIter]);
                                decypher_SCSI_Version_Descriptors(scsiDevInfo->inquiryData.versionDescriptors[versionIter], C_CAST(char*, versionString));
                                printf("%s\n", versionString);
                                //TODO: Note when finding SAT or USB, etc to and how it's helpful to figure out what a device supports and how to discover it.
                            }
                            if (is_Standard_Supported(scsiDevInfo->inquiryData.versionDescriptors[versionIter], STANDARD_CODE_SAT)
                                || is_Standard_Supported(scsiDevInfo->inquiryData.versionDescriptors[versionIter], STANDARD_CODE_SAT2)
                                || is_Standard_Supported(scsiDevInfo->inquiryData.versionDescriptors[versionIter], STANDARD_CODE_SAT3)
                                || is_Standard_Supported(scsiDevInfo->inquiryData.versionDescriptors[versionIter], STANDARD_CODE_SAT4)
                                //Next version descriptors aren't sat but should only appear on a SAT interface...at least we know they are ATA/ATAPI so it won't hurt to try issuing a command to the drive.
                                || is_Standard_Supported(scsiDevInfo->inquiryData.versionDescriptors[versionIter], STANDARD_CODE_ATA_ATAPI6)
                                || is_Standard_Supported(scsiDevInfo->inquiryData.versionDescriptors[versionIter], STANDARD_CODE_ATA_ATAPI7)
                                || is_Standard_Supported(scsiDevInfo->inquiryData.versionDescriptors[versionIter], STANDARD_CODE_ATA_ATAPI8)
                                || is_Standard_Supported(scsiDevInfo->inquiryData.versionDescriptors[versionIter], STANDARD_CODE_ACSx)
                                )
                            {
                                //SAT or ATA version found. Should definitely check for SAT VPD page.
                                scsiDevInfo->inquiryData.satVersionDescriptorFound = true;
                            }
                        }
                    }
                    else
                    {
                        printf("\tThis device reports SPC compliance which is prior to vesrion descriptors being added.\n");
                    }
                }
            }
            else
            {
                set_Console_Colors(true, NOTE_COLOR);
                printf("NOTE: Inquiry data is 36 bytes or less but reports format 2 which should be 96\n");
                printf("      bytes. In these bytes version descriptors will be reported and can be used\n");
                printf("      to help better understand device capabilities (starting with SPC2)\n\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }

        }

        if (!continueTesting)
        {
            return NOT_SUPPORTED;
        }
    }
    else
    {
        set_Console_Colors(true, ERROR_COLOR);
        printf("Fatal Error: Unable to get standard inquiry data. Unable to proceed with any more testing!\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        return FAILURE;
    }
    return SUCCESS;
}

static eReturnValues scsi_Capacity_Information(tDevice *device, ptrScsiDevInformation scsiDevInfo)
{
    //Read capacity. Start with 10, then do 16. Emit warnings when 16 doesn't work and the scsi versions is greater than SPC2
    //If read capacity 10 comes back saying UINT32_MAX for capacity, warn that this mismatches the SCSI version reported.
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n========================================\n");
    printf("Getting Read Capacity data. 10 & 16 byte\n");
    printf("========================================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    uint8_t readCapacityData[32] = { 0 };
    eReturnValues readCap10Result = SUCCESS, readCap16Result = SUCCESS;
    if (SUCCESS == (readCap10Result = scsi_Read_Capacity_10(device, readCapacityData, 8)))
    {
        scsiDevInfo->readCapData.rc10MaxLBA = M_BytesTo4ByteValue(readCapacityData[0], readCapacityData[1], readCapacityData[2], readCapacityData[3]);
        scsiDevInfo->readCapData.rc10blockSize = M_BytesTo4ByteValue(readCapacityData[4], readCapacityData[5], readCapacityData[6], readCapacityData[7]);
        printf("\tRead Capacity 10 data:\n");
        printf("\t\tMaxLBA: %" PRIu32 "\n", scsiDevInfo->readCapData.rc10MaxLBA);
        printf("\t\tBlock Size : %" PRIu32 "\n", scsiDevInfo->readCapData.rc10blockSize);
        device->drive_info.devicePhyBlockSize = device->drive_info.deviceBlockSize = scsiDevInfo->readCapData.rc10blockSize;
        device->drive_info.deviceMaxLba = scsiDevInfo->readCapData.rc10MaxLBA;
    }
    else
    {
        set_Console_Colors(true, WARNING_COLOR);
        printf("WARNING: Failed read capacity 10. While this command is superceeded by read capacity 16,\n");
        printf("         supporting it helps legacy system support and software. If not supported, it should\n");
        printf("         at least fail gracefully and report \"Invalid Operation Code\"\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }
    memset(readCapacityData, 0, 32);
    if (SUCCESS == (readCap16Result = scsi_Read_Capacity_16(device, readCapacityData, 32)))
    {
        scsiDevInfo->readCapData.rc16MaxLBA = M_BytesTo8ByteValue(readCapacityData[0], readCapacityData[1], readCapacityData[2], readCapacityData[3], readCapacityData[4], readCapacityData[5], readCapacityData[6], readCapacityData[7]);
        scsiDevInfo->readCapData.rc16blockSize = M_BytesTo4ByteValue(readCapacityData[8], readCapacityData[9], readCapacityData[10], readCapacityData[11]);
        scsiDevInfo->readCapData.rc16physBlockSize = C_CAST(uint32_t, power_Of_Two(M_Nibble0(readCapacityData[13]))) * scsiDevInfo->readCapData.rc16blockSize;
        printf("\tRead Capacity 16 data:\n");
        printf("\t\tMaxLBA: %" PRIu64 "\n", scsiDevInfo->readCapData.rc16MaxLBA);
        printf("\t\tLogical Block Size: %" PRIu32 "\n", scsiDevInfo->readCapData.rc16blockSize);
        printf("\t\tPhysical Block Size: %" PRIu32 "\n", scsiDevInfo->readCapData.rc16physBlockSize);
        device->drive_info.deviceBlockSize = scsiDevInfo->readCapData.rc16blockSize;
        device->drive_info.deviceMaxLba = scsiDevInfo->readCapData.rc16MaxLBA;
        device->drive_info.devicePhyBlockSize = scsiDevInfo->readCapData.rc16physBlockSize;
    }
    else
    {
        set_Console_Colors(true, WARNING_COLOR);
        printf("WARNING: Failed read capacity 16. This should only happen on old legacy SPC & earlier devices.\n");
        printf("         All new devices SHOULD support this command to report PI type (if any) and logical\n");
        printf("         to physical block relationships so that read/write can be properly aligned for the device.\n");
        printf("         this command also reports if logical block provisioning management is enabled and whether\n");
        printf("         or not zeros are reported when reading an unmapped LBA.\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }

    if (readCap16Result == SUCCESS && readCap10Result == SUCCESS)
    {
        if (scsiDevInfo->readCapData.rc16blockSize != scsiDevInfo->readCapData.rc10blockSize)
        {
            set_Console_Colors(true, ERROR_COLOR);
            printf("ERROR: Block length does not match between read capacity commands!\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
        if (scsiDevInfo->readCapData.rc10MaxLBA != UINT32_MAX && scsiDevInfo->readCapData.rc10MaxLBA != scsiDevInfo->readCapData.rc16MaxLBA)
        {
            set_Console_Colors(true, ERROR_COLOR);
            printf("ERROR: Max LBA does not match between read capacity commands!\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
    }
    return SUCCESS;
}

static eReturnValues use_Mode_Sense_6(tDevice * device, uint8_t pageCode, bool *use6Byte)
{
    eReturnValues ret = SUCCESS;
    if (!use6Byte)
    {
        return BAD_PARAMETER;
    }
    //TODO: This code currently always sets DBD, which is fine for SCSI2+, but not ok for earlier devices as that bit wasn't even a thought.
    if (SUCCESS != scsi_Mode_Sense_10(device, pageCode, 0, 0, false, false, MPC_CURRENT_VALUES, NULL))
    {
        //if invalid operation code, try again with 6 byte command.
        bool tryAnotherPage = false;
        senseDataFields senseFields;
        memset(&senseFields, 0, sizeof(senseDataFields));
        get_Sense_Data_Fields(device->drive_info.lastCommandSenseData, SPC3_SENSE_LEN, &senseFields);
        if (senseFields.scsiStatusCodes.senseKey == SENSE_KEY_ILLEGAL_REQUEST && senseFields.scsiStatusCodes.asc == 0x20 && senseFields.scsiStatusCodes.ascq == 0x00)
        {
            //didn't like the operation code, so retrying with mode sense 6
            if (SUCCESS == scsi_Mode_Sense_6(device, pageCode, 0, 0, false, MPC_CURRENT_VALUES, NULL))
            {
                *use6Byte = true;
            }
            else
            {
                get_Sense_Data_Fields(device->drive_info.lastCommandSenseData, SPC3_SENSE_LEN, &senseFields);
                if (senseFields.scsiStatusCodes.senseKey == SENSE_KEY_ILLEGAL_REQUEST && senseFields.scsiStatusCodes.asc == 0x20 && senseFields.scsiStatusCodes.ascq == 0x00)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: This device does not appear to support either mode sense 10 or mode sense 6!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    return FAILURE;
                }
                else
                {
                    tryAnotherPage = true;
                }
            }
        }
        if (tryAnotherPage)
        {
            //Need to try a different page since it is likely a unsupported page issue now.
            if (pageCode != 0x01)
            {
                return use_Mode_Sense_6(device, 0x01, use6Byte);//trying page 0x01 since any device type SHOULD have that page. For direct access block, it is the RW error recovery page
            }
            else
            {
                ret = FAILURE;//command is not supported at all
            }
        }
    }
    if (pageCode == 0)
    {
        //this is to handle legacy devices that only support vendor specific page 0 with a different return status.
        ret = NOT_SUPPORTED;
    }
    return ret;
}

static eReturnValues get_SCSI_Mode_Page_Data(tDevice * device, uint8_t pageCode, uint8_t subPageCode, bool sixByte, uint8_t **dataBuffer /*to allow for reallocation/resize if necessary to get the full page*/, uint32_t *dataBufferLength)
{
    eReturnValues ret = SUCCESS;
    memset(*dataBuffer, 0, *dataBufferLength);
    if (sixByte)
    {
        ret = scsi_Mode_Sense_6(device, pageCode, C_CAST(uint8_t, M_Min(*dataBufferLength, UINT8_MAX)), subPageCode, false, MPC_CURRENT_VALUES, *dataBuffer);
        if (ret == SUCCESS)
        {
            //uint8_t modeDataLength = (*dataBuffer)[0];
            uint8_t blockDescriptorLength = (*dataBuffer)[3];
            uint8_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
            uint8_t readpageCode = M_GETBITRANGE((*dataBuffer)[offset + 0], 5, 0);
            uint8_t readsubpage = 0;
            bool subpageFormat = M_ToBool((*dataBuffer)[offset + 0] & BIT6);
            uint16_t pageLength = (*dataBuffer)[offset + 1];
            uint16_t pageLengthValidation = 2;
            if (subpageFormat)
            {
                readsubpage = (*dataBuffer)[offset + 1];
                pageLength = M_BytesTo2ByteValue((*dataBuffer)[offset + 2], (*dataBuffer)[offset + 3]);
                pageLengthValidation = 4;
            }
            if (readpageCode != pageCode)
            {
                set_Console_Colors(true, ERROR_COLOR);
                printf("ERROR: Incorrect page returned from mode sense 6. Expected %02" PRIX8 "h, but got %02" PRIX8 "h\n", pageCode, readpageCode);
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                ret = FAILURE;
            }
            else if (subpageFormat && subPageCode != readsubpage)
            {
                set_Console_Colors(true, ERROR_COLOR);
                printf("ERROR: Incorrect subpage returned from mode sense 6. Expected %02" PRIX8 "h, but got %02" PRIX8 "h\n", subPageCode, readsubpage);
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                ret = FAILURE;
            }
            else if (subPageCode != 0 && !subpageFormat)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: NMSP\n");//mode page subpages are not supported by this device and it does not properly validate all fields of the CDB
                device->drive_info.passThroughHacks.scsiHacks.noModeSubPages = true;
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            else
            {
                //Everything looks ok, so double check the datalength
                pageLengthValidation += C_CAST(uint8_t, pageLength + MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength);
                if (*dataBufferLength < pageLengthValidation)
                {
                    //reallocate enough data and reread the page.
                    uint8_t *temp = C_CAST(uint8_t *, realloc_aligned(*dataBuffer, *dataBufferLength, pageLengthValidation, device->os_info.minimumAlignment));
                    if (temp)
                    {
                        *dataBuffer = temp;
                        *dataBufferLength = pageLengthValidation;
                        return get_SCSI_Mode_Page_Data(device, pageCode, subPageCode, sixByte, dataBuffer, dataBufferLength);
                    }
                    else
                    {
                        return MEMORY_FAILURE;
                    }
                }
            }
        }
    }
    else
    {
        ret = scsi_Mode_Sense_10(device, pageCode, *dataBufferLength, subPageCode, false, false, MPC_CURRENT_VALUES, *dataBuffer);
        if (ret == SUCCESS)
        {
            //uint16_t modeDataLength = M_BytesTo2ByteValue((*dataBuffer)[0], (*dataBuffer)[1]);
            uint16_t blockDescriptorLength = M_BytesTo2ByteValue((*dataBuffer)[6], (*dataBuffer)[7]);
            uint16_t offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
            uint8_t readpageCode = M_GETBITRANGE((*dataBuffer)[offset + 0], 5, 0);
            uint8_t readsubpage = 0;
            bool subpageFormat = M_ToBool((*dataBuffer)[offset + 0] & BIT6);
            uint16_t pageLength = (*dataBuffer)[offset + 1];
            uint16_t pageLengthValidation = 2;
            if (subpageFormat)
            {
                readsubpage = (*dataBuffer)[offset + 1];
                pageLength = M_BytesTo2ByteValue((*dataBuffer)[offset + 2], (*dataBuffer)[offset + 3]);
                pageLengthValidation = 4;
            }
            if (readpageCode != pageCode)
            {
                set_Console_Colors(true, ERROR_COLOR);
                printf("ERROR: Incorrect page returned from mode sense 6. Expected %02" PRIX8 "h, but got %02" PRIX8 "h\n", pageCode, readpageCode);
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                ret = FAILURE;
            }
            else if (subpageFormat && subPageCode != readsubpage)
            {
                set_Console_Colors(true, ERROR_COLOR);
                printf("ERROR: Incorrect subpage returned from mode sense 6. Expected %02" PRIX8 "h, but got %02" PRIX8 "h\n", subPageCode, readsubpage);
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                ret = FAILURE;
            }
            else if (subPageCode != 0 && !subpageFormat)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: NMSP\n");//mode page subpages are not supported by this device and it does not properly validate all fields of the CDB
                device->drive_info.passThroughHacks.scsiHacks.noModeSubPages = true;
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            else
            {
                //Everything looks ok, so double check the datalength
                pageLengthValidation += C_CAST(uint8_t, pageLength + MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength);
                if (*dataBufferLength < pageLengthValidation)
                {
                    //reallocate enough data and reread the page.
                    uint8_t *temp = C_CAST(uint8_t *, realloc_aligned(*dataBuffer, *dataBufferLength, pageLengthValidation, device->os_info.minimumAlignment));
                    if (temp)
                    {
                        *dataBuffer = temp;
                        *dataBufferLength = pageLengthValidation;
                        return get_SCSI_Mode_Page_Data(device, pageCode, subPageCode, sixByte, dataBuffer, dataBufferLength);
                    }
                    else
                    {
                        return MEMORY_FAILURE;
                    }
                }
            }
        }
    }
    return ret;
}

//TODO: Validate or check for default, changable, and saved values? Only checking current right now - TJE
static eReturnValues scsi_Mode_Information(tDevice *device, ptrScsiDevInformation scsiDevInfo)
{
    bool successfullyReadAtLeastOnePage = false;
    bool use6Byte = false;
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n==========================\n");
    printf("Checking Mode Page Support\n");
    printf("==========================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    //Attempting mode sense 10 since nearly EVERYTHING should support it. The only exception is REALLY old SCSI drives.
    eReturnValues sixTest = use_Mode_Sense_6(device, MP_CONTROL, &use6Byte);
    if (SUCCESS != sixTest)
    {
        if (use6Byte)
        {
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: MP6\n");
            device->drive_info.passThroughHacks.scsiHacks.mode6bytes = true;
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            //TODO: add this to the passthrough hacks structure.
        }
        if (sixTest == NOT_SUPPORTED)
        {
            set_Console_Colors(true, WARNING_COLOR);
            printf("WARNING: This device does not seem to support any standard mode pages. Skipping all mode page checks\n");
            printf("       This should only happen on SCSI (1) and earlier (SASI) devices!\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            return NOT_SUPPORTED;
        }
        else
        {
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: NMP\n");
            device->drive_info.passThroughHacks.scsiHacks.noModePages = true;
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            set_Console_Colors(true, ERROR_COLOR);
            printf("ERROR: This device doesn't support ANY mode sense commands. Mode sense commands can convey device support and change\n");
            printf("       device settings/capabilties. This can be especially important for disabling write caching for backups.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
        return sixTest;
    }
    uint8_t modeHeaderLength = MODE_PARAMETER_HEADER_10_LEN;
    if (use6Byte)
    {
        modeHeaderLength = MODE_PARAMETER_HEADER_6_LEN;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: MP6\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.scsiHacks.mode6bytes = true;
    }

    uint32_t commonModeDataLength = modeHeaderLength + 4 + 8;//Header length + 4 bytes to check initial size of a mode page + 8 bytes for typical short block descriptor.
    uint32_t modeDataLength = 0;
    uint8_t *modeData = NULL;
    //control mode page
    modeDataLength = MP_CONTROL_LEN + commonModeDataLength;
    modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (!modeData)
    {
        return MEMORY_FAILURE;
    }
    if (SUCCESS == get_SCSI_Mode_Page_Data(device, MP_CONTROL, 0, use6Byte, &modeData, &modeDataLength))
    {
        uint16_t blockDescriptorLength = modeData[3];
        uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
        //bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        //uint16_t pageLength = modeData[offset + 1];
        if (!use6Byte)
        {
            blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
            offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
            //subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        }
        //if (subpageFormat)
        //{
        //    pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
        //}
        //now save the fields we care about
        printf("Control Mode Page\n");
        successfullyReadAtLeastOnePage = true;
        scsiDevInfo->modeData.gotControlPage = true;
        scsiDevInfo->modeData.controlData.d_sense = M_ToBool(modeData[offset + 2] & BIT2);
        printf("\tD_Sense = ");
        if (scsiDevInfo->modeData.controlData.d_sense)
        {
            printf("1\n");
        }
        else
        {
            printf("0\n");
        }
        scsiDevInfo->modeData.controlData.extDSTCompletionTimeSeconds = M_BytesTo2ByteValue(modeData[offset + 10], modeData[offset + 11]);
        printf("\tExt DST Completion Time (Seconds): %" PRIu16 "\n", scsiDevInfo->modeData.controlData.extDSTCompletionTimeSeconds);
        safe_Free_aligned(modeData)

        //control extension mode page (not 6 byte, and check if it reports correctly)
        modeDataLength = MP_CONTROL_EXTENSION_LEN + commonModeDataLength;
        modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
        if (!modeData)
        {
            return MEMORY_FAILURE;
        }
        if (SUCCESS == get_SCSI_Mode_Page_Data(device, MP_CONTROL, 0x01, use6Byte, &modeData, &modeDataLength))
        {
            blockDescriptorLength = modeData[3];
            offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
            //bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
            //uint16_t pageLength = modeData[offset + 1];
            if (!use6Byte)
            {
                blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
                offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
                //subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
            }
            //if (subpageFormat)
            //{
            //    pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
            //}
            //now save the fields we care about
            printf("Control Extension Mode Page\n");
            successfullyReadAtLeastOnePage = true;
            scsiDevInfo->modeData.gotControlExtensionPage = true;
            scsiDevInfo->modeData.controlExtData.maxSenseDataLength = modeData[offset + 6];
            printf("\tMaximum Sense Data Length: %" PRIu8 "\n", scsiDevInfo->modeData.controlExtData.maxSenseDataLength);
        }
        safe_Free_aligned(modeData)
    }
    safe_Free_aligned(modeData)

    //read write error recovery mode page
    modeDataLength = MP_READ_WRITE_ERROR_RECOVERY_LEN + commonModeDataLength;
    modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (!modeData)
    {
        return MEMORY_FAILURE;
    }
    if (SUCCESS == get_SCSI_Mode_Page_Data(device, MP_READ_WRITE_ERROR_RECOVERY, 0, use6Byte, &modeData, &modeDataLength))
    {
        uint16_t blockDescriptorLength = modeData[3];
        uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
        //bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        //uint16_t pageLength = modeData[offset + 1];
        if (!use6Byte)
        {
            blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
            offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
            //subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        }
        //if (subpageFormat)
        //{
        //    pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
        //}
        //now save the fields we care about
        scsiDevInfo->modeData.gotReadWriteErrorRecoveryPage = true;
        printf("Read Write Error Recovery Mode Page\n");
        successfullyReadAtLeastOnePage = true;
        scsiDevInfo->modeData.rwErrRecData.awre = M_ToBool(modeData[offset + 2] & BIT7);
        printf("\tAutomatic Write Reallocation: ");
        if (scsiDevInfo->modeData.rwErrRecData.awre)
        {
            printf("Enabled\n");
        }
        else
        {
            printf("Disabled\n");
        }
        scsiDevInfo->modeData.rwErrRecData.arre = M_ToBool(modeData[offset + 2] & BIT6);
        printf("\tAutomatic Read Reallocation: ");
        if (scsiDevInfo->modeData.rwErrRecData.arre)
        {
            printf("Enabled\n");
        }
        else
        {
            printf("Disabled\n");
        }
        scsiDevInfo->modeData.rwErrRecData.recoveryTimeLimit = M_BytesTo2ByteValue(modeData[offset + 10], modeData[offset + 11]);
        printf("\tRecovery Time Limit: %" PRIu16 "\n", scsiDevInfo->modeData.rwErrRecData.recoveryTimeLimit);
    }
    safe_Free_aligned(modeData)
    //caching mode page
    modeDataLength = MP_CACHING_LEN + commonModeDataLength;
    modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (!modeData)
    {
        return MEMORY_FAILURE;
    }
    if (SUCCESS == get_SCSI_Mode_Page_Data(device, MP_CACHING, 0, use6Byte, &modeData, &modeDataLength))
    {
        uint16_t blockDescriptorLength = modeData[3];
        uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
        bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        uint16_t pageLength = modeData[offset + 1];
        if (!use6Byte)
        {
            blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
            offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
            subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        }
        if (subpageFormat)
        {
            pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
        }
        //now save the fields we care about
        scsiDevInfo->modeData.gotCachingPage = true;
        printf("Caching Mode Page\n");
        successfullyReadAtLeastOnePage = true;
        scsiDevInfo->modeData.cachingData.wce = M_ToBool(modeData[offset + 2] & BIT2);
        printf("\tWrite Cache: ");
        if (scsiDevInfo->modeData.cachingData.wce)
        {
            printf("Enabled\n");
        }
        else
        {
            printf("Disabled\n");
        }
        if (pageLength >= 0x0A) //checking this for SCSI 2 compatibility
        {
            scsiDevInfo->modeData.cachingData.dra = M_ToBool(modeData[offset + 12] & BIT5);
            printf("\tRead Ahead: ");
            if (scsiDevInfo->modeData.cachingData.dra)
            {
                printf("Disabled\n");
            }
            else
            {
                printf("Enabled\n");
            }
        }
    }
    safe_Free_aligned(modeData)

    //rigid disk geometry page
    modeDataLength = MP_RIGID_DISK_GEOMETRY_LEN + commonModeDataLength;
    modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (!modeData)
    {
        return MEMORY_FAILURE;
    }
    if (SUCCESS == get_SCSI_Mode_Page_Data(device, MP_RIGID_DISK_GEOMETRY, 0, use6Byte, &modeData, &modeDataLength))
    {
        uint16_t blockDescriptorLength = modeData[3];
        uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
        //bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        //uint16_t pageLength = modeData[offset + 1];
        if (!use6Byte)
        {
            blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
            offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
            //subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        }
        //if (subpageFormat)
        //{
        //   pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
        //}
        //now save the fields we care about
        scsiDevInfo->modeData.gotRigidDiskPage = true;
        printf("Rigid Disk Geometry Mode Page\n");
        successfullyReadAtLeastOnePage = true;
        scsiDevInfo->modeData.rigidDiskData.numberOfCylinders = M_BytesTo4ByteValue(0, modeData[offset + 2], modeData[offset + 3], modeData[offset + 4]);
        printf("\tNumber Of Cylinders: %" PRIu32 "\n", scsiDevInfo->modeData.rigidDiskData.numberOfCylinders);
        scsiDevInfo->modeData.rigidDiskData.numberOfHeads = modeData[offset + 5];
        printf("\tNumber Of Heads: %" PRIu8 "\n", scsiDevInfo->modeData.rigidDiskData.numberOfHeads);
        scsiDevInfo->modeData.rigidDiskData.startingCyliderWritePrecompensation = M_BytesTo4ByteValue(0, modeData[offset + 6], modeData[offset + 7], modeData[offset + 8]);
        printf("\tStarting Cylinder Write Precompensation: %" PRIu32 "\n", scsiDevInfo->modeData.rigidDiskData.startingCyliderWritePrecompensation);
        scsiDevInfo->modeData.rigidDiskData.startingCylinerReducedWriteCurrent = M_BytesTo4ByteValue(0, modeData[offset + 9], modeData[offset + 10], modeData[offset + 11]);
        printf("\tStarting Cylinder Reduced Write Current: %" PRIu32 "\n", scsiDevInfo->modeData.rigidDiskData.startingCylinerReducedWriteCurrent);
        scsiDevInfo->modeData.rigidDiskData.driveStepRate = M_BytesTo2ByteValue(modeData[offset + 12], modeData[offset + 13]);
        printf("\tDrive Step Rate: %" PRIu16 "\n", scsiDevInfo->modeData.rigidDiskData.driveStepRate);
        scsiDevInfo->modeData.rigidDiskData.landingZoneCylinder = M_BytesTo4ByteValue(0, modeData[offset + 14], modeData[offset + 15], modeData[offset + 16]);
        printf("\tLanding Zone Cylinder: %" PRIu32 "\n", scsiDevInfo->modeData.rigidDiskData.landingZoneCylinder);
        scsiDevInfo->modeData.rigidDiskData.rpl = M_GETBITRANGE(modeData[offset + 17], 1, 0);
        printf("\tRotational Position Locking: ");
        switch (scsiDevInfo->modeData.rigidDiskData.rpl)
        {
        case 0:
            printf("Spindle Synchronization Disabled Or Not Supported\n");
            break;
        case 1:
            printf("Operates As A Synchronized-Spindle Slave\n");
            break;
        case 2:
            printf("Operates As A Synchronized-Spindle Master\n");
            break;
        case 3:
            printf("Operates As A Synchronized-Spindle Master Control\n");
            break;
        default:
            printf("Unknown\n");
            break;
        }
        scsiDevInfo->modeData.rigidDiskData.rotationalOffset = modeData[offset + 18];
        printf("\tRotational Offset: %0.02f Revolution Skew\n", C_CAST(double, scsiDevInfo->modeData.rigidDiskData.rotationalOffset) / 256.0);
        scsiDevInfo->modeData.rigidDiskData.mediumRotationRate = M_BytesTo2ByteValue(modeData[offset + 20], modeData[offset + 21]);
        printf("\tMedium Rotation Rate: ");
        switch (scsiDevInfo->modeData.rigidDiskData.mediumRotationRate)
        {
        case 0:
            printf("Not Reported\n");
            break;
        case 1:
            printf("SSD\n");
            break;
        default:
            printf("%" PRIu16 "RPM\n", scsiDevInfo->modeData.rigidDiskData.mediumRotationRate);
            break;
        }
    }
    safe_Free_aligned(modeData)

    //informational exceptions mode page
    modeDataLength = MP_INFORMATION_EXCEPTIONS_LEN + commonModeDataLength;
    modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (!modeData)
    {
        return MEMORY_FAILURE;
    }
    if (SUCCESS == get_SCSI_Mode_Page_Data(device, MP_INFORMATION_EXCEPTIONS_CONTROL, 0, use6Byte, &modeData, &modeDataLength))
    {
        uint16_t blockDescriptorLength = modeData[3];
        uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
        //bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        //uint16_t pageLength = modeData[offset + 1];
        if (!use6Byte)
        {
            blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
            offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
            //subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        }
        //if (subpageFormat)
        //{
        //    pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
        //}
        //now save the fields we care about
        scsiDevInfo->modeData.gotInformationalExceptionsControlPage = true;
        printf("Informational Exceptions Control Mode Page\n");
        successfullyReadAtLeastOnePage = true;
        scsiDevInfo->modeData.infoExcepData.perf = M_ToBool(modeData[offset + 2] & BIT7);
        printf("\tPERF: ");
        if (scsiDevInfo->modeData.infoExcepData.perf)
        {
            printf("1\n");
        }
        else
        {
            printf("0\n");
        }
        scsiDevInfo->modeData.infoExcepData.ewasc = M_ToBool(modeData[offset + 2] & BIT4);
        printf("\tEWASC: ");
        if (scsiDevInfo->modeData.infoExcepData.ewasc)
        {
            printf("1\n");
        }
        else
        {
            printf("0\n");
        }
        scsiDevInfo->modeData.infoExcepData.dexcpt = M_ToBool(modeData[offset + 2] & BIT3);
        printf("\tDEXCPT: ");
        if (scsiDevInfo->modeData.infoExcepData.dexcpt)
        {
            printf("1\n");
        }
        else
        {
            printf("0\n");
        }
        scsiDevInfo->modeData.infoExcepData.mrie = M_Nibble0(modeData[offset + 3]);
        printf("\tMethod Of Reporting Informational Exceptions (MRIE): %" PRIX8 "h\n", scsiDevInfo->modeData.infoExcepData.mrie);
    }
    safe_Free_aligned(modeData)
    //power condition control mode page
    modeDataLength = MP_POWER_CONDITION_LEN + commonModeDataLength;
    modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (!modeData)
    {
        return MEMORY_FAILURE;
    }
    if (SUCCESS == get_SCSI_Mode_Page_Data(device, MP_POWER_CONDTION, 0, use6Byte, &modeData, &modeDataLength))
    {
        uint16_t blockDescriptorLength = modeData[3];
        uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
        bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        uint16_t pageLength = modeData[offset + 1];
        if (!use6Byte)
        {
            blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
            offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
            subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
        }
        if (subpageFormat)
        {
            pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
        }
        //now save the fields we care about
        scsiDevInfo->modeData.gotPowerConditionControlPage = true;
        printf("Power Condition Control Mode Page\n");
        successfullyReadAtLeastOnePage = true;
        scsiDevInfo->modeData.powerConditionCtrlData.standbyY = M_ToBool(modeData[offset + 2] & BIT0);
        scsiDevInfo->modeData.powerConditionCtrlData.standbyZ = M_ToBool(modeData[offset + 3] & BIT0);
        scsiDevInfo->modeData.powerConditionCtrlData.idleA = M_ToBool(modeData[offset + 3] & BIT1);
        scsiDevInfo->modeData.powerConditionCtrlData.idleB = M_ToBool(modeData[offset + 3] & BIT2);
        scsiDevInfo->modeData.powerConditionCtrlData.idleC = M_ToBool(modeData[offset + 3] & BIT3);
        scsiDevInfo->modeData.powerConditionCtrlData.idleATimer = M_BytesTo4ByteValue(modeData[offset + 4], modeData[offset + 5], modeData[offset + 6], modeData[offset + 7]);
        scsiDevInfo->modeData.powerConditionCtrlData.standbyZTimer = M_BytesTo4ByteValue(modeData[offset + 8], modeData[offset + 9], modeData[offset + 10], modeData[offset + 11]);
        if (scsiDevInfo->modeData.powerConditionCtrlData.standbyZ)
        {
            printf("\tStandbyZ Timer: %" PRIu32 "\n", scsiDevInfo->modeData.powerConditionCtrlData.standbyZTimer);
        }
        if (scsiDevInfo->modeData.powerConditionCtrlData.idleA)
        {
            printf("\tIdleA Timer: %" PRIu32 "\n", scsiDevInfo->modeData.powerConditionCtrlData.idleATimer);
        }
        if (pageLength > 0x0A) //need to check this for backwards compatibility since this page existed before all the other timers.
        {
            scsiDevInfo->modeData.powerConditionCtrlData.idleBTimer = M_BytesTo4ByteValue(modeData[offset + 12], modeData[offset + 13], modeData[offset + 14], modeData[offset + 15]);
            scsiDevInfo->modeData.powerConditionCtrlData.idleCTimer = M_BytesTo4ByteValue(modeData[offset + 16], modeData[offset + 17], modeData[offset + 18], modeData[offset + 19]);
            scsiDevInfo->modeData.powerConditionCtrlData.standbyYTimer = M_BytesTo4ByteValue(modeData[offset + 20], modeData[offset + 21], modeData[offset + 22], modeData[offset + 23]);
            if (scsiDevInfo->modeData.powerConditionCtrlData.idleB)
            {
                printf("\tIdleB Timer: %" PRIu32 "\n", scsiDevInfo->modeData.powerConditionCtrlData.idleBTimer);
            }
            if (scsiDevInfo->modeData.powerConditionCtrlData.idleC)
            {
                printf("\tIdleC Timer: %" PRIu32 "\n", scsiDevInfo->modeData.powerConditionCtrlData.idleCTimer);
            }
            if (scsiDevInfo->modeData.powerConditionCtrlData.standbyY)
            {
                printf("\tStandbyY Timer: %" PRIu32 "\n", scsiDevInfo->modeData.powerConditionCtrlData.standbyYTimer);
            }
        }
    }
    safe_Free_aligned(modeData)
    //TODO: The next 2 are ATA specific. Attempt to only read them when we suspect an ATA drive.
    //if () //ATA AND the passthrough hack for not supporting subpages is NOT set
    {
        //if () //TODO: Only read this if we suspect a PATA drive. Base this off of existance of ATA VPD page and PATA signature vs SATA signature
        {
            //pata control mode page - only read if the device could be a PATA drive.
            modeDataLength = 8 + commonModeDataLength;
            modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
            if (!modeData)
            {
                return MEMORY_FAILURE;
            }
            if (SUCCESS == get_SCSI_Mode_Page_Data(device, 0x0A, 0xF1, use6Byte, &modeData, &modeDataLength))
            {
                uint16_t blockDescriptorLength = modeData[3];
                uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
                //bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
                //uint16_t pageLength = modeData[offset + 1];
                if (!use6Byte)
                {
                    blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
                    offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
                    //subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
                }
                //if (subpageFormat)
                //{
                //    pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
                //}
                //now save the fields we care about
                scsiDevInfo->modeData.gotPataControlPage = true;
                printf("PATA Control Mode Page\n");
                successfullyReadAtLeastOnePage = true;
                scsiDevInfo->modeData.pataCtrlData.pio3 = M_ToBool(modeData[offset + 4] & BIT0);
                scsiDevInfo->modeData.pataCtrlData.pio4 = M_ToBool(modeData[offset + 4] & BIT1);
                scsiDevInfo->modeData.pataCtrlData.mwd0 = M_ToBool(modeData[offset + 4] & BIT4);
                scsiDevInfo->modeData.pataCtrlData.mwd1 = M_ToBool(modeData[offset + 4] & BIT5);
                scsiDevInfo->modeData.pataCtrlData.mwd2 = M_ToBool(modeData[offset + 4] & BIT6);
                scsiDevInfo->modeData.pataCtrlData.udma0 = M_ToBool(modeData[offset + 5] & BIT0);
                scsiDevInfo->modeData.pataCtrlData.udma1 = M_ToBool(modeData[offset + 5] & BIT1);
                scsiDevInfo->modeData.pataCtrlData.udma2 = M_ToBool(modeData[offset + 5] & BIT2);
                scsiDevInfo->modeData.pataCtrlData.udma3 = M_ToBool(modeData[offset + 5] & BIT3);
                scsiDevInfo->modeData.pataCtrlData.udma4 = M_ToBool(modeData[offset + 5] & BIT4);
                scsiDevInfo->modeData.pataCtrlData.udma5 = M_ToBool(modeData[offset + 5] & BIT5);
                scsiDevInfo->modeData.pataCtrlData.udma6 = M_ToBool(modeData[offset + 5] & BIT6);
            }
            safe_Free_aligned(modeData)
        }
        //ata power condition mode page
        modeDataLength = 16 + commonModeDataLength;
        modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
        if (!modeData)
        {
            return MEMORY_FAILURE;
        }
        if (SUCCESS == get_SCSI_Mode_Page_Data(device, 0x1A, 0xF1, use6Byte, &modeData, &modeDataLength))
        {
            uint16_t blockDescriptorLength = modeData[3];
            uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
            //bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
            //uint16_t pageLength = modeData[offset + 1];
            if (!use6Byte)
            {
                blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
                offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
                //subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
            }
            //if (subpageFormat)
            //{
            //    pageLength = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
            //}
            //now save the fields we care about
            scsiDevInfo->modeData.gotATAPowerConditionPage = true;
            printf("ATA Power Condition Mode Page\n");
            successfullyReadAtLeastOnePage = true;
            scsiDevInfo->modeData.ataPwrConditionData.apmp = M_ToBool(modeData[offset + 5] & BIT0);
            printf("\tAPMP: ");
            if (scsiDevInfo->modeData.ataPwrConditionData.apmp)
            {
                printf("1\n");
            }
            else
            {
                printf("0\n");
            }
            scsiDevInfo->modeData.ataPwrConditionData.apmValue = modeData[offset + 6];
            printf("\tAPM Value: %" PRIX8 "h\n", scsiDevInfo->modeData.ataPwrConditionData.apmValue);
        }
        safe_Free_aligned(modeData)
    }
    //Check for vendor specific page 0? May help detect true SCSI devices, but nothing says a translator cannot implement it.
    modeDataLength = UINT8_MAX;//try this size since it's unlikely this page will be this size, but it should be more than enough memory.
    modeData = C_CAST(uint8_t *, calloc_aligned(modeDataLength, sizeof(uint8_t), device->os_info.minimumAlignment));
    if (!modeData)
    {
        return MEMORY_FAILURE;
    }
    if (SUCCESS == get_SCSI_Mode_Page_Data(device, 0, 0, use6Byte, &modeData, &modeDataLength))
    {
//      uint16_t blockDescriptorLength = modeData[3];
//      uint16_t offset = MODE_PARAMETER_HEADER_6_LEN + blockDescriptorLength;
//      bool subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
//      uint16_t pageLength = modeData[offset + 1];
//      if (!use6Byte)
//      {
//          blockDescriptorLength = M_BytesTo2ByteValue(modeData[6], modeData[7]);
//          offset = MODE_PARAMETER_HEADER_10_LEN + blockDescriptorLength;
//          subpageFormat = M_ToBool(modeData[offset + 0] & BIT6);
//      }
//      if (subpage)
//      {
//          subpageFormat = M_BytesTo2ByteValue(modeData[offset + 2], modeData[offset + 3]);
//      }
        //now save the fields we care about
        scsiDevInfo->modeData.gotVendorUniquePage0 = true;
        printf("Vendor Specific Mode Page 0\n");
        //just saving that we did get this page. Nothing else is needed if this worked.
    }
    if (!successfullyReadAtLeastOnePage)
    {
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: NMP\n");
        device->drive_info.passThroughHacks.scsiHacks.noModePages = true;
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        set_Console_Colors(true, WARNING_COLOR);
        printf("WARNING: This device does not seem to support any standard mode pages. Multiple pages were attempted, but none were read successfully.\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        return NOT_SUPPORTED;
    }
    safe_Free_aligned(modeData)
    return SUCCESS;
}

//TODO: we can clean up the loops in each case to most likely a single loop somewhere, but will need to figure out a method to save the data fields we care about...-TJE
static eReturnValues scsi_Log_Information(tDevice *device, ptrScsiDevInformation scsiDevInfo)
{
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n==========================\n");
    printf("Testing for SCSI Log Pages\n");
    printf("==========================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    //Make sure on the first log sense, if it fails, we check for invalid operation code. If invalid code, test is over since the command isn't supported.
    //Let the user know about this though!
    uint8_t supportPages[255] = { 0 };
    if (SUCCESS == scsi_Log_Sense_Cmd(device, false, LPC_CUMULATIVE_VALUES, LP_SUPPORTED_LOG_PAGES, 0, 0, supportPages, 255))
    {
        bool hasSubpages = false;
        //we should have the supported logs at this point.
        //Now we need to attempt to read the list of supported subpages as well. WARNING: some device may respond because they don't properly validate reserved fields. Need to catch this!!!
        uint8_t supportedPagesAndSubpages[255] = { 0 };
        if (SUCCESS == scsi_Log_Sense_Cmd(device, false, LPC_CUMULATIVE_VALUES, LP_SUPPORTED_LOG_PAGES_AND_SUBPAGES, 0xFF, 0, supportedPagesAndSubpages, 255))
        {
            //While we got successful status for subpages, we need to validate the data!!!
            if (memcmp(supportPages, supportedPagesAndSubpages, 255) == 0)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: NLPS\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                device->drive_info.passThroughHacks.scsiHacks.noLogSubPages = true;
                printf("This device does NOT report log page subpages properly! Do not attempt to read ANY subpages as it only checks the page code!\n");
            }
            else
            {
                //more analysis required
                //The page code should be zero for both. The subpage code should be set to FF for supportedPagesAndSubpages.
                //The lengths MUST be different. Even if there aren't any supported subpages, the lengths cannot be the same since the descriptors are different sizes!
                if (M_GETBITRANGE(supportPages[0], 5, 0) == M_GETBITRANGE(supportedPagesAndSubpages[0], 5, 0))
                {
                    //So far so good...
                    //now make sure we got a proper subpage code for supported pages and subpages
                    if (supportedPagesAndSubpages[1] == 0xFF && supportedPagesAndSubpages[0] & BIT6)
                    {
                        //finally check that the page length for subpages is AT LEAST twice as long as without subpages. It SHOULD be, but if it isn't we can do one more analysis of the data to see if all pages are being reported correctly or not.
                        uint16_t supportLen = M_BytesTo2ByteValue(supportPages[2], supportPages[3]);
                        uint16_t supportSLen = M_BytesTo2ByteValue(supportedPagesAndSubpages[2], supportedPagesAndSubpages[3]);
                        if (supportSLen >= (2 * supportLen))
                        {
                            //This is good enough to consider subpages supported
                            hasSubpages = true;
                        }
                        else
                        {
                            set_Console_Colors(true, WARNING_COLOR);
                            printf("WARNING: Length of subpages does not appear to make sense. It should be AT LEAST twice as long as without reporting subpages.\n");
                            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                            //printf("         One more level analysis is allowed before this is considered a buggy device that reports incorrectly when asking for subpages.\n");
                            //TODO: final level of analysis...looping and checking to see if a single page is missing or not.
                            //      when completed, uncomment the above printf....if this case actually happens, will need another hack for this device since it reports correctly formatted data, but misses reporting some pages.
                            //      If this issue is found, a creative way to loop through and check both below will be needed. Maybe modify the supportedPagesAndSubpages with all the correct information and length before getting down below???
                            hasSubpages = false;
                        }
                    }
                    else
                    {
                        set_Console_Colors(true, HACK_COLOR);
                        printf("HACK FOUND: NLPS\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        printf("This device reports success from asking for subpage list, BUT doesn't report properly\n");
                        hasSubpages = false;
                        device->drive_info.passThroughHacks.scsiHacks.noLogSubPages = true;
                    }
                }
                else if (M_GETBITRANGE(supportPages[0], 5, 0) == LP_SUPPORTED_LOG_PAGES)
                {
                    hasSubpages = false;
                }
                else
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: Data looks completely invalid! Cancelling SCSI log page test!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    return FAILURE;
                }
            }
        }
        //now loop through the pages &/subpages supported by the device. Save anything interesting to compare against SAT reported data later.
        uint8_t *logpageSupportPointer = supportPages;
        uint8_t increment = 1;
        if (hasSubpages)
        {
            logpageSupportPointer = supportedPagesAndSubpages;
            increment = 2;
        }
        uint16_t supportedLogPagesLen = M_BytesTo2ByteValue(logpageSupportPointer[2], logpageSupportPointer[3]);
        for (uint32_t lpSupportOffset = 4; lpSupportOffset < C_CAST(uint32_t, (supportedLogPagesLen + 4)) && lpSupportOffset < 255; lpSupportOffset += increment)
        {
            bool genericLogPagePrintout = true;
            bool readLogPage = false;
            uint8_t pageCode = M_GETBITRANGE(logpageSupportPointer[lpSupportOffset], 5, 0);
            uint8_t subPageCode = 0;
            uint16_t logPageLength = 4;//start here to get full size
            uint8_t parameterLength = 0;//to be used as things are parsed below.
            if (hasSubpages)
            {
                subPageCode = logpageSupportPointer[lpSupportOffset + 1];
                printf("\tFound page %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
            }
            else
            {
                printf("\tFound page %02" PRIX8 "h\n", pageCode);
            }
            //TODO: Read the length of the current page/subpage, then read the whole thing. Keep this pointer for the rest of the loop below.
            uint8_t *pageToRead = C_CAST(uint8_t*, calloc_aligned(logPageLength, sizeof(uint8_t), device->os_info.minimumAlignment));

            if (SUCCESS == scsi_Log_Sense_Cmd(device, false, LPC_CUMULATIVE_VALUES, pageCode, subPageCode, 0, pageToRead, logPageLength))
            {
                logPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                uint8_t* temp = realloc_aligned(pageToRead, logPageLength, logPageLength + 4, device->os_info.minimumAlignment);
                if (temp)
                {
                    pageToRead = temp;
                    if (SUCCESS == scsi_Log_Sense_Cmd(device, false, LPC_CUMULATIVE_VALUES, pageCode, subPageCode, 0, pageToRead, logPageLength + 4))
                    {
                        genericLogPagePrintout = false;
                        logPageLength = M_BytesTo2ByteValue(pageToRead[2], pageToRead[3]);
                        //TODO: Also validate the peripheral qualifier and peripheral device type???
                        if (pageToRead[0] & BIT6)
                        {
                            //subpage format
                            if (M_GETBITRANGE(pageToRead[0], 5, 0) != pageCode)
                            {
                                set_Console_Colors(true, ERROR_COLOR);
                                printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", pageCode, C_CAST(uint8_t, M_GETBITRANGE(pageToRead[0], 5, 0)));
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                            }
                            else if (pageToRead[1] != subPageCode)
                            {
                                set_Console_Colors(true, ERROR_COLOR);
                                printf("ERROR: Expected subpage %" PRIX8 "h, but got %" PRIX8 "h\n", subPageCode, pageToRead[1]);
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                            }
                            else
                            {
                                readLogPage = true;
                            }
                        }
                        else
                        {
                            if (M_GETBITRANGE(pageToRead[0], 5, 0) != pageCode)
                            {
                                set_Console_Colors(true, ERROR_COLOR);
                                printf("ERROR: Expected page %" PRIX8 "h, but got %" PRIX8 "h\n", pageCode, C_CAST(uint8_t, M_GETBITRANGE(pageToRead[0], 5, 0)));
                                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                            }
                            else
                            {
                                readLogPage = true;
                            }
                        }
                    }
                    else
                    {
                        set_Console_Colors(true, ERROR_COLOR);
                        printf("ERROR: Failed to read page %02" PRIX8 "h-%02" PRIX8 "h. Was reported as supported, but cannot be read.\n", pageCode, subPageCode);
                        printf("       Attempted to read %02" PRIu16 " bytes as reported by device.\n", logPageLength);
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    }
                }
                else
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: Unable to allocate memory to read %" PRIu16 "B of page  %02" PRIX8 "h-%02" PRIX8 "h\n", logPageLength, pageCode, subPageCode);
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }

            //Loop through the pages. Only looking for direct access block device and host managed zoned block device pages for now. Can expand to others if necessary.
            switch (pageCode)
            {
            case 0x00:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Supported Log Pages\n");
                    break;
                case 0xFF:
                    printf("Supported Log Pages and Subpages\n");
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x01:
                switch (subPageCode)
                {
                case 0:
                    printf("Buffer Over-Run/Under-Run\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x02:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Write Error Counters\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        bool genericCounterOutput = true;
                        switch (parameterCode)
                        {
                        case 0x0000://Errors corrected without substantial delay
                            printf("\tErrors corrected without substantial delay: ");
                            break;
                        case 0x0001://Errors corrected with possible delays
                            printf("\tErrors corrected with possible delays: ");
                            break;
                        case 0x0002://total (eg rewrites or rereads)
                            printf("\tTotal (e.g. rewites or rereads): ");
                            break;
                        case 0x0003://total errors corrected
                            printf("\tTotal Errors Corrected: ");
                            break;
                        case 0x0004://total times correction algorithm processed
                            printf("\tTotal Times Correction Algorithm Processed: ");
                            break;
                        case 0x0005://total bytes processed
                            printf("\tTotal Bytes Processed: ");
                            break;
                        case 0x0006://total uncorrected errors
                            printf("\tTotal Uncorrected Errors: ");
                            break;
                        default:
                            if (parameterCode < 0x8000)
                            {
                                printf("Unknown Write Error Counter parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            else
                            {
                                printf("Vendor Specific Write Error Counter parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            genericCounterOutput = false;
                            break;
                        }
                        if (genericCounterOutput)
                        {
                            switch (parameterLength)
                            {
                            case 0:
                                break;
                            case 1:
                                printf("%" PRIu8 "\n", pageToRead[offset + 5]);
                                break;
                            case 2:
                                printf("%" PRIu16 "\n", M_BytesTo2ByteValue(pageToRead[offset + 5], pageToRead[offset + 6]));
                                break;
                            case 4:
                                printf("%" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8]));
                                break;
                            case 8:
                                printf("%" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11], pageToRead[offset + 12]));
                                break;
                            default:
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                                break;
                            }
                        }
                    }
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x03:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Read Error Counters\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        bool genericCounterOutput = true;
                        switch (parameterCode)
                        {
                        case 0x0000://Errors corrected without substantial delay
                            printf("\tErrors corrected without substantial delay: ");
                            break;
                        case 0x0001://Errors corrected with possible delays
                            printf("\tErrors corrected with possible delays: ");
                            break;
                        case 0x0002://total (eg rewrites or rereads)
                            printf("\tTotal (e.g. rewites or rereads): ");
                            break;
                        case 0x0003://total errors corrected
                            printf("\tTotal Errors Corrected: ");
                            break;
                        case 0x0004://total times correction algorithm processed
                            printf("\tTotal Times Correction Algorithm Processed: ");
                            //save for sat!
                            switch (parameterLength)
                            {
                            case 1:
                                scsiDevInfo->logData.readErrorData.totalTimesCorrectionAlgorithmProcessed = pageToRead[offset + 5];
                                break;
                            case 2:
                                scsiDevInfo->logData.readErrorData.totalTimesCorrectionAlgorithmProcessed = M_BytesTo2ByteValue(pageToRead[offset + 5], pageToRead[offset + 6]);
                                break;
                            case 4:
                                scsiDevInfo->logData.readErrorData.totalTimesCorrectionAlgorithmProcessed = M_BytesTo4ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8]);
                                break;
                            case 8:
                                scsiDevInfo->logData.readErrorData.totalTimesCorrectionAlgorithmProcessed = M_BytesTo8ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11], pageToRead[offset + 12]);
                                break;
                            }
                            printf("%" PRIu64 "\n", scsiDevInfo->logData.readErrorData.totalTimesCorrectionAlgorithmProcessed);
                            scsiDevInfo->logData.readErrorsFound = true;
                            break;
                        case 0x0005://total bytes processed
                            printf("\tTotal Bytes Processed: ");
                            break;
                        case 0x0006://total uncorrected errors
                            printf("\tTotal Uncorrected Errors: ");
                            //save for sat!
                            switch (parameterLength)
                            {
                            case 1:
                                scsiDevInfo->logData.readErrorData.totalUncorrectedErrors = pageToRead[offset + 5];
                                break;
                            case 2:
                                scsiDevInfo->logData.readErrorData.totalUncorrectedErrors = M_BytesTo2ByteValue(pageToRead[offset + 5], pageToRead[offset + 6]);
                                break;
                            case 4:
                                scsiDevInfo->logData.readErrorData.totalUncorrectedErrors = M_BytesTo4ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8]);
                                break;
                            case 8:
                                scsiDevInfo->logData.readErrorData.totalUncorrectedErrors = M_BytesTo8ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11], pageToRead[offset + 12]);
                                break;
                            }
                            printf("%" PRIu64 "\n", scsiDevInfo->logData.readErrorData.totalUncorrectedErrors);
                            scsiDevInfo->logData.readErrorsFound = true;
                            break;
                        default:
                            if (parameterCode < 0x8000)
                            {
                                printf("Unknown Read Error Counter parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            else
                            {
                                printf("Vendor Specific Read Error Counter parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            genericCounterOutput = false;
                            break;
                        }
                        if (genericCounterOutput)
                        {
                            switch (parameterLength)
                            {
                            case 0:
                                break;
                            case 1:
                                printf("%" PRIu8 "\n", pageToRead[offset + 5]);
                                break;
                            case 2:
                                printf("%" PRIu16 "\n", M_BytesTo2ByteValue(pageToRead[offset + 5], pageToRead[offset + 6]));
                                break;
                            case 4:
                                printf("%" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8]));
                                break;
                            case 8:
                                printf("%" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11], pageToRead[offset + 12]));
                                break;
                            default:
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                                break;
                            }
                        }
                    }
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x05:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Verify Error Counters\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        bool genericCounterOutput = true;
                        switch (parameterCode)
                        {
                        case 0x0000://Errors corrected without substantial delay
                            printf("\tErrors corrected without substantial delay: ");
                            break;
                        case 0x0001://Errors corrected with possible delays
                            printf("\tErrors corrected with possible delays: ");
                            break;
                        case 0x0002://total (eg rewrites or rereads)
                            printf("\tTotal (e.g. rewites or rereads): ");
                            break;
                        case 0x0003://total errors corrected
                            printf("\tTotal Errors Corrected: ");
                            break;
                        case 0x0004://total times correction algorithm processed
                            printf("\tTotal Times Correction Algorithm Processed: ");
                            break;
                        case 0x0005://total bytes processed
                            printf("\tTotal Bytes Processed: ");
                            break;
                        case 0x0006://total uncorrected errors
                            printf("\tTotal Uncorrected Errors: ");
                            break;
                        default:
                            if (parameterCode < 0x8000)
                            {
                                printf("Unknown Verify Error Counter parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            else
                            {
                                printf("Vendor Specific Verify Error Counter parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            genericCounterOutput = false;
                            break;
                        }
                        if (genericCounterOutput)
                        {
                            switch (parameterLength)
                            {
                            case 0:
                                break;
                            case 1:
                                printf("%" PRIu8 "\n", pageToRead[offset + 5]);
                                break;
                            case 2:
                                printf("%" PRIu16 "\n", M_BytesTo2ByteValue(pageToRead[offset + 5], pageToRead[offset + 6]));
                                break;
                            case 4:
                                printf("%" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8]));
                                break;
                            case 8:
                                printf("%" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11], pageToRead[offset + 12]));
                                break;
                            default:
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                                break;
                            }
                        }
                    }
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x06:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Non-Medium Error\n");
                    //TODO: Parse out this page
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x07:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Last n Error Events\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x08:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Format Status\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
                //9 & A are reserved for a different standard. Not applicable to what we're doing so skip them
            case 0x0B:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Last n Deferred Error or Asynchronous Events\n");
                    break;
                case 0x01:
                    printf("Last n Inquiry Data Changed\n");
                    break;
                case 0x02:
                    printf("Last n Mode Page Data Changed\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x0C:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Logical Block Provisioning\n");//If this is a tape drive, then this is sequential access device page! TODO: add an if in case someone runs this against a tape
                    //TODO: Parse out this page
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x0D:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Temperature\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        switch (parameterCode)
                        {
                        case 0x0000://Temperature
                            printf("\tTemperature (C): ");
                            if (pageToRead[offset + 5] == UINT8_MAX)
                            {
                                printf("Invalid reading.\n");
                            }
                            else
                            {
                                printf("%" PRIu8 "\n", pageToRead[offset + 5]);
                            }
                            scsiDevInfo->logData.temperatureData.temperature = pageToRead[offset + 5];
                            scsiDevInfo->logData.gotTemperature = true;
                            break;
                        case 0x0001://Reference Temperature
                            printf("\tReference Temperature (C): ");
                            if (pageToRead[offset + 5] == UINT8_MAX)
                            {
                                printf("Invalid reading.\n");
                            }
                            else
                            {
                                printf("%" PRIu8 "\n", pageToRead[offset + 5]);
                            }
                            scsiDevInfo->logData.temperatureData.referenceTemperature = pageToRead[offset + 5];
                            scsiDevInfo->logData.gotTemperature = true;
                            break;
                        default:
                            if (parameterCode < 0x8000)
                            {
                                printf("Unknown Temperature parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            else
                            {
                                printf("Vendor Specific Temperature parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            break;
                        }
                    }
                    break;
                case 0x01:
                    printf("Environmental Reporting\n");
                    //TODO: Parse out this page
                    break;
                case 0x02:
                    printf("Environmental Limits\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x0E:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Start-Stop Cycle Counter\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        switch (parameterCode)
                        {
                        case 0x0001://
                            printf("\tDate Of Manufacture: Week %c%c, %c%c%c%c\n", pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]);
                            break;
                        case 0x0002://
                            printf("\tAccounting Date: Week %c%c, %c%c%c%c\n", pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]);
                            break;
                        case 0x0003:
                            printf("\tSpecified Cycle Count Over Device Lifetime: %" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]));
                            break;
                        case 0x0004:
                            printf("\tAccumulated Start-Stop Cycles: %" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]));
                            scsiDevInfo->logData.startStopData.accumulatedStartStopCycles = M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]);
                            scsiDevInfo->logData.gotStartStop = true;
                            break;
                        case 0x0005:
                            printf("\tSpecified Load-Unload Count Over Device Lifetime: %" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]));
                            break;
                        case 0x0006:
                            printf("\tAccumulated Load-Unload Cycles: %" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]));
                            scsiDevInfo->logData.startStopData.accumulatedLoadUnloadCycles = M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]);
                            scsiDevInfo->logData.gotStartStop = true;
                            break;
                        default:
                            if (parameterCode < 0x8000)
                            {
                                printf("Unknown Start-Stop parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            else
                            {
                                printf("Vendor Specific Start-Stop parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            break;
                        }
                    }
                    break;
                case 0x01:
                    printf("Utilization\n");
                    //TODO: Parse out this page
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x0F:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Application Client\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x10:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Self-Test Results\n");
                    //TODO: Parse out this page
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x11:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Solid State Media\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        switch (parameterCode)
                        {
                        case 0x0001:
                            printf("\tPercent Used Indicator: %" PRIu8 "\n", pageToRead[offset + 7]);
                            scsiDevInfo->logData.gotSolidStateData = true;
                            scsiDevInfo->logData.solidStateData.percentUsedIndicator = pageToRead[offset + 7];
                            break;
                        default:
                            if (parameterCode < 0x8000)
                            {
                                printf("Unknown Solid State Media parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            else
                            {
                                printf("Vendor Specific Solid State Media parameter code: %04" PRIX16 "h\n", parameterCode);
                            }
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            break;
                        }
                    }
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x14:
                switch (subPageCode)
                {
                case 0x01:
                    printf("Zoned Block Device Statistics\n");
                    //TODO: Parse out this page
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x15:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Background Scan Results\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        switch (parameterCode)
                        {
                        case 0x0000:
                            printf("\tBackground Scan Status\n");
                            scsiDevInfo->logData.gotBackgroundScan = true;
                            scsiDevInfo->logData.backgroundData.accumulatedPowerOnMinutes = M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]);
                            printf("\t\tAccumulated Power On Minutes: %" PRIu64 "\n", scsiDevInfo->logData.backgroundData.accumulatedPowerOnMinutes);
                            printf("\t\tBackground Scan Status: ");
                            switch (pageToRead[offset + 9])
                            {
                            case 0x00:
                                printf("No background scan operation is active\n");
                                break;
                            case 0x01:
                                printf("A background medium scan operation is active\n");
                                break;
                            case 0x02:
                                printf("A background pre-scan operation is active\n");
                                break;
                            case 0x03:
                                printf("A background scan operation was halted due to a fatal error\n");
                                break;
                            case 0x04:
                                printf("A background scan operation was halted due to a vendor specific pattern of errors\n");
                                break;
                            case 0x05:
                                printf("A background scan operation was halted due to the medium being formatted without the Primary List\n");
                                break;
                            case 0x06:
                                printf("A background scan operation was halted due to a vendor specific cause\n");
                                break;
                            case 0x07:
                                printf("A background scan operation was halted due to the temperature being out of the allowed range\n");
                                break;
                            case 0x08:
                                printf("Background medium scan operations are enabled and no background medium scan operation is active\n");
                                break;
                            case 0x09:
                                printf("A background scan operation was halted due to the S_L_Full bit being set to one in the Background Control mode page and the background scan results list being full\n");
                                break;
                            case 0x0A:
                                printf("A background pre-scan operation was halted due ot the Background Pre-scan Time Limit timer expiring\n");
                                break;
                            default:
                                printf("Reserved value: %02" PRIX8 "h\n", pageToRead[offset + 9]);
                                break;
                            }
                            printf("\t\tNumber Of Background Scans Performed: %" PRIu16 "\n", M_BytesTo2ByteValue(pageToRead[offset + 10], pageToRead[offset + 11]));
                            printf("\t\tBackground Scan Progress: %" PRIu16 "%%\n", M_BytesTo2ByteValue(pageToRead[offset + 10], pageToRead[offset + 11]));
                            printf("\t\tNumber Of Background Medium Scans Performed: %" PRIu16 "\n", M_BytesTo2ByteValue(pageToRead[offset + 10], pageToRead[offset + 11]));
                            break;
                        default:
                            if (parameterCode > 0 && parameterCode <= 0x0800)
                            {
                                //background scan result entry.
                                printf("\tBackground Scan Result %" PRIu16 "\n", parameterCode);
                                printf("\t\tAccumulated Power On Minutes: %" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]));
                                printf("\t\tSense Key: %" PRIX8 "h\n", M_Nibble0(pageToRead[offset + 8]));
                                printf("\t\tAdditional Sense Code: %02" PRIX8 "h\n", pageToRead[offset + 9]);
                                printf("\t\tAdditional Sense Code Qualifier: %02" PRIX8 "h\n", pageToRead[offset + 10]);
                                printf("\t\tReassign Status: ");
                                switch (M_Nibble1(pageToRead[offset + 8]))
                                {
                                case 0x1:
                                    printf("The LBA has not yet been reassigned\n");
                                    break;
                                case 0x2:
                                    printf("The device server performed automatic read reassignment for the LBA\n");
                                    break;
                                case 0x4:
                                    printf("The device server's attempt to perform automatic read reassignment failed. The LBA may or may not now have an uncorrectable error\n");
                                    break;
                                case 0x5:
                                    printf("The error was corrected by the device server rewriting the LBA without performing a reassign operation\n");
                                    break;
                                case 0x6:
                                    printf("An application client caused automatic write reassignment for the LBA with a command performing a write operation or an application client caused an unmap operation for the LBA\n");
                                    break;
                                case 0x7:
                                    printf("An application client caused a reassign operation for the LBA with a reassign block command or an application client caused an unmap operation for the LBA\n");
                                    break;
                                case 0x8:
                                    printf("An application client's request for a reassign operation for the LBA with a reassign blocks command failed. The logical block referenced by the LBA may or may not still have an uncorrectable error\n");
                                    break;
                                default:
                                    printf("Reserved - %" PRIX8 "h\n", M_Nibble1(pageToRead[offset + 8]));
                                    break;
                                }
                                printf("\t\tLogical Block Address: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 16], pageToRead[offset + 17], pageToRead[offset + 18], pageToRead[offset + 19], pageToRead[offset + 20], pageToRead[offset + 21], pageToRead[offset + 22], pageToRead[offset + 23]));
                            }
                            else if (parameterCode < 0x8000)
                            {
                                printf("Unknown Background Scan parameter code: %04" PRIX16 "h\n", parameterCode);
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            }
                            else
                            {
                                printf("Vendor Specific Background Scan parameter code: %04" PRIX16 "h\n", parameterCode);
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            }
                            break;
                        }
                    }
                    break;
                case 0x01:
                    printf("Pending Defects\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        switch (parameterCode)
                        {
                        case 0x0000:
                            scsiDevInfo->logData.gotPendingDefects = true;
                            scsiDevInfo->logData.pendingDefectsData.pendingDefectsCount = M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]);
                            printf("\tPending Defect Count: %" PRIu32 "\n", scsiDevInfo->logData.pendingDefectsData.pendingDefectsCount);
                            break;
                        default:
                            if (parameterCode > 0 && parameterCode <= 0xF000)
                            {
                                printf("\tPending Defect %" PRIu16 "\n", parameterCode);
                                uint32_t poh = M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]);
                                printf("\t\tAccumulated Power On Hours: ");
                                if (poh == UINT32_MAX)
                                {
                                    printf("Unknown\n");
                                }
                                else
                                {
                                    printf("%" PRIu32 "\n", poh);
                                }
                                printf("\t\tLogical Block Address: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11], pageToRead[offset + 12], pageToRead[offset + 13], pageToRead[offset + 14], pageToRead[offset + 15]));
                            }
                            else if (parameterCode < 0x8000)
                            {
                                printf("Unknown Pending Defects parameter code: %04" PRIX16 "h\n", parameterCode);
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            }
                            else
                            {
                                printf("Vendor Specific Pending Defects parameter code: %04" PRIX16 "h\n", parameterCode);
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            }
                            break;
                        }
                    }
                    break;
                case 0x02:
                    printf("Background Operation\n");
                    break;
                case 0x03:
                    printf("LPS Misalignment\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x16:
                switch (subPageCode)
                {
                case 0x00:
                    printf("ATA Pass-through Results\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        if (parameterCode > 0 && parameterCode <= 0x0E)
                        {
                            printf("\tATA Result %" PRIu16 "\n", parameterCode);
                            //validate offset + 4 = 0x09 and offset + 5 == 0x0C
                            printf("\t\tExtend: ");
                            if (pageToRead[offset + 6] & BIT0)
                            {
                                printf("True\n");
                            }
                            else
                            {
                                printf("False\n");
                            }
                            printf("\t\tError:        %02" PRIX8 "h\n", pageToRead[offset + 7]);
                            printf("\t\tCount (15:8): %02" PRIX8 "h\n", pageToRead[offset + 8]);
                            printf("\t\tCount  (7:0): %02" PRIX8 "h\n", pageToRead[offset + 9]);
                            printf("\t\tLBA  (31:24): %02" PRIX8 "h\n", pageToRead[offset + 10]);
                            printf("\t\tLBA    (7:0): %02" PRIX8 "h\n", pageToRead[offset + 11]);
                            printf("\t\tLBA  (39:32): %02" PRIX8 "h\n", pageToRead[offset + 12]);
                            printf("\t\tLBA   (15:8): %02" PRIX8 "h\n", pageToRead[offset + 13]);
                            printf("\t\tLBA  (47:40): %02" PRIX8 "h\n", pageToRead[offset + 14]);
                            printf("\t\tLBA  (23:16): %02" PRIX8 "h\n", pageToRead[offset + 15]);
                            printf("\t\tDevice:       %02" PRIX8 "h\n", pageToRead[offset + 16]);
                            printf("\t\tStatus:       %02" PRIX8 "h\n", pageToRead[offset + 17]);
                        }
                        else if (parameterCode < 0x8000)
                        {
                            printf("Unknown ATA Passthrough Results parameter code: %04" PRIX16 "h\n", parameterCode);
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                        }
                        else
                        {
                            printf("Vendor Specific ATA Passthrough Results parameter code: %04" PRIX16 "h\n", parameterCode);
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                        }
                    }
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x17:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Non-Volatile Cache\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x18:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Protocol Specific Port\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Protocol Specific Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x19:
                switch (subPageCode)
                {
                case 0x00:
                    printf("General Statistics and Performance\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        switch (parameterCode)
                        {
                        case 0x0001:
                            printf("\tGeneral Access Statitics and Performance\n");
                            scsiDevInfo->logData.gotGeneralStatistics = true;
                            scsiDevInfo->logData.generalStatisticsData.numberOfReadCommands = M_BytesTo8ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11]);
                            scsiDevInfo->logData.generalStatisticsData.numberOfWriteCommands = M_BytesTo8ByteValue(pageToRead[offset + 12], pageToRead[offset + 13], pageToRead[offset + 14], pageToRead[offset + 15], pageToRead[offset + 16], pageToRead[offset + 17], pageToRead[offset + 18], pageToRead[offset + 19]);
                            scsiDevInfo->logData.generalStatisticsData.numberOfLogicalBlocksReceived = M_BytesTo8ByteValue(pageToRead[offset + 20], pageToRead[offset + 21], pageToRead[offset + 22], pageToRead[offset + 23], pageToRead[offset + 24], pageToRead[offset + 25], pageToRead[offset + 26], pageToRead[offset + 27]);
                            scsiDevInfo->logData.generalStatisticsData.numberOfLogicalBlocksTransmitted = M_BytesTo8ByteValue(pageToRead[offset + 28], pageToRead[offset + 29], pageToRead[offset + 30], pageToRead[offset + 31], pageToRead[offset + 32], pageToRead[offset + 33], pageToRead[offset + 34], pageToRead[offset + 35]);
                            printf("\t\tNumber Of Read Commands: %" PRIu64 "\n", scsiDevInfo->logData.generalStatisticsData.numberOfReadCommands);
                            printf("\t\tNumber Of Write Commands: %" PRIu64 "\n", scsiDevInfo->logData.generalStatisticsData.numberOfWriteCommands);
                            printf("\t\tNumber Of Logical Blocks Received: %" PRIu64 "\n", scsiDevInfo->logData.generalStatisticsData.numberOfLogicalBlocksReceived);
                            printf("\t\tNumber Of Logical Blocks Transmitted: %" PRIu64 "\n", scsiDevInfo->logData.generalStatisticsData.numberOfLogicalBlocksTransmitted);
                            printf("\t\tRead Command Processing Intervals: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 36], pageToRead[offset + 37], pageToRead[offset + 38], pageToRead[offset + 39], pageToRead[offset + 40], pageToRead[offset + 41], pageToRead[offset + 42], pageToRead[offset + 43]));
                            printf("\t\tWrite Command Processing Intervals: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 44], pageToRead[offset + 45], pageToRead[offset + 46], pageToRead[offset + 47], pageToRead[offset + 48], pageToRead[offset + 49], pageToRead[offset + 50], pageToRead[offset + 51]));
                            printf("\t\tWeighted Number Of Read Commands Plus Write Commands: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 52], pageToRead[offset + 53], pageToRead[offset + 54], pageToRead[offset + 55], pageToRead[offset + 56], pageToRead[offset + 57], pageToRead[offset + 58], pageToRead[offset + 59]));
                            printf("\t\tWeighted Read Command Processing Plus Write Command Processing: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 60], pageToRead[offset + 61], pageToRead[offset + 62], pageToRead[offset + 63], pageToRead[offset + 64], pageToRead[offset + 65], pageToRead[offset + 66], pageToRead[offset + 67]));
                            break;
                        case 0x0002:
                            printf("\tIdle Time\n");
                            printf("\t\tIdle Time Intervals: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11]));
                            break;
                        case 0x0003:
                            printf("\tTime Interval\n");
                            printf("\t\tTimer Interval Descriptor: %" PRIu32 " x 10^-%" PRIu32 "\n", M_BytesTo4ByteValue(pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11]), M_BytesTo4ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7]));
                            break;
                        case 0x0004:
                            printf("\tGeneral Access Statitics and Performance\n");
                            printf("\t\tNumber Of Read FUA Commands: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 4], pageToRead[offset + 5], pageToRead[offset + 6], pageToRead[offset + 7], pageToRead[offset + 8], pageToRead[offset + 9], pageToRead[offset + 10], pageToRead[offset + 11]));
                            printf("\t\tNumber Of Write FUA Commands: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 12], pageToRead[offset + 13], pageToRead[offset + 14], pageToRead[offset + 15], pageToRead[offset + 16], pageToRead[offset + 17], pageToRead[offset + 18], pageToRead[offset + 19]));
                            printf("\t\tNumber Of Read FUA_NV Commands: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 20], pageToRead[offset + 21], pageToRead[offset + 22], pageToRead[offset + 23], pageToRead[offset + 24], pageToRead[offset + 25], pageToRead[offset + 26], pageToRead[offset + 27]));
                            printf("\t\tNumber Of Write FUA_NV Commands: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 28], pageToRead[offset + 29], pageToRead[offset + 30], pageToRead[offset + 31], pageToRead[offset + 32], pageToRead[offset + 33], pageToRead[offset + 34], pageToRead[offset + 35]));
                            printf("\t\tRead FUA Command Processing Intervals: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 36], pageToRead[offset + 37], pageToRead[offset + 38], pageToRead[offset + 39], pageToRead[offset + 40], pageToRead[offset + 41], pageToRead[offset + 42], pageToRead[offset + 43]));
                            printf("\t\tWrite FUA Command Processing Intervals: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 44], pageToRead[offset + 45], pageToRead[offset + 46], pageToRead[offset + 47], pageToRead[offset + 48], pageToRead[offset + 49], pageToRead[offset + 50], pageToRead[offset + 51]));
                            printf("\t\tRead FUA_NV Command Processing Intervals: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 52], pageToRead[offset + 53], pageToRead[offset + 54], pageToRead[offset + 55], pageToRead[offset + 56], pageToRead[offset + 57], pageToRead[offset + 58], pageToRead[offset + 59]));
                            printf("\t\tWrite FUA_NV Command Processing Intervals: %" PRIu64 "\n", M_BytesTo8ByteValue(pageToRead[offset + 60], pageToRead[offset + 61], pageToRead[offset + 62], pageToRead[offset + 63], pageToRead[offset + 64], pageToRead[offset + 65], pageToRead[offset + 66], pageToRead[offset + 67]));
                            break;
                        default:
                            if (parameterCode < 0x8000)
                            {
                                printf("Unknown General Statistics and Performance parameter code: %04" PRIX16 "h\n", parameterCode);
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            }
                            else
                            {
                                printf("Vendor Specific General Statistics and Performance parameter code: %04" PRIX16 "h\n", parameterCode);
                                print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            }
                            break;
                        }
                    }
                    break;
                case 0x20:
                    printf("Cache Memory Statistics\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    if (subPageCode >= 0x01 && subPageCode <= 0x15)
                    {
                        printf("Unknown Group Statistics and Performance Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    }
                    else
                    {
                        printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    }
                    break;
                }
                break;
            case 0x1A:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Power Condition Transitions\n");
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            case 0x2F:
                switch (subPageCode)
                {
                case 0x00:
                    printf("Informational Exceptions\n");
                    genericLogPagePrintout = false;
                    for (uint32_t offset = 4; offset < C_CAST(uint32_t, logPageLength + 4) && offset < UINT16_MAX; offset += parameterLength + 4)
                    {
                        uint16_t parameterCode = M_BytesTo2ByteValue(pageToRead[offset + 0], pageToRead[offset + 1]);
                        //TODO: parameter control byte fields
                        parameterLength = pageToRead[offset + 3];
                        switch (parameterCode)
                        {
                        case 0x0000:
                            printf("\tInformational Exceptions General\n");
                            scsiDevInfo->logData.gotInformationalExceptions = true;
                            scsiDevInfo->logData.informationalExceptionsData.asc = pageToRead[offset + 4];
                            scsiDevInfo->logData.informationalExceptionsData.ascq = pageToRead[offset + 5];
                            scsiDevInfo->logData.informationalExceptionsData.mostRecentTemperatureReading = pageToRead[offset + 6];
                            printf("\t\tInformational Exception Additional Sense Code: %02" PRIX8 "h\n", scsiDevInfo->logData.informationalExceptionsData.asc);
                            printf("\t\tInformational Exception Additional Sense Code Qualifier: %02" PRIX8 "h\n", scsiDevInfo->logData.informationalExceptionsData.ascq);
                            if (scsiDevInfo->logData.informationalExceptionsData.mostRecentTemperatureReading == UINT8_MAX)
                            {
                                printf("\t\tMost Recent Temperature Reading: Invalid reading due to sensor failure or other condition\n");
                            }
                            else
                            {
                                printf("\t\tMost Recent Temperature Reading: %02" PRIX8 "h\n", scsiDevInfo->logData.informationalExceptionsData.mostRecentTemperatureReading);
                            }
                            break;
                        default:
                            printf("Vendor Specific Pending Defects parameter code: %04" PRIX16 "h\n", parameterCode);
                            print_Data_Buffer(&pageToRead[offset + 4], parameterLength, true);
                            break;
                        }
                    }
                    break;
                case 0xFF:
                    printf("Supported subpages of page code %02" PRIX8 "h\n", pageCode);
                    break;
                default:
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                    break;
                }
                break;
            default:
                if (pageCode >= 0x30 && pageCode <= 0x3E)
                {
                    //vendor specific
                    printf("Vendor Specific Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                }
                else if (pageCode == 0x3F)
                {
                    //reserved
                    printf("Reserved Page code 3Fh\n");
                }
                else
                {
                    //unknown or not known in this loop at this time.
                    printf("Unknown Page and Subpage code: %02" PRIX8 "h-%02" PRIX8 "h\n", pageCode, subPageCode);
                }
                break;
            }
            if (genericLogPagePrintout && readLogPage)
            {
                print_Data_Buffer(pageToRead, logPageLength, true);
            }
            safe_Free_aligned(pageToRead)
        }
    }
    else
    {
        senseDataFields senseData;
        memset(&senseData, 0, sizeof(senseDataFields));
        get_Sense_Data_Fields(device->drive_info.lastCommandSenseData, SPC3_SENSE_LEN, &senseData);
        if (senseData.scsiStatusCodes.senseKey == SENSE_KEY_ILLEGAL_REQUEST && senseData.scsiStatusCodes.asc == 0x20 && senseData.scsiStatusCodes.ascq == 0x00)
        {
            //Invalid operation code, so this device does not support log sense commands.
            set_Console_Colors(true, NOTE_COLOR);
            printf("NOTE: Skipping SCSI Log test since device reported invalid operation code.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
        else
        {
            //Print out the sense data and a error
            printf("Error in SCSI Log test-Device returned the following sense data:\n");
            print_Sense_Fields(&senseData);
            printf("\n");
        }
        device->drive_info.passThroughHacks.scsiHacks.noLogPages = true;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: NLP\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }
    return SUCCESS;
}

typedef struct _scsiRWSupport
{
    bool sixBytes;
    bool tenBytes;
    bool twelveBytes;
    bool sixteenBytes;
    bool nonZeroSectorCountRequired;
}scsiRWSupport, *ptrScsiRWSupport;

static eReturnValues scsi_Read_Check(tDevice *device, bool zeroLengthTransfers, ptrScsiRWSupport rwSupport, bool testZeroLengthTransfersToo)
{
    if (!device || !rwSupport)
    {
        return BAD_PARAMETER;
    }

    set_Console_Colors(true, HEADING_COLOR);
    if (zeroLengthTransfers)
    {
        printf("\n======================================================\n");
        printf("Checking SCSI Read/Write Command Support (Zero Length)\n");
        printf("======================================================\n");
    }
    else
    {
        printf("\n========================================\n");
        printf("Checking SCSI Read/Write Command Support\n");
        printf("========================================\n");
    }
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);

    uint8_t *ptrData = NULL;
    uint32_t transferLength = 0;
    uint32_t transferLengthBytes = 0;
    if (!zeroLengthTransfers)
    {
        transferLength = 1;
        ptrData = C_CAST(uint8_t *, calloc_aligned(device->drive_info.deviceBlockSize, sizeof(uint8_t), device->os_info.minimumAlignment));
        transferLengthBytes = device->drive_info.deviceBlockSize;
        if (!ptrData)
        {
            return MEMORY_FAILURE;
        }
        //read 6 - requires data transfer, so always do this one with a data transfer
        if (SUCCESS == scsi_Read_6(device, 0, C_CAST(uint8_t, transferLength), ptrData, transferLengthBytes))
        {
            rwSupport->sixBytes = true;
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: RW6\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            device->drive_info.passThroughHacks.scsiHacks.readWrite.available = true;
            device->drive_info.passThroughHacks.scsiHacks.readWrite.rw6 = true;
        }
    }
    else
    {
        set_Console_Colors(true, NOTE_COLOR);
        printf("NOTE: Skipping R6 since zero length has a different meaning than other commands\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }

    //read 10
    if (SUCCESS == scsi_Read_10(device, 0, false, false, false, 0, 0, C_CAST(uint16_t, transferLength), ptrData, transferLengthBytes))
    {
        rwSupport->tenBytes = true;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: RW10\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.scsiHacks.readWrite.available = true;
        device->drive_info.passThroughHacks.scsiHacks.readWrite.rw10 = true;
    }

    //read 12
    if (SUCCESS == scsi_Read_12(device, 0, false, false, false, 0, 0, transferLength, ptrData, transferLengthBytes))
    {
        rwSupport->twelveBytes = true;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: RW12\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.scsiHacks.readWrite.available = true;
        device->drive_info.passThroughHacks.scsiHacks.readWrite.rw12 = true;
    }

    //read 16
    if (SUCCESS == scsi_Read_16(device, 0, false, false, false, 0, 0, transferLength, ptrData, transferLengthBytes))
    {
        rwSupport->sixteenBytes = true;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: RW16\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.scsiHacks.readWrite.available = true;
        device->drive_info.passThroughHacks.scsiHacks.readWrite.rw16 = true;
    }


    safe_Free_aligned(ptrData)

    if (testZeroLengthTransfersToo)
    {
        return scsi_Read_Check(device, true, rwSupport, false);
    }
    else if (!zeroLengthTransfers)
    {
        set_Console_Colors(true, NOTE_COLOR);
        printf("NOTE: Skipping testing for zero length transfers. This test should be done for highest compatibilty testing!\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }


    if (zeroLengthTransfers && !rwSupport->tenBytes && !rwSupport->twelveBytes && !rwSupport->sixteenBytes)
    {
        set_Console_Colors(true, ERROR_COLOR);
        printf("ERROR: Failed to find any support read commands by the device!\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        rwSupport->nonZeroSectorCountRequired = true;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: NZTL\n");//non-zero transfer length
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        scsi_Test_Unit_Ready(device, NULL);//try to clear errors before leaving this test
        return FAILURE;
    }
    return SUCCESS;
}

typedef struct _otherSCSICmdSupport
{
    bool reportAllSupportedOperationCodes;
    bool reportSingleSupportedOperationCode;
    bool securityProtocol;
    bool secProtI512;
    bool reportLuns;
    bool sendDiagnostic;
    bool testUnitReady;
}otherSCSICmdSupport, *ptrOtherSCSICmdSupport;

static bool does_Sense_Data_Show_Invalid_OP(tDevice *device)
{
    bool invalidOperationCode = false;
    senseDataFields senseFields;
    memset(&senseFields, 0, sizeof(senseDataFields));
    get_Sense_Data_Fields(device->drive_info.lastCommandSenseData, SPC3_SENSE_LEN, &senseFields);
    if (senseFields.scsiStatusCodes.senseKey == SENSE_KEY_ILLEGAL_REQUEST && senseFields.scsiStatusCodes.asc == 0x20 && senseFields.scsiStatusCodes.ascq == 0x00)
    {
        invalidOperationCode = true;
    }
    return invalidOperationCode;
}

static bool does_Sense_Data_Show_Invalid_Field_In_CDB(tDevice *device)
{
    bool invalidField = false;
    senseDataFields senseFields;
    memset(&senseFields, 0, sizeof(senseDataFields));
    get_Sense_Data_Fields(device->drive_info.lastCommandSenseData, SPC3_SENSE_LEN, &senseFields);
    if (senseFields.scsiStatusCodes.senseKey == SENSE_KEY_ILLEGAL_REQUEST && senseFields.scsiStatusCodes.asc == 0x24 && senseFields.scsiStatusCodes.ascq == 0x00)
    {
        invalidField = true;
    }
    return invalidField;
}

// static bool does_Sense_Data_Show_Invalid_Field_In_Parameter_List(tDevice *device)
// {
//     bool invalidField = false;
//     senseDataFields senseFields;
//     memset(&senseFields, 0, sizeof(senseDataFields));
//     get_Sense_Data_Fields(device->drive_info.lastCommandSenseData, SPC3_SENSE_LEN, &senseFields);
//     if (senseFields.scsiStatusCodes.senseKey == SENSE_KEY_ILLEGAL_REQUEST && senseFields.scsiStatusCodes.asc == 0x26 && senseFields.scsiStatusCodes.ascq == 0x00)
//     {
//         invalidField = true;
//     }
//     return invalidField;
// }

static eReturnValues other_SCSI_Cmd_Support(tDevice *device, ptrOtherSCSICmdSupport scsiCmds)
{
    if (!device || !scsiCmds)
    {
        return BAD_PARAMETER;
    }

    set_Console_Colors(true, HEADING_COLOR);
    printf("\n===========================\n");
    printf("Testing Other SCSI Commands\n");
    printf("===========================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);

    uint8_t scsiDataBytes[512] = { 0 };//used by each command

    scsiStatus blah;
    memset(&blah, 0, sizeof(scsiStatus));
    if (SUCCESS == scsi_Test_Unit_Ready(device, &blah))
    {
        scsiCmds->testUnitReady = true;
    }
    else
    {
        set_Console_Colors(true, ERROR_COLOR);
        printf("ERROR: Test unit ready command total failure. This is a critical command!\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }

    if (SUCCESS == scsi_Report_Luns(device, 0, 256, scsiDataBytes))
    {
        scsiCmds->reportLuns = true;
    }
    else
    {
        set_Console_Colors(true, ERROR_COLOR);
        printf("ERROR: Report LUNS failed. This is a critical command!\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }

    if (SUCCESS == scsi_SecurityProtocol_In(device, 0, 0, false, 512, scsiDataBytes))
    {
        scsiCmds->securityProtocol = true;
        device->drive_info.passThroughHacks.scsiHacks.securityProtocolSupported = true;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: SECPROT\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }
    else
    {
        //Check if we had a invalid operation code, or if we need the inc512 bit set as some things may be requiring that bit to be set!
        if (does_Sense_Data_Show_Invalid_Field_In_CDB(device))
        {
            //invalid field in CDB...try again with inc512 bit set!
            if (SUCCESS == scsi_SecurityProtocol_In(device, 0, 0, true, 1, scsiDataBytes))
            {
                scsiCmds->securityProtocol = true;
                scsiCmds->secProtI512 = true;
                device->drive_info.passThroughHacks.scsiHacks.securityProtocolWithInc512 = true;
                device->drive_info.passThroughHacks.scsiHacks.securityProtocolSupported = true;
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: SECPROTI512\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            else
            {
                set_Console_Colors(true, WARNING_COLOR);
                printf("WARNING: Device reported in a way that suggests that security protocol commands work, but no security protocol commands were successful\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                set_Console_Colors(true, LIKELY_HACK_COLOR);
                printf("Likely HACK FOUND: SECPROTI512\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                device->drive_info.passThroughHacks.scsiHacks.securityProtocolWithInc512 = true;
                device->drive_info.passThroughHacks.scsiHacks.securityProtocolSupported = true;
            }
        }
        else if (does_Sense_Data_Show_Invalid_OP(device))
        {
            set_Console_Colors(true, WARNING_COLOR);
            printf("WARNING: Security protocol in failed. Access to device security subsystems may be inaccessible or limited!\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
    }

    if (SUCCESS == scsi_Report_Supported_Operation_Codes(device, false, 0, 0, 0, 512, scsiDataBytes))
    {
        scsiCmds->reportAllSupportedOperationCodes = true;
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: REPALLOP\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.scsiHacks.reportAllOpCodes = true;
    }
    else
    {
        if (does_Sense_Data_Show_Invalid_OP(device))
        {
            set_Console_Colors(true, WARNING_COLOR);
            printf("WARNING: Reporting supported operation codes failed! This command does not appear to be known by the device.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
        else if (does_Sense_Data_Show_Invalid_Field_In_CDB(device))
        {
            set_Console_Colors(true, WARNING_COLOR);
            printf("WARNING: Reporting all supported operation codes is not supported! Will attempt requesting a single operation code.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
    }
    if (!does_Sense_Data_Show_Invalid_OP(device))
    {
        //todo: if invalid operation code was returned above, don't try this test as well - TJE
        if (SUCCESS == scsi_Report_Supported_Operation_Codes(device, false, 1, INQUIRY_CMD, 0, 512, scsiDataBytes))
        {
            scsiCmds->reportSingleSupportedOperationCode = true;
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: SUPSOP\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            device->drive_info.passThroughHacks.scsiHacks.reportSingleOpCodes = true;
        }
        else
        {
            set_Console_Colors(true, WARNING_COLOR);
            printf("WARNING: Reporting support for single requested operation codes failed! Unable to request command support from the device!\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
    }

    if (!scsiCmds->reportAllSupportedOperationCodes && !scsiCmds->reportSingleSupportedOperationCode)
    {
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: NRSUPOP\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.scsiHacks.noReportSupportedOperations = true;
    }

    printf("Testing SCSI default self-test.\n");
    //send diagnostic for default device self test. Wait up to 5 minutes for this command since some devices could take longer to process this
    //TODO: it is possible for self-test to fail, which we should catch since it performed the test and didn't return invalid operation code or invalid field in CDB
    /*eReturnValues selfTestResult =*/ scsi_Send_Diagnostic(device, 0, 0, 1, 0, 0, 0, NULL, 0, 5 * 60);
    if (!does_Sense_Data_Show_Invalid_OP(device))
    {
        scsiCmds->sendDiagnostic = true;
        printf("Device is capable of performing default self-test\n");
    }
    else
    {
        set_Console_Colors(true, WARNING_COLOR);
        printf("WARNING: Default self-test is not available.\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    }

    //TODO: add testing for diagnostic pages (try to get a list of pages)

    return SUCCESS;
}

#include <math.h>
static eReturnValues scsi_Error_Handling_Test(tDevice *device, double *badCommandRelativeTimeToGood)
{
    if (!device)
    {
        return BAD_PARAMETER;
    }
    scsi_Test_Unit_Ready(device, NULL);
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n==============================================\n");
    printf("Testing Error Handling Of Unsupported Commands\n");
    printf("==============================================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);

    //need to figure out commands to try, and time each and everyone.
    //Start with determining a baseline from a 3-4 commands (TUR after each)
    //Now that a base is established, start sending 10 or more unsupported commands and check how long the last one took. If it's near the baseline, it's fine
    //If not, and the time is significantly longer, then we know that sending test unit ready commands after each failed command is necessary for it to perform well.
    #define MAX_COMMANDS_TO_TRY 16
    uint64_t commandTimes[MAX_COMMANDS_TO_TRY + 1] = { 0 };
    uint8_t commandIter = 0;

    uint8_t dataBuffer[255] = { 0 };
    scsi_Inquiry(device, dataBuffer, 96, 0, false, false);//starting with a good command.
    commandTimes[commandIter] = device->drive_info.lastCommandTimeNanoSeconds;
    ++commandIter;
    //Now try one that is likely to not be supported and return invalid field in CDB (vendor specific page of some kind or another...try something in middle of range to hopefully avoid something that may actually be implented.)
    eReturnValues ret = SUCCESS;
    uint8_t pageCode = 0x2B;
    uint8_t subpage = 0x01;
    //page 2B, 01 should be an invalid page-subpage combination which should generate invalid field in CDB...
    while (ret == SUCCESS && pageCode < 0x3F)
    {
        ret = scsi_Mode_Sense_10(device, pageCode, 255, subpage, true, false, MPC_CURRENT_VALUES, dataBuffer);
        if (ret == SUCCESS)
        {
            ++pageCode;
        }
    }
    commandTimes[commandIter] = device->drive_info.lastCommandTimeNanoSeconds;
    ++commandIter;
    scsi_Test_Unit_Ready(device, NULL);
    ret = scsi_Mode_Sense_10(device, pageCode, 255, subpage, true, false, MPC_CURRENT_VALUES, dataBuffer);
    commandTimes[commandIter] = device->drive_info.lastCommandTimeNanoSeconds;
    ++commandIter;
    scsi_Test_Unit_Ready(device, NULL);
    //now average the 3 times we got.
    uint64_t averageCommandTimeNS = (commandTimes[1] + commandTimes[2] + commandTimes[0]) / 3;

    printf("Average response time from 3 commands: %" PRIu64 " nanoseconds\n", averageCommandTimeNS);

    device->drive_info.defaultTimeoutSeconds = 15;

    //now loop through 10 bad commands and check the timing and see if it got super slow
    for (uint8_t counter = 0; counter < MAX_COMMANDS_TO_TRY && commandIter < MAX_COMMANDS_TO_TRY && ret != OS_COMMAND_TIMEOUT; ++counter)
    {
        ret = scsi_Mode_Sense_10(device, pageCode, 255, subpage, true, false, MPC_CURRENT_VALUES, dataBuffer);
        commandTimes[commandIter] = device->drive_info.lastCommandTimeNanoSeconds;
        if (commandTimes[commandIter] > (2 * averageCommandTimeNS))
        {
            ++commandIter;
            break;
        }
        ++commandIter;
    }

    device->drive_info.defaultTimeoutSeconds = 0;

    uint64_t averageFromBadCommands = 0;
    uint8_t totalCmds = 0;
    for (uint8_t badIter = 4; badIter < MAX_COMMANDS_TO_TRY; ++badIter, ++totalCmds)
    {
        averageFromBadCommands += commandTimes[badIter];
    }

    double xTimesHigher = C_CAST(double, averageFromBadCommands) / C_CAST(double, averageCommandTimeNS);
    if (badCommandRelativeTimeToGood)
    {
        *badCommandRelativeTimeToGood = xTimesHigher;
    }

    printf("Testing complete.\n");
    printf("\tTotal Commands Tried: %" PRIu8 "\n", totalCmds);
    printf("\tAverage return time from bad commands: %" PRIu64 " nanoseconds\n", averageFromBadCommands);
    printf("\tCommand time ratio (bad compared to good): %0.02f\n", xTimesHigher);

    if (ret == OS_COMMAND_TIMEOUT || xTimesHigher > 3.0)//2 is pretty bad, but not uncommon, so moved up to 3.0 to be a little conservative. Don't want or need to enable this on everything. - TJE
    {
        //check just how much higher it is...if more than 2x higher, then there is an issue
        //Checking the last one because it is the most important. For this test, it should get much MUCH longer quickly and stay that way.
        device->drive_info.passThroughHacks.turfValue = C_CAST(uint8_t, round(xTimesHigher));
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: TURF%" PRIu8 "\n", device->drive_info.passThroughHacks.turfValue);
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.testUnitReadyAfterAnyCommandFailure = true;
        scsi_Test_Unit_Ready(device, NULL);
    }
    else if (ret == OS_PASSTHROUGH_FAILURE)
    {
        set_Console_Colors(true, LIKELY_HACK_COLOR);
        printf("Likely HACK FOUND: TURF%" PRIu8 "\n", 33);//setting 33...this should be sooo much higher and worse that this should remain true for a long long time.
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.testUnitReadyAfterAnyCommandFailure = true;
        device->drive_info.passThroughHacks.turfValue = 33;
        scsi_Test_Unit_Ready(device, NULL);
    }
    else if (averageFromBadCommands == 0)
    {
        set_Console_Colors(true, LIKELY_HACK_COLOR);
        printf("Likely HACK FOUND: TURF%" PRIu8 "\n", 34);//setting 34...this helps differentiate from the issue above
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        device->drive_info.passThroughHacks.testUnitReadyAfterAnyCommandFailure = true;
        device->drive_info.passThroughHacks.turfValue = 34;
        scsi_Test_Unit_Ready(device, NULL);
    }

    return SUCCESS;
}

static eReturnValues sct_GPL_Test(tDevice *device, bool smartSupported, bool gplSupported, bool sctSupported)
{
    if (!device)
    {
        return BAD_PARAMETER;
    }
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n============\n");
    printf("SCT-GPL Test\n");
    printf("============\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    if (sctSupported && smartSupported && gplSupported)
    {
        bool smartWorked = false;
        uint8_t sctStatus[512] = { 0 };
        printf("This test tries reading the SCT status log with SMART and GPL commands.\n");
        printf("This is done to test if one of these causes a SATL to hang as has been seen in the past.\n");
        printf("If this test hangs the device, it will need to be unplugged and the tool rerun without the sctgpl test.\n");
        printf("If the device hangs, the additional hack \"SCTSM\" must be used with this device for full functionality.\n");

        //SMART first
        printf("\tTesting with SMART read log\n");
        if (SUCCESS == ata_SMART_Read_Log(device, ATA_SCT_COMMAND_STATUS, sctStatus, 512))
        {
            printf("\t    Successfully read SCT status with SMART Read Log!\n");
            smartWorked = true;
        }
        else
        {
            set_Console_Colors(true, ERROR_COLOR);
            printf("ERROR: Something went wrong trying to read the SCT status log!!!\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
        //now GPL
        printf("\tTesting with read log ext\n");
        if (SUCCESS == ata_Read_Log_Ext(device, ATA_SCT_COMMAND_STATUS, 0, sctStatus, 512, (device->drive_info.ata_Options.dmaMode != ATA_DMA_MODE_NO_DMA && device->drive_info.ata_Options.readLogWriteLogDMASupported) ? true : false, 0))
        {
            printf("\t    Successfully read SCT status with Read Log Ext!\n");
        }
        else
        {
            if (smartWorked)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: SCTSM\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                printf("\t\tA retest is recommended with this test turned off. Devices with this hack\n");
                printf("\t\toften do not recover properly and need a full power cycle once this issue is\n");
                printf("\t\ttested and identified.\n");
                device->drive_info.passThroughHacks.ataPTHacks.smartCommandTransportWithSMARTLogCommandsOnly = true;
            }
            set_Console_Colors(true, ERROR_COLOR);
            printf("ERROR: Something went wrong trying to read the SCT status log!!!\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }
        return SUCCESS;
    }
    else
    {
        printf("Skipping SCTGPL test because device does not support features required for this test.\n");
        printf("\t");
        if (!sctSupported)
        {
            printf("(SCT Not Supported) ");
        }
        if (!smartSupported)
        {
            printf("(SMART Not Supported) ");
        }
        if (!gplSupported)
        {
            printf("(GPL Not Supported) ");
        }
        printf("\n");
    }
    return NOT_SUPPORTED;
}

static void setup_ATA_ID_Info(ptrPassthroughTestParams inputs, bool *smartSupported, bool *smartLoggingSupported, bool *sctSupported)
{
    uint8_t *identifyData = C_CAST(uint8_t*, &inputs->device->drive_info.IdentifyData.ata.Word000);
    uint16_t *ident_word = C_CAST(uint16_t*, &inputs->device->drive_info.IdentifyData.ata.Word000);
    fill_ATA_Strings_From_Identify_Data(identifyData, inputs->device->drive_info.bridge_info.childDriveMN, inputs->device->drive_info.bridge_info.childDriveSN, inputs->device->drive_info.bridge_info.childDriveFW);

    //get the WWN
    inputs->device->drive_info.bridge_info.childWWN = M_WordsTo8ByteValue(inputs->device->drive_info.IdentifyData.ata.Word108, \
        inputs->device->drive_info.IdentifyData.ata.Word109, \
        inputs->device->drive_info.IdentifyData.ata.Word110, \
        inputs->device->drive_info.IdentifyData.ata.Word111);

    //get the sector sizes from the identify data
    if (((ident_word[106] & BIT14) == BIT14) && ((ident_word[106] & BIT15) == 0)) //making sure this word has valid data
    {
        //word 117 is only valid when word 106 bit 12 is set
        if ((ident_word[106] & BIT12) == BIT12)
        {
            inputs->device->drive_info.bridge_info.childDeviceBlockSize = M_BytesTo2ByteValue(ident_word[118], ident_word[117]);
            inputs->device->drive_info.bridge_info.childDeviceBlockSize *= 2; //convert to words to bytes
        }
        else //means that logical sector size is 512bytes
        {
            inputs->device->drive_info.bridge_info.childDeviceBlockSize = LEGACY_DRIVE_SEC_SIZE;
        }
        if ((ident_word[106] & BIT13) == 0)
        {
            inputs->device->drive_info.bridge_info.childDevicePhyBlockSize = inputs->device->drive_info.bridge_info.childDeviceBlockSize;
        }
        else //multiple logical sectors per physical sector
        {
            uint8_t sectorSizeExponent = 0;
            //get the number of logical blocks per physical blocks
            sectorSizeExponent = ident_word[106] & 0x000F;
            inputs->device->drive_info.bridge_info.childDevicePhyBlockSize = C_CAST(uint32_t, inputs->device->drive_info.bridge_info.childDeviceBlockSize * power_Of_Two(sectorSizeExponent));
        }
    }
    else
    {
        inputs->device->drive_info.bridge_info.childDeviceBlockSize = LEGACY_DRIVE_SEC_SIZE;
        inputs->device->drive_info.bridge_info.childDevicePhyBlockSize = LEGACY_DRIVE_SEC_SIZE;
    }
    //get the sector alignment
    if (ident_word[209] & BIT14)
    {
        //bits 13:0 are valid for alignment. bit 15 will be 0 and bit 14 will be 1. remove bit 14 with an xor
        inputs->device->drive_info.bridge_info.childSectorAlignment = ident_word[209] ^ BIT14;
    }
    if (inputs->device->drive_info.IdentifyData.ata.Word083 & BIT10)
    {
        //acs4 - word 69 bit3 means extended number of user addressable sectors word supported (words 230 - 233) (Use this to get the max LBA since words 100 - 103 may only contain a value of FFFF_FFFF)
        if (inputs->device->drive_info.IdentifyData.ata.Word069 & BIT3)
        {
            inputs->device->drive_info.bridge_info.childDeviceMaxLba = M_BytesTo8ByteValue(identifyData[467], identifyData[466], identifyData[465], identifyData[464], identifyData[463], identifyData[462], identifyData[461], identifyData[460]);
        }
        else
        {
            inputs->device->drive_info.bridge_info.childDeviceMaxLba = M_BytesTo8ByteValue(identifyData[207], identifyData[206], identifyData[205], identifyData[204], identifyData[203], identifyData[202], identifyData[201], identifyData[200]);
        }
    }
    else
    {
        inputs->device->drive_info.bridge_info.childDeviceMaxLba = M_BytesTo4ByteValue(identifyData[123], identifyData[122], identifyData[121], identifyData[120]);
    }
    if (inputs->device->drive_info.bridge_info.childDeviceMaxLba > 0)
    {
        inputs->device->drive_info.bridge_info.childDeviceMaxLba -= 1;
    }

    //This flag will get set so we can do a software translation of LBA to CHS during read/write
    if (!is_LBA_Mode_Supported(inputs->device) && is_CHS_Mode_Supported(inputs->device))
    {
        inputs->device->drive_info.ata_Options.chsModeOnly = true;
        //simulate a max LBA into device information
        uint16_t cylinder = M_BytesTo2ByteValue(identifyData[109], identifyData[108]);//word 54
        uint8_t head = identifyData[110];//Word55
        uint8_t sector = identifyData[112];//Word56
        //if (cylinder == 0 && head == 0 && sector == 0)
        //{
        //    //current inforation isn't there....so use default values
        //    cylinder = M_BytesTo2ByteValue(identifyData[3], identifyData[2]);//word 1
        //    head = identifyData[6];//Word3
        //    sector = identifyData[12];//Word6
        //}
        uint32_t lba = C_CAST(uint32_t, cylinder) * C_CAST(uint32_t, head) * C_CAST(uint32_t, sector);
        if (lba == 0)
        {
            //Cannot use "current" settings on this drive...use default (really old drive)
            cylinder = M_BytesTo2ByteValue(identifyData[3], identifyData[2]);//word 1
            head = identifyData[6];//Word3
            sector = identifyData[12];//Word6
            lba = C_CAST(uint32_t, cylinder) * C_CAST(uint32_t, head) * C_CAST(uint32_t, sector);
        }
        inputs->device->drive_info.bridge_info.childDeviceMaxLba = lba;
    }

    //set read/write buffer DMA
    if (ident_word[69] & BIT11)
    {
        inputs->device->drive_info.ata_Options.readBufferDMASupported = true;
    }
    if (ident_word[69] & BIT10)
    {
        inputs->device->drive_info.ata_Options.writeBufferDMASupported = true;
    }
    //set download microcode DMA support
    if (ident_word[69] & BIT8)
    {
        inputs->device->drive_info.ata_Options.downloadMicrocodeDMASupported = true;
    }
    //set zoned device type
    if (inputs->device->drive_info.zonedType != ZONED_TYPE_HOST_MANAGED)
    {
        switch (ident_word[69] & (BIT0 | BIT1))
        {
        case 0:
            inputs->device->drive_info.zonedType = ZONED_TYPE_NOT_ZONED;
            break;
        case 1:
            inputs->device->drive_info.zonedType = ZONED_TYPE_HOST_AWARE;
            break;
        case 2:
            inputs->device->drive_info.zonedType = ZONED_TYPE_DEVICE_MANAGED;
            break;
        case 3:
            inputs->device->drive_info.zonedType = ZONED_TYPE_RESERVED;
            break;
        default:
            break;
        }
    }
    //Determine if read/write log ext DMA commands are supported
    if (ident_word[119] & BIT3 || ident_word[120] & BIT3)
    {
        inputs->device->drive_info.ata_Options.readLogWriteLogDMASupported = true;
    }
    if (ident_word[47] != UINT16_MAX && ident_word[47] != 0)
    {
        if (M_Byte0(ident_word[47]) != 0)
        {
            inputs->device->drive_info.ata_Options.readWriteMultipleSupported = true;
            //set the number of logical sectors per DRQ data block (current setting)
            inputs->device->drive_info.ata_Options.logicalSectorsPerDRQDataBlock = M_Byte0(ident_word[59]);
        }
    }
    //check for tagged command queuing support
    if (ident_word[83] & BIT1 || ident_word[86] & BIT1)
    {
        inputs->device->drive_info.ata_Options.taggedCommandQueuingSupported = true;
    }
    //check for native command queuing support
    if (ident_word[76] & BIT8)
    {
        inputs->device->drive_info.ata_Options.nativeCommandQueuingSupported = true;
    }
    //check if the device is parallel or serial
    uint8_t transportType = C_CAST(uint8_t, (ident_word[222] & (BIT15 | BIT14 | BIT13 | BIT12)) >> 12);
    switch (transportType)
    {
    case 0x00://parallel
        inputs->device->drive_info.ata_Options.isParallelTransport = true;
        break;
    case 0x01://serial
    case 0x0E://PCIe
    default:
        break;
    }
    if (inputs->device->drive_info.IdentifyData.ata.Word076 > 0)//Only Serial ATA Devices will set the bits in words 76-79
    {
        inputs->device->drive_info.ata_Options.isParallelTransport = false;
    }
    if (ident_word[119] & BIT2 || ident_word[120] & BIT2)
    {
        inputs->device->drive_info.ata_Options.writeUncorrectableExtSupported = true;
    }
    if (ident_word[120] & BIT6)//word120 holds if this is enabled
    {
        inputs->device->drive_info.ata_Options.senseDataReportingEnabled = true;
    }

    if (inputs->device->drive_info.IdentifyData.ata.Word083 & BIT10)
    {
        inputs->device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported = true;
    }
    if ((inputs->device->drive_info.IdentifyData.ata.Word087 & BIT14 && inputs->device->drive_info.IdentifyData.ata.Word087 != 0xFFFF && inputs->device->drive_info.IdentifyData.ata.Word087 & BIT5)
        ||
        (inputs->device->drive_info.IdentifyData.ata.Word084 & BIT14 && inputs->device->drive_info.IdentifyData.ata.Word084 != 0xFFFF && inputs->device->drive_info.IdentifyData.ata.Word084 & BIT5))
    {
        inputs->device->drive_info.ata_Options.generalPurposeLoggingSupported = true;
    }

    //Test UDMA vs DMA mode on commands
    //This test will do a read DMA type command to the drive IF DMA is supported. If not, this will be skipped since this is the most basic DMA command we can try to do.
    //Now determine if the drive supports DMA and which DMA modes it supports
    if (inputs->device->drive_info.IdentifyData.ata.Word049 & BIT8)
    {
        inputs->device->drive_info.ata_Options.dmaSupported = true;
        inputs->device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_DMA;
    }
    //obsolete since ATA3, holds single word DMA support
    if (inputs->device->drive_info.IdentifyData.ata.Word062)
    {
        inputs->device->drive_info.ata_Options.dmaSupported = true;
        inputs->device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_DMA;
    }
    //check for multiword dma support
    if (inputs->device->drive_info.IdentifyData.ata.Word063 & (BIT0 | BIT1 | BIT2))
    {
        inputs->device->drive_info.ata_Options.dmaSupported = true;
        inputs->device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_MWDMA;
    }
    //check for UDMA support
    if (inputs->device->drive_info.IdentifyData.ata.Word088 & 0x007F)
    {
        inputs->device->drive_info.ata_Options.dmaSupported = true;
        inputs->device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_UDMA;
    }

    if ((inputs->device->drive_info.IdentifyData.ata.Word082 != 0 && inputs->device->drive_info.IdentifyData.ata.Word082 != 0xFFFF && inputs->device->drive_info.IdentifyData.ata.Word082 & BIT0)
        &&
        (inputs->device->drive_info.IdentifyData.ata.Word085 != 0 && inputs->device->drive_info.IdentifyData.ata.Word085 != 0xFFFF && inputs->device->drive_info.IdentifyData.ata.Word085 & BIT0)
        )
    {
        if (smartSupported)
        {
            *smartSupported = true;
        }
        if ((inputs->device->drive_info.IdentifyData.ata.Word084 & BIT14 && inputs->device->drive_info.IdentifyData.ata.Word084 != 0xFFFF && inputs->device->drive_info.IdentifyData.ata.Word084 & BIT0)
            ||
            (inputs->device->drive_info.IdentifyData.ata.Word087 & BIT14 && inputs->device->drive_info.IdentifyData.ata.Word087 != 0xFFFF && inputs->device->drive_info.IdentifyData.ata.Word087 & BIT0)
            )
        {
            if (smartLoggingSupported)
            {
                *smartLoggingSupported = true;
            }
        }
    }
    if (inputs->device->drive_info.IdentifyData.ata.Word206 != 0 && inputs->device->drive_info.IdentifyData.ata.Word206 != 0xFFFF && inputs->device->drive_info.IdentifyData.ata.Word206 & BIT0)
    {
        if (sctSupported)
        {
            *sctSupported = true;
        }
    }
    return;
}

static eReturnValues sat_Test_Identify(tDevice *device, uint8_t *ptrData, uint32_t dataSize, uint8_t cdbSize)
{
    eReturnValues ret = UNKNOWN;
    ataPassthroughCommand identify;
    memset(&identify, 0, sizeof(ataPassthroughCommand));
    identify.commandType = ATA_CMD_TYPE_TASKFILE;
    identify.commandDirection = XFER_DATA_IN;
    identify.commadProtocol = ATA_PROTOCOL_PIO;
    identify.tfr.SectorCount = 1;//some controllers have issues when this isn't set to 1
    identify.tfr.DeviceHead = DEVICE_REG_BACKWARDS_COMPATIBLE_BITS;
    identify.tfr.CommandStatus = ATA_IDENTIFY;
    identify.ataCommandLengthLocation = ATA_PT_LEN_SECTOR_COUNT;
    identify.ataTransferBlocks = ATA_PT_512B_BLOCKS;
    identify.ptrData = ptrData;
    identify.dataSize = dataSize;

    switch (cdbSize)
    {
    case 12:
        identify.forceCDBSize = 12;
        break;
    case 16:
        identify.forceCDBSize = 16;
        break;
    case 32:
        identify.forceCDBSize = 32;
        break;
    default:
        break;
    }

    if (device->drive_info.ata_Options.isDevice1)
    {
        identify.tfr.DeviceHead |= DEVICE_SELECT_BIT;
    }

    if (VERBOSITY_COMMAND_NAMES <= device->deviceVerbosity)
    {
        printf("Sending ATA Identify command\n");
    }
    ret = ata_Passthrough_Command(device, &identify);

    if (ret == SUCCESS && ptrData != (uint8_t*)&device->drive_info.IdentifyData.ata.Word000)
    {
        //copy the data to the device structure so that it's not (as) stale
        memcpy(&device->drive_info.IdentifyData.ata.Word000, ptrData, sizeof(tAtaIdentifyData));
    }

    if (ret == SUCCESS)
    {
        if (ptrData[510] == ATA_CHECKSUM_VALIDITY_INDICATOR)
        {
            //we got data, so validate the checksum
            uint32_t invalidSec = 0;
            if (!is_Checksum_Valid(ptrData, LEGACY_DRIVE_SEC_SIZE, &invalidSec))
            {
                ret = WARN_INVALID_CHECKSUM;
            }
        }
        else
        {
            //Don't do anything. This device doesn't use a checksum.
        }
    }

    if (VERBOSITY_COMMAND_NAMES <= device->deviceVerbosity)
    {
        print_Return_Enum("Identify", ret);
    }
    return ret;
}

static eReturnValues sat_Ext_Cmd_With_A1_When_Possible_Test(tDevice *device)
{
    eReturnValues ret = NOT_SUPPORTED;
    if (device->drive_info.ata_Options.generalPurposeLoggingSupported)
    {
        uint8_t log[512] = { 0 };
        printf("Testing if it is possible to issue limited Ext (48bit) commands with A1 CDB\n");
        device->drive_info.passThroughHacks.ataPTHacks.a1ExtCommandWhenPossible = true;
        if (SUCCESS == ata_Read_Log_Ext(device, ATA_LOG_DIRECTORY, 0, log, 512, false, 0))
        {
            //TODO: Validate the return data???
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: A1EXT\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            ret = SUCCESS;
        }
        else
        {
            printf("\tUnable to use limited 48bit commands with A1h\n");
            device->drive_info.passThroughHacks.ataPTHacks.a1ExtCommandWhenPossible = false;
        }
    }
    return ret;
}

static bool test_SAT_Capabilities(ptrPassthroughTestParams inputs, ptrScsiDevInformation scsiInformation)
{
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n=====================================\n");
    printf("Checking SAT ATA-passthrough commands\n");
    printf("=====================================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    bool satSupported = false;
    bool twelveByteSupported = false;
    bool sixteenByteSupported = false;
    bool doNotRetry16BSAT = false;
    //try A1h first to see if it works, then try 85h.
    inputs->device->drive_info.passThroughHacks.passthroughType = ATA_PASSTHROUGH_SAT;
    //inputs->device->drive_info.passThroughHacks.ataPTHacks.useA1SATPassthroughWheneverPossible = true; //forcing A1 first.
    uint8_t *identifyData = (uint8_t*)&inputs->device->drive_info.IdentifyData.ata.Word000;
    eReturnValues satRet = sat_Test_Identify(inputs->device, identifyData, 512, 12);
    if (SUCCESS == satRet || WARN_INVALID_CHECKSUM == satRet)
    {
        //TODO: Validate Identify data!
        printf("SAT 12-byte passthrough supported\n");
        satSupported = true;
        twelveByteSupported = true;
    }
    inputs->device->drive_info.passThroughHacks.ataPTHacks.a1NeverSupported = true;//forcing 16B command
    satRet = sat_Test_Identify(inputs->device, identifyData, 512, 16);//NOTE: On some devices this will fail because for some stupid reason, these devices will only issue a 48bit command with 16B commands and reject others...no idea why-TJE
    if (SUCCESS == satRet || WARN_INVALID_CHECKSUM == satRet)
    {
        //TODO: Validate Identify data!
        printf("SAT 16-byte passthrough supported\n");
        satSupported = true;
        sixteenByteSupported = true;
    }
    else
    {
        //check the sense data for invalid op code
        if (does_Sense_Data_Show_Invalid_OP(inputs->device))
        {
            doNotRetry16BSAT = true;
        }
    }
    inputs->device->drive_info.passThroughHacks.ataPTHacks.a1NeverSupported = false;//turn back off now
    if (satSupported)
    {
        if (twelveByteSupported)
        {
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: A1\n");//useA1SATPassthroughWheneverPossible
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            //inputs->device->drive_info.passThroughHacks.ataPTHacks.useA1SATPassthroughWheneverPossible = true;
        }
        else if (!twelveByteSupported && sixteenByteSupported)
        {
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: NA1\n");//a1NeverSupported
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            inputs->device->drive_info.passThroughHacks.ataPTHacks.a1NeverSupported = true;
        }

        //Testing 32B passthrough here!
        satRet = sat_Test_Identify(inputs->device, identifyData, 512, 32);
        if (satRet == SUCCESS)
        {
            printf("SAT 32-byte passthrough supported\n");
            satSupported = true;
        }
        else
        {
            satRet = sat_Test_Identify(inputs->device, identifyData, 512, 0);
            satSupported = true;
        }

        //Test TPSIU support
        bool tpsiuIdentifySuccess = false;
        inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysUseTPSIUForSATPassthrough = true;
        satRet = ata_Identify(inputs->device, identifyData, 512);
        if ((SUCCESS == satRet || WARN_INVALID_CHECKSUM == satRet) && !is_Empty(identifyData, 512))
        {
            tpsiuIdentifySuccess = true;
            inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysUseTPSIUForSATPassthrough = false;
        }
        else
        {
            inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysUseTPSIUForSATPassthrough = false;
            //send identify again to get the identify data populated again in the buffer
            satRet = ata_Identify(inputs->device, identifyData, 512);
        }

        bool smartSupported = false;
        bool smartLoggingSupported = false;
        bool sctSupported = false;
        setup_ATA_ID_Info(inputs, &smartSupported, &smartLoggingSupported, &sctSupported);

        //TODO: one more test to see if 48bit commands NOT setting ext register is possible/ Try something like readlogext to get the supported logs since that won't set ext registers.
        //TODO: Skip this for PATA adapters since this may cause a problem???
        if (!sixteenByteSupported)
        {
            sat_Ext_Cmd_With_A1_When_Possible_Test(inputs->device);
        }

        //Some devices only support 16B commands to send 48 bit commands (don't set the extend bit???)
        //Testing one more time for this here.
        if (!sixteenByteSupported && !doNotRetry16BSAT)
        {
            //Test with a read log ext or read ext or something like that - TJE
            if (inputs->device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported &&
                inputs->device->drive_info.ata_Options.generalPurposeLoggingSupported)
            {
                uint8_t data[512] = { 0 };
                eReturnValues retrySAT16 = ata_Read_Log_Ext(inputs->device, ATA_LOG_DIRECTORY, 0, data, 512, false, 0);
                if (retrySAT16 == SUCCESS)
                {
                    printf("SAT 16-byte passthrough supported, but only for 48bit commands!\n");
                    satSupported = true;
                    sixteenByteSupported = true;
                }
                else
                {
                    if (does_Sense_Data_Show_Invalid_OP(inputs->device))
                    {
                        doNotRetry16BSAT = true;
                    }
                }
            }
        }
        else if (!sixteenByteSupported && inputs->device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported)
        {
            set_Console_Colors(true, WARNING_COLOR);
            printf("WARNING: This device is not able to pass-through 48 bit (extended) commands!\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }

        if (twelveByteSupported && !sixteenByteSupported)
        {
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: ATA28\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            inputs->device->drive_info.passThroughHacks.ataPTHacks.ata28BitOnly = true;
        }

        if (tpsiuIdentifySuccess)
        {
            //if TPSIU worked for identify, we need to try another command, a read, to ensure that it actually works for other commands.
            //This was added after testing yet another USB bridge that did something odd and different that doesn't really work well
            uint8_t *data = C_CAST(uint8_t*, calloc_aligned(inputs->device->drive_info.deviceBlockSize * 1, sizeof(uint8_t), inputs->device->os_info.minimumAlignment));
            if (data)
            {
                bool use48 = inputs->device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported;
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.ata28BitOnly)
                {
                    use48 = false;
                }
                inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysUseTPSIUForSATPassthrough = true;
                //Before declaring TPSIU support to be always used, additionally test a read passthrough command to see if that works. - TJE
                //Using a PIO read command since the DMA test hasn't been performed yet. This is a single sector and should be OK to do at this point. - TJE
                eReturnValues tpsiuRet = ata_Read_Sectors(inputs->device, 0, data, 1, inputs->device->drive_info.deviceBlockSize * 1, use48);
                if (SUCCESS == tpsiuRet && !does_Sense_Data_Show_Invalid_Field_In_CDB(inputs->device))
                {
                    set_Console_Colors(true, HACK_COLOR);
                    printf("HACK FOUND: TPSIU\n");//alwaysUseTPSIUForSATPassthrough
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    set_Console_Colors(true, HACK_COLOR);
                    printf("HACK FOUND: TPID\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    inputs->device->drive_info.passThroughHacks.ataPTHacks.limitedUseTPSIU = true;
                    inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysUseTPSIUForSATPassthrough = false;
                }
                safe_Free_aligned(data)
            }
            else
            {
                set_Console_Colors(true, WARNING_COLOR);
                printf("WARNING: Unable to allocate memory and fully test TPSIU capability.\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysUseTPSIUForSATPassthrough = false;
            }
        }

        if (inputs->device->drive_info.ata_Options.dmaMode != ATA_DMA_MODE_NO_DMA)
        {
            sat_DMA_UDMA_Protocol_Test(inputs->device, smartSupported, smartLoggingSupported);
        }
        else
        {
            printf("Skipping DMA check as device does not report any support for DMA mode commands\n");
            printf("It is strongly recommended to retest with a device that does support DMA mode commands\n");
        }

        if (!inputs->noMultiSectorPIOTest)
        {
            multi_Sector_PIO_Test(inputs->device, smartSupported, smartLoggingSupported);
            if (!inputs->device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode && !inputs->device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly)
            {
                //printf("Multi-sector PIO commands seem to work properly\n");
                //don't print anything here because it could have had a problem within the test function
            }
            else if (inputs->device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: MMPIO\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            else if (inputs->device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: SPIO\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
        }
        else
        {
            set_Console_Colors(true, LIKELY_HACK_COLOR);
            printf("HACK FOUND: SPIO\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            inputs->device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly = true;
        }

        //In next tests, need to make sure we actually get response information that makes sense.
        // Will try SMART, get native max LBA first. Then try sending the drive to standby and checking the power mode, then spinning it up and checking again.
        // Need to make sure we look for ext RTFRs as well.

        if (!inputs->nocheckConditionTest)
        {
            check_Condition_Bit_Test(inputs->device, smartSupported, smartLoggingSupported);
        }

        if (!inputs->noReturnResponseInfoTest)
        {
            return_Response_Info_Test(inputs->device, smartSupported, smartLoggingSupported, (inputs->testPotentiallyDeviceHangingCommands && inputs->hangCommandsToTest.returnResponseInforWithoutTdir) ? true : false);
        }

        if (inputs->testPotentiallyDeviceHangingCommands && inputs->hangCommandsToTest.sctLogWithGPL)
        {
            sct_GPL_Test(inputs->device, smartSupported, inputs->device->drive_info.ata_Options.generalPurposeLoggingSupported, sctSupported);
        }


        set_Console_Colors(true, HEADING_COLOR);
        printf("\n====================================\n");
        printf("Comparing ATA and SCSI reported data\n");
        printf("====================================\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        //Here we will compare the reported information by the bridge from SCSI commands to what the ATA Identify data reports.
        //For MN, SN, FW, check for commonly broken reporting methods.
#define PASSTHROUGH_TEST_SCSI_PROD_ID_LEN 17
        char scsiProdID[PASSTHROUGH_TEST_SCSI_PROD_ID_LEN] = { 0 };
        snprintf(scsiProdID, PASSTHROUGH_TEST_SCSI_PROD_ID_LEN, "%s", scsiInformation->inquiryData.productId);
        remove_Leading_And_Trailing_Whitespace(scsiProdID);
        if (strncmp(scsiProdID, inputs->device->drive_info.bridge_info.childDriveMN, M_Min(16, strlen(scsiProdID))) == 0)
        {
            printf("\tSAT Compliant product ID reported\n");
        }
        else
        {
            set_Console_Colors(true, WARNING_COLOR);
            printf("\tWARNING: Non-SAT Compliant product ID reported. This may be a \"branded\" product or non-compliant translator.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            printf("\t  Checking common non-compliant reporting.\n");
            if (strstr(inputs->device->drive_info.bridge_info.childDriveMN, scsiInformation->inquiryData.productId) && strstr(inputs->device->drive_info.bridge_info.childDriveMN, scsiInformation->inquiryData.vendorId))
            {
                //Most likely had the vendor+productID set as the full ATA MN
#define PASSTHROUGH_TEST_FULL_MN_LENGTH 42
                char fullMN[PASSTHROUGH_TEST_FULL_MN_LENGTH] = { 0 };
                snprintf(fullMN, PASSTHROUGH_TEST_FULL_MN_LENGTH, "%s%s", scsiInformation->inquiryData.vendorId, scsiInformation->inquiryData.productId);
                if (strncmp(fullMN, inputs->device->drive_info.bridge_info.childDriveMN, M_Min(strlen(fullMN), strlen(inputs->device->drive_info.bridge_info.childDriveMN))) == 0)
                {
                    printf("\t\tTranslator put full ATA MN in combination of Vendor and Product ID fields.\n");
                }
            }
            else
            {
                printf("\t\tUnknown reporting format.\n");
            }
        }
        if (strlen(scsiInformation->vpdData.unitSN))
        {
            printf("\tChecking unit serial number\n");
            if (strstr(scsiInformation->vpdData.unitSN, inputs->device->drive_info.bridge_info.childDriveSN))//using strstr since we remove whitespace from childdriveSN, but not the unit serial number we saved.
            {
                printf("\tUnit serial number reported in SAT compliant translation!\n");
            }
            else
            {
                printf("\tUnit serial number not reported in a SAT compliant way.\n");
            }
        }
        else
        {
            printf("\tNo Unit serial number reported. Skipping this check\n");
        }

        if (scsiInformation->vpdData.gotBlockCharacteristicsVPDPage)
        {
            //Check rotation rate (if SCSI reported it)...if this doesn't match, check if it is byte swapped and throw a warning.
            if (inputs->device->drive_info.IdentifyData.ata.Word217 != 0 && inputs->device->drive_info.IdentifyData.ata.Word217 != scsiInformation->vpdData.blockCharacteristicsData.rotationRate)
            {
                set_Console_Colors(true, WARNING_COLOR);
                printf("WARNING: Rotation rate doesn't Match! Got %" PRIu16 ", expected %" PRIu16 "\n", scsiInformation->vpdData.blockCharacteristicsData.rotationRate, inputs->device->drive_info.IdentifyData.ata.Word217);
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                byte_Swap_16(&scsiInformation->vpdData.blockCharacteristicsData.rotationRate);
                if (inputs->device->drive_info.IdentifyData.ata.Word217 != scsiInformation->vpdData.blockCharacteristicsData.rotationRate)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: Even after byteswapping, the rotation rate still doesn't match!!!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
                else
                {
                    set_Console_Colors(true, WARNING_COLOR);
                    printf("It seems like the RPM was reported without byteswapping it first! Fix this in the translator firmware!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }

            //Check form factor (if SCSI reported it)..if this doesn't match, show a warning since this page was otherwise reported.
            if (M_Nibble0(inputs->device->drive_info.IdentifyData.ata.Word168) != 0 && M_Nibble0(inputs->device->drive_info.IdentifyData.ata.Word168) != scsiInformation->vpdData.blockCharacteristicsData.formFactor)
            {
                set_Console_Colors(true, ERROR_COLOR);
                printf("ERROR: Formfactor doesn't match! Got %" PRIu8 ", expected %" PRIu8 "\n", scsiInformation->vpdData.blockCharacteristicsData.formFactor, M_Nibble0(inputs->device->drive_info.IdentifyData.ata.Word168));
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }

        }

        //check the SCSI designators to see if we found the WWN translated through. 
        //Check WWN (if SCSI reported it and device supports it)
        if (inputs->device->drive_info.IdentifyData.ata.Word084 & BIT8 || inputs->device->drive_info.IdentifyData.ata.Word087 & BIT8)
        {
            if (scsiInformation->vpdData.gotDeviceIDVPDPage)
            {
                bool foundMatch = false;
                for (uint8_t iter = 0; iter < scsiInformation->vpdData.designatorCount; ++iter)
                {
                    if (scsiInformation->vpdData.designators[iter].valid && scsiInformation->vpdData.designators[iter].designatorType == 3)//need NAA type for WWN
                    {
                        if (scsiInformation->vpdData.designators[iter].designatorLength == 8)
                        {
                            uint64_t scsiWWN = M_BytesTo8ByteValue(scsiInformation->vpdData.designators[iter].designator[0], scsiInformation->vpdData.designators[iter].designator[1], scsiInformation->vpdData.designators[iter].designator[2], scsiInformation->vpdData.designators[iter].designator[3], scsiInformation->vpdData.designators[iter].designator[4], scsiInformation->vpdData.designators[iter].designator[5], scsiInformation->vpdData.designators[iter].designator[6], scsiInformation->vpdData.designators[iter].designator[7]);
                            uint64_t ataWWN = M_WordsTo8ByteValue(inputs->device->drive_info.IdentifyData.ata.Word108, inputs->device->drive_info.IdentifyData.ata.Word109, inputs->device->drive_info.IdentifyData.ata.Word110, inputs->device->drive_info.IdentifyData.ata.Word111);//words 108 - 111
                            //TODO: Need to make sure we handled inputting everything correctly! ATA may have some byteswaps to do...
                            if (scsiWWN == ataWWN)
                            {
                                foundMatch = true;
                                break;
                            }
                        }
                    }
                }
                if (!foundMatch)
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: WWN was not found in device identification VPD page!\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }
            else
            {
                set_Console_Colors(true, WARNING_COLOR);
                printf("WARNING: Device didn't report device ID VPD page, cannot check for WWN translation!\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
        }

        //check block sizes and maxLBA values
        //Throw a warning that the reverse 4k emulation crap is not useful since we are way past Windows XP at this point.

        //TODO: Based on other SCSI reported information, read the ATA logs that the information should come from to compare and make sure it matches, or is close (>= what SCSI reported)
        //      This should be device statistics, other identify data not checked above, command/feature support, EPC settings, etc


    }
    return satSupported;
}

static bool test_Legacy_ATA_Passthrough(ptrPassthroughTestParams inputs, ptrScsiDevInformation scsiInformation)
{
    bool legacyATAPassthroughSupported = false;
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n====================================================================\n");
    printf("Testing For Legacy ATA Passthrough Support With Known Legacy Methods\n");
    printf("====================================================================\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    set_Console_Colors(true, WARNING_COLOR);
    printf("WARNING! Legacy ATA Passthrough methods use vendor unique operation codes!\n");
    printf("         Sometimes these operation code will cause problems on devices where\n");
    printf("         these operation codes have different meanings!\n");
    printf("         In some rare cases, this can render the adapter or device unusable!\n");
    printf("         Do not run these legacy methods on USB thumb-drives/flash drives!\n");
    printf("         These methods may be ok to run on USB HDDs from about 2007 & earlier\n");
    printf("         Newer devices should use SAT!\n");
    printf("         Press CTRL-C to abort this test immediately and remove the legacy test option\n");
    printf("         if you are unsure if you should proceed with the test!\n");
    for (uint8_t counter = 30; counter > 0; --counter)
    {
        printf("\r%2" PRIu8 " seconds until legacy test begins", counter);
        fflush(stdout);
        delay_Seconds(1);
    }
    printf("\n");
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);

    //Now that the users have been warned, it's time to try the different legacy methods until we get a successful identify command through to the device.
    inputs->device->drive_info.passThroughHacks.passthroughType = ATA_PASSTHROUGH_SAT + 1;//go to the next passthrough type after SAT since SAT is the default since it is the standard.
    while (inputs->device->drive_info.passThroughHacks.passthroughType != ATA_PASSTHROUGH_UNKNOWN)
    {
        uint8_t identifyData[LEGACY_DRIVE_SEC_SIZE] = { 0 };
        if (scsiInformation->inquiryData.peripheralDeviceType == PERIPHERAL_DIRECT_ACCESS_BLOCK_DEVICE)
        {
            if (SUCCESS == ata_Identify(inputs->device, identifyData, LEGACY_DRIVE_SEC_SIZE))
            {
                //command succeeded so this is most likely the correct pass-through type to use for this device
                //setting drive type while we're in here since it could help with a faster scan
                inputs->device->drive_info.drive_type = ATA_DRIVE;
                break;
            }
        }
        else if (scsiInformation->inquiryData.peripheralDeviceType == PERIPHERAL_CD_DVD_DEVICE)
        {
            if (SUCCESS == ata_Identify_Packet_Device(inputs->device, identifyData, LEGACY_DRIVE_SEC_SIZE))
            {
                //command succeeded so this is most likely the correct pass-through type to use for this device
                //setting drive type while we're in here since it could help with a faster scan
                inputs->device->drive_info.drive_type = ATAPI_DRIVE;
                break;
            }
        }
        else
        {
            //No other peripheral device type should be being used by these legacy devices. 
            //They were most likely only HDDs in the first place, MAYBE a CD/DVD drive.
            inputs->device->drive_info.passThroughHacks.passthroughType = ATA_PASSTHROUGH_UNKNOWN;
            break;
        }
        ++(inputs->device->drive_info.passThroughHacks.passthroughType);
    }
    if (inputs->device->drive_info.passThroughHacks.passthroughType != ATA_PASSTHROUGH_UNKNOWN)
    {
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: passthrough type seems to be \n");//no passthrough available at all
        switch (inputs->device->drive_info.passThroughHacks.passthroughType)
        {
        case ATA_PASSTHROUGH_CYPRESS:
            printf("Cypress\n");
            inputs->device->drive_info.passThroughHacks.ataPTHacks.ata28BitOnly = true;
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: ATA28\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            break;
        case ATA_PASSTHROUGH_PROLIFIC:
            printf("Prolific\n");
            break;
        case ATA_PASSTHROUGH_TI:
            printf("TI\n");
            inputs->device->drive_info.passThroughHacks.ataPTHacks.ata28BitOnly = true;
            inputs->device->drive_info.passThroughHacks.ataPTHacks.noMultipleModeCommands = true;
            set_Console_Colors(true, HACK_COLOR);
            printf("HACK FOUND: ATA28\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            break;
        case ATA_PASSTHROUGH_NEC:
            printf("NEC\n");
            break;
        case ATA_PASSTHROUGH_PSP:
            printf("PSP\n");
            break;
        default:
            printf("Unknown legacy device type! This means the passthrough test code needs updating!\n");
            break;
        }
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        legacyATAPassthroughSupported = true;
        bool smartSupported = false;
        bool smartLoggingSupported = false;
        bool sctSupported = false;
        setup_ATA_ID_Info(inputs, &smartSupported, &smartLoggingSupported, &sctSupported);

        if (inputs->device->drive_info.ata_Options.dmaMode != ATA_DMA_MODE_NO_DMA)
        {
            sat_DMA_UDMA_Protocol_Test(inputs->device, smartSupported, smartLoggingSupported);
        }
        else
        {
            printf("Skipping DMA check as device does not report any support for DMA mode commands\n");
            printf("It is strongly recommended to retest with a device that does support DMA mode commands\n");
        }

        if (!inputs->noMultiSectorPIOTest)
        {
            multi_Sector_PIO_Test(inputs->device, smartSupported, smartLoggingSupported);
            if (!inputs->device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode && !inputs->device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly)
            {
                printf("Multi-sector PIO commands seem to work properly\n");
            }
            else if (inputs->device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: MMPIO\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
            else if (inputs->device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly)
            {
                set_Console_Colors(true, HACK_COLOR);
                printf("HACK FOUND: SPIO\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
        }
        //TODO: additional testing here for legacy capabilities!
    }
    else
    {
        set_Console_Colors(true, HACK_COLOR);
        printf("HACK FOUND: NOPT\n");//no passthrough available at all
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        inputs->device->drive_info.passThroughHacks.passthroughType = ATA_PASSTHROUGH_UNKNOWN;
    }
    return legacyATAPassthroughSupported;
}

#define THIRTY_TWO_KB UINT32_C(32768)
#define MAX_SCSI_SECTORS_TO_TEST UINT32_C(4096)
static eReturnValues scsi_Max_Transfer_Length_Test(tDevice *device, uint32_t reportedMax, uint32_t reportedOptimal)
{
    uint32_t maxTestSizeBlocks = MAX_SCSI_SECTORS_TO_TEST;
    if (reportedMax > 0)
    {
        if (reportedMax < maxTestSizeBlocks)
        {
            //drop down to here!
            maxTestSizeBlocks = reportedMax;
        }
    }
    size_t dataBufSize = C_CAST(size_t, maxTestSizeBlocks) * C_CAST(size_t, device->drive_info.deviceBlockSize);
    uint8_t *data = C_CAST(uint8_t*, calloc_aligned(dataBufSize, sizeof(uint8_t), device->os_info.minimumAlignment));
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n==================================\n");
    printf("Testing SCSI Maximum Transfer Size\n");
    printf("==================================\n");
    set_Console_Colors(true, NOTE_COLOR);
    printf("NOTE: This is currently limited to %" PRIu32 " sectors for now\n", maxTestSizeBlocks);
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    if (!data)
    {
        set_Console_Colors(true, ERROR_COLOR);
        printf("Fatal error, unable to allocate %" PRIu32 " sectors worth of memory to perform SCSI pass-through test.\n", maxTestSizeBlocks);
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        return MEMORY_FAILURE;
    }
    //start at 1 sector, then double until 32k. From here, increment one sector at a time since most USB devices only handle 32k properly.
    uint32_t transferLengthSectors = 1;
    eReturnValues readResult = SUCCESS;
    while (transferLengthSectors <= maxTestSizeBlocks && readResult == SUCCESS)
    {
        readResult = scsi_Read(device, 0, false, data, transferLengthSectors * device->drive_info.deviceBlockSize);
        if (readResult == SUCCESS)
        {
            device->drive_info.passThroughHacks.scsiHacks.maxTransferLength = transferLengthSectors * device->drive_info.deviceBlockSize;
        }
        if ((transferLengthSectors * device->drive_info.deviceBlockSize) < THIRTY_TWO_KB)
        {
            transferLengthSectors *= UINT32_C(2);
        }
        else
        {
            transferLengthSectors += UINT32_C(1);
        }
    }
    safe_Free_aligned(data)
    scsi_Test_Unit_Ready(device, NULL);
    printf("SCSI Max Transfer Size: %" PRIu32 "B\n", device->drive_info.passThroughHacks.scsiHacks.maxTransferLength);
    if (reportedMax > 0)
    {
        printf("SCSI reported max size: %" PRIu32 "B\n", reportedMax * device->drive_info.deviceBlockSize);
    }
    if (reportedOptimal > 0)
    {
        printf("SCSI reported optimal size: %" PRIu32 "B\n", reportedOptimal * device->drive_info.deviceBlockSize);
    }
    return SUCCESS;
}


static eReturnValues ata_PT_Read(tDevice *device, uint64_t lba, bool async, uint8_t *ptrData, uint32_t dataSize)
{
    eReturnValues ret = SUCCESS;//assume success
    uint16_t sectors = 0;
    //make sure that the data size is at least logical sector in size
    if (dataSize < device->drive_info.bridge_info.childDeviceBlockSize)
    {
        return BAD_PARAMETER;
    }
    uint32_t totalTransferSize = dataSize / device->drive_info.bridge_info.childDeviceBlockSize;
    if (totalTransferSize > 65536)
    {
        return BAD_PARAMETER;
    }
    else if (totalTransferSize == 65536)
    {
        sectors = 0;
    }
    else
    {
        sectors = C_CAST(uint16_t, dataSize / device->drive_info.bridge_info.childDeviceBlockSize);
    }
    if (async)
    {
        //asynchronous not supported yet
        return NOT_SUPPORTED;
    }
    else //synchronous reads
    {
        if (device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported && !device->drive_info.passThroughHacks.ataPTHacks.ata28BitOnly)
        {
            //use 48bit commands by default
            //make sure the LBA is within range
            if (lba > MAX_48_BIT_LBA)
            {
                ret = BAD_PARAMETER;
            }
            else
            {
                if (device->drive_info.ata_Options.dmaMode == ATA_DMA_MODE_NO_DMA || device->drive_info.passThroughHacks.ataPTHacks.dmaNotSupported)
                {
                    //use PIO commands
                    //check if read multiple is supported (current # logical sectors per DRQ data block)
                    //Also, only bother with read multiple if it's a PATA drive. There isn't really an advantage to this on SATA other than backwards compatibility.
                    if (!device->drive_info.passThroughHacks.ataPTHacks.noMultipleModeCommands && device->drive_info.ata_Options.readWriteMultipleSupported && device->drive_info.ata_Options.logicalSectorsPerDRQDataBlock > 0 && device->drive_info.ata_Options.logicalSectorsPerDRQDataBlock <= ATA_MAX_BLOCKS_PER_DRQ_DATA_BLOCKS && device->drive_info.ata_Options.isParallelTransport)
                    {
                        //read multiple supported and drive is currently configured in a mode that will work.
                        if (device->drive_info.ata_Options.chsModeOnly)
                        {
                            uint16_t cylinder = 0;
                            uint8_t head = 0;
                            uint8_t sector = 0;
                            if (SUCCESS == convert_LBA_To_CHS(device, C_CAST(uint32_t, lba), &cylinder, &head, &sector))
                            {
                                ret = ata_Legacy_Read_Multiple_CHS(device, cylinder, head, sector, ptrData, sectors, dataSize, true);
                            }
                            else //Couldn't convert or the LBA is greater than the current CHS mode
                            {
                                ret = NOT_SUPPORTED;
                            }
                        }
                        else
                        {
                            ret = ata_Read_Multiple(device, lba, ptrData, sectors, dataSize, true);
                        }
                    }
                    else
                    {
                        if (device->drive_info.ata_Options.chsModeOnly)
                        {
                            uint16_t cylinder = 0;
                            uint8_t head = 0;
                            uint8_t sector = 0;
                            if (SUCCESS == convert_LBA_To_CHS(device, C_CAST(uint32_t, lba), &cylinder, &head, &sector))
                            {
                                ret = ata_Legacy_Read_Sectors_CHS(device, cylinder, head, sector, ptrData, sectors, dataSize, true);
                            }
                            else //Couldn't convert or the LBA is greater than the current CHS mode
                            {
                                ret = NOT_SUPPORTED;
                            }
                        }
                        else
                        {
                            ret = ata_Read_Sectors(device, lba, ptrData, sectors, dataSize, true);
                        }
                    }
                }
                else
                {
                    if (device->drive_info.ata_Options.chsModeOnly)
                    {
                        uint16_t cylinder = 0;
                        uint8_t head = 0;
                        uint8_t sector = 0;
                        if (SUCCESS == convert_LBA_To_CHS(device, C_CAST(uint32_t, lba), &cylinder, &head, &sector))
                        {
                            ret = ata_Legacy_Read_DMA_CHS(device, cylinder, head, sector, ptrData, sectors, dataSize, true);
                        }
                        else //Couldn't convert or the LBA is greater than the current CHS mode
                        {
                            ret = NOT_SUPPORTED;
                        }
                    }
                    else
                    {
                        //use DMA commands
                        ret = ata_Read_DMA(device, lba, ptrData, sectors, dataSize, true);
                    }
                    if (ret != SUCCESS)
                    {
                        //check the sense data. Make sure we didn't get told we have an invalid field in the CDB.
                        //If we do, try turning off DMA mode and retrying with PIO mode commands.
                        uint8_t senseKey = 0, asc = 0, ascq = 0, fru = 0;
                        get_Sense_Key_ASC_ASCQ_FRU(device->drive_info.lastCommandSenseData, SPC3_SENSE_LEN, &senseKey, &asc, &ascq, &fru);
                        //Checking for illegal request, invalid field in CDB since this is what we've seen reported when DMA commands are not supported.
                        if (senseKey == SENSE_KEY_ILLEGAL_REQUEST && asc == 0x24 && ascq == 0x00)
                        {
                            //turn off DMA mode
                            eATASynchronousDMAMode currentDMAMode = device->drive_info.ata_Options.dmaMode;
                            device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_NO_DMA;//turning off DMA to try PIO mode
                            //recursively call this function to retry in PIO mode.
                            ret = ata_Read(device, lba, async, ptrData, dataSize);
                            if (ret != SUCCESS)
                            {
                                //this means that the error is not related to DMA mode command, so we can turn that back on and pass up the return status.
                                device->drive_info.ata_Options.dmaMode = currentDMAMode;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            //use the 28bit commands...first check that they aren't requesting more data than can be transferred in a 28bit command, exception being 256 since that can be represented by a 0
            if (sectors > 256 || sectors == 0)
            {
                ret = BAD_PARAMETER;
            }
            else
            {
                if (sectors == 256)
                {
                    sectors = 0;
                }
                //make sure the LBA is within range
                if (lba > MAX_28_BIT_LBA)
                {
                    ret = BAD_PARAMETER;
                }
                else
                {
                    if (device->drive_info.ata_Options.dmaMode == ATA_DMA_MODE_NO_DMA)
                    {
                        //use PIO commands
                        //check if read multiple is supported (current # logical sectors per DRQ data block)
                        //Also, only bother with read multiple if it's a PATA drive. There isn't really an advantage to this on SATA other than backwards compatibility.
                        if (!device->drive_info.passThroughHacks.ataPTHacks.noMultipleModeCommands && device->drive_info.ata_Options.readWriteMultipleSupported && device->drive_info.ata_Options.logicalSectorsPerDRQDataBlock > 0 && device->drive_info.ata_Options.logicalSectorsPerDRQDataBlock <= ATA_MAX_BLOCKS_PER_DRQ_DATA_BLOCKS && device->drive_info.ata_Options.isParallelTransport)
                        {
                            //read multiple supported and drive is currently configured in a mode that will work.
                            if (device->drive_info.ata_Options.chsModeOnly)
                            {
                                uint16_t cylinder = 0;
                                uint8_t head = 0;
                                uint8_t sector = 0;
                                if (SUCCESS == convert_LBA_To_CHS(device, C_CAST(uint32_t, lba), &cylinder, &head, &sector))
                                {
                                    ret = ata_Legacy_Read_Multiple_CHS(device, cylinder, head, sector, ptrData, sectors, dataSize, false);
                                }
                                else //Couldn't convert or the LBA is greater than the current CHS mode
                                {
                                    ret = NOT_SUPPORTED;
                                }
                            }
                            else
                            {
                                ret = ata_Read_Multiple(device, lba, ptrData, sectors, dataSize, false);
                            }
                        }
                        else
                        {
                            if (device->drive_info.ata_Options.chsModeOnly)
                            {
                                uint16_t cylinder = 0;
                                uint8_t head = 0;
                                uint8_t sector = 0;
                                if (SUCCESS == convert_LBA_To_CHS(device, C_CAST(uint32_t, lba), &cylinder, &head, &sector))
                                {
                                    ret = ata_Legacy_Read_Sectors_CHS(device, cylinder, head, sector, ptrData, sectors, dataSize, false);
                                }
                                else //Couldn't convert or the LBA is greater than the current CHS mode
                                {
                                    ret = NOT_SUPPORTED;
                                }
                            }
                            else
                            {
                                ret = ata_Read_Sectors(device, lba, ptrData, sectors, dataSize, false);
                            }
                        }
                    }
                    else
                    {
                        if (device->drive_info.ata_Options.chsModeOnly)
                        {
                            uint16_t cylinder = 0;
                            uint8_t head = 0;
                            uint8_t sector = 0;
                            if (SUCCESS == convert_LBA_To_CHS(device, C_CAST(uint32_t, lba), &cylinder, &head, &sector))
                            {
                                ret = ata_Legacy_Read_DMA_CHS(device, cylinder, head, sector, ptrData, sectors, dataSize, false);
                            }
                            else //Couldn't convert or the LBA is greater than the current CHS mode
                            {
                                ret = NOT_SUPPORTED;
                            }
                        }
                        else
                        {
                            //use DMA commands
                            ret = ata_Read_DMA(device, lba, ptrData, sectors, dataSize, false);
                        }
                        if (ret != SUCCESS)
                        {
                            //check the sense data. Make sure we didn't get told we have an invalid field in the CDB.
                            //If we do, try turning off DMA mode and retrying with PIO mode commands.
                            uint8_t senseKey = 0, asc = 0, ascq = 0, fru = 0;
                            get_Sense_Key_ASC_ASCQ_FRU(device->drive_info.lastCommandSenseData, SPC3_SENSE_LEN, &senseKey, &asc, &ascq, &fru);
                            //Checking for illegal request, invalid field in CDB since this is what we've seen reported when DMA commands are not supported.
                            if (senseKey == SENSE_KEY_ILLEGAL_REQUEST && asc == 0x24 && ascq == 0x00)
                            {
                                //turn off DMA mode
                                eATASynchronousDMAMode currentDMAMode = device->drive_info.ata_Options.dmaMode;
                                device->drive_info.ata_Options.dmaMode = ATA_DMA_MODE_NO_DMA;//turning off DMA to try PIO mode
                                //recursively call this function to retry in PIO mode.
                                ret = ata_Read(device, lba, async, ptrData, dataSize);
                                if (ret != SUCCESS)
                                {
                                    //this means that the error is not related to DMA mode command, so we can turn that back on and pass up the return status.
                                    device->drive_info.ata_Options.dmaMode = currentDMAMode;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ret;
}

#define MAX_ATA_SECTORS_TO_TEST UINT32_C(4096)
static eReturnValues ata_Passthrough_Max_Transfer_Length_Test(tDevice *device, uint32_t scsiReportedMax, uint32_t scsiReportedOptimal)
{
    uint32_t maxTestSizeBlocks = MAX_ATA_SECTORS_TO_TEST;
    if (scsiReportedMax > 0)
    {
        if (scsiReportedMax < maxTestSizeBlocks)
        {
            //drop down to here!
            maxTestSizeBlocks = scsiReportedMax;
        }
    }
    size_t dataBufSize = C_CAST(size_t, maxTestSizeBlocks) * C_CAST(size_t, device->drive_info.bridge_info.childDeviceBlockSize);
    uint8_t *data = C_CAST(uint8_t*, calloc_aligned(dataBufSize, sizeof(uint8_t), device->os_info.minimumAlignment));
    set_Console_Colors(true, HEADING_COLOR);
    printf("\n=============================================\n");
    printf("Testing ATA Pass-through Maximum transfer size\n");
    printf("=============================================\n");
    set_Console_Colors(true, NOTE_COLOR);
    printf("NOTE: This is currently limited to %" PRIu32 " sectors for now\n", maxTestSizeBlocks);
    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
    if (!data)
    {
        set_Console_Colors(true, ERROR_COLOR);
        printf("Fatal error, unable to allocate %" PRIu32 " sectors worth of memory to perform ATA pass-through test.\n", maxTestSizeBlocks);
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        return MEMORY_FAILURE;
    }
    //start at 1 sector, then double until 32k. From here, increment one sector at a time since most USB devices only handle 32k properly.
    uint32_t transferLengthSectors = 1;
    eReturnValues readResult = SUCCESS;
    while (transferLengthSectors <= maxTestSizeBlocks && readResult == SUCCESS)
    {
        readResult = ata_PT_Read(device, 0, false, data, transferLengthSectors * device->drive_info.bridge_info.childDeviceBlockSize);
        scsi_Test_Unit_Ready(device, NULL);
        if (readResult == SUCCESS)
        {
            device->drive_info.passThroughHacks.ataPTHacks.maxTransferLength = transferLengthSectors * device->drive_info.bridge_info.childDeviceBlockSize;
        }
        if ((transferLengthSectors * device->drive_info.bridge_info.childDeviceBlockSize) < THIRTY_TWO_KB)
        {
            transferLengthSectors *= UINT32_C(2);
        }
        else
        {
            transferLengthSectors += UINT32_C(1);
        }
    }
    safe_Free_aligned(data)
    scsi_Test_Unit_Ready(device, NULL);
    printf("ATA Max Transfer Size: %" PRIu32 "B\n", device->drive_info.passThroughHacks.ataPTHacks.maxTransferLength);
    if (scsiReportedMax > 0)
    {
        printf("SCSI reported max size: %" PRIu32 "B\n", scsiReportedMax * device->drive_info.deviceBlockSize);
    }
    if (scsiReportedOptimal > 0)
    {
        printf("SCSI reported optimal size: %" PRIu32 "B\n", scsiReportedOptimal * device->drive_info.deviceBlockSize);
    }
    return SUCCESS;
}

eReturnValues perform_Passthrough_Test(ptrPassthroughTestParams inputs)
{
    eReturnValues ret = SUCCESS;
    printf("Performing Pass-through test. \n");
    printf("If at any point during the test, a crash, a hang, or the device\n");
    printf("seems to stop responding to any normal request, save all previously\n");
    printf("reported hacks, stop the tool, reset the device, then restart\n");
    printf("the test, skipping to another part of the test to avoid the same issue.\n");
    printf("All discovered hacks will be necessary in order to fully communicate\n");
    printf("with the end target device to the highest capabilities of the adapter or\n");
    printf("bridge performing the translations.\n\n");

    //TODO: Need a way to report when an OS doesn't provide the information necessary to setup hacks based on IDs.

    if (inputs && inputs->device)
    {
        //step 1, check if already in the code database.
        if (setup_Passthrough_Hacks_By_ID(inputs->device) && !inputs->forceRetest)
        {
            printf("This device is already in the database with known pass-through hacks.\n");
            printf("Use the --%s option to force a retest of pass-through hacks already known.\n", "forceRetest");
            return SUCCESS;
        }
        //Need to clear out the hacks or incorrect results may be found since they will be used while testing.
        memset(&inputs->device->drive_info.passThroughHacks, 0, sizeof(passthroughHacks));

        //2. Check what things the device reports for SCSI capabilities, SAT VPD page, etc. Emit warnings for pages that are missing that were expected
        printf("Checking standard SCSI inquiry data, VPD pages, and some mode pages\n");
        printf("to understand device capabilities. Only commands specified in translator\n");
        printf("specifications will be tested.\n");

        //TODO: save information from some pages to compare later to the data retrieved through passthrough
        scsiDevInformation scsiInformation;
        memset(&scsiInformation, 0, sizeof(scsiDevInformation));
        scsi_Information(inputs->device, &scsiInformation);
        scsi_VPD_Pages(inputs->device, &scsiInformation);
        scsi_Capacity_Information(inputs->device, &scsiInformation);

        //check SCSI read/write CDB support. This runs a normal test, but will check for zero-length transfers IF asked to to do.
        scsiRWSupport rwSupport;
        memset(&rwSupport, 0, sizeof(scsiRWSupport));
        scsi_Read_Check(inputs->device, false, &rwSupport, (inputs->testPotentiallyDeviceHangingCommands && inputs->hangCommandsToTest.zeroLengthReads) ? true : false);

        //Now check for mode pages - Warn about any missing MANDATORY pages
        scsi_Mode_Information(inputs->device, &scsiInformation);
        //Now check for log pages - Warn about any missing MANDATORY pages
        //This is likely where we'll find issues with subpages not being supported, but the device returning data anyways
        scsi_Log_Information(inputs->device, &scsiInformation);

        //5. optionally do a more in depth check for additional SCSI commands like report supported operation codes that would also be useful, or security protocol commands.
        otherSCSICmdSupport supScsiCmds;
        memset(&supScsiCmds, 0, sizeof(otherSCSICmdSupport));
        other_SCSI_Cmd_Support(inputs->device, &supScsiCmds);

        //now perform a test to check the device error handling. Some have poor error handling and time to report errors grows with each command slowing the whole device down.
        double relativeCommandProcessingPerformance = 0;
        scsi_Error_Handling_Test(inputs->device, &relativeCommandProcessingPerformance);

        scsi_Max_Transfer_Length_Test(inputs->device, scsiInformation.vpdData.blockLimitsData.maximumXferLen, scsiInformation.vpdData.blockLimitsData.optimalXferLen);

        //3. Start checking for SAT or VS NVMe passthrough, unless given information to use a different passthrough.
        //TODO: Make sure to do this only for direct access block devices OR zoned block devices
        //TODO: Move these passthrough tests to separate functions.
        if (!inputs->disableTranslatorPassthroughTesting)
        {
            if (strncmp(scsiInformation.inquiryData.vendorId, "NVMe", 4) == 0 || inputs->suspectedDriveType == NVME_DRIVE) //NVMe
            {
                set_Console_Colors(true, HEADING_COLOR);
                printf("\n==================================================\n");
                printf("Checking Vendor Specific NVMe-passthrough commands\n");
                printf("==================================================\n");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                //TODO: If given a specific NVMe passthrough type, only test that
                inputs->device->drive_info.passThroughHacks.passthroughType = NVME_PASSTHROUGH_UNKNOWN;
                if (inputs->device->drive_info.interface_type != NVME_INTERFACE)
                {
                    //Try sending NVMe passthrough commands to see if one works.
                    inputs->device->drive_info.passThroughHacks.passthroughType = NVME_PASSTHROUGH_JMICRON;//first NVMe passthrough in the list
                    while (inputs->device->drive_info.passThroughHacks.passthroughType < NVME_PASSTHROUGH_UNKNOWN)
                    {
                        //Try admin identify controller until we get success or run out of passthroughs to try
                        if (SUCCESS == nvme_Identify(inputs->device, (uint8_t*)&inputs->device->drive_info.IdentifyData.nvme.ctrl, 0, 1))
                        {
                            break;
                        }
                        ++inputs->device->drive_info.passThroughHacks.passthroughType;
                    }
                    switch (inputs->device->drive_info.passThroughHacks.passthroughType)
                    {
                    case NVME_PASSTHROUGH_JMICRON:
                        set_Console_Colors(true, HACK_COLOR);
                        printf("HACK FOUND: JMNVME\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        break;
                    case NVME_PASSTHROUGH_ASMEDIA_BASIC:
                        set_Console_Colors(true, HACK_COLOR);
                        printf("HACK FOUND: ASMNVMEBASIC\n");
                        printf("HACK FOUND: IDGLP\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedPassthroughCapabilities = true;
                        inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedCommandsSupported.identifyController = true;
                        inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedCommandsSupported.identifyNamespace = true;
                        inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedCommandsSupported.getLogPage = true;
                        break;
                    case NVME_PASSTHROUGH_ASMEDIA:
                        set_Console_Colors(true, HACK_COLOR);
                        printf("HACK FOUND: ASMNVME\n");
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        break;
                    default:
                        printf("No NVMe pass-through detected for this device!\n");
                        break;
                    }
                    //TODO: Based on passthrough type, do more testing for max xferlength, etc
                }
            }
            else if (strncmp(scsiInformation.inquiryData.vendorId, "ATA", 3) == 0 || inputs->suspectedDriveType == ATA_DRIVE || inputs->allowLegacyATAPTTest) //ATA
            {
                //TODO: need to move this SAT testing to another function.
                if (!test_SAT_Capabilities(inputs, &scsiInformation))
                {
                    set_Console_Colors(true, ERROR_COLOR);
                    printf("ERROR: SAT Pass-through failed both 12B and 16B CDBs.\n");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                    if (inputs->allowLegacyATAPTTest)
                    {
                        test_Legacy_ATA_Passthrough(inputs, &scsiInformation);
                    }
                    else
                    {
                        set_Console_Colors(true, HACK_COLOR);
                        printf("HACK FOUND: NOPT\n");//no passthrough available at all
                        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                        inputs->device->drive_info.passThroughHacks.passthroughType = ATA_PASSTHROUGH_UNKNOWN;
                    }
                }
                if (inputs->device->drive_info.passThroughHacks.passthroughType != ATA_PASSTHROUGH_UNKNOWN)
                {
                    ata_Passthrough_Max_Transfer_Length_Test(inputs->device, scsiInformation.vpdData.blockLimitsData.maximumXferLen, scsiInformation.vpdData.blockLimitsData.optimalXferLen);
                }
            }
        }
        else
        {
            set_Console_Colors(true, NOTE_COLOR);
            printf("NOTE: Attempting passthrough CDBs for SAT or a vendor unique methods has been disabled and is being skipped.\n");
            set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        }

        //Finally. Display the results and which hacks were found while testing the device.

        set_Console_Colors(true, HEADING_COLOR);
        printf("\n\n==================\n");
        printf("Final Test Results\n");
        printf("==================\n");
        set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
        printf("SEND THIS INFO BELOW TO seaboard@seagate.com:\n");
        //low-level OS device VID/PID/REV as available
        if (inputs->device->drive_info.adapter_info.infoType != ADAPTER_INFO_UNKNOWN)
        {
            printf("Adapter Information:\n");
            printf("\tInformation Type: ");
            switch (inputs->device->drive_info.adapter_info.infoType)
            {
            case ADAPTER_INFO_IEEE1394:
                printf("IEEE 1394\n");
                break;
            case ADAPTER_INFO_PCI:
                printf("PCI\n");
                break;
            case ADAPTER_INFO_USB:
                printf("USB\n");
                break;
            default:
                printf("Unknown\n");
                break;
            }
            if (inputs->device->drive_info.adapter_info.vendorIDValid)
            {
                printf("\tVendor ID: %04" PRIX16 "h\n", inputs->device->drive_info.adapter_info.vendorID);
            }
            else
            {
                printf("\tVendor ID: Did not get vendor ID from OS.\n");
            }
            if (inputs->device->drive_info.adapter_info.productIDValid)
            {
                printf("\tProduct ID: %04" PRIX16 "h\n", inputs->device->drive_info.adapter_info.productID);
            }
            else
            {
                printf("\tProduct ID: Did not get vendor ID from OS.\n");
            }
            if (inputs->device->drive_info.adapter_info.revisionValid)
            {
                printf("\tRevision: %04" PRIX16 "h\n", inputs->device->drive_info.adapter_info.revision);
            }
            else
            {
                printf("\tRevision: Did not get vendor ID from OS.\n");
            }
        }
        //MN, designators, firmware, SAT vendor etc
        if (!inputs->device->drive_info.passThroughHacks.scsiHacks.preSCSI2InqData)
        {
            printf("\tVendor ID: %s\n", scsiInformation.inquiryData.vendorId);
            printf("\tProduct ID: %s\n", scsiInformation.inquiryData.productId);
            printf("\tProduct Rev: %s\n", scsiInformation.inquiryData.productRev);
            if (strlen(scsiInformation.vpdData.unitSN) > 0)
            {
                printf("\tUnit Serial Number: %s\n", scsiInformation.vpdData.unitSN);
            }
            if (scsiInformation.vpdData.gotSATVPDPage)
            {
                printf("\tSAT Vendor ID: %s\n", inputs->device->drive_info.bridge_info.t10SATvendorID);
                printf("\tSAT Product ID: %s\n", inputs->device->drive_info.bridge_info.SATproductID);
                printf("\tSAT Product Rev: %s\n", inputs->device->drive_info.bridge_info.SATfwRev);
            }
            //TODO: Designator information that might be benefitial!!!
            if (scsiInformation.vpdData.gotDeviceIDVPDPage)
            {
                printf("\tDevice IDs:\n");
                for (uint8_t iter = 0; iter < scsiInformation.vpdData.designatorCount; ++iter)
                {
                    if (scsiInformation.vpdData.designators[iter].valid)
                    {
                        printf("\t\t");
                        for (uint16_t desIter = 0; desIter < scsiInformation.vpdData.designators[iter].designatorLength && desIter < UINT16_C(16); ++desIter)
                        {
                            printf("%02" PRIX8, scsiInformation.vpdData.designators[iter].designator[desIter]);
                        }
                        printf("\n");
                    }
                }
            }
        }
        else
        {
            printf("\tDevice information could not be detected by this tool since it was reported in a legacy\n");
            printf("\tvendor specific format from old SCSI or CCS (pre-SCSI2) standards. Detecting this must\n");
            printf("\tbe done manually by specifying specific offsets at this time for optimal support.\n");
            printf("\tPlease include the full output from the tool when sending to seaboard@seagate.com\n");
        }
        printf("\tCommand Processing (bad relative to good): %0.02f\n", relativeCommandProcessingPerformance);

        if (inputs->device->drive_info.passThroughHacks.scsiHacks.maxTransferLength < (MAX_SCSI_SECTORS_TO_TEST * inputs->device->drive_info.deviceBlockSize))
        {
            printf("\tSCSI Max Transfer Size: %" PRIu32 "\n", inputs->device->drive_info.passThroughHacks.scsiHacks.maxTransferLength);
        }
        if (inputs->device->drive_info.passThroughHacks.passthroughType < ATA_PASSTHROUGH_UNKNOWN && (inputs->device->drive_info.drive_type != NVME_DRIVE || (inputs->suspectedDriveTypeProvidedByUser && inputs->suspectedDriveType != NVME_DRIVE)) && inputs->device->drive_info.passThroughHacks.ataPTHacks.maxTransferLength < (MAX_ATA_SECTORS_TO_TEST * inputs->device->drive_info.bridge_info.childDeviceBlockSize))
        {
            printf("\tPassthrough Max Transfer Size: %" PRIu32 "\n", inputs->device->drive_info.passThroughHacks.ataPTHacks.maxTransferLength);
        }
        //TODO: NVMe passthrough max transfer size

        printf("\nHacks For This Device:\n");
        //go through and print each one on a new line based on what the test found.
        //This should focus on generic SCSI data reporting first, followed by SAT/NVMe/Legacy Passthrough hacks after that
        //Add a warning that these hacks should be tested on another tool with the (TODO) deviceHacks command line option to make sure everything functions optimally.
        if (inputs->device->drive_info.passThroughHacks.testUnitReadyAfterAnyCommandFailure)
        {
            printf("\t\tTURF:%" PRIu8 "\n", inputs->device->drive_info.passThroughHacks.turfValue);
        }
        printf("\tSCSI Hacks:");
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.preSCSI2InqData)
        {
            printf(" PRESCSI2,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.unitSNAvailable)
        {
            printf(" UNA,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.readWrite.available)
        {
            if (inputs->device->drive_info.passThroughHacks.scsiHacks.readWrite.rw6)
            {
                printf(" RW6,");
            }
            if (inputs->device->drive_info.passThroughHacks.scsiHacks.readWrite.rw10)
            {
                printf(" RW10,");
            }
            if (inputs->device->drive_info.passThroughHacks.scsiHacks.readWrite.rw12)
            {
                printf(" RW12,");
            }
            if (inputs->device->drive_info.passThroughHacks.scsiHacks.readWrite.rw16)
            {
                printf(" RW16,");
            }
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.noVPDPages)
        {
            printf(" NVPD,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.noModePages)
        {
            printf(" NMP,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.noLogPages)
        {
            printf(" NLP,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.noLogSubPages)
        {
            printf(" NLPS,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.mode6bytes)
        {
            printf(" MP6,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.noModeSubPages)
        {
            printf(" NMSP,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.noReportSupportedOperations)
        {
            printf(" NRSUPOP,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.reportSingleOpCodes)
        {
            printf(" SUPSOP,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.reportAllOpCodes)
        {
            printf(" REPALLOP,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.securityProtocolSupported)
        {
            printf(" SECPROT,");
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.securityProtocolWithInc512)
        {
            printf(" SECPROTI512,");
        }
        if (inputs->testPotentiallyDeviceHangingCommands)
        {
            if (!inputs->hangCommandsToTest.zeroLengthReads)
            {
                set_Console_Colors(true, LIKELY_HACK_COLOR);
                printf(" NORWZ,");
                set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
            }
        }
        if (inputs->device->drive_info.passThroughHacks.scsiHacks.maxTransferLength < (MAX_SCSI_SECTORS_TO_TEST * inputs->device->drive_info.deviceBlockSize))
        {
            printf(" MXFER:%" PRIu32 "\n", inputs->device->drive_info.passThroughHacks.scsiHacks.maxTransferLength);
        }
        //////////////////////////////////////////////////////////////////////////////// 
        //TODO: if NVMe don't show this, but rather NVMe specific things. 
        if (inputs->device->drive_info.passThroughHacks.passthroughType < ATA_PASSTHROUGH_UNKNOWN && (inputs->device->drive_info.drive_type != NVME_DRIVE || (inputs->suspectedDriveTypeProvidedByUser && inputs->suspectedDriveType != NVME_DRIVE)))
        {
            printf("\tATA Hacks: ");
            switch (inputs->device->drive_info.passThroughHacks.passthroughType)
            {
            case ATA_PASSTHROUGH_SAT:
                printf(" SAT,");
                break;
            case ATA_PASSTHROUGH_CYPRESS:
                printf(" APTCYPRESS,");
                break;
            case ATA_PASSTHROUGH_PROLIFIC:
                printf(" APTPROLIFIC,");
                break;
            case ATA_PASSTHROUGH_TI:
                printf(" APTTI,");
                break;
            case ATA_PASSTHROUGH_NEC:
                printf(" APTNEC,");
                break;
            case ATA_PASSTHROUGH_PSP:
                printf(" APTPSP,");
                break;
            default:
                printf(" NOPT,");
                break;
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.ata28BitOnly)
            {
                printf(" ATA28,");
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.smartCommandTransportWithSMARTLogCommandsOnly)
            {
                printf(" SCTSM,");
            }
            if (inputs->device->drive_info.passThroughHacks.passthroughType == ATA_PASSTHROUGH_SAT)
            {
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.a1NeverSupported)
                {
                    printf(" NA1,");
                }
                else
                {
                    printf(" A1,");
                }
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.a1ExtCommandWhenPossible)
                {
                    printf(" A1EXT,");
                }
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.returnResponseInfoSupported)
                {
                    printf(" RS,");
                    if (inputs->device->drive_info.passThroughHacks.ataPTHacks.returnResponseInfoNeedsTDIR)
                    {
                        printf(" RSTD,");
                    }
                    if (inputs->device->drive_info.passThroughHacks.ataPTHacks.returnResponseIgnoreExtendBit)
                    {
                        printf(" RSIE,");
                    }
                }
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysUseTPSIUForSATPassthrough)
                {
                    printf(" TPSIU,");
                }
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.limitedUseTPSIU)
                {
                    printf(" TPID,");
                }
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysCheckConditionAvailable)
                {
                    printf(" CHK,");
                }
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.checkConditionEmpty)
                {
                    printf(" CHKE,");
                }
                if (inputs->device->drive_info.passThroughHacks.ataPTHacks.disableCheckCondition)
                {
                    printf(" NCHK,");
                }
                if (!inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysCheckConditionAvailable &&
                    !inputs->device->drive_info.passThroughHacks.ataPTHacks.checkConditionEmpty &&
                    !inputs->device->drive_info.passThroughHacks.ataPTHacks.disableCheckCondition)
                {
                    printf(" CHKND,");
                }
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.alwaysUseDMAInsteadOfUDMA)
            {
                printf(" FDMA,");
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.partialRTFRs)
            {
                printf(" PARTRTFR,");
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.noRTFRsPossible)
            {
                printf(" NORTFR,");
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
            {
                printf(" MMPIO,");
            }
            else if (inputs->device->drive_info.passThroughHacks.ataPTHacks.noMultipleModeCommands)
            {
                printf(" NOMMPIO,");
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly)
            {
                printf(" SPIO,");
            }
            if (inputs->device->drive_info.ata_Options.dmaMode == ATA_DMA_MODE_NO_DMA)
            {
                printf(" NDMA,");
            }
            else if (inputs->device->drive_info.ata_Options.dmaMode < ATA_DMA_MODE_UDMA)
            {
                printf(" FDMA,");
            }
            if (inputs->testPotentiallyDeviceHangingCommands)
            {
                if (!inputs->hangCommandsToTest.sctLogWithGPL)
                {
                    set_Console_Colors(true, LIKELY_HACK_COLOR);
                    printf(" SCTSM,");// -please retest to ensure that reading the SCT status log with GPL commands is indeed a necessary hack");
                    set_Console_Colors(true, CONSOLE_COLOR_DEFAULT);
                }
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.maxTransferLength < (MAX_ATA_SECTORS_TO_TEST * inputs->device->drive_info.bridge_info.childDeviceBlockSize))
            {
                printf(" MPTXFER:%" PRIu32 "\n", inputs->device->drive_info.passThroughHacks.ataPTHacks.maxTransferLength);
            }
        }
        else if ((inputs->suspectedDriveTypeProvidedByUser && inputs->suspectedDriveType == NVME_DRIVE) || (inputs->device->drive_info.passThroughHacks.passthroughType >= NVME_PASSTHROUGH_JMICRON && inputs->device->drive_info.passThroughHacks.passthroughType < NVME_PASSTHROUGH_UNKNOWN))
        {
            printf("\tNVMe Hacks: ");
            switch (inputs->device->drive_info.passThroughHacks.passthroughType)
            {
            case NVME_PASSTHROUGH_JMICRON:
                printf(" JMNVME,");
                break;
            case NVME_PASSTHROUGH_ASMEDIA_BASIC:
                printf(" ASMNVMEBASIC,");
                break;
            case NVME_PASSTHROUGH_ASMEDIA:
                printf(" ASMNVME,");
                break;
            default:
                printf(" NOPT\n");
                break;
            }
            if (inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedPassthroughCapabilities)
            {
                printf(" LIMPT,");
                if (inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedCommandsSupported.getLogPage &&
                    (inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedCommandsSupported.identifyGeneric ||
                    (inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedCommandsSupported.identifyController && inputs->device->drive_info.passThroughHacks.nvmePTHacks.limitedCommandsSupported.identifyNamespace)
                        )
                    )
                {
                    printf(" IDGLP,");
                }
            }
        }
        else
        {
            //printf("\t\tNOPT\n");
        }
        printf("\nRecommendations For Device Makers:\n");
        //Add recommendations based on the hacks to improve in future products
        //Do this based on SCSI reported information. Things that appear to possibly be missing, etc.
        //If a translator is found or suspected, mention pursuing a standardized passthrough if not already using one. If one doesn't exist, suggest creating one.
        uint8_t recommendationCounter = 1;
        printf("%" PRIu8 "\tTest this device with this tool in multiple OS's. Linux and Windows testing should be done to find highest compatibility.\n", recommendationCounter);
        ++recommendationCounter;
        if (inputs->device->drive_info.passThroughHacks.testUnitReadyAfterAnyCommandFailure)
        {
            printf("%" PRIu8 "\tImprove error handling for unknown commands. This device takes longer\n", recommendationCounter);
            printf("\t    and longer to respond with each unknown command.\n");
            ++recommendationCounter;
        }
        if (inputs->device->drive_info.passThroughHacks.passthroughType != ATA_PASSTHROUGH_SAT && inputs->device->drive_info.passThroughHacks.passthroughType != ATA_PASSTHROUGH_UNKNOWN)
        {
            printf("%" PRIu8 "\tUse a standardized passthrough CDB. If a standard does not exist, take it to\n", recommendationCounter);
            printf("\tone of the spec committees and get one created! If they won't do it, at least\n");
            printf("\tmake it an open document that anyone else can find and use!\n");
            ++recommendationCounter;
        }
        if (!inputs->testPotentiallyDeviceHangingCommands)
        {
            printf("%" PRIu8 "\tRerun this test to test for commands that are known to hang some devices to ensure\n", recommendationCounter);
            printf("\tall hacks are known for this device to perform properly...if the firmware can be\n");
            printf("\tfixed to prevent these hangs, that is the best solution!\n");
            ++recommendationCounter;
        }
#if defined (__linux__)
        if (inputs->device->drive_info.passThroughHacks.passthroughType == ATA_PASSTHROUGH_UNKNOWN)
        {
            printf("%" PRIu8 "\tThis device doesn't support passthrough OR this device is being blacklisted\n", recommendationCounter);
            printf("\tby the Linux kernel from issuing SAT ATA-Passthrough commands.\n");
            printf("\tPlease test in another OS and see if this device still reports the same way.\n");
            printf("\tIt may fail even if SAT ATA-Passthrough commands are allowed, but the device isn't\n");
            printf("\thandling them correctly in UAS mode.\n");
            ++recommendationCounter;
        }
#endif
        if (scsiInformation.vpdData.gotBlockLimitsVPDPage)
        {
            //TODO:Check if we hit the maximum in our test: MAX_SCSI_SECTORS_TO_TEST
            if (inputs->device->drive_info.passThroughHacks.scsiHacks.maxTransferLength < (scsiInformation.vpdData.blockLimitsData.maximumXferLen * inputs->device->drive_info.deviceBlockSize))
            {
                printf("%" PRIu8 "\tThe maximum transfer length is less than was reported by the block limits VPD page!\n", recommendationCounter);
                printf("\tThis page should report the maximum transfer length supported by the SCSI device or SCSI translator\n");
                printf("\tThis should be a true maximum. If this seems low, retest on another OS to ensure it is not a\n");
                printf("\tlimitation on a specific OS. For example, it is fairly common to be limited to 64k in Windows.\n");
                ++recommendationCounter;
            }
        }
        if (inputs->device->drive_info.passThroughHacks.passthroughType < ATA_PASSTHROUGH_UNKNOWN && (inputs->device->drive_info.drive_type != NVME_DRIVE || (inputs->suspectedDriveTypeProvidedByUser && inputs->suspectedDriveType != NVME_DRIVE)))
        {
            //ATA pass-through specific recommendations, if any.
            if (inputs->device->drive_info.ata_Options.dmaMode == ATA_DMA_MODE_NO_DMA)
            {
                if (inputs->device->drive_info.ata_Options.dmaSupported || inputs->device->drive_info.ata_Options.downloadMicrocodeDMASupported || inputs->device->drive_info.ata_Options.readBufferDMASupported || inputs->device->drive_info.ata_Options.readLogWriteLogDMASupported || inputs->device->drive_info.ata_Options.writeBufferDMASupported)
                {
                    printf("%" PRIu8 "\tThis device has no way to passthrough DMA mode commands. This should be fixed as DMA mode\n", recommendationCounter);
                    printf("\tprovides better performance, and on some devices is necessary to get some data due to PIO issues.\n");
                    printf("\tDMA passthrough should be available on the device.\n");
                    ++recommendationCounter;
                }
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.multiSectorPIOWithMultipleMode)
            {
                printf("%" PRIu8 "\tThis device has a problem issuing PIO commands. The workaround is to set the multiple mode,\n", recommendationCounter);
                printf("\tbut this issue should be resolved by the driver/translator. This is likely a problem where it is assumed\n");
                printf("\tthat all PIO transfers will be single sectors, which is an incorrect assumption, and the device does not\n");
                printf("\tproperly handle the interrupts/PIO setup FISs between each sector of data being transferred.\n");
                printf("\tMulti-sector PIO transfers need to be supported for more compatibility with tools that read logs or update firmware.\n");
                printf("\tMulti-sector PIO transfers should be supported without this workaround, which may still be restrictive\n");
                printf("\tfor some cases. Multiple mode has been removed from newer ATA specifications, so this needs supported without\n");
                printf("\tthis workaround.\n");
                ++recommendationCounter;
            }
            else if (inputs->device->drive_info.passThroughHacks.ataPTHacks.singleSectorPIOOnly)
            {
                printf("%" PRIu8 "\tThis device can only issue single sector PIO commands. This is an issue as many logs are read\n", recommendationCounter);
                printf("\tusing SMART read log or Read Log Ext for multiple 512B pages at a time. These will not be accessible on this\n");
                printf("\tdevice. It is also an issue as device firmware may not be upgradeable since that is also a multiple sector transfer\n");
                printf("\tthat is nearly always PIO mode as the DMA mode command in not often supported.\n");
                printf("\tMulti-sector PIO transfers need to be supported for more compatibility with tools that read logs or update firmware.\n");
                ++recommendationCounter;
            }
            if (inputs->device->drive_info.passThroughHacks.ataPTHacks.ata28BitOnly && inputs->device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported)
            {
                printf("%" PRIu8 "\tThis device can only issue 28bit commands, but supports 48bit addressing.\n", recommendationCounter);
                printf("\tThis means loss of capability through pass-through and needs to be resolved to allow\n");
                printf("\tsending read, write, or read log ext commands to the device for more compatibility.\n");
                ++recommendationCounter;
            }
        }
    }
    else
    {
        ret = BAD_PARAMETER;
    }
    return ret;
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
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, SAT_INFO_LONG_OPT_STRING);
    printf("\t%s -d %s --%s\n", util_name, deviceHandleExample, LOWLEVEL_INFO_LONG_OPT_STRING);
    printf("\tTypical use of this test tool usually requires running it twice:\n");
    printf("\t1. %s -d %s --%s --%s ata --%s sat\n", util_name, deviceHandleExample, RUN_PASSTHROUGH_TEST_LONG_OPT_STRING, PT_DRIVE_HINT_LONG_OPT_STRING, PT_PTTYPE_HINT_LONG_OPT_STRING);
    printf("\tEither reboot or power cycle the adapter between runs\n");
    printf("\t2. %s -d %s --%s --%s ata --%s sat --%s all\n", util_name, deviceHandleExample, RUN_PASSTHROUGH_TEST_LONG_OPT_STRING, PT_DRIVE_HINT_LONG_OPT_STRING, PT_PTTYPE_HINT_LONG_OPT_STRING, ENABLE_HANG_COMMANDS_TEST_LONG_OPT_STRING);
    printf("\tLogs from each run should be collected to share with the openSeaChest developers.\n");
    //TODO: another example for USB to NVMe adapters. There are a few other changes necessary to make them able to be specified directly.

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
    print_Low_Level_Info_Help(shortUsage);
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Agressive_Scan_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    //multiple interfaces
    print_Fast_Discovery_Help(shortUsage);
    print_Drive_Type_Hint_Help(shortUsage);
    print_Passthrough_Type_Hint_Help(shortUsage);
    print_Disable_PT_Testing_Help(shortUsage);
    print_Enable_Legacy_ATA_PT_Testing_Help(shortUsage);
    print_Enable_Hang_Commands_Test_Help(shortUsage);
    print_Force_Retest_Help(shortUsage);
    print_Run_Passthrough_Test_Help(shortUsage);

    //SATA Only Options
    //printf("\n\tSATA Only:\n\t=========\n");

    //SAS Only Options
    //printf("\n\tSAS Only:\n\t=========\n");

    //data destructive commands - alphabetized
    //printf("\nData Destructive Commands\n");
    //printf("=========================\n");
    //utility data destructive tests/operations go here
}

