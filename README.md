
# openSeaChest

## Cross platform utilities useful for performing various operations on SATA, SAS, NVMe, and USB storage devices

### Copyright (c) 2014-2021 Seagate Technology LLC and/or its Affiliates, All Rights Reserved

[![Build(develop) Status](https://travis-ci.org/Seagate/openSeaChest.svg?branch=develop)](https://travis-ci.org/Seagate/openSeaChest)
[![License: Mozilla Public License 2.0](https://img.shields.io/badge/License-MPL--2.0-blue.svg?longCache=true)](https://opensource.org/licenses/MPL-2.0)
![OS](https://img.shields.io/badge/OS-Windows-blue)
![OS](https://img.shields.io/badge/OS-Linux-blue)
![OS](https://img.shields.io/badge/OS-FreeBSD-blue)

Welcome to the `openSeaChest` open source project!

openSeaChest is a collection of comprehensive, easy-to-use command line
diagnostic tools and programming libraries for storage devices that help you
quickly determine the health and status of your storage product.  The collection
includes several tests that show device information, properties and settings.
It includes several tests which may modify the storage product such as power
management features or firmware download.  It includes various commands to
examine the physical media on your storage device.  Close to 200 commands and
sub-commands are available in the various openSeaChest utilities.  These are described in more detail below.

Here is an overview presentation we gave at the *Storage Networking Industry Association - Storage Developer Conference 2018* that describes the design architecture for the **_opensea API_** and **_openSeaChest_** storage resource management utilities:

[![Video: SDC 2018 - What's better than sg3_utils, hdparm, sdparm?](https://img.youtube.com/vi/LMt8Ztlj5oQ/hqdefault.jpg)][video]

(Note: The openSeaChest team has the utmost respect for the highly regarded [sg3_utils](http://sg.danny.cz/sg/sg3_utils.html), [hdparm](https://sourceforge.net/projects/hdparm/), [sdparm](http://sg.danny.cz/sg/sdparm.html) and [nvme-cli](https://github.com/linux-nvme/nvme-cli) open source projects.  Since this is all pretty low-level stuff, we chose the presentation title *"What's better than sg3_utils, hdparm, sdparm?"* only to grab the attention of a few extra people attending the [SNIA SDC 2018 conference](https://www.snia.org/events/storage-developer/presentations18).)

[video]: https://www.youtube.com/watch?v=LMt8Ztlj5oQ

### About openSeaChest Command Line Diagnostics

Seagate offers both graphical user interface (GUI) and command line interface
(CLI) diagnostic tools for our storage devices.  SeaTools for end users is the
most popular GUI tools  These tools support 15 languages.

openSeaChest diagnostics are command line utilities which are available for
expert users.  These command line tools assume the user is knowledgeable about
running software from the operating system command prompt.  CLI tools are in
the English language only and use "command line arguments" to define the
various tasks and specific devices.  openSeaChest diagnostics are available for
both Linux and Windows environments.

Linux versions of openSeaChest tools are available as stand alone 32 or 64-bit
executables you can copy to your own system.  Windows OS versions of
openSeaChest diagnostics are also available.

Technical Support for openSeaChest drive utilities is not available.  If you
have the time to send us some feedback about this software, especially if you
notice something we should fix or improve, we would greatly appreciate hearing
from you.  To report your comments and suggestions, please use the issue
reporting feature available in this git repository.  Additionally, you can
contact us through this email address seaboard @ seagate.com.  Please let us
know the name and version of the tool you are using.

openSeaChest drive utilities support SATA, SAS, NVMe and USB interface devices.  In some cases openSeaChest has successfully recognized PATA and SCSI legacy devices, although the software is not expected to do so.

**openSeaChest_Basics** - Contains the most important tests and tools.  These include Drive Self Test (DST), Device Information, simple firmware download, and a few of the simple data erasure commands.

Other openSeaChest "break out" utilities are available and listed below which
offer more in-depth functionality in specific areas. These are:

**openSeaChest_Configure** - Tools to control various settings on the drives are
brought together into this one tool.  Typical commands, for example, include
Write Cache and Read Lookahead Cache enable or disable.  This is where you will find SCSI Mode Page and SCSI Log Page commands.

**openSeaChest_Erase** - The focus is on data erasure.  There are many different
choices for erasing data... from simple boot track erase to full cryptographic
erasure (when supported).  Some commands can take many hours to complete while
others may take less than a minute.  This tool is designed to erase data which will
be lost forever so be sure to back up important data before using this tool.  Actually, you should **always** maintain ongoing backups of your important data even if you do not intend to erase your drive.  Seagate is not responsible for lost user data.

**openSeaChest_Firmware** - Seagate products are run by firmware.  Having the
latest firmware can improve performance and or reliability of your product.
Seagate recommends applying new firmware to enhance the performance and or
reliability of your drive.  Most products may see one or two firmware updates
within the early life of the product.

**openSeaChest_Format** - Storage products which utilize the SAS, SCSI or Fibre
Channel interfaces are able to perform a full low-level media format in the
field.  The SCSI Format Unit command offers many specialty options to
accommodate a variety of conditions and to fine tune the procedure.  This
complex command deserves its own "break out" utility.

**openSeaChest_GenericTests** - Read Tests are the original disk drive diagnostic
which is to just read every sector on the drive, sequentially.  This tool
offers several common read tests which can be defined by either time or range.
In addition, the Long Generic test has the ability to repair problem sectors,
possibly eliminating an unnecessary drive return.

**openSeaChest_Info** - Historical generic activity logs (like total bytes written
and power on hours) and performance logs (like temperature) are available for
analytical review.  Identification and inquiry data stored on the drive is also
provided.  A view of SMART and Device Statistics is available when supported by
the drive.

**openSeaChest_Logs** - Several binary logs are maintained on disk drives.  These logs are not stored in the user data area. Sometimes these logs are reviewed by
support engineers to help analyze event history on the device.  These binary
data logs are saved to filenames using the drive's serial number and date-time.
Some logs are Seagate-specific while many others are common to the interface
specifications.  Several of these binary logs may be parsed into human-readable form by using the openSeaChest_LogParser utility.

**openSeaChest_NVMe** - NVMe interface devices tend to have unique commands and challenges.  Many of these commands are also unique to NVMe SSD products.  This tool gathers the most important commands under one title.  This tool is similar to openSeaChest_Basics but for NVMe.

**openSeaChest_PassThroughTest** - A comprehensive command line tool that can be
used to identify and analyze the unique quirks which may be present in a
USB-to-SATA or USB-to-NVMe bridge adapter.  The findings from this tool are
studied to help optimize openSeaChest libraries to send the best and safest
device interface commands, which are passed through to the storage device.

**openSeaChest_PowerControl** - Disk drives offer a multitude of options to
manage power.  This tool manipulates the various power modes.  Some power commands are Seagate-specific while many others are common to the interface specifications.

**openSeaChest_Security** - Various settings are available on modern Seagate disk
drives which may be locked and unlocked.  These settings may interact with the
operating systems and systems BIOS.  Options also include ATA Security Erase
for SATA interface drives.

**openSeaChest_SMART** - This tool provides a closer look at the collected SMART
attributes data.  SMART stands for Self-Monitoring, Analysis and Reporting
Technology.  Short Drive Self Test is included as one of the standard SMART
commands. In addition, the DST & Clean test has the ability to repair problem
sectors, possibly eliminating an unnecessary drive return.

### Important Notes

If this is your drive, you should always keep a current backup of your
important data.

Many tests and commands are completely data safe, while others will change the
drive (like firmware download or data erasure or setting the maximum capacity,
etc).  Be careful using openSeaChest because some of the features, like the
data erasure options, will cause data loss.   Some commands, like setting the
maximum LBA, may cause existing data on the drive to become inaccessible.  Some
commands, like disabling the read look ahead buffer, may affect the performance
of the drive.  Seagate is not responsible for lost user data.

**Important note:** Many tests in this tool directly reference storage device data
sectors, also known as Logical Block Addresses (LBA). Test arguments may
require a starting LBA or an LBA range.  The predefined variable 'maxLBA'
refers to the last sector on the drive.  Many older SATA and SAS storage
controllers (also known as Host Bus Adapters [HBA]) have a maximum addressable
limit of 4294967295 [FFFFh] LBAs hard wired into their design.  This equates to
2.1TB using 512 byte sectors.  This also means accessing an LBA beyond the
2.1TB limitation either will result in an error or simply the last readable LBA
(usually LBA 4294967295 [FFFFh]) depending on the actual hardware.  This
limitation can have important consequences.  For example, if you intended to
erase a 4TB drive, then only the first 2TB will actually get erased (or maybe
even twice!) and the last 2TB will remain untouched.  You should carefully
evaluate your system hardware to understand if your storage controllers provide
support for greater than 2.1TB.

**Note:** One gigabyte, or GB, equals one billion bytes when referring to hard
drive capacity. This software may use information provided by the operating
system to display capacity and volume size. The Windows file system uses a
binary calculation for gibibyte or GiB (2^30) which causes the abbreviated size
to appear smaller. The total number of bytes on a disk drive divided by the
decimal calculation for gigabyte or GB (10^9) shows the expected abbreviated
size. See this FAQ for more information
<http://knowledge.seagate.com/articles/en_US/FAQ/172191en?language=en_US>.

### Binary Availability

BINARIES and SOURCE CODE files of the openSeaChest open source project have
been made available to you under the [Mozilla Public License 2.0 (MPL-2.0)](LICENSE.md).  The
openSeaChest project repository is maintained at
[https://github.com/Seagate/openSeaChest](https://github.com/Seagate/openSeaChest).

Compiled binary versions of the openSeaChest utilities for various operating
systems may be found at
[https://github.com/Seagate/ToolBin/tree/master/openSeaChest](https://github.com/Seagate/ToolBin/tree/master/openSeaChest)

This collection of storage device utility software is branched (forked) off of
an original utility collection called the `Seagate SeaChest Utilities` by Seagate
Technology LLC.  The original SeaChest Utilities are still available at
www.seagate.com or [https://github.com/Seagate/ToolBin/tree/master/SeaChest](https://github.com/Seagate/ToolBin/tree/master/SeaChest).
Binary versions are available for Linux or Windows, with the Windows versions
signed by Seagate Technology LLC.

### The libraries

**opensea-common**      - Operating System common operations, not specific to
                      storage standards. Contains functions and defines that
                      are useful to all other libraries.

**opensea-transport**   - Contains standard ATA/SCSI/NVMe functions based on open
                      standards for these command sets.  This layer also
                      supports different transporting these commands through
                      operating systems to the storage devices. Code depends on
                      opensea-common.

**opensea-operations**  - Contains common use cases for operations to be performed
                      on a storage device. This layer encapsulates the nuances
                      of each command set (ATA/SCSI) and operating systems
                      (Linux/Windows etc.) Depends on opensea-common and
                      opensea-transport.

### Source

Depending on your git version & client you can use either of the following two commands to clone the repository.

`git clone --recurse-submodules -j8 https://github.com/Seagate/openSeaChest.git`

or

`git clone --recursive https://github.com/Seagate/openSeaChest.git`

Note that cloning **_recursively_** is **_important_** as it clones all the necessary submodules.

See [BUILDING.md](BUILDING.md) for more information about how to use git.

### Building

See [BUILDING.md](BUILDING.md) for information on how to build the openSeaChest tools on Windows, Linux, FreeBSD, and Solaris/Illumos.

### Contributions & Issues

See [CONTRIBUTING.md](CONTRIBUTING.md) for more information on contributions that will be accepted.
This document also describes how to create an issue, generate a pull request, and licenses that will be accepted.

### Security policy

See [SECURITY.md](SECURITY.md) for information on Seagate's security policy for details on how to report security vulnerabilities.

### Names, Logos, and Brands

All product names, logos, and brands are property of their respective owners.
All company, product and service names mentioned in the source code are for
clarification purposes only. Use of these names, logos, and brands does not
imply endorsement.

### Support and Open Source Statement

Support from Seagate Technology for open source projects is different than traditional Technical Support.  If possible, please use the **Issues tab** in the individual software projects so that others may benefit from the questions and answers.  Include the output of --version information in the message. See the user guide section 'General Usage Hints' for information about saving output to a log file.

If you need to contact us through email, please choose one of these
two email addresses:

- opensource@seagate.com   for general questions and bug reports
- opensea-build@seagate.com   for specific questions about programming and building the software

Seagate offers technical support for drive installation.  If you have any questions related to Seagate products and technologies, feel free to submit your request on our web site. See the web site for a list of world-wide telephone numbers.

- [http://www.seagate.com/support-home/](http://www.seagate.com/support-home/)
- Contact Us:
[http://www.seagate.com/contacts/](http://www.seagate.com/contacts/)

This software uses open source packages obtained with permission from the
relevant parties. For a complete list of open source components, sources and
licenses, please see our Linux USB Boot Maker Utility FAQ for additional
information.

The newest online version of the openSeaChest Utilities documentation, open
source usage and acknowledgement licenses, and our Linux USB Boot Maker FAQ can
be found at: [https://github.com/Seagate/openSeaChest](https://github.com/Seagate/openSeaChest).

Copyright (c) 2014-2021 Seagate Technology LLC and/or its Affiliates, All Rights Reserved

### License

BINARIES and SOURCE CODE files of the openSeaChest open source project have
been made available to you under the Mozilla Public License 2.0 (MPL).  Mozilla
is the custodian of the Mozilla Public License ("MPL"), an open source/free
software license.

The license can be views at [https://www.mozilla.org/en-US/MPL/](https://www.mozilla.org/en-US/MPL/) or [LICENSE.md](LICENSE.md)
