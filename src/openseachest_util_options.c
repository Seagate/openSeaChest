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
// \file openseachest_util_options.c
// \brief This file defines the functions and macros to make building a utility easier.

#include "openseachest_util_options.h"

#if defined (__linux__)
#if defined (VMK_CROSS_COMP)
const char *deviceHandleExample = "vmhba1";
const char *deviceHandleName = "<deviceHandle>";
const char *commandWindowType = "terminal";
#else
const char *deviceHandleExample = "/dev/sg?";
const char *deviceHandleName = "<sg_device>";
const char *commandWindowType = "terminal";
#endif
#elif defined (__FreeBSD__)
const char *deviceHandleExample = "/dev/da?";
const char *deviceHandleName = "<da_device>";
const char *commandWindowType = "shell";
#elif defined (_WIN32)
const char *deviceHandleExample = "PD?";
const char *deviceHandleName = "<physical_device>";
const char *commandWindowType = "command";
#elif defined (__sun)
const char *deviceHandleExample = "/dev/rdsk/c?t?d?";
const char *deviceHandleName = "<rdsk_device>";
const char *commandWindowType = "shell";
#else
#error "OS Not Defined or known"
#endif

#if defined (ENABLE_CSMI)
const char *csmiDeviceHandleName = "<csmi_device>";
#if defined (_WIN32)
const char *csmiDeviceHandleExample = "SCSI\?:\?";
#else
const char *csmiDeviceHandleExample = "<error\?\?\?>";
#endif
#endif

void print_Bug_Report_Email(bool shortHelp)
{
    //we only want to show this for long help information. - TJE
    if (!shortHelp)
    {
        printf("\nPlease report bugs/suggestions to seaboard@seagate.com.\n");
        printf("Include the output of --%s information in the email.\n\n", VERSION_LONG_OPT_STRING);
    }
}

void print_Elevated_Privileges_Text()
{
    printf("You must run with elevated privileges to communicate with devices in the system.");
#if defined (_WIN32)
    printf("(admin)");
#elif defined (__unix__) || defined(__APPLE__)
    printf("(root, sudo)");
#else
    printf("(admin, root, sudo, etc)");
#endif
    printf("\nExamples of elevated priveleges: \n");
#if defined (_WIN32)
    printf("In Windows, open the Command Prompt using \"Run as administrator\".\n");
#elif defined (__unix__) || defined(__APPLE__)
    //TODO: handle the various linux/unix/unix-like OSs with more ifdefs here
#if defined (__linux__)
#if defined (VMK_CROSS_COMP)
    printf("In ESXi, put sudo before the SeaChest command. This may require inputting your login password.\n");
    printf("In ESXi, log in to a root terminal (su), then execute the SeaChest command. This requires the root password.\n");
#else
    printf("In Linux, put sudo before the SeaChest command. This may require inputting your login password.\n");
    printf("In Linux, log in to a root terminal (su), then execute the SeaChest command. This requires the root password.\n");
#endif
#elif defined (__FreeBSD__)
    printf("In FreeBSD, put sudo before the SeaChest command. This may require inputting your login password.\n");
    printf("In FreeBSD, log in to a root terminal (su), then execute the SeaChest command. This requires the root password.\n");
#elif defined (__sun)
    printf("In Solaris, put sudo before the SeaChest command. This may require inputting your login password.\n");
    printf("In Solaris, log in to a root terminal (su), then execute the SeaChest command. This requires the root password.\n");
#else //generic unix/unix-like case //TODO: Add more OS specific ifdefs to customize messages above
    printf("In Linux/Unix, put sudo before the SeaChest command. This may require inputting your login password.\n");
    printf("In Linux/Unix, log in to a root terminal (su), then execute the SeaChest command. This requires the root password.\n");
#endif
#else //generic, who knows what OS this is case
    printf("In Windows, open the Command Prompt using \"Run as administrator\".\n");
    printf("In Linux/Unix, put sudo before the SeaChest command. This may require inputting your login password.\n");
    printf("In Linux/Unix, log in to a root terminal (su), then execute the SeaChest command. This requires the root password.\n");
#endif
    return;
}

char* get_current_year(char *temp_year)
{
    size_t len = strlen(__DATE__);
    temp_year[4] = '\0';
    temp_year[3] = __DATE__[len - 1];
    temp_year[2] = __DATE__[len - 2];
    temp_year[1] = __DATE__[len - 3];
    temp_year[0] = __DATE__[len - 4];
    return temp_year;
}

void openseachest_utility_Info(const char *utilityName, const char *buildVersion, char *seaCPublicVersion)
{
    eArchitecture architecture = get_Compiled_Architecture();
    time_t g_curTime = time(NULL);
    char *year = calloc(CURRENT_YEAR_LENGTH, sizeof(char));
    //char g_timeString[64] = { 0 };
    printf("==========================================================================================\n");
    printf(" %s - openSeaChest drive utilities", utilityName);
#if !defined (DISABLE_NVME_PASSTHROUGH)
    printf(" - NVMe Enabled");
#endif
    printf("\n Copyright (c) 2014-%s Seagate Technology LLC and/or its Affiliates, All Rights Reserved\n", get_current_year(year));
    printf(" %s Version: %s-%s ", utilityName, buildVersion, seaCPublicVersion);
    print_Architecture(architecture);
    printf("\n");
    printf(" Build Date: %s\n", __DATE__);
    printf(" Today: %s", ctime(&g_curTime));
    printf("==========================================================================================\n");
    free(year);
    year = NULL;
}

void utility_Full_Version_Info(const char *utilityName, const char *buildVersion, int seaCPublicMajorVersion, int seaCPublicMinorVersion, int seaCPublicPatchVersion, const char * openseaCommonVersion, const char * openseaOperationVersion)
{
    char osName[OS_NAME_SIZE] = { 0 };
    OSVersionNumber osversionnumber;
    eCompiler compilerUsed = OPENSEA_COMPILER_UNKNOWN;
    compilerVersion compilerVersionInfo;
    memset(&osversionnumber, 0, sizeof(OSVersionNumber));
    memset(&compilerVersionInfo, 0, sizeof(compilerVersion));
    get_Compiler_Info(&compilerUsed, &compilerVersionInfo);
    get_Operating_System_Version_And_Name(&osversionnumber, &osName[0]);

    printf("Version Info for %s:\n", utilityName);
    printf("\tUtility Version: %s\n", buildVersion);
    printf("\topensea-common Version: %s\n", openseaCommonVersion);
    printf("\topensea-transport Version: %" PRId32".%" PRId32".%" PRId32"\n", seaCPublicMajorVersion, seaCPublicMinorVersion, seaCPublicPatchVersion);
    printf("\topensea-operations Version: %s\n", openseaOperationVersion);
    printf("\tBuild Date: %s\n", __DATE__);
    printf("\tCompiled Architecture: ");
    print_Architecture(get_Compiled_Architecture());
    printf("\n\tDetected Endianness: ");
    print_Endianness(get_Compiled_Endianness(), false);
    printf("\n\tCompiler Used: ");
    print_Compiler(compilerUsed);
    printf("\n\tCompiler Version: ");
    print_Compiler_Version_Info(&compilerVersionInfo);
    printf("\n\tOperating System Type: ");
    print_OS_Type(osversionnumber.osVersioningIdentifier);
    printf("\n\tOperating System Version: ");
    print_OS_Version(&osversionnumber);
    printf("\n\tOperating System Name: %s\n", osName);
}

void print_Final_newline(void)
{
    printf("\n");
}

