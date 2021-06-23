# Building openSeaChest

openSeaChest tools can be built for a variety of platforms.
Windows, Linux, & FreeBSD are supported. openSeaChest has also been compiled and run on Solaris/Illumos distributions, but are not tested or updated as frequently. Please open an issue if you are unable to build on any of these systems.

The openSeaChest tools have been built for the UEFI shell as well, but this is not currently supported due to inconsistencies in support from UEFI passthrough drivers across manufacturers.

Compilers or operating systems not mentioned in this file may be supported as well using similar build instructions. The openSeaChest tools and opensea-*libs should be compatible with a C99 compliant compiler.

## Table Of Contents

[C99 Features Used](#C99-Features-Used)

[Linux](#linux)

* [Building with a different compiler](#building-with-a-different-compiler)

[FreeBSD/Solaris/Illumos](#FreeBSD/Solaris/Illumos)

[Windows](#Windows)

* [Option #1 - Microsoft tools](#Option-#1---Microsoft-tools)
  * [Visual Studio](#visual-studio)
  * [msbuild](#msbuild)
* [Option #2 - MSYS2 with Open Source GNU Tools](#option-#2---msys2-with-open-source-gnu-tools)
  * [MSYS2 and MinGW setup](#msys2-and-mingw-setup)
  * [Installation of the MSYS2 and MinGW environments](#installation-of-the-msys2-and-mingw-environments)
    * [Update MSYS2](#update-msys2)
    * [Install a compiler toolchain](#install-a-compiler-toolchain)
    * [Installing git](#installing-git)
    * [Set up $PATH](#set-up-\$path)
    * [Set MSYS2 to Run as administrator](#set-msys2-to-run-as-administrator)
    * [Building openSeaChest with MinGW](#building-openSeaChest-with-mingw)

[UEFI](#UEFI)

* [Build System Info](#Build-System-Info)
* [Checking out EDK2 and EDK2-libc](#Checking-out-EDK2-and-EDK2-libc)
* [Setting up the EDK2 Build Environment](#Setting-up-the-EDK2-Build-Environment)
* [Executing the UEFI Build](#Executing-the-UEFI-Build)
* [Running OpenSeaChest UEFI Tools](#Running-OpenSeaChest-UEFI-Tools)

[Documentation](#documentation)

[Platforms](#platforms)

[Git Instructions](#git-instructions)

* [Use git to pull the files from the Seagate openSeaChest repository on Github](#use-git-to-pull-the-files-from-the-seagate-openseachest-repository-on-github)
  * [Clone the openSeaChest projects](#clone-the-openseachest-projects)
    * [Note about single-branch option](#note-about-single-branch-option)
* [Folder Structure](#folder-structure)

## C99 Features Used

This is a brief list of the known features used from C99 that a compiler should support in order to compie the openSeaChest tools:

* Declare variables anywhere within scope:

      int myFunc()
      {
          int delareAtBeg = 0;
          // code
          // more code
          int newVariable = 3;
          // more code
      }

* Declare variable within for loop declaration
      for (int i = 1; ...
* C99 integer Types
      #include <inttypes.h>
      //or
      #include <stdint.h>

      //to use the following types:
      uint8_t, uint16_t, uint32_t, uint64_t
      int8_t, int16_t, int32_t, int64_t

* C99 boolean types
      #include <stdbool.h>

* C99 printf & scanf format macros
      PRIu64, SCNu64, etc

* C++ style comments
      //C++ style comment

## Linux

Required Tools:

* gcc or clang
* make

From a terminal, change directory to `openSeaChest/Make/gcc`

Run the command "make release" to build the release version of the openSeaChest tools.
After compilation is complete, the tools will be output into a subfolder named `openseachest_exes`

### Building with a different compiler

To build with clang (or any other compiler), specify with `CC=compilerName`.
Building this way allows for using cross-compilers that are available in some distributions, or with other compilers such as Clang.

Example: `make release CC=clang`

## FreeBSD/Solaris/Illumos

Required Tools:

* gcc
* gmake

From a terminal, change directory to `openSeaChest/Make/gcc`

Run the command `gmake release` to build the release version of the openSeaChest tools.
After compilation is complete, the tools will be output into a subfolder named `openseachest_exes`

Similar to Linux, it is possible to specify a different compiler with `CC=compilerName`

## Windows

### Option #1 - Microsoft tools

Your system will require the latest [Microsoft Visual C++ 2017 Redistributable](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) to run the compiled openSeaChest tools.

Required Tools:

* Visual Studio 2015 or 2017 (can also use msbuild)
* Visual Studio 2013 support has been deprecated
* Windows 10 SDK version 10.0.14393.0 for Visual Studio 2015
* Windows 10 SDK version 10.0.15063.0 for Visual Studio 2017 (x86 and x86_84)
* Windows 10 SDK version 10.0.16299.0 for Visual Studio 2017 (ARM and ARM64)

#### Visual Studio

  Open the solution file in "openSeaChest/Make/VS.(version)/openseachest.sln".
  Set the desired build configuration.
  Press "F7" to build all of the tools and libraries, or select "Build->Build All" from the menu.
  The tools will be output into `openSeaChest/Make/VS.(version)/(platform)/(build type)`.

  Example: `openSeaChest/Make/VS.2017/x64/Release`

#### msbuild

  From the developer command prompt for the version of visual studio that installed msbuild, change directory to "openSeaChest/Make/VS.(version)".

  Build with the command `msbuild /p:Configuration=(build type) /p:Platform=(platform)`.

  Example: `msbuild /p:Configuration=Release /p:Platform=x64`

  Available Platforms:

* Win32
* x64
* ARM (VS2017 only)
* ARM64 (VS2017 only)

Available Build Types:

* Release
* Debug
* Static-Release
* Static-Debug

### Option #2 - MSYS2 with Open Source GNU Tools

We support the [MSYS2](https://github.com/msys2/msys2/wiki/MSYS2-introduction) with [MinGW](http://www.mingw.org/) development environments.  Make files are located under the individual ./Make/gccWin folders.

#### MSYS2 and MinGW setup

**MSYS2** is software distribution and a building platform for Windows. It provides a Unix-like environment, a command-line interface and a software repository making it easier to install, use, build and port software on Windows.  See [https://github.com/msys2/msys2/wiki/MSYS2-introduction](https://github.com/msys2/msys2/wiki/MSYS2-introduction)

**MinGW** provides a complete Open Source programming tool set which is suitable for the development of native MS-Windows applications, and which do not depend on any 3rd-party C-Runtime DLLs.  See [http://www.mingw.org/](http://www.mingw.org/)

You are encouraged to do additional study on the MSYS2 and MinGW applications, and on Linux commands in general if you are new to the subject.  There are many wiki, FAQ and supporting discussion forums on all them.  Search on 'setup MSYS2 MinGW

#### Installation of the MSYS2 and MinGW environments

Download the MSYS2 installation setup files from [https://www.msys2.org/](https://www.msys2.org/) (about 75MB)
We'll use `http://repo.msys2.org/distrib/x86_64/msys2-x86_64-20180531.exe` for this discussion.

* Run the setup and set your installation folder.  We'll use `C:\msys64`  
* Set the Windows Start Menu name, we'll use `MSYS2 64bit`  
* Finish with the box checked `Run MSYS2 64-bit now`

The MSYS2 terminal window opens to the `~` folder, this is the traditional Linux user home folder.  Your corresponding Windows folder for `~` is `C:\msys64\home\<user name>`

##### Update MSYS2

First update the new MSYS2 the package database and core system packages by running this command at the command prompt:  

`pacman -Syu`   *(remember Linux commands are cAsE SeNsiTiVe)*

In case you are curious, pacman ("package manager") help shows this for the commands:  

    usage:  pacman {-S --sync} [options] [package(s)]
    options:
      -u, --sysupgrade     upgrade installed packages (-uu enables downgrades)
      -y, --refresh        download fresh package databases from the server
          --needed         do not reinstall up to date packages

You will probably see this message:  

    warning: terminate MSYS2 without returning to shell and check for updates again  
    warning: for example close your terminal window instead of calling exit

Just close the window by clicking the red X or with Alt+F4, the typical Windows close window command.
A dialog stating `Processes are running in session, Close anyway?` Click `OK`

Go to the `Windows Start Menu > All Programs > MSYS2 64bit > MSYS2 MinGW 64-bit`

At the command prompt run `pacman -Syu`  again.  (Hint: press the up arrow to retrieve the last command.)  This starts an update to the individual components and will take several minutes to complete.

Run `pacman -Syu`  again until it says there is nothing to do.

##### Install a compiler toolchain

a) for compiling 64-bit applications:  
`pacman -S --needed base-devel mingw-w64-x86_64-toolchain`

b) for compiling 32-bit applications:  
`pacman -S --needed base-devel mingw-w64-i686-toolchain`

This starts the retrieval and installation of the base development components (probably more than 100 small individual components) and will take several minutes to complete.

##### Installing Git

This step is optional, however you may wish to install `git` and perform the cloning, pulling, etc from the MSYS2 environment. To be able to do this, you must install `git` with the command below.

Install git on MSYS2 with `pacman -S --needed git`

##### Set up $PATH

Update the path via 'Environment Variables' from the `Windows Control Panel > System > Advanced System Settings > Environment Variables > (User variables) Path > Edit`
At the end, add  

    ;C:\msys64\usr\bin;C:\msys64\mingw64\bin

#### Set MSYS2 to Run as administrator

Because openSeaChest needs to access the system hardware the Windows operating system will need your special permission to allow that to happen.  From the Windows Start menu open the MSYS2 64bit folder, right click on the environment you want, like MSYS2 MinGW 64-bit,  Select `Properties > Shortcut tab > Advanced > and check Run as administrator > OK > Apply > OK`

NOTE: This step is only necessary if you wish to run the openSeaChest tools from the MSYS2 command prompt.

#### Building openSeaChest with MinGW

Assuming you are in the `~` home directory, `cd` to `./openSeaChest-develop/Make/gccWin`
or `./openSeaChest-MinGW/Make/gccWin` whichever you used.  Run:

    make -f Makefile.gccWin
This should build all of the openSeaChest tools as defined by the variable *BUILD_ALL*.  To quickly see which tools are defined, you can run:  

    make -f Makefile.gccWin print-BUILD_ALL
You will find the newly compiled executable files in the `./openseachest_exes/ folder`.

### UEFI

This section will document the process that was used to build the EFI shell binary versions of openSeaChest. Please note that this is not actively maintained and there are many issues trying to run these tools on a variety of systems.

#### Build System Info

To build the EFI binaries, use Ubuntu with thebuild-essential package installed. This has been done on Ubuntu 18.04 and Ubuntu 20.04 versions. This will likely work on other systems too, but they have not been tested.

#### Checking out EDK2 and EDK2-libc

To build the tools, you will need to check out EDK2 and EDK2-libc. Please note that the EDK2 project has removed the insecure string functions from EDK2 and libc no longer builds. So in order to build, you will need to check out an older tag.

First clone EDK2:

    git clone -b edk2-stable202005 https://github.com/tianocore/edk2.git

Next, clone the EDK2 libc fork from Seagate:

    git clone -b openseachest https://github.com/Seagate/edk2-libc.git

The Seagate fork includes additions to the EDK2 libc package that has getopt_long and C99 style printf formatting macros (and others from inttypes.h).

#### Setting up the EDK2 Build Environment

Refer to [this EDK2 getting started documentation](https://github.com/tianocore/tianocore.github.io/wiki/Using-EDK-II-with-Native-GCC) to setup the system, then [this documentation](https://github.com/tianocore/tianocore.github.io/wiki/Common-instructions) to build the basetools.

Once the basetools are setup, you must get the EDK2 directories setup to build openSeaChest. There is some EDK2 documentation that describes a split folder build environment, but this did not seem to work as expected, so this documentation copies the necessary folders to the right place.

From the edk2-libc checkout, copy the StdLib and StdLibPrivateInternalFiles folders to the EDK2 directory.

Now, it's time to copy the openSeaChest and opensea-*libs to the EDK2 directory. This has been consolidated into a single bash shell script to keep this simple.
Navigate to `openSeaChest/Make/UEFI` and execute the script providing the path to the EDK2 directory. Relative paths are ok.
Example:

    ./copy_files.sh /this/is/the/path/to/edk2/checkout

The last thing required before executing the build is to set the target for EDK2. Open the file `target.txt` which can be found in `edk2/Conf` in your editor of choice.

Set `ACTIVE_PLATFORM` to `openSeaChestPkg/openSeaChestPkg.dsc`, then set the `TARGET` to `RELEASE` for a release build or set to `DEBUG` if you want a debug build.
`TARGET_ARCH` should be set to the intended build target. NOTE: This requires setting the correct cross-compiler if targetting another CPU architecture. At this time, only `X64` has been used as that is what has been able to be tested.
`TOOL_CHAIN_TAG` should be set to `GCC5`, but if cross-compiling this may need to be set differently.

#### Executing the UEFI Build

Once everything is setup, it's time to build. Some of the following steps may have already been performed when following EDK2's build setup intructions mentioned earlier.

Set the base-tools directory:

    export EDK_TOOLS_PATH=$HOME/src/edk2/BaseTools

Call the EDK2 setup script:

    bash$ . edksetup.sh BaseTools

From the EDK2 directory, call build:

    build

The build will run, and as long as there are no errors, the output files will be found in the following directory:

    edk2/Build/openSeaChestPkg/<TARGET>_<TOOL_CHAIN_CONFIG>/<TARGET_ARCH>

If using the setting recommended earlier, this will be:

    edk2/Build/openSeaChestPkg/RELEASE_GCC5/X64

#### Running OpenSeaChest UEFI Tools

All openSeaChest UEFI tools have been run through the EFI shell. If you are using a system with secureboot enabled, you must either disable secure boot, or have the tools signed and follow the chain of trust...or add your own signature to the tools and to your UEFI systems list of keys. This process is outside the scope of this documentation.

### Documentation

Header files & functions have doxygen documentation.

### Platforms

Under Linux this library can be built on the following platforms using
a cross platform compiler:

        aarch64
        alpha
        arm
        armhf
        hppa
        m68k
        mips
        mips64
        mips64el
        mipsel
        powerpc
        powerpc64
        powerpc64le
        s390x
        sh4
        x86
        x86_64

This project can be built under Windows Visual Studio 2013 & 2015 solution
files for x86 and x64 targets.

## Git Instructions

**Git** is a distributed version-control system for tracking changes in source code during software development. The openSeaChest projects are maintained on GitHub which follows traditional Git architecture.

[GitHub](https://github.com/) is a Git repository hosting service, but it adds many of its own features. While Git is a command line tool, GitHub provides a Web-based graphical interface. Seagate maintains a [projects repository](https://github.com/seagate) on GitHub.

The following is the list of repositories that will be cloned for openSeaChest:

     https://github.com/Seagate/openSeaChest
     sub-modules:
     https://github.com/Seagate/opensea-common
     https://github.com/Seagate/opensea-operations
     https://github.com/Seagate/opensea-transport

Make a project folder off of the home folder or anywhere else you would like, a name something like `mygitstuff`, it is your choice.

### Use git to pull the files from the Seagate openSeaChest repository on GitHub

 Start by listing all of the available openSeaChest branches:  

     git ls-remote --heads https://github.com/Seagate/openSeaChest.git | sed 's?.*refs/heads/??'

 A short list of branches similar to this is returned:  

* develop  *(<<-- our default branch)*
* feature/VMWare-Support
* feature/minGW_Support
* master

#### Clone the openSeaChest projects

 **Examples:**

 1. Clone the develop branch:  

        git clone --recurse-submodules --branch develop https://github.com/Seagate/openSeaChest.git openSeaChest-develop

 2. Clone a feature branch:  

        git clone --recurse-submodules --branch feature/minGW_Support https://github.com/Seagate/openSeaChest.git openSeaChest-MinGW

 3. Make sure the recursive sub-module projects (opensea-common etc) are completely cloned from the specific branch.  `cd` into the new branch folder (openSeaChest-develop or openSeaChest-MinGW from the examples above)
 Run:  

        git submodule foreach --recursive 'git checkout develop'  
        or git submodule foreach --recursive 'git checkout feature/minGW_Support'

 Later, if you want to pull the newest updates, then change directory to your top branch folder where you can **inspect and backup your current changes** (if you made any) with:  

     git status
     git submodule foreach 'git status'

     git stash
     git submodule foreach 'git stash'

 Then get the latest openSeaChest files  

        git pull
        git pull --recurse-submodules

##### Note about single-branch option

 If the `--single-branch` option is used during the initial clone, fetching submodules to the head of a specific branch as in step 3 (above) may fail until this has been "undone".
 To undo a `--single-branch` pull for the submodules, perform the following:

     git submodule foreach 'git config remote.origin.fetch refs/heads/*:refs/remotes/origin/*'
     git submodule foreach 'git fetch'

 At this point, `git branch -a` should show all available branches that can be pulled for a given repository, or in this case submodule. From here, you can now recursively pull the branch of interest like this:

     git submodule foreach --recursive 'git checkout branch-of-interest'

### Folder Structure

Once cloned, the folder structure is similar to below for where to find the makefiles

        openSeaChest
          |_ Make
          |  |_ gcc
          |  |_ gccWin
          |_ opensea-common
          |  |_ Make
          |    |_ gcc
          |    |_ gccWin
          |_ opensea-operations
          |  |_ Make
          |    |_ gcc
          |    |_ gccWin
          |_ opensea-transport
             |_ Make
               |_ gcc
               |_ gccWin
