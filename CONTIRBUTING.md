# Contributing to OpenSeaChest

Thank you for being interested in contributing!
In here, you will find information on the ways contributions can be made to the project.

#### Table Of Contents

[Code Of Conduct](#code-of-conduct)

[Asking Questions](#asking-questions)

[Getting Started](#getting-started)

[Ways to Contribute](#ways-to-contribute)
  * [Report Bugs](#report-bugs)
    * [Submit Compatibility Report for USB devices](#submit-compatibility-report-for-usb-devices)
  * [Suggest New Features](#suggest-new-features)
  * [Contributing Code](#contributing-code)
    * [Commit Messages](#commit-messages)
      * [Commit Format](#commit-format)
      * [Using Commit Template](#using-commit-template)
      * [Commit Message Components](#commit-message-components)
    * [Code Style](#code-style)
    * [Contribution Licenses](#contribution-licenses)

## Code Of Conduct
This project is governed by the [code of conduct](CODE_OF_CONDUCT.md). You are expected to follow this as you contribute to the project.
Please report all unacceptable behavior to [opensource@seagate.com](mailto:opensource@seagate.com).

## Asking questions
If you have a question, please create a new issue with the label `question`. We will do our best to respond to questions, and document answers to them wherever we think it will be best to store these answers. Before asking a question, please review the tool's help files and user guides to make sure your question isn't already answered in there.

If you need to contact us through email for other questions, please choose one of these two email addresses:

[opensource@seagate.com](mailto:opensource@seagate.com) for general questions and bug reports
[opensea-build@seagate.com](mailto:opensea-build@seagate.com) for specific questions about programming and building the software

## Getting Started
OpenSeaChest is made up of a few libraries, and it is important to understand which one is responsible for what.
The Libraries:
  * [opensea-common](https://github.com/seagate/opensea-common) - This is a set of common functions and macros to provide some commonly used things within all the other tools or libraries.
  In here you will find things for high resolution timers, bit-banging, temperature conversion, operating system functions, etc.
  * [opensea-transport](https://github.com/seagate/opensea-transport) - Handles the way in which an IO is issued to a storage device in different operating systems, APIs, drivers, etc.
  Aside from the OS specific passthrough code, you will also find definitions for various SCSI, ATA, & NVMe commands, and SCSI translation code for different interfaces.
  * [opensea-operations](https://github.com/seagate/opensea-operations) - Performs the testing most people are familiar with for things like SMART check, Diagnostic Self Test (DST),
  configuring drive features, etc. Many of these are focused on sets of commands or command sequences to do the "fun" stuff to storage devices.
  * [wingetopt](https://github.com/Seagate/wingetopt) - This library is a forked, minimally changed, library that provides support for the getopt and getoptlong functions defined by POSIX to run in Windows.

## Ways to Contribute
### Report Bugs
Found a bug? Well it's best to report it!

To report a bug, please create a new issue with the label `bug` and provide a one line description for the issue title.

In your report, provide as much detail as possible. The following is a list of things that are helpful to ensure the issue is properly debugged and repeated:
  * Storage Product model (Seagate products typically start with ST)
  * How is the product attached to the host? AHCI controller, SAS/SATA HBA, RAID HBA, USB, FireWire, etc
  * What operating system (OS) was being used when the bug was found? Please provide as much version information as possible: Linux kernel version, Windows 10 update # (1903, 1909, 2004, etc)
  * What is the exact command line you used with the tool?
  * If possible, run with these options and attach the verbose output: `--echoCommandLine -v 3`, in some cases, `-v 4` will be necessary to debug the issue.
  * Names & versions of other software that works on this hardware configuration (if any)
  * If possible, does this bug show up in multiple operating systems, or does it show up if you change how the device is connected to the system (Ex: Moving from USB to SATA controller)

#### Submit Compatibility Report for USB devices
USB Devices are tricky for many technical reasons. In order to best work around them, Seagate created openSeaChest_PassthroughTest to quickly test them and output information related to
their compatibility to issue commands. If you are reporting USB compatibility test information, please include the full output of the tool. Currently, this is best run under Windows.

### Suggest New features
Thought of another cool thing to be able to do? Want to control some other aspect not already available? Want support on a new OS? Great!

Create a new issue with the label `enhancement` describing what it is.
If this is a feature available through other storage tools, please let us know the tool, version, and what options it uses so we can check into it.

If requesting support for an operating system we don't already support, please list any other tools you know of that run on that OS and where it can be
downloaded from. If it's an opensource OS and you know where the source is, please provide a link to it as well.

### Contributing Code
Like to code and want to be more involved? Awesome!

Here's the best way to submit your code:
  1. Fork the repository
  2. Make your changes (follow commit message format and code style below). Update version numbers for the tool(s) modified.
  Use [SemVer](https://semver.org/). For library changes, patch versions are automatically updated by CI, only changes to Major or Minor versioning is required.
  3. Submit a pull request. If it is associated with an issue, add a comment in the issue referencing the pull request.
  4. A Seagate developer will review it. Please make any additional changes that are suggested.
  5. Pull request is accepted! Celebrate!

#### Commit Messages
Commit messages should have the following format described below.

If in doubt, check the repository log for examples of other commits.

All commits should include the [Developer Certificate of Origin](https://developercertificate.org/) signoff.

##### Commit Format
`category`:`short description`

`Body of message should be here to thoroughly describe your changes. This can be as long as you want.`

`[issue #, if any]`

`Signed-off-by: Your Name <Your Email Address>`

##### Using Commit Template
A template for the commit message is provided in the repository, named commit_template.txt
First, you will need to make a modified copy inserting your name and email for DCO.

Once, modified, you can set this template to be used globally, or per repository:
###### Global Commit Template
git config –-global commit.template path/modified_commit_template.txt

###### Local Commit Template
git config commit.template path/modified_commit_template.txt

##### Commit Message Components
| Category | Description |
| --- | --- |
| `bug` | This commit is to fix a bug |
| `feat` | This commit contains code adding a new feature |
| `quick` | Code quickly fixes, or hot fixes, a small bug |
| `clean` | Cleaning up warnings, or general code cleanup |
| `doc` | Update to documentation, which includes comments |
| `make` | Fixes to makefiles, visual studio project files, or any other files related to ability to build the code |
| `lib` | Updating the submodule/library pointers to a new commit |
| `release` | Software release |
| `save` | checking in code to save place while working on larger bug or feature. |

#### Code Style
This section will be kept as short as possible. In general, try to match the existing style in the rest of the code base.

* All code should compile without warnings or errors to the best of your ability, at least in the compiler you have used while making your changes. If you can test more than one compiler, that would be great!
* Code should be compatible with the C99 standard, but does not use variable length stack arrays.
* Parenthesis around if's, loops, etc should be on new lines.
* Name variables appropriately and so that others can understand what your code is trying to accomplish. If using abbreviations or acronyms, please add comments describing what they mean.
* Use C99 inttypes whenever possible. Bools are also OK. If interacting with an API to the OS, this is not required.
* Avoid gotos in favor of other ways of writing the code.
* If code is specific to a given drive type (ATA, SCSI, NVMe, etc), place the name of that drive type in the beginning of the function. Ex: ata_Write_Long()
* Add code comments that you think are appropriate for someone else to come along and understand what is going on to the best of your ability.

#### Contribution Licenses
All contributions should be under the same MPL 2.0 license the project uses, with exceptions for additions that come from projects under other licenses.

In general, the license should be:
 1. compatible with MPL 2.0
 2. Allow for binary redistribution without the need for source code distribution to also be distributed for the projects that include this (Ex: SeaTools can include these libraries without needing to opensource all of the SeaTools code)

Any code being contributed without MPL 2.0 license will require additional review by Seagate to make sure it is ok to include in this project and the review may take longer to complete before it is accepted.

 * 2-Clause BSD, 3-Clause BSD, MIT, & Apache 2.0 licensed code is most likely to be accepted.
 * LGPL, GPL, and Affero GPL licensed code will not be accepted as it is too restrictive on redistributed binary builds for some of Seagate's customers that use this project.
 * Other licenses will require further review at when contributions are made.

Example: Adding code from another opensource project that adds support for a USB adapter or RAID not already supported may fall under a different license. In this case, it will require review to make sure that this license is compatible with the MPL 2.0 license and fits the redistribution model that Seagate would like to preserve.