void print_SeaChest_Util_Exit_Codes(int numberOfToolSpecificExitCodes, ptrToolSpecificxitCode toolSpecificExitCodeList, const char * toolName)
{
    printf("\tGeneric/Common exit codes\n");
    printf("\t%d = No Error Found\n", UTIL_EXIT_NO_ERROR);
    printf("\t%d = Error in command line options\n", UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    printf("\t%d = Invalid Device Handle or Missing Device Handle\n", UTIL_EXIT_INVALID_DEVICE_HANDLE);
    printf("\t%d = Operation Failure\n", UTIL_EXIT_OPERATION_FAILURE);
    printf("\t%d = Operation not supported\n", UTIL_EXIT_OPERATION_NOT_SUPPORTED);
    printf("\t%d = Operation Aborted\n", UTIL_EXIT_OPERATION_ABORTED);
    printf("\t%d = File Path Not Found\n", UTIL_EXIT_PATH_NOT_FOUND);
    printf("\t%d = Cannot Open File\n", UTIL_EXIT_CANNOT_OPEN_FILE);
    printf("\t%d = File Already Exists\n", UTIL_EXIT_FILE_ALREADY_EXISTS);
    printf("\t%d = Need Elevated Privileges\n", UTIL_EXIT_NEED_ELEVATED_PRIVILEGES);
    //TODO: more generic exit code help above this comment. Tool specific exit codes in if statement below
    if (numberOfToolSpecificExitCodes > 0 && toolSpecificExitCodeList)
    {
        printf("\t---%s specific exit codes---\n", toolName);
        //We have some number of tool specific exit codes to print out in addition to the generic exit codes we have.
        for (int exitCodeListIter = 0; exitCodeListIter < numberOfToolSpecificExitCodes; ++exitCodeListIter)
        {
            printf("\t%d = %s\n", toolSpecificExitCodeList[exitCodeListIter].exitCode, toolSpecificExitCodeList[exitCodeListIter].exitCodeString);
        }
    }
    printf("\tAnything else = unknown error\n\n");
}

void print_Scan_Help(bool shortHelp, const char *deviceHandleExample)
{
    printf("\t-%c, --%s\n", SCAN_SHORT_OPT, SCAN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tScan the system and list all storage devices with logical\n");
        printf("\t\t%s assignments. Shows model, serial and firmware\n", deviceHandleExample);
        printf("\t\tnumbers.  If your device is not listed on a scan  immediately\n");
        printf("\t\tafter booting, then wait 10 seconds and run it again.\n\n");
    }
}

void print_Agressive_Scan_Help(bool shortHelp)
{
    printf("\t-%c, --%s\n", AGRESSIVE_SCAN_SHORT_OPT, AGRESSIVE_SCAN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is the same as --%s or -%c,\n", SCAN_LONG_OPT_STRING, SCAN_SHORT_OPT);
        printf("\t\thowever it will also perform a low level rescan to pick up\n");
        printf("\t\tother devices. This low level rescan may wake devices from low\n");
        printf("\t\tpower states and may cause the OS to re-enumerate them.\n");
        printf("\t\tUse this option when a device is plugged in and not discovered in\n");
        printf("\t\ta normal scan.\n");
        printf("\t\tNOTE: A low-level rescan may not be available on all interfaces or\n");
        printf("\t\tall OSs. The low-level rescan is not guaranteed to find additional\n");
        printf("\t\tdevices in the system when the device is unable to come to a ready state.\n\n");
    }
}

void print_Scan_Flags_Help(bool shortHelp)
{
    printf("\t-%c, --%s [option list]\n", SCAN_FLAGS_SHORT_OPT, SCAN_FLAGS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to control the output from scan with the\n");
        printf("\t\toptions listed below. Multiple options can be combined.\n");
        printf("\t\t\tata - show only ATA (SATA) devices\n");
        printf("\t\t\tusb - show only USB devices\n");
        printf("\t\t\tscsi - show only SCSI (SAS) devices\n");
#if !defined (DISABLE_NVME_PASSTHROUGH)
        printf("\t\t\tnvme - show only NVMe devices\n");
#endif
        //printf("\t\t\traid - show RAID devices\n");//commented out until we officially add raid support. Currently raids show up as SCSI devices
        printf("\t\t\tinterfaceATA - show devices on an ATA interface\n");
        printf("\t\t\tinterfaceUSB - show devices on a USB interface\n");
        printf("\t\t\tinterfaceSCSI - show devices on a SCSI or SAS interface\n");
#if !defined (DISABLE_NVME_PASSTHROUGH)
        printf("\t\t\tinterfaceNVME = show devices on an NVMe interface\n");
#endif
#if defined(__linux__)
        printf("\t\t\tsd - show sd device handles\n");
        printf("\t\t\tsgtosd - show the sd and sg device handle mapping\n");
#endif
#if defined (ENABLE_CSMI)
        printf("\t\t\tignoreCSMI - do not scan for any CSMI devices\n");
        printf("\t\t\tallowDuplicates - allow drives with both CSMI and PD handles\n");
        printf("\t\t\t                  to show up multiple times in the list\n");
#endif
        printf("\n");
    }
}

void print_Device_Help(bool shortHelp, const char *deviceHandleExample)
{
    printf("\t-%c, --%s deviceHandle\n", DEVICE_SHORT_OPT, DEVICE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with most commands to specify the device\n");
        printf("\t\thandle on which to perform an operation. Example: %s\n", deviceHandleExample);
#if defined(_WIN32)
        printf("\t\tA handle can also be specified as \\\\.\\PhysicalDrive?\n");
#endif
#if defined (ENABLE_CSMI)
        printf("\t\tCSMI device handles can be specified as %s\n", csmiDeviceHandleExample);
#endif
        printf("\n");
    }
}

void print_Device_Information_Help(bool shortHelp)
{
    printf("\t-%c, --%s\n", DEVICE_INFO_SHORT_OPT, DEVICE_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tShow information and features for the storage device\n\n");
    }
}

void print_Verbose_Help(bool shortHelp)
{
    printf("\t-%c [0-4], --%s [0 | 1 | 2 | 3 | 4]\n", VERBOSE_SHORT_OPT, VERBOSE_LONG_OPT_STRING);

    if (!shortHelp)
    {
        printf("\t\tShow verbose information. Verbosity levels are:\n");
        printf("\t\t0 - quiet\n");
        printf("\t\t1 - default\n");
        printf("\t\t2 - command descriptions\n");
        printf("\t\t3 - command descriptions and values\n");
        printf("\t\t4 - command descriptions, values, and data buffers\n");
        printf("\t\tExample: -v 3 or --verbose 3\n\n");
    }
}

void print_Quiet_Help(bool shortHelp, const char *utilName)
{
    printf("\t-%c, --%s\n", QUIET_SHORT_OPT, QUIET_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tRun %s in quiet mode. This is the same as\n", utilName);
        printf("\t\t-v 0 or --verbose 0\n\n");
    }
}

void print_Version_Help(bool shortHelp, const char *utilName)
{
    printf("\t-%c, --%s\n", VERSION_SHORT_OPT, VERSION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tShow %s version and copyright information & exit\n\n", utilName);
    }
}

void print_Confirm_Help(bool shortHelp)
{
    printf("\t--%s %s\n", CONFIRM_LONG_OPT_STRING, DATA_ERASE_ACCEPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option should be used only with Data Destructive Commands\n\n");
    }
}

void print_License_Help(bool shortHelp)
{
    printf("\t--%s\n", LICENSE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tDisplay the Seagate End User License Agreement (EULA).\n\n");
    }
}

void print_Echo_Command_Line_Help(bool shortHelp)
{
    printf("\t--%s\n", ECHO_COMMAND_LINE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tEcho the command line entered into the utility on the screen.\n\n");
    }
}

void print_Help_Help(bool shortHelp)
{
    printf("\t-%c, --%s\n", HELP_SHORT_OPT, HELP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tShow utility options and example usage (this output you see now)\n");
        printf("\t\tPlease report bugs/suggestions to seaboard@seagate.com.\n");
        printf("\t\tInclude the output of --%s information in the email.\n\n", VERSION_LONG_OPT_STRING);
    }
}

void print_OutputPath_Help(bool shortHelp)
{
    printf("\t--%s [folder]\n", PATH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tTo set a path to the directory/folder where all logs should be created.\n");
        printf("\t\tThe directory/folder must already exist with write permissions\n");
        printf("\t\tIf this option is not used, logs are created in the current working folder\n\n");
    }
}

void print_Erase_Range_Help(bool shortHelp)
{
    printf("\t--eraseRange [startLBA] [endLBA] [forceWrites]\n");
    if (!shortHelp)
    {
        printf("\t\tEnter a starting and ending lba to erase a range of data on the\n");
        printf("\t\tdrive. If no endLBA is specified, the MaxLBA of the drive will\n");
        printf("\t\tbe used. This test will write 0's to the device for the\n");
        printf("\t\tspecified range for HDDs and on SSDs a TRIM or UNMAP command\n");
        printf("\t\twill be used instead. If you add\"forceWrites\" to the options,\n");
        printf("\t\tthis will force write commands to be used instead of TRIM or\n");
        printf("\t\tUNMAP on SSDs. The forceWrites option does not affect HDDs.\n\n");
    }
}

void print_Erase_Time_Help(bool shortHelp)
{
    printf("\t--eraseTime [startLBA] [time] [units]\n");
    if (!shortHelp)
    {
        printf("\t\tUse this option to erase a drive for a certain amount of time.\n");
        printf("\t\tA startLBA must be specified. A time is a numberic value\n");
        printf("\t\tand if no unit is given, seconds is assumed. Valid units are\n");
        printf("\t\t \"seconds\", \"minutes\", \"hours\", or \"days\".\n");
        printf("\t\tIf the end of the drive is reached before the time is up, the\n");
        printf("\t\toperation will continue at the beginning of the drive until the\n");
        printf("\t\tspecified time is finished.  This means your starting LBA may\n");
        printf("\t\tnot be the lowest LBA erased.  Use --eraseRange instead for\n");
        printf("\t\tmore control of the starting and ending LBAs.  This test always\n");
        printf("\t\tissues write commands to the drive. No TRIM or UNMAP commands\n");
        printf("\t\tare used during this operation.\n\n");
    }
}

void print_Sanitize_Help(bool shortHelp, const char *utilName)
{
    printf("\t--%s [info | blockerase | cryptoerase |\n", SANITIZE_LONG_OPT_STRING);
    printf("\t            overwrite | freezelock | antifreezelock]\n");
    if (!shortHelp)
    {
        printf("\t\tUse the info argument to show supported sanitize operations.\n");
        printf("\t\tOptionally, use blockerase, cryptoerase, or overwrite to start\n");
        printf("\t\ta sanitize operation. Adding the --%s option will cause\n", POLL_LONG_OPT_STRING);
        printf("\t\t%s to poll the drive for progress until the\n", utilName);
        printf("\t\toperation is complete, or has aborted for some reason. All\n");
        printf("\t\tsanitize erase operations are persistent across a power cycle\n");
        printf("\t\tand cannot be stopped\n");
        printf("\t\tExample: --%s blockerase --%s\n\n", SANITIZE_LONG_OPT_STRING, POLL_LONG_OPT_STRING);
#if defined (_WIN32)//TODO: handle Win PE somehow when we support WinPE
        printf("\t\tNote: Windows 8 and higher block sanitize commands. Sanitize\n");
        printf("\t\toperations will show a failure status on these systems.\n\n");
#endif
        //blockerase info
        printf("\t\t* blockerase on some solid state drives is very fast at less\n");
        printf("\t\tthan one (1) second, while others may take more that 30 seconds\n");
        printf("\t\tThis operation performs a physical low level block erase\n");
        printf("\t\toperation on all current, past, and potential user data.\n");
        printf("\t\tThe contents on user data are indeterminate upon completion.\n\n");
        //cryptoerase info
        printf("\t\t* cryptoerase is very fast at less than one (1) second. It\n");
        printf("\t\tchanges the internal encryption keys that are used for user\n");
        printf("\t\tdata causing all previous data to be useless.\n\n");
        //overwrite info
        printf("\t\t* overwrite is a physical overwrite on all current, past, and\n");
        printf("\t\tpotential user data. The ATA and SCSI specifications allow a\n");
        printf("\t\tuser defined pattern and multiple passes. %s will\n", utilName);
        printf("\t\tuse a zero pattern and a single pass for this operation.\n\n");
        //freezelock info
        printf("\t\t* freezelock is a command to block processing of sanitize\n");
        printf("\t\toperations until a power cycle is performed on a device.\n");
        printf("\t\tIt is only available on ATA drives. Once this command has been\n");
        printf("\t\tsent, the freezelock status becomes immediate and cannot be\n");
        printf("\t\tcleared until the drive has been powered off. All sanitize\n");
        printf("\t\tcommands, except a sanitize status will be aborted.\n\n");
        //anti freezlock info
        printf("\t\t* antifreezelock is a command that is designed to block a\n");
        printf("\t\tfreezelock command from locking out the sanitize feature set.\n");
        printf("\t\tIt is only available on ATA drives that support the ACS3, or\n");
        printf("\t\tnewer specification.\n\n");
    }
}

void print_Poll_Help(bool shortHelp)
{
    printf("\t--%s\n", POLL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to cause another operation to poll for progress\n");
        printf("\t\tuntil it has completed.  This argument does not return to the\n");
        printf("\t\tcommand prompt and prints ongoing completion percentages (%%)\n");
        printf("\t\t the final test result.  Full drive procedures will take a\n");
        printf("\t\tvery long time.  Used with --sanitize, or --writeSame (SATA).\n\n");
    }
}

void print_Writesame_Help(bool shortHelp)
{
    printf("\t--%s [starting LBA]\n", WRITE_SAME_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tEnter a starting lba to begin a write same on to erase a range\n");
        printf("\t\tof data on the drive. On SCSI devices, this uses the\n");
        printf("\t\twritesame16 command. On ATA devices, this uses the SCT writesame\n");
        printf("\t\tfeature. Combine this option with the writeSameRange option to\n");
        printf("\t\tselect the range. This operation will write 0's to the device for the\n");
        printf("\t\tspecified range. For SATA drives, adding the --poll option will\n");
        printf("\t\tcause this operation to poll for progress until complete. This\n");
        printf("\t\tis not available on SAS and SCSI drives because SCSI drives do not\n");
        printf("\t\treport the progress on a Write Same operation.\n");
        printf("\t\tOn SATA, if any other commands are sent to the drive while it's\n");
        printf("\t\tperforming a write same, the write same will be aborted.\n\n");
    }
}

void print_Writesame_Range_Help(bool shortHelp)
{
    printf("\t--%s [range]\n", WRITE_SAME_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tSpecify a range to writesame to. Use this option with the\n");
        printf("\t\twriteSame option in order to begin a write same operation.\n\n");
    }
}

void print_Revert_Help(bool shortHelp)
{
    printf("\t--%s\n", TCG_REVERT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis operation performs an Opal SSC spec Revert on the adminSP.\n");
        printf("\t\tThis operation is only available on Seagate TCG Opal drives.\n");
        printf("\t\tThe --%s flag can be provided to perform the revert with\n", TCG_PSID_LONG_OPT_STRING);
        printf("\t\tthe PSID authority incase of a lost password.\n");
        printf("\t\tThe --%s flag can be provided to perform the revert with SID.\n", TCG_SID_LONG_OPT_STRING);
        printf("\t\tIf neither the --%s or the --%s options are provided, then the\n", TCG_PSID_LONG_OPT_STRING, TCG_SID_LONG_OPT_STRING);
        printf("\t\trevert will be sent setting SID as the MSID value. This will only work\n");
        printf("\t\ton a drive not already activated by security software.\n");
        printf("\t\tUpon completion, the drive will be \"like new\" with all\n");
        printf("\t\tuser data being cryptographically erased and all other settings\n");
        printf("\t\tset to factory defaults. If this operation fails, try using --%s\n", TCG_REVERT_SP_LONG_OPT_STRING);
        printf("\t\tinstead.\n\n");
    }
}

void print_RevertSP_Help(bool shortHelp)
{
    printf("\t--%s\t(Seagate Only)\n", TCG_REVERT_SP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis operation performs a revertSP on a Seagate SED drive\n");
        printf("\t\tin the adminSP with the PSID.\n");
        printf("\t\tThe PSID must be provided using the --%s option.\n", TCG_PSID_LONG_OPT_STRING);
        printf("\t\tThis operation is available on all Seagate SED HDD drives and some SSDs.\n");
        printf("\t\tUpon completion, the drive will be \"like new\" with all\n");
        printf("\t\tuser data being cryptographically erased and all other\n");
        printf("\t\tsettings set to factory defaults.\n\n");
    }
}

void print_Progress_Help(bool shortHelp, char* testsTogetProgressFor)
{
    printf("\t--%s [%s]\n", PROGRESS_LONG_OPT_STRING, testsTogetProgressFor);
    if (!shortHelp)
    {
        printf("\t\tGet the progress for a test that was started quietly without\n");
        printf("\t\tthe polling option (default). You must specify a test you wish to\n");
        printf("\t\tget progress from. Ex: \"--%s dst\" or \"--%s sanitize\"\n", PROGRESS_LONG_OPT_STRING, PROGRESS_LONG_OPT_STRING);
        printf("\t\tThe progress counts up from 0%% to 100%%.\n\n");
    }
}

void print_SMART_Check_Help(bool shortHelp)
{
    printf("\t--%s\n", SMART_CHECK_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tPerform a SMART check on a device to see if any internal\n");
        printf("\t\tthresholds have been tripped or if the drive is still operating\n");
        printf("\t\twithin specification.\n\n");
    }
}

void print_Short_DST_Help(bool shortHelp)
{
    printf("\t--%s\n", SHORT_DST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tExecute a short diagnostic drive self test. This test can take\n");
        printf("\t\tup to 2 minutes to complete. Use the --poll argument to make\n");
        printf("\t\tthis operation poll for progress until complete. Use the\n");
        printf("\t\t--progress dst command to check on the completion percentage\n");
        printf("\t\t(%%) and test result.\n");
        printf("\t\tNOTE: Short DST may take longer if there is other disk usage\n");
        printf("\t\twhile the DST is running. If the DST takes longer than 10 minutes\n");
        printf("\t\tit will automatically be aborted while polling for progress.\n");
        printf("\t\tTo override this behavior, use the --%s option.\n\n", IGNORE_OPERATION_TIMEOUT_LONG_OPT_STRING);
    }
}

void print_Conveyance_DST_Help(bool shortHelp)
{
    printf("\t--%s\n", CONVEYANCE_DST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tExecute a conveyance diagnostic drive self test. A conveyance\n");
        printf("\t\ttest can be used to check if a drive has incurred handling damage.\n");
        printf("\t\tThis test can take up to 2 minutes to complete. Use the --poll\n");
        printf("\t\targument to make this operation poll for progress until complete.\n");
        printf("\t\tUse the --progress dst command to check on the completion\n");
        printf("\t\tpercentage (%%) and test result.\n");
        printf("\t\tNOTE: conveyance DST may take longer if there is other disk usage\n");
        printf("\t\twhile the DST is running. If the DST takes longer than 10 minutes\n");
        printf("\t\tit will automatically be aborted while polling for progress.\n");
        printf("\t\tTo override this behavior, use the --%s option.\n\n", IGNORE_OPERATION_TIMEOUT_LONG_OPT_STRING);
    }
}

void print_Long_DST_Help(bool shortHelp, const char *commandWindowType)
{
    printf("\t--%s\n", LONG_DST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tExecute a long diagnostic drive self test. This test takes\n");
        printf("\t\thours to complete.  A 2TB drive may take six (6) hours to\n");
        printf("\t\tcomplete. Use with the --poll argument to let SeaChest check\n");
        printf("\t\tfor progress and print it to the screen until complete.\n");
        printf("\t\tUse the --progress dst command to check on the completion\n");
        printf("\t\tpercentage(%%) and test result.\n");
        printf("\t\tThis test stops on the first error. Use --abortDST\n");
        printf("\t\tto manually stop the test. SAS drives give status in 1%%\n");
        printf("\t\tincrements. SATA drives give status in 10%% increments which\n");
        printf("\t\tmeans more than an hour may elapse between updates on a SATA\n");
        printf("\t\tdrive > 2TB.\n\n");
        printf("\t\tIf the --longDST poll option is running and you want to abort\n");
        printf("\t\tthe test then you will need to open a second %s window\n", commandWindowType);
        printf("\t\tand run the --abortDST command. Otherwise, it is safe to\n");
        printf("\t\trestart the system while long DST is running which also ends the\n");
        printf("\t\ttest.\n");
        printf("\t\tNOTE: Long DST may take longer if there is other disk usage\n");
        printf("\t\twhile the DST is running. If the DST takes longer than 5x the\n");
        printf("\t\tdrive reported time, it will automatically be aborted while\n");
        printf("\t\tpolling for progress.\n");
        printf("\t\tTo override this behavior, use the--%s option.\n\n", IGNORE_OPERATION_TIMEOUT_LONG_OPT_STRING);
    }
}

void print_SMART_Attributes_Help(bool shortHelp)
{
    printf("\t--%s [raw | analyzed]\t(SATA Only)\n", SMART_ATTRIBUTES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThe drive will display its list of supported SMART attributes.\n");
        printf("\t\tSome attributes names are commonly standard and most others are\n");
        printf("\t\tvendor unique. In either case, the attribute thresholds are\n");
        printf("\t\talways vendor unique. Most attributes are informational and not\n");
        printf("\t\tused to determine a warranty return. Use the --smartCheck\n");
        printf("\t\tcommand to determine if one of the warranty attributes has been\n");
        printf("\t\ttripped. Seagate Support does not help to analyze SMART\n");
        printf("\t\tattributes.\n\n");
    }
}

void print_Abort_DST_Help(bool shortHelp)
{
    printf("\t--%s\n", ABORT_DST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tAbort a diagnostic Drive Self Test that is in progress.\n\n");
    }
}

void print_Captive_Foreground_Help(bool shortHelp)
{
    printf("\t--%s or --%s\n", CAPTIVE_LONG_OPT_STRING, FOREGROUND_LONG_OPT_STRGIN);
    if (!shortHelp)
    {
        printf("\t\tUse this option to run a DST operation in captive/foreground\n");
        printf("\t\tmode. This mode may allow a device to test more of itself and\n");
        printf("\t\tmay be slightly faster than offline/background mode since it does\n");
        printf("\t\tnot need to service additional command during the test.\n");
        printf("\t\tWhen using this mode, the utility must wait the entire time for\n");
        printf("\t\tthe DST to complete and progress cannot be indicated during this time.\n\n");
    }
}

void print_Abort_IDD_Help(bool shortHelp)
{
    printf("\t--%s (Seagate Only)\n", ABORT_IDD_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tAbort a Seagate In Drive Diagnostic (IDD) that is in progress.\n\n");
    }
}

void print_IDD_Help(bool shortHelp)
{
    printf("\t--%s [short | long]\t\t(Seagate Only)\n", IDD_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tStart an In Drive Diagnostic (IDD) test on a Seagate\n");
        printf("\t\tdrive. Not all tests are supported by all products. If a\n");
        printf("\t\tselected test is not supported, the utility will return\n");
        printf("\t\ta error code meaning \"not supported\".\n");
        printf("\t\tShort:  Reset and Recalibration test. Be careful running this\n");
        printf("\t\t        test on the boot device.\n");
        printf("\t\tLong:   Reset and Recalibration test and test G list and \n");
        printf("\t\t        P list\n");
        printf("\t\tNote: the --%s option can be added to run the long test in\n", CAPTIVE_LONG_OPT_STRING);
        printf("\t\t      foreground/captive mode. This allows for G-list healing\n");
        printf("\t\t      and some additional checks to be performed. This may not\n");
        printf("\t\t      work on some products.\n\n");
    }
}

void print_Check_Power_Mode_Help(bool shortHelp)
{
    printf("\t--%s\n", CHECK_POWER_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tGet the current power mode of a drive.\n");
        printf("\t\tOn SCSI devices, this will only work if the drive has\n");
        printf("\t\ttransitioned from active state to another state.\n");
        #if defined (_WIN32)
        printf("\t\tThis option may wake a SAS drive under Windows due to\n");
        printf("\t\ta Windows limitation. If used on the command line with\n");
        printf("\t\tthe transition option, the output will show correctly.\n");
        printf("\t\tex of usage: --%s standby_z --%s\n", TRANSITION_POWER_MODE_LONG_OPT_STRING, CHECK_POWER_LONG_OPT_STRING);
        #endif
        printf("\n");
    }
}

void print_EnableDisableEPC_Help(bool shortHelp)
{
    printf("\t--%s [enable | disable]\n", EPC_ENABLED_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tEnables or disables Extended Power Conditions (EPC) support for\n");
        printf("\t\tdevices. To disable EPC use --EPCfeature disable. Note that the\n");
        printf("\t\tEPC Feature Set is not supported on all devices.\n");
        printf("\t\tUse --deviceInfo option to see if EPC is supported.\n\n");
    }
}

void print_Spindown_Help(bool shortHelp)
{
    printf("\t--%s\n", SPIN_DOWN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tRemoves power to the disk drive motor with the Standby Immediate\n");
        printf("\t\tcommand. Use this before moving a hard disk drive. The drive\n");
        printf("\t\twill spin back up if the operating system selects the drive.\n");
        printf("\t\tThis means that an active drive will not stay spun down.\n\n");
    }
}

void print_Standby_Help(bool shortHelp)
{
    printf("\t--%s\n", STANDBY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis command will transition a drive to the standby power state.\n");
        printf("\t\tThis command will flush the cache before the transition to this state.\n");
        printf("\t\tThis command is for non-EPC enabled drives. If the drive has the EPC\n");
        printf("\t\tfeature enabled, it is recommended that the --%s option\n", TRANSITION_POWER_MODE_LONG_OPT_STRING);
        printf("\t\tbe used instead. This option is compatible with EPC enabled drives as well,");
        printf("\t\tbut offers less control over the transition.\n");
        printf("\t\tOn an HDD, this will cause the spindle motor to stop.\n");
        printf("\t\tIf the operating system selects this drive, it will transition it back to\n");
        printf("\t\tan active state.\n\n");
    }
}

void print_Idle_Help(bool shortHelp)
{
    printf("\t--%s\n", IDLE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis command will transition a drive to the idle power state.\n");
        printf("\t\tThis command may flush the cache before the transition to this state.\n");
        printf("\t\tThis command is for non-EPC enabled drives. If the drive has the EPC\n");
        printf("\t\tfeature enabled, it is recommended that the --%s option\n", TRANSITION_POWER_MODE_LONG_OPT_STRING);
        printf("\t\tbe used instead. This option is compatible with EPC enabled drives as well,");
        printf("\t\tbut offers less control over the transition.\n");
        printf("\t\tIf the operating system selects this drive, it will transition it back to\n");
        printf("\t\tan active state.\n\n");
    }
}

void print_Idle_Unload_Help(bool shortHelp)
{
    printf("\t--%s\n", IDLE_UNLOAD_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis command will transition a drive to the idle, heads unloaded, power state.\n");
        printf("\t\tThis command may flush the cache before the transition to this state.\n");
        printf("\t\tThis command is for non-EPC enabled drives. If the drive has the EPC\n");
        printf("\t\tfeature enabled, it is recommended that the --%s option\n", TRANSITION_POWER_MODE_LONG_OPT_STRING);
        printf("\t\tbe used instead. This option is compatible with EPC enabled drives as well,");
        printf("\t\tbut offers less control over the transition.\n");
        printf("\t\tIf the operating system selects this drive, it will transition it back to\n");
        printf("\t\tan active state.\n\n");
    }
}

void print_Sleep_Help(bool shortHelp)
{
    printf("\t--%s\n", SLEEP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis command will transition a drive to the sleep power state.\n");
        printf("\t\tA reset must be sent to wake a drive from sleep state.\n");
        printf("\t\tThe OS may not be able to wake a drive from this state once it has\n");
        printf("\t\tbeen entered. Use this option with caution!\n\n");
    }
}

void print_Active_Help(bool shortHelp)
{
    printf("\t--%s\n", ACTIVE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis command will transition a drive to the active power state.\n");
        printf("\t\tThis uses a ATA read-verify command to a random LBA, or a SCSI\n");
        printf("\t\tstart-stop unit command.\n\n");
    }
}

void print_Enable_Power_Mode_Help(bool shortHelp)
{
    printf("\t--%s\n", ENABLE_POWER_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with the change power mode option to enable\n");
        printf("\t\ta specific power mode.\n\n");
    }
}

void print_Disable_Power_Mode_Help(bool shortHelp)
{
    printf("\t--%s\n", DISABLE_POWER_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with the change power mode option to disable\n");
        printf("\t\ta specific power mode.\n\n");
    }
}

void print_Default_Power_Mode_Help(bool shortHelp)
{
    printf("\t--%s\n", DEFAULT_POWER_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with the change power mode option restore the \n");
        printf("\t\tdefault settings for a specific power mode.\n\n");
    }
}

void print_Power_Mode_Help(bool shortHelp)
{
    printf("\t--%s [powermode]\n", POWER_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a power mode. This should be used\n");
        printf("\t\twith the --transitionPower or --changePower options.\n");
        printf("\t\tValid power modes (SAS and SATA):\n");
        printf("\t\t\tidle_a\n");
        printf("\t\t\tidle_b\n");
        printf("\t\t\tidle_c\n");
        printf("\t\t\tstandby_y (SAS only)\n");
        printf("\t\t\tstandby_z\n");
        printf("\t\t\tactive (SAS only)\n");
        printf("\t\t\tall (enable or disable only)\n");
        printf("\t\tValid power modes (NVMe): 0 - 30\n");
        printf("\n");
    }
}

void print_Timer_Mode_Help(bool shortHelp)
{
    printf("\t--%s [timer value in milliseconds]\n", POWER_MODE_TIMER_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a timer value to use with the\n");
        printf("\t\t--changePower mode option.\n\n");
    }
}

void print_Change_Power_Help(bool shortHelp)
{
    printf("\t--%s\n", CHANGE_POWER_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable certain --powerMode and\n");
        printf("\t\ttheir timers. You can also use this option to restore default\n");
        printf("\t\tvalues to the drive.\n\n");
    }
}

//Mainly used for NVMe devices.
void print_extSmatLog_Help(bool shortHelp)
{
    printf("\t--%s\n", EXT_SMART_LOG_LONG_OPT_STRING1);
    if (!shortHelp)
    {
        printf("\t\tUse this option to Extract the Extended Smart Log Attributes.\n\n");
    }
}

void print_pcierr_Help (bool shortHelp)
{
    printf("\t--%s\n", CLEAR_PCIE_CORRECTABLE_ERRORS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to clear correctable errors.\n\n");
    }
}

void print_Transition_Power_State_Help(bool shortHelp)
{
    printf("\t--%s [new power state]\n", TRANSITION_POWER_STATE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to transition to a specific power state.\n");
        printf("\t\tHINT:\n\t\t  Use --%s to show number of supported states\n\n",DEVICE_INFO_LONG_OPT_STRING);
    }
}

void print_Transition_Power_Help(bool shortHelp)
{
    printf("\t--%s\n", TRANSITION_POWER_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to transition the drive to a specific\n");
        printf("\t\tpower state.\n\n");
    }
}

void print_Buffer_Test_Help(bool shortHelp)
{
    printf("\t--%s\n", BUFFER_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will perform a test using the device's echo buffer.\n");
        printf("\t\tThe write buffer and read buffer commands are used to send &\n");
        printf("\t\treceive different data patterns. The patterns are compared\n");
        printf("\t\tand interface CRC errors are also checked (when available).\n");
        printf("\t\tTest patterns performed are all 0's, all F's, all 5's, all A's,\n");
        printf("\t\twalking 1's, walking 0's, and random data patterns.\n");
        printf("\t\tAt completion, a count of the number of errors will be displayed.\n\n");
    }
}

void print_Short_Generic_Help(bool shortHelp)
{
    printf("\t--%s\n", SHORT_GENERIC_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will run a short generic read test on a\n");
        printf("\t\tspecified device. A short generic read test has 3\n");
        printf("\t\tcomponents. A read at the Outer Diameter (OD) of the\n");
        printf("\t\tdrive for 1%% of the LBAs, then a read at the Inner\n");
        printf("\t\tDiameter of the drive for 1%% of the LBAs, and lastly\n");
        printf("\t\ta random read of 5000 LBAs. This test will stop on\n");
        printf("\t\tthe first read error that occurs.\n");
        printf("\t\tInner and outer diameter tests refer to the physical\n");
        printf("\t\tbeginning and ending sections of a hard disk drive with\n");
        printf("\t\trotating magnetic media.In the case of SSD devices,\n");
        printf("\t\tthese tests refer to the logical beginning and ending\n");
        printf("\t\tsections of the solid state drive.\n\n");
    }
}

void print_two_Minute_Test_Help(bool shortHelp)
{
    printf("\t--%s\n", TWO_MINUTE_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will run a 2 minute generic read test on\n");
        printf("a specified device. There are 3 components to this test.\n");
        printf("A read at the Outer Diameter (OD) of the drive for 45\n");
        printf("\t\tseconds, then a read at the Inner Diameter of the\n");
        printf("\t\tdrive for 45 seconds, and lastly a random read test\n");
        printf("\t\tfor 30 seconds. This test will stop on the first\n");
        printf("\t\tread error that occurs.\n");
        printf("\t\tInner and outer diameter tests refer to the physical\n");
        printf("\t\tbeginning and ending sections of a hard disk drive with\n");
        printf("\t\trotating magnetic media.In the case of SSD devices,\n");
        printf("\t\tthese tests refer to the logical beginning and ending\n");
        printf("\t\tsections of the solid state drive.\n\n");
    }
}


void print_Long_Generic_Help(bool shortHelp)
{
    printf("\t--%s\n", LONG_GENERIC_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will run a long generic read test on a\n");
        printf("\t\tspecified device. A long generic read test reads every\n");
        printf("\t\tLBA on the device and gives a report of error LBAs at\n");
        printf("\t\tthe end of the test, or when the error limit has been\n");
        printf("\t\treached. Using the --stopOnError option will make this\n");
        printf("\t\ttest stop on the first read error that occurs.\n");
        printf("\t\tThe default error limit is 50 x number of logical\n");
        printf("\t\tsectors per physical sector. Example error limits\n");
        printf("\t\tare as follows:\n");
        printf("\t\t\t512L/512P: error limit = 50\n");
        printf("\t\t\t4096L/4096P: error limit = 50\n");
        printf("\t\t\t512L/4096P: error limit = 400 (50 * 8)\n");
        printf("\n");
    }
}

void print_User_Generic_Start_Help(bool shortHelp)
{
    printf("\t--%s [LBA]\n", USER_GENERIC_LONG_OPT_START_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the starting LBA number for a\n");
        printf("\t\tgeneric read test. The --userGenericRange option must be used\n");
        printf("\t\twith this one in order to start the test.  Use the stop on\n");
        printf("\t\terror, repair flags, and/or error limit flags to further\n");
        printf("\t\tcustomize this test.\n\n");
    }
}

void print_User_Generic_Range_Help(bool shortHelp)
{
    printf("\t--%s [range]\n", USER_GENERIC_LONG_OPT_RANGE_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the range for a \n");
        printf("\t\tgeneric read test. See the --userGenericStart\n");
        printf("\t\thelp for additional information about using\n");
        printf("\t\tthe User Generic Read tests.\n\n");
    }
}

void print_Error_Limit_Help(bool shortHelp)
{
    printf("\t--%s [limit in number of LBAs]\n", ERROR_LIMIT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a different error\n");
        printf("\t\tlimit for a user generic or long generic read\n");
        printf("\t\ttest or DST and Clean. This must be a number of\n");
        printf("\t\t logical LBAs to have errors. If a drive has multiple\n");
        printf("\t\t logical sectors per physical sector, this number will\n");
        printf("\t\tbe adjusted for you to reflect the drive\n");
        printf("\t\tarchitecture.\n\n");
    }
}

void print_Stop_On_Error_Help(bool shortHelp)
{
    printf("\t--%s\n", STOP_ON_ERROR_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to make a generic read test\n");
        printf("\t\tstop on the first error found.\n\n");
    }
}

void print_Repair_At_End_Help(bool shortHelp)
{
    printf("\t--%s\n", REPAIR_AT_END_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to repair any bad sectors\n");
        printf("\t\tfound during a long or user generic read\n");
        printf("\t\ttest at the end of the test.\n\n");
    }
}

void print_Repair_On_Fly_Help(bool shortHelp)
{
    printf("\t--%s\n", REPAIR_ON_FLY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to repair any bad sectors\n");
        printf("\t\tfound during a long or user generic read\n");
        printf("\t\ttest as they are found.\n\n");
    }
}

void print_Time_Hours_Help(bool shortHelp)
{
    printf("\t--%s [hours]\n", HOURS_TIME_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a time in hours\n");
        printf("\t\tfor a timed operation to run.\n\n");
    }
}

void print_Time_Minutes_Help(bool shortHelp)
{
    printf("\t--%s [minutes]\n", MINUTES_TIME_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a time in minutes\n");
        printf("\t\tfor a timed operation to run.\n\n");
    }
}

void print_Time_Seconds_Help(bool shortHelp)
{
    printf("\t--%s [seconds]\n", SECONDS_TIME_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a time in seconds\n");
        printf("\t\tfor a timed operation to run.\n\n");
    }
}

void print_Random_Read_Test_Help(bool shortHelp)
{
    printf("\t--%s\n", RANDOM_READ_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to start a random test.\n");
        printf("\t\tThis is a timed operation. Use the time options\n");
        printf("\t\tto control how long to run this test for. The\n");
        printf("\t\tdefault time for this test is 1 minute.\n\n");
    }
}

void print_Butterfly_Read_Test_Help(bool shortHelp)
{
    printf("\t--%s\n", BUTTERFLY_READ_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to start a butterfly test.\n");
        printf("\t\tA butterfly test is a test that moves\n");
        printf("\t\tback and forth between the OD and ID of the drive\n");
        printf("\t\tover and over again until the time has expired\n");
        printf("\t\tThis is a timed operation. Use the time options\n");
        printf("\t\tto control how long to run this test for. The\n");
        printf("\t\tdefault time for this test is 1 minute.\n\n");
    }
}

void print_Overwrite_Help(bool shortHelp)
{
    printf("\t--%s [starting LBA]\n", OVERWRITE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to start an overwrite erase at\n");
        printf("\t\tthe specified starting LBA. Combine this option\n");
        printf("\t\twith overwriteRange or time options (hours, minutes\n");
        printf("\t\tseconds) to erase a portion of the drive.\n\n");
    }
}

void print_Overwrite_Range_Help(bool shortHelp)
{
    printf("\t--%s [range]\n", OVERWRITE_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse with option with the overwrite option to\n");
        printf("\t\terase a range of LBAs on the selected drive.\n\n");
    }
}

void print_Trim_Unmap_Help(bool shortHelp)
{
    printf("\t--%s or --%s [starting LBA]\n", TRIM_LONG_OPT_STRING, UNMAP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse one of these options to start a trim or\n");
        printf("\t\tunmap operation on a drive at the provided LBA.\n");
        printf("\t\tA range must also be provided with the range option.\n\n");
    }
}

void print_Trim_Unmap_Range_Help(bool shortHelp)
{
    printf("\t--%s or --%s [range]\n", TRIM_RANGE_LONG_OPT_STRING, UNMAP_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse one of these options to specify a range to trim\n");
        printf("\t\tor unmap on a drive. A starting point must be specified\n");
        printf("\t\twith the --trim/--unmap option.\n\n");
    }
}

void print_Show_Power_Consumption_Help(bool shortHelp)
{
    printf("\t--%s\t(SAS Only)\n", SHOW_POWER_CONSUMPTION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will show the power consumption\n");
        printf("\t\trates supported by the device and the current power\n");
        printf("\t\tconsumption rate of the device. Use a supported watt value\n");
        printf("\t\twith the --%s option to set the\n", SET_POWER_CONSUMPTION_LONG_OPT_STRING);
        printf("\t\tpower consumption to that value.\n\n");
    }
}

void print_Set_Power_Consumption_Help(bool shortHelp)
{
    printf("\t--%s [default | highest | intermediate | lowest | watt value]\t(SAS Only) \n", SET_POWER_CONSUMPTION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will set the power consumption rate of\n");
        printf("\t\tthe device to the value input.\n");
        printf("\t\tOptions:\n");
        printf("\t\t-default - sets the device back to default settings\n");
        printf("\t\t-highest - sets the active level to \"highest\"\n");
        printf("\t\t-intermediate - sets the active level to \"intermediate\"\n");
        printf("\t\t-lowest - sets the active level to \"lowest\"\n");
        printf("\t\t-watt value - sets the device to a nearest watt value\n");
        printf("\t\tless than or equal to the value entered.\n");
        printf("\t\tPower consumption watt values are listed with the\n");
        printf("\t\t--%s command line option.\n\n", SHOW_POWER_CONSUMPTION_LONG_OPT_STRING);
    }
}

void print_Test_Unit_Ready_Help(bool shortHelp)
{
    printf("\t--%s\n", TEST_UNIT_READY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tIssues a SCSI Test Unit Ready command and displays the\n");
        printf("\t\tstatus. If the drive is not ready, the sense key, asc,\n");
        printf("\t\tascq, and fru will be displayed and a human readable\n");
        printf("\t\ttranslation from the SPC spec will be displayed if one\n");
        printf("\t\tis available.\n\n");
    }
}

void print_Firmware_Download_Help(bool shortHelp)
{
    printf("\t--%s [firmware_filename]\t\t\t\t\n", DOWNLOAD_FW_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tDownload firmware to a Seagate storage product. Use only\n");
        printf("\t\tSeagate authorized firmware data files which are designated\n");
        printf("\t\tfor the specific model drive. Improper use of this option may\n");
        printf("\t\tharm a device and or its data. You may specify the path (without\n");
        printf("\t\tspaces) if the firmware data file is in a different location.\n");
        printf("\t\tThis option will use segmented download by default. Use the\n");
        printf("\t\t--downloadMode option to specify a different download mode.\n\n");
    }
}

void print_Firmware_Slot_Buffer_ID_Help(bool shortHelp)
{
    printf("\t--%s/--%s slot#\n", FIRMWARE_SLOT_LONG_OPT_STRING, FIRMWARE_BUFFER_ID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a firmware slot (NVMe) or a buffer ID (SCSI)\n");
        printf("\t\talong with the --%s (SCSI) or --%s (NVMe & SCSI) options.\n", DOWNLOAD_FW_MODE_LONG_OPT_STRING, ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
        printf("\t\tIf this option is not used, a value of zero will be used instead,\n");
        printf("\t\twhich means the drive will automatically select the slot number.\n\n");
    }
}

void print_Firmware_Activate_Help(bool shortHelp)
{
    printf("\t--%s\n", ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to issue the command to activate code that was\n");
        printf("\t\tsent to the drive using a deferred download command. This will\n");
        printf("\t\timmediately activate the new code on the drive.\n");
        printf("\t\tYou can use this along with a --%s & --%s to\n", DOWNLOAD_FW_LONG_OPT_STRING, DOWNLOAD_FW_MODE_LONG_OPT_STRING);
        printf("\t\tautomatically issue the activate command after the download has\n");
        printf("\t\tcompleted.\n\n");
    }
}

void print_Firmware_Switch_Help(bool shortHelp)
{
    printf("\t--%s \t(NVMe Only) (Seagate Only)\n", SWITCH_FW_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to switch to a different firmware slot on an\n");
        printf("\t\tNVMe drive. You must specify a slot with the --%s option\n", FIRMWARE_SLOT_LONG_OPT_STRING);
        printf("\t\tor this will fail. The specified slot must already have a\n");
        printf("\t\tvalid firmware image in it as well.\n\n");
    }
}

void print_Firmware_Download_Mode_Help(bool shortHelp)
{
    printf("\t--%s [ full | segmented | deferred ]\n", DOWNLOAD_FW_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option along with the --%s option\n", DOWNLOAD_FW_LONG_OPT_STRING);
        printf("\t\tto set the firmware download mode.\n");
        printf("\t\tSupported Modes:\n");
        printf("\t\t\tfull - performs a download in one large\n");
        printf("\t\t\t            transfer to the device.\n");
        printf("\t\t\tsegmented - downloads the firmware in multiple\n");
        printf("\t\t\t            segments to the device. (Most compatible)\n");
        printf("\t\t\tdeferred - performs a segmented download to the\n");
        printf("\t\t\t           device, but does not activate the new\n");
        printf("\t\t\t           firmware until a powercycle or activate\n");
        printf("\t\t\t           command is sent.\n\n");
    }
}

void print_Output_Mode_Help(bool shortHelp)
{
    printf("\t--%s   [ raw | binary ]\n", OUTPUT_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with others options such as --%s\n",GET_NVME_LOG_LONG_OPT_STRING);
        printf("\t\tand --%s to show a buffer output or to\n",DEVICE_INFO_LONG_OPT_STRING);
        printf("\t\tcreate a binary file.\n");
        printf("\t\tSupported Modes:\n");
        printf("\t\t\traw    - prints the raw buffer on stdout\n");
        printf("\t\t\t  \n");
        printf("\t\t\tbinary - writes data to a file with device\n");
        printf("\t\t\t         Serial Number & time stamp\n\n");
    }
}


void print_Get_Features_Help(bool shortHelp)
{
    printf("\t--%s  [ help | list | # ]\n", GET_FEATURES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to list the NVMe features\n");
        printf("\t\tSupported Modes:\n");
        printf("\t\t\thelp  - prints the descriptions of all\n");
        printf("\t\t\t        the features\n");
        printf("\t\t\tlist  - lists raw values of all mandatory\n");
        printf("\t\t\t        features supported by device\n");
        printf("\t\t\t#     - feature number will print the \n");
        printf("\t\t\t        humanized details\n\n");
    }
}

void print_NVMe_Get_Log_Help(bool shortHelp)
{
    printf("\t--%s   [ error | smart | fwSlots | suppCmds | selfTest | # ]\n", GET_NVME_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to get the NVMe log pages\n");
        printf("\t\tSupported Modes:\n");
        printf("\t\t\terror   - lists the valid entries found in the\n");
        printf("\t\t\t          Error Information Log.\n");
        printf("\t\t\tsmart   - lists information found in the \n");
        printf("\t\t\t          SMART/Health Information Log\n");
        printf("\t\t\tfwSlots - lists currently active Firmware slot\n");
        printf("\t\t\t          and Firmware Revision in each slot.\n");
        printf("\t\t\tsuppCmds - lists the commands that the controller\n");
        printf("\t\t\t          supports and the effects of those commands.\n");
        printf("\t\t\tselfTest - lists the status of any device\n");
        printf("\t\t\t          self-test operation in progress\n");
        printf("\t\t\t          and the results of the last 20 device\n");
        printf("\t\t\t          self-test operations.\n");
        printf("\t\t\t#       - option to get the log page using\n");
        printf("\t\t\t          a number\n\n");
    }
}

void print_NVMe_Get_Tele_Help(bool shortHelp)
{
    printf("\t--%s [host | ctrl]\n", GET_NVME_TELE_LONG_OPT_STRING);

    if (!shortHelp)
    {
        printf("\t\tUse this option to get the NVMe Telemetry data for a device.\n");
        printf("\t\tUse the --telemetryDataArea option to control the amount of \n");
        printf("\t\tdata collected. \n\n");
        printf("\t\tSupported Modes:\n");
        printf("\t\t\thost - get Host Telemetry\n");
        printf("\t\t\tctrl - get Ctrl Telemetry\n\n");
        printf("\n");
    }

    printf("\t--%s [1 | 2 | 3]\n", NVME_TELE_DATA_AREA_LONG_OPT_STRING);

    if(!shortHelp)
    {
        printf("\t\tThis is a sub-command which defines the amount of data \n");
        printf("\t\tcollected by the --%s option. Data Area 3 is assumed \n", GET_NVME_TELE_LONG_OPT_STRING);
        printf("\t\tif this option is not used. \n");
        printf("\n");
        printf("\t\tSupported Data Area.\n");
        printf("\t\t1 - get minimal telemetry data\n");
        printf("\t\t2 - get telemetry data additional to data area 1\n");
        printf("\t\t3 - get telemetry data additional to data area 2 (default data area)\n");
        printf("\n");
    }
}

void print_NVMe_Temp_Stats_Help(bool shortHelp)
{
    printf("\t--%s   \n", NVME_TEMP_STATS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to get the NVMe Temperature Statistics\n\n");
    }
}

void print_NVMe_Pci_Stats_Help(bool shortHelp)
{
    printf("\t--%s   \n", NVME_PCI_STATS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to get the NVMe PCIe Statistics\n\n");
    }
}


void print_Set_Max_LBA_Help(bool shortHelp)
{
    printf("\t--%s newMaxLBA\n", SET_MAX_LBA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tSet the max accessible address of your drive to any value less\n");
        printf("\t\tthan the device's default native size. A power cycle is\n");
        printf("\t\trequired after this command before resetting or setting a new\n\t\tmax LBA.\n\n");
    }
}

void print_Restore_Max_LBA_Help(bool shortHelp)
{
    printf("\t--%s\n", RESTORE_MAX_LBA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tRestore the max accessible address of your drive to its native\n");
        printf("\t\tsize. A power cycle is required after this command before\n");
        printf("\t\tsetting a new max LBA.\n\n");
    }
}

void printf_Set_Phy_Speed_Help(bool shortHelp)
{
    printf("\t--%s [0 | 1 | 2 | 3] (SATA Only) (Seagate Only)\n", SET_PHY_SPEED_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to change the PHY speed to a\n");
        printf("\t\tnew maximum value.\n");
        printf("\t\t0 - allow full negotiation (default drive behavior)\n");
        printf("\t\t1 - allow negotiation up to 1.5Gb/s\n");
        printf("\t\t2 - allow negotiation up to 3.0Gb/s\n");
        printf("\t\t3 - allow negotiation up to 6.0Gb/s\n");
        //printf("\t\t4 - allow negotiation up to 12.0Gb/s (SAS Only)\n");//enable this when SAS support for this option has been enabled
        printf("\n");
    }
}

void print_Set_SSC_Help(bool shortHelp)
{
    printf("\t--%s [info | default | enable | disable] (SATA Only) (Seagate Only)\n", SSC_FEATURE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to change or view the SSC (Spread Spectrum\n");
        printf("\t\tClocking) mode on a Seagate SATA drive. Only change this\n");
        printf("\t\tsetting if you are experiencing compatibility problems with\n");
        printf("\t\tthe drive in a system.\n");
        printf("\t\tinfo - show current SSC state\n");
        printf("\t\tdefault - set to drive default mode\n");
        printf("\t\tenable - enable SSC\n");
        printf("\t\tdisable - disable SSC\n");
        printf("\n");
    }
}

void print_Set_Ready_LED_Help(bool shortHelp)
{
    printf("\t--%s [info | on | off | default] (SAS Only)\n", SET_READY_LED_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to get the current state or change the\n");
        printf("\t\tbehavior of the ready LED.\n");
        printf("\t\tSee the SPL spec for full details on how this changes LED\n");
        printf("\t\t    info - gets the current state of the ready LED.\n");
        printf("\t\t    on - sets the ready LED to usually off unless\n");
        printf("\t\t         processing a command.\n");
        printf("\t\t    off - sets the ready LED to usually on unless\n");
        printf("\t\t          processing a command\n");
        printf("\t\t    default - sets the ready LED to the drive's default value\n\n");
    }
}

void print_Read_Look_Ahead_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable]\n", READ_LOOK_AHEAD_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable read look-ahead\n");
        printf("\t\tsupport on a drive. Use the \"info\" argument to get\n");
        printf("\t\tthe current status of the read look ahead feature.\n\n");
    }
}


