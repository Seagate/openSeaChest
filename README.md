
# openSeaChest

## Cross platform storage device utilities to manage, configure, and read health information for SATA, SAS, NVMe, and USB attached HDDs and SSDs.

### Copyright (c) 2014-2025 Seagate Technology LLC and/or its Affiliates, All Rights Reserved

[![MSBuild](https://github.com/Seagate/openSeaChest/actions/workflows/msbuild.yml/badge.svg)](https://github.com/Seagate/openSeaChest/actions/workflows/msbuild.yml)
[![CodeQL](https://github.com/Seagate/openSeaChest/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/Seagate/openSeaChest/actions/workflows/codeql-analysis.yml)
[![C/C++ CI](https://github.com/Seagate/openSeaChest/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/Seagate/openSeaChest/actions/workflows/c-cpp.yml)
[![CI for meson build](https://github.com/Seagate/openSeaChest/actions/workflows/meson.yml/badge.svg)](https://github.com/Seagate/openSeaChest/actions/workflows/meson.yml)
[![Reproducible Builds](https://img.shields.io/badge/reproducible_builds-compatible-blue)](https://reproducible-builds.org/)
[![License: MPL 2.0](https://img.shields.io/badge/License-MPL--2.0-blue.svg?longCache=true)](https://opensource.org/licenses/MPL-2.0)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/10600/badge)](https://www.bestpractices.dev/projects/10600)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/Seagate/openSeaChest/badge)](https://scorecard.dev/viewer/?uri=github.com/Seagate/openSeaChest)
[![SLSA 3](https://slsa.dev/images/gh-badge-level3.svg)](https://slsa.dev)
[![Dependabot Status](https://img.shields.io/badge/dependabot-enabled-success)](https://github.com/Seagate/openSeaChest/security/dependabot)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/Seagate/openSeaChest/graphs/commit-activity)
[![GitHub last commit](https://img.shields.io/github/last-commit/Seagate/openSeaChest)](https://github.com/Seagate/openSeaChest/commits/develop)

Welcome to the `openSeaChest` open source project!

openSeaChest is your storage device expert - a collection of **18 task-focused utilities** covering **200+ commands** so you don't need to be a low-level command expert. These comprehensive, easy-to-use command line tools and programming libraries help you quickly determine the health and status of your storage devices, configure settings, update firmware, securely erase data, and much more. Whether you're checking drive health, running diagnostics, or performing advanced operations, openSeaChest provides the capabilities you need across SATA, SAS, NVMe, and USB interfaces.

## 🚀 Get Started

**New to openSeaChest or command-line tools?**

📚 **[Visit Our Wiki](https://github.com/Seagate/openSeaChest/wiki)** - Step-by-step guides, tutorials, and comprehensive documentation

🏥 **[Check Your Drive Health](https://github.com/Seagate/openSeaChest/wiki/Drive-Health-and-SMART)** - Learn how to diagnose drive issues and monitor SMART data

💾 **[Download Latest Release](https://github.com/Seagate/openSeaChest/releases)** - Pre-compiled binaries for Windows, Linux, and BSD systems

🎯 **[Introduction to Command Line Tools](https://github.com/Seagate/openSeaChest/wiki/Introduction-To-Command-Line-Tools)** - Perfect for beginners

## Why Choose openSeaChest?

openSeaChest provides a **task-focused, cross-platform approach** to storage device management. Instead of requiring deep knowledge of low-level ATA, SCSI, or NVMe command sets, openSeaChest handles the complexity for you with utilities organized by what you want to accomplish.

**Key advantages:**
- **Cross-platform**: Same commands work on Windows, Linux, FreeBSD, OpenBSD, NetBSD, Solaris, and more
- **Unified interface**: One set of tools for ATA/SATA, SCSI/SAS, and NVMe devices
- **Task-focused**: Choose utilities by goal (check health, erase data, update firmware) rather than by command
- **Extensive USB support**: Compatibility testing and workarounds for USB-attached devices
- **Comprehensive**: Diagnostics, configuration, firmware updates, security, erasure, and advanced features

For a detailed comparison with [smartctl](https://www.smartmontools.org/), [sg3_utils](https://sg.danny.cz/sg/sg3_utils.html), [hdparm](https://sourceforge.net/projects/hdparm/), [sdparm](https://sg.danny.cz/sg/sdparm.html), [nvme-cli](https://github.com/linux-nvme/nvme-cli), and other tools, see our [wiki comparison page](https://github.com/Seagate/openSeaChest/wiki#-why-choose-openseachest).

**Note:** We have the utmost respect for these excellent, proven tools used by professionals worldwide. Each has specific strengths, and openSeaChest complements them with a unified, task-focused approach across platforms and device types.

### About openSeaChest Command Line Diagnostics

Seagate offers both graphical user interface (GUI) and command line interface
(CLI) diagnostic tools for our storage devices. [SeaTools](https://www.seagate.com/support/downloads/seatools/) is our popular GUI tool for end users, supporting 15 languages.

openSeaChest provides command line utilities for users who prefer or require CLI tools.
These tools use "command line arguments" to define tasks and specify devices. While designed
to be accessible to newcomers (see our [beginner-friendly wiki guides](https://github.com/Seagate/openSeaChest/wiki/Introduction-To-Command-Line-Tools)),
they also provide advanced capabilities for expert users. CLI tools are in English and available
for Windows, Linux, FreeBSD, and other platforms.

Linux versions of openSeaChest tools are available as stand alone 32 or 64-bit
executables you can copy to your own system.  Windows OS versions of
openSeaChest diagnostics are also available.

**Community Support:** Traditional technical support is not available for openSeaChest. However, we greatly appreciate feedback! Please use the [Issues](https://github.com/Seagate/openSeaChest/issues) tab to report bugs, request features, or ask questions. You can also reach us at opensource@seagate.com. Please include the tool name and version (use `--version`) when reporting issues.

**Device Compatibility:** openSeaChest supports SATA, SAS, NVMe, and USB interface devices across multiple operating systems. In some cases, it has successfully recognized legacy PATA and SCSI devices, though this is not the primary focus.

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
| DragonFlyBSD | Yes      | More or less same capabilities as FreeBSD. Notable difference is that DragonflyBSD uses a SAT translator for ATA devices rather than CAM ATA support. This may or may not limit capabilities. |
| UEFI Shell | Yes, but not currently maintained | While source code support is largely maintained for UEFI, it has not been updated or built due to many significant limitations on shipping systems. Some do not include the ATA driver that can respond to passthrough requests, only a block driver is available to allow booting the system. We are happy to revive this and find a way to add CI for this upon request. |
| Solaris    | Yes        | This column is for the Oracle release of Solaris. USCSI ioctl support is implemented for passthrough support. No known limitations at this time. |
| Illumos    | Yes        | This column is for Illumos based distributions/openSolaris. Uses same USCSI ioctl as Solaris. No known limitations at this time. |
| AIX        | Yes        | Support for SATA and SAS is available. Code was tested on version 7.1, but may work on earlier versions. Some flags may need adjusting for earlier systems. Build was completed using GCC available in AIX toolkit with gmake. Only rhdisk handles are supported. pdisk support is not known since IBM does not appear to provide information on passing requests through RAID with what is normally available. |
| ESXI       | Yes        | Uses SG_IO v3 like Linux through compatibility layer. Requires complicated build system with special GCC build/VM from VMWare and some other development packges installed to compile. |
| NetBSD     | Yes | ATA Passthrough limited to 28bit commands only due to kernel IOCTL limitations. |
| OpenBSD    | Yes | ATA Passthrough limited to 28bit commands only due to kernel IOCTL limitations. |
| HP-UX      | Partially  | Code has been implemented to support HP-UX's SCSI passthrough but it is untested. |

##### NVMe Compatibility

| OS                | Supported? | Notes |
|-------------------|------------|-------|
| Windows 10 & later| Yes        | Microsoft driver has many limitations, but many are documented online. Sometimes need to use SCSI translation. |
| Windows 8.1       | Limited    | Mostly limited to SCSI translation except for FW update. |
| Windows 7         | Limited    | Entirely limited to SCSI translation |
| Windows openFabrics compatible driver | Yes | Supported, but may be limited to what commands are allowed by the driver (at least in latest openSource code). Some vendor's NVMe drivers reuse the IOCTL for passthrough from this driver and may be supported. |
| Linux             | Yes        | Using built-in kernel driver and IOCTLs |
| FreeBSD           | Yes        |       |
| DragonflyBSD      | Maybe      | No NVMe in CAM at time of writing. Unknown if IOCTL is available. It will automatically enable during build time if it matches FreeBSD's NVMe IOCTL that was first available prior to CAM NVMe support. |
| UEFI Shell        | Yes, but not currently maintained | While source code support is largely maintained for UEFI, it has not been updated or built due to many significant limitations on shipping systems. Some systems do not include an NVMe driver that can respond to passthrough requests, only a block driver is available to allow booting the system.  We are happy to revive this and find a way to add CI for this upon request. |
| Solaris           | No, but possible | This column is for the Oracle release of Solaris. Possible to support NVMe through UNVME ioctl. |
| Illumos           | No, but possible | Been a while since last looked at, but appeared limited in what commands were available. |
| AIX               | Yes, untested | Implemented according to header from AIX 7.2, but support has not been tested |
| ESXI              | Yes        | Requires complicated build system with special GCC build/VM from VMWare and some other development packges installed to compile. |
| NetBSD            | No, but possible | |
| OpenBSD           | No, but possible | |
| HP-UX             | No         | Could not find any documentation about NVMe support in HP-UX |

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
