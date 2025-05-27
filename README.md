
# openSeaChest

## Cross platform storage device utilities to manage, configure, and read health information for SATA, SAS, NVMe, and USB attached HDDs and SSDs.

### Copyright (c) 2014-2025 Seagate Technology LLC and/or its Affiliates, All Rights Reserved

[![MSBuild](https://github.com/Seagate/openSeaChest/actions/workflows/msbuild.yml/badge.svg)](https://github.com/Seagate/openSeaChest/actions/workflows/msbuild.yml)
[![CodeQL](https://github.com/Seagate/openSeaChest/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/Seagate/openSeaChest/actions/workflows/codeql-analysis.yml)
[![C/C++ CI](https://github.com/Seagate/openSeaChest/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/Seagate/openSeaChest/actions/workflows/c-cpp.yml)
[![FreeBSD build status](https://api.cirrus-ci.com/github/Seagate/openSeaChest.svg)](https://cirrus-ci.com/github/Seagate/openSeaChest)
[![License: Mozilla Public License 2.0](https://img.shields.io/badge/License-MPL--2.0-blue.svg?longCache=true)](https://opensource.org/licenses/MPL-2.0)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/10600/badge)](https://www.bestpractices.dev/projects/10600)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/Seagate/openSeaChest/badge)](https://scorecard.dev/viewer/?uri=github.com/Seagate/openSeaChest)
[![SLSA 3](https://slsa.dev/images/gh-badge-level3.svg)](https://slsa.dev)

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

For a description of all of our tools please see [our Wiki](https://github.com/Seagate/openSeaChest/wiki/Introduction-To-Command-Line-Tools).
This pages also includes some instructions for those who are new to command line tools to help get you started!

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
[openSeaChest Releases](https://github.com/Seagate/openSeaChest/releases).

This collection of storage device utility software is branched (forked) off of
an original utility collection called the `Seagate SeaChest Utilities` by Seagate
Technology LLC.  The original SeaChest Utilities are still available at
[www.seagate.com](www.seagate.com) or [https://github.com/Seagate/ToolBin/tree/master/SeaChest](https://github.com/Seagate/ToolBin/tree/master/SeaChest).
Binary versions are available for Linux or Windows, with the Windows versions
signed by Seagate Technology LLC.

For details on what is different between openSeaChest and SeaChest, see [this wiki page](https://github.com/Seagate/openSeaChest/wiki/openSeaChest-VS-SeaChest).

#### Repository Availability

With help from various community members across distributions, openSeaChest may be
available from within your package manager.

[![Packaging status](https://repology.org/badge/vertical-allrepos/openseachest.svg)](https://repology.org/project/openseachest/versions)

Please be aware this list may not be complete.
Seagate is happy to work with community members to help make openSeaChest available in
other package repositories as well. Please reach out through an issue and we will
do our best to help make it more available when possible.

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

#### Source Compatibility with various OSs

OpenSeaChest strives to be made available on any platform and any processor. Support for platforms has been
added upon request from customers and issues that are created on this repository.
The following tables illustrate platform support that exists today, at least in source code form, and contains
notes about other platforms that are not currently supported, but may be possible to support. The list may not be
complete. Feel free to request other OS support as well, but it may require more research or development in order
to support.
Please create an issue to request support for these other platforms when you need it!

##### SAS/SATA Compatibility

| OS         | Supported? | Notes |
|------------|------------|-------|
| Windows    | Yes        | Windows vista and higher are supported |
| Linux      | Yes        | libATA blocks ATA security commands be default. Can add kernel parameter to disable this block. Only SG_IO IOCTL support implemented for version 3 today. HDIO support is not available. Should work on any 2.6 and later kernel. Earlier kernels may work too, but has not been tested. |
| FreeBSD    | Yes        | No known limitations at this time |
| UEFI Shell | Yes, but not currently maintained | While source code support is largely maintained for UEFI, it has not been updated or built due to many significant limitations on shipping systems. Some do not include the ATA driver that can respond to passthrough requests, only a block driver is available to allow booting the system. We are happy to revive this and find a way to add CI for this upon request. |
| Solaris    | Yes        | This column is for the Oracle release of Solaris. USCSI ioctl support is implemented for passthrough support. No known limitations at this time. |
| Illumos    | Yes        | This column is for Illumos based distributions/openSolaris. Uses same USCSI ioctl as Solaris. No known limitations at this time. |
| AIX        | Yes        | Support for SATA and SAS is available. Code was tested on version 7.1, but may work on earlier versions. Some flags may need adjusting for earlier systems. Build was completed using GCC available in AIX toolkit with gmake. Only rhdisk handles are supported. pdisk support is not known since IBM does not appear to provide information on passing requests through RAID with what is normally available. |
| ESXI       | Yes        | Uses SG_IO v3 like Linux through compatibility layer. Requires complicated build system with special GCC build/VM from VMWare and some other development packges installed to compile. |
| NetBSD     | Yes | ATA Passthrough limited to 28bit commands only due to kernel IOCTL limitations. |
| OpenBSD    | Yes | ATA Passthrough limited to 28bit commands only due to kernel IOCTL limitations. |

##### NVMe Compatibility

| OS                | Supported? | Notes |
|-------------------|------------|-------|
| Windows 10 & later| Yes        | Microsoft driver has many limitations, but many are documented online. Sometimes need to use SCSI translation. |
| Windows 8.1       | Limited    | Mostly limited to SCSI translation except for FW update. |
| Windows 7         | Limited    | Entirely limited to SCSI translation |
| Windows openFabrics compatible driver | Yes | Supported, but may be limited to what commands are allowed by the driver (at least in latest openSource code). Some vendor's NVMe drivers reuse the IOCTL for passthrough from this driver and may be supported. |
| Linux             | Yes        | Using built-in kernel driver and IOCTLs |
| FreeBSD           | Yes        |       |
| UEFI Shell        | Yes, but not currently maintained | While source code support is largely maintained for UEFI, it has not been updated or built due to many significant limitations on shipping systems. Some systems do not include an NVMe driver that can respond to passthrough requests, only a block driver is available to allow booting the system.  We are happy to revive this and find a way to add CI for this upon request. |
| Solaris           | No, but possible | This column is for the Oracle release of Solaris. Possible to support NVMe through UNVME ioctl. |
| Illumos           | No, but possible | Been a while since last looked at, but appeared limited in what commands were available. |
| AIX               | Yes, untested | Implemented according to header from AIX 7.2, but support has not been tested |
| ESXI              | Yes        | Requires complicated build system with special GCC build/VM from VMWare and some other development packges installed to compile. |
| NetBSD            | No, but possible | |
| OpenBSD           | No, but possible | |

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

Copyright (c) 2014-2025 Seagate Technology LLC and/or its Affiliates, All Rights Reserved

### License

BINARIES and SOURCE CODE files of the openSeaChest open source project have
been made available to you under the Mozilla Public License 2.0 (MPL).  Mozilla
is the custodian of the Mozilla Public License ("MPL"), an open source/free
software license.

The license can be views at [https://www.mozilla.org/en-US/MPL/](https://www.mozilla.org/en-US/MPL/) or [LICENSE.md](LICENSE.md)