void print_NV_Cache_Bit_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable]\t(SAS Only)\n", NV_CACHE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable the SCSI Non-Volatile cache\n");
        printf("\t\t on a drive. Use the \"info\" argument to get\n");
        printf("\t\tthe current status of the Non-Volatile Cache setting.\n\n");
    }
}

void print_Write_Cache_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable]\n", WRITE_CACHE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable write cache\n");
        printf("\t\tsupport on a drive. Use the \"info\" argument to get\n");
        printf("\t\tthe current status of the write cache feature.\n\n");
    }
}

void print_SCT_Write_Cache_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable | default] (SATA Only)\n", SCT_WRITE_CACHE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable write cache\n");
        printf("\t\tsupport on a drive using SMART command transport.\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tWhen using this option, the --%s option\n", WRITE_CACHE_LONG_OPT_STRING);
        printf("\t\twill always return success, but no write cache changes\n");
        printf("\t\twill occur. This follows ATA spec.\n");
        printf("\t\tUsing the \"default\" argument returns the drive to\n");
        printf("\t\tdefault settings and allowing the --%s\n", WRITE_CACHE_LONG_OPT_STRING);
        printf("\t\toption to work again.\n");
        printf("\t\tUse the \"info\" argument to get the current status\n");
        printf("\t\tof the write cache feature. Note: On some SAT\n");
        printf("\t\tHBAs/bridges, status will not be able to be\n");
        printf("\t\tdetermined due to HBA/bridge limitations.\n\n");
    }
}

