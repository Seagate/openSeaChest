# Building openSeaChest

openSeaChest tools can be built for a variety of platforms.
Windows, Linux, & FreeBSD are supported. openSeaChest has also been compiled and run on Solaris/Illumos distributions, but are not tested or updated as frequently. Please open an issue if you are unable to build on any of these systems.

The openSeaChest tools have been built for the UEFI shell as well, but this is not currently supported due to inconsistencies in support from UEFI passthrough drivers across manufacturers. To build these see [BUILDING_OLD.md](BUILDING_OLD.md#UEFI)

Compilers or operating systems not mentioned in this file may be supported as well using similar build instructions. The openSeaChest tools and opensea-*libs should be compatible with a C99 compliant compiler.

Be sure to perform a recursive clone to ensure all submodules are pulled and properly initialized. If the submodules are missing, the build will fail since these must be compiled to build the openSeaChest tools.
Example:

      git clone --recursive <https://github.com/Seagate/openSeaChest.git>

## Table Of Contents

[C99 Features Used](#c99-features-used)

[Build Requirements](#build-requirements)

* [Linux/FreeBSD/Solaris/Illumos](#linuxfreebsdsolarisillumos)

* [Windows](#windows)


[Building](#building)

[Documentation](#documentation)

[Platforms](#platforms)

[Git Instructions](#git-instructions)

* [Use git to pull the files from the Seagate openSeaChest repository on Github](#use-git-to-pull-the-files-from-the-seagate-openseachest-repository-on-github)
  * [Clone the openSeaChest projects](#clone-the-openseachest-projects)
    * [Note about single-branch option](#note-about-single-branch-option)
* [Folder Structure](#folder-structure)

## C99 Features Used

This is a brief list of the known features used from C99 that a compiler should support in order to compile the openSeaChest tools:

* Declare variables anywhere within scope:

      int myFunc()
      {
          int declareAtBeg = 0;
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

## Build Requirements

### Windows

* A C compiler. Either:
  * Clang or MSVC from [Visual Studio](https://visualstudio.microsoft.com/downloads/)
  * [MSYS2/MinGW](https://www.msys2.org/wiki/MSYS2-introduction/)
* [meson](https://github.com/mesonbuild/meson/releases)
* Use either the Visual Studio Command Prompt or MinGW shell (or make sure you can run MinGW gcc from your shell) depending on your chosen compiler

### Linux/FreeBSD/Solaris/Illumos

* A C compiler such as clang or gcc
* meson and ninja

## Building

Run the following commands:

      meson --buildtype=release builddir
      ninja -C builddir

The tools will be in the builddir.
To install them system-wide, simply run `ninja -C builddir install` with appropriate permissions

To cross-compile, see [the official docs](https://mesonbuild.com/Cross-compilation.html)

To build with a different compiler, set the CC environment variable to your desired compiler.
Example:

      CC=clang meson --buildtype=release buildclang
      ninja -C buildclang

NOTE: If using GCC and older than [5.5.0](https://gcc.gnu.org/onlinedocs/gcc-4.9.4/gcc/Standards.html#Standards), the C standard defaults to gnu90. Add the flag `-Dc_std=gnu99` to build openSeaChest. This is needed for CentOS 7 which uses GCC [4.8.5](https://gcc.gnu.org/onlinedocs/gcc-4.8.5/gcc/Standards.html#Standards)
Example:

      meson --buildtype=release -Dc_std=gnu99 buildOldGCC
      ninja -C buildOldGCC

## Documentation

Header files & functions have doxygen documentation.

## Platforms

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

This project can be built for x86, x64 and arm targets on Windows.

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
