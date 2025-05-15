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
// \file openseachest_util_options.c
// \brief This file defines the functions and macros to make building a utility easier.

#include "common_types.h"
#include "io_utils.h"
#include "memory_safety.h"
#include "string_utils.h"
#include "time_utils.h"
#include "type_conversion.h"

#include "openseachest_util_options.h"

#if defined(__linux__)
#    if defined(VMK_CROSS_COMP)
const char* deviceHandleExample = "vmhba<#>";
const char* deviceHandleName    = "<deviceHandle>";
const char* commandWindowType   = "terminal";
#    else
const char* deviceHandleExample = "/dev/sg<#>";
const char* deviceHandleName    = "<sg_device>";
const char* commandWindowType   = "terminal";
#    endif
#elif defined(__FreeBSD__) || defined(__DragonFly__)
const char* deviceHandleExample = "/dev/da<#>";
const char* deviceHandleName    = "<da_device>";
const char* commandWindowType   = "shell";
#elif defined(__NetBSD__) || defined(__OpenBSD__)
// TODO: Need a better way to handle WD vs SD devices
const char* deviceHandleExample = "/dev/sd<#>";
const char* deviceHandleName    = "<sd_device>";
const char* commandWindowType   = "shell";
#elif defined(_WIN32)
#    include "windows_version_detect.h"
const char* deviceHandleExample = "PD<#>";
const char* deviceHandleName    = "<physical_device>";
const char* commandWindowType   = "command";
#elif defined(__sun)
const char* deviceHandleExample = "/dev/rdsk/c<#>t<#>d<#>";
const char* deviceHandleName    = "<rdsk_device>";
const char* commandWindowType   = "shell";
#elif defined(_AIX)
const char* deviceHandleExample = "/dev/rhdisk<#>";
const char* deviceHandleName    = "<rhdisk_device>";
const char* commandWindowType   = "shell";
#else
#    error "OS Not Defined or known"
#endif

#if defined(ENABLE_CSMI)
// static const char *csmiDeviceHandleName = "<csmi_device>";
#    if defined(_WIN32)
static const char* csmiDeviceHandleExample = "csmi<#>:<#>:<#>:<#>";
#    else
static const char* csmiDeviceHandleExample = "<error<#><#><#>>";
#    endif //_WIN32
#endif     // ENABLE_CSMI

void print_Bug_Report_Email(bool shortHelp)
{
    // we only want to show this for long help information. - TJE
    if (!shortHelp)
    {
        printf("\nPlease report bugs/suggestions to seaboard@seagate.com.\n");
        printf("Include the output of --%s information in the email.\n\n", VERSION_LONG_OPT_STRING);
    }
}

M_NODISCARD bool set_Verbosity_From_String(const char* requestedLevel, eVerbosityLevels* verbosity)
{
    bool set = false;
    if (requestedLevel && verbosity)
    {
        char* end  = M_NULLPTR;
        long  temp = 0L;
        if (0 == safe_strtol(&temp, requestedLevel, &end, BASE_10_DECIMAL) && strcmp(end, "") == 0 &&
            C_CAST(eVerbosityLevels, temp) < VERBOSITY_MAX)
        {
            *verbosity = M_STATIC_CAST(eVerbosityLevels, temp);
            set        = true;
        }
    }
    return set;
}