void print_SCT_Write_Cache_Reordering_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable | default] (SATA Only)\n", SCT_WRITE_CACHE_REORDER_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable write cache reordering\n");
        printf("\t\tsupport on a drive using SMART command transport.\n");
        printf("\t\tWrite cache reordering allows the drive to reorder moving data\n");
        printf("\t\tout of cache to media for better performance on synchronous\n");
        printf("\t\tcommands. Asynchronous commands are only affected when in-order\n");
        printf("\t\tdata delivery is enabled.\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tUse the \"info\" argument to get the current status\n");
        printf("\t\tof the write cache reordering feature. Note: On some SAT\n");
        printf("\t\tHBAs/bridges, status will not be able to be\n");
        printf("\t\tdetermined due to HBA/bridge limitations.\n\n");
    }
}

void print_SCT_Error_Recovery_Read_Help(bool shortHelp)
{
    printf("\t--%s [info | value] (SATA Only)\n", SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the read command timer value for\n");
        printf("\t\tsynchronous commands and NCQ commands with in-order data\n");
        printf("\t\tdelivery enabled. Note: this timer starts at the time that\n");
        printf("\t\tthe drive processes the command, not the time it is received.\n");
        printf("\t\tThis timer value is volatile and is cleared at each power cycle.\n");
        printf("\t\tUse the \"info\" argument to get the current status\n");
        printf("\t\tof the read timer. A value of 0 means that all possible\n");
        printf("\t\terror recovery will be performed before returning status.\n");
        printf("\t\tOther values should include a unit to know the time to use.\n");
        printf("\t\tIf no unit is provided, it is assumed to be the value * 100 ms\n");
        printf("\t\tEx1: --%s 15s for a 15 second timer.\n", SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
        printf("\t\tEx2: --%s 15000ms for a 15 second timer expressed in milliseconds\n", SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
        printf("\t\tEx2: --%s 150 for a 15 second timer with no units specified\n", SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
        printf("\t\tThe maximum time that can be specified is 1 hour, 49 minutes, 13 seconds\n");
        printf("\t\tNote: On some SAT HBAs/bridges, status will not be able to be\n");
        printf("\t\tdetermined due to HBA/bridge limitations.\n\n");
    }
}

void print_SCT_Error_Recovery_Write_Help(bool shortHelp)
{
    printf("\t--%s [info | value] (SATA Only)\n", SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the write command timer value for\n");
        printf("\t\tsynchronous commands and NCQ commands with in-order data\n");
        printf("\t\tdelivery enabled. Note: this timer starts at the time that\n");
        printf("\t\tthe drive processes the command, not the time it is received.\n");
        printf("\t\tThis timer value is volatile and is cleared at each power cycle.\n");
        printf("\t\tUse the \"info\" argument to get the current status\n");
        printf("\t\tof the write timer. A value of 0 means that all possible\n");
        printf("\t\terror recovery will be performed before returning status.\n");
        printf("\t\tOther values should include a unit to know the time to use.\n");
        printf("\t\tIf no unit is provided, it is assumed to be the value * 100 ms\n");
        printf("\t\tEx1: --%s 15s for a 15 second timer.\n", SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING);
        printf("\t\tEx2: --%s 15000ms for a 15 second timer expressed in milliseconds\n", SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING);
        printf("\t\tEx2: --%s 150 for a 15 second timer with no units specified\n", SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING);
        printf("\t\tThe maximum time that can be specified is 1 hour, 49 minutes, 13 seconds\n");
        printf("\t\tNote: On some SAT HBAs/bridges, status will not be able to be\n");
        printf("\t\tdetermined due to HBA/bridge limitations.\n\n");
    }
}

void print_Provision_Help(bool shortHelp)
{
    printf("\t--%s newMaxLBA\n", PROVISION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tProvision your drive to a new max LBA to any value less\n");
        printf("\t\tthan the device's current max LBA. A power cycle is required\n");
        printf("\t\tafter this command before resetting the max LBA or changing\n");
        printf("\t\tthe provisioning again. This command erases all data between\n");
        printf("\t\tthe new maxLBA specified and the current maxLBA of the device.\n");
        printf("\t\tusing a TRIM/UNMAP command.\n\n");
    }
}

void print_Phy_Speed_Help(bool shortHelp)
{
    printf("\t--%s [0 | 1 | 2 | 3 | 4 | 5]\n", SET_PHY_SPEED_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to change the PHY speed to a\n");
        printf("\t\tnew maximum value. On SAS, this option will\n");
        printf("\t\tset all phys to the specified speed unless the\n");
        printf("\t\t--%s option is given to select a specific phy.\n", SET_PHY_SAS_PHY_LONG_OPT_STRING);
        printf("\t\t0 - allow full negotiation (default drive behavior)\n");
        printf("\t\t1 - allow negotiation up to 1.5Gb/s\n");
        printf("\t\t2 - allow negotiation up to 3.0Gb/s\n");
        printf("\t\t3 - allow negotiation up to 6.0Gb/s\n");
        printf("\t\t4 - allow negotiation up to 12.0Gb/s (SAS Only)\n");
        printf("\t\t5 - allow negotiation up to 22.5Gb/s (SAS Only)\n");
        printf("\n");
    }
}

void print_SAS_Phy_Help(bool shortHelp)
{
    printf("\t--%s [phy number] (SAS Only)\n", SET_PHY_SAS_PHY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a specific phy to use\n");
        printf("\t\twith another option that uses a phy identifier value.\n");
        printf("\t\tSome tool options will assume all SAS Phys when this\n");
        printf("\t\toption is not present. Others will produce an error when\n");
        printf("\t\ta specific phy is needed for an operation.\n");
        printf("\t\tUse the -%c option to learn more about the supported phys.\n\n", DEVICE_INFO_SHORT_OPT);
    }
}

void print_SAS_Phy_Start_Test_Pattern(bool shortHelp)
{
    printf("\t--%s\n", SAS_PHY_START_TEST_PATTERN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to start a test pattern on a SAS phy.\n");
        printf("\t\tYou must specify a phy with the --%s option.\n", SET_PHY_SAS_PHY_LONG_OPT_STRING);
        printf("\t\tUse the --%s option when you want to stop the test\n", SAS_PHY_STOP_TEST_PATTERN_LONG_OPT_STRING);
        printf("\t\tpattern on a selected phy.\n");
        printf("\t\tThe following options are used to control the test pattern:\n");
        printf("\t\t\t--%s\n", SAS_PHY_TEST_PATTERN_LONG_OPT_STRING);
        printf("\t\t\t--%s\n", SAS_PHY_TEST_FUNCTION_SSC_LONG_OPT_STRING);
        printf("\t\t\t--%s\n", SAS_PHY_LINK_RATE_LONG_OPT_STRING);
        printf("\t\t\t--%s\n", SAS_PHY_DWORD_CONTROL_LONG_OPT_STRING);
        printf("\t\t\t--%s\n", SAS_PHY_DWORDS_LONG_OPT_STRING);
        printf("\n");
    }
}

void print_SAS_Phy_Stop_Test_Pattern(bool shortHelp)
{
    printf("\t--%s\n", SAS_PHY_STOP_TEST_PATTERN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to stop a test pattern on a SAS phy that\n");
        printf("\t\twas started with the --%s option.\n", SAS_PHY_START_TEST_PATTERN_LONG_OPT_STRING);
        printf("\t\tYou must specify a phy with the --%s option.\n\n", SET_PHY_SAS_PHY_LONG_OPT_STRING);
    }
}

void print_SAS_Phy_Test_Pattern(bool shortHelp)
{
    printf("\t--%s [test pattern or hex code as 0x?? or ??h]\n", SAS_PHY_TEST_PATTERN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to start the specified test pattern.\n");
        printf("\t\tFor details on the test patterns, see SPL and SAS specs.\n");
        printf("\t\tList of known test patterns:\n");
        printf("\t\t\tJTPAT\n");
        printf("\t\t\tCJTPAT\n");
        printf("\t\t\tPRBS9\n");
        printf("\t\t\tPRBS15\n");
        printf("\t\t\tTRAIN\n");
        printf("\t\t\tTRAIN_DONE\n");
        printf("\t\t\tIDLE\n");
        printf("\t\t\tSCRAMBLED_0\n");
        printf("\t\t\tTWO_DWORDS\n");
        printf("\t\t\t   See --%s and --%s options for Two DWords\n", SAS_PHY_DWORD_CONTROL_LONG_OPT_STRING, SAS_PHY_DWORDS_LONG_OPT_STRING);
        printf("\t\t\tFERL (Seagate Only)\n");
        printf("\t\t\tTRAIN_DFE (Seagate Only)\n");
        printf("\n");
    }
}

void print_SAS_Phy_Test_SSC_Function(bool shortHelp)
{
    printf("\t--%s [NO_SPREADING | DOWN_SPREADING | CENTER_SPREADING]\n", SAS_PHY_TEST_FUNCTION_SSC_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the SSC modulation type.\n");
        printf("\t\tSee SAS spec for details on the modulation types.\n");
        printf("\t\tSupported SSC types:\n");
        printf("\t\t\tNO_SPREADING\n");
        printf("\t\t\tCENTER_SPREADING\n");
        printf("\t\t\tDOWN_SPREADING\n");
        printf("\n");
    }
}

void print_SAS_Phy_Test_Link_Rate(bool shortHelp)
{
    printf("\t--%s [1.5 | 3.0 | 6.0 | 12.0 | 22.5]\n", SAS_PHY_LINK_RATE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the link rate to test using\n");
        printf("\t\tthe --%s option.\n", SAS_PHY_START_TEST_PATTERN_LONG_OPT_STRING);
        printf("\t\tSupported values:\n");
        printf("\t\t    1.5 - 1.5Gb/s\n");
        printf("\t\t    3.0 - 3.0Gb/s\n");
        printf("\t\t    6.0 - 6.0Gb/s\n");
        printf("\t\t    12.0 - 12.0Gb/s\n");
        printf("\t\t    22.5 - 22.5Gb/s\n");
        printf("\t\t    hex codes from SPL spec also supported.\n");
        printf("\n");
    }
}

void print_SAS_Phy_Test_DWord_Control(bool shortHelp)
{
    printf("\t--%s [NO_SCRAMBLING | 5THCONTROL | 1STCONTROL | 1ST&5THCONTROL]\n", SAS_PHY_DWORD_CONTROL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the DWord Control value to use when\n");
        printf("\t\tspecifying a dword pattern with the --%s option.\n", SAS_PHY_DWORDS_LONG_OPT_STRING);
        printf("\t\tSupported values: (see SPL spec for details)\n");
        printf("\t\t\tNO_SCRAMBLING\n");
        printf("\t\t\t5THCONTROL\n");
        printf("\t\t\t1STCONTROL\n");
        printf("\t\t\t1ST&5THCONTROL\n");
        printf("\n");
    }
}

void print_SAS_Phy_Test_DWord_Pattern(bool shortHelp)
{
    printf("\t--%s [hex value | D10.2 | D21.5 | D24.3 | D25.6&D6.1 | D30.3 | ALIGN_0 | ALIGN_1 | PAIR_ALIGN_0]\n", SAS_PHY_DWORDS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the two DWords to use for a test\n");
        printf("\t\tpattern. Specify a hex value like 7878787878787878h.\n");
        printf("\t\tKnown values that specify a spec pattern:\n");
        printf("\t\t\tD10.2 - specifies 4A4A4A4A4A4A4A4Ah\n");
        printf("\t\t\tD21.5 - specifies B5B5B5B5B5B5B5B5h\n");
        printf("\t\t\tD24.3 - specifies 7878787878787878h\n");
        printf("\t\t\tD25.6&D6.1 - specifies D926D926D926D926h\n");
        printf("\t\t\tD30.3 - specifies 7E7E7E7E7E7E7E7Eh\n");
        printf("\t\t\tALIGN_0 - specifies BC4A4A7BBC4A4A7Bh\n");
        printf("\t\t\tALIGN_1 - specifies BC070707BC070707h\n");
        printf("\t\t\tPAIR_ALIGN_0 - specifies BC4A4A7B4A787E7Eh\n");
        printf("\n");
    }
}

void print_SAS_Phy_Partial_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable] (SAS Only)\n", SAS_PARTIAL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable the partial phy power\n");
        printf("\t\tcondition. This is from the enhanced phy control mode page.\n");
        printf("\t\tUse the --%s option to specify an individual phy,\n", SET_PHY_SAS_PHY_LONG_OPT_STRING);
        printf("\t\totherwise this will be changed on all phys.\n");
        printf("\t\tWARNING: Configuring this setting may cause the drive to be\n");
        printf("\t\tundetectable by other hardware if this power condition is not\n");
        printf("\t\tsupported by a controller or expander.\n\n");
    }
}

