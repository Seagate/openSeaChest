15 4611686018427387903
./openSeaChest_LogParser_x86_64
-h
===============================================================================
 openSeaChest_LogParser - Seagate drive utilities
 Copyright (c) 2018-2020 Seagate Technology LLC and/or its Affiliates
 openSeaChest_LogParser Version: 1.3.2-2.0.1 X86_64
 Build Date: Jun 16 2022
 Today: Thu Jul  7 14:38:58 2022
===============================================================================
Usage
=====
	 openSeaChest_LogParser {arguments} {options}

Examples
========
	openSeaChest_LogParser --inputLog <filename> --logType farmLog --printType json --outputLog <filename>

Return Codes
============
	Generic/Common error codes
	0 = No Error Found
	1 = Error in command line options
	2 = Invalid Device Handle or Missing Device Handle
	3 = Operation Failure
	4 = Operation not supported
	5 = Operation Failed and was still in progress 
	6 = Operation Aborted
	7 = Operation Failed for Bad Parameter in the log 
	8 = Operation had Memory Failures
	9 = Operation Failed for Invaild Lengths in the log
	10 = File Path Not Found
	11 = Cannot Open File
	12 = File Already Exists
	13 = Not Valid for this parser
	20 = Validation Failed
	21 = Error in Header and Footer validation
	22 = Parsing Failure
	Anything else = unknown error

Utility Arguments
=================
	--inputLog [log file name]
		Use this option to pass a log into the tool for parsing.
		Use --inputLog <fromPipe> option to pass a farm log buffer into the tool for parsing.

	--logType [choose from list below]
		{ farmLog, identify, IDDataLog, deviceStatisticsLog, extCompErrorLog
		 sctTempLog, ncqErrorLog, powerConditionLog, extSelfTestLog
		 scsiLogPages } 

		Use this option to describe the type of log that is being passed in.

		 SCSI Log Pages  List -  
			Application Client, Background Scan, Cache Statistics
			Background Operation, Environmental Reporting (Coming soon)
			Factory Log, Environmental Limits (Coming soon)
			Write Log Page, Read Log Page, Verify Log Page
			Informational Exceptions, Format Status, DST Log Page
			Non-Medium Error, Logical Block Provisioning
			Power Conditions, Pending Defects, Protocol Page
			Start Stop Cycle Page, Solid State Media (Coming soon)
			Supported Log Pages and Subpages, Supported Log Pages
			Temperature Log Page, Utilization (Coming soon)

	--showStatusBits
		Command Line Option for the FARM Log ONLY 
		Use this option to set the parser to gather the status bytes for each field.
		For each field the Supported byte will show TRUE or FALSE 
		For each field the Valid byte will show TRUE or FALSE 
		Device Information will not show status bytes.

	--outputLog [log file name]
		To set a name of the output file being generated. This option will overwrite
		file if it exists.
		If no --outputLog given then data will be printed to the screen.

	--printType [choose from list below]
		{json, text, csv, flatcsv, prom}
		Use this option to set the output format. 

		json - prints the data in a printable json format
		text - prints the data in a printable and human readable format
		csv  - The data flows downwards
		flatcsv - The data is set to flow in two rows only
		prom - Prints the data in a format readable by Prometheus.

			If no output log is specified (with --outputLog), data is printed
			out to standard output and a file is automatically created.
			The output file will have the same name as the input file
			with the extension ".prom" which can be opened in any text editor.

			If an output log is specified (with --outputLog), data is printed
			directly to the specified file.

			[USAGE EXAMPLES]
			openSeaChest_LogParser --inputLog <fileName>.bin --logType farmLog --printType prom
				Takes in a FARM log, <fileName>.bin, prints the data in Prometheus' format
				to standard output, and saves the output in the current directory as <fileName>.prom.

			openSeaChest_LogParser --inputLog <inputFile>.bin --logType farmLog --printType prom --outputLog <outputFile>.prom
				Takes in a FARM log, <inputFile>.bin, and saves the output in the current directory
				<outputFile>.prom without printing to standard output.


Utility Options
===============
	--echoCommandLine
		Shows the command line above the banner in the statndard ouput.
		Useful when saving output to logs

	-h, --help
		Show utility options and example usage (this output you see now)

	--license
		Display the Seagate End User License Agreement (EULA).

	-v [0-4], --verbose [0 | 1 | 2 | 3 | 4]
		Show verbose information. Verbosity levels are:
		0 - quiet
		1 - default
		2 - command descriptions
		3 - command descriptions and values
		Example: -v 3 or --verbose 3

	-V, --version
		Show openSeaChest_LogParser version and copyright information & exit