void print_Elevated_Privileges_Text(void)
{
    printf("WARNING: You must run with elevated privileges to communicate with devices in the system.");
#if defined(_WIN32)
    printf("(admin)");
#elif defined(__unix__) || defined(__APPLE__)
    printf("(root, sudo)");
#else
    printf("(admin, root, sudo, etc)");
    printf("\nor be part of a privileged group with disk access.");
#    if defined(__linux__)
    printf("(disk)");
// If other systems have groups which get disk access, list them here. Currently only know about the disk group in Linux
#    endif

#endif
    printf("\nExamples of elevated privileges: \n");
#if defined(_WIN32)
    printf("In Windows, open the Command Prompt using \"Run as administrator\".\n");
#elif defined(__unix__) || defined(__APPLE__)
    // handle the various linux/unix/unix-like OSs with more ifdefs here
#    if defined(__linux__)
#        if defined(VMK_CROSS_COMP)
    printf("In ESXi, put sudo before the command. This may require inputting your login password.\n");
    printf("In ESXi, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#        else
    printf("In Linux, put sudo before the command. This may require inputting your login password.\n");
    printf("In Linux, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#        endif
#    elif defined(__DragonFly__)
    printf("In DragonFlyBSD, put sudo before the command. This may require inputting your login password.\n");
    printf("In DragonFlyBSD, log in to a root terminal (su), then execute the command. This requires the root "
           "password.\n");
#    elif defined(__FreeBSD__)
    printf("In FreeBSD, put sudo before the command. This may require inputting your login password.\n");
    printf("In FreeBSD, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#    elif defined(__OpenBSD__)
    printf("In OpenBSD, put sudo before the command. This may require inputting your login password.\n");
    printf("In OpenBSD, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#    elif defined(__NetBSD__)
    printf("In NetBSD, put sudo before the command. This may require inputting your login password.\n");
    printf("In NetBSD, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#    elif defined(__illumos__) || defined(THIS_IS_ILLUMOS)
    printf("In Illumos, put sudo before the command. This may require inputting your login password.\n");
    printf("In Illumos, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#    elif defined(__sun)
    printf("In Solaris, put sudo before the command. This may require inputting your login password.\n");
    printf("In Solaris, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#    else // generic unix/unix-like case //Add more OS specific ifdefs to customize messages above
    printf("In Linux/Unix, put sudo before the command. This may require inputting your login password.\n");
    printf(
        "In Linux/Unix, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#    endif
#else // generic, who knows what OS this is case
    printf("In Windows, open the Command Prompt using \"Run as administrator\".\n");
    printf("In Linux/Unix, put sudo before the command. This may require inputting your login password.\n");
    printf(
        "In Linux/Unix, log in to a root terminal (su), then execute the command. This requires the root password.\n");
#endif
}

char* get_current_year(char* temp_year)
{
    size_t len   = safe_strlen(__DATE__);
    temp_year[4] = '\0';
    temp_year[3] = __DATE__[len - 1];
    temp_year[2] = __DATE__[len - 2];
    temp_year[1] = __DATE__[len - 3];
    temp_year[0] = __DATE__[len - 4];
    return temp_year;
}

void openseachest_utility_Info(const char* utilityName, const char* buildVersion, const char* seaCPublicVersion)
{
    eArchitecture architecture = get_Compiled_Architecture();
    char*         year         = safe_calloc(CURRENT_YEAR_LENGTH, sizeof(char));
    char*         userName     = M_NULLPTR;
    errno_t       userdup      = 0;
#if defined(ENABLE_READ_USERNAME)
    if (SUCCESS != get_Current_User_Name(&userName))
    {
#    define UNKNOWN_USER_NAME_MAX_LENGTH 36
        userName = M_REINTERPRET_CAST(char*, safe_calloc(UNKNOWN_USER_NAME_MAX_LENGTH, sizeof(char)));
        if (userName)
        {
            snprintf_err_handle(userName, UNKNOWN_USER_NAME_MAX_LENGTH, "Unable to retrieve current username");
        }
        else
        {
            userdup = ENOMEM;
        }
    }
#else //! ENABLE_READ_USERNAME
    if (is_Running_Elevated())
    {
#    if defined(_WIN32)
        userdup = safe_strdup(&userName, "admin");
#    else  //!_WIN32
        userdup = safe_strdup(&userName, "root");
#    endif //_WIN32
    }
    else
    {
        userdup = safe_strdup(&userName, "current user");
    }
#endif     // ENABLE_READ_USERNAME
    printf("==========================================================================================\n");
    printf(" %s - openSeaChest drive utilities", utilityName);
    printf(" - NVMe Enabled");
    printf("\n Copyright (c) 2014-%s Seagate Technology LLC and/or its Affiliates, All Rights Reserved\n",
           get_current_year(year));
    printf(" %s Version: %s-%s ", utilityName, buildVersion, seaCPublicVersion);
    print_Architecture(architecture);
    printf("\n");
    printf(" Build Date: %s\n", __DATE__);
    if (get_current_timestamp() == false)
    {
        snprintf_err_handle(CURRENT_TIME_STRING, CURRENT_TIME_STRING_LENGTH, "Unable to get local time");
    }
    printf(" Today: %s", CURRENT_TIME_STRING);
    if (userdup == 0)
    {
        printf("\tUser: %s", userName);
    }
    printf("\n==========================================================================================\n");
    safe_free(&userName);
    safe_free(&year);
}

void utility_Full_Version_Info(const char* utilityName,
                               const char* buildVersion,
                               int         seaCPublicMajorVersion,
                               int         seaCPublicMinorVersion,
                               int         seaCPublicPatchVersion,
                               const char* openseaCommonVersion,
                               const char* openseaOperationVersion)
{
    DECLARE_ZERO_INIT_ARRAY(char, osName, OS_NAME_SIZE);
    OSVersionNumber osversionnumber;
    eCompiler       compilerUsed = OPENSEA_COMPILER_UNKNOWN;
    compilerVersion compilerVersionInfo;
    safe_memset(&osversionnumber, sizeof(OSVersionNumber), 0, sizeof(OSVersionNumber));
    safe_memset(&compilerVersionInfo, sizeof(compilerVersion), 0, sizeof(compilerVersion));
    get_Compiler_Info(&compilerUsed, &compilerVersionInfo);
    get_Operating_System_Version_And_Name(&osversionnumber, &osName[0]);

    printf("Version Info for %s:\n", utilityName);
    printf("\tUtility Version: %s\n", buildVersion);
    printf("\topensea-common Version: %s\n", openseaCommonVersion);
    printf("\topensea-transport Version: %" PRId32 ".%" PRId32 ".%" PRId32 "\n", seaCPublicMajorVersion,
           seaCPublicMinorVersion, seaCPublicPatchVersion);
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

// This function is used in at_exit() only.
// It will flush stdout and stderr after printing one final newline character.
void print_Final_newline(void)
{
    printf("\n");
    // Flushing stdout and stderr
    // https://wiki.sei.cmu.edu/confluence/display/c/FIO23-C.+Do+not+exit+with+unflushed+data+in+stdout+or+stderr
    // NOTE: Links shows checking for EOF for an error....this is at an exit...I have no idea what error handling we
    // would want at this point.
    flush_stdout();
    flush_stderr();
}

void print_SeaChest_Util_Exit_Codes(int                    numberOfToolSpecificExitCodes,
                                    ptrToolSpecificxitCode toolSpecificExitCodeList,
                                    const char*            toolName)
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
    printf("\t%d = Not enough resourced\n", UTIL_EXIT_NOT_ENOUGH_RESOURCES);
    printf("\t%d = Error Writing File\n", UTIL_EXIT_ERROR_WRITING_FILE);
    printf("\t%d = Device not found when opening handle.\n", UTIL_EXIT_NO_DEVICE);
    printf("\t%d = Device not opened. Handle is busy.\n", UTIL_EXIT_DEVICE_BUSY);
    // more generic exit code help above this comment. Tool specific exit codes in if statement below
    if (numberOfToolSpecificExitCodes > 0 && toolSpecificExitCodeList)
    {
        printf("\t---%s specific exit codes---\n", toolName);
        // We have some number of tool specific exit codes to print out in addition to the generic exit codes we have.
        for (int exitCodeListIter = 0; exitCodeListIter < numberOfToolSpecificExitCodes; ++exitCodeListIter)
        {
            printf("\t%d = %s\n", toolSpecificExitCodeList[exitCodeListIter].exitCode,
                   toolSpecificExitCodeList[exitCodeListIter].exitCodeString);
        }
    }
    printf("\tAnything else = unknown error\n\n");
}

void get_Scan_Flags(deviceScanFlags* scanFlags, const char* optarg)
{
    if (scanFlags != M_NULLPTR && optarg != M_NULLPTR)
    {
        if (strcmp("ata", optarg) == 0)
        {
            scanFlags->scanATA = true;
        }
        else if (strcmp("usb", optarg) == 0)
        {
            scanFlags->scanUSB = true;
        }
        else if (strcmp("scsi", optarg) == 0)
        {
            scanFlags->scanSCSI = true;
        }
        else if (strcmp("nvme", optarg) == 0)
        {
            scanFlags->scanNVMe = true;
        }
        else if (strcmp("raid", optarg) == 0)
        {
            scanFlags->scanRAID = true;
        }
        else if (strcmp("interfaceATA", optarg) == 0)
        {
            scanFlags->scanInterfaceATA = true;
        }
        else if (strcmp("interfaceUSB", optarg) == 0)
        {
            scanFlags->scanInterfaceUSB = true;
        }
        else if (strcmp("interfaceSCSI", optarg) == 0)
        {
            scanFlags->scanInterfaceSCSI = true;
        }
        else if (strcmp("interfaceNVME", optarg) == 0)
        {
            scanFlags->scanInterfaceNVMe = true;
        }
        else if (strcmp("sd", optarg) == 0)
        {
            scanFlags->scanSD = true;
        }
        else if (strcmp("sgtosd", optarg) == 0)
        {
            scanFlags->scanSDandSG = true;
        }
        else if (strcmp("ignoreCSMI", optarg) == 0)
        {
            scanFlags->scanIgnoreCSMI = true;
        }
        else if (strcmp("allowDuplicates", optarg) == 0)
        {
            scanFlags->scanAllowDuplicateDevices = true;
        }
    }
}

void print_Scan_Help(bool shortHelp, const char* helpdeviceHandleExample)
{
    printf("\t-%c, --%s\n", SCAN_SHORT_OPT, SCAN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tScan the system and list all storage devices with logical\n");
        printf("\t\t%s assignments. Shows model, serial and firmware\n", helpdeviceHandleExample);
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
        printf("\t\t\tnvme - show only NVMe devices\n");
        // printf("\t\t\traid - show RAID devices\n");//commented out until we officially add raid support. Currently
        // raids show up as SCSI devices
        printf("\t\t\tinterfaceATA - show devices on an ATA interface\n");
        printf("\t\t\tinterfaceUSB - show devices on a USB interface\n");
        printf("\t\t\tinterfaceSCSI - show devices on a SCSI or SAS interface\n");
        printf("\t\t\tinterfaceNVME = show devices on an NVMe interface\n");
#if defined(__linux__)
        printf("\t\t\tsd - show sd device handles\n");
        printf("\t\t\tsgtosd - show the sd and sg device handle mapping\n");
#endif
#if defined(ENABLE_CSMI)
        printf("\t\t\tignoreCSMI - do not scan for any CSMI devices\n");
        printf("\t\t\tallowDuplicates - allow drives with both CSMI and PD handles\n");
        printf("\t\t\t                  to show up multiple times in the list\n");
#endif
        printf("\n");
    }
}

void print_Device_Help(bool shortHelp, const char* helpdeviceHandleExample)
{
    printf("\t-%c, --%s [deviceHandle | all]\n", DEVICE_SHORT_OPT, DEVICE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with most commands to specify the device\n");
        printf("\t\thandle on which to perform an operation. Example: %s\n", helpdeviceHandleExample);
#if defined(_WIN32)
        printf("\t\tA handle can also be specified as \\\\.\\PhysicalDrive<#>\n");
#endif
#if defined(ENABLE_CSMI)
        printf("\t\tCSMI device handles can be specified as %s\n", csmiDeviceHandleExample);
#endif
        printf("\t\tTo run across all devices detected in the system, use the\n");
        printf("\t\t\"all\" argument instead of a device handle.\n");
        printf("\t\tExample: -%c all\n", DEVICE_SHORT_OPT);
        printf("\t\tNOTE: The \"all\" argument is handled by running the\n");
        printf("\t\t      specified options on each drive detected in the\n");
        printf("\t\t      OS sequentially. For parallel operations, please\n");
        printf("\t\t      use a script opening a separate instance for each\n");
        printf("\t\t      device handle.\n");
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

void print_Low_Level_Info_Help(bool shortHelp)
{
    printf("\t--%s\n", LOWLEVEL_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tDump low-level information about the device to assist with debugging.\n\n");
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

void print_Quiet_Help(bool shortHelp, const char* utilName)
{
    printf("\t-%c, --%s\n", QUIET_SHORT_OPT, QUIET_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tRun %s in quiet mode. This is the same as\n", utilName);
        printf("\t\t-v 0 or --verbose 0\n\n");
    }
}

void print_Version_Help(bool shortHelp, const char* utilName)
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

void print_Sanitize_Freeze_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", SANITIZE_FREEZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tFreezelock is a command to block processing of sanitize\n");
        printf("\t\toperations until a power cycle is performed on a device.\n");
        printf("\t\tIt is only available on ATA drives. Once this command has been\n");
        printf("\t\tsent, the freezelock status becomes immediate and cannot be\n");
        printf("\t\tcleared until the drive has been powered off. All sanitize\n");
        printf("\t\tcommands, except a sanitize status will be aborted.\n\n");
    }
}

void print_Sanitize_Anti_Freeze_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", SANITIZE_ANTIFREEZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tAntifreezelock is a command that is designed to block a\n");
        printf("\t\tfreezelock command from locking out the sanitize feature set.\n");
        printf("\t\tIt is only available on ATA drives that support the ACS3, or\n");
        printf("\t\tnewer specification.\n\n");
    }
}

void print_Sanitize_Help(bool shortHelp, const char* utilName)
{
    printf("\t--%s [info | blockerase | cryptoerase |\n", SANITIZE_LONG_OPT_STRING);
    printf("\t            overwrite | freezelock | antifreezelock] \t(Purge)\n");
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
        printf("\t\tBy default, sanitize runs in restricted exit mode, meaning the\n");
        printf("\t\tonly way to exit a failed sanitize is to attempt sanitize again\n");
        printf("\t\tuntil it completes successfully. Add the --%s option to run\n", SANITIZE_AUSE_LONG_OPT_STRING);
        printf("\t\tin unrestricted mode. In unrestricted mode, if sanitize fails\n");
        printf("\t\tyou can exit this mode with the \"exit failure mode\" command\n");
        printf("\t\tor a successful sanitize command.\n\n");
        printf("\t\tFor Zoned block devices, the --%s option can be used\n", ZONE_NO_RESET_LONG_OPT_STRING);
        printf("\t\tto stop the write pointers from resetting allowing full\n");
        printf("\t\tdrive verification to be performed upon completion of sanitize\n");
        printf("\t\tFor NVMe devices that support the deallocate feature (TRIM), the\n");
        printf("\t\t--%s option can be used to prevent the deallocation of blocks at\n",
               NO_DEALLOCATE_AFTER_ERASE_LONG_OPT_STRING);
        printf("\t\tcompletion of sanitize to allow for full drive verification.\n");
        printf("\t\tNOTE: An NVMe controller may inhibit the no deallocate behavior\n");
        printf("\t\t      and may deallocate anyways or fail the sanitize command when\n");
        printf("\t\t      no deallocate is specified.\n\n");
#if defined(_WIN32)
        if (!is_Windows_PE())
        {
            printf("\t\tNote: Windows 8 and higher block sanitize commands. Sanitize\n");
            printf("\t\toperations will show a failure status on these systems.\n");
            printf("\t\tStarting in Windows 11 or Windows 10 21H1, sanitize crypto\n");
            printf("\t\tor block erase can be executed on NVMe data drives that\n");
            printf("\t\tsupport the sanitize command set.\n\n");
        }
#endif //_WIN32
       // blockerase info
        printf("\t\t* blockerase on some solid state drives is very fast at less\n");
        printf("\t\tthan one (1) second, while others may take more that 30 seconds\n");
        printf("\t\tThis operation performs a physical low level block erase\n");
        printf("\t\toperation on all current, past, and potential user data.\n");
        printf("\t\tThe contents on user data are indeterminate upon completion.\n\n");
        // cryptoerase info
        printf("\t\t* cryptoerase is very fast at less than one (1) second. It\n");
        printf("\t\tchanges the internal encryption keys that are used for user\n");
        printf("\t\tdata causing all previous data to be useless.\n\n");
        // overwrite info
        printf("\t\t* overwrite is a physical overwrite on all current, past, and\n");
        printf("\t\tpotential user data. The ATA, NVMe, & SCSI specifications allow a\n");
        printf("\t\tuser defined pattern and multiple passes. %s will\n", utilName);
        printf("\t\tuse a zero pattern and a single pass for this operation\n");
        printf("\t\tby default. Use --%s and --%s to specify\n", SANITIZE_OVERWRITE_PASSES_LONG_OPT_STRING,
               SANITIZE_IPBP_LONG_OPT_STRING);
        printf("\t\tthe number of passes and whether to invert the pattern between\n");
        printf("\t\teach overwrite pass.\n\n");
        // freezelock info
        printf("\t\t* freezelock - migrate to use of --%s\n\n", SANITIZE_FREEZE_LONG_OPT_STRING);
        // anti freezlock info
        printf("\t\t* antifreezelock - migrate to use of --%s\n\n", SANITIZE_ANTIFREEZE_LONG_OPT_STRING);
        printf("\t\tWARNING: Sanitize may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Sanitize_Overwrite_Passes_Help(bool shortHelp)
{
    printf("\t--%s [ number of overwrite passes ]\n", SANITIZE_OVERWRITE_PASSES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tSpecify the number of overwrite passes to use during a sanitize\n");
        printf("\t\toverwrite operation. By default, only a single overwrite pass\n");
        printf("\t\tis used unless this option specifies a different value.\n");
        printf("\t\tThe maximum number of passes varies by drive type:\n");
        printf("\t\tATA:  16 passes\n");
        printf("\t\tNVMe: 16 passes\n");
        printf("\t\tSCSI: 31 passes\n");
        printf("\t\tUse the --%s option to instruct the device to invert\n", SANITIZE_IPBP_LONG_OPT_STRING);
        printf("\t\tthe pattern between each overwrite pass.\n");
        printf("\n");
    }
}

void print_Sanitize_Overwrite_Invert_Help(bool shortHelp)
{
    printf("\t--%s\n", SANITIZE_IPBP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to instruct the drive to invert the requested\n");
        printf("\t\tsanitize overwrite pattern between each overwrite pass.\n");
        printf("\t\tFor the default pattern of all zeroes, this means that after\n");
        printf("\t\ta first pass of zeroes, the second pass will be all 1's (binary)\n");
        printf("\t\tor all F's (hexadecimal)\n\n");
    }
}

void print_Sanitize_AUSE_Help(bool shortHelp)
{
    printf("\t--%s\n", SANITIZE_AUSE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to allow running a sanitize operation in\n");
        printf("\t\tunrestricted mode. Without this option, all sanitize options\n");
        printf("\t\tare run in restricted mode by default.\n");
        printf("\t\tIn unrestricted mode, if a sanitize erase fails the drive enters\n");
        printf("\t\ta failure state. The failure state can be cleared with a Sanitize\n");
        printf("\t\texit failure mode command, or it can be cleared with a successful\n");
        printf("\t\tsanitize erase.\n");
        printf("\t\tIn restricted mode, if a sanitize fails, the failure state can only\n");
        printf("\t\tbe cleared with a successful sanitize erase.\n\n");
    }
}

void print_Zone_No_Reset_Help(bool shortHelp)
{
    printf("\t--%s\n", ZONE_NO_RESET_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tFor ZBD's (Zoned Block Devices), use this option during a\n");
        printf("\t\tSanitize or ATA Security Erase to specify leaving all zones\n");
        printf("\t\tfull so that full verification of erasure can be performed.\n");
        printf("\t\tWhen this option is not specified, all zones will be empty\n");
        printf("\t\tupon completion of these erases.\n\n");
    }
}

void print_Sanitize_No_Deallocate_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe Only)\n", NO_DEALLOCATE_AFTER_ERASE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tFor NVMe devices, specify this option during a sanitize to\n");
        printf("\t\tleave all blocks allocated after a sanitize erase. By default\n");
        printf("\t\tan NVMe controller will deallocate (TRIM/Unmap) all the LBAs.\n");
        printf("\t\tUsing this option allows for full verification of erasure after\n");
        printf("\t\ta Sanitize command.\n");
        printf("\t\tNOTE: An NVMe controller may inhibit this option in certain\n");
        printf("\t\tconfigurations meaning the sanitize may produce a warning or\n");
        printf("\t\ta failure depending on how this is configured on the controller.\n");
        printf("\t\tAfter verifying an erasure with this option, run a deallocate/TRIM\n");
        printf("\t\tacross the entire device/namespace to match default behavior of\n");
        printf("\t\ta sanitize erase.\n\n");
    }
}

void print_Refresh_Filesystems_Help(bool shortHelp)
{
    printf("\t--%s\n", REFRESH_FILE_SYSTEMS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will call an OS unique low-level routine to rescan\n");
        printf("\t\ta device for any file systems it can detect through the\n");
        printf("\t\tpartition table. The detected filesystems will vary by OS\n");
        printf("\t\tand OS capabilities.\n");
        printf("\t\tThis option is useful to call after completing a full disk erase\n");
        printf("\t\tas it may make a cached volume in the OS go away or detect that a device\n");
        printf("\t\tis empty and ready to have a new file system written to it.\n\n");
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
    printf("\t--%s [starting LBA]\t (Clear)\n", WRITE_SAME_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tEnter a starting lba to begin a write same on to erase a range\n");
        printf("\t\tof data on the drive. On SCSI devices, this uses the\n");
        printf("\t\twritesame16 command. On ATA devices, this uses the SCT writesame\n");
        printf("\t\tfeature. Combine this option with the writeSameRange option to\n");
        printf("\t\tselect the range. This operation will write 0's to the device for the\n");
        printf("\t\tspecified range. For SATA drives, this option will poll for progress\n");
        printf("\t\tuntil the write same has completed. SAS/SCSI drives will hold the\n");
        printf("\t\ttool busy until the write same has completed without progress\n");
        printf("\t\tindication since this is not possible on SAS/SCSI due to specification\n");
        printf("\t\tlimitations on how write same was defined.\n");
        printf("\t\tOn SATA, if any other commands are sent to the drive while it's\n");
        printf("\t\tperforming a write same, the write same will be aborted.\n");
        printf("\t\tNOTE: On SAS/SCSI drives this command is optional. Additionally,\n");
        printf("\t\t      the range may be limited to much less than the full device\n");
        printf("\t\t      size. Due to the history of this command, there is not a great\n");
        printf("\t\t      way to confirm support in all cases. Some ranges will be too\n");
        printf("\t\t      large, and some devices may or may not allow writing the full\n");
        printf("\t\t      medium in a single command. If you wish to write an entire\n");
        printf("\t\t      device, consider a different command such as format unit or\n");
        printf("\t\t      sanitize overwrite to accomplish this.\n\n");
    }
}

void print_Writesame_Range_Help(bool shortHelp)
{
    printf("\t--%s [range in # of LBAs]\n", WRITE_SAME_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tSpecify a range to writesame to. Use this option with the\n");
        printf("\t\twriteSame option in order to begin a write same operation.\n\n");
    }
}

void print_Revert_Help(bool shortHelp)
{
    printf("\t--%s\t(Purge)\n", TCG_REVERT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis operation performs an Opal SSC spec Revert on the adminSP.\n");
        printf("\t\tThis operation is only available on Seagate TCG Opal drives.\n");
        printf("\t\tRevert meets data sanitization purge capabilities to erase data\n");
        printf("\t\tfor Opal, Opalite, and Ruby SSCs\n");
        printf("\t\tThe --%s flag can be provided to perform the revert with\n", TCG_PSID_LONG_OPT_STRING);
        printf("\t\tthe PSID authority in case of a lost password.\n");
        printf("\t\tThe --%s flag can be provided to perform the revert with SID.\n", TCG_SID_LONG_OPT_STRING);
        printf("\t\tIf neither the --%s or the --%s options are provided, then the\n", TCG_PSID_LONG_OPT_STRING,
               TCG_SID_LONG_OPT_STRING);
        printf("\t\trevert will be sent setting SID as the MSID value. This will only work\n");
        printf("\t\ton a drive not already activated by security software.\n");
        printf("\t\tUpon completion, the drive will be \"like new\" with all\n");
        printf("\t\tuser data being cryptographically erased and all other settings\n");
        printf("\t\tset to factory defaults. If this operation fails, try using --%s\n", TCG_REVERT_SP_LONG_OPT_STRING);
        printf("\t\tinstead.\n\n");
        printf("\t\tWARNING: The Revert may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_RevertSP_Help(bool shortHelp)
{
    printf("\t--%s\t(Purge)\n", TCG_REVERT_SP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis operation performs a revertSP on a Seagate SED drive\n");
        printf("\t\tin the adminSP with the PSID.\n");
        printf("\t\tRevertSP meets data sanitization purge capabilities to erase data\n");
        printf("\t\tfor Opal, Opalite, and Ruby SSCs\n");
        printf("\t\tRevertSP also meets data sanitization purge capabilities to erase data\n");
        printf("\t\ton Seagate TCG Enterprise SSC HDDs\n");
        printf("\t\tThe PSID must be provided using the --%s option.\n", TCG_PSID_LONG_OPT_STRING);
        printf("\t\tThis operation is available on all Seagate SED HDD drives and some SSDs.\n");
        printf("\t\tUpon completion, the drive will be \"like new\" with all\n");
        printf("\t\tuser data being cryptographically erased and all other\n");
        printf("\t\tsettings set to factory defaults.\n\n");
        printf("\t\tWARNING: The RevertSP may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Progress_Help(bool shortHelp, const char* testsTogetProgressFor)
{
    printf("\t--%s [%s]\n", PROGRESS_LONG_OPT_STRING, testsTogetProgressFor);
    if (!shortHelp)
    {
        printf("\t\tGet the progress for a test that was started quietly without\n");
        printf("\t\tthe polling option (default). You must specify a test you wish to\n");
        printf("\t\tget progress from. Ex: \"--%s dst\" or \"--%s sanitize\"\n", PROGRESS_LONG_OPT_STRING,
               PROGRESS_LONG_OPT_STRING);
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

void print_SMART_Offline_Data_Collection_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", SHORT_DST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tExecute the SMART off-line data collection routine on an ATA drive.\n");
        printf("\t\tThis will run for the amount of time the device specifies that this\n");
        printf("\t\toperation will take to complete. Progress updates are not available\n");
        printf("\t\twhile this is running. Some drives allow other commands to be processed\n");
        printf("\t\tand some require no interruption while this routine completes.\n");
        printf("\t\tThis routine does vendor unique activities to update the SMART data and\n");
        printf("\t\tSMART attributes the device reports.\n");
        printf("\t\tIf the SMART auto-off-line feature is supported and enabled, then this\n");
        printf("\t\troutine is already running automatically in the background periodically.\n\n");
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

void print_Long_DST_Help(bool shortHelp, const char* helpcommandWindowType)
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
        printf("\t\tthe test then you will need to open a second %s window\n", helpcommandWindowType);
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
    printf("\t--%s [raw | hybrid | analyzed]\t(SATA Only)\n", SMART_ATTRIBUTES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThe drive will display its list of supported SMART attributes.\n");
        printf("\t\tSome attributes names are commonly standard and most others are\n");
        printf("\t\tvendor unique. In either case, the attribute thresholds are\n");
        printf("\t\talways vendor unique. Most attributes are informational and not\n");
        printf("\t\tused to determine a warranty return. Use the --smartCheck\n");
        printf("\t\tcommand to determine if one of the warranty attributes has been\n");
        printf("\t\ttripped. Seagate Support does not help to analyze SMART\n");
        printf("\t\tattributes.\n");
        printf("\t\tOutput modes:\n");
        printf("\t\t  raw - All hex output for those that need every single bit.\n");
        printf("\t\t  hybrid - classic table view with some interpretation of some\n");
        printf("\t\t           fields. Partial raw interpretation, but not all drive\n");
        printf("\t\t           and firmware combinations are supported.\n");
        printf("\t\t  analyzed - a full breakdown of all parts of each individual\n");
        printf("\t\t             attribute's data. Full raw data interpretation only\n");
        printf("\t\t             available on select devices.\n");
        printf("\t\tNOTE: Migration to device statistics is recommended.\n\n");
    }
}

void print_Show_FARM_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_FARM_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show the data in the FARM log.\n");
        printf("\t\tFARM is the Field Accessible Reliability Metrics that\n");
        printf("\t\tcan be reported from a Seagate hard drive.\n");
        printf("\t\tFarm data has a few sections available:\n");
        printf("\t\t\tGeneral Drive Info\n");
        printf("\t\t\tWorkload Statistics\n");
        printf("\t\t\tError Statistics\n");
        printf("\t\t\tEnvironment Statistics\n");
        printf("\t\t\tReliability Statistics\n");
        printf("\t\tNewer Seagate HDDs may be able to report more information than\n");
        printf("\t\tolder models. Data that can be reported may vary depending on the\n");
        printf("\t\tcapabilities of a given product.\n\n");
    }
}

void print_NVME_Health_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe Only)\n", NVME_HEALTH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThe drive will display the NVMe Health log (also called\n");
        printf("\t\tSMART log). All standardized fields will be printed to the\n");
        printf("\t\tscreen for the device.\n\n");
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
        printf("\t\tAbort a Seagate In Drive Diagnostic (IDD) that is in progress.\n");
        printf("\t\tThis may return failure if IDD is not running or has already\n");
        printf("\t\tcompleted running.\n\n");
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
        printf("\t\t      work on some products.\n");
        printf("\t\tNote: Progress cannot be checked for the first 2 minutes of IDD.\n");
        printf("\t\t      The drive is busy with the test and is not able to respond.\n");
        printf("\t\t      Attempting to retrieve progress during this time will hang and\n");
        printf("\t\t      may cause the IDD to abort due to the host issuing resets to\n");
        printf("\t\t      recover access to the drive.\n\n");
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
#if defined(_WIN32)
        printf("\t\tThis option may wake a SAS drive under Windows due to\n");
        printf("\t\ta Windows limitation. If used on the command line with\n");
        printf("\t\tthe transition option, the output will show correctly.\n");
        printf("\t\tex of usage: --%s standby_z --%s\n", TRANSITION_POWER_MODE_LONG_OPT_STRING,
               CHECK_POWER_LONG_OPT_STRING);
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
        printf("\t\tWARNING: The EPC settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Spindown may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Idle_A_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable | default | timerValueMilliseconds ]\n", IDLE_A_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this setting to change the EPC Idle_A power mode settings.\n");
        printf("\t\t    enable  - enable the power state\n");
        printf("\t\t    disable - disable the power state\n");
        printf("\t\t    default - restore default settings for this power state\n");
        printf("\t\t    timerValue - number of milliseconds to set for the timer\n");
        printf("\t\t                 used in this power state. If a timer is provided\n");
        printf("\t\t                 the state will also be enabled, if not already.\n");
        printf("\t\t                 EPC spec timers are set in 100 millisecond increments.\n");
        printf("\t\t                 Timers will be truncated to fit 100 millisecond increments.\n\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tWARNING: EPC Settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Idle_B_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable | default | timerValueMilliseconds ]\n", IDLE_B_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this setting to change the EPC Idle_B power mode settings.\n");
        printf("\t\t    enable  - enable the power state\n");
        printf("\t\t    disable - disable the power state\n");
        printf("\t\t    default - restore default settings for this power state\n");
        printf("\t\t    timerValue - number of milliseconds to set for the timer\n");
        printf("\t\t                 used in this power state. If a timer is provided\n");
        printf("\t\t                 the state will also be enabled, if not already.\n");
        printf("\t\t                 EPC spec timers are set in 100 millisecond increments.\n");
        printf("\t\t                 Timers will be truncated to fit 100 millisecond increments.\n\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tWARNING: EPC Settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Idle_C_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable | default | timerValueMilliseconds ]\n", IDLE_C_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this setting to change the EPC Idle_C power mode settings.\n");
        printf("\t\t    enable  - enable the power state\n");
        printf("\t\t    disable - disable the power state\n");
        printf("\t\t    default - restore default settings for this power state\n");
        printf("\t\t    timerValue - number of milliseconds to set for the timer\n");
        printf("\t\t                 used in this power state. If a timer is provided\n");
        printf("\t\t                 the state will also be enabled, if not already.\n");
        printf("\t\t                 EPC spec timers are set in 100 millisecond increments.\n");
        printf("\t\t                 Timers will be truncated to fit 100 millisecond increments.\n\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tWARNING: EPC Settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Standby_Y_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable | default | timerValueMilliseconds ]\n", STANDBY_Y_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this setting to change the EPC Standby_Y power mode settings.\n");
        printf("\t\t    enable  - enable the power state\n");
        printf("\t\t    disable - disable the power state\n");
        printf("\t\t    default - restore default settings for this power state\n");
        printf("\t\t    timerValue - number of milliseconds to set for the timer\n");
        printf("\t\t                 used in this power state. If a timer is provided\n");
        printf("\t\t                 the state will also be enabled, if not already.\n");
        printf("\t\t                 EPC spec timers are set in 100 millisecond increments.\n");
        printf("\t\t                 Timers will be truncated to fit 100 millisecond increments.\n\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tWARNING: EPC Settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Standby_Z_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable | default | timerValueMilliseconds ]\n", STANDBY_Z_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this setting to change the EPC Standby_Z power mode settings.\n");
        printf("\t\t    enable  - enable the power state\n");
        printf("\t\t    disable - disable the power state\n");
        printf("\t\t    default - restore default settings for this power state\n");
        printf("\t\t    timerValue - number of milliseconds to set for the timer\n");
        printf("\t\t                 used in this power state. If a timer is provided\n");
        printf("\t\t                 the state will also be enabled, if not already.\n");
        printf("\t\t                 EPC spec timers are set in 100 millisecond increments.\n");
        printf("\t\t                 Timers will be truncated to fit 100 millisecond increments.\n\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tWARNING: EPC Settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Legacy_Idle_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable | default | timerValueMilliseconds ]\t(SAS Only)\n", LEGACY_IDLE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this setting to change the idle power mode settings.\n");
        printf("\t\tNOTE: This is the legacy idle timer before EPC drives.\n");
        printf("\t\t      If this is used on an EPC drive, this will modify\n");
        printf("\t\t      the idle_a power state and timer values.\n");
        printf("\t\t    enable  - enable the power state\n");
        printf("\t\t    disable - disable the power state\n");
        printf("\t\t    default - restore default settings for this power state\n");
        printf("\t\t    timerValue - number of milliseconds to set for the timer\n");
        printf("\t\t                 used in this power state. If a timer is provided\n");
        printf("\t\t                 the state will also be enabled, if not already.\n");
        printf("\t\t                 Spec timers are set in 100 millisecond increments.\n");
        printf("\t\t                 Timers will be truncated to fit 100 millisecond increments.\n\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tThis is only available on SAS/SCSI drives as ATA drives did not\n");
        printf("\t\thave a separate configurable idle timer.\n\n");
        printf("\t\tWARNING: EPC Settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Legacy_Standby_Help(bool shortHelp)
{
    printf("\t--%s [ enable | disable | default | timerValueMilliseconds ] (Some settings are SAS only)\n",
           LEGACY_STANDBY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this setting to change the standby power mode settings.\n");
        printf("\t\tNOTE: This is the legacy standby timer before EPC drives.\n");
        printf("\t\t      If this is used on an EPC drive, this will modify\n");
        printf("\t\t      the standby_z power state and timer values.\n");
        printf("\t\t    enable  - enable the power state\t(SAS Only)\n");
        printf("\t\t    disable - disable the power state\t(SAS Only)\n");
        printf("\t\t    default - restore default settings for this power state\t(SAS Only)\n");
        printf("\t\t    timerValue - number of milliseconds to set for the timer\n");
        printf("\t\t                 used in this power state. If a timer is provided\n");
        printf("\t\t                 the state will also be enabled, if not already.\n");
        printf("\t\t                 Spec timers are set in 100 millisecond increments.\n");
        printf("\t\t                 Timers will be truncated to fit 100 millisecond increments.\n\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tATA drives can only change the standby timer, not disable it.\n");
        printf("\t\tOn ATA drives, the standby timer set by this command is volatile\n");
        printf("\t\tand drive defaults are restored on next power cycle.\n\n");
        printf("\t\tWARNING: EPC Settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

// Mainly used for NVMe devices.
void print_extSmatLog_Help(bool shortHelp)
{
    printf("\t--%s\n", EXT_SMART_LOG_LONG_OPT_STRING1);
    if (!shortHelp)
    {
        printf("\t\tUse this option to Extract the Extended Smart Log Attributes.\n\n");
    }
}

void print_pcierr_Help(bool shortHelp)
{
    printf("\t--%s\n", CLEAR_PCIE_CORRECTABLE_ERRORS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to clear correctable errors.\n\n");
    }
}

void print_Transition_Power_State_Help(bool shortHelp)
{
    printf("\t--%s [new power state]\t(NVMe Only)\n", TRANSITION_POWER_STATE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to transition to a specific power state.\n");
        printf("\t\tWARNING: Transitioning the drive to a non-operational power state\n");
        printf("\t\t         may make the device stop responding. The operating system\n");
        printf("\t\t         may or may not block this transition. It is recommended\n");
        printf("\t\t         to only use this option for operational power states\n");
        printf("\t\tHINT:\n\t\t  Use --%s to view the supported states\n", SHOW_NVM_POWER_STATES_LONG_OPT_STRING);
    }
}

void print_Show_NVM_Power_States_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe Only)\n", SHOW_NVM_POWER_STATES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to display a device's supported power states.\n");
    }
}

void print_Show_Phy_Event_Counters_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", SHOW_PHY_EVENT_COUNTERS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to display a SATA device's supported\n");
        printf("\t\tevent counters.\n\n");
    }
}

void print_Transition_Power_Help(bool shortHelp)
{
    printf("\t--%s [active | idle | idleUnload | standby | idle_a | idle_b | idle_c | standby_y | standby_z | sleep]\n",
           TRANSITION_POWER_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to transition the drive to a specific power state.\n");
        printf("\t\tEPC and legacy power states are supported. EPC states are only available\n");
        printf("\t\ton devices supporting the EPC feature.\n");
        printf("\t\tSupported power states:\n");
        printf("\t\t  active\n");
        printf("\t\t  idle - idle mode (legacy mode equivalent to idle_a on EPC)\n");
        printf("\t\t  idleUnload - same as above, but heads are unloaded. This may not\n");
        printf("\t\t               be supported on all devices.\n");
        printf("\t\t  standby - standby mode (legacy mode equivalent to standby_z on EPC)\n");
        printf("\t\t  idle_a - EPC idle mode\n");
        printf("\t\t  idle_b - EPC lower power idle mode\n");
        printf("\t\t  idle_c - EPC lowest power idle mode\n");
        printf("\t\t  standby_y - EPC low power standby mode\n");
        printf("\t\t  standby_z - EPC lowest power standby mode\n");
        printf("\t\t  sleep - Sleep state. WARNING: This requires a reset to wake from.\n");
        printf("\t\t          Once in this state, this tool cannot wake the drive on its own.\n");
        printf("\t\t          The OS or adapter will need to issue a reset, which may or may not happen.\n\n");
        printf("\t\tWARNING: Transitioning power modes may affect all LUNs/namespaces\n");
        printf("\t\t         for devices with multiple logical units or namespaces.\n\n");
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
    printf("\t--%s [range in # of LBAs]\n", USER_GENERIC_LONG_OPT_RANGE_STRING);
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
    printf("\t--%s [starting LBA]\t(Clear)\n", OVERWRITE_LONG_OPT_STRING);
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
    printf("\t--%s [range in # of LBAs]\t(Clear)\n", OVERWRITE_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse with the overwrite option (--%s) to\n", OVERWRITE_LONG_OPT_STRING);
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
    printf("\t--%s or --%s [range in # of LBAs]\n", TRIM_RANGE_LONG_OPT_STRING, UNMAP_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse one of these options to specify a range to trim\n");
        printf("\t\tor unmap on a drive. A starting point must be specified\n");
        printf("\t\twith the --%s/--%s option.\n\n", TRIM_LONG_OPT_STRING, UNMAP_RANGE_LONG_OPT_STRING);
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
    printf("\t--%s [default | highest | intermediate | lowest | watt value]\t(SAS Only) \n",
           SET_POWER_CONSUMPTION_LONG_OPT_STRING);
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

void print_Fast_Discovery_Help(bool shortHelp)
{
    printf("\t--%s\n", FAST_DISCOVERY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option  to issue a fast scan on the specified drive. \n\n");
    }
}

void print_Firmware_Download_Help(bool shortHelp)
{
    printf("\t--%s [firmware_filename]\n", DOWNLOAD_FW_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tDownload firmware to a Seagate storage product. Use only\n");
        printf("\t\tdevice manufacturer authorized firmware data files which are designated\n");
        printf("\t\tfor the specific model drive. Improper use of this option may\n");
        printf("\t\tharm a device and or its data. You may specify the path (without\n");
        printf("\t\tspaces) if the firmware data file is in a different location.\n");
        printf("\t\tThis option will use auto mode by default to choose the best method\n");
        printf("\t\tfor updating the drive firmware. Use the --%s option to", DOWNLOAD_FW_MODE_LONG_OPT_STRING);
        printf("\t\tspecify a different download mode.\n\n");
        printf("\t\tWARNING: Firmware updates may affect all LUNs/namespaces\n");
        printf("\t\t         for devices with multiple logical units or namespaces.\n\n");
    }
}

void print_Firmware_Slot_Buffer_ID_Help(bool shortHelp)
{
    printf("\t--%s/--%s slot#\n", FIRMWARE_SLOT_LONG_OPT_STRING, FIRMWARE_BUFFER_ID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a firmware slot (NVMe) or a buffer ID (SCSI)\n");
        printf("\t\talong with the --%s (SCSI) or --%s (NVMe & SCSI) options.\n", DOWNLOAD_FW_MODE_LONG_OPT_STRING,
               ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
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
        printf("\t\tYou can use this along with a --%s & --%s to\n", DOWNLOAD_FW_LONG_OPT_STRING,
               DOWNLOAD_FW_MODE_LONG_OPT_STRING);
        printf("\t\tautomatically issue the activate command after the download has\n");
        printf("\t\tcompleted.\n\n");
        printf("\t\tWARNING: Firmware activation may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Firmware_Switch_Help(bool shortHelp)
{
    printf("\t--%s \t(NVMe Only)\n", SWITCH_FW_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to switch to a different firmware slot on an\n");
        printf("\t\tNVMe drive. You must specify a slot with the --%s option\n", FIRMWARE_SLOT_LONG_OPT_STRING);
        printf("\t\tor this will fail. The specified slot must already have a\n");
        printf("\t\tvalid firmware image in it as well.\n\n");
        printf("\t\tWARNING: Switching firmware may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Force_NVMe_Commit_Action_Help(bool shortHelp)
{
    printf("\t--%s [ # ]\t(NVMe Only)\n", FORCE_NVME_COMMIT_ACTION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to force a specific commit action when\n");
        printf("\t\tactivating firmware on an NVMe drive.\n");
        printf("\t\tThe commit action can be specified in decimal or hexadecimal.\n");
        printf("\t\tWARNING: This is not possible on most Windows nvme drivers.\n");
        printf("\t\t         Recommend using a different OS if this is necessary.\n");
        printf("\t\t         This utility will skip all activation when this option\n");
        printf("\t\t         is present on an interface where this is not possible\n\n");
    }
}

void print_Force_NVMe_Disable_FW_Reset_Help(bool shortHelp)
{
    printf("\t--%s \t(NVMe Only)\n", FORCE_DISABLE_NVME_FW_COMMIT_RESET_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to forcefully disable the reset to complete\n");
        printf("\t\tactivating firmware on an NVMe drive that is performed by\n");
        printf("\t\tdefault. This will happen automatically as required without\n");
        printf("\t\tthis option for commit actions that require resets to complete\n");
        printf("\t\tfirmware activation (ex: 001b or 010b)\n");
        printf("\t\tWARNING: This is not possible on most Windows nvme drivers.\n");
        printf("\t\t         Recommend using a different OS if this is necessary.\n");
        printf("\t\t         This utility will skip all activation when this option\n");
        printf("\t\t         is present on an interface where this is not possible\n\n");
    }
}

void print_Firmware_Download_Mode_Help(bool shortHelp)
{
    printf("\t--%s [ auto | full | segmented | deferred | deferred+activate ]\n", DOWNLOAD_FW_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option along with the --%s option\n", DOWNLOAD_FW_LONG_OPT_STRING);
        printf("\t\tto set the firmware download mode.\n");
        printf("\t\tSupported Modes:\n");
        printf("\t\t\tauto - automatically determines the best mode to use to\n");
        printf("\t\t\t       perform the firmware update.\n");
        printf("\t\t\tfull - performs a download in one large\n");
        printf("\t\t\t            transfer to the device.\n");
        printf("\t\t\tsegmented - downloads the firmware in multiple\n");
        printf("\t\t\t            segments to the device. (Most compatible)\n");
        printf("\t\t\tdeferred - performs a segmented download to the\n");
        printf("\t\t\t           device, but does not activate the new\n");
        printf("\t\t\t           firmware until a powercycle or activate\n");
        printf("\t\t\t           command is sent.\n");
        printf("\t\t\tdeferred+activate - performs a deferred download and\n");
        printf("\t\t\t                    automatically acitvates it for you.\n");
        printf("\t\t\t                    Similar to how a segmented download works\n");
        printf("\t\t\t                    but uses a separate activate command. This\n");
        printf("\t\t\t                    is the recommended mode that \"auto\" will\n");
        printf("\t\t\t                    select when possible for maximum compatibility\n");
        printf("\t\t\t                    with Windows 10 and later operating systems.\n");
        printf("\n\t\tWARNING: Firmware Updates may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Output_Mode_Help(bool shortHelp)
{
    printf("\t--%s   [ raw | binary ]\n", OUTPUT_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with others options such as --%s\n", GET_NVME_LOG_LONG_OPT_STRING);
        printf("\t\tand --%s to show a buffer output or to\n", DEVICE_INFO_LONG_OPT_STRING);
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

void print_Get_Telemetry_Help(bool shortHelp)
{
    printf("\t--%s [host | current || ctrl | saved]\n", GET_TELEMETRY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to get the Telemetry data for a device.\n");
        printf("\t\tThis log is also known as the internal status log on SATA\n");
        printf("\t\tand SAS devices.\n");
        printf("\t\tUse the --%s option to control the amount of \n", GET_TELEMETRY_LONG_OPT_STRING);
        printf("\t\tdata collected. \n\n");
        printf("\t\tSupported Modes:\n");
        printf("\t\t\thost - get Host initiated Telemetry on NVMe (same as \"current\")\n");
        printf("\t\t\tctrl - get Controller initiated Telemetry on NVMe (Same as \"saved\"\n");
        printf("\t\t\tcurrent - get the current internal status log on SAS/SATA\n");
        printf("\t\t\tsaved - get the saved internal status log on SAS/SATA\n");
        printf("\n");
    }
}

void print_Telemetry_Data_Set_Help(bool shortHelp)
{
    printf("\t--%s [1 | 2 | 3 | 4 (SAS only)]\n", TELEMETRY_DATA_AREA_LONG_OPT_STRING);

    if (!shortHelp)
    {
        printf("\t\tThis is a sub-command which defines the amount of data \n");
        printf("\t\tcollected by the --%s option. Data Area 3 is assumed \n", GET_TELEMETRY_LONG_OPT_STRING);
        printf("\t\tif this option is not used. \n");
        printf("\n");
        printf("\t\tSupported Data Areas:\n");
        printf("\t\t1 - get minimal telemetry data (data set 1)\n");
        printf("\t\t2 - get additional/medium telemetry data (data set 2)\n");
        printf("\t\t3 - get additional/large telemetry data (data set 3) (default)\n");
        printf("\t\t4 - get additional/x-large telemetry data (data set 4) (SAS only)\n");
        printf("\n");
    }
}

// NOTE: This is legacy for SeaChest_NVMe only!
void print_NVMe_Get_Tele_Help(bool shortHelp)
{
    printf("\t--%s [host | ctrl]\n", GET_TELEMETRY_LONG_OPT_STRING);

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

    printf("\t--%s [1 | 2 | 3]\n", TELEMETRY_DATA_AREA_LONG_OPT_STRING);

    if (!shortHelp)
    {
        printf("\t\tThis is a sub-command which defines the amount of data \n");
        printf("\t\tcollected by the --%s option. Data Area 3 is assumed \n", GET_TELEMETRY_LONG_OPT_STRING);
        printf("\t\tif this option is not used. \n");
        printf("\n");
        printf("\t\tSupported Data Area.\n");
        printf("\t\t1 - get minimal telemetry data\n");
        printf("\t\t2 - get telemetry data additional to data area 2\n");
        printf("\t\t3 - get telemetry data additional to data area 3 (default data area)\n");
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
        printf("\t\tWARNING: The EPC settings may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Changing Read look-ahead may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Changing NV Cache may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Changing Write Cache may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
    printf("\t--%s [info | value | default] (SATA Only)\n", SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the read command timer value for\n");
        printf("\t\tsynchronous commands and NCQ commands with in-order data\n");
        printf("\t\tdelivery enabled. Note: this timer starts at the time that\n");
        printf("\t\tthe drive processes the command, not the time it is received.\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tUse the \"info\" argument to get the current status\n");
        printf("\t\tof the read timer. A value of 0 means that all possible\n");
        printf("\t\terror recovery will be performed before returning status.\n");
        printf("\t\tOther values should include a unit to know the time to use.\n");
        printf("\t\tIf no unit is provided, it is assumed to be the value * 100 ms\n");
        printf("\t\tEx1: --%s 15s for a 15 second timer.\n", SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
        printf("\t\tEx2: --%s 15000ms for a 15 second timer expressed in milliseconds\n",
               SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
        printf("\t\tEx2: --%s 150 for a 15 second timer with no units specified\n",
               SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING);
        printf("\t\tThe maximum time that can be specified is 1 hour, 49 minutes, 13 seconds\n");
        printf("\t\tUsing the \"default\" argument restores default settings.\n");
        printf("\t\tNote: On some SAT HBAs/bridges, status will not be able to be\n");
        printf("\t\tdetermined due to HBA/bridge limitations.\n\n");
    }
}

void print_SCT_Error_Recovery_Write_Help(bool shortHelp)
{
    printf("\t--%s [info | value | default] (SATA Only)\n", SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the write command timer value for\n");
        printf("\t\tsynchronous commands and NCQ commands with in-order data\n");
        printf("\t\tdelivery enabled. Note: this timer starts at the time that\n");
        printf("\t\tthe drive processes the command, not the time it is received.\n");
        printf("\t\tWhen using this option, the setting is non-volatile.\n");
        printf("\t\tUse this with the --%s flag to make the\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\tsetting volatile.\n");
        printf("\t\tUse the \"info\" argument to get the current status\n");
        printf("\t\tof the write timer. A value of 0 means that all possible\n");
        printf("\t\terror recovery will be performed before returning status.\n");
        printf("\t\tOther values should include a unit to know the time to use.\n");
        printf("\t\tIf no unit is provided, it is assumed to be the value * 100 ms\n");
        printf("\t\tEx1: --%s 15s for a 15 second timer.\n", SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING);
        printf("\t\tEx2: --%s 15000ms for a 15 second timer expressed in milliseconds\n",
               SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING);
        printf("\t\tEx2: --%s 150 for a 15 second timer with no units specified\n",
               SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING);
        printf("\t\tThe maximum time that can be specified is 1 hour, 49 minutes, 13 seconds\n");
        printf("\t\tUsing the \"default\" argument restores default settings.\n");
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
        printf("\t\tNOTE: SATA phy speed changes are only available on Seagate drives.\n");
        printf("\n");
        printf("\t\tWARNING: Check the minimum phy speed supported by your adapter before\n");
        printf("\t\t         using this option. A phy speed below the adapter's capability\n");
        printf("\t\t         will result in the drive not being seen by the adapter or the OS.\n\n");
        printf("\t\tWARNING: Changing Phy speed may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\t\t   See --%s and --%s options for Two DWords\n", SAS_PHY_DWORD_CONTROL_LONG_OPT_STRING,
               SAS_PHY_DWORDS_LONG_OPT_STRING);
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
    printf("\t--%s [NO_SCRAMBLING | 5THCONTROL | 1STCONTROL | 1ST&5THCONTROL]\n",
           SAS_PHY_DWORD_CONTROL_LONG_OPT_STRING);
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
    printf("\t--%s [hex value | D10.2 | D21.5 | D24.3 | D25.6&D6.1 | D30.3 | ALIGN_0 | ALIGN_1 | PAIR_ALIGN_0]\n",
           SAS_PHY_DWORDS_LONG_OPT_STRING);
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
        printf("\t\tWARNING: Changing SAS Phy partial may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Changing SAS Phy slumber may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\t\t      screen as stdout.\n");
        printf("\t\t\tbin - Pulls log & saves it to\n");
        printf("\t\t\t      a timestamped binary file. (default)\n");
        printf("\t\t\tpipe - Pulls log, prints it to the\n");
        printf("\t\t\t       screen as stdout & send the\n");
        printf("\t\t\t       result to openSeaChest_LogParser.\n");
        printf("\t\t\t       (available for FARM only)\n\n");
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
        printf("\t\tWARNING: Some erase methods may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Some erase methods may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Format_Unit_Help(bool shortHelp)
{
    printf("\t--%s [current | new sector size]\t(SAS Only)\t(Clear)\n", FORMAT_UNIT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will start a format unit operation on a SAS drive\n");
        printf("\t\tUse \"current\" to perform a format unit operation with the\n");
        printf("\t\tSector size currently being used, otherwise enter a new sector\n");
        printf("\t\tsize to use upon format completion. This command will erase all\n");
        printf("\t\tdata on the drive. Combine this option with --%s to poll\n", POLL_LONG_OPT_STRING);
        printf("\t\tfor progress until the format is complete.\n");
        printf("\t\tChanging sector sizes is intended for supported Seagate products\n");
        printf("\t\tused in some hardware RAID configurations. Please consult your\n");
        printf("\t\thardware RAID documentation for information about compatibility and\n");
        printf("\t\tsupported/required sector sizes!\n\n");
        printf("\t\tWARNING: Format Unit may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
        printf("\t\tWARNING: Customer unique firmware may have specific requirements that \n");
        printf("\t\t         restrict sector sizes on some products. It may not be possible to format/ \n");
        printf("\t\t         fast format to common sizes like 4K or 512B due to these customer requirements.\n\n");
    }
}

void print_Fast_Format_Help(bool shortHelp)
{
    printf("\t--%s [fast format mode]\t(SAS Only) (SBC4 required)\n", FAST_FORMAT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option with the --%s option\n", FORMAT_UNIT_LONG_OPT_STRING);
        printf("\t\tto run a fast format.\n");
        printf("\t\tChanging sector sizes is intended for supported Seagate products\n");
        printf("\t\tused in some hardware RAID configurations. Please consult your\n");
        printf("\t\thardware RAID documentation for information about compatibility and\n");
        printf("\t\tusing 4K native sectors before using this option!\n");
        printf("\t\tSoftware RAID or individual/JBOD drive solutions will see no benefit as modern\n");
        printf("\t\tfile systems and modern operating systems are already 4K aware even on\n");
        printf("\t\t512 emulation drives. Modern operating systems already align file systems to 4K\n");
        printf("\t\tboundaries required by these drives for optimal performance.\n");
        printf("\t\tPerforming a sector size change is data destructive and has a risk that\n");
        printf("\t\tthe adapter, driver, or operating system may not know how to communicate with\n");
        printf("\t\tthe device once this has completed.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
        printf("\t\tThere is an additional risk when performing a low-level fast format that may\n");
        printf("\t\tmake the drive inoperable if it is reset at any time while it is formatting.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
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
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_COLOR_DEFAULT);
        printf("\t\tWARNING: Any interruption to the device while it is formatting may render the\n");
        printf("\t\t         drive inoperable! Use this at your own risk!\n");
        printf("\t\tWARNING: Set sector size may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n");
        printf("\t\tWARNING: Disable any out-of-band management systems/services/daemons\n");
        printf("\t\t         before using this option. Interruptions can be caused by these\n");
        printf("\t\t         and may prevent completion of a sector size change.\n");
        printf("\t\tWARNING: It is recommended that this operation is done from a bootable environment\n");
        printf("\t\t         (Live USB) to reduce the risk of OS background activities running and\n");
        printf("\t\t         triggering a device reset while reformating the drive.\n\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
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
        printf("\t\t       and --%s options will be used. All others are\n",
               FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG_LONG_OPT_STRING);
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
    printf("\t--%s [ new max LBA ]\n", FORMAT_UNIT_NEW_MAX_LBA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a new Max LBA for a drive during a\n");
        printf("\t\tformat unit operation. This may speed up a format unit if\n");
        printf("\t\tformatting to test something, or also desiring to reduce a drive's\n");
        printf("\t\tcapacity while formatting.\n");
        printf("\t\tNOTE: Not all devices support reducing capacity during a format.\n");
        printf("\t\tSome may ignore this parameter and format the full medium anyways.\n");
        printf("\t\tThis is not guaranteed to stick or reduce formatting time.\n\n");
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
    printf("\t--%s [new sector size]\n", SET_SECTOR_SIZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tChanging sector sizes is intended for supported Seagate products\n");
        printf("\t\tused in some hardware RAID configurations. Please consult your\n");
        printf("\t\thardware RAID documentation for information about compatibility and\n");
        printf("\t\tusing 4K native sectors before using this option!\n");
        printf("\t\tSoftware RAID or individual/JBOD drive solutions will see no benefit as modern\n");
        printf("\t\tfile systems and modern operating systems are already 4K aware even on\n");
        printf("\t\t512 emulation drives. Modern operating systems already align file systems to 4K\n");
        printf("\t\tboundaries required by these drives for optimal performance.\n");
        printf("\t\tPerforming a sector size change is data destructive and has a risk that\n");
        printf("\t\tthe adapter, driver, or operating system may not know how to communicate with\n");
        printf("\t\tthe device once this has completed.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
        printf("\t\tThere is an additional risk when performing a low-level format/fast format that may\n");
        printf("\t\tmake the drive inoperable if it is reset at any time while it is formatting.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
        printf("\t\tFor SATA Drives, the set sector configuration command must be supported.\n");
        printf("\t\tOn SAS Drives, fast format must be supported to make these changes.\n\n");
        printf("\t\tUse the --%s option to see the sector\n", SHOW_SUPPORTED_FORMATS_LONG_OPT_STRING);
        printf("\t\tsizes the drive reports supporting. If this option\n");
        printf("\t\tdoesn't list anything, please consult your product manual.\n");
        printf("\t\tThis option should be used to quickly change between 5xxe and\n");
        printf("\t\t4xxx sector sizes. Using this option to change from 512 to 520\n");
        printf("\t\tor similar is not recommended at this time due to limited drive\n\t\tsupport\n\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_COLOR_DEFAULT);
        printf("\t\tWARNING: Any interruption to the device while it is formatting may render the\n");
        printf("\t\t         drive inoperable! Use this at your own risk!\n");
        printf("\t\tWARNING: Set sector size may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n");
        printf("\t\tWARNING (SATA): Do not interrupt this operation once it has started or \n");
        printf("\t\t         it may cause the drive to become unusable. Stop all possible background\n");
        printf("\t\t         activity that would attempt to communicate with the device while this\n");
        printf("\t\t         operation is in progress\n");
        printf("\t\tWARNING: It is not recommended to do this on USB as not\n");
        printf("\t\t         all USB adapters can handle a 4k sector size.\n");
        printf("\t\tWARNING: Disable any out-of-band management systems/services/daemons\n");
        printf("\t\t         before using this option. Interruptions can be caused by these\n");
        printf("\t\t         and may prevent completion of a sector size change.\n");
        printf("\t\tWARNING: It is recommended that this operation is done from a bootable environment\n");
        printf("\t\t         (Live USB) to reduce the risk of OS background activities running and\n");
        printf("\t\t         triggering a device reset while reformating the drive.\n\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
    }
}

void print_Force_Help(bool shortHelp)
{
    printf("\t--%s\n", FORCE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse the --%s option to attempt to override and force a specific\n", FORCE_LONG_OPT_STRING);
        printf("\t\toperation on a drive in case it is returning \"Not supported\"\n");
        printf("\t\tmessages. This can be used to override some checks for command or\n");
        printf("\t\tfeature support. Be aware that sending unsupported commands may\n");
        printf("\t\tresult in command failures, and in some circumstances, it may also\n");
        printf("\t\tcause indeterminate behavior of a device.\n");
        printf("\t\tDo not use this command unless you are certain that a device supports\n");
        printf("\t\tthe command or feature you are attempting to use.\n");
        printf("\t\tThis option is not guaranteed to make things work or fix issues. This\n");
        printf("\t\toption is not available to override every support check or other\n");
        printf("\t\tincompatibility check in the software.\n\n");
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
        printf("\t\tWARNING: Customer unique firmware may have specific requirements that \n");
        printf("\t\t         restrict sector sizes on some products. It may not be possible to format/ \n");
        printf("\t\t         fast format to common sizes like 4K or 512B due to these customer requirements.\n\n");
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
        printf("\t\toperations.\n");
        printf("\t\tOn Seagate drives, PSIDs are 32 digits long, all uppercase,\n");
        printf("\t\tand uses zeros and ones but do NOT use O's and I's.\n");
        printf("\t\tAdditionally, it is possible to exhaust the number of attempts\n");
        printf("\t\t the device allows. Seagate drives have this set to 5 attempts.\n");
        printf("\t\tOnce this is exhausted, a full power cycle of the device is required\n");
        printf("\t\tbefore you can try again.\n\n");
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
    printf("\t--%s\n", DISABLE_DATA_LOCKING_LONG_OPT_STRING);
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

void print_WRV_Help(bool shortHelp)
{
    printf("\t--%s [ info | all | vendor | # | disable ]\t(SATA Only)\n", WRV_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option can report the current configuration of the\n");
        printf("\t\tATA Write-Read-Verify feature, enable the feature, or\n");
        printf("\t\tdisable the feature.\n");
        printf("\t\tEnabling this feature instructs the device to perform a\n");
        printf("\t\tverification of all data after it has been written.\n");
        printf("\t\tEnabling this may result in lower device performance.\n");
        printf("\t\tIf write caching is enabled, this feature may return\n");
        printf("\t\tcompletion before writing to the medium and verifying\n");
        printf("\t\tthe medium. If Write caching is disabled, the write and\n");
        printf("\t\tverification must complete before returning command status.\n");
        printf("\t\tArgument usage:\n");
        printf("\t\t  info    - Display the current status of the feature\n");
        printf("\t\t  all     - set verification on for all written sectors\n");
        printf("\t\t  vendor  - set verification for the 1st vendor specific\n");
        printf("\t\t            number of sectors.\n");
        printf("\t\t  #       - Perform verification for the first user defined\n");
        printf("\t\t            number of sectors. Note: This is rounded up to the\n");
        printf("\t\t            nearest 1024 sectors. Max value of 261120 sectors.\n");
        printf("\t\t  disable - disable the Write-Read-Verify feature.\n\n");
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
    printf("\t--%s [ info | spinup | enable | disable ]  (SATA Only)\n", PUIS_FEATURE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to enable or disable the power up in standby\n");
        printf("\t\t(PUIS) feature on SATA drives. \n");
        printf("\t\tArguments:\n");
        printf("\t\t    info    - display information about the PUIS support on the device\n");
        printf("\t\t    spinup  - issue the PUIS spinup set features command to spin up\n");
        printf("\t\t              the device to active/idle state.\n");
        printf("\t\t    enable  - enable the PUIS feature using setfeatures command\n");
        printf("\t\t    disable - disable the PUIS feature using setfeatures command\n");
        printf("\t\tNote: If this is configured on the drive with a jumper, this\n");
        printf("\t\t      command will fail.\n");
        printf("\t\tNote2: Not all products support this feature.\n");
        printf("\t\tWARNING: Before enabling this feature on any SAS/SATA HBA,\n");
        printf("\t\t         check the HBA documentation to see if this feature\n");
        printf("\t\t         is supported by the HBA. Enabling this on an HBA that\n");
        printf("\t\t         does not support this feature will cause the drive to\n");
        printf("\t\t         stop showing up to the host OS or even in the HBA's\n");
        printf("\t\t         firmware/BIOS/UEFI configuration.\n\n");
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
        printf("\t\tWARNING: Changing MRIE may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
    if (!shortHelp)
    {
        printf("\t\tUsing this option will force the current drive to\n");
        printf("\t\tbe treated as a SCSI drive. Only SCSI commands will\n");
        printf("\t\tbe used to talk to the drive.\n\n");
    }
}

void print_Force_ATA_Help(bool shortHelp)
{
    printf("\t--%s\n", FORCE_ATA_LONG_OPT_STRING);
    if (!shortHelp)
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

void print_Reinitialize_Device_Statistics_Help(bool shortHelp)
{
    printf("\t--%s [", REINITIALIZE_DEV_STATS_LONG_OPT_STRING);
    const char* opts[] = REINITIALIZE_DEV_STATS_ARG_OPTIONS;
    for (size_t optit = SIZE_T_C(0); optit < SIZE_OF_STACK_ARRAY(opts); ++optit)
    {
        printf(" %s", opts[optit]);
        if (optit + SIZE_T_C(1) < SIZE_OF_STACK_ARRAY(opts))
        {
            printf(" |");
        }
    }
    printf("] (SATA Only)\n");
    if (!shortHelp)
    {
        printf("\t\tUse this option to reinitialize supported device statistics\n");
        printf("\t\tto their default values. Supported statistics are indicated in\n");
        printf("\t\tthe output of the --%s option.\n", DEVICE_STATISTICS_LONG_OPT_STRING);
        printf("\t\tArguments to this option can be used to reinitialize specific pages or\n");
        printf("\t\tall supported pages.\n\n");
    }
}

void print_Reinitialize_SATA_Phy_Events_Help(bool shortHelp)
{
    printf("\t--%s (SATA Only)\n", REINITIALIZE_SATA_PHY_EVENTS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to reinitialize the SATA Phy event counters\n");
        printf("\t\tlog page back to their default values. This can be useful when\n");
        printf("\t\tdebugging cabling issues.\n\n");
    }
}

void print_Set_Timestamp_Help(bool shortHelp)
{
    printf("\t--%s\n", SET_TIMESTAMP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the date and time timestamp on the device\n");
        printf("\t\tto the time based on your current system clock.\n\n");
    }
}

void print_Zone_ID_Help(bool shortHelp)
{
    printf("\t--%s [LBA/zone ID | all]\n", ZONE_ID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a zone ID for use with other options\n");
        printf("\t\tsuch as --%s, --%s, --%s, --%s, --%s.", REPORT_ZONES_LONG_OPT_STRING, CLOSE_ZONE_LONG_OPT_STRING,
               FINISH_ZONE_LONG_OPT_STRING, OPEN_ZONE_LONG_OPT_STRING, RESET_WP_LONG_OPT_STRING);
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
        printf("\t\tEx: --%s empty --%s 123456 --%s 30\n", REPORT_ZONES_LONG_OPT_STRING, ZONE_ID_LONG_OPT_STRING,
               MAX_ZONES_LONG_OPT_STRING);
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
        printf("\t\t*allNonWP - show all non-write pointer zones.\n\n");
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
        printf("\t\tUse this option to reset a write pointer at the zone specified with the --%s option\n\n",
               ZONE_ID_LONG_OPT_STRING);
    }
}

void print_FWDL_Segment_Size_Help(bool shortHelp)
{
    printf("\t--%s [segment size in 512B blocks]\n", FWDL_SEGMENT_SIZE_LONG_OPT_STRING);
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

void print_FWDL_Ignore_Final_Segment_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_LOCKED_REGIONS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option should only be used when performing firmware\n");
        printf("\t\tupdates on legacy products. What this does is it ignores\n");
        printf("\t\ta failing error code from the OS on the final segment of a\n");
        printf("\t\tfirmware update, but this update is actually successful.\n");
        printf("\t\tThis is needed to workaround hardware or firmware limitations\n");
        printf("\t\tthat were present in some old products.\n\n");
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

void print_Error_In_Cmd_Line_Args(const char* optstring, const char* arg)
{
    printf("\nError in option --%s. Invalid argument given '%s'.\n", optstring, arg);
    printf("Use -h option to view command line help\n");
}

void print_Error_In_Cmd_Line_Args_Short_Opt(char opt, const char* arg)
{
    printf("\nError in option -%c. Invalid argument given '%s'.\n", opt, arg);
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

// Yes, this function uses a tripple pointer and is complicated.
// The idea is there is no limit to a number of device handles given on a command line.
// Because there is no limit, we need to allocate/reallocate heap memory to add more handles (as strings) to the list.
// In order to have a list of strings, I had to use a double pointer from the caller and pass an address to this 2D
// array, hence the tripple pointer. I do not recommend touching this function unless you have to. It should be capable
// of accepting any type of handle input to it. Windows is likely the only OS to need changes (see comment in code
// below).
int parse_Device_Handle_Argument(char*     optarg,
                                 bool*     allDrives,
                                 bool*     userHandleProvided,
                                 uint32_t* deviceCount,
                                 char***   handleList)
{
    if (!optarg || !allDrives || !userHandleProvided || !deviceCount)
    {
        return 254; // one of the required parameters is missing. handleList is checked below...
    }
    /*get the number out of optarg to tack onto the device handle*/
    if (M_NULLPTR != optarg)
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
#    define WINDOWS_MAX_HANDLE_STRING_LENGTH 50
            DECLARE_ZERO_INIT_ARRAY(char, windowsHandle, WINDOWS_MAX_HANDLE_STRING_LENGTH);
            char* deviceHandle = &windowsHandle[0];
            char* physicalDeviceNumber; /*making this a string in case the handle is two or more digits long*/
            /*make sure the user gave us "PD" for the device handle...*/
            if (strncasecmp(optarg, "PD", 2) == 0)
            {
                physicalDeviceNumber = strpbrk(optarg, "0123456789");
                snprintf_err_handle(deviceHandle, WINDOWS_MAX_HANDLE_STRING_LENGTH, "\\\\.\\PhysicalDrive%s",
                                    physicalDeviceNumber);
            }
#    if defined(ENABLE_CSMI)
            else if (strncmp(optarg, "csmi", 4) == 0)
            {
                snprintf_err_handle(deviceHandle, WINDOWS_MAX_HANDLE_STRING_LENGTH, "%s", optarg);
            }
#    endif
            else if (strncmp(optarg, "\\\\.\\", 4) == 0)
            {
                snprintf_err_handle(deviceHandle, WINDOWS_MAX_HANDLE_STRING_LENGTH, "%s", optarg);
            }
            /*If we want to add another format for accepting a handle, then add an else-if here*/
            else /*we have an invalid handle*/
            {
                printf("Error: %s is an invalid handle format for this tool.\n", optarg);
                exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
            }
#else
            char* deviceHandle = optarg;
#endif

            ++(*deviceCount); /*increment this variable if we've made it this far.*/
            /*The code below is where this function gets complicated. Be very careful changing anything below this
             * comment.*/
            if (!*handleList)
            {
                /*allocate the list and add this handle to it.*/
                *handleList = M_REINTERPRET_CAST(char**, safe_calloc((*deviceCount), sizeof(char*)));
                if (!*handleList)
                {
                    perror("error allocating memory for handle list\n");
                    return 255;
                }
            }
            else
            {
                /*list already allocated, so reallocate and add this next handle to it.*/
                char** temp = M_REINTERPRET_CAST(
                    char**, safe_reallocf(C_CAST(void**, handleList), (*deviceCount) * sizeof(char*)));
                if (temp == M_NULLPTR)
                {
                    perror("error reallocating memory for handle list\n");
                    return 255;
                }
                *handleList = temp;
            }
            /*the list has been allocated, now put the handle we've received into the list*/
            /*start by allocating memory for the handle at the new list location*/
            size_t handleListNewHandleLength = safe_strlen(deviceHandle) + 1;
            (*handleList)[(*deviceCount) - 1] =
                M_REINTERPRET_CAST(char*, safe_calloc(handleListNewHandleLength, sizeof(char)));
            if (!(*handleList)[(*deviceCount) - 1])
            {
                perror("error allocating memory for adding device handle to list\n");
                return 255;
            }
            /*copy the handle into memory*/
            snprintf_err_handle((*handleList)[(*deviceCount) - 1], handleListNewHandleLength, "%s", deviceHandle);
        }
    }
    return 0;
}

void free_Handle_List(char*** handleList, uint32_t listCount)
{
    if (*handleList)
    {
        for (uint32_t handleIter = UINT32_C(0); handleIter < listCount; ++handleIter)
        {
            safe_free(&(*handleList)[handleIter]);
        }
        safe_free(M_REINTERPRET_CAST(void**, &(*handleList)));
    }
}

#if defined(ENABLE_CSMI)
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
    printf("\t--%s (Obsolete)\n", CSMI_VERBOSE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is obsolete and will be removed in future versions.\n\n");
    }
}

void print_CSMI_Force_Flags_Help(bool shortHelp)
{
    printf("\t--%s (Obsolete)\n", CSMI_FORCE_IGNORE_PORT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is obsolete and will be removed in future versions.\n\n");
    }

    printf("\t--%s (Obsolete)\n", CSMI_FORCE_USE_PORT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is obsolete and will be removed in future versions.\n\n");
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
        printf("\t\tof the depopulation operation.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
        printf("\t\tThere is an additional risk when performing a remove physical element as it low-level formats\n");
        printf("\t\tthe drive and may make the drive inoperable if it is reset at any time while it is formatting.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
        printf("\t\tWARNING: Removing a physical element affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Depop_MaxLBA_Help(bool shortHelp)
{
    printf("\t--%s [requested MaxLBA]\n", DEPOP_MAX_LBA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a new maximum LBA when\n");
        printf("\t\tremoving (depopulating) a physical storage element.\n");
        printf("\t\tThis is optional. If this is not specified, the device\n");
        printf("\t\twill determine the new maximum LBA.\n");
        printf("\t\tNOTE: If you specify a maximum LBA the device does not\n");
        printf("\t\tsupport, it will not start the depopulation.\n\n");
    }
}

void print_Repopulate_Elements_Help(bool shortHelp)
{
    printf("\t--%s\n", REPOPULATE_ELEMENTS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to repopulate any physical storage\n");
        printf("\t\telements that have been removed from use.\n");
        printf("\t\tA full disk overwrite is necessary before\n");
        printf("\t\tthe drive is usable.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_BRIGHT_RED, CONSOLE_COLOR_DEFAULT);
        printf("\t\tThere is an additional risk when performing a repopulate as it low-level formats\n");
        printf("\t\tthe drive and may make the drive inoperable if it is reset at any time while it is formatting.\n");
        set_Console_Foreground_Background_Colors(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT);
        printf("\t\tWARNING: Removing a physical element affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
        printf("\n");
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
    printf("\t--%s [ info | enable | disable | limited]\t (Seagate Only)\n", SEAGATE_POWER_BALANCE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to see the state of the Seagate Power Balance\n");
        printf("\t\tfeature or to change its state.\n");
        printf("\t\tSeagate's PowerBalance feature will adjust drive performance during\n");
        printf("\t\trandom operations to reduce power consumption of the drive.\n");
        printf("\t\t  info - will dump the state of the Power Balance feature on the screen\n");
        printf("\t\t  enable - use this to enable Power Balance (lowest power consumption)\n");
        printf("\t\t  disable - use this to disable Power Balance (hihgest power consumption)\n");
        printf("\t\t  limited - 12w limited mode. Dual actuator SATA only\n");
        printf("\t\tNote: While this feature is available on some SAS products,\n");
        printf("\t\tit is recommended that the --%s option is\n", SET_POWER_CONSUMPTION_LONG_OPT_STRING);
        printf("\t\tused instead since it allows more levels of control.\n");
        printf("\t\tThis option and the --%s option are incompatible\n", SET_POWER_CONSUMPTION_LONG_OPT_STRING);
        printf("\t\tbecause they use the same mode page fields (1Ah-01h).\n\n");
        printf("\t\tWARNING: Seagate Power Balance may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tUse the --%s option to specify the mode to display the defects.\n",
               SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING);
        printf("\t\tIf no mode is specified, physical cylinder-head-sector mode is assumed\n");
        printf("\t\tArguments:\n");
        printf("\t\t  p - use this option to pull and display the primary (factory) defects\n");
        printf("\t\t  g - use this option to pull and display the grown (reallocated) defects\n");
        printf("\t\t The above options can be combined to pull and display both defect lists.\n\n");
    }
}

void print_SCSI_Defects_Format_Help(bool shortHelp)
{
    printf("\t--%s [ # | shortBlock | longBlock | xbfi | xchs | bfi | chs ]\t(SAS Only)\n",
           SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING);
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

void print_Show_Concurrent_Position_Ranges_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_CONCURRENT_RANGES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to display the concurrent positioning ranges\n");
        printf("\t\tsupported by a device. Concurrent positioning ranges are used\n");
        printf("\t\tto inform which actuator is used for a given range in LBA space.\n\n");
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

void print_Log_Length_Help(bool shortHelp)
{
    printf("\t--%s [length in bytes]\t(NVMe Only)\n", LOG_LENGTH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the total length of a log\n");
        printf("\t\tto retrieve from a device. This is required for NVMe\n");
        printf("\t\tlogs not part of the standards or not currently known\n");
        printf("\t\tby this utility in order to retrieve all the data.\n");
        printf("\t\tThe following post fixes are allowed for\n");
        printf("\t\tspecifying a transfer length:\n");
        printf("\t\t\tBLOCKS or SECTORS - used to specify a transfer length\n");
        printf("\t\t\t\tin device in 512Byte blocks/sectors\n");
        printf("\t\t\tKB - length in kilobytes (val * 1000)\n");
        printf("\t\t\tKiB - length in kibibytes (val * 1024)\n");
        printf("\t\t\tMB - length in megabytes (val * 1000000)\n");
        printf("\t\t\tMiB - length in mebibytes (val * 1048576)\n");
        printf("\n");
    }
}

void print_FARM_Log_Help(bool shortHelp)
{
    printf("\t--%s\n", FARM_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tPull the Seagate Field Accessible Reliability Metrics (FARM)\n");
        printf("\t\tLog from the specified drive. Saves the binary logs to the\n");
        printf("\t\tcurrent directory as <serialnumber>FARM<date and time>.bin (as default)\n\n");
    }
}

void print_FARM_Combined_Log_Help(bool shortHelp)
{
    printf("\t--%s\n", FARM_COMBINED_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tPull the Seagate Combined Field Accessible Reliability Metrics (FARM)\n");
        printf("\t\tLog from the specified drive. This log contains a combination of all\n");
        printf("\t\tFARM Sub Log Pages in a single Log File.Saves the binary logs to the\n");
        printf("\t\tcurrent directory as <serialnumber>FARMC<date and time>.FRMC\n\n");
    }
}

void print_Sata_FARM_Copy_Type_Flag_Help(bool shortHelp)
{
    printf("\t--%s [ disc | flash ]\t(SATA Only)\n", SATA_FARM_COPY_TYPE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to provide copy type while extracting FARM copy type with --%s\n",
               FARM_COMBINED_LONG_OPT_STRING);
        printf("\t\toption. The default mode is \"disc\"\n");
        printf("\t\t    disc - Pull Disc copy of SATA Farm logs.\n");
        printf("\t\t    flash - Pull Flash copy of SATA Farm logs.\n");
        printf("\n");
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
        printf("\t\t      will be missing to describe the operation of certain commands.\n");
        printf("\n");
    }
}

void print_SMART_Error_Log_Format_Help(bool shortHelp)
{
    printf("\t--%s [ raw | detailed ]\t(SATA Only)\n", SMART_ERROR_LOG_FORMAT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to change the format of the output from the --%s\n",
               SHOW_SMART_ERROR_LOG_LONG_OPT_STRING);
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

void print_ATA_Security_Erase_Help(bool shortHelp, const char* password)
{
    printf("\t--%s [normal | enhanced]\t(SATA only)\t(Clear | Purge)\n", ATA_SECURITY_ERASE_OP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse \"normal\" to start a standard ATA security erase (Clear)\n");
        printf("\t\tor \"enhanced\" to start an enhanced ATA security erase (Purge).\n\n");
        printf("\t\tATA Security Erase takes a very long time to complete at\n");
        printf("\t\tapproximately three (3) hours per Tera-byte (HDD). Some Seagate\n");
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
        printf("\t\t* normal writes binary zeros (0) or ones (1) to all user\n");
        printf("\t\tdata areas.\n\n");
        printf("\t\t* enhanced will fill all user data areas and reallocated\n");
        printf("\t\tuser data with a vendor specific pattern. Some Seagate\n");
        printf("\t\tInstant Secure Erase will perform a cryptographic\n");
        printf("\t\terase instead of an overwrite.\n\n");
    }
}

void print_Disable_ATA_Security_Password_Help(bool shortHelp, const char* utilName)
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
        printf("\t\t--%s to specify whether it is the user or master password.\n",
               ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
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
    printf("byteswapped | zeropad | spacepad | fpad | leftAlign | rightAlign | uppercase | lowercase | invertcase] "
           "(SATA Only)\n");
    if (!shortHelp)
    {
        printf("\t\tUse this option to have the utility make modifications to\n");
        printf("\t\tthe ATA security password to attempt other various ways it may\n");
        printf("\t\tbe sent by a system bios. These are not guaranteed to work, but\n");
        printf("\t\tmay help unlock a drive that was locked by a BIOS that encoded\n");
        printf("\t\tthe password in a unique way.\n");
        printf("\t\tThis option can be presented multiple times to select multiple modificaitons.\n");
        printf("\t\tEX: --%s byteswapped --%s invertcase\n", ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING,
               ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING);
#if defined MD5_PASSWORD_SUPPORTED
        printf("\t\t  md5 - sends a md5 sum of the password as the password\n"); // Internal SeaChest only since this
                                                                                 // requires mbedtls library!!!
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
#if defined(_WIN32)
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
        printf("\t\t--%s option to set an ATA security password on a drive.\n",
               ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
        printf("\t\tIf setting the master password, it is strongly recommended\n");
        printf("\t\tthat a master password identifier is specified with the\n");
        printf("\t\t--%s option as well.\n", ATA_SECURITY_MASTER_PW_ID_LONG_OPT_STRING);
        printf("\t\tThe --%s option can be provided to set the drive to\n",
               ATA_SECURITY_MASTER_PW_CAPABILITY_LONG_OPT_STRING);
        printf("\t\tmaximum security mode when setting the user password.\n\n");
    }
}

void print_ATA_Security_Unlock_Help(bool shortHelp)
{
    printf("\t--%s\t\t(SATA only)\n", ATA_SECURITY_UNLOCK_OP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option along with the --%s option and\n", ATA_SECURITY_PASSWORD_LONG_OPT_STRING);
        printf("\t\t--%s option to unlock a drive with the provided password.\n",
               ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING);
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
        printf("\t\t(SP) Subpage FFh specifies all subpages of a given page and will reset all those subpages.\n");
        printf("\t\tUsing both MP 3Fh and SP FFh will reset all pages and subpages on a device.\n\n");
        printf("\t\tWARNING: Resetting mode pages may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Restoring mode pages may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Saving mode pages may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
    printf("\t--%s [cumulative | threshold | defCumulative | defThreshold | all]\t\t(SAS only)\n",
           SCSI_RESET_LP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to reset all SCSI Log Pages.\n");
        printf("\t\tIf the device is compliant with SPC4 or later, the\n");
        printf("\t\t--%s option may be used to specify a specific page to reset.\n",
               SCSI_RESET_LP_PAGE_LONG_OPT_STRING);
        printf("\t\tThe --%s option may also be passed to prevent saving changes.\n", VOLATILE_LONG_OPT_STRING);
        printf("\t\t    cumulative - reset the cumulative values\n");
        printf("\t\t    threshold  - reset the threshold values\n");
        printf("\t\t    defCumulative - reset the cumulative values to default without saving.\n");
        printf("\t\t    defThreshold  - reset the threshold values to default without saving.\n");
        printf("\t\t    all - sends the log page reset command to all of the above control values\n\n");
        printf("\t\tWARNING: Resetting log pages may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tWARNING: Resetting log pages may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
    }
}

void print_Set_SCSI_MP_Help(bool shortHelp)
{
    printf("\t--%s [ mp[-sp]:byte:highestBit:fieldWidthInBits=value | file=filename.txt ]\t(SAS only)\n",
           SCSI_SET_MP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set a specific field in a mode page to a value.\n");
        printf("\t\tThere are two argument formats to this option:\n");
        printf("\t\t1. The first format expects a mode page (in hex), optionally a subpage code (in hex),\n");
        printf("\t\t   the byte offset that the field starts at (in decimal), the highest bit the field starts\n");
        printf("\t\t   at (0-7), the width of the field in as a number of bits (decimal), and the value to set (hex or "
               "decimal)\n");
        printf("\t\t   A maximum of 64bits can be set at a time with this option.\n");
        printf("\t\t2. The second format is a text file that contains all bytes of the mode page in hex. Each byte\n");
        printf("\t\t   must be separated by a space, new line, or underscore. It is recommended that this file\n");
        printf(
            "\t\t   is created by copy-pasting the output of the --%s option's default classic view, then modifying\n",
            SCSI_SHOW_MP_LONG_OPT_STRING);
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
        printf("\t\tWARNING: Changing mode pages may affect all LUNs/namespaces for devices\n");
        printf("\t\t         with multiple logical units or namespaces.\n\n");
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
        printf("\t\tfor progress until the format is complete.\n");
        printf("\t\tA data sanitization compliant with IEEE 2883 Clear requires the --%s\n",
               NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING);
        printf("\t\toption to be provided. Without this option the controller\n");
        printf("\t\tmay not erase all user data and substitute returning zeroes\n");
        printf("\t\tfor performance instead.\n\n");
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
    printf("\t--%s [none | user | crypto]\t(NVMe Only)\t(None | Clear | Clear, Possible Purge)\n",
           NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to specify the type of erase to perform\n");
        printf("\t\tduring an NVM format operation. All user data will be inaccessible\n");
        printf("\t\tupon completion of an NVM format, no matter the erase requested.\n");
        printf("\t\tOptions:\n");
        printf("\t\t  none - no secure erase requested (previous data will not be accessible,\n");
        printf("\t\t         however the media may not have been erased by the controller.)\n");
        printf("\t\t  user - requests all user data is erased by the device. (Clear)\n");
        printf("\t\t  crypto - requests a cryptographic erase of all user data. Note: this mode\n");
        printf("\t\t    is not supported on all devices. (Clear, Possible Purge)\n\n");
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

void print_No_Banner_Help(bool shortHelp)
{
    printf("\t--%s\n", NO_BANNER_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to suppress the text banner that displays each time\n");
        printf("\t\topenSeaChest is run.\n\n");
    }
}

void print_Show_Power_Telemetry_Help(bool shortHelp)
{
    printf("\t--%s\t(Seagate Only)\n", SHOW_POWER_TELEMETRY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show the power telemetry data from\n");
        printf("\t\ta Seagate drive that supports the power telemetry feature\n");
        printf("\t\tIf a measurement was not previously requested, this will show\n");
        printf("\t\tfree-running mode data from the last 10 minutes.\n");
        printf("\t\tIf this option is provided while a measurement is still\n");
        printf("\t\tin progress, this will show all data that is currently available\n");
        printf("\t\tUse the --%s option to request a power\n", REQUEST_POWER_TELEMETRY_MEASUREMENT_LONG_OPT_STRING);
        printf("\t\tmeasurement with a set time window.\n\n");
        printf("\t\tNOTE: Power measurements are for the full device, not individual\n");
        printf("\t\t      logical units. All logical units will be measured for this data.\n\n");
    }
}

void print_Request_Power_Measurement_Help(bool shortHelp)
{
    printf("\t--%s [seconds to perform measurement]\t(Seagate Only)\n",
           REQUEST_POWER_TELEMETRY_MEASUREMENT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used to specify a time to perform a power\n");
        printf("\t\tmeasurement for. The minimum measurement time is 22 seconds\n");
        printf("\t\tand the maximum is 65535 seconds. If a time less than 22 seconds\n");
        printf("\t\tis provided, 22 seconds will be used by the drive. A value greater\n");
        printf("\t\tthan 65535 will result in error.\n");
        printf("\t\tUse the --%s option to specify which mode to measure.\n\n",
               REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE_LONG_OPT_STRING);
    }
}

void print_Request_Power_Measurement_Mode_Help(bool shortHelp)
{
    printf("\t--%s [all | 5 | 12]\t(Seagate Only)\n", REQUEST_POWER_TELEMETRY_MEASUREMENT_MODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option along with --%s to specify\n", REQUEST_POWER_TELEMETRY_MEASUREMENT_LONG_OPT_STRING);
        printf("\t\twhich sources to measure power on for the requested time.\n");
        printf("\t\t all - measure all power sources\n");
        printf("\t\t 5   - measure only the 5v power\n");
        printf("\t\t 12  - measure only the 12v power\n");
        printf("\n");
    }
}

void print_Pull_Power_Telemetry_Help(bool shortHelp)
{
    printf("\t--%s\t(Seagate Only)\n", PULL_POWER_TELEMETRY_DATA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to pull the power telemetry\n");
        printf("\t\tdata and save it to a binary file.\n\n");
    }
}

void print_Show_Reservation_Capabilities(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS Only)\n", SHOW_RESERVATION_CAPABILITIES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis options shows the persistent reservation\n");
        printf("\t\tcapabilities for a device.\n");
        printf("\t\tNOTE: Older device supporting SPC or SPC2 may not support\n");
        printf("\t\tshowing capabilities, but do support persistent reservations.\n\n");
    }
}

void print_Show_Full_Reservation_Info(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS Only)\n", SHOW_FULL_RESERVATION_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis options reads the persistent reservation full\n");
        printf("\t\tinformation (registrations and reservations) and\n");
        printf("\t\tprints it to the screen.\n\n");
    }
}

void print_Show_Registration_Keys(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS Only)\n", SHOW_REGISTRATION_KEYS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis options reads the persistent reservation\n");
        printf("\t\tregistration keys and prints it to the screen.\n\n");
    }
}

void print_Show_Reservations(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS Only)\n", SHOW_RESERVATIONS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis options reads the persistent reservation\n");
        printf("\t\treservations and prints it to the screen.\n\n");
    }
}

void print_Persistent_Reservations_Key_Help(bool shortHelp)
{
    printf("\t--%s [key]\t(NVMe & SAS Only)\n", PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the key to use for persistent\n");
        printf("\t\treservation options. When registering a key, this provides\n");
        printf("\t\tthe value expected for your use. It should continue to be used\n");
        printf("\t\tthrough all persistent reseve operations until unregistered or\n");
        printf("\t\tcleared.\n\n");
    }
}

void print_Persistent_Reservations_Type_Help(bool shortHelp)
{
    printf("\t--%s [wrex | ex | wrexro | exro | wrexar | exar]\t(NVMe and SAS only)\n",
           PERSISTENT_RESERVATION_TYPE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option specifies the type of reservation to hold.\n");
        printf("\t\tThis is required for acquiring, preempting, and releasing\n");
        printf("\t\treservations.\n");
        printf("\t\tAvailable reservation types:\n");
        printf("\t\t\twrex   - write exclusive\n");
        printf("\t\t\tex     - exclusive access\n");
        printf("\t\t\twrexro - write exclusive, registrants only\n");
        printf("\t\t\texro   - exclusive access, registrants only\n");
        printf("\t\t\twrexar - write exclusive, all registrants\n");
        printf("\t\t\texar   - exclusive access, all registrants\n");
        // NOTE: There are obsolete types available for SAS, but not implementing them since they are long obsolete.
        printf("\n");
    }
}

void print_Persistent_Reservations_All_Target_Ports_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_ATP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option when registering a new key to specify\n");
        printf("\t\tthat it applies to all target ports.\n");
        printf("\t\tNOTE: Not all devices will support this option\n\n");
    }
}

void print_Persistent_Reservations_Persist_Through_Power_Loss_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_PTPL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option when registering a new key to activate\n");
        printf("\t\tthe persist through power loss capability.\n");
        printf("\t\tNOTE: Not all devices will support this option\n\n");
    }
}

void print_Persistent_Reservations_Register_Ignore_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_REGISTER_I_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option when registering a new key to instruct\n");
        printf("\t\tthe device to ignore any previous registration key\n");
        printf("\t\tthat has already been registered from the same initiator\n");
        printf("\t\tNOTE: Not all devices will support this option\n\n");
    }
}

void print_Persistent_Reservations_Register_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_REGISTER_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to register a new key as specified by\n");
        printf("\t\tthe --%s option.\n", PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING);
        printf("\t\tCombine this with the following options as needed:\n");
        printf("\t\t--%s\n", PERSISTENT_RESERVATION_ATP_LONG_OPT_STRING);
        printf("\t\t--%s\n", PERSISTENT_RESERVATION_PTPL_LONG_OPT_STRING);
        printf("\t\t--%s\n", PERSISTENT_RESERVATION_REGISTER_I_LONG_OPT_STRING);
        printf("\n");
    }
}

void print_Persistent_Reservations_Unregister_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_UNREGISTER_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to unregister a key that is specified by\n");
        printf("\t\tthe --%s option.\n\n", PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING);
    }
}

void print_Persistent_Reservations_Reserve_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_RESERVE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to acquire a reservation using a key that is specified by\n");
        printf("\t\tthe --%s option.\n", PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING);
        printf("\t\tThe specified key must already be registered with the device.\n");
        printf("\t\tUse the --%s option to specifiy the reservation type\n",
               PERSISTENT_RESERVATION_TYPE_LONG_OPT_STRING);
        printf("\t\tto acquire.\n\n");
    }
}

void print_Persistent_Reservations_Release_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_RELEASE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to release reservation using a key that is specified by\n");
        printf("\t\tthe --%s option.\n", PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING);
        printf("\t\tThe specified key must already be registered with the device and must\n");
        printf("\t\thave an active reservation that can be released.\n");
        printf("\t\tUse the --%s option to specifiy the reservation type\n",
               PERSISTENT_RESERVATION_TYPE_LONG_OPT_STRING);
        printf("\t\tto release.\n\n");
    }
}

void print_Persistent_Reservations_Clear_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_CLEAR_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to clear all reservations using a key that is specified by\n");
        printf("\t\tthe --%s option.\n", PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING);
        printf("\t\tThe specified key must already be registered with the device.\n\n");
    }
}

void print_Persistent_Reservations_Preempt_Help(bool shortHelp)
{
    printf("\t--%s [reservation key to preempt]\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_PREEMPT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to preempt another reservation using a registration key\n");
        printf("\t\t that is specified by the --%s option.\n", PERSISTENT_RESERVATION_KEY_LONG_OPT_STRING);
        printf("\t\tThe specified key must already be registered with the device.\n");
        printf("\t\tThis is used to remove a reservation from another initiator and start\n");
        printf("\t\ta new one using the specified registration key. --%s must also\n",
               PERSISTENT_RESERVATION_TYPE_LONG_OPT_STRING);
        printf("\t\tbe provided to specify the type of reservation that should be active\n");
        printf("\t\tonce the preempt has completed.\n");
        printf("\t\tUse with the --%s option to cause the preempt to abort all\n",
               PERSISTENT_RESERVATION_PREEMPT_ABORT_LONG_OPT_STRING);
        printf("\t\toutstanding commands to the previous reservation holder.\n\n");
    }
}

void print_Persistent_Reservations_Preempt_Abort_Help(bool shortHelp)
{
    printf("\t--%s\t(NVMe & SAS only)\n", PERSISTENT_RESERVATION_PREEMPT_ABORT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to cause a preempt to abort commands to the previous\n");
        printf("\t\treservation holder. This must be used in combination with the\n");
        printf("\t\t--%s option in order to specify the key to preempt.\n\n",
               PERSISTENT_RESERVATION_PREEMPT_LONG_OPT_STRING);
    }
}

void print_Erase_Restore_Max_Prep_Help(bool shortHelp)
{
    printf("\t--%s\n", ERASE_RESTORE_MAX_PREP_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will attempt to restore the max LBA to the highest\n");
        printf("\t\tuser addressable sector prior to beginning a drive erasure.\n");
        printf("\t\tIf any failure is encountered while restoring the maxLBA, then\n");
        printf("\t\tan error will be indicated and the erase will not be started or\n");
        printf("\t\tattempted until other user intervention can be completed.\n");
        printf("\t\tIf a feature is frozen, locked, or has already been used during\n");
        printf("\t\tthe current power cycle, then these things can cause a failure.\n");
        printf("\t\tThe solution is to power cycle the drive, but in some cases it may\n");
        printf("\t\tbe necessary to try a different computer or adapter as commands may\n");
        printf("\t\tbe blocked by the system or automatically issued by the BIOS to lock\n");
        printf("\t\taccess to capacity changing commands.\n");
        printf("\t\tThis option will handle the ATA HPA (Host Protected Area), AMAC (Accessible\n");
        printf("\t\tMax Address Configuration), HPA Security Extension, and DCO (Device\n");
        printf("\t\tConfiguration Overlay) features in accordance with the specifications.\n");
        printf("\t\tIf the restore completes without error, then the erase will proceed\n");
        printf("\t\tand additional errors will only be in relation to those erasure methods.\n\n");
    }
}

void print_Zero_Verify_Help(bool shortHelp)
{
    printf("\t--%s [full | quick]\n", ZERO_VERIFY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to verify drive content, whether it's set to zero or not.\n");
        printf("\t\tThis operation will read user accessible address and validate if content at\n");
        printf("\t\tthat address is zero or not.\n");
        printf("\t\tValidation modes:\n");
        printf("\t\t  full - Complete drive will be scanned for verification.\n");
        printf("\t\t  quick - 0.1%% of total capacity will be scanned for ID and OD validation along with\n");
        printf("\t\t          2 random addresses from 10000 equal size sections each.\n\n");
    }
}

void print_Partition_Info_Help(bool shortHelp)
{
    printf("\t--%s\n", PARTITION_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to look for a partition table and dump\n");
        printf("\t\tthe list of partitions on a given disk.\n");
        printf("\t\tCurrently only MBR and GPT partition tables are supported.\n\n");
    }
}

void print_DCO_Restore_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", ATA_DCO_RESTORE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to restore device capabilities and features\n");
        printf("\t\thidden by DCO back to factory defaults.\n");
        printf("\t\tThis can only be used if DCO is not frozen and HPA has not\n");
        printf("\t\tbeen used to reduce the maximum LBA already. Recommend restoring\n");
        printf("\t\tthe max LBA prior to this option for best results.\n");
        printf("\t\tNOTE: Some motherboards will issue a DCO freezelock when booted.\n");
        printf("\t\t      If DCO is frozen each time the system is rebooted, try a\n");
        printf("\t\t      different system or add-in card to work around this.\n\n");
    }
}

void print_DCO_FreezeLock_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", ATA_DCO_FREEZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to issue the DCO freeze-lock command. Issuing\n");
        printf("\t\tthis command will prevent the ability to modify available capabilities\n");
        printf("\t\tor restore default capabilities until the device has been power cycled.\n\n");
    }
}

void print_DCO_Identify_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", ATA_DCO_IDENTIFY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will list the capabilities that can be restricted with DCO.\n");
        printf("\t\tRestricted capabilities are MWDMA and UDMA transfer modes, maximum LBA,\n");
        printf("\t\tand some ATA features or commands.\n");
        printf("\t\tThis will not work if the device has been DCO frozen.\n");
        printf("\t\tNOTE: Some motherboards will issue a DCO freezelock when booted.\n");
        printf("\t\t      If DCO is frozen each time the system is rebooted, try a\n");
        printf("\t\t      different system or add-in card to work around this.\n\n");
    }
}

void print_DCO_Set_Max_LBA_Help(bool shortHelp)
{
    printf("\t--%s [new max LBA]\t(SATA Only)\n", ATA_DCO_SETMAXLBA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set a lower max/native max LBA using the DCO\n");
        printf("\t\tfeature. This should be combined with --%s and --%s\n", ATA_DCO_SETMAXMODE_LONG_OPT_STRING,
               ATA_DCO_DISABLE_FEEATURES_LONG_OPT_STRING);
        printf("\t\tto make any and all DCO related changes at the same time in one command.\n");
        printf("\t\tThis will not work if the device has been DCO frozen.\n");
        printf("\t\tNOTE: Some motherboards will issue a DCO freezelock when booted.\n");
        printf("\t\t      If DCO is frozen each time the system is rebooted, try a\n");
        printf("\t\t      different system or add-in card to work around this.\n\n");
    }
}

void print_DCO_Set_Max_Mode_Help(bool shortHelp)
{
    printf("\t--%s [udma# | mwdma# | nodma]\t(SATA Only)\n", ATA_DCO_SETMAXMODE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set a different maximum supported DMA transfer mode\n");
        printf("\t\tusing the DCO feature. This should be combined with --%s and --%s\n",
               ATA_DCO_SETMAXLBA_LONG_OPT_STRING, ATA_DCO_DISABLE_FEEATURES_LONG_OPT_STRING);
        printf("\t\tto make any and all DCO related changes at the same time in one command.\n");
        printf("\t\tThe following arguments are available. Supported modes are set based on\n");
        printf("\t\tthe provided maximum and all modes below the given maximum:\n");
        printf("\t\t  %s - UDMA 6 and lower, including all MWDMA modes\n", ATA_DCO_MODE_UDMA6);
        printf("\t\t  %s - UDMA 5 and lower, including all MWDMA modes\n", ATA_DCO_MODE_UDMA5);
        printf("\t\t  %s - UDMA 4 and lower, including all MWDMA modes\n", ATA_DCO_MODE_UDMA4);
        printf("\t\t  %s - UDMA 3 and lower, including all MWDMA modes\n", ATA_DCO_MODE_UDMA3);
        printf("\t\t  %s - UDMA 2 and lower, including all MWDMA modes\n", ATA_DCO_MODE_UDMA2);
        printf("\t\t  %s - UDMA 1 and lower, including all MWDMA modes\n", ATA_DCO_MODE_UDMA1);
        printf("\t\t  %s - UDMA 0 and lower, including all MWDMA modes\n", ATA_DCO_MODE_UDMA0);
        printf("\t\t  %s - MWDMA 2 and lower; No UDMA support\n", ATA_DCO_MODE_MWDMA2);
        printf("\t\t  %s - MWDMA 1 and lower; No UDMA support\n", ATA_DCO_MODE_MWDMA1);
        printf("\t\t  %s - MWDMA 0 and lower; No UDMA support\n", ATA_DCO_MODE_MWDMA0);
        printf("\t\t  %s - No MWDMA or UDMA mode support listed in identify.\n", ATA_DCO_MODE_NODMA);
        printf("\t\tThis will not work if the device has been DCO frozen.\n");
        printf("\t\tNOTE: Some motherboards will issue a DCO freezelock when booted.\n");
        printf("\t\t      If DCO is frozen each time the system is rebooted, try a\n");
        printf("\t\t      different system or add-in card to work around this.\n\n");
    }
}

void print_DCO_Disable_Features_Help(bool shortHelp)
{
    printf("\t--%s [csv,list,of,features]\t(SATA Only)\n", ATA_DCO_DISABLE_FEEATURES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to disable different ATA commands and features\n");
        printf("\t\tusing the DCO feature. This should be combined with --%s and --%s\n",
               ATA_DCO_SETMAXLBA_LONG_OPT_STRING, ATA_DCO_SETMAXMODE_LONG_OPT_STRING);
        printf("\t\tto make any and all DCO related changes at the same time in one command.\n");
        printf("\t\tThe following arguments are available. Specifying a feature that the drive\n");
        printf("\t\tdoes not support restricting or does not support at all will not be\n");
        printf("\t\tconsidered an error.\n");
        printf("\t\tBelow is a full list of features that can be given with this option.\n");
        printf("\t\tit is unlikely a drive will support restricting all of these features.\n");
        printf("\t\t  %s - Write-Read-Verify feature\n", ATA_DCO_FEATURE_OPTION_WRV);
        printf("\t\t  %s - SMART Conveyance self-test\n", ATA_DCO_FEATURE_OPTION_SMART_CONVEYANCE);
        printf("\t\t  %s - SMART Seledtive self-test\n", ATA_DCO_FEATURE_OPTION_SMART_SELECTIVE);
        printf("\t\t  %s - Forced Unit Access\n", ATA_DCO_FEATURE_OPTION_FUA);
        printf("\t\t  %s - Time Limited Commands\n", ATA_DCO_FEATURE_OPTION_TLC);
        printf("\t\t  %s - Streaming Feature set\n", ATA_DCO_FEATURE_OPTION_STREAMING);
        printf("\t\t  %s - 48bit addressing\n", ATA_DCO_FEATURE_OPTION_48BIT);
        printf("\t\t  %s - Host Protected Area (HPA)\n", ATA_DCO_FEATURE_OPTION_HPA);
        printf("\t\t  %s - Automatic Accoustic Management\n", ATA_DCO_FEATURE_OPTION_AAM);
        printf("\t\t  %s - Tagged Command Queuing (TCQ)\n", ATA_DCO_FEATURE_OPTION_TCQ);
        printf("\t\t  %s - Power Up In Standby (PUIS)\n", ATA_DCO_FEATURE_OPTION_PUIS);
        printf("\t\t  %s - ATA Security\n", ATA_DCO_FEATURE_OPTION_SECURITY);
        printf("\t\t  %s - SMART Error Logging\n", ATA_DCO_FEATURE_OPTION_SMART_ERRORLOG);
        printf("\t\t  %s - SMART Self-test\n", ATA_DCO_FEATURE_OPTION_SMART_SELF_TEST);
        printf("\t\t  %s - SMART Feature set\n", ATA_DCO_FEATURE_OPTION_SMART_FEATURE);
        printf("\t\t  %s - SATA Software Settings Preservation (SSP)\n", ATA_DCO_FEATURE_OPTION_SSP);
        printf("\t\t  %s - SATA Asynchronous Notification\n", ATA_DCO_FEATURE_OPTION_ASYNC_NOTIFICATION);
        printf("\t\t  %s - SATA Interface Power Management\n", ATA_DCO_FEATURE_OPTION_INTERFACE_POWER_MGMT);
        printf("\t\t  %s - SATA Non-Zero Buffer Offsets\n", ATA_DCO_FEATURE_OPTION_NZ_BUFF);
        printf("\t\t  %s - SATA Native Command Queuing (NCQ)\n", ATA_DCO_FEATURE_OPTION_NCQ);
        printf("\t\t  %s - Non-Volatile Cache (NVCache)\n", ATA_DCO_FEATURE_OPTION_NVCACHE);
        printf("\t\t  %s - NVCache Power Management\n", ATA_DCO_FEATURE_OPTION_NVC_PM);
        printf("\t\t  %s - Write Uncorrectable Ext\n", ATA_DCO_FEATURE_OPTION_WUE);
        printf("\t\t  %s - Trusted Computing Group\n", ATA_DCO_FEATURE_OPTION_TCG);
        printf("\t\t  %s - Free-fall Control\n", ATA_DCO_FEATURE_OPTION_FREE_FALL);
        printf("\t\t  %s - Data Set Management\n", ATA_DCO_FEATURE_OPTION_DSM);
        printf("\t\t  %s - TRIM (Data Set Management)\n", ATA_DCO_FEATURE_OPTION_TRIM);
        printf("\t\t  %s - Extended Power Conditions\n", ATA_DCO_FEATURE_OPTION_EPC);
        printf("\t\tThis will not work if the device has been DCO frozen.\n");
        printf("\t\tNOTE: Some motherboards will issue a DCO freezelock when booted.\n");
        printf("\t\t      If DCO is frozen each time the system is rebooted, try a\n");
        printf("\t\t      different system or add-in card to work around this.\n\n");
    }
}

void print_Raw_CDB_Length_Help(bool shortHelp)
{
    printf("\t--%s [length in bytes]\n", RAW_CDB_LEN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the length of the CDB\n");
        printf("\t\tto send to the device. Max length is %" PRIu8 "\n", UINT8_MAX);
        printf("\t\tSome OS's may not support CDBs larger than 16 bytes\n\n");
    }
}

void print_Raw_CDB_Help(bool shortHelp)
{
    printf("\t--%s [csv CDB]\n", RAW_CDB_ARRAY_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a specific CDB to\n");
        printf("\t\tsend to a device. The entered value must be\n");
        printf("\t\tin comma separated value (csv) format. To\n");
        printf("\t\tspecify a value as hex, it must be either\n");
        printf("\t\tpre-pended with \"0x\" or post-pended with\n");
        printf("\t\t\"h\" or \"H\"\n");
        printf("\t\tExamples:\n");
        printf("\t\t\t1) inquiry: --%s 12h,0,0,0,60h,0\n", RAW_CDB_ARRAY_LONG_OPT_STRING);
        printf("\t\t\t2) inquiry: --%s 0x12,0,0,0,0x60,0\n", RAW_CDB_ARRAY_LONG_OPT_STRING);
        printf("\t\t\t3) inquiry: --%s 18,0,0,0,96,0\n", RAW_CDB_ARRAY_LONG_OPT_STRING);
        printf("\t\tAll 3 examples send the same command to a drive\n\n");
    }
}

void print_Raw_TFR_Command_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_COMMAND_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the command operation code for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_Feature_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_FEATURE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the feature register for\n");
        printf("\t\tsending a raw SATA command. (Lower 8 bits on 48 bit commands)\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_Feature_Ext_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_FEATURE_EXT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the feature ext register for\n");
        printf("\t\tsending a raw SATA command. (Upper 8 bits on 48 bit commands)\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_Feature_Full_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_FEATURE_FULL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the feature and feature ext register for\n");
        printf("\t\tsending a raw SATA command. This will be interpretted as a 16bit value.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_LBA_Low_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_LBA_LOW_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the LBA low (sector number) register for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_LBA_Mid_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_LBA_MID_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the LBA mid (Cylinder Low) register for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_LBA_High_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_LBA_HIGH_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the LBA high (Cylinder High) register for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_LBA_Low_Ext_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_LBA_LOW_EXT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the LBA low ext (sector number ext) register for\n");
        printf("\t\tsending a raw SATA command. This is for 48 bit commands.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_LBA_Mid_Ext_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_LBA_MID_EXT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the LBA mid ext (Cylinder Low ext) register for\n");
        printf("\t\tsending a raw SATA command. This is for 48 bit commands.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_LBA_High_Ext_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_LBA_HIGH_EXT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the LBA high ext (Cylinder High) ext register for\n");
        printf("\t\tsending a raw SATA command. This is for 48 bit commands.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_LBA_Full_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_LBA_FULL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the LBA registers for\n");
        printf("\t\tsending a raw SATA command. This will be interpretted as a 48 bit value\n");
        printf("\t\tto put into the appropriate LBA registers. This option is more useful when specifying\n");
        printf("\t\tan LBA value for a command like a read or a write.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_Device_Head_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_DEVICE_HEAD_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the Device/Head register for\n");
        printf("\t\tsending a raw SATA command. If this option is not provided, a value of\n");
        printf("\t\tA0h will be used for backwards compatibility with older ATA command specifications.\n");
        printf("\t\tNOTE: This option should be specified BEFORE the --%s option\n",
               RAW_TFR_LBA_MODE_BIT_LONG_OPT_STRING);
        printf("\t\tNOTE: On 28bit read/write commands, the high 4 bits of the LBA register need to be\n");
        printf("\t\t      placed in the lower 4 bits of this register.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_LBA_Mode_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", RAW_TFR_LBA_MODE_BIT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the LBA Mode bit of the Device/Head register for\n");
        printf("\t\tsending a raw SATA command. \n");
        printf("\t\tThis bit is necessary for performing read/write commands on modern drives.\n");
        printf("\t\tNOTE: This bit will NOT be set by default since it only applies to read/write commands\n");
        printf("\t\t      but not all other commands in the ATA specifications.\n");
        printf("\n");
    }
}

void print_Raw_TFR_Sector_Count_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_SECTOR_COUNT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the sector count register for\n");
        printf("\t\tsending a raw SATA command. (Lower 8 bits on 48 bit commands)\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_Sector_Count_Ext_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_SECTOR_COUNT_EXT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the sector count ext register for\n");
        printf("\t\tsending a raw SATA command. (Upper 8 bits on 48 bit commands)\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_Sector_Count_Full_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_SECTOR_COUNT_FULL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the sector count and sector count ext register for\n");
        printf("\t\tsending a raw SATA command. This will be interpretted as a 16bit value.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n\n");
    }
}

void print_Raw_TFR_ICC_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_ICC_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the ICC register for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n");
        printf("\t\tNOTE: Not all interfaces support setting this register. 32B SAT CDB required.\n\n");
    }
}

void print_Raw_TFR_AUX1_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_AUX1_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the Aux (7:0) register for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n");
        printf("\t\tNOTE: Not all interfaces support setting this register. 32B SAT CDB required.\n\n");
    }
}

void print_Raw_TFR_AUX2_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_AUX2_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the Aux (15:8) register for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n");
        printf("\t\tNOTE: Not all interfaces support setting this register. 32B SAT CDB required.\n\n");
    }
}

void print_Raw_TFR_AUX3_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_AUX3_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the Aux (23:16) register for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n");
        printf("\t\tNOTE: Not all interfaces support setting this register. 32B SAT CDB required.\n\n");
    }
}

void print_Raw_TFR_AUX4_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_AUX4_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the Aux (31:24) register for\n");
        printf("\t\tsending a raw SATA command.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n");
        printf("\t\tNOTE: Not all interfaces support setting this register. 32B SAT CDB required.\n\n");
    }
}

void print_Raw_TFR_Aux_Full_Help(bool shortHelp)
{
    printf("\t--%s [hex or decimal]\t(SATA Only)\n", RAW_TFR_AUX_FULL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the Aux (31:0) registers for\n");
        printf("\t\tsending a raw SATA command. This will be interpretted as a 32bit value.\n");
        printf("\t\tThe value should be specified in hex as ??h or 0x?? or as a decimale value\n");
        printf("\t\tNOTE: Not all interfaces support setting these registers. 32B SAT CDB required.\n\n");
    }
}

void print_Raw_TFR_Protocol_Help(bool shortHelp)
{
    printf("\t--%s [pio | dma | udma | fpdma | ncq | nodata | reset | dmaque | diag]\t(SATA Only)\n",
           RAW_TFR_PROTOCOL_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the protocol for\n");
        printf("\t\tsending a raw SATA command. This option must match the definition\n");
        printf("\t\tof the command in the ATA/ACS specification.\n");
        printf("\t\tThis option must be provided before a command will be sent.\n");
        printf("\t\tArguments:\n");
        printf("\t\t  pio - send as programmed IO protocol.\n");
        printf("\t\t  dma - send as direct memory access protocol\n");
        printf("\t\t  udma - send as ultra direct memory access protocol\n");
        printf("\t\t  fpdma/ncq - send as first party direct memory access protocol (NCQ)\n");
        printf("\t\t  nodata - send as non-data protocol\n");
        printf("\t\t  reset - send as reset protocol (ATAPI only)\n");
        printf("\t\t  dmaque - send as direct memory access queued protocol (TCQ)\n");
        printf("\t\t  diag - send as devie diagnostic protocol\n");
        printf("\t\tNOTE: If a command with dma doesn't work, try udma. Some SATLs like it better.\n");
        printf("\t\tNOTE: Most SATLs don't allow sending queued commands as pass-through. Some OSs\n");
        printf("\t\t      also will not allow queued pass-through commands.\n\n");
    }
}

void print_Raw_TFR_XFer_Length_Register_Help(bool shortHelp)
{
    printf("\t--%s [sectorCount | feature | tpsiu | nodata]\t(SATA Only)\n",
           RAW_TFR_XFER_LENGTH_LOCATION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the registers used to specify the length of data\n");
        printf("\t\tbeing sent or received when issuing a raw SATA command. This option must match\n");
        printf("\t\tthe definition of the command in the ATA/ACS specification.\n");
        printf("\t\tThis option must be provided before a command will be sent.\n");
        printf("\t\tArguments:\n");
        printf("\t\t  sectorCount - the sector count registers specify the number of blocks (most commands)\n");
        printf("\t\t  feature - the feature registers specify the number of blocks (queued commands)\n");
        printf("\t\t  tpsiu - a transport specific location will specify the length of the data transfer\n");
        printf("\t\t  nodata - no data transfer. Used on non-data protocol commands\n");
        printf("\t\tNOTE: tpsiu is generally only seen supported on USB adapters, but not all USB SATL's\n");
        printf("\t\t      will recognize this option.\n");
        printf("\t\tNOTE: For commands, such as identify (ECh), that transfer data, but do not specify\n");
        printf("\t\t      a value of 1 in the sector count, it is recommended that this is added\n");
        printf("\t\t      to the sector count register and and \"sectorCount\" is used for better\n");
        printf("\t\t      compatibility with various SATLs.\n\n");
    }
}

void print_Raw_TFR_Byte_Block_Help(bool shortHelp)
{
    printf("\t--%s [512 | logical | bytes | nodata]\t(SATA Only)\n", RAW_TFR_BYTE_BLOCK_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the data transfer length\n");
        printf("\t\tbeing sent or received when issuing a raw SATA command. This option must match\n");
        printf("\t\tthe definition of the command in the ATA/ACS specification.\n");
        printf("\t\tThis option must be provided before a command will be sent.\n");
        printf("\t\tArguments:\n");
        printf("\t\t  512 - the data transfer is a number of 512B blocks (most commands)\n");
        printf("\t\t  logical - data transfer is a number of logical block sizes transfers (read commands)\n");
        printf(
            "\t\t  bytes - the data transfer is a specific number of bytes (some legacy commands or tpsiu is used)\n");
        printf("\t\t  nodata - no data transfer. Used on non-data protocol commands\n");
        printf("\t\tNOTE: All read/write commands should use \"logical\", all other data transfers should use 512\n\n");
    }
}

void print_Raw_TFR_Size_Help(bool shortHelp)
{
    printf("\t--%s [28 | 48 | complete]\t(SATA Only)\n", RAW_TFR_SIZE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the command type: 28bit or 48bit\n");
        printf("\t\twhen issuing a raw SATA command. This option must match\n");
        printf("\t\tthe definition of the command in the ATA/ACS specification.\n");
        printf("\t\tThis option must be provided before a command will be sent.\n");
        printf("\t\tArguments:\n");
        printf("\t\t  28 - the command is a 28 bit command (ex: identify, SMART)\n");
        printf("\t\t  48 - the command is a 48 bit command (ex: read DMA ext, read log ext)\n");
        printf("\t\t  complete - 48 bit command that also sets ICC or AUX registers.\n");
        printf("\t\t             use this to force a 32B CDB. If specifying 48 and AUX or ICC\n");
        printf("\t\t             are set, 32B CDB will automatically be generated without needing\n");
        printf("\t\t             this option explicitly set.\n");
        printf("\t\tNOTE: complete TFR requires SAT 32B cdb, which many devices or interfaces\n");
        printf("\t\t      may not support. These commands may not be available.\n\n");
    }
}

void print_Raw_TFR_Check_Condition_Help(bool shortHelp)
{
    printf("\t--%s\t(SATA Only)\n", RAW_TFR_CHECK_CONDITION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to set the check condition bit in the SAT CDB that may be\n");
        printf("\t\tsent to a translator to inform it to generate a check condition and return\n");
        printf("\t\tall task file results.\n");
        printf("\t\tNOTE: This option may not work on all SATLs.\n\n");
    }
}

void print_Raw_Data_Length_Help(bool shortHelp)
{
    printf("\t--%s [length in bytes]\n", RAW_DATA_LEN_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the data transfer\n");
        printf("\t\tlength for a data-in or data-out transfer.\n");
        printf("\t\tThe following post fixes are allowed for\n");
        printf("\t\tspecifying a transfer length:\n");
        printf("\t\t\tBLOCKS - used to specify a transfer length\n");
        printf("\t\t\t\tin device logical blocks.\n");
        printf("\t\t\tB - length in bytes (val * 1) (default if no unit given)\n");
        printf("\t\t\tKB - length in kilobytes (val * 1000)\n");
        printf("\t\t\tKiB - length in kibibytes (val * 1024)\n");
        printf("\t\t\tMB - length in megabytes (val * 1000000)\n");
        printf("\t\t\tMiB - length in mebibytes (val * 1048576)\n");
        printf("\t\tYou must enter a size that is greater than or\n");
        printf("\t\tequal to any length in the entered raw command\n");
        printf("\t\tdata. If a lesser value is entered, then the\n");
        printf("\t\tutility may experience errors or crash.\n\n");
    }
}

void print_Raw_Data_Direction_Help(bool shortHelp)
{
    printf("\t--%s [in | out | none]\n", RAW_DATA_DIRECTION_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the data direction\n");
        printf("\t\tof the entered raw command.\n");
        printf("\t\tin - transfer data from the device to host\n");
        printf("\t\tout - transfer data from the host to device\n");
        printf("\t\tnone - no data is transferred\n");
    }
}

void print_Raw_Output_File_Help(bool shortHelp)
{
    printf("\t--%s [path/filename]\n", RAW_OUTPUT_FILE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify an output file to\n");
        printf("\t\tsave data returned from a command, or in the\n");
        printf("\t\tcase of an error, the returned error buffer.\n");
        printf("\t\tThis option will always append data to already\n");
        printf("\t\tcreated files. If an error occurs on a datain\n");
        printf("\t\traw command, the returned error data will not be\n");
        printf("\t\tsaved to a file to prevent adding unexpected data\n");
        printf("\t\tto the created file.\n\n");
    }
}

void print_Raw_Input_File_Help(bool shortHelp)
{
    printf("\t--%s [path/filename]\n", RAW_INPUT_FILE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify an input file to\n");
        printf("\t\tsend to a device. Must be a binary file.\n\n");
    }
}

void print_Raw_Timeout_Help(bool shortHelp)
{
    printf("\t--%s [time in seconds]\n", RAW_TIMEOUT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify an timeout in seconds\n");
        printf("\t\tfor a raw command being sent to a device.\n\n");
    }
}

void print_Raw_Input_File_Offset_Help(bool shortHelp)
{
    printf("\t--%s [offset in bytes]\n", RAW_INPUT_FILE_OFFSET_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the offset within\n");
        printf("\t\tthe raw input file to start sending data from.\n");
        printf("\t\tThe following post fixes are allowed for\n");
        printf("\t\tspecifying a transfer length:\n");
        printf("\t\t\tBLOCKS - used to specify an offset length\n");
        printf("\t\t\t\tin device logical blocks.\n");
        printf("\t\t\tB - length in bytes (val * 1) (Default if no unit given)\n");
        printf("\t\t\tKB - length in kilobytes (val * 1000)\n");
        printf("\t\t\tKiB - length in kibibytes (val * 1024)\n");
        printf("\t\t\tMB - length in megabytes (val * 1000000)\n");
        printf("\t\t\tMiB - length in mebibytes (val * 1048576)\n");
        printf("\n");
    }
}

void print_Check_Pending_List_Help(bool shortHelp)
{
    printf("\t--%s [count to check]\n", CHECK_PENDING_LIST_COUNT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to check if the pending defect list count is\n");
        printf("\t\tgreater than the provided value.\n");
        printf("\t\tNOTE: This only works on SAS products that support the Pending\n");
        printf("\t\tDefects log page from SBC4 or later\n\n");
    }
}

void print_Check_Grown_List_Help(bool shortHelp)
{
    printf("\t--%s [count to check]\n", CHECK_GROWN_LIST_COUNT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to check if the grown defect list count is\n");
        printf("\t\tgreater than the provided value.\n\n");
    }
}

void print_Show_Pending_List_Help(bool shortHelp)
{
    printf("\t--%s\n", SHOW_PENDING_LIST_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to show the pending defect list\n");
        printf("\t\tas it is reported by the drive.\n");
        printf("\t\tNOTE: SBC4 pending log required for SAS support.\n\n");
    }
}

void print_Create_Uncorrectable_Help(bool shortHelp)
{
    printf("\t--%s [lba]\n", CREATE_UNCORRECTABLE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to create a uncorrectable error at\n");
        printf("\t\tthe specified LBA. Use --%s to specify a range.\n", UNCORRECTABLE_RANGE_LONG_OPT_STRING);
        printf("\t\tBy default, pseudo uncorrectable errors will be created for\n");
        printf("\t\tthe entire physical sector. Use the --%s option\n", FLAG_UNCORRECTABLE_LONG_OPT_STRING);
        printf("\t\tto flag errors instead. Flagged errors do not get logged\n");
        printf("\t\tor have any error processing when encountered.\n\n");
    }
}

void print_Flag_Uncorrectable_Help(bool shortHelp)
{
    printf("\t--%s\n", FLAG_UNCORRECTABLE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to flag an uncorrectable error instead of\n");
        printf("\t\tcreating a pseudo uncorrectable error with the\n");
        printf("\t\t--%s or --%s options.\n", CREATE_UNCORRECTABLE_LONG_OPT_STRING,
               RANDOM_UNCORRECTABLES_LONG_OPT_STRING);
        printf("\t\tError types:\n");
        printf("\t\t    Pseudo - creates a pseudo uncorrectable error. The device\n");
        printf("\t\t             will perform full error recovery and logging on failure.\n");
        printf("\t\t    Flagged - flags an error. The device will not perform error\n");
        printf("\t\t              recovery and will not log on failure.\n");
        printf("\n");
    }
}

void print_Uncorrectable_Range_Help(bool shortHelp)
{
    printf("\t--%s [range]\n", UNCORRECTABLE_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify a range of LBAs to create an\n");
        printf("\t\tuncorrectable error at. This option must be used with the\n");
        printf("\t\t--%s or --%s options so that a\n", CREATE_UNCORRECTABLE_LONG_OPT_STRING,
               FLAG_UNCORRECTABLE_LONG_OPT_STRING);
        printf("\t\tstarting LBA is specified. \n\n");
    }
}

void print_Random_Uncorrectable_Help(bool shortHelp)
{
    printf("\t--%s [number of errors]\n", RANDOM_UNCORRECTABLES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to create a number of random uncorrectable\n");
        printf("\t\tLBAs on a drive. \n\n");
    }
}

void print_Disable_Read_Uncorrectables_Help(bool shortHelp)
{
    printf("\t--%s\n", DISABLE_READ_UNCORRECTABLES_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to disable issuing read commands to the LBAs\n");
        printf("\t\twhere errors are written. This option should only be used for\n");
        printf("\t\tdebugging. When this option is used, the uncorrectable errors\n");
        printf("\t\tmay not end up being logged in the Pending Defect List on the\n");
        printf("\t\tdrive.\n\n");
    }
}

void print_Corrupt_LBA_Help(bool shortHelp)
{
    printf("\t--%s [lba]\n", CORRUPT_LBA_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to corrupt the data bytes of an LBA. The\n");
        printf("\t\t--%s option can be used to specify the number of bytes to\n", BYTES_TO_CORRUPT_LONG_OPT_STRING);
        printf("\t\tcorrupt. If that option is not given, a default will be used\n");
        printf("\t\tthat attempts to create a correctable error on the drive.\n");
        printf("\t\tThis option can be used to create uncorrectable or correctable\n");
        printf("\t\terrors on a drive, depending on it's ECC algorithm and the number\n");
        printf("\t\tof corrupted data bytes.\n\n");
    }
}

void print_Corrupt_Random_LBAs_Help(bool shortHelp)
{
    printf("\t--%s [# of LBAs to corrupt]\n", CORRUPT_RANDOM_LBAS_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will corrupt the specified number of LBAs randomly\n");
        printf("\t\ton the device. The --%s option can be used to specify the number of bytes to\n",
               BYTES_TO_CORRUPT_LONG_OPT_STRING);
        printf("\t\tcorrupt. If that option is not given, a default will be used\n");
        printf("\t\tthat attempts to create a correctable error on the drive.\n");
        printf("\t\tThis option can be used to create uncorrectable or correctable\n");
        printf("\t\terrors on a drive, depending on it's ECC algorithm and the number\n");
        printf("\t\tof corrupted data bytes.\n\n");
    }
}

void print_Corrupt_Range_Help(bool shortHelp)
{
    printf("\t--%s [# of LBAs]\n", CORRUPT_LBA_RANGE_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option is used with the --%s option to\n", CORRUPT_LBA_LONG_OPT_STRING);
        printf("\t\tcorrupt a range of LBAs on the drive.\n\n");
    }
}

void print_Bytes_To_Corrupt_Help(bool shortHelp)
{
    printf("\t--%s [# of bytes]\n", BYTES_TO_CORRUPT_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to specify the number of data bytes to change\n");
        printf("\t\twhen used with the --%s option.\n\n", CORRUPT_LBA_LONG_OPT_STRING);
    }
}

void print_SCSI_FW_Info_Help(bool shortHelp)
{
    printf("\t--%s (SAS Only) (Seagate Only)\n", SHOW_SCSI_FW_INFO_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tThis option will show the SCSI Firmware info from\n");
        printf("\t\ta Seagate SAS drive. This is the extended firmware build\n");
        printf("\t\tinformation.\n\n");
    }
}

void print_Capacity_Model_Number_Mapping_Help(bool shortHelp)
{
    printf("\t--%s\n", CAPACITY_MODEL_NUMBER_MAPPING_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to display the capacity model number mapping\n");
        printf("\t\tTBD\n\n");
    }
}

void print_Change_Id_String_Help(bool shortHelp)
{
    printf("\t--%s\n", CHANGE_ID_STRING_LONG_OPT_STRING);
    if (!shortHelp)
    {
        printf("\t\tUse this option to change ID string according to capacity - model number\n");
        printf("\t\tmapping. Need to use together with --%s or --%s\n\n", SET_MAX_LBA_LONG_OPT_STRING,
               RESTORE_MAX_LBA_LONG_OPT_STRING);
    }
}