void print_SAS_Phy_Slumber_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable] (SAS Only)\n", SAS_SLUMBER_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable the slumber phy power\n");
        printf("\t\tcondition. This is from the enhanced phy control mode page.\n");
        printf("\t\tUse the --%s option to specify an individual phy,\n", SET_PHY_SAS_PHY_LONG_OPT_STRING);
        printf("\t\totherwise this will be changed on all phys.\n");
        printf("\t\tWARNING: Configuring this setting may cause the drive to be\n");
        printf("\t\tundetectable by other hardware if this power condition is not\n");
        printf("\t\tsupported by a controller or expander.\n\n");
    }
}

void print_Supported_Logs_Help(bool shortHelp)
{
    printf("\t--%s\n", LIST_LOGS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tDisplays a list of all supported logs by this device type.\n\n");
    }
}

void print_Pull_Generic_Logs_Help(bool shortHelp)
{
    printf("\t--%s [Log Number]\n", GENERIC_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tPulls specific log number from the device\n");
        printf("\t\t[Log Number] is required argument & can be passed\n");
        printf("\t\tas an decimal or hex value.\n");
        printf("\t\tWARNING:  Vendor Unique Logs pulled using this option\n");
        printf("\t\t          may not be valid due to unknown vendor unique\n");
        printf("\t\t          bits in ATA/SCSI/NVMe etc. command fields.\n\n");
    }
}

void print_Pull_Generic_Logs_Subpage_Help(bool shortHelp)
{
    printf("\t--%s [Subpage Number]\t\t(SAS Only)\n", GENERIC_LOG_SUBPAGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with the --%s option to specify\n", GENERIC_LOG_LONG_OPT_STRING);
        printf("\t\ta log subpage to pull. Use this for SCSI Logs.\n");
        printf("\t\t[Subpage Number] can be passed as an decimal or hex value.\n");
        printf("\t\tWARNING:  Vendor Unique Logs pulled using this option\n");
        printf("\t\t          may not be valid due to unknown vendor unique\n");
        printf("\t\t          bits in ATA/SCSI/NVMe etc. command fields.\n\n");
    }
}

void print_Supported_Error_History_Help(bool shortHelp)
{
    printf("\t--%s\t\t\t(SAS Only)\n", LIST_ERROR_HISTORY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tDisplays a list of all supported error history buffer IDs\n");
        printf("\t\tsupported by the device.\n\n");
    }
}

void print_Pull_Generic_Error_History_Help(bool shortHelp)
{
    printf("\t--%s [Buffer ID]\t(SAS Only)\n", GENERIC_ERROR_HISTORY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tPulls specific error history buffer ID from the device\n");
        printf("\t\t[Buffer ID] is required argument & can be passed\n");
        printf("\t\tas an decimal or hex value.\n");
        printf("\t\tWARNING:  Vendor Unique Logs pulled using this option\n");
        printf("\t\t          may not be valid due to unknown vendor unique\n");
        printf("\t\t          bits in ATA/SCSI/NVMe etc. command fields.\n\n");
    }
}

void print_Log_Mode_Help(bool shortHelp)
{
    printf("\t--%s [mode]\n", PULL_LOG_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tSets the mode to pull the log. \n");
        printf("\t\tUse this option with --%s to set the desired mode\n", GENERIC_LOG_LONG_OPT_STRING);
        printf("\t\t\traw - Pulls log & prints it to the\n");
        printf("\t\t\t      screen as stdout. (default)\n");
        printf("\t\t\tbin - Pulls log & saves it to\n");
        printf("\t\t\t      a timestamped binary file.\n\n");
    }
}

void print_SAT_Info_Help(bool shortHelp)
{
    printf("\t--%s\n", SAT_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tDisplays SATA device information on any interface\n");
        printf("\t\tusing both SCSI Inquiry / VPD / Log reported data\n");
        printf("\t\t(translated according to SAT) and the ATA Identify / Log\n");
        printf("\t\treported data.\n\n");
    }
}

void print_DST_And_Clean_Help(bool shortHelp)
{
    printf("\t--%s\n", DST_AND_CLEAN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tRuns DST, then checks for an error and repairs the\n");
        printf("\t\terror if possible. This continues until all errors\n");
        printf("\t\treported by DST are fixed, or when the error limit is\n");
        printf("\t\treached. The default limit is 50 errors.\n\n");
    }
}

void print_Generic_Test_Mode_Help(bool shortHelp)
{
    printf("\t--%s [ read | write | verify ]\n", GENERIC_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis options allows selection of the type of commands\n");
        printf("\t\tto use while performing a generic test. The modes supported\n");
        printf("\t\tare listed below:\n");
        printf("\t\t\tread - performs a generic test using read commands\n");
        printf("\t\t\twrite - performs a generic test using write commands\n");
        printf("\t\t\tverify - performs a generic test using verify commands\n\n");
    }
}

void print_Show_Supported_Erase_Modes_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_ERASE_SUPPORT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option checks the drive to determine which methods of\n");
        printf("\t\tdata erasure are supported and lists them, from fastest to\n");
        printf("\t\tslowest.\n\n");
    }
}

void print_Perform_Quickest_Erase_Help(bool shortHelp)
{
    printf("\t--%s\n", PERFORM_FASTEST_ERASE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option checks the drive to determine which methods of\n");
        printf("\t\tdata erasure are supported and determines which is the\n");
        printf("\t\tquickest to erase ALL data on the drive. It then starts the\n");
        printf("\t\tquickest erase. Combine this option with the --%s option to\n", POLL_LONG_OPT_STRING);
        printf("\t\tenable polling for progress on the fastest erase.\n");
        printf("\t\tNote: Some erase methods require polling and will have polling\n");
        printf("\t\tenabled by default.\n");
        printf("\t\tNote 2: If revertSP is the fastest, it will not be started since\n");
        printf("\t\tthe drive PSID must be passed in on the command line.\n\n");
    }
}

void print_Format_Unit_Help(bool shortHelp)
{
    printf("\t--%s [current | new sector size]    (SAS Only)\n", FORMAT_UNIT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will start a format unit operation on a SAS drive\n");
        printf("\t\tUse \"current\" to perform a format unit operation with the\n");
        printf("\t\tSector size currently being used, otherwise enter a new sector\n");
        printf("\t\tsize to use upon format completion. This command will erase all\n");
        printf("\t\tdata on the drive. Combine this option with --%s to poll\n", POLL_LONG_OPT_STRING);
        printf("\t\tfor progress until the format is complete.\n\n");
    }
}

void print_Fast_Format_Help(bool shortHelp)
{
    printf("\t--%s [fast format mode]\t(SAS Only) (SBC4 required)\n", FAST_FORMAT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with the --%s option\n", FORMAT_UNIT_LONG_OPT_STRING);
        printf("\t\tto run a fast format.\n");
        printf("\t\tAvailable fast format modes:\n");
        printf("\t\t    0 - This is a standard format unit command. All logical\n");
        printf("\t\t        blocks will be overwritten. This command will take a\n");
        printf("\t\t        very long time\n");
        printf("\t\t    1 - This is a fast format unit command keeping existing\n");
        printf("\t\t        data in physical sector. This option can be used to\n");
        printf("\t\t        quickly change the the logical sector size between\n");
        printf("\t\t        5xxe and 4xxx. The media may be readable, but data\n");
        printf("\t\t        may be unspecified or may return errors on read access\n");
        printf("\t\t        according to it's error processing algorithms.\n");
        printf("\t\t    2 - This is a fast format unit command that can change the\n");
        printf("\t\t        logical sector size quickly. Media may or may not be\n");
        printf("\t\t        read accessible until a write has been performed to\n");
        printf("\t\t        the media.\n\n");
    }
}

void print_Format_Disable_Primary_List_Help(bool shortHelp)
{
    printf("\t--%s\n", FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to disable using the primary defect list\n");
        printf("\t\twhen performing a format unit operation.\n\n");
    }
}

void print_Format_Discard_Grown_Defect_List_Help(bool shortHelp)
{
    printf("\t--%s\n", FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to discard the existing grown defect list\n");
        printf("\t\twhen performing a format unit operation. (set complete list bit)\n\n");
    }
}

void print_Format_Disable_Certification_Help(bool shortHelp)
{
    printf("\t--%s\n", FORMAT_UNIT_DISABLE_CERTIFICATION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to disable the certification operation\n");
        printf("\t\twhen performing a format unit operation.\n\n");
    }
}

void print_Format_Security_Initialize_Help(bool shortHelp)
{
    printf("\t--%s\n", FORMAT_UNIT_SECURITY_INITIALIZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the security initialize bit in the\n");
        printf("\t\tinitialization pattern for a format unit command.\n");
        printf("\t\tSBC recommends migrating to sanitize to overwrite previously\n");
        printf("\t\treallocated sectors.\n");
        printf("\t\tNote: Not all products support this option.\n\n");
    }
}

void print_Format_Protection_Type_Help(bool shortHelp)
{
    printf("\t--%s [ 0 | 1 | 2 | 3 ]\n", FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the protection type to format the\n");
        printf("\t\tmedium with.\n");
        printf("\t\tNote: Not all devices support protection types.\n\n");
    }
}

void print_Format_Protection_Interval_Exponent_Help(bool shortHelp)
{
    printf("\t--%s [ exponent value ]\n", FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the protection interval exponent\n");
        printf("\t\tfor protection types 2 & 3. This option is ignored for all\n");
        printf("\t\tother protection types.\n\n");
    }
}
void print_Format_Default_Format_Help(bool shortHelp)
{
    printf("\t--%s\n", FORMAT_UNIT_DEFAULT_FORMAT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to perform a device's default format operation.\n");
        printf("\t\tNote: This mode may take a long time to complete.\n");
        printf("\t\tNote2: When this option is specified, only the --%s\n", FAST_FORMAT_LONG_OPT_STRING);
        printf("\t\t       and --%s options will be used. All others are\n", FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG_LONG_OPT_STRING);
        printf("\t\t       ignored.\n\n");
    }
}

void print_Format_Disable_Immediate_Response_Help(bool shortHelp)
{
    printf("\t--%s\n", FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to disable the immediate response bit in\n");
        printf("\t\ta format unit operation.\n");
        printf("\t\tNote: This mode may take a long time to complete.\n\n");
    }
}

void print_Format_Stop_On_List_Error_Help(bool shortHelp)
{
    printf("\t--%s\n", FORMAT_UNIT_STOP_ON_LIST_ERROR_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the stop format bit in a format unit.\n");
        printf("\t\tIf the device cannot locate or access an existing primary or\n");
        printf("\t\tgrown defect list, the format will stop and return with an error.\n\n");
    }
}

void print_Format_New_Max_LBA_Help(bool shortHelp)
{
    printf("\t--%s\n", FORMAT_UNIT_NEW_MAX_LBA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a new Max LBA for a drive during a\n");
        printf("\t\tformat unit operation. This can speed up a format unit if\n");
        printf("\t\tformatting to test something, or also desiring to reduce a drive's\n");
        printf("\t\tcapacity while formattting.\n\n");
    }
}

void print_Show_Format_Status_Log_Help(bool shortHelp)
{
    printf("\t--%s (SAS Only)\n", SHOW_FORMAT_STATUS_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to view the SCSI format status log.\n");
        printf("\t\tNote: This log is only valid after a successful format\n");
        printf("\t\tunit operation.\n\n");
    }
}

void print_Set_Sector_Size_Help(bool shortHelp)
{
    printf("\t--%s [new sector size]\t\n", SET_SECTOR_SIZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is only available for drives that support sector\n");
        printf("\t\tsize changes. On SATA Drives, the set sector configuration\n");
        printf("\t\tcommand must be supported. On SAS Drives, fast format must\n");
        printf("\t\tbe supported. A format unit can be used instead of this\n");
        printf("\t\toption to perform a long format and adjust sector size.\n");
        printf("\t\tUse the --%s option to see the sector\n", SHOW_SUPPORTED_FORMATS_LONG_OPT_STRING);
        printf("\t\tsizes the drive reports supporting. If this option\n");
        printf("\t\tdoesn't list anything, please consult your product manual.\n");
        printf("\t\tThis option should be used to quickly change between 5xxe and\n");
        printf("\t\t4xxx sector sizes. Using this option to change from 512 to 520\n");
        printf("\t\tor similar is not recommended at this time due to limited drive\n\t\tsupport\n\n");
    }
}

void print_Show_Supported_Formats_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_SUPPORTED_FORMATS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will show the supported formats of a device.\n");
        printf("\t\tThese can be used to change the sector size or \n");
        printf("\t\tused with a format operation. On SAS, this is the\n");
        printf("\t\tsupported block lengths and protection types VPD page. (SBC4\n");
        printf("\t\tand later) On SATA, this is the sector configuration log. (ACS4\n");
        printf("\t\tand later) If the device does not report supported sector\n");
        printf("\t\tsizes, please consult your product manual.\n\n");
    }
}

void print_TCG_Info_Help(bool shortHelp)
{
    printf("\t--%s\n", TCG_DEVICE_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option shows information about a TCG device\n\n");
    }
}

void print_Set_FWDL_Port_Help(bool shortHelp)
{
    printf("\t--%s [lock | unlock] \n", FWDL_PORT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will set the logical firmware download\n");
        printf("\t\tport on a TCG drive to the requested state.\n");
        printf("\t\tUse the --%s option to provide SID for this operation.\n", TCG_SID_LONG_OPT_STRING);
        printf("\t\tIf SID is not provided, MSID will be used.\n");
        printf("\t\t    lock - lock the port and prevent its use\n");
        printf("\t\t    unlock - unlock the port to allow its use\n\n");
    }
}

void print_TCG_SID_Help(bool shortHelp)
{
    printf("\t--%s [yourTCGpassword]\n", TCG_SID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option can be used to specify the value of SID.\n");
        printf("\t\tThis may be required in order to perform certain TCG\n");
        printf("\t\toperations. If this is not provided, MSID will be used\n\n");
    }
}

void print_TCG_PSID_Help(bool shortHelp)
{
    printf("\t--%s [32-digit alpha-numeric code from drive label]\n", TCG_PSID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option can be used to specify the value of the PSID.\n");
        printf("\t\tThis may be required in order to perform certain TCG\n");
        printf("\t\toperations.\n\n");
    }
}

void print_Low_Current_Spinup_Help(bool shortHelp)
{
    printf("\t--%s [ low | ultra | disable ]  (SATA Only) (Seagate Only)\n", LOW_CURRENT_SPINUP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the state of the low current spinup\n");
        printf("\t\tfeature on Seagate SATA drives.\n");
        printf("\t\tWhen this setting is enabled for low or ultra low mode,\n");
        printf("\t\tthe drive will take longer to spinup and become ready.\n");
        printf("\t\tNote: This feature is not available on every drive.\n");
        printf("\t\tNote: Some products will support low, but not the ultra\n");
        printf("\t\t      low current spinup mode.\n\n");
    }
}

void print_Disable_Data_Locking_Help(bool shortHelp)
{
    printf("\t--%s (Seagate Only)\n", DISABLE_DATA_LOCKING_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to disable data locking on a TCG drive.\n");
        printf("\t\tThis option is only available when the drive has not had\n");
        printf("\t\tSID changed (taken ownership). This prevents locking the\n");
        printf("\t\tdrive when it isn't being used with security software.\n");
        printf("\t\tOnce this operation is completed, you will not be able to\n");
        printf("\t\tauthenticate to SID as MSID. This can be undone with a revertSP\n");
        printf("\t\tNote: Not all Enterprise TCG drives support this feature or\n");
        printf("\t\tthey may need a firmware update to enable this capability\n\n");
    }
}

void print_Model_Match_Help(bool shortHelp)
{
    printf("\t--%s [model Number]\n", MODEL_MATCH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to run on all drives matching the provided\n");
        printf("\t\tmodel number. This option will provide a closest match although\n");
        printf("\t\tan exact match is preferred. Ex: ST500 will match ST500LM0001\n\n");
    }
}

void print_Firmware_Revision_Match_Help(bool shortHelp)
{
    printf("\t--%s [firmware revision]\n", FW_MATCH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to run on all drives matching the provided\n");
        printf("\t\tfirmware revision. This option will only do an exact match.\n\n");
    }
}

void print_New_Firmware_Revision_Match_Help(bool shortHelp)
{
    printf("\t--%s [firmware revision]\n", NEW_FW_MATCH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to skip drives matching the provided\n");
        printf("\t\tfirmware revision. This option will only do an exact match.\n");
        printf("\t\tThis option should be used to skip performing an update if\n");
        printf("\t\ta drive already has this firmware revision loaded.\n\n");
    }
}

void print_Only_Seagate_Help(bool shortHelp)
{
    printf("\t--%s\n", ONLY_SEAGATE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to match only Seagate drives for the options\n\t\tprovided\n\n");
    }
}

void print_Set_APM_Level_Help(bool shortHelp)
{
    printf("\t--%s [1 - 254]   (SATA Only)\n", SET_APM_LEVEL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the APM level of a device.\n");
        printf("\t\tValid values are between 1 and 254.\n");
        printf("\t\t    1 = Minimum power consumption with standby mode\n");
        printf("\t\t    2-127 = Intermediate power management with standby\n\t\t        mode\n");
        printf("\t\t    128 = Minimum power consumption without standby mode\n");
        printf("\t\t    129-253 = Intermediate power management without\n\t\t        standby mode\n");
        printf("\t\t    254 = Maximum Performance.\n\n");
    }
}

void print_Show_APM_Level_Help(bool shortHelp)
{
    printf("\t--%s   (SATA Only)\n", SHOW_APM_LEVEL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show the current APM level of a device\n\n");
    }
}

void print_Disable_APM_Help(bool shortHelp)
{
    printf("\t--%s   (SATA Only)\n", DISABLE_APM_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to disable the APM feature on a device.\n");
        printf("\t\tNote: This command is optional and some device may not\n");
        printf("\t\t      disabling the APM feature.\n\n");
    }
}

void print_PUIS_Feature_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable ]  (SATA Only)\n", PUIS_FEATURE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable the power up in standby\n");
        printf("\t\t(PUIS) feature on SATA drives. \n");
        printf("\t\tNote: If this is configured on the drive with a jumper, this\n");
        printf("\t\t      command will fail.\n");
        printf("\t\tNote2: Not all products support this feature.\n\n");
    }
}

