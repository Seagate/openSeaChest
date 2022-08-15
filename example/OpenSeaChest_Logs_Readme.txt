==========================================================================================
 openSeaChest_Logs - openSeaChest drive utilities - NVMe Enabled
 Copyright (c) 2014-2022 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
 openSeaChest_Logs Version: 2.0.3-3_0_0 X86_64
 Build Date: Jun 21 2022
 Today: Thu Jul  7 14:37:52 2022	User: root
==========================================================================================
Usage
=====
	 openSeaChest_Logs [-d <sg_device>] {arguments} {options}

Examples
========
	openSeaChest_Logs --scan
	openSeaChest_Logs -d /dev/sg? -i
	openSeaChest_Logs -d /dev/sg? --farm --outputPath logs

Return codes
============
	Generic/Common exit codes
	0 = No Error Found
	1 = Error in command line options
	2 = Invalid Device Handle or Missing Device Handle
	3 = Operation Failure
	4 = Operation not supported
	5 = Operation Aborted
	6 = File Path Not Found
	7 = Cannot Open File
	8 = File Already Exists
	9 = Need Elevated Privileges
	Anything else = unknown error


Utility Options
===============
	--echoCommandLine
		Echo the command line entered into the utility on the screen.

	--enableLegacyUSBPassthrough
		Only use this option on old USB or IEEE1394 (Firewire)
		products that do not otherwise work with the tool.
		This option will enable a trial and error method that
		attempts sending various ATA Identify commands through
		vendor specific means. Because of this, certain products
		that may respond in unintended ways since they may interpret
		these commands differently than the bridge chip the command
		was designed for.

	--forceATA
		Using this option will force the current drive to
		be treated as a ATA drive. Only ATA commands will
		be used to talk to the drive.

	--forceATADMA	(SATA Only)
		Using this option will force the tool to issue SAT
		commands to ATA device using the protocol set to DMA
		whenever possible (on DMA commands).
		This option can be combined with --forceATA

	--forceATAPIO	(SATA Only)
		Using this option will force the tool to issue PIO
		commands to ATA device when possible. This option can
		be combined with --forceATA

	--forceATAUDMA	(SATA Only)
		Using this option will force the tool to issue SAT
		commands to ATA device using the protocol set to UDMA
		whenever possible (on DMA commands).
		This option can be combined with --forceATA

	--forceSCSI
		Using this option will force the current drive to
		be treated as a SCSI drive. Only SCSI commands will
		be used to talk to the drive.

	-h, --help
		Show utility options and example usage (this output you see now)
		Please report bugs/suggestions to seaboard@seagate.com.
		Include the output of --version information in the email.

	--license
		Display the Seagate End User License Agreement (EULA).

	--modelMatch [model Number]
		Use this option to run on all drives matching the provided
		model number. This option will provide a closest match although
		an exact match is preferred. Ex: ST500 will match ST500LM0001

	--noBanner
		Use this option to suppress the text banner that displays each time
		openSeaChest is run.

	--onlyFW [firmware revision]
		Use this option to run on all drives matching the provided
		firmware revision. This option will only do an exact match.

	--onlySeagate
		Use this option to match only Seagate drives for the options
		provided

	--outputPath [folder]
		To set a path to the directory/folder where all logs should be created.
		The directory/folder must already exist with write permissions
		If this option is not used, logs are created in the current working folder

	-q, --quiet
		Run openSeaChest_Logs in quiet mode. This is the same as
		-v 0 or --verbose 0

	-v [0-4], --verbose [0 | 1 | 2 | 3 | 4]
		Show verbose information. Verbosity levels are:
		0 - quiet
		1 - default
		2 - command descriptions
		3 - command descriptions and values
		4 - command descriptions, values, and data buffers
		Example: -v 3 or --verbose 3

	-V, --version
		Show openSeaChest_Logs version and copyright information & exit


