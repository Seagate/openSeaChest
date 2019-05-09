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
// \file openseachest_util_options.h
// \brief This file defines the functions and macros to make building a open seachest utility easier.

#pragma once

#if defined (__cplusplus)
//defining these macros for C++ to make older C++ compilers happy and work like the newer C++ compilers
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
extern "C"
{
#endif

    #include "common.h"
#include "opensea_common_version.h"
#include "opensea_operation_version.h"

    //this is being defined for using bools with getopt since using a bool (1 byte typically) will cause stack corruption at runtime
    //This type should only be used where a boolean is desired when using the getopt parser (which expects an int), otherwise bool will do just fine
    #define getOptBool int
    #define goFalse 0
    #define goTrue !goFalse
	#define CURRENT_YEAR_LENGTH 5

    typedef enum _eUtilExitCodes{
        //Generic exit codes
        UTIL_EXIT_NO_ERROR = 0,
        UTIL_EXIT_ERROR_IN_COMMAND_LINE,
        UTIL_EXIT_INVALID_DEVICE_HANDLE,
        UTIL_EXIT_OPERATION_FAILURE,
        UTIL_EXIT_OPERATION_NOT_SUPPORTED,
        UTIL_EXIT_OPERATION_ABORTED,
        UTIL_EXIT_PATH_NOT_FOUND,
        UTIL_EXIT_CANNOT_OPEN_FILE,
        UTIL_EXIT_FILE_ALREADY_EXISTS,
        //TODO: More generic exit codes go here
        //Tool specific exit codes go here
        UTIL_TOOL_SPECIFIC_STARTING_ERROR_CODE = 32,//Use this value in a tool to start the tool's specific error code enumeration
    }eUtilExitCodes;

    typedef enum _eOutputMode {
        UTIL_OUTPUT_MODE_HUMAN = 0,
        UTIL_OUTPUT_MODE_RAW, // print it to screen 
        UTIL_OUTPUT_MODE_BIN, // create a binary file. 
        UTIL_OUTPUT_MODE_JSON,// create a JSON file
    } eOutputMode;

    //standard utility options

    #define RUN_ON_ALL_DRIVES runOnAllDrives
    #define USER_PROVIDED_HANDLE handleProvided
    #define DEVICE_LIST deviceList
    #define DEVICE_LIST_COUNT numberOfDevices
    #define HANDLE_LIST deviceHandleList
    #define DEVICE_UTIL_VARS \
    bool RUN_ON_ALL_DRIVES = false;\
    tDevice *DEVICE_LIST = NULL;\
    uint32_t DEVICE_LIST_COUNT = 0;\
    bool USER_PROVIDED_HANDLE = false;\
    char **HANDLE_LIST = NULL;
    #define DEVICE_SHORT_OPT 'd'
    #define DEVICE_LONG_OPT_STRING "device"
    #define DEVICE_LONG_OPT { DEVICE_LONG_OPT_STRING, required_argument, NULL, DEVICE_SHORT_OPT }

    #define SHOW_HELP_FLAG showHelp
    #define SHOW_HELP_VAR bool SHOW_HELP_FLAG = false;
    #define HELP_SHORT_OPT 'h'
    #define HELP_LONG_OPT_STRING "help"
    #define HELP_LONG_OPT { HELP_LONG_OPT_STRING, no_argument, NULL, HELP_SHORT_OPT }

    #define DEVICE_INFO_FLAG devInfo
    #define DEVICE_INFO_VAR bool DEVICE_INFO_FLAG = false;
    #define DEVICE_INFO_SHORT_OPT 'i'
    #define DEVICE_INFO_LONG_OPT_STRING "deviceInfo"
    #define DEVICE_INFO_LONG_OPT { DEVICE_INFO_LONG_OPT_STRING, no_argument, NULL, DEVICE_INFO_SHORT_OPT }

    #define TCG_DEVICE_INFO_FLAG tcgDevInfo
    #define TCG_DEVICE_INFO_VAR getOptBool TCG_DEVICE_INFO_FLAG = goFalse;
    #define TCG_DEVICE_INFO_LONG_OPT_STRING "tcgInfo"
    #define TCG_DEVICE_INFO_LONG_OPT { TCG_DEVICE_INFO_LONG_OPT_STRING, no_argument, &TCG_DEVICE_INFO_FLAG, goTrue }

    #define SAT_INFO_FLAG satInfo
    #define SAT_INFO_VAR getOptBool SAT_INFO_FLAG = goFalse;
    #define SAT_INFO_LONG_OPT_STRING "SATInfo"
    #define USB_CHILD_INFO_LONG_OPT_STRING "usbChildInfo"
    #define SAT_INFO_LONG_OPT { SAT_INFO_LONG_OPT_STRING, no_argument, &SAT_INFO_FLAG, goTrue }
    #define USB_CHILD_INFO_LONG_OPT { USB_CHILD_INFO_LONG_OPT_STRING, no_argument, &SAT_INFO_FLAG, goTrue }

    #define SCAN_FLAG enumerate
    #define SCAN_FLAG_VAR bool SCAN_FLAG = false;
    #define SCAN_SHORT_OPT 's'
    #define SCAN_LONG_OPT_STRING "scan"
    #define SCAN_LONG_OPT { SCAN_LONG_OPT_STRING, no_argument, NULL, SCAN_SHORT_OPT }

    #define AGRESSIVE_SCAN_FLAG agressiveScan
    #define AGRESSIVE_SCAN_FLAG_VAR bool AGRESSIVE_SCAN_FLAG = false;
    #define AGRESSIVE_SCAN_SHORT_OPT 'S'
    #define AGRESSIVE_SCAN_LONG_OPT_STRING "Scan"
    #define AGRESSIVE_SCAN_LONG_OPT { AGRESSIVE_SCAN_LONG_OPT_STRING, no_argument, NULL, AGRESSIVE_SCAN_SHORT_OPT }

    #define SCAN_FLAGS_SHORT_OPT 'F'
    #define SCAN_FLAGS_LONG_OPT_STRING "scanFlags"
    #define SCAN_FLAGS_LONG_OPT { SCAN_FLAGS_LONG_OPT_STRING, required_argument, NULL, SCAN_FLAGS_SHORT_OPT }

    #define SHOW_BANNER_FLAG showBanner
    #define SHOW_BANNER_VAR bool SHOW_BANNER_FLAG = false;
    #define VERSION_SHORT_OPT 'V'
    #define VERSION_LONG_OPT_STRING "version"
    #define VERSION_LONG_OPT { VERSION_LONG_OPT_STRING, no_argument, NULL, VERSION_SHORT_OPT }

    #define VERBOSE_SHORT_OPT 'v'
    #define VERBOSE_LONG_OPT_STRING "verbose"
    #define VERBOSE_LONG_OPT { VERBOSE_LONG_OPT_STRING, required_argument, NULL, VERBOSE_SHORT_OPT }

    #define QUIET_SHORT_OPT 'q'
    #define QUIET_LONG_OPT_STRING "quiet"
    #define QUIET_LONG_OPT { QUIET_LONG_OPT_STRING, no_argument, NULL, QUIET_SHORT_OPT }

    #define TEST_UNIT_READY_FLAG sendTestUnitReady
    #define TEST_UNIT_READY_VAR getOptBool TEST_UNIT_READY_FLAG = goFalse;
    #define TEST_UNIT_READY_LONG_OPT_STRING "testUnitReady"
    #define TEST_UNIT_READY_LONG_OPT { TEST_UNIT_READY_LONG_OPT_STRING, no_argument, &TEST_UNIT_READY_FLAG, goTrue }

    #define SAT_12_BYTE_CDBS_FLAG useSat12
    #define SAT_12_BYTE_CDBS_VAR getOptBool SAT_12_BYTE_CDBS_FLAG = goFalse;
    #define SAT_12_BYTE_CDBS_LONG_OPT_STRING "sat12byte"
    #define SAT_12_BYTE_CDBS_LONG_OPT { SAT_12_BYTE_CDBS_LONG_OPT_STRING, no_argument, &SAT_12_BYTE_CDBS_FLAG, goTrue }

    #define ONLY_SEAGATE_FLAG onlySeagateDrives
    #define ONLY_SEAGATE_VAR getOptBool ONLY_SEAGATE_FLAG = goFalse;
    #define ONLY_SEAGATE_LONG_OPT_STRING "onlySeagate"
    #define ONLY_SEAGATE_LONG_OPT { ONLY_SEAGATE_LONG_OPT_STRING, no_argument, &ONLY_SEAGATE_FLAG, goTrue }
    
    #define FORCE_SCSI_FLAG forceSCSI
    #define FORCE_ATA_FLAG forceATA
	#define FORCE_ATA_PIO_FLAG forcePIOATA
    #define FORCE_ATA_DMA_FLAG forceATADMA
    #define FORCE_ATA_UDMA_FLAG forceATAUDMA
    #define FORCE_DRIVE_TYPE_VARS\
    getOptBool FORCE_SCSI_FLAG = goFalse;\
    getOptBool FORCE_ATA_FLAG = goFalse;\
    getOptBool FORCE_ATA_PIO_FLAG = goFalse;\
    getOptBool FORCE_ATA_DMA_FLAG = goFalse;\
    getOptBool FORCE_ATA_UDMA_FLAG = goFalse;
    #define FORCE_SCSI_LONG_OPT_STRING "forceSCSI"
    #define FORCE_SCSI_LONG_OPT { FORCE_SCSI_LONG_OPT_STRING, no_argument, &FORCE_SCSI_FLAG, goTrue }
    #define FORCE_ATA_LONG_OPT_STRING "forceATA"
    #define FORCE_ATA_LONG_OPT { FORCE_ATA_LONG_OPT_STRING, no_argument, &FORCE_ATA_FLAG, goTrue }
    #define FORCE_ATA_PIO_LONG_OPT_STRING "forceATAPIO"
    #define FORCE_ATA_PIO_LONG_OPT { FORCE_ATA_PIO_LONG_OPT_STRING, no_argument, &FORCE_ATA_PIO_FLAG, goTrue }
    #define FORCE_ATA_DMA_LONG_OPT_STRING "forceATADMA"
    #define FORCE_ATA_DMA_LONG_OPT { FORCE_ATA_DMA_LONG_OPT_STRING, no_argument, &FORCE_ATA_DMA_FLAG, goTrue }
    #define FORCE_ATA_UDMA_LONG_OPT_STRING "forceATAUDMA"
    #define FORCE_ATA_UDMA_LONG_OPT { FORCE_ATA_UDMA_LONG_OPT_STRING, no_argument, &FORCE_ATA_UDMA_FLAG, goTrue }
    #define FORCE_DRIVE_TYPE_LONG_OPTS \
    FORCE_SCSI_LONG_OPT, FORCE_ATA_LONG_OPT, FORCE_ATA_PIO_LONG_OPT, FORCE_ATA_DMA_LONG_OPT, FORCE_ATA_UDMA_LONG_OPT

    #define USE_MAX_LBA useMaxLBA
    #define USE_CHILD_MAX_LBA useChildMaxLBA
    #define MAX_LBA_VARS \
    bool USE_MAX_LBA = false;\
    bool USE_CHILD_MAX_LBA = false;

    #define DISPLAY_LBA_FLAG displayLBA
    #define DISPLAY_LBA_THE_LBA theDisplayLBA
    #define DISPLAY_LBA_VAR \
    uint64_t DISPLAY_LBA_THE_LBA = UINT64_MAX;\
    bool DISPLAY_LBA_FLAG = false;
    #define DISPLAY_LBA_LONG_OPT_STRING "displayLBA"
    #define DISPLAY_LBA_LONG_OPT { DISPLAY_LBA_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define PATTERN_BUFFER patternBuffer
    #define PATTERN_FLAG usePattern
    #define PATTERN_BUFFER_LENGTH 8192
    #define PATTERN_VARS\
    bool PATTERN_FLAG = false;\
    uint8_t PATTERN_BUFFER[PATTERN_BUFFER_LENGTH] = { 0 };/*Allocating 2 * current largest logical sector (4096) for now...-TJE */
    #define PATTERN_LONG_OPT_STRING "pattern"
    #define PATTERN_LONG_OPT { PATTERN_LONG_OPT_STRING, required_argument, NULL, 0 }
    
    #define OUTPUTPATH_FLAG outputPathPtr
    #define OUTPUTPATH_VAR char *OUTPUTPATH_FLAG = NULL;

    #define PATH_LONG_OPT_STRING "outputPath"
    #define OUTPUTPATH_LONG_OPT { PATH_LONG_OPT_STRING, required_argument, NULL, 0}

    #define LICENSE_FLAG showLicense
    #define LICENSE_VAR getOptBool LICENSE_FLAG = goFalse;
    #define LICENSE_LONG_OPT_STRING "license"
    #define LICENSE_LONG_OPT { LICENSE_LONG_OPT_STRING, no_argument, &LICENSE_FLAG, goTrue }

    #define ECHO_COMMAND_LINE_FLAG echoCommandLine
    #define ECHO_COMMAND_LINE_VAR getOptBool ECHO_COMMAND_LINE_FLAG = goFalse;
    #define ECHO_COMMAND_LINE_LONG_OPT_STRING "echoCommandLine"
    #define ECHO_COMMAND_LIN_LONG_OPT { ECHO_COMMAND_LINE_LONG_OPT_STRING, no_argument, &ECHO_COMMAND_LINE_FLAG, goTrue }

    #define POLL_FLAG pollForProgress
    #define POLL_VAR getOptBool POLL_FLAG = goFalse; /*false = quiet, true = poll for progress/run timeout timer*/
    #define POLL_LONG_OPT_STRING "poll"
    #define POLL_LONG_OPT { POLL_LONG_OPT_STRING, no_argument, &POLL_FLAG, goTrue }

    #define PROGRESS_CHAR progressTest
    #define PROGRESS_VAR char *PROGRESS_CHAR = NULL;
    #define PROGRESS_SHORT_OPT '%'
    #define PROGRESS_LONG_OPT_STRING "progress"
    #define PROGRESS_LONG_OPT { PROGRESS_LONG_OPT_STRING, required_argument, NULL, PROGRESS_SHORT_OPT }

    #define DATA_ERASE_ACCEPT_STRING "this-will-erase-data"

    #define LONG_TEST_ACCEPT_STRING "I-understand-this-command-will-take-a-long-time-to-complete"
    #define SINGLE_SECTOR_DATA_ERASE_ACCEPT_STRING "I-understand-this-command-may-erase-single-sectors-if-they-are-already-unreadable"

    #define DATA_ERASE_FLAG dataEraseAccepted
    #define DATA_ERASE_VAR bool DATA_ERASE_FLAG = false;

    #define LONG_TEST_FLAG longOperationAccepted
    #define LONG_TEST_VAR bool LONG_TEST_FLAG = false;
    #define SINGLE_SECTOR_DATA_ERASE_FLAG singleSectorDataEraseAccepted
    #define SINGLE_SECTOR_DATA_ERASE_VAR bool SINGLE_SECTOR_DATA_ERASE_FLAG = false;
    #define CONFIRM_LONG_OPT_STRING "confirm"
    #define CONFIRM_LONG_OPT { CONFIRM_LONG_OPT_STRING, required_argument, NULL, 0}

    //SMART related options
    #define SMART_CHECK_FLAG smartCheck
    #define SMART_CHECK_VAR getOptBool SMART_CHECK_FLAG = goFalse;
    #define SMART_CHECK_LONG_OPT_STRING "smartCheck"
    #define SMART_CHECK_LONG_OPT { SMART_CHECK_LONG_OPT_STRING, no_argument, &SMART_CHECK_FLAG, goTrue }
    
    #define EXT_SMART_LOG_FLAG1 extSmatLog
    #define EXT_SMART_LOG_VAR1 getOptBool EXT_SMART_LOG_FLAG1 = goFalse;
    #define EXT_SMART_LOG_LONG_OPT_STRING1 "extSmartLog"
    #define EXT_SMART_LOG_LONG_OPT1 { EXT_SMART_LOG_LONG_OPT_STRING1, no_argument, &EXT_SMART_LOG_FLAG1, goTrue }

    #define SHORT_DST_FLAG shortDST
    #define SHORT_DST_VAR getOptBool SHORT_DST_FLAG = goFalse;
    #define SHORT_DST_LONG_OPT_STRING "shortDST"
    #define SHORT_DST_LONG_OPT { SHORT_DST_LONG_OPT_STRING, no_argument, &SHORT_DST_FLAG, goTrue }

    #define LONG_DST_FLAG longDST
    #define LONG_DST_VAR getOptBool LONG_DST_FLAG = goFalse;
    #define LONG_DST_LONG_OPT_STRING "longDST"
    #define LONG_DST_LONG_OPT { LONG_DST_LONG_OPT_STRING, no_argument, &LONG_DST_FLAG, goTrue }
    
    #define CONVEYANCE_DST_FLAG conveyanceDST
    #define CONVEYANCE_DST_VAR getOptBool CONVEYANCE_DST_FLAG = goFalse;
    #define CONVEYANCE_DST_LONG_OPT_STRING "conveyanceDST"
    #define CONVEYANCE_DST_LONG_OPT { CONVEYANCE_DST_LONG_OPT_STRING, no_argument, &CONVEYANCE_DST_FLAG, goTrue }

    #define CAPTIVE_FOREGROUND_FLAG captiveForeground
    #define CAPTIVE_FOREGROUND_VAR getOptBool CAPTIVE_FOREGROUND_FLAG = goFalse;
    #define CAPTIVE_LONG_OPT_STRING "captive"
    #define FOREGROUND_LONG_OPT_STRGIN "foreground"
    #define CAPTIVE_LONG_OPT { CAPTIVE_LONG_OPT_STRING, no_argument, &CAPTIVE_FOREGROUND_FLAG, goTrue }
    #define FOREGROUND_LONG_OPT { FOREGROUND_LONG_OPT_STRGIN, no_argument, &CAPTIVE_FOREGROUND_FLAG, goTrue }
    #define CAPTIVE_FOREGROUND_LONG_OPTS \
    CAPTIVE_LONG_OPT, FOREGROUND_LONG_OPT

    #define SMART_ATTRIBUTES_FLAG showSMARTAttributes
    #define SMART_ATTRIBUTES_MODE_FLAG showSMARTAttributesMode
    #define SMART_ATTRIBUTES_VARS \
    bool SMART_ATTRIBUTES_FLAG = false; \
    int SMART_ATTRIBUTES_MODE_FLAG = 0;
    #define SMART_ATTRIBUTES_LONG_OPT_STRING "smartAttributes"
    #define SMART_ATTRIBUTES_LONG_OPT { SMART_ATTRIBUTES_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define ABORT_DST_FLAG abortDST
    #define ABORT_DST_VAR getOptBool ABORT_DST_FLAG = goFalse;
    #define ABORT_DST_LONG_OPT_STRING "abortDST"
    #define ABORT_DST_LONG_OPT { ABORT_DST_LONG_OPT_STRING, no_argument, &ABORT_DST_FLAG, goTrue }

    #define IDD_TEST_FLAG iddOperation
    #define RUN_IDD_FLAG runIDDOp
    #define IDD_TEST_VARS \
     int IDD_TEST_FLAG = 0;\
     bool RUN_IDD_FLAG = false;
    #define IDD_TEST_LONG_OPT_STRING "idd"
    #define IDD_TEST_LONG_OPT { IDD_TEST_LONG_OPT_STRING, required_argument, NULL, 0 }
    
    #define ABORT_IDD_FLAG abortIDD
    #define ABORT_IDD_VAR getOptBool ABORT_IDD_FLAG = goFalse;
    #define ABORT_IDD_LONG_OPT_STRING "abortIDD"
    #define ABORT_IDD_LONG_OPT { ABORT_IDD_LONG_OPT_STRING, no_argument, &ABORT_IDD_FLAG, goTrue }

    #define DST_AND_CLEAN_FLAG runDSTAndClean
    #define DST_AND_CLEAN_VAR getOptBool DST_AND_CLEAN_FLAG = goFalse;
    #define DST_AND_CLEAN_LONG_OPT_STRING "dstAndClean"
    #define DST_AND_CLEAN_LONG_OPT { DST_AND_CLEAN_LONG_OPT_STRING, no_argument, &DST_AND_CLEAN_FLAG, goTrue }

    #define SEAGATE_CLEAN_FLAG runSeagateClean
    #define SEAGATE_CLEAN_VAR getOptBool SEAGATE_CLEAN_FLAG = goFalse;
    #define SEAGATE_CLEAN_LONG_OPT_STRING "seagateClean"
    #define SEAGATE_CLEAN_LONG_OPT { SEAGATE_CLEAN_LONG_OPT_STRING, no_argument, &SEAGATE_CLEAN_FLAG, goTrue }

    //Power related options
    #define CHECK_POWER_FLAG checkPower
    #define CHECK_POWER_VAR getOptBool CHECK_POWER_FLAG = goFalse;
    #define CHECK_POWER_LONG_OPT_STRING "checkPowerMode"
    #define CHECK_POWER_LONG_OPT { CHECK_POWER_LONG_OPT_STRING, no_argument, &CHECK_POWER_FLAG, goTrue }

    #define SPIN_DOWN_FLAG spinDown
    #define SPIN_DOWN_VAR getOptBool SPIN_DOWN_FLAG = goFalse;
    #define SPIN_DOWN_LONG_OPT_STRING "spinDown"
    #define SPIN_DOWN_LONG_OPT { SPIN_DOWN_LONG_OPT_STRING, no_argument, &SPIN_DOWN_FLAG, goTrue }

    #define STANDBY_FLAG standby
    #define STANDBY_VAR getOptBool STANDBY_FLAG = goFalse;
    #define STANDBY_LONG_OPT_STRING "standby"
    #define STANDBY_LONG_OPT { STANDBY_LONG_OPT_STRING, no_argument, &STANDBY_FLAG, goTrue }

    #define SLEEP_FLAG sleep
    #define SLEEP_VAR getOptBool SLEEP_FLAG = goFalse;
    #define SLEEP_LONG_OPT_STRING "sleep"
    #define SLEEP_LONG_OPT { SLEEP_LONG_OPT_STRING, no_argument, &SLEEP_FLAG, goTrue }

    #define IDLE_FLAG idle
    #define IDLE_VAR getOptBool IDLE_FLAG = goFalse;
    #define IDLE_LONG_OPT_STRING "idle"
    #define IDLE_LONG_OPT { IDLE_LONG_OPT_STRING, no_argument, &IDLE_FLAG, goTrue }

    #define IDLE_UNLOAD_FLAG idleUnload
    #define IDLE_UNLOAD_VAR getOptBool IDLE_UNLOAD_FLAG = goFalse;
    #define IDLE_UNLOAD_LONG_OPT_STRING "idleUnload"
    #define IDLE_UNLOAD_LONG_OPT { IDLE_UNLOAD_LONG_OPT_STRING, no_argument, &IDLE_UNLOAD_FLAG, goTrue }

    #define ACTIVE_FLAG activeState
    #define ACTIVE_VAR getOptBool ACTIVE_FLAG = goFalse;
    #define ACTIVE_LONG_OPT_STRING "active"
    #define ACTIVE_LONG_OPT { ACTIVE_LONG_OPT_STRING, no_argument, &ACTIVE_FLAG, goTrue }

    #define ENABLE_POWER_MODE_FLAG enablePowerMode
    #define ENABLE_POWER_MODE_VAR getOptBool ENABLE_POWER_MODE_FLAG = goFalse;
    #define ENABLE_POWER_MODE_LONG_OPT_STRING "enableMode"
    #define ENABLE_POWER_MODE_LONG_OPT { ENABLE_POWER_MODE_LONG_OPT_STRING, no_argument, &ENABLE_POWER_MODE_FLAG, goTrue }

    #define DISABLE_POWER_MODE_FLAG disablePowerMode
    #define DISABLE_POWER_MODE_VAR getOptBool DISABLE_POWER_MODE_FLAG = goFalse;
    #define DISABLE_POWER_MODE_LONG_OPT_STRING "disableMode"
    #define DISABLE_POWER_MODE_LONG_OPT { DISABLE_POWER_MODE_LONG_OPT_STRING, no_argument, &DISABLE_POWER_MODE_FLAG, goTrue }

    #define DEFAULT_POWER_MODE_FLAG restoreDefaultPowerMode
    #define DEFAULT_POWER_MODE_VAR getOptBool DEFAULT_POWER_MODE_FLAG = goFalse;
    #define DEFAULT_POWER_MODE_LONG_OPT_STRING "defaultMode"
    #define DEFAULT_POWER_MODE_LONG_OPT { DEFAULT_POWER_MODE_LONG_OPT_STRING, no_argument, &DEFAULT_POWER_MODE_FLAG, goTrue }

    #define OUTPUT_MODE_IDENTIFIER outputMode
    #define OUTPUT_MODE_VAR eOutputMode OUTPUT_MODE_IDENTIFIER = 0;
    #define OUTPUT_MODE_LONG_OPT_STRING "outputMode"
    #define OUTPUT_MODE_LONG_OPT { OUTPUT_MODE_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define POWER_MODE_IDENTIFIER powerMode
    #define POWER_MODE_VAR ePowerConditionID POWER_MODE_IDENTIFIER = PWR_CND_NOT_SET;
    #define POWER_MODE_LONG_OPT_STRING "powerMode"
    #define POWER_MODE_LONG_OPT { POWER_MODE_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define EPC_ENABLED_IDENTIFIER enableEPC
    #define EPC_ENABLED_VAR eEPCFeatureSet EPC_ENABLED_IDENTIFIER = ENABLE_EPC_NOT_SET;
    #define EPC_ENABLED_LONG_OPT_STRING "EPCfeature"
    #define EPC_ENABLED_LONG_OPT { EPC_ENABLED_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define POWER_MODE_TIMER powerModeTimer
    #define POWER_MODE_TIMER_VALID powerModeTimerValid
    #define POWER_MODE_TIMER_VARS \
    uint32_t POWER_MODE_TIMER = -1;\
    bool POWER_MODE_TIMER_VALID = false;
    #define POWER_MODE_TIMER_LONG_OPT_STRING "modeTimer"
    #define POWER_MODE_TIMER_LONG_OPT { POWER_MODE_TIMER_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define CHANGE_POWER_MODE_FLAG changePowerMode
    #define CHANGE_POWER_MODE_VAR getOptBool CHANGE_POWER_MODE_FLAG = goFalse;
    #define CHANGE_POWER_MODE_LONG_OPT_STRING "changePower"
    #define CHANGE_POWER_MODE_LONG_OPT { CHANGE_POWER_MODE_LONG_OPT_STRING, no_argument, &CHANGE_POWER_MODE_FLAG, goTrue }

    #define TRANSITION_POWER_MODE_FLAG transitionPowerMode
    #define TRANSITION_POWER_MODE_VAR getOptBool TRANSITION_POWER_MODE_FLAG = goFalse;
    #define TRANSITION_POWER_MODE_LONG_OPT_STRING "transitionPower"
    #define TRANSITION_POWER_MODE_LONG_OPT { TRANSITION_POWER_MODE_LONG_OPT_STRING, no_argument, &TRANSITION_POWER_MODE_FLAG, goTrue }

    //Following is for NVMe Utilities.
    #define TRANSITION_POWER_STATE_TO transitionPowerState
    #define TRANSITION_POWER_STATE_VAR int32_t TRANSITION_POWER_STATE_TO = -1;
    #define TRANSITION_POWER_STATE_LONG_OPT_STRING "transitionPowerState"
    #define TRANSITION_POWER_STATE_LONG_OPT { TRANSITION_POWER_STATE_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define GET_NVME_LOG_IDENTIFIER nvmeGetLogPage
    #define GET_NVME_LOG_VAR int32_t GET_NVME_LOG_IDENTIFIER = -1;
    #define GET_NVME_LOG_LONG_OPT_STRING "getLogPage"
    #define GET_NVME_LOG_LONG_OPT { GET_NVME_LOG_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_FLAG  clearpciecorrectableerrors
    #define CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_VAR getOptBool CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_FLAG = goFalse;
    #define CLEAR_PCIE_CORRECTABLE_ERRORS_LONG_OPT_STRING "clearPciErr"
    #define CLEAR_PCIE_CORRECTABLE_ERRORS_LONG_OPT { CLEAR_PCIE_CORRECTABLE_ERRORS_LONG_OPT_STRING, no_argument, &CLEAR_PCIE_CORRECTABLE_ERRORS_LOG_FLAG, goTrue }

    #define GET_FEATURES getFeatures
    #define GET_FEATURES_VAR int32_t GET_FEATURES_IDENTIFIER = -1;
    #define GET_FEATURES_LONG_OPT_STRING "getFeatures"
    #define GET_FEATURES_LONG_OPT { GET_FEATURES_LONG_OPT_STRING, required_argument, NULL, 0 }

    // NVMe Temperature Statistics 
    #define NVME_TEMP_STATS_FLAG nvmeTempStats
    #define NVME_TEMP_STATS_VAR getOptBool NVME_TEMP_STATS_FLAG = goFalse;
    #define NVME_TEMP_STATS_LONG_OPT_STRING "tempStats"
    #define NVME_TEMP_STATS_LONG_OPT { NVME_TEMP_STATS_LONG_OPT_STRING, no_argument, &NVME_TEMP_STATS_FLAG, goTrue }

    // NVMe PCIe Statistics 
    #define NVME_PCI_STATS_FLAG nvmePciStats
    #define NVME_PCI_STATS_VAR getOptBool NVME_PCI_STATS_FLAG = goFalse;
    #define NVME_PCI_STATS_LONG_OPT_STRING "pciStats"
    #define NVME_PCI_STATS_LONG_OPT { NVME_PCI_STATS_LONG_OPT_STRING, no_argument, &NVME_PCI_STATS_FLAG, goTrue }

    // NVMe Telemetry
    #define GET_NVME_TELE_IDENTIFIER nvmeGetTelemetry
    #define GET_NVME_TELE_VAR int32_t GET_NVME_TELE_IDENTIFIER = -1;
    #define GET_NVME_TELE_LONG_OPT_STRING "getTelemetry"
    #define GET_NVME_TELE_LONG_OPT { GET_NVME_TELE_LONG_OPT_STRING, required_argument, NULL, 0 }

    // NVMe Telemetry Data Area
    #define NVME_TELE_DATA_AREA telemetryDataArea
    #define NVME_TELE_DATA_AREA_VAR int32_t NVME_TELE_DATA_AREA = 3;
    #define NVME_TELE_DATA_AREA_LONG_OPT_STRING "telemetryDataArea"
    #define NVME_TELE_DATA_AREA_LONG_OPT { NVME_TELE_DATA_AREA_LONG_OPT_STRING, required_argument, NULL, 0 }

    //Generic read test options
    #define GENERIC_TEST_MODE_FLAG genericTestMode
    #define GENERIC_TEST_MODE_VAR int genericTestMode = 0; //0 = read, 1 = write, 2 = verify
    #define GENERIC_TEST_LONG_OPT_STRING "genericMode"
    #define GENERIC_TEST_LONG_OPT { GENERIC_TEST_LONG_OPT_STRING, required_argument, NULL, 0 }

    //buffer test
    #define BUFFER_TEST_FLAG performBufferTest
    #define BUFFER_TEST_VAR getOptBool BUFFER_TEST_FLAG = goFalse;
    #define BUFFER_TEST_LONG_OPT_STRING "bufferTest"
    #define BUFFER_TEST_LONG_OPT { BUFFER_TEST_LONG_OPT_STRING, no_argument, &BUFFER_TEST_FLAG, goTrue }

    #define SHORT_GENERIC_FLAG runShortGeneric
    #define SHORT_GENERIC_VAR \
    getOptBool SHORT_GENERIC_FLAG = goFalse;
    #define SHORT_GENERIC_LONG_OPT_STRING "shortGeneric"
    #define SHORT_GENERIC_LONG_OPT { SHORT_GENERIC_LONG_OPT_STRING, no_argument, &SHORT_GENERIC_FLAG, goTrue }

    #define TWO_MINUTE_TEST_FLAG runTwoMinuteTest
    #define TWO_MINUTE_TEST_VAR \
    getOptBool TWO_MINUTE_TEST_FLAG = goFalse;
    #define TWO_MINUTE_TEST_LONG_OPT_STRING "twoMinuteGeneric"
    #define TWO_MINUTE_TEST_LONG_OPT { TWO_MINUTE_TEST_LONG_OPT_STRING, no_argument, &TWO_MINUTE_TEST_FLAG, goTrue }

    #define LONG_GENERIC_FLAG runLongGeneric
    #define LONG_GENERIC_VAR \
    getOptBool LONG_GENERIC_FLAG = goFalse;
    #define LONG_GENERIC_LONG_OPT_STRING "longGeneric"
    #define LONG_GENERIC_LONG_OPT { LONG_GENERIC_LONG_OPT_STRING, no_argument, &LONG_GENERIC_FLAG, goTrue }

	#define RUN_USER_GENERIC_TEST runUserGeneric
    #define USER_GENERIC_START_FLAG userGenericStart
    #define USER_GENERIC_START_VAR \
    uint64_t USER_GENERIC_START_FLAG = UINT64_MAX;
    #define RUN_USER_GENERIC_TEST_VAR \
    bool RUN_USER_GENERIC_TEST = false;
    #define USER_GENERIC_LONG_OPT_START_STRING "userGenericStart"
    #define USER_GENERIC_START_LONG_OPT { USER_GENERIC_LONG_OPT_START_STRING, required_argument, NULL, 0 }

    #define USER_GENERIC_RANGE_UNITS_SPECIFIED userGenericRangeUsingUnits
    #define USER_GENERIC_RANGE_FLAG userGenericRange
    #define USER_GENERIC_RANGE_VAR \
    uint64_t USER_GENERIC_RANGE_FLAG = 0;\
    bool USER_GENERIC_RANGE_UNITS_SPECIFIED = false;
    #define USER_GENERIC_LONG_OPT_RANGE_STRING "userGenericRange"
    #define USER_GENERIC_RANGE_LONG_OPT { USER_GENERIC_LONG_OPT_RANGE_STRING, required_argument, NULL, 0 }

    #define ERROR_LIMIT_FLAG errorLimit
    #define ERROR_LIMIT_LOGICAL_COUNT errorLimitIsInLogicalBlocks
    #define ERROR_LIMIT_VAR \
    uint16_t ERROR_LIMIT_FLAG = 50;/*default value unless 512/4k which will be 400*/\
    bool ERROR_LIMIT_LOGICAL_COUNT = false;//default to being in physical blocks
    #define ERROR_LIMIT_LONG_OPT_STRING "errorLimit" 
    #define ERROR_LIMIT_LONG_OPT { ERROR_LIMIT_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define RANDOM_READ_TEST_FLAG randomReadTest
    #define RANDOM_READ_TEST_VAR \
    getOptBool RANDOM_READ_TEST_FLAG = goFalse;
    #define RANDOM_READ_TEST_LONG_OPT_STRING "randomTest"
    #define RANDOM_READ_TEST_LONG_OPT { RANDOM_READ_TEST_LONG_OPT_STRING, no_argument, &RANDOM_READ_TEST_FLAG, goTrue }

    #define BUTTERFLY_READ_TEST_FLAG butterflyTest
    #define BUTTEFFLY_READ_TEST_VAR \
    getOptBool BUTTERFLY_READ_TEST_FLAG = goFalse;
    #define BUTTERFLY_READ_TEST_LONG_OPT_STRING "butterflyTest"
    #define BUTTERFLY_TEST_LONG_OPT { BUTTERFLY_READ_TEST_LONG_OPT_STRING, no_argument, &BUTTERFLY_READ_TEST_FLAG, goTrue }

    #define STOP_ON_ERROR_FLAG stopOnFirstError
    #define STOP_ON_ERROR_VAR \
    getOptBool STOP_ON_ERROR_FLAG = goFalse;
    #define STOP_ON_ERROR_LONG_OPT_STRING "stopOnError"
    #define STOP_ON_ERROR_LONG_OPT { STOP_ON_ERROR_LONG_OPT_STRING, no_argument, &STOP_ON_ERROR_FLAG, goTrue }

    //repair flags
    #define REPAIR_AT_END_FLAG repairAtEnd
    #define REPAIR_AT_END_VAR \
    getOptBool REPAIR_AT_END_FLAG = goFalse;
    #define REPAIR_AT_END_LONG_OPT_STRING "repairAtEnd"
    #define REPAIR_AT_END_LONG_OPT { REPAIR_AT_END_LONG_OPT_STRING, no_argument, &REPAIR_AT_END_FLAG, goTrue }

    #define REPAIR_ON_FLY_FLAG repairOnTheFly
    #define REPAIR_ON_FLY_VAR \
    getOptBool REPAIR_ON_FLY_FLAG = goFalse;
    #define REPAIR_ON_FLY_LONG_OPT_STRING "repairOnFly"
    #define REPAIR_ON_FLY_LONG_OPT { REPAIR_ON_FLY_LONG_OPT_STRING, no_argument, &REPAIR_ON_FLY_FLAG, goTrue }

    //time related flags
    #define HOURS_TIME_FLAG timeHours
    #define HOURS_TIME_VAR \
    uint8_t HOURS_TIME_FLAG = 0;
    #define HOURS_TIME_LONG_OPT_STRING "hours"
    #define HOURS_TIME_LONG_OPT { HOURS_TIME_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define MINUTES_TIME_FLAG timeMinutes
    #define MINUTES_TIME_VAR \
    uint16_t MINUTES_TIME_FLAG = 0;
    #define MINUTES_TIME_LONG_OPT_STRING "minutes"
    #define MINUTES_TIME_LONG_OPT { MINUTES_TIME_LONG_OPT_STRING, required_argument, NULL , 0 }

    #define SECONDS_TIME_FLAG timeSeconds
    #define SECONDS_TIME_VAR \
    uint32_t SECONDS_TIME_FLAG = 0;
    #define SECONDS_TIME_LONG_OPT_STRING "seconds"
    #define SECONDS_TIME_LONG_OPT { SECONDS_TIME_LONG_OPT_STRING, required_argument, NULL, 0 }

    //overwrite flags (overwrite for a time will be handled with the above time flags)
    #define OVERWRITE_START_FLAG overwriteStart
    #define RUN_OVERWRITE_FLAG performOverwrite
    #define OVERWRITE_RANGE_FLAG overWriteRange
    #define OVERWRITE_VARS \
    uint64_t OVERWRITE_START_FLAG = UINT64_MAX; \
    uint64_t OVERWRITE_RANGE_FLAG = 0; \
    bool RUN_OVERWRITE_FLAG = false;
    #define OVERWRITE_LONG_OPT_STRING "overwrite"
    #define OVERWRITE_LONG_OPT { OVERWRITE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define OVERWRITE_RANGE_LONG_OPT_STRING "overwriteRange"
    #define OVERWRITE_RANGE_LONG_OPT { OVERWRITE_RANGE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define OVERWRITE_LONG_OPTS OVERWRITE_LONG_OPT,OVERWRITE_RANGE_LONG_OPT

    //trim/unmap flags
    #define TRIM_UNMAP_START_FLAG trimUnmapStart
    #define TRIM_UNMAP_RANGE_FLAG trimUnmapRange
    #define RUN_TRIM_UNMAP_FLAG performTrimUnmap
    #define TRIM_UNMAP_VARS \
    uint64_t TRIM_UNMAP_START_FLAG = UINT64_MAX; \
    uint64_t TRIM_UNMAP_RANGE_FLAG = 0;\
    bool RUN_TRIM_UNMAP_FLAG = false;
    #define TRIM_LONG_OPT_STRING "trim"
    #define UNMAP_LONG_OPT_STRING "unmap"
    #define TRIM_LONG_OPT { TRIM_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define UNMAP_LONG_OPT { UNMAP_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define TRIM_RANGE_LONG_OPT_STRING "trimRange"
    #define TRIM_RANGE_LONG_OPT { TRIM_RANGE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define UNMAP_RANGE_LONG_OPT_STRING "unmapRange"
    #define UNMAP_RANGE_LONG_OPT { UNMAP_RANGE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define TRIM_LONG_OPTS TRIM_LONG_OPT,TRIM_RANGE_LONG_OPT
    #define UNMAP_LONG_OPTS UNMAP_LONG_OPT,UNMAP_RANGE_LONG_OPT

    //write same flags
    #define WRITE_SAME_START_FLAG writesameStartLBA
    #define WRITE_SAME_RANGE_FLAG writeSameRange
    #define RUN_WRITE_SAME_FLAG runWruteSame
    #define WRITE_SAME_UTIL_VARS \
    uint64_t WRITE_SAME_START_FLAG = UINT64_MAX;\
    uint64_t WRITE_SAME_RANGE_FLAG = 0;\
    bool RUN_WRITE_SAME_FLAG = false;
    #define WRITE_SAME_LONG_OPT_STRING "writeSame"
    #define WRITE_SAME_LONG_OPT { WRITE_SAME_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define WRITE_SAME_RANGE_LONG_OPT_STRING "writeSameRange"
    #define WRITE_SAME_RANGE_LONG_OPT { WRITE_SAME_RANGE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define WRITE_SAME_LONG_OPTS WRITE_SAME_LONG_OPT,WRITE_SAME_RANGE_LONG_OPT

    //TCG SID flag
    #define TCG_SID_FLAG sid
    #define TCG_SID_VARS \
    char sidBuf[33] = { 0 };\
    char* TCG_SID_FLAG = &sidBuf[0];
    #define TCG_SID_LONG_OPT_STRING "sid"
    #define TCG_SID_LONG_OPT { TCG_SID_LONG_OPT_STRING, required_argument, NULL, 0 }

    //TCG PSID flag
    #define TCG_PSID_FLAG psid
    #define TCG_PSID_VARS \
    char psidBuf[33] = { 0 };\
    char* TCG_PSID_FLAG = &psidBuf[0];
    #define TCG_PSID_LONG_OPT_STRING "psid"
    #define TCG_PSID_LONG_OPT { TCG_PSID_LONG_OPT_STRING, required_argument, NULL, 0 }

    //revertSP flags
    #define TCG_REVERT_SP_FLAG revertSP
    #define TCG_REVERT_SP_VARS \
    getOptBool TCG_REVERT_SP_FLAG = goFalse;
    #define TCG_REVERT_SP_LONG_OPT_STRING "revertSP"
    #define TCG_REVERT_SP_LONG_OPT { TCG_REVERT_SP_LONG_OPT_STRING, no_argument, &TCG_REVERT_SP_FLAG, goTrue }

    //revert flags
    #define TCG_REVERT_FLAG tcgRevert
    #define TCG_REVERT_VAR \
    getOptBool TCG_REVERT_FLAG = goFalse;
    #define TCG_REVERT_LONG_OPT_STRING "revert"
    #define TCG_REVERT_LONG_OPT { TCG_REVERT_LONG_OPT_STRING, no_argument, &TCG_REVERT_FLAG, goTrue }

    //show power consumption
    #define SHOW_POWER_CONSUMPTION_FLAG showPowerConsumption
    #define SHOW_POWER_CONSUMPTION_VAR getOptBool SHOW_POWER_CONSUMPTION_FLAG = goFalse;
    #define SHOW_POWER_CONSUMPTION_LONG_OPT_STRING "showPowerConsumption"
    #define SHOW_POWER_CONSUMPTION_LONG_OPT { SHOW_POWER_CONSUMPTION_LONG_OPT_STRING, no_argument, &SHOW_POWER_CONSUMPTION_FLAG, goTrue }

    //set power consumption
    #define SET_POWER_CONSUMPTION_FLAG setPowerConsumption
    #define SET_POWER_CONSUMPTION_VALUE powerConsumptionIdentifierValue
    #define SET_POWER_CONSUMPTION_WATTS_VALUE powerConsumptionWatts
    #define SET_POWER_CONSUMPTION_DEFAULT_FLAG setDefaultPowerConsumption
    #define SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE powerConsumptionActiveLevel
    #define SET_POWER_CONSUMPTION_VARS \
    bool SET_POWER_CONSUMPTION_FLAG = false;\
    uint8_t SET_POWER_CONSUMPTION_VALUE = 0;\
    bool SET_POWER_CONSUMPTION_DEFAULT_FLAG = false;\
    uint8_t SET_POWER_CONSUMPTION_ACTIVE_LEVEL_VALUE = 0;\
    double SET_POWER_CONSUMPTION_WATTS_VALUE = 0;
    #define SET_POWER_CONSUMPTION_LONG_OPT_STRING "setPowerConsumption"
    #define SET_POWER_CONSUMPTION_LONG_OPT { SET_POWER_CONSUMPTION_LONG_OPT_STRING, required_argument, NULL, 0 }

    //sanitize TODO: This is incomplete. Need to expand this a bit more and then clean up the implementation for command line parsing in openSeaChest_Erase
    #define SANITIZE_RUN_FLAG sanitize
    #define SANITIZE_UTIL_VARS \
    bool SANITIZE_RUN_FLAG = false; \
    bool sanitizeInfo = false; \
    bool sanblockErase = false; \
    bool sancryptoErase = false; \
    bool sanoverwrite = false; \
    bool sanfreezelock = false; \
    bool sanAntiFreezeLock = false;
    #define SANITIZE_LONG_OPT_STRING "sanitize"

    //download FW
    #define FIRMWARE_FILE_NAME_MAX_LEN 4096
    #define DOWNLOAD_FW_FLAG downloadFW
    #define DOWNLOAD_FW_FILENAME_FLAG downloadFWFilename
    #define DOWNLOAD_FW_MODE downloadMode
    #define USER_SET_DOWNLOAD_MODE userSpecifiedDownloadMode
    #define DOWNLOAD_FW_VARS \
    bool DOWNLOAD_FW_FLAG = false;\
    char firmwareFileName[FIRMWARE_FILE_NAME_MAX_LEN] = { 0 };\
    char *DOWNLOAD_FW_FILENAME_FLAG =  &firmwareFileName[0];\
    int DOWNLOAD_FW_MODE = DL_FW_SEGMENTED;/*3*/\
    bool USER_SET_DOWNLOAD_MODE = false;
    #define DOWNLOAD_FW_LONG_OPT_STRING "downloadFW"
    #define DOWNLOAD_FW_MODE_LONG_OPT_STRING "downloadMode"
    #define DOWNLOAD_FW_LONG_OPT { DOWNLOAD_FW_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define DOWNLOAD_FW_MODE_LONG_OPT { DOWNLOAD_FW_MODE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //activate deferred FW
    #define ACTIVATE_DEFERRED_FW_FLAG activateDeferredFW
    #define ACTIVATE_DEFERRED_FW_VAR getOptBool activateDeferredFW = goFalse;
    #define ACTIVATE_DEFERRED_FW_LONG_OPT_STRING "activateFW"
    #define ACTIVATE_DEFERRED_FW_LONG_OPT { ACTIVATE_DEFERRED_FW_LONG_OPT_STRING, no_argument, &ACTIVATE_DEFERRED_FW_FLAG, goTrue }

    //switch FW slot (activates firmware already stored on an NVMe drive to a different slot) (NVMe option since it supports multiple slots)
    #define SWITCH_FW_FLAG switchFW
    #define SWITCH_FW_VAR getOptBool SWITCH_FW_FLAG = goFalse;
    #define SWITCH_FW_LONG_OPT_STRING "switchFW"
    #define SWITCH_FW_LONG_OPT { SWITCH_FW_LONG_OPT_STRING, no_argument, &SWITCH_FW_FLAG, goTrue }

    //Win10 allow flexible use of Win10 api for any supported command to any device on any interface (removes strict requirement that the matching command to device type and interface type is required)
    #define WIN10_FLEXIBLE_API_USE_FLAG windows10AllowFlexibleUseOfWinFWDLAPI
    #define WIN10_FLEXIBLE_API_USE_VAR getOptBool WIN10_FLEXIBLE_API_USE_FLAG = goFalse;
    #define WIN10_FLEXIBLE_API_USE_LONG_OPT_STRING "allowFlexibleFWDLAPIUse"
    #define WIN10_FLEXIBLE_API_USE_LONG_OPT { WIN10_FLEXIBLE_API_USE_LONG_OPT_STRING, no_argument, &WIN10_FLEXIBLE_API_USE_FLAG, goTrue }

    //this is a troubleshooting option when trying to update firmware and it's not working with the Win10 API.
    #define WIN10_FWDL_FORCE_PT_FLAG windows10ForceFWDLPassthrough
    #define WIN10_FWDL_FORCE_PT_VAR getOptBool WIN10_FWDL_FORCE_PT_FLAG = goFalse;
    #define WIN10_FWDL_FORCE_PT_LONG_OPT_STRING "forceFWDLPassthrough"
    #define WIN10_FWDL_FORCE_PT_LONG_OPT { WIN10_FWDL_FORCE_PT_LONG_OPT_STRING, no_argument, &WIN10_FWDL_FORCE_PT_FLAG, goTrue }

    //FW slot
    #define FIRMWARE_SLOT_FLAG firmwareSlot
    #define FIRMWARE_SLOT_VAR uint8_t FIRMWARE_SLOT_FLAG = 0;//default to zero should be ok
    #define FIRMWARE_SLOT_LONG_OPT_STRING "firmwareSlot"
    #define FIRMWARE_BUFFER_ID_LONG_OPT_STRING "fwBufferID"
    #define FIRMWARE_SLOT_LONG_OPT { FIRMWARE_SLOT_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define FIRMWARE_BUFFER_ID_LONG_OPT { FIRMWARE_BUFFER_ID_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define FIRMWARE_SLOT_BUFFER_ID_LONG_OPT FIRMWARE_SLOT_LONG_OPT,FIRMWARE_BUFFER_ID_LONG_OPT

    //model number match
    #define MODEL_MATCH_FLAG modelNumberMatch
    #define MODEL_STRING_FLAG modelString
    #define MODEL_MATCH_VARS \
    bool MODEL_MATCH_FLAG = false;\
    char modelMatchArray[41] = { 0 };\
    char *MODEL_STRING_FLAG = &modelMatchArray[0];
    #define MODEL_MATCH_LONG_OPT_STRING "modelMatch"
    #define MODEL_MATCH_LONG_OPT { MODEL_MATCH_LONG_OPT_STRING, required_argument, NULL, 0 }

    //fw rev match
    #define FW_MATCH_FLAG fwNumberMatch
    #define FW_STRING_FLAG fwString
    #define FW_MATCH_VARS \
    bool FW_MATCH_FLAG = false;\
    char fwMatchArray[9] = { 0 };\
    char *FW_STRING_FLAG = &fwMatchArray[0];
    #define FW_MATCH_LONG_OPT_STRING "onlyFW"
    #define FW_MATCH_LONG_OPT { FW_MATCH_LONG_OPT_STRING, required_argument, NULL, 0 }

    //new fw rev match
    #define NEW_FW_MATCH_FLAG newfwNumberMatch
    #define NEW_FW_STRING_FLAG newfwString
    #define NEW_FW_MATCH_VARS \
    bool NEW_FW_MATCH_FLAG = false;\
    char newfwMatchArray[9] = { 0 };\
    char *NEW_FW_STRING_FLAG = &newfwMatchArray[0];
    #define NEW_FW_MATCH_LONG_OPT_STRING "newFW"
    #define NEW_FW_MATCH_LONG_OPT { NEW_FW_MATCH_LONG_OPT_STRING, required_argument, NULL, 0 }

    //child model number match
    #define CHILD_MODEL_MATCH_FLAG childModelNumberMatch
    #define CHILD_MODEL_STRING_FLAG childModelString
    #define CHILD_MODEL_MATCH_VARS \
    bool CHILD_MODEL_MATCH_FLAG = false;\
    char childModelMatchArray[41] = { 0 };\
    char *CHILD_MODEL_STRING_FLAG = &childModelMatchArray[0];
    #define CHILD_MODEL_MATCH_LONG_OPT_STRING "childModelMatch"
    #define CHILD_MODEL_MATCH_LONG_OPT { CHILD_MODEL_MATCH_LONG_OPT_STRING, required_argument, NULL, 0 }

    //child fw rev match
    #define CHILD_FW_MATCH_FLAG childFwNumberMatch
    #define CHILD_FW_STRING_FLAG childFwString
    #define CHILD_FW_MATCH_VARS \
    bool CHILD_FW_MATCH_FLAG = false;\
    char childFwMatchArray[9] = { 0 };\
    char *CHILD_FW_STRING_FLAG = &childFwMatchArray[0];
    #define CHILD_FW_MATCH_LONG_OPT_STRING "childOnlyFW"
    #define CHILD_FW_MATCH_LONG_OPT { CHILD_FW_MATCH_LONG_OPT_STRING, required_argument, NULL, 0 }

    //child new fw rev match
    #define CHILD_NEW_FW_MATCH_FLAG childNewfwNumberMatch
    #define CHILD_NEW_FW_STRING_FLAG childNewfwString
    #define CHILD_NEW_FW_MATCH_VARS \
    bool CHILD_NEW_FW_MATCH_FLAG = false;\
    char childNewfwMatchArray[9] = { 0 };\
    char *CHILD_NEW_FW_STRING_FLAG = &childNewfwMatchArray[0];
    #define CHILD_NEW_FW_MATCH_LONG_OPT_STRING "childNewFW"
    #define CHILD_NEW_FW_MATCH_LONG_OPT { CHILD_NEW_FW_MATCH_LONG_OPT_STRING, required_argument, NULL, 0 }

    //setmaxlba
    #define SET_MAX_LBA_FLAG setMaxLBA
    #define SET_MAX_LBA_VALUE newMaxLBA
    #define SET_MAX_LBA_VARS \
    bool SET_MAX_LBA_FLAG = false;\
    uint64_t SET_MAX_LBA_VALUE = UINT64_MAX;
    #define SET_MAX_LBA_LONG_OPT_STRING "setMaxLBA"
    #define SET_MAX_LBA_LONG_OPT { SET_MAX_LBA_LONG_OPT_STRING, required_argument, NULL, 0 }

    //restore maxlba
    #define RESTORE_MAX_LBA_FLAG restoreMaxLBA
    #define RESTORE_MAX_LBA_VAR getOptBool RESTORE_MAX_LBA_FLAG = goFalse;
    #define RESTORE_MAX_LBA_LONG_OPT_STRING "restoreMaxLBA"
    #define RESTORE_MAX_LBA_LONG_OPT { RESTORE_MAX_LBA_LONG_OPT_STRING, no_argument, &RESTORE_MAX_LBA_FLAG, goTrue }

    //requires setmaxLBA AND TRIM/UNMAP flags since a TRIM/UNMAP is sent to the drive before setting the maxLBA
    #define PROVISION_FLAG provisionDrive
    #define PROVISION_VAR bool provisionDrive = false;
    #define PROVISION_LONG_OPT_STRING "provision"
    #define PROVISION_LONG_OPT { PROVISION_LONG_OPT_STRING, required_argument, NULL, 0 }

    //set phy speed
    #define SET_PHY_ALL_PHYS setAllPhys
    #define SET_PHY_SPEED_FLAG setPhy
    #define SET_PHY_SPEED_GEN phySpeedGen
    #define SET_PHY_SPEED_VARS \
    bool SET_PHY_SPEED_FLAG = false;\
    uint8_t SET_PHY_SPEED_GEN = 0;\
    bool SET_PHY_ALL_PHYS = true;//this will be changed to false when a specific phy is selected for SAS
    #define SET_PHY_SPEED_LONG_OPT_STRING "phySpeed"
    #define SET_PHY_SPEED_LONG_OPT { SET_PHY_SPEED_LONG_OPT_STRING, required_argument, NULL, 0 }

    //--sasPhy....to be used with various other SAS operations that need an identifier.
    #define SET_PHY_SAS_PHY_IDENTIFIER sasPhyIdentifier
    #define SET_PHY_SAS_PHY_IDENTIFIER_VAR uint8_t SET_PHY_SAS_PHY_IDENTIFIER = 0xFF;
    #define SET_PHY_SAS_PHY_LONG_OPT_STRING "sasPhy"
    #define SET_PHY_SAS_PHY_LONG_OPT { SET_PHY_SAS_PHY_LONG_OPT_STRING, required_argument, NULL, 0 }

    //sas phy link test patterns options...STD spec, but a couple options may be Seagate only.
    #define SAS_PHY_TEST_PATTERN testPattern
    #define SAS_PHY_TEST_FUNCTION_SSC testFunctionSSC
    #define SAS_PHY_PHYSICAL_LINK_RATE physicalLinkRate
    #define SAS_PHY_DWORD_CONTROL sasPhyTestDWordControl
    #define SAS_PHY_TEST_PATTERN_DWORDS testPatternDWords
    #define SAS_PHY_START_TEST_PATTERN_FLAG sasPhyStartTestPattern
    #define SAS_PHY_STOP_TEST_PATTERN_FLAG sasPhyStopTestPattern

    #define SAS_PHY_START_TEST_PATTERN_VARS \
    int SAS_PHY_TEST_PATTERN = 0;\
    int SAS_PHY_TEST_FUNCTION_SSC = 0;\
    uint8_t SAS_PHY_PHYSICAL_LINK_RATE = 0;\
    int SAS_PHY_DWORD_CONTROL = 0xFF;\
    uint64_t SAS_PHY_TEST_PATTERN_DWORDS = 0xFF;\
    getOptBool SAS_PHY_START_TEST_PATTERN_FLAG = goFalse;\
    getOptBool SAS_PHY_STOP_TEST_PATTERN_FLAG = goFalse;

    #define SAS_PHY_START_TEST_PATTERN_LONG_OPT_STRING "sasStartPhyTestPattern"
    #define SAS_PHY_START_TEST_PATTERN_LONG_OPT { SAS_PHY_START_TEST_PATTERN_LONG_OPT_STRING, no_argument, &SAS_PHY_START_TEST_PATTERN_FLAG, goTrue }
    #define SAS_PHY_STOP_TEST_PATTERN_LONG_OPT_STRING "sasStopPhyTestPattern"
    #define SAS_PHY_STOP_TEST_PATTERN_LONG_OPT { SAS_PHY_STOP_TEST_PATTERN_LONG_OPT_STRING, no_argument, &SAS_PHY_STOP_TEST_PATTERN_FLAG, goTrue }
    #define SAS_PHY_TEST_PATTERN_LONG_OPT_STRING "sasPhyTestPattern"
    #define SAS_PHY_TEST_PATTERN_LONG_OPT { SAS_PHY_TEST_PATTERN_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SAS_PHY_TEST_FUNCTION_SSC_LONG_OPT_STRING "sasPhySSC"
    #define SAS_PHY_TEST_FUNCTION_SSC_LONG_OPT { SAS_PHY_TEST_FUNCTION_SSC_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SAS_PHY_LINK_RATE_LONG_OPT_STRING "sasPhyTestLinkRate"
    #define SAS_PHY_LINK_RATE_LONG_OPT { SAS_PHY_LINK_RATE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SAS_PHY_DWORD_CONTROL_LONG_OPT_STRING "sasPhyDWordControl"
    #define SAS_PHY_DWORD_CONTROL_LONG_OPT { SAS_PHY_DWORD_CONTROL_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SAS_PHY_DWORDS_LONG_OPT_STRING "sasPhyDwordPattern"
    #define SAS_PHY_DWORDS_LONG_OPT { SAS_PHY_DWORDS_LONG_OPT_STRING, required_argument, NULL, 0 }

    //set/get SSC
    #define SET_SSC_FLAG setSSC
    #define GET_SSC_FLAG getSSC
    #define SSC_MODE sscMode
    #define SSC_FEATURE_VARS \
    bool SET_SSC_FLAG = false;\
    bool GET_SSC_FLAG = false;\
    int SSC_MODE = 0;
    #define SSC_FEATURE_LONG_OPT_STRING "sscFeature"
    #define SSC_FEATURE_LONG_OPT { SSC_FEATURE_LONG_OPT_STRING, required_argument, NULL, 0 }


    //set ready LED - previously misnamed pin11
    #define READY_LED_INFO_FLAG readyLEDInfo
    #define SET_READY_LED_FLAG setReadyLED
    #define SET_READY_LED_MODE readyLEDMode //on or off or default
    #define SET_READY_LED_DEFAULT readyLEDDefault
    #define SET_READY_LED_VARS \
    bool SET_READY_LED_FLAG = false;\
    bool SET_READY_LED_MODE = false;\
    bool SET_READY_LED_DEFAULT = false;\
    bool READY_LED_INFO_FLAG = false;
    #define SET_PIN_11_LONG_OPT_STRING "pin11" //left for backwards compatibility
    #define SET_PIN_11_LONG_OPT { SET_PIN_11_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define SET_READY_LED_LONG_OPT_STRING "readyLED" //left for backwards compatibility
	#define SET_READY_LED_LONG_OPT { SET_READY_LED_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define SET_READY_LED_LONG_OPTS SET_PIN_11_LONG_OPT, SET_READY_LED_LONG_OPT

    //read look ahead
    #define READ_LOOK_AHEAD_INFO readLookAheadInfo
    #define READ_LOOK_AHEAD_SETTING readLookAheadSetting
    #define READ_LOOK_AHEAD_FLAG setReadLookAhead
    #define READ_LOOK_AHEAD_VARS \
    bool READ_LOOK_AHEAD_INFO = false;\
    bool READ_LOOK_AHEAD_SETTING = false;\
    bool READ_LOOK_AHEAD_FLAG = false;
    #define READ_LOOK_AHEAD_LONG_OPT_STRING "readLookAhead"
    #define READ_LOOK_AHEAD_LONG_OPT { READ_LOOK_AHEAD_LONG_OPT_STRING, required_argument, NULL, 0 }

    //write cache
    #define WRITE_CACHE_INFO writeCacheInfo
    #define WRITE_CACHE_SETTING writeCacheSetting
    #define WRITE_CACHE_FLAG setWriteCache
    #define WRITE_CACHE_VARS \
    bool WRITE_CACHE_INFO = false;\
    bool WRITE_CACHE_SETTING = false;\
    bool WRITE_CACHE_FLAG = false;
    #define WRITE_CACHE_LONG_OPT_STRING "writeCache"
    #define WRITE_CACHE_LONG_OPT { WRITE_CACHE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //sct write cache (enable/disable/default)
    #define SCT_WRITE_CACHE_INFO sctwriteCacheInfo
    #define SCT_WRITE_CACHE_SETTING sctwriteCacheSetting
    #define SCT_WRITE_CACHE_SET_DEFAULT sctWriteCacheDefault
    #define SCT_WRITE_CACHE_FLAG sctsetWriteCache
    #define SCT_WRITE_CACHE_VARS \
    bool SCT_WRITE_CACHE_INFO = false;\
    bool SCT_WRITE_CACHE_SETTING = false;\
    bool SCT_WRITE_CACHE_FLAG = false;\
    bool SCT_WRITE_CACHE_SET_DEFAULT = false;
    #define SCT_WRITE_CACHE_LONG_OPT_STRING "sctWriteCache"
    #define SCT_WRITE_CACHE_LONG_OPT { SCT_WRITE_CACHE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //sct write cache reordering
    #define SCT_WRITE_CACHE_REORDER_INFO sctwriteCacheReorderInfo
    #define SCT_WRITE_CACHE_REORDER_SETTING sctwriteCacheReorderSetting
    #define SCT_WRITE_CACHE_REORDER_SET_DEFAULT sctWriteCacheReorderDefault
    #define SCT_WRITE_CACHE_REORDER_FLAG sctsetWriteCacheReorder
    #define SCT_WRITE_CACHE_REORDER_VARS \
    bool SCT_WRITE_CACHE_REORDER_INFO = false;\
    bool SCT_WRITE_CACHE_REORDER_SETTING = false;\
    bool SCT_WRITE_CACHE_REORDER_FLAG = false;\
    bool SCT_WRITE_CACHE_REORDER_SET_DEFAULT = false;
    #define SCT_WRITE_CACHE_REORDER_LONG_OPT_STRING "sctWriteCacheReordering"
    #define SCT_WRITE_CACHE_REORDER_LONG_OPT { SCT_WRITE_CACHE_REORDER_LONG_OPT_STRING, required_argument, NULL, 0 }

    //sct error recovery control
    #define SCT_ERROR_RECOVERY_CONTROL_READ_INFO showSCTErrorRecoveryReadInfo
    #define SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO showSCTErrorRecoveryWriteInfo
    #define SCT_ERROR_RECOVERY_CONTROL_SET_READ_TIMER setSCTErrorRecoveryControlReadTimer
    #define SCT_ERROR_RECOVERY_CONTROL_READ_TIMER_VALUE sctErrorRecoveryControlReadTimerValue
    #define SCT_ERROR_RECOVERY_CONTROL_SET_WRITE_TIMER setSCTErrorRecoveryControlWriteTimer
    #define SCT_ERROR_RECOVERY_CONTROL_WRITE_TIMER_VALUE sctErrorRecoveryControlWriteTimerValue
    #define SCT_ERROR_RECOVERY_CONTROL_READ_VARS \
    bool SCT_ERROR_RECOVERY_CONTROL_READ_INFO = false;\
    bool SCT_ERROR_RECOVERY_CONTROL_SET_READ_TIMER = false;\
    uint32_t SCT_ERROR_RECOVERY_CONTROL_READ_TIMER_VALUE = 0;
    #define SCT_ERROR_RECOVERY_CONTROL_WRITE_VARS \
    bool SCT_ERROR_RECOVERY_CONTROL_WRITE_INFO = false;\
    bool SCT_ERROR_RECOVERY_CONTROL_SET_WRITE_TIMER = false;\
    uint32_t SCT_ERROR_RECOVERY_CONTROL_WRITE_TIMER_VALUE = 0;
    #define SCT_ERROR_RECOVERY_CONTROL_VARS \
    SCT_ERROR_RECOVERY_CONTROL_READ_VARS \
    SCT_ERROR_RECOVERY_CONTROL_WRITE_VARS
    #define SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING "sctReadTimer"
    #define SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING "sctWriteTimer"
    #define SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT { SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT { SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCT_ERROR_RECOVERY_CONTROL_LONG_OPTS SCT_ERROR_RECOVERY_CONTROL_READ_LONG_OPT,SCT_ERROR_RECOVERY_CONTROL_WRITE_LONG_OPT

    //volatile
    #define VOLATILE_FLAG isVolatile
    #define VOLATILE_VAR getOptBool VOLATILE_FLAG = goFalse;
    #define VOLATILE_LONG_OPT_STRING "volatile"
    #define VOLATILE_LONG_OPT { VOLATILE_LONG_OPT_STRING, no_argument, &VOLATILE_FLAG, goTrue }

    //Show supported erase functionality
    #define SHOW_ERASE_SUPPORT_FLAG showEraseSupport
    #define SHOW_ERASE_SUPPORT_VAR getOptBool SHOW_ERASE_SUPPORT_FLAG = goFalse;
    #define SHOW_ERASE_SUPPORT_LONG_OPT_STRING "showEraseSupport"
    #define SHOW_ERASE_SUPPORT_LONG_OPT { SHOW_ERASE_SUPPORT_LONG_OPT_STRING, no_argument, &SHOW_ERASE_SUPPORT_FLAG, goTrue }

    //fastest erase
    #define PERFORM_FASTEST_ERASE_FLAG performFastestErase
    #define PERFORM_FASTEST_ERASE_VAR getOptBool PERFORM_FASTEST_ERASE_FLAG = goFalse;
    #define PERFORM_FASTEST_ERASE_LONG_OPT_STRING "performQuickestErase"
    #define PERFORM_FASTEST_ERASE_LONG_OPT { PERFORM_FASTEST_ERASE_LONG_OPT_STRING, no_argument, &PERFORM_FASTEST_ERASE_FLAG, goTrue }

    //format unit
    #define FORMAT_UNIT_FLAG formatUnit
    #define FORMAT_SECTOR_SIZE formatLogicalSectorSize
    #define FORMAT_UNIT_VARS \
    bool FORMAT_UNIT_FLAG = false; \
    uint32_t FORMAT_SECTOR_SIZE = 0;
    #define FORMAT_UNIT_LONG_OPT_STRING "formatUnit"
    #define FORMAT_UNIT_LONG_OPT { FORMAT_UNIT_LONG_OPT_STRING, required_argument, NULL, 0 }

    //other format unit options
    #define FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG       disablePrimaryList
    #define FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG  discardExistingGList
    #define FORMAT_UNIT_DISABLE_CERTIFICATION           disableCertification
    #define FORMAT_UNIT_SECURITY_INITIALIZE             formatSecurityInitialize
    #define FORMAT_UNIT_PROECTION_TYPE_FROM_USER        userSpecifiedPI
    #define FORMAT_UNIT_PROTECTION_TYPE                 formatProtectionType
    #define FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT    formatProtectionIntervalExponent
    #define FORMAT_UNIT_PROECTION_INTERVAL_EXPONENT_FROM_USER        userSpecifiedPIexponent
    #define FORMAT_UNIT_DEFAULT_FORMAT                  performDefaultFormat
    #define FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE      disableFormatImmediateResponse
    #define FORMAT_UNIT_STOP_ON_LIST_ERROR              formatStopOnListError
	#define FORMAT_UNIT_NEW_MAX_LBA                     newMaxLBA

    #define FORMAT_UNIT_OPTION_FLAGS \
    getOptBool FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG = goFalse;\
    getOptBool FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG = goFalse;\
    getOptBool FORMAT_UNIT_DISABLE_CERTIFICATION = goFalse;\
    getOptBool FORMAT_UNIT_SECURITY_INITIALIZE = goFalse;\
    bool FORMAT_UNIT_PROECTION_TYPE_FROM_USER = false;\
    uint8_t FORMAT_UNIT_PROTECTION_TYPE = 0;\
    bool FORMAT_UNIT_PROECTION_INTERVAL_EXPONENT_FROM_USER = false;\
    uint8_t FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT = 0;\
    getOptBool FORMAT_UNIT_DEFAULT_FORMAT = goFalse;\
    getOptBool FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE = goFalse;\
    getOptBool FORMAT_UNIT_STOP_ON_LIST_ERROR = goFalse;\
    uint64_t FORMAT_UNIT_NEW_MAX_LBA = 0;

    #define FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG_LONG_OPT_STRING "disablePrimaryList"
    #define FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG_LONG_OPT_STRING "discardGList"
    #define FORMAT_UNIT_DISABLE_CERTIFICATION_LONG_OPT_STRING "disableCertification"
    #define FORMAT_UNIT_SECURITY_INITIALIZE_LONG_OPT_STRING "securityInitialize"
    #define FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT_STRING "protectionType"
    #define FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT_LONG_OPT_STRING "protectionIntervalExponent"
    #define FORMAT_UNIT_DEFAULT_FORMAT_LONG_OPT_STRING "defaultFormat"
    #define FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE_LONG_OPT_STRING "disableImmediateResponse"  
    #define FORMAT_UNIT_STOP_ON_LIST_ERROR_LONG_OPT_STRING "stopOnListError"
	#define FORMAT_UNIT_NEW_MAX_LBA_LONG_OPT_STRING "formatMaxLBA"

    #define FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG_LONG_OPT { FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG_LONG_OPT_STRING, no_argument, &FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG, goTrue }
    #define FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG_LONG_OPT { FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG_LONG_OPT_STRING, no_argument, &FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG, goTrue }
    #define FORMAT_UNIT_DISABLE_CERTIFICATION_LONG_OPT { FORMAT_UNIT_DISABLE_CERTIFICATION_LONG_OPT_STRING, no_argument, &FORMAT_UNIT_DISABLE_CERTIFICATION, goTrue }
    #define FORMAT_UNIT_SECURITY_INITIALIZE_LONG_OPT { FORMAT_UNIT_SECURITY_INITIALIZE_LONG_OPT_STRING, no_argument, &FORMAT_UNIT_SECURITY_INITIALIZE, goTrue }
    #define FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT { FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT_LONG_OPT { FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define FORMAT_UNIT_DEFAULT_FORMAT_LONG_OPT { FORMAT_UNIT_DEFAULT_FORMAT_LONG_OPT_STRING, no_argument, &FORMAT_UNIT_DEFAULT_FORMAT, goTrue }
    #define FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE_LONG_OPT { FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE_LONG_OPT_STRING, no_argument, &FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE, goTrue }
    #define FORMAT_UNIT_STOP_ON_LIST_ERROR_LONG_OPT { FORMAT_UNIT_STOP_ON_LIST_ERROR_LONG_OPT_STRING, no_argument, &FORMAT_UNIT_STOP_ON_LIST_ERROR, goTrue }
	#define FORMAT_UNIT_NEW_MAX_LBA_LONG_OPT { FORMAT_UNIT_NEW_MAX_LBA_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define FORMAT_UNIT_ADDITIONAL_OPTIONS\
     FORMAT_UNIT_DISABLE_PRIMARY_LIST_FLAG_LONG_OPT,FORMAT_UNIT_DISCARD_GROWN_DEFECT_LIST_FLAG_LONG_OPT,FORMAT_UNIT_DISABLE_CERTIFICATION_LONG_OPT,FORMAT_UNIT_SECURITY_INITIALIZE_LONG_OPT,\
     FORMAT_UNIT_PROTECTION_TYPE_LONG_OPT,FORMAT_UNIT_PROTECTION_INTERVAL_EXPONENT_LONG_OPT,FORMAT_UNIT_DEFAULT_FORMAT_LONG_OPT,FORMAT_UNIT_DISABLE_IMMEDIATE_RESPONSE_LONG_OPT,FORMAT_UNIT_STOP_ON_LIST_ERROR_LONG_OPT,FORMAT_UNIT_NEW_MAX_LBA_LONG_OPT

	//NVM Format
	#define NVM_FORMAT_FLAG nvmFormat
	#define NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM nvmFormatDetails
	#define NVM_FORMAT_VARS \
	bool NVM_FORMAT_FLAG = false;\
	uint32_t NVM_FORMAT_SECTOR_SIZE_OR_FORMAT_NUM = 16;/*leave this at 16 since it's neither a valid sector size or a valid format to format the drive with.*/
	#define NVM_FORMAT_LONG_OPT_STRING "nvmFormat"
	#define NVM_FORMAT_LONG_OPT { NVM_FORMAT_LONG_OPT_STRING, required_argument, NULL, 0 }

	//Additional flags for nvmeFormat
	#define NVM_FORMAT_NSID				nvmFormatNSID //default to all F's
	#define NVM_FORMAT_SECURE_ERASE		nvmFormatSecureEraseType //default to 0, no erase
	#define NVM_FORMAT_PI_TYPE			nvmFormatPI //default to UINT8_MAX to filter out when it's provided or not.
	#define NVM_FORMAT_PI_LOCATION		nvmFormatPILocation //default to UINT8_MAX to filter out when it's provided or not.
	#define NVM_FORMAT_METADATA_SIZE	nvmFormatMetadataSize //default to UINT32_MAX to filter out when it is or is not being provided by the user
	#define NVM_FORMAT_METADATA_SETTING	nvmFormatMetadataSetting //default to UINT8_MAX to filter out when it's provided or not.

	#define NVM_FORMAT_OPTION_VARS \
	uint32_t NVM_FORMAT_NSID = UINT32_MAX;\
	uint8_t NVM_FORMAT_SECURE_ERASE = 0;\
	uint8_t NVM_FORMAT_PI_TYPE = UINT8_MAX;\
	uint8_t NVM_FORMAT_PI_LOCATION = UINT8_MAX;\
	uint32_t NVM_FORMAT_METADATA_SIZE = UINT32_MAX;\
	uint8_t NVM_FORMAT_METADATA_SETTING = UINT8_MAX;

	#define NVM_FORMAT_NSID_LONG_OPT_STRING				"nvmFmtNSID" //[all | current]
	#define NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING		"nvmFmtSecErase" //[none | user | crypto]
	#define NVM_FORMAT_PI_TYPE_LONG_OPT_STRING		"nvmFmtPI" //[0 | 1 | 2 | 3]
	#define NVM_FORMAT_PI_LOCATION_LONG_OPT_STRING		"nvmFmtPIL" //[beginning | end]
	#define NVM_FORMAT_METADATA_SIZE_LONG_OPT_STRING	"nvmFmtMS" //value in bytes for metadata size
	#define NVM_FORMAT_METADATA_SETTING_LONG_OPT_STRING "nvmFmtMetadataSet" //[xlba | separate]

	#define NVM_FORMAT_NSID_LONG_OPT { NVM_FORMAT_NSID_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define NVM_FORMAT_SECURE_ERASE_LONG_OPT { NVM_FORMAT_SECURE_ERASE_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define NVM_FORMAT_PI_TYPE_LONG_OPT { NVM_FORMAT_PI_TYPE_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define NVM_FORMAT_PI_LOCATION_LONG_OPT { NVM_FORMAT_PI_LOCATION_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define NVM_FORMAT_METADATA_SIZE_LONG_OPT { NVM_FORMAT_METADATA_SIZE_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define NVM_FORMAT_METADATA_SETTING_LONG_OPT { NVM_FORMAT_METADATA_SETTING_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define NVM_FORMAT_OPTIONS_LONG_OPTS \
		NVM_FORMAT_NSID_LONG_OPT,NVM_FORMAT_SECURE_ERASE_LONG_OPT,NVM_FORMAT_PI_TYPE_LONG_OPT,NVM_FORMAT_PI_LOCATION_LONG_OPT,NVM_FORMAT_METADATA_SIZE_LONG_OPT,NVM_FORMAT_METADATA_SETTING_LONG_OPT

    //show format status log
    #define SHOW_FORMAT_STATUS_LOG_FLAG showFormatStatusLog
    #define SHOW_FORMAT_STATUS_LOG_VAR getOptBool SHOW_FORMAT_STATUS_LOG_FLAG = goFalse;
    #define SHOW_FORMAT_STATUS_LOG_LONG_OPT_STRING "showFormatStatusLog"
    #define SHOW_FORMAT_STATUS_LOG_LONG_OPT { SHOW_FORMAT_STATUS_LOG_LONG_OPT_STRING, no_argument, &SHOW_FORMAT_STATUS_LOG_FLAG, goTrue }

    //fast format
    #define FAST_FORMAT_FLAG fastFormat
    #define FAST_FORMAT_VAR int FAST_FORMAT_FLAG = 0;
    #define FAST_FORMAT_LONG_OPT_STRING "fastFormat"
    #define FAST_FORMAT_LONG_OPT { FAST_FORMAT_LONG_OPT_STRING, required_argument, NULL, 0 }

    //set sector size
    #define SET_SECTOR_SIZE_FLAG setSectorSize
    #define SET_SECTOR_SIZE_SIZE setSectorSizeLogicalSectorSize
    #define SET_SECTOR_SIZE_VARS \
    bool setSectorSize = false;\
    uint32_t setSectorSizeLogicalSectorSize = 0;
    #define SET_SECTOR_SIZE_LONG_OPT_STRING "setSectorSize"
    #define SET_SECTOR_SIZE_LONG_OPT { SET_SECTOR_SIZE_LONG_OPT_STRING, required_argument, NULL, 0 }

	//related to formatting and setting sector sizes
    #define SHOW_SUPPORTED_FORMATS_FLAG showSupportedFormats
    #define SHOW_SUPPORTED_FORMATS_VAR getOptBool SHOW_SUPPORTED_FORMATS_FLAG = goFalse;
    #define SHOW_SUPPORTED_FORMATS_LONG_OPT_STRING "showSupportedFormats"
    #define SHOW_SUPPORTED_FORMATS_LONG_OPT { SHOW_SUPPORTED_FORMATS_LONG_OPT_STRING, no_argument, &SHOW_SUPPORTED_FORMATS_FLAG, goTrue }

    //port locking/unlocking (UDS, IEEE1667, FWDL)
    #define FWDL_PORT_FLAG fwdlPortLock
    #define FWDL_PORT_MODE_FLAG fwdlPortLockMode
    #define FWDL_PORT_VARS bool FWDL_PORT_FLAG = false; bool FWDL_PORT_MODE_FLAG = false;
    #define FWDL_PORT_LONG_OPT_STRING "fwdlPort"
    #define FWDL_PORT_LONG_OPT { FWDL_PORT_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define UDS_PORT_FLAG udsPortLock
    #define UDS_PORT_MODE_FLAG udsPortLockMode
    #define UDS_PORT_VARS bool UDS_PORT_FLAG = false; bool UDS_PORT_MODE_FLAG = false;
    #define UDS_PORT_LONG_OPT_STRING "udsPort"
    #define UDS_PORT_LONG_OPT { UDS_PORT_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define IEEE1667_PORT_FLAG ieee1667PortLock
    #define IEEE1667_PORT_MODE_FLAG ieee1667PortLockMode
    #define IEEE1667_PORT_VARS bool IEEE1667_PORT_FLAG = false; bool IEEE1667_PORT_MODE_FLAG = false;
    #define IEEE1667_PORT_LONG_OPT_STRING "ieee1667Port"
    #define IEEE1667_PORT_LONG_OPT { IEEE1667_PORT_LONG_OPT_STRING, required_argument, NULL, 0 }

    //low current spinup
    #define LOW_CURRENT_SPINUP_FLAG lowCurrentSpinUp
    #define LOW_CURRENT_SPINUP_STATE lowCurrentSpinupState
    #define LOW_CURRENT_SPINUP_VARS \
    bool LOW_CURRENT_SPINUP_FLAG = false;\
    int LOW_CURRENT_SPINUP_STATE = 0;
    #define LOW_CURRENT_SPINUP_LONG_OPT_STRING "lowCurrentSpinup"
    #define LOW_CURRENT_SPINUP_LONG_OPT { LOW_CURRENT_SPINUP_LONG_OPT_STRING, required_argument, NULL, 0 }

    //disable data locking
    #define DISABLE_DATA_LOCKING_FLAG disableDataLocking
    #define DISABLE_DATA_LOCKING_VAR getOptBool DISABLE_DATA_LOCKING_FLAG = goFalse;
    #define DISABLE_DATA_LOCKING_LONG_OPT_STRING "disableDataLocking"
    #define DISABLE_DATA_LOCKING_LONG_OPT { DISABLE_DATA_LOCKING_LONG_OPT_STRING, no_argument, &DISABLE_DATA_LOCKING_FLAG, goTrue }
    
    //Set APM Level
    #define SET_APM_LEVEL_FLAG setAPMLevel
    #define SET_APM_LEVEL_VALUE_FLAG setApmLevelValue
    #define SET_APM_LEVEL_VARS \
    bool SET_APM_LEVEL_FLAG = false;\
    uint8_t SET_APM_LEVEL_VALUE_FLAG = 0;
    #define SET_APM_LEVEL_LONG_OPT_STRING "setAPMLevel"
    #define SET_APM_LEVEL_LONG_OPT { SET_APM_LEVEL_LONG_OPT_STRING, required_argument, NULL, 0 }

    //disable APM
    #define DISABLE_APM_FLAG disableAPM
    #define DISABLE_APM_VAR getOptBool DISABLE_APM_FLAG = goFalse;
    #define DISABLE_APM_LONG_OPT_STRING "disableAPM"
    #define DISABLE_APM_LONG_OPT { DISABLE_APM_LONG_OPT_STRING, no_argument, &DISABLE_APM_FLAG, goTrue }
    
    //Show APM Level
    #define SHOW_APM_LEVEL_FLAG showAPMLevel
    #define SHOW_APM_LEVEL_VALUE_FLAG showApmLevelValue
    #define SHOW_APM_LEVEL_VARS \
    getOptBool SHOW_APM_LEVEL_FLAG = goFalse;\
    uint8_t SHOW_APM_LEVEL_VALUE_FLAG = 0;
    #define SHOW_APM_LEVEL_LONG_OPT_STRING "showAPMLevel"
    #define SHOW_APM_LEVEL_LONG_OPT { SHOW_APM_LEVEL_LONG_OPT_STRING, no_argument, &SHOW_APM_LEVEL_FLAG, goTrue }
    
    //PUIS Feature enable/disable
    #define PUIS_FEATURE_FLAG puisFeature
    #define PUIS_FEATURE_STATE_FLAG puisFeatureState
    #define PUIS_FEATURE_VARS \
    bool PUIS_FEATURE_FLAG = false;\
    bool puisFeatureState = false;/* False = disable, true = enable */
    #define PUIS_FEATURE_LONG_OPT_STRING "puisFeature"
    #define PUIS_FEATURE_LONG_OPT { PUIS_FEATURE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //Show EPC Settings
    #define SHOW_EPC_SETTINGS_FLAG showEPC
    #define SHOW_EPC_SETTINGS_VAR getOptBool SHOW_EPC_SETTINGS_FLAG = goFalse;
    #define SHOW_EPC_SETTINGS_LONG_OPT_STRING "showEPCSettings"
    #define SHOW_EPC_SETTINGS_LONG_OPT { SHOW_EPC_SETTINGS_LONG_OPT_STRING, no_argument, &SHOW_EPC_SETTINGS_FLAG, goTrue }
    
    //SMART Feature
    #define SMART_FEATURE_FLAG smartFeature
    #define SMART_FEATURE_STATE_FLAG enableDisableSMART
    #define SMART_FEATURE_VARS \
    bool SMART_FEATURE_FLAG = false;\
    bool SMART_FEATURE_STATE_FLAG = false;
    #define SMART_FEATURE_LONG_OPT_STRING "smartFeature"
    #define SMART_FEATURE_LONG_OPT { SMART_FEATURE_LONG_OPT_STRING, required_argument, NULL, 0 }
    
    //Set MRIE mode
    #define SET_MRIE_MODE_FLAG setMRIEMode
    #define SET_MRIE_MODE_VALUE setMRIEValue
    #define SET_MRIE_MODE_DEFAULT setMRIEDefault
    #define SET_MRIE_MODE_VARS \
    bool SET_MRIE_MODE_FLAG = false;\
    bool SET_MRIE_MODE_DEFAULT = false;\
    uint8_t SET_MRIE_MODE_VALUE = 6;
    #define SET_MRIE_MODE_LONG_OPT_STRING "setMRIE"
    #define SET_MRIE_MODE_LONG_OPT { SET_MRIE_MODE_LONG_OPT_STRING, required_argument, NULL, 0 }
    
    //SMART Attribute Autosave
    #define SMART_ATTR_AUTOSAVE_FEATURE_FLAG smartAttrAutosave
    #define SMART_ATTR_AUTOSAVE_FEATURE_STATE_FLAG enableDisableSMARTAtrAutosave
    #define SMART_ATTR_AUTOSAVE_FEATURE_VARS \
    bool SMART_ATTR_AUTOSAVE_FEATURE_FLAG = false;\
    bool SMART_ATTR_AUTOSAVE_FEATURE_STATE_FLAG = false;
    #define SMART_ATTR_AUTOSAVE_FEATURE_LONG_OPT_STRING "smartAttributeAutosave"
    #define SMART_ATTR_AUTOSAVE_FEATURE_LONG_OPT { SMART_ATTR_AUTOSAVE_FEATURE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //SMART Info
    #define SMART_INFO_FLAG smartInformation
    #define SMART_INFO_VAR getOptBool SMART_INFO_FLAG = goFalse;
    #define SMART_INFO_LONG_OPT_STRING "smartInfo"
    #define SMART_INFO_LONG_OPT { SMART_INFO_LONG_OPT_STRING, no_argument, &SMART_INFO_FLAG, goTrue }

    //SMART Auto Off-line
    #define SMART_AUTO_OFFLINE_FEATURE_FLAG smartAutoOffline
    #define SMART_AUTO_OFFLINE_FEATURE_STATE_FLAG enableDisableSMARTAutoOffline
    #define SMART_AUTO_OFFLINE_FEATURE_VARS \
    bool SMART_AUTO_OFFLINE_FEATURE_FLAG = false;\
    bool SMART_AUTO_OFFLINE_FEATURE_STATE_FLAG = false;
    #define SMART_AUTO_OFFLINE_FEATURE_LONG_OPT_STRING "smartAutoOffline"
    #define SMART_AUTO_OFFLINE_FEATURE_LONG_OPT { SMART_AUTO_OFFLINE_FEATURE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //Show DST log
    #define SHOW_DST_LOG_FLAG showDSTLog
    #define SHOW_DST_LOG_VAR getOptBool SHOW_DST_LOG_FLAG = goFalse;
    #define SHOW_DST_LOG_LONG_OPT_STRING "showDSTLog"
    #define SHOW_DST_LOG_LONG_OPT { SHOW_DST_LOG_LONG_OPT_STRING, no_argument, &SHOW_DST_LOG_FLAG, goTrue }

	//Generic Public Logs 
	#define LIST_LOGS_FLAG listSupportedLogs
	#define LIST_LOGS_VAR getOptBool LIST_LOGS_FLAG = goFalse;
	#define LIST_LOGS_LONG_OPT_STRING "listSupportedLogs"
	#define LIST_LOGS_LONG_OPT { LIST_LOGS_LONG_OPT_STRING, no_argument, &LIST_LOGS_FLAG, goTrue }

	#define GENERIC_LOG_PULL_FLAG pullGenericLog
	#define GENERIC_LOG_DATA_SET genericLogDataSet
	#define GENERIC_LOG_VAR \
		bool GENERIC_LOG_PULL_FLAG = false; \
		uint64_t GENERIC_LOG_DATA_SET = 0;
	#define GENERIC_LOG_LONG_OPT_STRING "pullLog"
	#define GENERIC_LOG_LONG_OPT { GENERIC_LOG_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define GENERIC_LOG_SUBPAGE_DATA_SET genericLogDataSetSubpage
	#define GENERIC_LOG_SUBPAGE_VAR \
		uint64_t GENERIC_LOG_SUBPAGE_DATA_SET = 0;
	#define GENERIC_LOG_SUBPAGE_LONG_OPT_STRING "pullSubpage"
	#define GENERIC_LOG_SUBPAGE_LONG_OPT { GENERIC_LOG_SUBPAGE_LONG_OPT_STRING, required_argument, NULL, 0 }

	#define PULL_LOG_MODE logMode
	#define PULL_LOG_MODE_LONG_OPT_STRING "logMode"
	#define PULL_LOG_MODE_LONG_OPT { PULL_LOG_MODE_LONG_OPT_STRING, required_argument, NULL, 0 }
	#define PULL_LOG_MODE_VARS \
		int PULL_LOG_MODE = 0;

    //Generic SCSI Error history stuff
    #define LIST_ERROR_HISTORY_FLAG listSupportedErrorHistoryBufferIDs
    #define LIST_ERROR_HISTORY_VAR getOptBool LIST_ERROR_HISTORY_FLAG = goFalse;
    #define LIST_ERROR_HISTORY_LONG_OPT_STRING "listErrorHistoryIDs"
    #define LIST_ERROR_HISTORY_LONG_OPT { LIST_ERROR_HISTORY_LONG_OPT_STRING, no_argument, &LIST_ERROR_HISTORY_FLAG, goTrue }

    #define GENERIC_ERROR_HISTORY_PULL_FLAG pullErrorHistory
    #define GENERIC_ERROR_HISTORY_BUFFER_ID errorHistoryBufferID
    #define GENERIC_ERROR_HISTORY_VARS \
    bool GENERIC_ERROR_HISTORY_PULL_FLAG = false;\
    uint64_t GENERIC_ERROR_HISTORY_BUFFER_ID = 0;
    #define GENERIC_ERROR_HISTORY_LONG_OPT_STRING "pullErrorHistoryID"
    #define GENERIC_ERROR_HISTORY_LONG_OPT { GENERIC_ERROR_HISTORY_LONG_OPT_STRING, required_argument, NULL, 0 }

	//device statistics (display)
    #define DEVICE_STATISTICS_FLAG showDeviceStatistics
    #define DEVICE_STATISTICS_VAR getOptBool DEVICE_STATISTICS_FLAG = goFalse;
    #define DEVICE_STATISTICS_LONG_OPT_STRING "deviceStatistics"
    #define DEVICE_STATISTICS_LONG_OPT { DEVICE_STATISTICS_LONG_OPT_STRING, no_argument, &DEVICE_STATISTICS_FLAG, goTrue }

    //SMR Options
    #define ZONE_ID_FLAG zoneID
    #define ZONE_ID_ALL_FLAG allZones
    #define ZONE_ID_VARS \
    uint64_t ZONE_ID_FLAG = UINT64_MAX;\
    bool ZONE_ID_ALL_FLAG = false;
    #define ZONE_ID_LONG_OPT_STRING "zoneID"
    #define ZONE_ID_LONG_OPT { ZONE_ID_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define MAX_ZONES_FLAG maxZones
    #define MAX_ZONES_VAR \
    uint32_t MAX_ZONES_FLAG = UINT64_MAX;
    #define MAX_ZONES_LONG_OPT_STRING "maxZones"
    #define MAX_ZONES_LONG_OPT { MAX_ZONES_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define REPORT_ZONES_FLAG reportZones
    #define REPORT_ZONES_REPORTING_MODE_FLAG zoneReportingMode
    #define REPORT_ZONES_VARS \
    bool REPORT_ZONES_FLAG = false;\
    int REPORT_ZONES_REPORTING_MODE_FLAG = 0;/*This will be cast to a proper enum type later...*/
    #define REPORT_ZONES_LONG_OPT_STRING "reportZones"
    #define REPORT_ZONES_LONG_OPT { REPORT_ZONES_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define CLOSE_ZONE_FLAG zoneClose
    #define FINISH_ZONE_FLAG zoneFinish
    #define OPEN_ZONE_FLAG zoneOpen
    #define RESET_WP_FLAG zoneReset
    #define ZONE_MANAGEMENT_VARS \
    getOptBool CLOSE_ZONE_FLAG = goFalse;\
    getOptBool FINISH_ZONE_FLAG = goFalse;\
    getOptBool OPEN_ZONE_FLAG = goFalse;\
    getOptBool RESET_WP_FLAG = goFalse;
    #define CLOSE_ZONE_LONG_OPT_STRING "closeZone"
    #define FINISH_ZONE_LONG_OPT_STRING "finishZone"
    #define OPEN_ZONE_LONG_OPT_STRING "openZone"
    #define RESET_WP_LONG_OPT_STRING "resetWritePointer"
    #define CLOSE_ZONE_LONG_OPT { CLOSE_ZONE_LONG_OPT_STRING, no_argument, &CLOSE_ZONE_FLAG, goTrue }
    #define FINISH_ZONE_LONG_OPT { FINISH_ZONE_LONG_OPT_STRING, no_argument, &FINISH_ZONE_FLAG, goTrue }
    #define OPEN_ZONE_LONG_OPT { OPEN_ZONE_LONG_OPT_STRING, no_argument, &OPEN_ZONE_FLAG, goTrue }
    #define RESET_WP_LONG_OPT { RESET_WP_LONG_OPT_STRING, no_argument, &RESET_WP_FLAG, goTrue }

    //fwdl segment size
    #define FWDL_SEGMENT_SIZE_FLAG fwdlSegmentSize
    #define FWDL_SEGMENT_SIZE_FROM_USER userSegSize
    #define FWDL_SEGMENT_SIZE_VARS\
    uint16_t FWDL_SEGMENT_SIZE_FLAG = 64;/*default value*/\
    bool FWDL_SEGMENT_SIZE_FROM_USER = false;
    #define FWDL_SEGMENT_SIZE_LONG_OPT_STRING "fwdlSegSize"
    #define FWDL_SEGMENT_SIZE_LONG_OPT { FWDL_SEGMENT_SIZE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //show Download support
    #define SHOW_FWDL_SUPPORT_INFO_FLAG showFWDLsupport
    #define SHOW_FWDL_SUPPORT_VAR getOptBool SHOW_FWDL_SUPPORT_INFO_FLAG = goFalse;
    #define SHOW_FWDL_SUPPORT_LONG_OPT_STRING "fwdlInfo"
    #define SHOW_FWDL_SUPPORT_LONG_OPT { SHOW_FWDL_SUPPORT_LONG_OPT_STRING, no_argument, &SHOW_FWDL_SUPPORT_INFO_FLAG, goTrue }

    //Enable auto detect for legacy ATA Passthrough (USB)
    #define ENABLE_LEGACY_PASSTHROUGH_FLAG enableLegacyATAPassthrough
    #define ENABLE_LEGACY_PASSTHROUGH_VAR getOptBool ENABLE_LEGACY_PASSTHROUGH_FLAG = goFalse;
    #define ENABLE_LEGACY_PASSTHROUGH_LONG_OPT_STRING "enableLegacyUSBPassthrough"
    #define ENABLE_LEGACY_PASSTHROUGH_LONG_OPT { ENABLE_LEGACY_PASSTHROUGH_LONG_OPT_STRING, no_argument, &ENABLE_LEGACY_PASSTHROUGH_FLAG, goTrue }

#if defined (ENABLE_CSMI)
    #define CSMI_INFO_FLAG csmiInfo
    #define CSMI_INFO_VAR getOptBool CSMI_INFO_FLAG = goFalse;
    #define CSMI_INFO_LONG_OPT_STRING "csmiInfo"
    #define CSMI_INFO_LONG_OPT { CSMI_INFO_LONG_OPT_STRING, no_argument, &CSMI_INFO_FLAG, goTrue }

    #define CSMI_VERBOSE_FLAG csmiVerbose
    #define CSMI_VERBOSE_VAR getOptBool CSMI_VERBOSE_FLAG = goFalse;
    #define CSMI_VERBOSE_LONG_OPT_STRING "csmiVerbose"
    #define CSMI_VERBOSE_LONG_OPT { CSMI_VERBOSE_LONG_OPT_STRING, no_argument, &CSMI_VERBOSE_FLAG, goTrue }

    #define CSMI_FORCE_USE_PORT_FLAG csmiUsePort
    #define CSMI_FORCE_IGNORE_PORT_FLAG csmiIgnorePort
    #define CSMI_FORCE_VARS \
    getOptBool CSMI_FORCE_USE_PORT_FLAG = goFalse;\
    getOptBool CSMI_FORCE_IGNORE_PORT_FLAG = goFalse;
    #define CSMI_FORCE_USE_PORT_LONG_OPT_STRING "csmiUsePort"
    #define CSMI_FORCE_USE_PORT_LONG_OPT { CSMI_FORCE_USE_PORT_LONG_OPT_STRING, no_argument, &CSMI_FORCE_USE_PORT_FLAG, goTrue }
    #define CSMI_FORCE_IGNORE_PORT_LONG_OPT_STRING "csmiIgnorePort"
    #define CSMI_FORCE_IGNORE_PORT_LONG_OPT { CSMI_FORCE_IGNORE_PORT_LONG_OPT_STRING, no_argument, &CSMI_FORCE_IGNORE_PORT_FLAG, goTrue }
    #define CSMI_FORCE_LONG_OPTS \
    CSMI_FORCE_USE_PORT_LONG_OPT, CSMI_FORCE_IGNORE_PORT_LONG_OPT
#endif

    //OD, MD, ID test
    #define OD_MD_ID_TEST_UNITS_SPECIFIED odmdidUnitsSpecified
    #define PERFORM_OD_TEST performODTest
    #define PERFORM_MD_TEST performMDTest
    #define PERFORM_ID_TEST performIDTest
    #define OD_ID_MD_TEST_RANGE odmdidRange
    #define OD_MD_ID_TEST_VARS \
    bool PERFORM_OD_TEST = false;\
    bool PERFORM_MD_TEST = false;\
    bool PERFORM_ID_TEST = false;\
    uint64_t OD_ID_MD_TEST_RANGE = 0;\
    bool OD_MD_ID_TEST_UNITS_SPECIFIED = false;
    #define OD_MD_ID_TEST_LONG_OPT_STRING "diameterTest"
    #define OD_MD_ID_TEST_RANGE_LONG_OPT_STRING "diameterTestRange"
    #define OD_MD_ID_TEST_LONG_OPT { OD_MD_ID_TEST_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define OD_MD_ID_TEST_RANGE_LONG_OPT { OD_MD_ID_TEST_RANGE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //hide LBA counter
    #define HIDE_LBA_COUNTER hideLBACounter
    #define HIDE_LBA_COUNTER_VAR getOptBool HIDE_LBA_COUNTER = goFalse;
    #define HIDE_LBA_COUNTER_LONG_OPT_STRING "hideLBACounter"
    #define HIDE_LBA_COUNTER_LONG_OPT { HIDE_LBA_COUNTER_LONG_OPT_STRING, no_argument, &HIDE_LBA_COUNTER, goTrue }

    //depopulate / remanufacture 
    #define SHOW_PHYSICAL_ELEMENT_STATUS_FLAG showPhysicalElementStatus
    #define SHOW_PHYSICAL_ELEMENT_STATUS_VAR getOptBool SHOW_PHYSICAL_ELEMENT_STATUS_FLAG = goFalse;
    #define SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT_STRING "showPhysicalElementStatus"
    #define SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT { SHOW_PHYSICAL_ELEMENT_STATUS_LONG_OPT_STRING, no_argument, &SHOW_PHYSICAL_ELEMENT_STATUS_FLAG, goTrue }

    #define REMOVE_PHYSICAL_ELEMENT_FLAG removePhysicalElement //zero means not set and is not a valid element. Must be non-zero
    #define REMOVE_PHYSICAL_ELEMENT_VAR uint32_t REMOVE_PHYSICAL_ELEMENT_FLAG = 0;
    #define REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING "removePhysicalElement"
    #define REMOVE_PHYSICAL_ELEMENT_LONG_OPT { REMOVE_PHYSICAL_ELEMENT_LONG_OPT_STRING, required_argument, NULL, 0 }

    //This is a force flag to use Seagate commands instead of the STD spec...really just for debugging.
    #define FORCE_SEAGATE_DEPOPULATE_COMMANDS forceSeagateDepopCommands
    #define FORCE_SEAGATE_DEPOPULATE_COMMANDS_VAR getOptBool FORCE_SEAGATE_DEPOPULATE_COMMANDS = goFalse;
    #define FORCE_SEAGATE_DEPOPULATE_COMMANDS_LONG_OPT_STRING "forceSeagateDepop"
    #define FORCE_SEAGATE_DEPOPULATE_COMMANDS_LONG_OPT { FORCE_SEAGATE_DEPOPULATE_COMMANDS_LONG_OPT_STRING, no_argument, &FORCE_SEAGATE_DEPOPULATE_COMMANDS, goTrue }

    //TCG Band/Range info
    #define SHOW_LOCKED_REGIONS showLockedRegions
    #define SHOW_LOCKED_REGIONS_VAR getOptBool SHOW_LOCKED_REGIONS = goFalse;
    #define SHOW_LOCKED_REGIONS_LONG_OPT_STRING "showLockedRegions"
    #define SHOW_LOCKED_REGIONS_LONG_OPT { SHOW_LOCKED_REGIONS_LONG_OPT_STRING, no_argument, &SHOW_LOCKED_REGIONS, goTrue }

    //Seagate Power Balance options (SATA only since SAS can use the setPowerConsumption options)
    #define SEAGATE_POWER_BALANCE_FLAG powerBalanceFeature
    #define SEAGATE_POWER_BALANCE_ENABLE_FLAG powerBalanceEnable
    #define SEAGATE_POWER_BALANCE_INFO_FLAG powerBalanceInfo
    #define SEAGATE_POWER_BALANCE_VARS \
        bool SEAGATE_POWER_BALANCE_FLAG = false;\
        bool SEAGATE_POWER_BALANCE_ENABLE_FLAG = false;\
        bool SEAGATE_POWER_BALANCE_INFO_FLAG = false;
    #define SEAGATE_POWER_BALANCE_LONG_OPT_STRING "powerBalanceFeature"
    #define SEAGATE_POWER_BALANCE_LONG_OPT { SEAGATE_POWER_BALANCE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //SATA DIPM feature (device initiated power management)
    #define SATA_DIPM_FLAG sataDIPMFeature
    #define SATA_DIPM_ENABLE_FLAG sataDIPMEnable
    #define SATA_DIPM_INFO_FLAG sataDIPMInfo
    #define SATA_DIPM_VARS \
        bool SATA_DIPM_FLAG = false;\
        bool SATA_DIPM_ENABLE_FLAG = false;\
        bool SATA_DIPM_INFO_FLAG = false;
    #define SATA_DIPM_LONG_OPT_STRING "sataDIPMfeature"
    #define SATA_DIPM_LONG_OPT { SATA_DIPM_LONG_OPT_STRING, required_argument, NULL, 0 }

    //SATA DAPS feature (device automatic partial to slumber transitions)
    #define SATA_DAPS_FLAG sataDAPSFeature
    #define SATA_DAPS_ENABLE_FLAG sataDAPSEnable
    #define SATA_DAPS_INFO_FLAG sataDAPSInfo
    #define SATA_DAPS_VARS \
            bool SATA_DAPS_FLAG = false;\
            bool SATA_DAPS_ENABLE_FLAG = false;\
            bool SATA_DAPS_INFO_FLAG = false;
    #define SATA_DAPS_LONG_OPT_STRING "sataDAPSfeature"
    #define SATA_DAPS_LONG_OPT { SATA_DAPS_LONG_OPT_STRING, required_argument, NULL, 0 }

    //Free fall control
    #define FREE_FALL_FLAG setFreeFall
    #define FREE_FALL_DISABLE disableFreeFall
    #define FREE_FALL_INFO freeFallInfo
    #define FREE_FALL_SENSITIVITY freeFallSensitivity
    #define FREE_FALL_VARS \
    bool FREE_FALL_FLAG = false;\
    bool FREE_FALL_INFO = false;\
    bool FREE_FALL_DISABLE = false;\
    uint8_t FREE_FALL_SENSITIVITY = 0;
    #define FREE_FALL_LONG_OPT_STRING "freeFall"
    #define FREE_FALL_LONG_OPT { FREE_FALL_LONG_OPT_STRING, required_argument, NULL, 0 }

    //SCSI defect list
    #define SCSI_DEFECTS_FLAG showSCSIDefects
    #define SCSI_DEFECTS_PRIMARY_LIST scsiPrimaryDefects
    #define SCSI_DEFECTS_GROWN_LIST scsiGrownDefects
    #define SCSI_DEFECTS_DESCRIPTOR_MODE scsiDefectsAddressType
    #define SCSI_DEFECTS_VARS \
    bool SCSI_DEFECTS_FLAG = false;\
    bool SCSI_DEFECTS_PRIMARY_LIST = false;\
    bool SCSI_DEFECTS_GROWN_LIST = false;\
    int SCSI_DEFECTS_DESCRIPTOR_MODE = 5;//physical CHS as default
    #define SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING "defectFormat"
    #define SCSI_DEFECTS_LONG_OPT_STRING "showSCSIDefects"
    #define SCSI_DEFECTS_LONG_OPT { SCSI_DEFECTS_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCSI_DEFECTS_MODE_LONG_OPTS { SCSI_DEFECTS_DESCRIPTOR_MODE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCSI_DEFECTS_LONG_OPTS SCSI_DEFECTS_LONG_OPT,SCSI_DEFECTS_MODE_LONG_OPTS

    //logTransferLength
    #define LOG_TRANSFER_LENGTH_BYTES logTransferLengthBytes
    #define LOG_TRANSFER_LENGTH_BYTES_VAR uint32_t logTransferLengthBytes = 0;/*0 means that the library will decide.*/
    #define LOG_TRANSFER_LENGTH_LONG_OPT_STRING "logTransferLength"
    #define LOG_TRANSFER_LENGTH_LONG_OPT { LOG_TRANSFER_LENGTH_LONG_OPT_STRING, required_argument, NULL, 0 }

    //FARM Log
    #define FARM_PULL_FLAG pullFarmLog
    #define FARM_VAR \
    getOptBool FARM_PULL_FLAG = goFalse;
    #define FARM_LONG_OPT_STRING "farm"
    #define FARM_LONG_OPT { FARM_LONG_OPT_STRING, no_argument, &FARM_PULL_FLAG, goTrue }

    //DST Log (standard spec)
    #define DST_LOG_FLAG pullDSTLog
    #define DST_LOG_VAR \
    getOptBool DST_LOG_FLAG = goFalse;
    #define DST_LOG_LONG_OPT_STRING "selfTestLog"
    #define DST_LOG_LONG_OPT { DST_LOG_LONG_OPT_STRING, no_argument, &DST_LOG_FLAG, goTrue }

    //Identify Device Data Log (standard spec)
    #define IDENTIFY_DEVICE_DATA_LOG_FLAG pullIDDataLog
    #define IDENTIFY_DEVICE_DATA_LOG_VAR getOptBool IDENTIFY_DEVICE_DATA_LOG_FLAG = goFalse;
    #define IDENTIFY_DEVICE_DATA_LOG_LONG_OPT_STRING "identifyDataLog"
    #define IDENTIFY_DEVICE_DATA_LOG_LONG_OPT { IDENTIFY_DEVICE_DATA_LOG_LONG_OPT_STRING, no_argument, &IDENTIFY_DEVICE_DATA_LOG_FLAG, goTrue }

    //SATA Phy Event Counters Log (standard spec)
    #define SATA_PHY_COUNTERS_LOG_FLAG sataPhyCountersLog
    #define SATA_PHY_COUNTERS_LOG_VAR getOptBool SATA_PHY_COUNTERS_LOG_FLAG = goFalse;
    #define SATA_PHY_COUNTERS_LOG_LONG_OPT_STRING "SATAPhyCntLog"
    #define SATA_PHY_COUNTERS_LONG_OPT { SATA_PHY_COUNTERS_LOG_LONG_OPT_STRING, no_argument, &SATA_PHY_COUNTERS_LOG_FLAG, goTrue }

    //Device Statistics Log (standard spec)
    #define DEVICE_STATS_LOG_FLAG deviceStatisticsLog
    #define DEVICE_STATS_LOG_VAR getOptBool DEVICE_STATS_LOG_FLAG = goFalse;
    #define DEVICE_STATS_LOG_LONG_OPT_STRING "deviceStatisticsLog"
    #define DEVICE_STATS_LOG_LONG_OPT { DEVICE_STATS_LOG_LONG_OPT_STRING, no_argument, &DEVICE_STATS_LOG_FLAG, goTrue }

    //Informational Exceptions Log (standard spec, but may include vendor unique parameters)
    #define INFORMATIONAL_EXCEPTIONS_FLAG informationExceptionsLog
    #define INFORMATIONAL_EXCEPTIONS_VAR getOptBool INFORMATIONAL_EXCEPTIONS_FLAG = goFalse;
    #define INFORMATIONAL_EXCEPTIONS_LONG_OPT_STRING "infoExceptionsLog"
    #define INFROMATIONAL_EXCEPTIONS_LONG_OPT { INFORMATIONAL_EXCEPTIONS_LONG_OPT_STRING, no_argument, &INFORMATIONAL_EXCEPTIONS_FLAG, goTrue }

    //Show SMART Error Log
    #define SHOW_SMART_ERROR_LOG_FLAG showSMARTErrorLog
    #define SHOW_SMART_ERROR_LOG_MODE smartErrorLogMode //summary vs (ext) comp. NOTE: can easily be changed to show comp vs ext comp as separate options
    #define SHOW_SMART_ERROR_LOG_VARS \
    bool SHOW_SMART_ERROR_LOG_FLAG  = false;\
    uint8_t SHOW_SMART_ERROR_LOG_MODE = 0; //0 = summary, 1 = (ext) comp. If a request to see the comp vs ext comp comes in, we can add 2 = ext comp.
    #define SHOW_SMART_ERROR_LOG_LONG_OPT_STRING "showSMARTErrorLog"
    #define SHOW_SMART_ERROR_LOG_LONG_OPT { SHOW_SMART_ERROR_LOG_LONG_OPT_STRING, required_argument, NULL, 0 }

    //SMART Error log format: detailed vs raw/generic
    #define SMART_ERROR_LOG_FORMAT_FLAG showSMARTErrorLogGenericFormat //default to false
    #define SMART_ERROR_LOG_FORMAT_VAR bool SMART_ERROR_LOG_FORMAT_FLAG = false;
    #define SMART_ERROR_LOG_FORMAT_LONG_OPT_STRING "smartErrorLogFormat"
    #define SMART_ERROR_LOG_FORMAT_LONG_OPT { SMART_ERROR_LOG_FORMAT_LONG_OPT_STRING, required_argument, NULL, 0 }

    //These defines make it easy to access the variable name
    #define ATA_SECURITY_USER_PROVIDED_PASS     atSecurityPasswordProvidedByUser
    #define ATA_SECURITY_PASSWORD               ataSecPassword
    #define ATA_SECURITY_PASSWORD_BYTE_COUNT    ataSecPasswordBytesProvided //count of how many bytes of password were given by the user so that when the modificaions are performed, they are done based on this value.
    #define ATA_SECURITY_USING_MASTER_PW        ataSecurityUsingMasterPW
    #define ATA_SECURITY_ERASE_ENHANCED         ataSecEnhancedErase
    #define ATA_SECURITY_MASTER_PW_CAPABILITY   ataMasterPWCap
    #define ATA_SECURITY_MASTER_PW_ID           masterPasswordIdentifier
    #define ATA_SECURITY_FORCE_SAT              forceATASecViaSAT
    #define ATA_SECURITY_FORCE_SAT_VALID        forceATASecSATValid

    typedef struct _ataSecPWModifications
    {
        bool byteSwapped;
        bool md5Hash;//Hash should ALWAYS be performed last of the mods so that everything can be combined nicely
        bool zeroPadded;//default padding
        bool spacePadded;//padded with spaces
        bool fpadded;//Pad with FFh
        bool leftAligned;
        bool rightAligned;
        bool forceUppercase;
        bool forceLowercase;
        bool invertCase;
        //TODO: add other modifications as we find or hear other that work with some odd BIOS chips.
    }ataSecPWModifications;
    #define ATA_SECURITY_PASSWORD_MODIFICATIONS passwordModificationType
    #define ATA_SECURITY_PASSWORD_MODIFICATIONS_VAR ataSecPWModifications ATA_SECURITY_PASSWORD_MODIFICATIONS = { false, false, false, false, false, false, false };
    #define ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING "ataSecPWMod"
    #define ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT { ATA_SECURITY_PASSWORD_MODIFICATIONS_LONG_OPT_STRING, required_argument, NULL, 0 }

    //These defines are to put the variable definitions in a file
    #define ATA_SECURITY_PASSWORD_VARS \
    bool ATA_SECURITY_USER_PROVIDED_PASS = false;\
    uint8_t ATA_SECURITY_PASSWORD[32] = { 0 }; \
    uint8_t ATA_SECURITY_PASSWORD_BYTE_COUNT = 0;
    #define ATA_SECURITY_PASSWORD_LONG_OPT_STRING "ataSecPassword" //agrs are: password in quotes, SeaChest, or the word empty
    #define ATA_SECURITY_PASSWORD_LONG_OPT { ATA_SECURITY_PASSWORD_LONG_OPT_STRING, required_argument, NULL, 0 }
                                                                                          //
    #define ATA_SECURITY_USING_MASTER_PW_VAR    bool ATA_SECURITY_USING_MASTER_PW = false;//false means user password. True means master password
    #define ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING "ataSecPassType"
    #define ATA_SECURITY_USING_MASTER_PW_LONG_OPT { ATA_SECURITY_USING_MASTER_PW_LONG_OPT_STRING, required_argument, NULL, 0 }

    //TODO: This needs to handle the request to do secure erase AND which TYPE of secure erase
    #define ATA_SECURITY_ERASE_OP performATASecurityErase
    #define ATA_SECURITY_ERASE_OP_VARS \
    bool ATA_SECURITY_ERASE_OP = false; \
    bool ATA_SECURITY_ERASE_ENHANCED = false;//false = normal erase, true - enhanced erase
    #define ATA_SECURITY_ERASE_OP_LONG_OPT_STRING "ataSecureErase"
    #define ATA_SECURITY_ERASE_OP_LONG_OPT { ATA_SECURITY_ERASE_OP_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define ATA_SECURITY_MASTER_PW_CAPABILITY_VAR   bool ATA_SECURITY_MASTER_PW_CAPABILITY = false;//false = high, true = maximum
    #define ATA_SECURITY_MASTER_PW_CAPABILITY_LONG_OPT_STRING "ataSecCapability"
    #define ATA_SECURITY_MASTER_PW_CAPABILITY_LONG_OPT { ATA_SECURITY_MASTER_PW_CAPABILITY_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define ATA_SECURITY_MASTER_PW_ID_VAR       uint16_t ATA_SECURITY_MASTER_PW_ID = 0;//value set by the user
    #define ATA_SECURITY_MASTER_PW_ID_LONG_OPT_STRING "ataSecMasterPWID"
    #define ATA_SECURITY_MASTER_PW_ID_LONG_OPT { ATA_SECURITY_MASTER_PW_ID_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define ATA_SECURITY_FORCE_SAT_LONG_OPT_STRING "ataSATsecurityProtocol"
    #define ATA_SECURITY_FORCE_SAT_LONG_OPT { ATA_SECURITY_FORCE_SAT_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define ATA_SECURITY_FORCE_SAT_VARS \
    bool ATA_SECURITY_FORCE_SAT_VALID = false;\
    bool ATA_SECURITY_FORCE_SAT = false;//false = turn OFF SAT security protocol commands. true = turn ON SAT security protocol commands

    //set password
    #define ATA_SECURITY_SET_PASSWORD_OP setATASecurityPassword
    #define ATA_SECURITY_SET_PASSWORD_OP_VAR getOptBool ATA_SECURITY_SET_PASSWORD_OP = goFalse;
    #define ATA_SECURITY_SET_PASSWORD_OP_LONG_OPT_STRING "setATASecPW"
    #define ATA_SECURITY_SET_PASSWORD_OP_LONG_OPT { ATA_SECURITY_SET_PASSWORD_OP_LONG_OPT_STRING, no_argument, &ATA_SECURITY_SET_PASSWORD_OP, goTrue }

    //unlock
    #define ATA_SECURITY_UNLOCK_OP unlockATASecurity
    #define ATA_SECURITY_UNLOCK_OP_VAR getOptBool ATA_SECURITY_UNLOCK_OP = goFalse;
    #define ATA_SECURITY_UNLOCK_OP_LONG_OPT_STRING "unlockATASec"
    #define ATA_SECURITY_UNLOCK_OP_LONG_OPT { ATA_SECURITY_UNLOCK_OP_LONG_OPT_STRING, no_argument, &ATA_SECURITY_UNLOCK_OP, goTrue }

    //disable the password
    #define ATA_SECURITY_DISABLE_OP disableATASecurityPW
    #define ATA_SECURITY_DISABLE_OP_VAR getOptBool ATA_SECURITY_DISABLE_OP = goFalse;
    #define ATA_SECURITY_DISABLE_OP_LONG_OPT_STRING "disableATASecPW"
    #define ATA_SECURITY_DISABLE_OP_LONG_OPT { ATA_SECURITY_DISABLE_OP_LONG_OPT_STRING, no_argument, &ATA_SECURITY_DISABLE_OP, goTrue }

    //freezelock
    #define ATA_SECURITY_FREEZELOCK_OP freezelockATASecurity
    #define ATA_SECURITY_FREEZELOCK_OP_VAR getOptBool ATA_SECURITY_FREEZELOCK_OP = goFalse;
    #define ATA_SECURITY_FREEZELOCK_OP_LONG_OPT_STRING "ataSecFreeze"
    #define ATA_SECURITY_FREEZELOCK_OP_LONG_OPT { ATA_SECURITY_FREEZELOCK_OP_LONG_OPT_STRING, no_argument, &ATA_SECURITY_FREEZELOCK_OP, goTrue }

    //ata security info
    #define ATA_SECURITY_INFO_OP ataSecurityInfo
    #define ATA_SECURITY_INFO_OP_VAR getOptBool ATA_SECURITY_INFO_OP = goFalse;
    #define ATA_SECURITY_INFO_OP_LONG_OPT_STRING "ataSecurityInfo"
    #define ATA_SECURITY_INFO_OP_LONG_OPT { ATA_SECURITY_INFO_OP_LONG_OPT_STRING, no_argument, &ATA_SECURITY_INFO_OP, goTrue }

    //scsi mode page reset/restore/save
    #define SCSI_MP_RESET_OP resetSCSIModePage
    #define SCSI_MP_RESET_PAGE_NUMBER resetModePageNumber
    #define SCSI_MP_RESET_SUBPAGE_NUMBER resetModeSubPageNumber
    #define SCSI_MP_RESET_VARS \
    bool SCSI_MP_RESET_OP = false;\
    uint8_t SCSI_MP_RESET_PAGE_NUMBER = 0;\
    uint8_t SCSI_MP_RESET_SUBPAGE_NUMBER = 0;
    #define SCSI_MP_RESET_LONG_OPT_STRING "scsiMPReset"
    #define SCSI_MP_RESET_LONG_OPT { SCSI_MP_RESET_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define SCSI_MP_RESTORE_OP restoreSCSIModePage
    #define SCSI_MP_RESTORE_PAGE_NUMBER restoreModePageNumber
    #define SCSI_MP_RESTORE_SUBPAGE_NUMBER restoreModeSubPageNumber
    #define SCSI_MP_RESTORE_VARS \
    bool SCSI_MP_RESTORE_OP = false;\
    uint8_t SCSI_MP_RESTORE_PAGE_NUMBER = 0;\
    uint8_t SCSI_MP_RESTORE_SUBPAGE_NUMBER = 0;
    #define SCSI_MP_RESTORE_LONG_OPT_STRING "scsiMPRestore"
    #define SCSI_MP_RESTORE_LONG_OPT { SCSI_MP_RESTORE_LONG_OPT_STRING, required_argument, NULL, 0 }

    #define SCSI_MP_SAVE_OP saveSCSIModePage
    #define SCSI_MP_SAVE_PAGE_NUMBER saveModePageNumber
    #define SCSI_MP_SAVE_SUBPAGE_NUMBER saveModeSubPageNumber
    #define SCSI_MP_SAVE_VARS \
    bool SCSI_MP_SAVE_OP = false;\
    uint8_t SCSI_MP_SAVE_PAGE_NUMBER = 0;\
    uint8_t SCSI_MP_SAVE_SUBPAGE_NUMBER = 0;
    #define SCSI_MP_SAVE_LONG_OPT_STRING "scsiMPSave"
    #define SCSI_MP_SAVE_LONG_OPT { SCSI_MP_SAVE_LONG_OPT_STRING, required_argument, NULL, 0 }

    //show scsi mode page (TODO: different output modes) output modes: classic vs "neat" or some other name...
    #define SCSI_SHOW_MP_OP showSCSIModePage
    #define SCSI_SHOW_MP_PAGE_NUMBER showModePageNumber
    #define SCSI_SHOW_MP_SUBPAGE_NUMBER showModeSubPageNumber
    #define SCSI_SHOW_MP_MPC_VALUE showSCSIMPmpc
    #define SCSI_SHOW_MP_BUFFER_MODE showSCSIMPAsBuffer //default to false for classic
    #define SCSI_SHOW_MP_VARS \
    bool SCSI_SHOW_MP_OP = false;\
    uint8_t SCSI_SHOW_MP_PAGE_NUMBER = 0;\
    uint8_t SCSI_SHOW_MP_SUBPAGE_NUMBER = 0;\
    bool SCSI_SHOW_MP_BUFFER_MODE = false;\
    int SCSI_SHOW_MP_MPC_VALUE = 0;//leave at zero to default to current values
    #define SCSI_SHOW_MP_MPC_LONG_OPT_STRING "showSCSIMPControl"
    #define SCSI_SHOW_MP_LONG_OPT_STRING "showSCSIMP"
    #define SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT_STRING "showMPOutputMode"
    #define SCSI_SHOW_MP_LONG_OPT { SCSI_SHOW_MP_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCSI_SHOW_MP_MPC_LONG_OPT { SCSI_SHOW_MP_MPC_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT { SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCSI_SHOW_MP_LONG_OPTS SCSI_SHOW_MP_LONG_OPT,SCSI_SHOW_MP_MPC_LONG_OPT,SCSI_SHOW_MP_BUFFER_MODE_LONG_OPT

    //setting a SCSI mode page
    #define SCSI_SET_MP_OP setSCSIModePage 
    #define SCSI_SET_MP_FILENAME setSCSIModePageFilename
    #define SCSI_SET_MP_PAGE_NUMBER setSCSIModePageNumber
    #define SCSI_SET_MP_SUBPAGE_NUMBER setSCSIModeSubpageNumber
    #define SCSI_SET_MP_BYTE setSCSIMPByte
    #define SCSI_SET_MP_BIT setSCSIMPBit
    #define SCSI_SET_MP_FIELD_LEN_BITS setSCSIMPFieldLen
    #define SCSI_SET_MP_FIELD_VALUE setSCSIMPFieldVal
    #define SCSI_SET_MP_VARS \
    bool SCSI_SET_MP_OP = false;\
    char SCSI_SET_MP_FILENAME[OPENSEA_PATH_MAX * 2] = { 0 };\
    uint8_t SCSI_SET_MP_PAGE_NUMBER = 0;\
    uint8_t SCSI_SET_MP_SUBPAGE_NUMBER = 0;\
    uint16_t SCSI_SET_MP_BYTE = 0;\
    uint8_t SCSI_SET_MP_BIT = 0;\
    uint8_t SCSI_SET_MP_FIELD_LEN_BITS = 0;\
    uint64_t SCSI_SET_MP_FIELD_VALUE = 0;
    #define SCSI_SET_MP_LONG_OPT_STRING "setSCSIMP" //mp[-sp]:byte:highestBit:fieldWidthInBits=value OR file=filename.txt
    #define SCSI_SET_MP_LONG_OPT { SCSI_SET_MP_LONG_OPT_STRING, required_argument, NULL, 0 }


    //reset a SCSI Log page
    #define SCSI_RESET_LP_OP resetSCSILogPage
    #define SCSI_RESET_LP_LPC resetSCSILogPageControl
    #define SCSI_RESET_LP_PAGE_NUMBER resetSCSILogPageNumber
    #define SCSI_RESET_LP_SUBPAGE_NUMBER resetSCSILogSubPageNumber
    #define SCSI_RESET_LP_VARS \
    bool SCSI_RESET_LP_OP = false;\
    uint8_t SCSI_RESET_LP_PAGE_NUMBER = 0;\
    uint8_t SCSI_RESET_LP_SUBPAGE_NUMBER = 0;\
    int SCSI_RESET_LP_LPC = 1; /*default to the thresholds*/
    #define SCSI_RESET_LP_LONG_OPT_STRING "scsiLPReset"
    #define SCSI_RESET_LP_PAGE_LONG_OPT_STRING "scsiLPResetPage"
    #define SCSI_RESET_LP_LONG_OPT { SCSI_RESET_LP_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCSI_RESET_LP_PAGE_LONG_OPT {SCSI_RESET_LP_PAGE_LONG_OPT_STRING, required_argument, NULL, 0 }
    #define SCSI_RESET_LP_LONG_OPTS SCSI_RESET_LP_LONG_OPT,SCSI_RESET_LP_PAGE_LONG_OPT
    
    #define LONG_OPT_TERMINATOR { NULL, 0, NULL, 0 }

    extern const char *deviceHandleExample;
    extern const char *deviceHandleName;
    extern const char *commandWindowType;

	char* get_current_year(char *temp_year);

    void openseachest_utility_Info(const char *utilityName, const char *buildVersion, char *seaCPublicVersion);

    void utility_Full_Version_Info(const char *utilityName, const char *buildVersion, int seaCPublicMajorVersion, int seaCPublicMinorVersion, int seaCPublicPatchVersion, const char * openseaCommonVersion, const char * openseaOperationVersion);

    //-----------------------------------------------------------------------------
    //
    //  print_Bug_Report_Email()
    //
    //! \brief   Description:  Prints some information on where to send bug reports to for a utilitie's help information.
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Bug_Report_Email(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Final_newline()
    //
    //! \brief   Description:  This function prints out a single newline character. This is meant to be used with atexit() for a newline space before returning to the command prompt.
    //
    //  Entry:
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Final_newline(void);

    //-----------------------------------------------------------------------------
    //
    //  print_Scan_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the scan option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] deviceHandleExample = a string that is an example of the device handle (used so that help printed out matches what is expected for a particular OS
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Scan_Help(bool shortHelp, const char *deviceHandleExample);

	void print_Agressive_Scan_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Scan_Flags_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the scan flags option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Scan_Flags_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Device_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the device option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] deviceHandleExample = a string that is an example of the device handle (used so that help printed out matches what is expected for a particular OS
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Device_Help(bool shortHelp, const char *deviceHandleExample);

    //-----------------------------------------------------------------------------
    //
    //  print_Device_Information_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the device information option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Device_Information_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Verbose_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the verbose option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Verbose_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Quiet_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the quiet option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] utilName = a string that is the name of the utility
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Quiet_Help(bool shortHelp, const char *utilName);

    //-----------------------------------------------------------------------------
    //
    //  print_Version_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the version option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] utilName = a string that is the name of the utility
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Version_Help(bool shortHelp, const char *utilName);

    //-----------------------------------------------------------------------------
    //
    //  print_Confirm_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the confirm option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //--------------------------------------------]---------------------------------
    void print_Confirm_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_License_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the license option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_License_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Echo_Command_Line_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the echo command line option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Echo_Command_Line_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Help_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the help option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Help_Help(bool shortHelp);

	//-----------------------------------------------------------------------------
	//
	//  print_OutputPath_Help()
	//
	//! \brief   Description:  This function prints out the short or long help for the
	//!						   output folder help.
	//
	//  Entry:
	//!   \param[in] shortHelp = bool used to select when to print short or long help
	//!
	//  Exit:
	//!   \return VOID
	//
	//-----------------------------------------------------------------------------
	void print_OutputPath_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Poll_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the poll option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Poll_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_ATA_Security_Erase_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the ata security erase option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] password = a string that is the password used by the utility for the erase. Typically this is the name of the utility
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_ATA_Security_Erase_Help(bool shortHelp, const char *password);

    //-----------------------------------------------------------------------------
    //
    //  print_Erase_Range_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the erase range option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Erase_Range_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Erase_Time_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the erase time option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Erase_Time_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Disable_ATA_Security_Password_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the disable ATA security password option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] utilName = a string that is the name of the utility
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Disable_ATA_Security_Password_Help(bool shortHelp, const char *utilName);

    //-----------------------------------------------------------------------------
    //
    //  print_Sanitize_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the sanitize option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] utilName = a string that is the name of the utility
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Sanitize_Help(bool shortHelp, const char *utilName);

    //-----------------------------------------------------------------------------
    //
    //  print_Writesame_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the writesame option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Writesame_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Writesame_Range_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the writesame range option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Writesame_Range_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Revert_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the revert option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Revert_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_RevertSP_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the revertSP option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_RevertSP_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Progress_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the progress option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] testsTogetProgressFor = this is a string formatted as: "[test1 | test2 | ...]" that is a list of the operations that the utility supports getting progress for
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Progress_Help(bool shortHelp, char* testsTogetProgressFor);

    #define TOOL_EXIT_CODE_STRING_MAX_LENGTH 60 //60 characters SHOULD be plenty, but increase this if we do need more characters for the string. Must be NULL terminated.
    typedef struct _toolSpecificExitCode
    {
        int exitCode;
        char exitCodeString[TOOL_EXIT_CODE_STRING_MAX_LENGTH];
    }toolSpecificxitCode, *ptrToolSpecificxitCode;

    //-----------------------------------------------------------------------------
    //
    //  print_SeaChest_Util_Exit_Codes()
    //
    //! \brief   Description:  This function prints out the information about the utility's exit codes (common exit codes only)
    //
    //  Entry:
    //!   \param numberOfToolSpecificExitCodes - number of exit codes in the list being given
    //!   \param toolSpecificExitCodeList - pointer to the list of exit codes that are unique to the tool calling this function. Can be NULL if there are not specific exit codes to print
    //!   \param toolName - name of the tool the exit codes are being printed for. Must be non-null.
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_SeaChest_Util_Exit_Codes(int numberOfToolSpecificExitCodes, ptrToolSpecificxitCode toolSpecificExitCodeList, const char * toolName);

    //-----------------------------------------------------------------------------
    //
    //  print_SMART_Check_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the SMART Check option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_SMART_Check_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Short_DST_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the short DST option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Short_DST_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Long_DST_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the long DST option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!   \param[in] commandWindowType = string that represents the name of a command window type for the current OS
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Long_DST_Help(bool shortHelp, const char *commandWindowType);

    void print_Captive_Foreground_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Long_DST_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the long DST option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_SMART_Attributes_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Abort_DST_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the abort DST option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Abort_DST_Help(bool shortHelp);
    void print_Abort_IDD_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_IDD_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the IDD option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_IDD_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Check_Power_Mode_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the check power mode option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Check_Power_Mode_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Spindown_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the spindown option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Spindown_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_EnableDisableEPC_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the Enable Disable EPC option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_EnableDisableEPC_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Enable_Power_Mode_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the enableMode (power) option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Enable_Power_Mode_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Disable_Power_Mode_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the disableMode (power) option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Disable_Power_Mode_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Default_Power_Mode_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the defaultMode (power) option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Default_Power_Mode_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Enable_Power_Mode_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the enableMode (power) option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Power_Mode_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Timer_Mode_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the modeTimer (power) option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Timer_Mode_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Change_Power_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the changePower option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Change_Power_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Transition_Power_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the transitionPower option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Transition_Power_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Transition_Power_State_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the 
    //!                        transitionPower option for NVMe devices. 
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Transition_Power_State_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Short_Generic_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the short generic read test option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Short_Generic_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_two_Minute_Test_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the two minute generic read test option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_two_Minute_Test_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Long_Generic_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the long generic read test option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Long_Generic_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_User_Generic_Start_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the user generic read test option (starting LBA)
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_User_Generic_Start_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_User_Generic_Range_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the user generic read test option (range)
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_User_Generic_Range_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Error_Limit_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the error limit option for read tests
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Error_Limit_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Stop_On_Error_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the stop on error option for read tests
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Stop_On_Error_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Repair_At_End_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the repair at end option for read tests
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Repair_At_End_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Repair_On_Fly_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the repair on the fly option for read tests
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Repair_On_Fly_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Time_Hours_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the hours option option for timed operations
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Time_Hours_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Time_Minutes_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the minutes option option for timed operations
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Time_Minutes_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Time_Seconds_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the seconds option option for timed operations
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Time_Seconds_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Random_Read_Test_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the random read test option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Random_Read_Test_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Butterfly_Read_Test_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the butterfly read test option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Butterfly_Read_Test_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Overwrite_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the overwrite option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Overwrite_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Overwrite_Range_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the overwrite range option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Overwrite_Range_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Trim_Unmap_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the trim or unmap option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Trim_Unmap_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Trim_Unmap_Range_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the trim or unmap range option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Trim_Unmap_Range_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Show_Power_Consumption_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the show power consumption option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Show_Power_Consumption_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Set_Power_Consumption_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the set power consumption option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Set_Power_Consumption_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Test_Unit_Ready_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the test unit ready option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Test_Unit_Ready_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_SAT_12_Byte_CDB_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the SAT 12byte CDBs option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_SAT_12_Byte_CDB_Help(bool shortHelp);

    void print_Firmware_Download_Help(bool shortHelp);

    void print_Firmware_Slot_Buffer_ID_Help(bool shortHelp);

    void print_Firmware_Download_Mode_Help(bool shortHelp);

    void print_NVMe_Get_Log_Help(bool shortHelp);

    void print_NVMe_Get_Tele_Help(bool shortHelp);

	void print_extSmatLog_Help (bool shortHelp);

    void print_pcierr_Help(bool shortHelp);

    void print_NVMe_Temp_Stats_Help(bool shortHelp);

    void print_NVMe_Pci_Stats_Help(bool shortHelp);

    void print_Set_Max_LBA_Help(bool shortHelp);

    void print_Restore_Max_LBA_Help(bool shortHelp);

    void printf_Set_Phy_Speed_Help(bool shortHelp);

    void print_Set_Ready_LED_Help(bool shortHelp);

    void print_Read_Look_Ahead_Help(bool shortHelp);

    void print_Write_Cache_Help(bool shortHelp);

    void print_SCT_Write_Cache_Help(bool shortHelp);

    void print_SCT_Write_Cache_Reordering_Help(bool shortHelp);

    void print_Provision_Help(bool shortHelp);

    void print_Phy_Speed_Help(bool shortHelp);

    void print_SAT_Info_Help(bool shortHelp);

	void print_Supported_Logs_Help(bool shortHelp);

	void print_Pull_Generic_Logs_Help(bool shortHelp);

    void print_Pull_Generic_Logs_Subpage_Help(bool shortHelp);

    void print_Supported_Error_History_Help(bool shortHelp);

    void print_Pull_Generic_Error_History_Help(bool shortHelp);
	
	void print_Log_Mode_Help(bool shortHelp);

    void print_DST_And_Clean_Help(bool shortHelp);

    void print_Generic_Test_Mode_Help(bool shortHelp);

    void print_Show_Supported_Erase_Modes_Help(bool shortHelp);

    void print_Perform_Quickest_Erase_Help(bool shortHelp);

    void print_Format_Unit_Help(bool shortHelp);

    void print_Format_Disable_Primary_List_Help(bool shortHelp);

    void print_Format_Discard_Grown_Defect_List_Help(bool shortHelp);

    void print_Format_Disable_Certification_Help(bool shortHelp);

    void print_Format_Security_Initialize_Help(bool shortHelp);

    void print_Format_Protection_Type_Help(bool shortHelp);

    void print_Format_Protection_Interval_Exponent_Help(bool shortHelp);

    void print_Format_Default_Format_Help(bool shortHelp);

    void print_Format_Disable_Immediate_Response_Help(bool shortHelp);

    void print_Format_Stop_On_List_Error_Help(bool shortHelp);

	void print_Format_New_Max_LBA_Help(bool shortHelp);

    void print_Show_Format_Status_Log_Help(bool shortHelp);

    void print_Set_Sector_Size_Help(bool shortHelp);

    void print_Show_Supported_Formats_Help(bool shortHelp);

    void print_Fast_Format_Help(bool shortHelp);

    void print_TCG_Info_Help(bool shortHelp);

    void print_Set_FWDL_Port_Help(bool shortHelp);

    void print_TCG_SID_Help(bool shortHelp);

    void print_TCG_PSID_Help(bool shortHelp);

    void print_Get_Features_Help(bool shortHelp);

    void print_Output_Mode_Help(bool shortHelp);

    void print_Low_Current_Spinup_Help(bool shortHelp);

    void print_Disable_Data_Locking_Help(bool shortHelp);

    void print_Model_Match_Help(bool shortHelp);

    void print_Firmware_Revision_Match_Help(bool shortHelp);

    void print_New_Firmware_Revision_Match_Help(bool shortHelp);

    void print_Only_Seagate_Help(bool shortHelp);
    
    void print_Set_APM_Level_Help(bool shortHelp);
    
    void print_Show_APM_Level_Help(bool shortHelp);

    void print_Disable_APM_Help(bool shortHelp);
    
    void print_PUIS_Feature_Help(bool shortHelp);

    void print_Show_EPC_Settings_Help(bool shortHelp);
    
    void print_SMART_Feature_Help(bool shortHelp);
    
    void print_Set_MRIE_Help(bool shortHelp);
    
    void print_SMART_Attribute_Autosave_Help(bool shortHelp);

    void print_SMART_Info_Help(bool shortHelp);

    void print_SMART_Auto_Offline_Help(bool shortHelp);

    void print_Show_DST_Log_Help(bool shortHelp);
    
    void print_Conveyance_DST_Help(bool shortHelp);
    
    void print_Force_SCSI_Help(bool shortHelp);
    
    void print_Force_ATA_Help(bool shortHelp);

	void print_Force_ATA_PIO_Help(bool shortHelp);

    void print_Force_ATA_DMA_Help(bool shortHelp);

    void print_Force_ATA_UDMA_Help(bool shortHelp);
	
    void print_Display_LBA_Help(bool shortHelp);

    void print_Pattern_Help(bool shortHelp);

    void print_Device_Statistics_Help(bool shortHelp);

    void print_Reset_Write_Pointer_Zone_Help(bool shortHelp);

    void print_Open_Zone_Help(bool shortHelp);

    void print_Finish_Zone_Help(bool shortHelp);

    void print_Close_Zone_Help(bool shortHelp);

    void print_Report_Zones_Help(bool shortHelp);

    void print_Max_Zones_Help(bool shortHelp);

    void print_Zone_ID_Help(bool shortHelp);

    void print_FWDL_Segment_Size_Help(bool shortHelp);

    void print_show_FWDL_Support_Help(bool shortHelp);

    void print_Firmware_Activate_Help(bool shortHelp);

    void print_Firmware_Switch_Help(bool shortHelp);

    void print_Enable_Legacy_USB_Passthrough_Help(bool shortHelp);

    void print_Set_SSC_Help(bool shortHelp);

    void print_Error_In_Cmd_Line_Args(const char * optstring, const char * arg);

    void print_Buffer_Test_Help(bool shortHelp);

    //Returns 255 when a memory allocation/reallocation error occurs (prints to stderr), 254 when a parameter is missing, zero otherwise.
    int parse_Device_Handle_Argument(char * optarg, bool *allDrives, bool *userHandleProvided, uint32_t *deviceCount, char ***handleList);

    //this call is to free the entire list of handles since they are all dynamically allocated.
    void free_Handle_List(char ***handleList, uint32_t listCount);

#if defined (ENABLE_CSMI)
    void print_CSMI_Force_Flags_Help(bool shortHelp);

    void print_CSMI_Verbose_Help(bool shortHelp);

    void print_CSMI_Info_Help(bool shortHelp);
#endif

    void print_OD_MD_ID_Test_Help(bool shortHelp);

    void print_OD_MD_ID_Test_Range_Help(bool shortHelp);

    void print_SAS_Phy_Help(bool shortHelp);

    void print_SAS_Phy_Start_Test_Pattern(bool shortHelp);

    void print_SAS_Phy_Stop_Test_Pattern(bool shortHelp);

    void print_SAS_Phy_Test_Pattern(bool shortHelp);

    void print_SAS_Phy_Test_SSC_Function(bool shortHelp);

    void print_SAS_Phy_Test_Link_Rate(bool shortHelp);

    void print_SAS_Phy_Test_DWord_Control(bool shortHelp);

    void print_SAS_Phy_Test_DWord_Pattern(bool shortHelp);

    void print_Hide_LBA_Counter_Help(bool shortHelp);

    void print_Show_Physical_Element_Status_Help(bool shortHelp);

    void print_Remove_Physical_Element_Status_Help(bool shortHelp);

    void print_Force_Seagate_Depop_Help(bool shortHelp);

    void print_Show_Locked_Regions_Help(bool shortHelp);

    void print_SCT_Error_Recovery_Read_Help(bool shortHelp);
    
    void print_SCT_Error_Recovery_Write_Help(bool shortHelp);

    void print_Seagate_Power_Balance_Help(bool shortHelp);

    void print_DIPM_Help(bool shortHelp);

    void print_DAPS_Help(bool shortHelp);

    void print_Active_Help(bool shortHelp);

    void print_Sleep_Help(bool shortHelp);

    void print_Idle_Unload_Help(bool shortHelp);

    void print_Idle_Help(bool shortHelp);

    void print_Standby_Help(bool shortHelp);

    void print_Free_Fall_Help(bool shortHelp);

    void print_SCSI_Defects_Help(bool shortHelp);

    void print_SCSI_Defects_Format_Help(bool shortHelp);

    void print_Log_Transfer_Length_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Pull_Device_Statistics_Log_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the pull device statistics log help
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Pull_Device_Statistics_Log_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Pull_Informational_Exceptions_Log_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the pull Informational Exceptions log help
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Pull_Informational_Exceptions_Log_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Pull_Self_Test_Results_Log_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the pull self test results log option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Pull_Self_Test_Results_Log_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Pull_Identify_Device_Data_Log_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the pull identify device data log option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Pull_Identify_Device_Data_Log_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_Pull_SATA_Phy_Event_Counters_Log_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the pull SATA Phy Event Counters log option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Pull_SATA_Phy_Event_Counters_Log_Help(bool shortHelp);

    //-----------------------------------------------------------------------------
    //
    //  print_FARM_Log_Help()
    //
    //! \brief   Description:  This function prints out the short or long help for the farm option
    //
    //  Entry:
    //!   \param[in] shortHelp = bool used to select when to print short or long help
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_FARM_Log_Help(bool shortHelp);

    void print_Show_SMART_Error_Log_Help(bool shortHelp);

    void print_SMART_Error_Log_Format_Help(bool shortHelp);

    void print_FWDL_Allow_Flexible_Win10_API_Use_Help(bool shortHelp);

    void print_FWDL_Force_Win_Passthrough_Help(bool shortHelp);

    void print_ATA_Security_Password_Modifications_Help(bool shortHelp);

    void print_ATA_Security_Password_Help(bool shortHelp);

    void print_ATA_Security_Password_Type_Help(bool shortHelp);

    void print_ATA_Security_Master_Password_Capability_Help(bool shortHelp);

    void print_ATA_Security_Master_Password_ID_Help(bool shortHelp);

    void print_ATA_Security_Force_SAT_Security_Protocol_Help(bool shortHelp);

    void print_ATA_Security_Set_Password_Help(bool shortHelp);

    void print_ATA_Security_Unlock_Help(bool shortHelp);

    void print_ATA_Security_Freezelock_Help(bool shortHelp);

    void print_ATA_Security_Info_Help(bool shortHelp);

    void print_SCSI_MP_Reset_Help(bool shortHelp);

    void print_SCSI_MP_Restore_Help(bool shortHelp);

    void print_SCSI_MP_Save_Help(bool shortHelp);

    void print_SCSI_Show_MP_Help(bool shortHelp);

    void print_SCSI_Show_MP_Control_Help(bool shortHelp);

    void print_SCSI_Reset_LP_Help(bool shortHelp);

    void print_SCSI_Reset_LP_Page_Help(bool shortHelp);

    void print_Set_SCSI_MP_Help(bool shortHelp);

    void print_Show_SCSI_MP_Output_Mode_Help(bool shortHelp);

    void print_NVM_Format_Help(bool shortHelp);

    void print_NVM_Format_NSID_Help(bool shortHelp);

    void print_NVM_Format_Secure_Erase_Help(bool shortHelp);

    void print_NVM_Format_PI_Type_Help(bool shortHelp);

    void print_NVM_Format_PIL_Help(bool shortHelp);

    void print_NVM_Format_Metadata_Size_Help(bool shortHelp);

    void print_NVM_Format_Metadata_Setting_Help(bool shortHelp);

#define OUTPUTPATH_PARSE outputPathPtr = optarg; 

#if defined (ENABLE_CSMI) //Since we are using macros to instert the following code, we get warnings in Linux...so I'm ifdefing it but there is no good way to do this so it's a lot of almost duplicated code - TJE
    #define SCAN_FLAGS_UTIL_VARS \
        bool                scanSD = false;\
        bool                scanSDandSG = false;\
        bool                scanATA = false;\
        bool                scanUSB = false;\
        bool                scanSCSI = false;\
        bool                scanNVMe = false;\
        bool                scanRAID = false;\
        bool                scanInterfaceATA = false;\
        bool                scanInterfaceUSB = false;\
        bool                scanInterfaceSCSI = false;\
        bool                scanInterfaceNVMe = false;\
        bool                scanIgnoreCSMI = false;\
        bool                scanAllowDuplicateDevices = false;

    #define SCAN_FLAGS_SUBOPT_PARSING                                                                               \
    if (optarg != NULL)                                                                                             \
    {                                                                                                               \
        int  index = optind - 1;                                                                                    \
        char *nextSubOpt = NULL;                                                                                    \
        while (index < argc)                                                                                        \
        {                                                                                                           \
            nextSubOpt = strdup(argv[index]);                                                                       \
            if (strncmp("-", nextSubOpt, 1) != 0)                                                                   \
            {                                                                                                       \
                if (strlen(nextSubOpt) == 3 && strncmp("ata", nextSubOpt, strlen(nextSubOpt)) == 0)                 \
                {                                                                                                   \
                    scanATA = true;                                                                                 \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 3 && strncmp("usb", nextSubOpt, strlen(nextSubOpt)) == 0)            \
                {                                                                                                   \
                    scanUSB = true;                                                                                 \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 4 && strncmp("scsi", nextSubOpt, strlen(nextSubOpt)) == 0)           \
                {                                                                                                   \
                    scanSCSI = true;                                                                                \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 4 && strncmp("nvme", nextSubOpt, strlen(nextSubOpt)) == 0)           \
                {                                                                                                   \
                    scanNVMe = true;                                                                                \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 4 && strncmp("raid", nextSubOpt, strlen(nextSubOpt)) == 0)           \
                {                                                                                                   \
                    scanRAID = true;                                                                                \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 12 && strncmp("interfaceATA", nextSubOpt, strlen(nextSubOpt)) == 0)  \
                {                                                                                                   \
                    scanInterfaceATA = true;                                                                        \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 12 && strncmp("interfaceUSB", nextSubOpt, strlen(nextSubOpt)) == 0)  \
                {                                                                                                   \
                    scanInterfaceUSB = true;                                                                        \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 13 && strncmp("interfaceSCSI", nextSubOpt, strlen(nextSubOpt)) == 0) \
                {                                                                                                   \
                    scanInterfaceSCSI = true;                                                                       \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 13 && strncmp("interfaceNVME", nextSubOpt, strlen(nextSubOpt)) == 0) \
                {                                                                                                   \
                    scanInterfaceNVMe = true;                                                                       \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 2 && strncmp("sd", nextSubOpt, strlen(nextSubOpt)) == 0)             \
                {                                                                                                   \
                    scanSD = true;                                                                                  \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 6 && strncmp("sgtosd", nextSubOpt, strlen(nextSubOpt)) == 0)         \
                {                                                                                                   \
                    scanSDandSG = true;                                                                             \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 10 && strncmp("ignoreCSMI", nextSubOpt, strlen(nextSubOpt)) == 0)    \
                {                                                                                                   \
                    scanIgnoreCSMI = true;                                                                          \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 15 && strncmp("allowDuplicates", nextSubOpt, strlen(nextSubOpt)) == 0) \
                {                                                                                                   \
                    scanAllowDuplicateDevices = true;                                                               \
                }                                                                                                   \
            }                                                                                                       \
            else                                                                                                    \
            {                                                                                                       \
                break;                                                                                              \
            }                                                                                                       \
            index++;                                                                                                \
        }                                                                                                           \
        optind = index;                                                                                             \
    }
#else
    #define SCAN_FLAGS_UTIL_VARS \
    bool                scanSD = false;\
    bool                scanSDandSG = false;\
    bool                scanATA = false;\
    bool                scanUSB = false;\
    bool                scanSCSI = false;\
    bool                scanNVMe = false;\
    bool                scanRAID = false;\
    bool                scanInterfaceATA = false;\
    bool                scanInterfaceUSB = false;\
    bool                scanInterfaceSCSI = false;\
    bool                scanInterfaceNVMe = false;

    #define SCAN_FLAGS_SUBOPT_PARSING                                                                               \
    if (optarg != NULL)                                                                                             \
    {                                                                                                               \
        int  index = optind - 1;                                                                                    \
        char *nextSubOpt = NULL;                                                                                    \
        while (index < argc)                                                                                        \
        {                                                                                                           \
            nextSubOpt = strdup(argv[index]);                                                                       \
            if (strncmp("-", nextSubOpt, 1) != 0)                                                                   \
            {                                                                                                       \
                if (strlen(nextSubOpt) == 3 && strncmp("ata", nextSubOpt, strlen(nextSubOpt)) == 0)                 \
                {                                                                                                   \
                    scanATA = true;                                                                                 \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 3 && strncmp("usb", nextSubOpt, strlen(nextSubOpt)) == 0)            \
                {                                                                                                   \
                    scanUSB = true;                                                                                 \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 4 && strncmp("scsi", nextSubOpt, strlen(nextSubOpt)) == 0)           \
                {                                                                                                   \
                    scanSCSI = true;                                                                                \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 4 && strncmp("nvme", nextSubOpt, strlen(nextSubOpt)) == 0)           \
                {                                                                                                   \
                    scanNVMe = true;                                                                                \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 4 && strncmp("raid", nextSubOpt, strlen(nextSubOpt)) == 0)           \
                {                                                                                                   \
                    scanRAID = true;                                                                                \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 12 && strncmp("interfaceATA", nextSubOpt, strlen(nextSubOpt)) == 0)  \
                {                                                                                                   \
                    scanInterfaceATA = true;                                                                        \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 12 && strncmp("interfaceUSB", nextSubOpt, strlen(nextSubOpt)) == 0)  \
                {                                                                                                   \
                    scanInterfaceUSB = true;                                                                        \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 13 && strncmp("interfaceSCSI", nextSubOpt, strlen(nextSubOpt)) == 0) \
                {                                                                                                   \
                    scanInterfaceSCSI = true;                                                                       \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 13 && strncmp("interfaceNVME", nextSubOpt, strlen(nextSubOpt)) == 0) \
                {                                                                                                   \
                    scanInterfaceNVMe = true;                                                                       \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 2 && strncmp("sd", nextSubOpt, strlen(nextSubOpt)) == 0)             \
                {                                                                                                   \
                    scanSD = true;                                                                                  \
                }                                                                                                   \
                else if (strlen(nextSubOpt) == 6 && strncmp("sgtosd", nextSubOpt, strlen(nextSubOpt)) == 0)         \
                {                                                                                                   \
                    scanSDandSG = true;                                                                             \
                }                                                                                                   \
            }                                                                                                       \
            else                                                                                                    \
            {                                                                                                       \
                break;                                                                                              \
            }                                                                                                       \
            index++;                                                                                                \
        }                                                                                                           \
        optind = index;                                                                                             \
    }
#endif //if defined (ENABLE_CSMI)

#define ERASE_RANGE_UTIL_VARS \
    bool                eraseRange                  = false; \
    uint64_t            eraseRangeStart             = 0; \
    uint64_t            eraseRangeEnd               = UINT64_MAX;/*set to a maximum value. If it stays this way, we know to correct this to the device's maxLBA later*/ \
    bool                eraseForceWrites            = false;//this is an option for SSD's in an erase test like eraseRange where we want to force the write commands to the drive instead of using TRIM/Unmap

#define ERASE_RANGE_SUBOPT_PARSE                                                \
    if (optarg != NULL)                                                         \
    {                                                                           \
        eraseRange = true;                                                      \
        int  index = optind - 1;                                                \
        char *nextSubOpt = NULL;                                                \
        bool startSet = false;                                                  \
        while (index < argc)                                                    \
        {                                                                       \
            nextSubOpt = strdup(argv[index]);                                   \
            if (strncmp("-", nextSubOpt, 1) != 0)                               \
            {                                                                   \
                if (strncmp("forceWrites", nextSubOpt, strlen(nextSubOpt)) == 0)\
                {                                                               \
                    eraseForceWrites = true;                                    \
                }                                                               \
                else if (isdigit(nextSubOpt[0]) != 0)                           \
                {                                                               \
                    if (startSet == false)                                      \
                    {                                                           \
                        startSet = true;                                        \
                        eraseRangeStart = (uint64_t)atoll(nextSubOpt);          \
                    }                                                           \
                    else                                                        \
                    {                                                           \
                        eraseRangeEnd = (uint64_t)atoll(nextSubOpt);            \
                    }                                                           \
                }                                                               \
            }                                                                   \
            else                                                                \
            {                                                                   \
                break;                                                          \
            }                                                                   \
            index++;                                                            \
        }                                                                       \
        optind = index;                                                         \
    }

#define ERASE_TIME_UTIL_VARS \
bool                eraseTime = false;\
time_t              eraseTimeSeconds = 0;\
uint64_t            eraseTimeStartLBA = 0;

#define ERASE_TIME_SUBOPT_PARSE                                                                                                 \
    if (optarg != NULL)                                                                                                         \
    {                                                                                                                           \
        eraseTime = true;                                                                                                       \
        int  index = optind - 1;                                                                                                \
        char *nextSubOpt = NULL;                                                                                                \
        bool lbaSet = false;                                                                                                    \
        uint64_t timeMultiplier = 1;/*use this if the user enters hours or minutes to increment the erase time*/                \
        while (index < argc)                                                                                                    \
        {                                                                                                                       \
            nextSubOpt = strdup(argv[index]); /*get the next subopt*/                                                           \
            if (strncmp("-", nextSubOpt, 1) != 0)/*check if optarg is next switch so that we break out of parsing suboptions*/  \
            {                                                                                                                   \
                if (strncmp("minutes", nextSubOpt, strlen(nextSubOpt)) == 0)                                                    \
                {                                                                                                               \
                    timeMultiplier = 60;                                                                                        \
                }                                                                                                               \
                else if (strncmp("hours", nextSubOpt, strlen(nextSubOpt)) == 0)                                                 \
                {                                                                                                               \
                    timeMultiplier = 3600;                                                                                      \
                }                                                                                                               \
                else if (strncmp("days", nextSubOpt, strlen(nextSubOpt)) == 0)                                                  \
                {                                                                                                               \
                    timeMultiplier = 86400;                                                                                     \
                }                                                                                                               \
                else if (isdigit(nextSubOpt[0]) != 0)/*this is an LBA*/                                                         \
                {                                                                                                               \
                    if (lbaSet == false)                                                                                        \
                    {                                                                                                           \
                        lbaSet = true;                                                                                          \
                        eraseTimeStartLBA = (uint64_t)atoll(nextSubOpt);                                                        \
                    }                                                                                                           \
                    else                                                                                                        \
                    {                                                                                                           \
                        eraseTimeSeconds = (uint64_t)atoll(nextSubOpt);                                                         \
                    }                                                                                                           \
                }                                                                                                               \
            }                                                                                                                   \
            else                                                                                                                \
            {                                                                                                                   \
                break;                                                                                                          \
            }                                                                                                                   \
            index++;                                                                                                            \
        }                                                                                                                       \
        optind = index; /*reset this since we were searching for options to pull out around getopt*/                            \
        eraseTimeSeconds *= timeMultiplier;/*adjust to a time in seconds if something other than seconds were entered*/         \
    }

#define SANITIZE_SUBOPT_PARSE \
if (optarg != NULL)                                                                                                         \
{                                                                                                                           \
    int  index = optind - 1;                                                                                                \
    char *nextSubOpt = NULL;                                                                                                \
    while (index < argc)                                                                                                    \
    {                                                                                                                       \
        nextSubOpt = strdup(argv[index]); /*get the next subopt*/                                                           \
        if (strncmp("-", nextSubOpt, 1) != 0) /*check if optarg is next switch so that we break out of parsing suboptions*/ \
        {                                                                                                                   \
            if (strncmp("blockerase", nextSubOpt, strlen(nextSubOpt)) == 0)                                                 \
            {                                                                                                               \
                sanitize = true;                                                                                            \
                sanblockErase = true;                                                                                       \
            }                                                                                                               \
            else if (strncmp("cryptoerase", nextSubOpt, strlen(nextSubOpt)) == 0)                                           \
            {                                                                                                               \
                sanitize = true;                                                                                            \
                sancryptoErase = true;                                                                                      \
            }                                                                                                               \
            else if (strncmp("overwrite", nextSubOpt, strlen(nextSubOpt)) == 0)                                             \
            {                                                                                                               \
                sanitize = true;                                                                                            \
                sanoverwrite = true;                                                                                        \
            }                                                                                                               \
            else if (strncmp("freezelock", nextSubOpt, strlen(nextSubOpt)) == 0)                                            \
            {                                                                                                               \
                sanitize = true;                                                                                            \
                sanfreezelock = true;                                                                                       \
            }                                                                                                               \
            else if (strncmp("antifreezelock", nextSubOpt, strlen(nextSubOpt)) == 0)                                        \
            {                                                                                                               \
                sanitize = true;                                                                                            \
                sanAntiFreezeLock = true;                                                                                   \
            }                                                                                                               \
            else if (strncmp("info", nextSubOpt, strlen(nextSubOpt)) == 0)                                                  \
            {                                                                                                               \
                sanitize = true; /*this flag is used to show the support sanitize commands*/                                \
                sanitizeInfo = true;                                                                                        \
            }                                                                                                               \
        }                                                                                                                   \
        else                                                                                                                \
        {                                                                                                                   \
            break;                                                                                                          \
        }                                                                                                                   \
        index++;                                                                                                            \
    }                                                                                                                       \
    optind = index; /*reset this since we were searching for options to pull out around getopt*/                            \
}

#if defined (__cplusplus)
}
#endif