void print_Show_EPC_Settings_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_EPC_SETTINGS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show the current EPC settings on the screen.\n");
        printf("\t\tOnly drives supporting the EPC feature will show this data and\n");
        printf("\t\tonly supported power conditions will be shown.\n\n");
    }
}

void print_SMART_Feature_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable ]  (SATA Only)\n", SMART_FEATURE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable the SMART\n");
        printf("\t\tfeature on a SATA drive.\n");
        printf("\t\tNote: This command is declared obsolete in ACS4.\n\n");
    }
}

void print_Set_MRIE_Help(bool shortHelp)
{
    printf("\t--%s [ default | 0 - 6 ]  (SAS Only)\n", SET_MRIE_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to change the MRIE mode on the informational\n");
        printf("\t\texceptions mode page.\n");
        printf("\t\t    default - set to the drive default\n");
        printf("\t\t    0 - disable exception reporting\n");
        printf("\t\t    1 - Asynchronous reporting (obsolete)\n");
        printf("\t\t    2 - Establish unit attention condition\n");
        printf("\t\t    3 - Conditionally generate recovered error\n");
        printf("\t\t    4 - Unconditionally generate recovered error\n");
        printf("\t\t    5 - Generate no sense\n");
        printf("\t\t    6 - Report on request\n\n");
    }
}

void print_SMART_Attribute_Autosave_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable ]  (SATA Only)\n", SMART_ATTR_AUTOSAVE_FEATURE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable SMART\n");
        printf("\t\tattribute auto-save on an ATA drive.\n");
        printf("\t\tNote: This command is declared obsolete in ACS4.\n\n");
    }
}

void print_SMART_Info_Help(bool shortHelp)
{
    printf("\t--%s (SATA Only)\n", SMART_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will show SMART information reported\n");
        printf("\t\tby a given device.\n\n");
    }
}

void print_SMART_Auto_Offline_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable ]  (SATA Only)\n", SMART_AUTO_OFFLINE_FEATURE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable SMART\n");
        printf("\t\tauto-off-line feature on an ATA drive.\n\n");
    }
}

void print_Show_DST_Log_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_DST_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will show the entries in the DST log.\n");
        printf("\t\tUp to 21 entries may be shown (pending drive support)\n");
        printf("\t\tand will be shown with the most recent entry first.\n\n");
    }
}

void print_Force_SCSI_Help(bool shortHelp)
{
    printf("\t--%s\n", FORCE_SCSI_LONG_OPT_STRING);
    if(!shortHelp)
    {
        printf("\t\tUsing this option will force the current drive to\n");
        printf("\t\tbe treated as a SCSI drive. Only SCSI commands will\n");
        printf("\t\tbe used to talk to the drive.\n\n");
    }
}

void print_Force_ATA_Help(bool shortHelp)
{
    printf("\t--%s\n", FORCE_ATA_LONG_OPT_STRING);
    if(!shortHelp)
    {
        printf("\t\tUsing this option will force the current drive to\n");
        printf("\t\tbe treated as a ATA drive. Only ATA commands will\n");
        printf("\t\tbe used to talk to the drive.\n\n");
    }
}

void print_Force_ATA_PIO_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", FORCE_ATA_PIO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUsing this option will force the tool to issue PIO\n");
        printf("\t\tcommands to ATA device when possible. This option can\n");
        printf("\t\tbe combined with --%s\n\n", FORCE_ATA_LONG_OPT_STRING);
    }
}

void print_Force_ATA_DMA_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", FORCE_ATA_DMA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUsing this option will force the tool to issue SAT\n");
        printf("\t\tcommands to ATA device using the protocol set to DMA\n");
        printf("\t\twhenever possible (on DMA commands).\n");
        printf("\t\tThis option can be combined with --%s\n\n", FORCE_ATA_LONG_OPT_STRING);
    }
}

void print_Force_ATA_UDMA_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", FORCE_ATA_UDMA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUsing this option will force the tool to issue SAT\n");
        printf("\t\tcommands to ATA device using the protocol set to UDMA\n");
        printf("\t\twhenever possible (on DMA commands).\n");
        printf("\t\tThis option can be combined with --%s\n\n", FORCE_ATA_LONG_OPT_STRING);
    }
}

void print_Display_LBA_Help(bool shortHelp)
{
    printf("\t--%s [LBA]\n", DISPLAY_LBA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will read and display the contents of\n");
        printf("\t\tthe specified LBA to the screen. The display format\n");
        printf("\t\tis hexadecimal with an ASCII translation on the side\n");
        printf("\t\t(when available).\n\n");
    }
}

void print_Pattern_Help(bool shortHelp)
{
    printf("\t--%s [repeat:asciinospaces | random | increment:startValue | file:filename]\n", PATTERN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with overwrite, sanitize, and format unit\n");
        printf("\t\toperations to write a specific pattern to a range of LBAs\n");
        printf("\t\tor the whole drive.\n\n");
        printf("\t\t* repeat - without spaces, enter an ASCII text string or a\n");
        printf("\t\thexadecimal string terminated by a lower case \"h\". This\n");
        printf("\t\tpattern will be repeated until it fills the logical size\n");
        printf("\t\tof the LBA. i.e. helloword or FFFFFFFFh\n");
        printf("\t\tNote: A hexadecimal pattern will be interpreted as a 32bit\n");
        printf("\t\tunsigned integer. 4 hex bytes (8 characters) must be given\n");
        printf("\t\tfor a hex value to be used. Ex: 1F037AC8h or 0000FFFFh\n");
        printf("\t\t* random - the entire logical sector size will be filled with\n");
        printf("\t\trandom bytes.This pattern will be written to all LBAs in the\n");
        printf("\t\tdesired range.\n");
        printf("\t\t* increment - enter the starting numerical value. Starting with\n");
        printf("\t\tthis value, each byte will be written with 1 + previous value.\n");
        printf("\t\t* file - user supplied file name to use for a pattern. The file\n");
        printf("\t\twill be truncated or padded with zeros to the logical sector size\n");
        printf("\t\tNote 1: Each file will be interpreted as a binary file.\n");
        printf("\t\tNote 2: A path must also be provided if the file is not in the\n");
        printf("\t\t        local directory.\n");
        printf("\t\tNote 3: Sanitize Overwrite on SATA only supports a 32bit pattern.\n");
        printf("\t\t        The file option will get truncated to a 32bit pattern for\n");
        printf("\t\t        SATA products.\n\n");
    }
}

void print_Device_Statistics_Help(bool shortHelp)
{
    printf("\t--%s\n", DEVICE_STATISTICS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to display the device statistics reported\n");
        printf("\t\tby the device. On SATA, this uses the Device Statistics\n");
        printf("\t\tlog, and the notifications log (if DSN feature is supported)\n");
        printf("\t\tto display these statistics. On SAS, various log pages are\n");
        printf("\t\tread to collect a bunch of reported parameter information.\n\n");
    }
}

void print_Zone_ID_Help(bool shortHelp)
{
    printf("\t--%s [LBA/zone ID | all]\n", ZONE_ID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a zone ID for use with other options\n");
        printf("\t\tsuch as --%s, --%s, --%s, --%s, --%s.", REPORT_ZONES_LONG_OPT_STRING, CLOSE_ZONE_LONG_OPT_STRING, FINISH_ZONE_LONG_OPT_STRING, OPEN_ZONE_LONG_OPT_STRING, RESET_WP_LONG_OPT_STRING);
        printf("\t\tA zone ID is an LBA at the start of a specified zone.\n");
        printf("\t\tUse \"all\" with the options listed above to apply an action\n");
        printf("\t\tto all zones on the device.\n\n");
    }
}

void print_Max_Zones_Help(bool shortHelp)
{
    printf("\t--%s [count]\n", MAX_ZONES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set a maximum number of zones to display with\n");
        printf("\t\tthe --%s option.\n\n", REPORT_ZONES_LONG_OPT_STRING);
    }
}

void print_Report_Zones_Help(bool shortHelp)
{
    printf("\t--%s [all | empty | implicitOpen | explicitOpen | closed | \n", REPORT_ZONES_LONG_OPT_STRING);
    printf("\t          full | readOnly | offline | resetRecommended | nonSeqResourceAvailable | allNonWP]\n");
    if (!shortHelp)
    {
        printf("\t\tUse this option to display the zones of a specific type, where they start,\n");
        printf("\t\tthe length of the zones, and where the write pointer is at.\n");
        printf("\t\tCombine this option with --%s and --%s\n", ZONE_ID_LONG_OPT_STRING, MAX_ZONES_LONG_OPT_STRING);
        printf("\t\tTo show a different list/subset of the zones on the drive.\n");
        printf("\t\tEx: --%s empty --%s 123456 --%s 30\n", REPORT_ZONES_LONG_OPT_STRING, ZONE_ID_LONG_OPT_STRING, MAX_ZONES_LONG_OPT_STRING);
        printf("\t\t*all - show all zones\n");
        printf("\t\t*empty - show only empty zones\n");
        printf("\t\t*implicitOpen - show only implicitly opened zones\n");
        printf("\t\t*explicitOpen - show only explicitly opened zones\n");
        printf("\t\t*closed - show only closed zones\n");
        printf("\t\t*full - show only full zones\n");
        printf("\t\t*readOnly - show only read only zones\n");
        printf("\t\t*offline - show only offline zones\n");
        printf("\t\t*resetRecommended - show only zones that have the reset recommended bit set\n");
        printf("\t\t*nonSeqResourceAvailable - show only zones with a non sequential access resource available\n");
        printf("\t\t*allNonWp - show all non-write pointer zones.\n\n");
    }
}

void print_Close_Zone_Help(bool shortHelp)
{
    printf("\t--%s\n", CLOSE_ZONE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to close a zone specified with the --%s option\n\n", ZONE_ID_LONG_OPT_STRING);
    }
}

void print_Finish_Zone_Help(bool shortHelp)
{
    printf("\t--%s\n", FINISH_ZONE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to finish a zone specified with the --%s option\n\n", ZONE_ID_LONG_OPT_STRING);
    }
}

void print_Open_Zone_Help(bool shortHelp)
{
    printf("\t--%s\n", OPEN_ZONE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to open a zone specified with the --%s option\n\n", ZONE_ID_LONG_OPT_STRING);
    }
}

void print_Reset_Write_Pointer_Zone_Help(bool shortHelp)
{
    printf("\t--%s\n", RESET_WP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to reset a write pointer at the zone specified with the --%s option\n\n", ZONE_ID_LONG_OPT_STRING);
    }
}

void print_FWDL_Segment_Size_Help(bool shortHelp)
{
    printf("\t--%s [segment size in 512B blocks]\t\n", FWDL_SEGMENT_SIZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a segment size in 512B blocks\n");
        printf("\t\tto use for a segmented or deferred download. This option\n");
        printf("\t\twill not affect an immediate download (full buffer at once).\n");
        printf("\t\tThe default segment size used is 64. Larger segment sizes\n");
        printf("\t\tmay be faster, however they may also be incompatible with\n");
        printf("\t\tcontrollers or drivers in the system. Smaller values are\n");
        printf("\t\tmore likely to be compatible, but also slower.\n");
        printf("\t\tUse this option if the default used by the tool is not\n");
        printf("\t\tworking correctly for firmware updates.\n\n");
    }
}

void print_show_FWDL_Support_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_FWDL_SUPPORT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show the firmware download support\n");
        printf("\t\tinformation for a drive.\n\n");
    }
}

void print_Error_In_Cmd_Line_Args(const char * optstring, const char * arg)
{
    printf("\nError in option --%s. Invalid argument given '%s'.\n", optstring, arg);
    printf("Use -h option to view command line help\n");
}

void print_Enable_Legacy_USB_Passthrough_Help(bool shortHelp)
{
    printf("\t--%s\n", ENABLE_LEGACY_PASSTHROUGH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tOnly use this option on old USB or IEEE1394 (Firewire)\n");
        printf("\t\tproducts that do not otherwise work with the tool.\n");
        printf("\t\tThis option will enable a trial and error method that\n");
        printf("\t\tattempts sending various ATA Identify commands through\n");
        printf("\t\tvendor specific means. Because of this, certain products\n");
        printf("\t\tthat may respond in unintended ways since they may interpret\n");
        printf("\t\tthese commands differently than the bridge chip the command\n");
        printf("\t\twas designed for.\n\n");
    }
}

//Yes, this function uses a tripple pointer and is complicated.
//The idea is there is no limit to a number of device handles given on a command line.
//Because there is no limit, we need to allocate/reallocate heap memory to add more handles (as strings) to the list.
//In order to have a list of strings, I had to use a double pointer from the caller and pass an address to this 2D array, hence the tripple pointer.
//I do not recommend touching this function unless you have to.
//It should be capable of accepting any type of handle input to it. Windows is likely the only OS to need changes (see comment in code below).
int parse_Device_Handle_Argument(char * optarg, bool *allDrives, bool *userHandleProvided, uint32_t *deviceCount, char ***handleList)
{
    if (!optarg || !allDrives || !userHandleProvided || !deviceCount)
    {
        return 254;//one of the required parameters is missing. handleList is checked below...
    }
    /*get the number out of optarg to tack onto the device handle*/
    if (NULL != optarg)
    {
        if (strcmp(optarg, "all") == 0)
        {
            /*this is a request to run on all drives.*/
            *allDrives = true;
        }
        else
        {
            *userHandleProvided = true;
#if defined(_WIN32)
            char windowsHandle[50] = { 0 };
            char *deviceHandle = &windowsHandle[0];
            char *physicalDeviceNumber; /*making this a string in case the handle is two or more digits long*/
            /*make sure the user gave us "PD" for the device handle...*/
            if (_strnicmp((char *)optarg, "PD", 2) == 0)
            {
                physicalDeviceNumber = strpbrk((char *)optarg, "0123456789");
                sprintf(deviceHandle, "\\\\.\\PhysicalDrive%s", physicalDeviceNumber);
            }
#if defined(ENABLE_CSMI)
            else if (strncmp((char *)optarg, "SCSI", 4) == 0)
            {
                sprintf(deviceHandle, "\\\\.\\%s", optarg);
            }
#endif
            else if (strncmp((char *)optarg, "\\\\.\\", 4) == 0)
            {
                sprintf(deviceHandle, optarg);
            }
            /*If we want to add another format for accepting a handle, then add an else-if here*/
            else /*we have an invalid handle*/
            {
                printf("Error: %s is an invalid handle format for this tool.\n", optarg);
                exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
            }
#else
            char *deviceHandle = optarg;
#endif

            ++(*deviceCount);/*increment this variable if we've made it this far.*/
            /*The code below is where this function gets complicated. Be very careful changing anything below this comment.*/
            if (!*handleList)
            {
                /*allocate the list and add this handle to it.*/
                *handleList = (char**)calloc((*deviceCount), sizeof(char*));
                if (!*handleList)
                {
                    perror("error allocating memory for handle list\n");
                    return 255;
                }
            }
            else
            {
                /*list already allocated, so reallocate and add this next handle to it.*/
                char **temp = (char**)realloc(*handleList, (*deviceCount) * sizeof(char*));
                if (!temp)
                {
                    perror("error reallocating memory for handle list\n");
                    return 255;
                }
                *handleList = temp;
            }
            /*the list has been allocated, now put the handle we've received into the list*/
            /*start by allocating memory for the handle at the new list location*/
            (*handleList)[(*deviceCount) - 1] = (char*)calloc(strlen(deviceHandle) + 1 * sizeof(char), sizeof(char));
            if (!(*handleList)[(*deviceCount) - 1])
            {
                perror("error allocating memory for adding device handle to list\n");
                return 255;
            }
            /*copy the handle into memory*/
            strcpy((*handleList)[(*deviceCount) - 1], deviceHandle);
        }
    }
    return 0;
}

void free_Handle_List(char ***handleList, uint32_t listCount)
{
    if (*handleList)
    {
        for (uint32_t handleIter = 0; handleIter < listCount; ++handleIter)
        {
            safe_Free((*handleList)[handleIter]);
        }
        safe_Free((*handleList));
    }
}

#if defined (ENABLE_CSMI)
void print_CSMI_Info_Help(bool shortHelp)
{
    printf("\t--%s\n", CSMI_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show some information reported by the\n");
        printf("\t\tCSMI compliant driver. This includes driver information,\n");
        printf("\t\tcontroller information, PHY information, SAS address, and\n");
        printf("\t\tSATA signature as they were reported by the driver. This\n");
        printf("\t\tinformation can be useful when troubleshooting CSMI\n");
        printf("\t\tsystem/driver compatibility issues.\n\n");
    }
}

void print_CSMI_Verbose_Help(bool shortHelp)
{
    printf("\t--%s\n", CSMI_VERBOSE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show some verbose output when running the\n");
        printf("\t\ttool on a CSMI handle. The debugging information shown will be\n");
        printf("\t\tspecific to the CSMI passthrough mechanism and may be useful\n");
        printf("\t\twhen troubleshooting system/driver compatibility issues.\n\n");
    }
}

void print_CSMI_Force_Flags_Help(bool shortHelp)
{
    printf("\t--%s\n", CSMI_FORCE_IGNORE_PORT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to force setting the \"ignore Port\" flag\n");
        printf("\t\tfor the port identifier in a CSMI passthrough command.\n");
        printf("\t\tThis option can be combined with --%s which\n", CSMI_FORCE_USE_PORT_LONG_OPT_STRING);
        printf("\t\twill force the passthrough to rely on only the SAS address.\n");
        printf("\t\tThis flag is intended to help troubleshoot or improve CSMI\n");
        printf("\t\tcompatibility on systems that are otherwise not functional.\n\n");
    }

    printf("\t--%s\n", CSMI_FORCE_USE_PORT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to force setting the \"Use Port\" flag\n");
        printf("\t\tfor the PHY identifier in a CSMI passthrough command.\n");
        printf("\t\tThis option can be combined with --%s which\n", CSMI_FORCE_IGNORE_PORT_LONG_OPT_STRING);
        printf("\t\twill force the passthrough to rely on only the SAS address.\n");
        printf("\t\tThis flag is intended to help troubleshoot or improve CSMI\n");
        printf("\t\tcompatibility on systems that are otherwise not functional.\n\n");
    }
}
#endif