Utility Arguments
=================
	-d, --device deviceHandle
		Use this option with most commands to specify the device
		handle on which to perform an operation. Example: /dev/sg?

	-F, --scanFlags [option list]
		Use this option to control the output from scan with the
		options listed below. Multiple options can be combined.
			ata - show only ATA (SATA) devices
			usb - show only USB devices
			scsi - show only SCSI (SAS) devices
			nvme - show only NVMe devices
			interfaceATA - show devices on an ATA interface
			interfaceUSB - show devices on a USB interface
			interfaceSCSI - show devices on a SCSI or SAS interface
			interfaceNVME = show devices on an NVMe interface
			sd - show sd device handles
			sgtosd - show the sd and sg device handle mapping

	-i, --deviceInfo
		Show information and features for the storage device

	-s, --scan
		Scan the system and list all storage devices with logical
		/dev/sg? assignments. Shows model, serial and firmware
		numbers.  If your device is not listed on a scan  immediately
		after booting, then wait 10 seconds and run it again.

	-S, --Scan
		This option is the same as --scan or -s,
		however it will also perform a low level rescan to pick up
		other devices. This low level rescan may wake devices from low
		power states and may cause the OS to re-enumerate them.
		Use this option when a device is plugged in and not discovered in
		a normal scan.
		NOTE: A low-level rescan may not be available on all interfaces or
		all OSs. The low-level rescan is not guaranteed to find additional
		devices in the system when the device is unable to come to a ready state.

	--SATInfo
		Displays SATA device information on any interface
		using both SCSI Inquiry / VPD / Log reported data
		(translated according to SAT) and the ATA Identify / Log
		reported data.

	--testUnitReady
		Issues a SCSI Test Unit Ready command and displays the
		status. If the drive is not ready, the sense key, asc,
		ascq, and fru will be displayed and a human readable
		translation from the SPC spec will be displayed if one
		is available.


	--deviceStatisticsLog
		This option will pull the Device Statistics Log
		from a device.

	--farm
		Pull the Seagate Field Accessible Reliability Metrics (FARM)
		Log from the specified drive.Saves the binary logs to the
		current directory as <serialnumber>FARM<date and time>.bin

	--listSupportedLogs
		Displays a list of all supported logs by this device type.

	--logMode [mode]
		Sets the mode to pull the log. 
		Use this option with --pullLog to set the desired mode
			raw - Pulls log & prints it to the
			      screen as stdout. (default)
			bin - Pulls log & saves it to
			      a timestamped binary file.
			pipe - Pulls log, prints it to the
			       screen as stdout & send the
			       result to openSeaChest_LogParser.
			       (available for FARM only)

	--logTransferLength [length in bytes]
		Use this option to specify the data transfer
		length for a log transfer.
		Larger transfer sizes may speed up log retrieval at the
		loss of compatibility.
		The following post fixes are allowed for
		specifying a transfer length:
			BLOCKS or SECTORS - used to specify a transfer length
				in device in 512Byte blocks/sectors
			KB - length in kilobytes (val * 1000)
			KiB - length in kibibytes (val * 1024)
			MB - length in megabytes (val * 1000000)
			MiB - length in mebibytes (val * 1048576)
		ATA drives must be given a value in 512B increments.
		Warning: Specifying a large size may result in
		failures due to OS, driver, or HBA/bridge specific limitations.

	--pullLog [Log Number]
		Pulls specific log number from the device
		[Log Number] is required argument & can be passed
		as an decimal or hex value.
		WARNING:  Vendor Unique Logs pulled using this option
		          may not be valid due to unknown vendor unique
		          bits in ATA/SCSI/NVMe etc. command fields.

	--selfTestLog
		This option will pull the self test results log
		from a device. On ATA drives, this will pull the
		extended SMART self tests result log when it is
		supported by the device.


	SATA Only:

	--identifyDataLog		(SATA only)
		This option will pull the Identify Device data
		log from an ATA drive.

	--SATAPhyCntLog			(SATA only)
		This option will pull the SATA Phy Event Counters
		log from a SATA drive.


	SAS Only:

	--listErrorHistoryIDs			(SAS Only)
		Displays a list of all supported error history buffer IDs
		supported by the device.

	--pullErrorHistoryID [Buffer ID]	(SAS Only)
		Pulls specific error history buffer ID from the device
		[Buffer ID] is required argument & can be passed
		as an decimal or hex value.
		WARNING:  Vendor Unique Logs pulled using this option
		          may not be valid due to unknown vendor unique
		          bits in ATA/SCSI/NVMe etc. command fields.

	--infoExceptionsLog			(SAS only)
		This option will pull the SCSI Informational
		Exceptions log page from a SCSI device.

	--pullSubpage [Subpage Number]		(SAS Only)
		Use this option with the --pullLog option to specify
		a log subpage to pull. Use this for SCSI Logs.
		[Subpage Number] can be passed as an decimal or hex value.
		WARNING:  Vendor Unique Logs pulled using this option
		          may not be valid due to unknown vendor unique
		          bits in ATA/SCSI/NVMe etc. command fields.