void print_OD_MD_ID_Test_Help(bool shortHelp)
{
    printf("\t--%s [O | M | I]\n", OD_MD_ID_TEST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to perform a generic read/write/verify\n");
        printf("\t\ttest at the specified diameter of the drive.\n");
        printf("\t\tUse the time options to specify a time based test or\n");
        printf("\t\tthe --%s option for a range based test.\n", OD_MD_ID_TEST_RANGE_LONG_OPT_STRING);
        printf("\t\t    O - outer diameter\n");
        printf("\t\t    M - middle diameter\n");
        printf("\t\t    I - inner diameter\n");
        printf("\t\tThe different diameters can be combined or run individually.\n");
        printf("\t\tEx1: --%s OMI\n", OD_MD_ID_TEST_LONG_OPT_STRING);
        printf("\t\tEx2: --%s O\n", OD_MD_ID_TEST_LONG_OPT_STRING);
        printf("\t\tEx3: --%s MI\n\n", OD_MD_ID_TEST_LONG_OPT_STRING);
        printf("\t\tInner, middle, and outer diameter tests refer to the physical\n");
        printf("\t\tbeginning and ending sections of a hard disk drive with\n");
        printf("\t\trotating magnetic media.In the case of SSD devices,\n");
        printf("\t\tthese tests refer to the logical beginning and ending\n");
        printf("\t\tsections of the solid state drive.\n\n");
    }
}

void print_OD_MD_ID_Test_Range_Help(bool shortHelp)
{
    printf("\t--%s [range]\n", OD_MD_ID_TEST_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with the --%s option to\n", OD_MD_ID_TEST_LONG_OPT_STRING);
        printf("\t\tperform a range based test. If a range is\n");
        printf("\t\tspecified without any units, it is assumed\n");
        printf("\t\tto be an LBA count.\n");
        printf("\t\tValid units are KB, KiB, MB, MiB, GB, GiB, TB\n");
        printf("\t\tand TiB.\n");
        printf("\t\tEx1: \"--%s 1234567\" for an LBA count\n", OD_MD_ID_TEST_RANGE_LONG_OPT_STRING);
        printf("\t\tEx2: \"--%s 2GB\" for a 2GB range.\n\n", OD_MD_ID_TEST_RANGE_LONG_OPT_STRING);
    }
}

void print_Hide_LBA_Counter_Help(bool shortHelp)
{
    printf("\t--%s\n", HIDE_LBA_COUNTER_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to suppress the output from\n");
        printf("\t\toptions that show LBA counters without turning\n");
        printf("\t\toff all output to the screen.\n\n");
    }
}

void print_Show_Physical_Element_Status_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to see the status/health of\n");
        printf("\t\tthe storage elements inside a drive.\n");
        printf("\t\tUse the element # shown with the --%s\n", REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING);
        printf("\t\toption to remove that storage element from use.\n");
        printf("\t\tThis option can also be used to see if a depopulation\n");
        printf("\t\tis still in progress or if it has completed.\n\n");
    }
}

void print_Remove_Physical_Element_Status_Help(bool shortHelp)
{
    printf("\t--%s [element #]\n", REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to remove a storage element\n");
        printf("\t\tfrom use on a drive. When this is done, the\n");
        printf("\t\tdrive will erase all user data and lower the\n");
        printf("\t\tcapacity to a new point where the drive is still\n");
        printf("\t\tusable without the provided element #.\n");
        printf("\t\tUse the --%s option to see the status\n", SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT_STRING);
        printf("\t\tof the depopulation operation.\n\n");
    }
}

void print_Force_Seagate_Depop_Help(bool shortHelp)
{
    printf("\t--%s \n", FORCE_SEAGATE_DEPOPULATE_COMMANDS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to force using Seagate's\n");
        printf("\t\tvendor unique method of seeing and removing\n");
        printf("\t\tstorage elements from use. This is a legacy\n");
        printf("\t\tmethod and the default mode is to use the\n");
        printf("\t\tACS4 or SBC4 methods when available.\n\n");
    }
}

void print_Show_Locked_Regions_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_LOCKED_REGIONS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show the locked ranges or bands\n");
        printf("\t\ton TCG Opal or TCG Enterprise SED drives.\n\n");
    }
}

void print_Seagate_Power_Balance_Help(bool shortHelp)
{
    printf("\t--%s [ info | enable | disable ]\t (Seagate Only)\n", SEAGATE_POWER_BALANCE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to see the state of the Seagate Power Balance\n");
        printf("\t\tfeature or to change its state.\n");
        printf("\t\tSeagate's PowerBalance feature will adjust drive performance during\n");
        printf("\t\trandom operations to reduce power consumption of the drive.\n");
        printf("\t\t  info - will dump the state of the Power Balance feature on the screen\n");
        printf("\t\t  enable - use this to enable Power Balance\n");
        printf("\t\t  disable - use this to disable Power Balance\n");
        printf("\t\tNote: While this feature is available on some SAS products,\n");
        printf("\t\tit is recommended that the --%s option is\n", SET_POWER_CONSUMPTION_LONG_OPT_STRING);
        printf("\t\tused instead since it allows more levels of control.\n");
        printf("\t\tThis option and the --%s option are incompatible\n", SET_POWER_CONSUMPTION_LONG_OPT_STRING);
        printf("\t\tbecause they use the same mode page fields (1Ah-01h).\n\n");
    }
}

void print_DIPM_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable]\t(SATA Only)\n", SATA_DIPM_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable the SATA Device Initiated\n");
        printf("\t\tPower Management (DIPM) feature. Use the \"info\" option to see\n");
        printf("\t\tthe current state of the DIPM feature on the device.\n");
        printf("\t\tNOTE: Please ensure that the host adapter/controller/driver can\n");
        printf("\t\thandle this before enabling it, otherwise the drive link may\n");
        printf("\t\tgo down and the device will not be able to communicate.\n\n");
    }
}

void print_DAPS_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable]\t(SATA Only)\n", SATA_DAPS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable the SATA Device Automatic\n");
        printf("\t\tPartial To Slumber Transitions (DAPS) feature. Use the \"info\"\n");
        printf("\t\toption to see the current state of the DIPM feature on the device.\n");
        printf("\t\tThe use of this feature requires that the DIPM feature is enabled.\n");
        printf("\t\tNOTE: Please ensure that the host adapter/controller/driver can\n");
        printf("\t\thandle this before enabling it, otherwise the drive link may\n");
        printf("\t\tgo down and the device will not be able to communicate.\n\n");
    }
}

void print_Free_Fall_Help(bool shortHelp)
{
    printf("\t--%s [info | enable | disable | sensitivity value]\t(SATA only)\n", FREE_FALL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to configure the Free Fall control feature\n");
        printf("\t\tfound on some SATA drives. This feature allows the drive to\n");
        printf("\t\ttake action if it detects it is in free fall to protect the data\n");
        printf("\t\tfrom harm due to a drop.\n");
        printf("\t\t    info - use this to see the current sensitivity value\n");
        printf("\t\t    enable - this option will set the sensitivity to the vendor's\n");
        printf("\t\t             recommended value.\n");
        printf("\t\t    disable - this will disable the free fall control feature.\n");
        printf("\t\t    sensitivity value - set a value between 1 and 255 to control\n");
        printf("\t\t                        how sensitive the detection is. A value of zero\n");
        printf("\t\t                        will set the vendor's recommended value.\n\n");
    }
}

void print_SCSI_Defects_Help(bool shortHelp)
{
    printf("\t--%s [ p | g | pg ]\t(SAS Only)\n", SCSI_DEFECTS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will display the SCSI defects on the screen.\n");
        printf("\t\tThe arguments to this will tell whether to get the grown, \n");
        printf("\t\tprimary, or primary and grown defects from the drive.\n");
        printf("\t\tUse the --%s option to specify the mode to display the defects.\n", SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING);
        printf("\t\tIf no mode is specified, physical cylinder-head-sector mode is assumed\n");
        printf("\t\tArguments:\n");
        printf("\t\t  p - use this option to pull and display the primary (factory) defects\n");
        printf("\t\t  g - use this option to pull and display the grown (reallocated) defects\n");
        printf("\t\t The above options can be combined to pull and display both defect lists.\n\n");
    }
}

void print_SCSI_Defects_Format_Help(bool shortHelp)
{
    printf("\t--%s [ # | shortBlock | longBlock | xbfi | xchs | bfi | chs ]\t(SAS Only)\n", SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option set the format of the defects to output.\n");
        printf("\t\tNot all drives will support all defect modes!\n");
        printf("\t\tSSDs will only support block modes!\n");
        printf("\t\tArguments: (name | #)\n");
        printf("\t\t shortBlock | 0 - show the defects in short block address mode (drives < 32bit LBA)\n");
        printf("\t\t xbfi       | 1 - show the defects in extended bytes from index mode\n");
        printf("\t\t xchs       | 2 - show the defects in extended physical cylinder-head-sector mode\n");
        printf("\t\t longBlock  | 3 - show the defects in long block address mode (drives > 32bit LBA)\n");
        printf("\t\t bfi        | 4 - show the defects in bytes from index mode\n");
        printf("\t\t chs        | 5 - show the defects in physical cylinder-head-sector mode\n");
        printf("\n");
    }
}

void print_Pull_Self_Test_Results_Log_Help(bool shortHelp)
{
    printf("\t--%s\n", DST_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will pull the self test results log\n");
        printf("\t\tfrom a device. On ATA drives, this will pull the\n");
        printf("\t\textended SMART self tests result log when it is\n");
        printf("\t\tsupported by the device.\n\n");
    }
}

void print_Pull_Identify_Device_Data_Log_Help(bool shortHelp)
{
    printf("\t--%s\t\t(SATA only)\n", IDENTIFY_DEVICE_DATA_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will pull the Identify Device data\n");
        printf("\t\tlog from an ATA drive.\n\n");
    }
}

void print_Pull_SATA_Phy_Event_Counters_Log_Help(bool shortHelp)
{
    printf("\t--%s\t\t\t(SATA only)\n", SATA_PHY_COUNTERS_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will pull the SATA Phy Event Counters\n");
        printf("\t\tlog from a SATA drive.\n\n");
    }
}

void print_Pull_Device_Statistics_Log_Help(bool shortHelp)
{
    printf("\t--%s\n", DEVICE_STATS_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will pull the Device Statistics Log\n");
        printf("\t\tfrom a device.\n\n");
    }
}

void print_Pull_Informational_Exceptions_Log_Help(bool shortHelp)
{
    printf("\t--%s\t\t\t(SAS only)\n", INFORMATIONAL_EXCEPTIONS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will pull the SCSI Informational\n");
        printf("\t\tExceptions log page from a SCSI device.\n\n");
    }
}

void print_Log_Transfer_Length_Help(bool shortHelp)
{
    printf("\t--%s [length in bytes]\n", LOG_TRANSFER_LENGTH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the data transfer\n");
        printf("\t\tlength for a log transfer.\n");
        printf("\t\tLarger transfer sizes may speed up log retrieval at the\n");
        printf("\t\tloss of compatibility.\n");
        printf("\t\tThe following post fixes are allowed for\n");
        printf("\t\tspecifying a transfer length:\n");
        printf("\t\t\tBLOCKS or SECTORS - used to specify a transfer length\n");
        printf("\t\t\t\tin device in 512Byte blocks/sectors\n");
        printf("\t\t\tKB - length in kilobytes (val * 1000)\n");
        printf("\t\t\tKiB - length in kibibytes (val * 1024)\n");
        printf("\t\t\tMB - length in megabytes (val * 1000000)\n");
        printf("\t\t\tMiB - length in mebibytes (val * 1048576)\n");
        printf("\t\tATA drives must be given a value in 512B increments.\n");
        printf("\t\tWarning: Specifying a large size may result in\n");
        printf("\t\tfailures due to OS, driver, or HBA/bridge specific limitations.\n\n");
    }
}

void print_FARM_Log_Help(bool shortHelp)
{
    printf("\t--%s\n", FARM_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tPull the Seagate FARM log from the specified drive.\n\n");
    }
}

void print_Show_SMART_Error_Log_Help(bool shortHelp)
{
    printf("\t--%s [ summary | comprehensive ]\t(SATA Only)\n", SHOW_SMART_ERROR_LOG_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will display the ATA SMART Error log on the screen.\n");
        printf("\t\tUse \"summary\" to view the summary SMART error log (last 5 entries)\n");
        printf("\t\tUse \"comprehensive\" to view all the entires the drive has available.\n");
        printf("\t\tSpecifying \"comprehensive\" will automatically pull the ext error log\n");
        printf("\t\ton drives that support 48bit LBAs.\n");
        printf("\t\tNote: The summary error log will truncate 48bit commands, so some information\n");
        printf("\t\t      will be missing to desribe the operation of certain commands.\n");
        printf("\n");
    }
}

void print_SMART_Error_Log_Format_Help(bool shortHelp)
{
    printf("\t--%s [ raw | detailed ]\t(SATA Only)\n", SMART_ERROR_LOG_FORMAT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to change the format of the output from the --%s\n", SHOW_SMART_ERROR_LOG_LONG_OPT_STRING);
        printf("\t\toption. The default mode is \"detailed\"\n");
        printf("\n");
    }
}

void print_FWDL_Allow_Flexible_Win10_API_Use_Help(bool shortHelp)
{
    printf("\t--%s\n", WIN10_FLEXIBLE_API_USE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to control when to use the Windows 10+\n");
        printf("\t\tFirmware Download API calls. Default behavior is to only\n");
        printf("\t\tuse this call when the interface and drive and command all\n");
        printf("\t\tmatch. The default behavior means that only ATA drives on\n");
        printf("\t\tan ATA interface with a ATA download command will use this call\n");
        printf("\t\tSCSI drives with a supported write buffer command will also use\n");
        printf("\t\tthis call.\n");
        printf("\t\tUsing this option allows a detected ATA drive on any interface to\n");
        printf("\t\tuse this call if the OS/driver supports it regardless of the command\n");
        printf("\t\tbeing sent by the opensea-transport library.\n\n");
    }
}

void print_FWDL_Force_Win_Passthrough_Help(bool shortHelp)
{
    printf("\t--%s\n", WIN10_FWDL_FORCE_PT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to control when to use the Windows 10+\n");
        printf("\t\tFirmware Download API calls. This option will force using\n");
        printf("\t\tATA or SCSI passthrough to issue the FWDL commands instead of\n");
        printf("\t\tautomatically selecting passthrough or the Win10 API.\n");
        printf("\t\tIt is strongly recommended that this option only be used when\n");
        printf("\t\ttroubleshooting problems when updating firmware as the Win10 API allows\n");
        printf("\t\tfor handling when device firmware changes versions where this is not\n");
        printf("\t\tpossible with passthrough commands.\n\n");
    }
}

void print_ATA_Security_Erase_Help(bool shortHelp, const char *password)
{
    printf("\t--%s [normal | enhanced]\t\t(SATA only)\n", ATA_SECURITY_ERASE_OP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse \"normal\" to start a standard ATA security erase\n");
        printf("\t\tor \"enhanced\" to start an enhanced ATA security erase.\n\n");
        printf("\t\tATA Security Erase takes a very long time to complete at\n");
        printf("\t\tapproximately three (3) hours per Terabyte (HDD). Some Seagate\n");
        printf("\t\tSED models will perform a quick cryptographic erase in enhanced\n");
        printf("\t\tmode and the time for completion is reported as 2 minutes by\n");
        printf("\t\tthe drive, but will take only seconds. This industry\n");
        printf("\t\tstandard command begins by locking the drive with a temporary\n");
        printf("\t\tpassword which is cleared at the end of the erasure. Do not run\n");
        printf("\t\tthis command unless you have ample time to allow it to run\n");
        printf("\t\tthrough to the end. If the procedure is interrupted prior to\n");
        printf("\t\tcompletion, then the drive will remain in a locked state and\n");
        printf("\t\tyou must manually restart from the beginning again. The\n");
        printf("\t\ttool will attempt to automatically clear the password that was set\n");
        printf("\t\tupon failure. The default password used by the tool is\n");
        printf("\t\t\"%s\", plain ASCII letters without the quotes\n\n", password);
        printf("\t\t* normal writes binary zeroes (0) or ones (1) to all user\n");
        printf("\t\tdata areas.\n\n");
        printf("\t\t* enhanced will fill all user data areas and reallocated\n");
        printf("\t\tuser data with a vendor specific pattern. Some Seagate\n");
        printf("\t\tInstant Secure Erase will perform a cryptographic\n");
        printf("\t\terase instead of an overwrite.\n\n");
    }
}

void print_Disable_ATA_Security_Password_Help(bool shortHelp, const char *utilName)
{
    printf("\t--%s\t\t(SATA Only)\n", ATA_SECURITY_DISABLE_OP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to disable an ATA security password.\n");
        printf("\t\tIf the drive is in high security mode, either user or\n");
        printf("\t\tmaster password may be provided. In maximum security mode\n");
        printf("\t\tonly the user password can be provided to unlock and disable the\n");
        printf("\t\tATA security password. The master may only be used to erase the drive\n");
        printf("\t\tin maximum security mode.\n");
        printf("\t\tUse the --%s option to provide the password to use and\n", ATA_SECURITY_PASSWORD_LONG_OPT_STRING);
        printf("\t\t--%s to specify whether it is the user or master password.\n", ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
        printf("\t\tIf a drive lost power during an ATA Security Erase in\n");
        printf("\t\t%s, then providing --%s SeaChest\n", utilName, ATA_SECURITY_PASSWORD_LONG_OPT_STRING);
        printf("\t\twill use the default SeaChest password used during the erase.\n");
        printf("\t\t To disable a password set by a BIOS, the BIOS must have set the\n");
        printf("\t\tpassword in ASCII. A BIOS may choose to hash or modify the\n");
        printf("\t\tpassword typed in the configuration however it\n");
        printf("\t\tchooses and this utility has no idea how to match what\n");
        printf("\t\tthe BIOS has done so it may not always work to remove\n");
        printf("\t\ta password set by something other than this utility.\n\n");
    }
}

void print_ATA_Security_Password_Modifications_Help(bool shortHelp)
{
    printf("\t--%s [", ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING);
    #if defined MD5_PASSWORD_SUPPORTED
    printf("md5 | ");
    #endif
    printf("byteswapped | zeropad | spacepad | fpad | leftAlign | rightAlign | uppercase | lowercase | invertcase] (SATA Only)\n");
    if (!shortHelp)
    {
        printf("\t\tUse this option to have the utility make modifications to\n");
        printf("\t\tthe ATA security password to attempt other various ways it may\n");
        printf("\t\tbe sent by a system bios. These are not guaranteed to work, but\n");
        printf("\t\tmay help unlock a drive that was locked by a BIOS that encoded\n");
        printf("\t\tthe password in a unique way.\n");
        printf("\t\tThis option can be presented multiple times to select multiple modificaitons.\n");
        printf("\t\tEX: --%s byteswapped --%s invertcase\n", ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING, ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING);
        #if defined MD5_PASSWORD_SUPPORTED
        printf("\t\t  md5 - sends a md5 sum of the password as the password\n");//Internal SeaChest only since this requires mbedtls library!!!
        #endif
        printf("\t\t  byteswapped - byteswaps the password. EX: blah -> lbha\n");
        printf("\t\t  zeropad - zero pads the password if less than 32 characters\n");
        printf("\t\t  spacepad - space pads the password if less than 32 characters\n");
        printf("\t\t  fpad - pads the passwords with Fh (all 1's) if less than 32characters\n");
        printf("\t\t  leftAlign - left aligns the password in the buffer\n");
        printf("\t\t  rightAlign - right aligns the password in the buffer\n");
        printf("\t\t  uppercase - sends the password as all uppercase\n");
        printf("\t\t  lowercase - sends the password as all lowercase\n");
        printf("\t\t  invertcase - switches uppercase for lower, and lowercase for upper\n");
        printf("\n");
    }
}

void print_ATA_Security_Password_Help(bool shortHelp)
{
    printf("\t--%s [\"ASCII password\" | SeaChest | empty]\t\t(SATA only)\n", ATA_SECURITY_PASSWORD_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a password to use with an ATA security\n");
        printf("\t\toperation. If specifying a password with spaces, quotes must be used.\n");
        printf("\t\tIf SeaChest is given, the default SeaChest password will be used.\n");
        printf("\t\tIf empty is given, an empty password will be used.\n");
        printf("\t\tExamples:\n");
        printf("\t\t  \"This is a valid password\"\n");
        printf("\t\t  ThisIsAlsoValid\n");
        printf("\t\t  \"This password uses \\\"quotes\\\"\n");
        printf("\t\t  \"This password is \\/\\/eird\"\n");
        #if defined (_WIN32)
        printf("\t\tIn Windows PE, MS will allow issuing secure erase using the following\n");
        printf("\t\tas a USER password: AutoATAWindowsString12345678901\n");
        #endif
        printf("\n");
    }
}
void print_ATA_Security_Password_Type_Help(bool shortHelp)
{
    printf("\t--%s [user | master]\t\t(SATA only)\n", ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify if the password being given with the\n");
        printf("\t\t--%s option is a user or a master password.\n", ATA_SECURITY_PASSWORD_LONG_OPT_STRING);
        printf("\t\tIf this option is not provided, user is assumed.\n\n");
    }
}
void print_ATA_Security_Master_Password_Capability_Help(bool shortHelp)
{
    printf("\t--%s [high | maximum]\t\t(SATA only)\n", ATA_SECURITY_MASTER_PW_CAPABILITY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the master password capability when setting\n");
        printf("\t\tan ATA security user password. In high security mode, either password\n");
        printf("\t\tmay be used to unlock and disable the password on a drive.\n");
        printf("\t\tIn maximum security mode, the master password can only be used to erase\n");
        printf("\t\tthe drive for repurposing it.\n");
        printf("\t\tIf this option is not provided, high security is assumed.\n\n");
    }
}

void print_ATA_Security_Master_Password_ID_Help(bool shortHelp)
{
    printf("\t--%s [numberic ID]\t\t(SATA only)\n", ATA_SECURITY_MASTER_PW_ID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option when setting a master password to also set this value\n");
        printf("\t\tas a numeric hint to remember/recover the drive using the master password.\n\n");
    }
}

void print_ATA_Security_Force_SAT_Security_Protocol_Help(bool shortHelp)
{
    printf("\t--%s [enable | disable]\t\t(SATA only)\n", ATA_SECURITY_FORCE_SAT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option can be used to force enable or disable using the\n");
        printf("\t\tATA security protocol as specified in the SAT specification.\n");
        printf("\t\tBy default, the tool will use this method when it is supported \n");
        printf("\t\tto allow the SATL to understand and manage the security commands\n");
        printf("\t\tbeing performed and prevent other issues.\n\n");
    }
}

void print_ATA_Security_Set_Password_Help(bool shortHelp)
{
    printf("\t--%s\t\t(SATA only)\n", ATA_SECURITY_SET_PASSWORD_OP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option along with the --%s option and\n", ATA_SECURITY_PASSWORD_LONG_OPT_STRING);
        printf("\t\t--%s option to set an ATA security password on a drive.\n", ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
        printf("\t\tIf setting the master password, it is strongly recommended\n");
        printf("\t\tthat a master password identifier is specified with the\n");
        printf("\t\t--%s option as well.\n", ATA_SECURITY_MASTER_PW_ID_LONG_OPT_STRING);
        printf("\t\tThe --%s option can be provided to set the drive to\n", ATA_SECURITY_MASTER_PW_CAPABILITY_LONG_OPT_STRING);
        printf("\t\tmaximum security mode when setting the user password.\n\n");
    }
}

void print_ATA_Security_Unlock_Help(bool shortHelp)
{
    printf("\t--%s\t\t(SATA only)\n", ATA_SECURITY_UNLOCK_OP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option along with the --%s option and\n", ATA_SECURITY_PASSWORD_LONG_OPT_STRING);
        printf("\t\t--%s option to unlock a drive with the provided password.\n", ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
        printf("\t\tIf the drive is in maximum security mode, only the user password\n");
        printf("\t\tmay be used to unlock the device.\n\n");
    }
}
void print_ATA_Security_Freezelock_Help(bool shortHelp)
{
    printf("\t--%s\t\t(SATA only)\n", ATA_SECURITY_FREEZELOCK_OP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will send the ATA security freezelock command to\n");
        printf("\t\ta device. This command prevents all other ATA security commands\n");
        printf("\t\tfrom being processed until the next reset or power cycle.\n\n");
    }
}

void print_ATA_Security_Info_Help(bool shortHelp)
{
    printf("\t--%s\t\t(SATA only)\n", ATA_SECURITY_INFO_OP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option shows information about the ATA security\n");
        printf("\t\tfeature on ATA devices. It will show the security state and\n");
        printf("\t\tflags related to the state, Master password capability & ID,\n");
        printf("\t\ttime to perform a secure erase, whether user data is encrypted,\n");
        printf("\t\tand whether sanitize can override ATA security to repurpose a drive.\n\n");
    }
}

void print_SCSI_MP_Reset_Help(bool shortHelp)
{
    printf("\t--%s [page# | page-subpage#]\t\t(SAS only)\n", SCSI_MP_RESET_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will reset the specified mode page(s) to their default\n");
        printf("\t\tsettings. Valid page numbers range from 0 to 3Fh. Valid subpage numbers\n");
        printf("\t\trange from 0 to FFh.\n");
        printf("\t\t(MP) Mode page 3Fh specifies all mode pages and can be used to reset all mode pages.\n");
        printf("\t\t(SP) Subpage FFH specifies all subpages of a given page and will reset all those subpages.\n");
        printf("\t\tUsing both MP 3Fh and SP FFh will reset all pages and subpages on a device.\n\n");
    }
}

void print_SCSI_MP_Restore_Help(bool shortHelp)
{
    printf("\t--%s [page# | page-subpage#]\t\t(SAS only)\n", SCSI_MP_RESTORE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will restore the specified mode page(s) to their saved\n");
        printf("\t\tsettings. Valid page numbers range from 0 to 3Fh. Valid subpage numbers\n");
        printf("\t\trange from 0 to FFh.\n");
        printf("\t\t(MP) Mode page 3Fh specifies all mode pages and can be used to restore all mode pages.\n");
        printf("\t\t(SP) Subpage FFH specifies all subpages of a given page and will restore all those subpages.\n");
        printf("\t\tUsing both MP 3Fh and SP FFh will restore all pages and subpages on a device.\n\n");
    }
}

void print_SCSI_MP_Save_Help(bool shortHelp)
{
    printf("\t--%s [page# | page-subpage#]\t\t(SAS only)\n", SCSI_MP_SAVE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will save the current specified mode page(s) to the saved\n");
        printf("\t\tsettings. Valid page numbers range from 0 to 3Fh. Valid subpage numbers\n");
        printf("\t\trange from 0 to FFh.\n");
        printf("\t\t(MP) Mode page 3Fh specifies all mode pages and can be used to save all mode pages.\n");
        printf("\t\t(SP) Subpage FFH specifies all subpages of a given page and will save all those subpages.\n");
        printf("\t\tUsing both MP 3Fh and SP FFh will save all pages and subpages on a device.\n\n");
    }
}

void print_SCSI_Show_MP_Help(bool shortHelp)
{
    printf("\t--%s [page# | page-subpage#]\t\t(SAS only)\n", SCSI_SHOW_MP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will display the specified mode page on the screen as raw\n");
        printf("\t\thexadecimal data bytes. Use --%s to control the output.\n", SCSI_SHOW_MP_MPC_LONG_OPT_STRING);
        printf("\t\tIf --%s is not provided, the current values will be shown.\n\n", SCSI_SHOW_MP_MPC_LONG_OPT_STRING);
    }
}

void print_SCSI_Show_MP_Control_Help(bool shortHelp)
{
    printf("\t--%s [current | default | saved | changeable | all]\t\t(SAS only)\n", SCSI_SHOW_MP_MPC_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to control the output of the --%s option.\n", SCSI_SHOW_MP_LONG_OPT_STRING);
        printf("\t\t    current - show the current values of the mode page.\n");
        printf("\t\t    default - show the default values of the mode page.\n");
        printf("\t\t    saved   - show the saved values of the mode page.\n");
        printf("\t\t    changeable - show the changable fields in a mode page.\n");
        printf("\t\t    all - show all of the above formats for a given mode page.\n\n");
    }
}

void print_SCSI_Reset_LP_Help(bool shortHelp)
{
    printf("\t--%s [cumulative | threshold | defCumulative | defThreshold | all]\t\t(SAS only)\n", SCSI_RESET_LP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to reset all SCSI Log Pages.\n");
        printf("\t\tIf the device is compliant with SPC4 or later, the\n");
        printf("\t\t--%s option may be used to specify a specific page to reset.\n", SCSI_RESET_LP_PAGE_LONG_OPT_STRING);
        printf("\t\tThe --%s option may also be passed to prevent saving changes.\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\t    cumulative - reset the cumulative values\n");
        printf("\t\t    threshold  - reset the threshold values\n");
        printf("\t\t    defCumulative - reset the cumulative values to default without saving.\n");
        printf("\t\t    defThreshold  - reset the threshold values to default without saving.\n");
        printf("\t\t    all - sends the log page reset command to all of the above control values\n\n");
    }
}

void print_SCSI_Reset_LP_Page_Help(bool shortHelp)
{
    printf("\t--%s [page# | page-subpage#]\t\t(SAS only)\n", SCSI_RESET_LP_PAGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to specify a specific page, and/or subpage\n");
        printf("\t\tto be used with the --%s option.\n", SCSI_RESET_LP_LONG_OPT_STRING);
        printf("\t\tNOTE: This option will only work on newer drives compliant with\n");
        printf("\t\tthe SPC4 specification.\n\n");
    }
}

void print_Set_SCSI_MP_Help(bool shortHelp)
{
    printf("\t--%s [ mp[-sp]:byte:highestBit:fieldWidthInBits=value | file=filename.txt ]\t(SAS only) (Seagate Only)\n", SCSI_SET_MP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set a specific field in a mode page to a value.\n");
        printf("\t\tThere are two argument formats to this option:\n");
        printf("\t\t1. The first format expects a mode page (in hex), optionally a subpage code (in hex),\n");
        printf("\t\t   the byte offset that the field starts at (in decimal), the highest bit the field starts\n");
        printf("\t\t   at (0-7), the width of the field in as a number of bits (decimal), and the value to set (hex or decimal)\n");
        printf("\t\t   A maximum of 64bits can be set at a time with this option.\n");
        printf("\t\t2. The second format is a text file that contains all bytes of the mode page in hex. Each byte\n");
        printf("\t\t   must be separated by a space, new line, or underscore. It is recommended that this file\n");
        printf("\t\t   is created by copy-pasting the output of the --%s option's default classic view, then modifying\n", SCSI_SHOW_MP_LONG_OPT_STRING);
        printf("\t\t   after that.");
        printf("\t\tExample use of the arguments:\n");
        printf("\t\t1. Setting WCE to zero on caching MP from a file:\n");
        printf("\t\t   command line: file=cachingModePage.txt\n");
        printf("\t\t   File contents: 88 12 10 00 FF FF 00 00 FF FF FF FF 90 20 00 00 00 00 00 00\n");
        printf("\t\t2. Setting WCE to zero on caching MP from command line:\n");
        printf("\t\t   command line: 08:2:2:1=0\n");
        printf("\t\t3. Setting DLC to one on Control Extension MP from command line:\n");
        printf("\t\t   command line: 0A-01:4:3:1=1\n");
        printf("\n");
    }
}

void print_Show_SCSI_MP_Output_Mode_Help(bool shortHelp)
{
    printf("\t--%s [classic | buffer]\t(SAS Only)\n", SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to control the format of the output when displaying a SCSI mode page.\n");
        printf("\t\tModes:\n");
        printf("\t\t  classic - This output is a classic output from old SCSI manuals where the bytes of\n");
        printf("\t\t            the page are output in a rows across the screen in hexadecimal format.\n");
        printf("\t\t  buffer  - This output is a formatted buffer showing offsets on the top and side in hex.\n");
        printf("\t\t            This will output each row with up to 16 bytes of data before moving to the\n");
        printf("\t\t            next row.\n\n");
    }
}

void print_NVM_Format_Help(bool shortHelp)
{
    printf("\t--%s [current | format # | sector size]\t(NVMe Only)\n", NVM_FORMAT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to start an NVM format operation.\n");
        printf("\t\tUse \"current\" to perform a format operation with the\n");
        printf("\t\tSector size currently being used.\n");
        printf("\t\tIf a value between 0 and 15 is given, then that will issue\n");
        printf("\t\tthe NVM format with the specified sector size/metadata size for\n");
        printf("\t\tthat supported format on the drive.\n");
        printf("\t\tValues 512 and higher will be treated as a new sector size\n");
        printf("\t\tto switch to and will be matched to an appropriate lba format\n");
        printf("\t\tsupported by the drive.\n");
        printf("\t\tThis command will erase all data on the drive.\n");
        printf("\t\tCombine this option with--%s to poll\n", POLL_LONG_OPT_STRING);
        printf("\t\tfor progress until the format is complete.\n\n");
    }
}

void print_NVM_Format_NSID_Help(bool shortHelp)
{
    printf("\t--%s [all | current]\t(NVMe Only)\n", NVM_FORMAT_NSID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option changes the NSID used when issuing the NVM format\n");
        printf("\t\tcommand. This can be used to control formatting an entire\n");
        printf("\t\tdevice or a specific namespace if the device supports specifying\n");
        printf("\t\tspecific namespaces for a format command. Not all devices support\n");
        printf("\t\tthis behavior. This has no effect on devices that do not support\n");
        printf("\t\ttargeting a specific namespace and will format the entire device\n");
        printf("\t\tIf this option is not given, the format will be issued to all\n");
        printf("\t\tnamespaces by default.\n\n");
    }
}

void print_NVM_Format_Secure_Erase_Help(bool shortHelp)
{
    printf("\t--%s [none | user | crypto]\t(NVMe Only)\n", NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to specify the type of erase to perform\n");
        printf("\t\tduring an NVM format operation. All user data will be inaccessible\n");
        printf("\t\tupon completion of an NVM format, no matter the erase requested.\n");
        printf("\t\tOptions:\n");
        printf("\t\t  none - no secure erase requested (previous data will not be accessible)\n");
        printf("\t\t  user - requests all user data is erased by the device.\n");
        printf("\t\t  crypto - requests a cryptographic erase of all user data. Note: this mode\n");
        printf("\t\t    is not supported on all devices.\n\n");
    }
}

void print_NVM_Format_PI_Type_Help(bool shortHelp)
{
    printf("\t--%s [ 0 | 1 | 2 | 3 ]\t(NVMe Only)\n", NVM_FORMAT_PI_TYPE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the protection type to format the\n");
        printf("\t\tmedium with.\n");
        printf("\t\tNote: Not all devices support protection types.\n");
        printf("\t\tIf this option is not provided, the NVM format will\n");
        printf("\t\treuse the current setting.\n\n");
    }
}

void print_NVM_Format_PIL_Help(bool shortHelp)
{
    printf("\t--%s [ beginning | end ]\t(NVMe Only)\n", NVM_FORMAT_PI_LOCATION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the location protection\n");
        printf("\t\tinformation in an NVM device's metadata.\n");
        printf("\t\tNote: Not all devices support specifying this.\n");
        printf("\t\tIf this option is not provided, the NVM format will\n");
        printf("\t\treuse the current setting.\n\n");
    }
}

void print_NVM_Format_Metadata_Size_Help(bool shortHelp)
{
    printf("\t--%s [ # of bytes for metadata ]\t(NVMe Only)\n", NVM_FORMAT_METADATA_SIZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to specify the length of metadata\n");
        printf("\t\twith a requested logical block size. The device must\n");
        printf("\t\tsupport the combination of logical block size and metadata size\n");
        printf("\t\tor the format will be rejected by the device.\n\n");
    }
}

void print_NVM_Format_Metadata_Setting_Help(bool shortHelp)
{
    printf("\t--%s [ xlba | separate ]\t(NVMe Only)\n", NVM_FORMAT_METADATA_SETTING_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify how metadata is transmitted to\n");
        printf("\t\tthe host system.\n");
        printf("\t\tOptions:\n");
        printf("\t\t  xlba - metadata is transferred as part of the logical block data\n");
        printf("\t\t  separate - metadata is transferred as a separate buffer\n");
        printf("\t\tNote: Not all devices support specifying this.\n");
        printf("\t\tIf this option is not provided, the NVM format will\n");
        printf("\t\treuse the current setting.\n\n");
    }
}

void print_No_Time_Limit_Help(bool shortHelp)
{
    printf("\t--%s\n", IGNORE_OPERATION_TIMEOUT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse with utility command arguments which have a built in timeout\n");
        printf("\t\tvalue. For example, --%s has a 10 minute default\n", SHORT_DST_LONG_OPT_STRING);
        printf("\t\ttimeout. In some cases a good drive may need more time to\n");
        printf("\t\tcomplete the test due to other legitimate system activity.\n\n");
    }
}