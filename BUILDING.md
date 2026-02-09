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

openSeaChest supports two build systems:

1. **Meson** (recommended) - Modern, cross-platform, well-documented
2. **GNU Make** - Maintained for backwards compatibility and platforms where Meson is unavailable

**We recommend using Meson** for new builds. The GNU Make build system is provided for:
- Platforms where Meson is not available or not working
- Backwards compatibility with existing build workflows
- Environments where installing Python (required for Meson) is not feasible
- Supporting platforms the meson does not currently support (ex: AIX, HP-UX)

### Building with Meson (Recommended)

Meson is a modern build system with excellent cross-platform support and comprehensive documentation.

**Quick Start:**

      # Basic build
      meson setup --buildtype=release builddir
      ninja -C builddir

      # Parallel build (faster)
      ninja -C builddir -j$(nproc)

      # Install system-wide
      sudo ninja -C builddir install

The tools will be in `builddir/` after building.

**Build Types:**

      meson setup --buildtype=release builddir      # Optimized (-O2, default)
      meson setup --buildtype=debug builddir        # Debug (-O0 -g)
      meson setup --buildtype=debugoptimized builddir  # Debug + optimization (-O2 -g)
      meson setup --buildtype=minsize builddir      # Minimize size (-Os)

**Build Options:**

      # Enable JSON support
      meson setup -Denable_json=true builddir

      # Use different compiler
      CC=clang CXX=clang++ meson setup builddir

      # Custom installation prefix
      meson setup --prefix=/usr builddir

      # Old GCC (< 5.5.0) requiring C99
      meson setup -Dc_std=gnu99 builddir

**Cross-Compilation:**

We provide **38 pre-configured cross-compilation files** in `meson_crosscompile/` for various architectures:

*MUSL-based cross-compilers (portable, static):*
- ARM: `aarch64`, `armv5l`, `armv6`, `armv7l`, `armv7r`
- x86: `i486`, `i686`, `x86_64`
- MIPS: `mips`, `mips64`, `mipsel`, `mips64el`
- PowerPC: `powerpc`, `powerpc64`, `powerpc64le`
- Others: `riscv64`, `s390x`

*Ubuntu-based cross-compilers:*
- Various architectures: `aarch64`, `armhf`, `ppc`, `ppc64le`, `riscv64`, `s390x`, `sparc64`, etc.

*Windows:*
- `Windows-Clang.txt` - Clang on Windows
- `msvc_arm64.txt` - MSVC ARM64

**Example cross-compilation:**

      # Using MUSL cross-compiler for ARM
      meson setup --cross-file meson_crosscompile/aarch64-linux-musl-cross.txt buildarm

      # Using Ubuntu cross-compiler
      meson setup --cross-file meson_crosscompile/ubuntu_ppc64le.txt buildppc

**Tips and Troubleshooting:**

*Faster Builds:*

      ninja -C builddir -j$(nproc)    # Use all CPU cores

*Optimization Flags:*

      # Maximum optimization (may increase size)
      meson setup -Dc_args='-O3 -march=native' builddir

      # Link-time optimization
      meson setup -Db_lto=true builddir

*Debugging:*

      # Verbose build output
      ninja -C builddir -v

      # Introspect configuration
      meson introspect builddir --buildoptions
      meson configure builddir

*Reconfigure existing build:*

      meson configure builddir --buildtype=debug
      ninja -C builddir

*Common Issues:*
- Meson requires Python 3.6+ - install with `pip3 install meson ninja`
- For detailed cross-compilation docs, see: https://mesonbuild.com/Cross-compilation.html
- For all Meson options and features, see: https://mesonbuild.com/

### Building with GNU Make

The GNU Make build system is maintained for backwards compatibility and platforms where Meson is unavailable.

**Note:** We recommend using Meson for new builds. Use Make only when:
- Meson is not available on your platform
- You cannot install Python (required for Meson)
- You have existing Make-based workflows

**Requirements:**
* GNU Make 3.81 or later
* GCC 4.7+ or Clang 3.0+
* For JSON support (optional): CMake and json-c library

**Quick Start:**

      # Basic build (auto-detects platform, builds release)
      make

      # Parallel build using all CPU cores
      make -j$(nproc)

      # Install to system (default: /usr/local)
      sudo make install

      # Create distribution tarball
      make package

**Build Configuration:**

      # Build types
      make BUILD_TYPE=debug           # Debug build (-g -O0)
      make BUILD_TYPE=release         # Release build (-O2, default)
      make BUILD_TYPE=static-debug    # Static debug build
      make BUILD_TYPE=static-release  # Static release build

      # Optional features
      make BUILD_JSON=1               # Enable JSON output support
      make BUILD_SHARED_LIBS=1        # Build shared libraries (.so)

      # Platform override (for cross-compilation)
      make PLATFORM=windows CC=x86_64-w64-mingw32-gcc
      make PLATFORM=freebsd

      # Custom installation prefix
      make install PREFIX=/opt/seagate

**Available Targets:**

      make all                 # Build all libraries and tools (default)
      make libraries           # Build only opensea-* libraries
      make tools               # Build only openSeaChest utilities
      make check / test        # Run basic smoke tests
      make clean               # Clean current build type
      make clean-all           # Clean all build types
      make mostlyclean         # Clean objects and tools (keep libraries)
      make distclean           # Remove all generated files including packages
      make install             # Install binaries, libraries, headers
      make install-strip       # Install with debug symbols stripped
      make installdirs         # Create installation directories only
      make uninstall           # Remove installed files
      make package             # Create tar.gz distribution
      make show-config         # Display build configuration
      make print-tools         # List all tools to build
      make print-libs          # List all libraries to build
      make help                # Show all available targets

**Build Type Shortcuts:**

      make release             # Optimized build (-O3, default)
      make debug               # Debug build (-O0 -g3)
      make relwithdebinfo      # Optimized with debug symbols (-O2 -g)
      make minsize             # Minimize binary size (-Os)
      make static-release      # Static release build
      make static-debug        # Static debug build

**Individual Library Targets:**

      make opensea-common      # Build opensea-common library only
      make opensea-transport   # Build opensea-transport library only
      make opensea-operations  # Build opensea-operations library only
      make opensea-jsonformat  # Build opensea-jsonformat (if BUILD_JSON=1)

**Individual Tool Targets:**

All 18 openSeaChest utilities can be built individually:

      make openSeaChest_Basics
      make openSeaChest_Configure
      make openSeaChest_Defect
      make openSeaChest_Erase
      make openSeaChest_Firmware
      make openSeaChest_Format
      make openSeaChest_GenericTests
      make openSeaChest_Info
      make openSeaChest_Logs
      make openSeaChest_NVMe
      make openSeaChest_PassthroughTest
      make openSeaChest_PowerControl
      make openSeaChest_Raw
      make openSeaChest_Reservations
      make openSeaChest_Security
      make openSeaChest_SMART
      make openSeaChest_ZBD

**Supported Platforms:**
* Linux (all architectures)
* Windows (MinGW/MSYS2)
* FreeBSD, OpenBSD, NetBSD, DragonFlyBSD
* Solaris, Illumos
* AIX
* VMware ESXi (requires VMware DDK)

**Build System Features:**
* Automatic dependency tracking (`.d` files)
* Parallel build support (`make -j`)
* Cross-platform support with automatic detection
* OpenSSF security hardening (stack protection, RELRO, NX, PIE)
* Compiler warning flags from Meson build (50+ warnings)
* Platform-specific optimizations

**Directory Structure:**

Build outputs are organized by platform and configuration:

      build/
      └── linux-x86_64-release/
          ├── bin/           # openSeaChest utilities
          ├── lib/           # Static/shared libraries
          └── obj/           # Object files and .d dependencies
              ├── common/
              ├── transport/
              ├── operations/
              └── tools/

**Examples:**

      # Development build with verbose output
      make BUILD_TYPE=debug V=1

      # Optimized static build
      make BUILD_TYPE=static-release -j$(nproc)

      # Build with JSON support and install to /usr
      make BUILD_JSON=1 PREFIX=/usr install

      # Cross-compile for Windows from Linux
      make PLATFORM=windows CC=x86_64-w64-mingw32-gcc

      # Package for distribution
      make BUILD_TYPE=static-release package
      # Creates: openSeaChest-<VERSION>-<PLATFORM>-<ARCH>.tar.gz

**Tips and Troubleshooting:**

**Faster Builds:**

Use parallel compilation to significantly reduce build time:

      # Use all CPU cores
      make -j$(nproc)

      # Use specific number of cores
      make -j8

**Debugging Build Failures:**

Enable verbose output to see full compiler commands:

      make V=1                    # See all compiler commands
      make V=1 BUILD_TYPE=debug   # Verbose debug build

Check build configuration:

      make show-config            # Display current settings
      make print-tools            # List tools to build
      make print-libs             # List libraries to build

**Cross-Compilation:**

When cross-compiling, explicitly set compiler and platform:

      # MinGW cross-compile from Linux
      make PLATFORM=windows \
           CC=x86_64-w64-mingw32-gcc \
           CXX=x86_64-w64-mingw32-g++ \
           AR=x86_64-w64-mingw32-ar

      # Install MinGW toolchain on Ubuntu/Debian:
      sudo apt-get install mingw-w64

**Platform-Specific Notes:**

*Solaris/OmniOS:* Explicitly set compiler if needed:

      # Use GCC on Solaris
      make CC=gcc CXX=g++

      # Some linker flags may not be supported
      # The build system tests flags automatically

*BSD Systems:* Default compiler is usually Clang:

      # FreeBSD/OpenBSD/NetBSD auto-detect Clang
      make

      # Force GCC if installed
      make CC=gcc CXX=g++

*Windows (MinGW/MSYS2):* Use the appropriate shell:

      # From MSYS2 MinGW64 shell
      make

      # From MSYS2 UCRT64 shell
      make

**Common Issues:**

1. **Compiler Warnings (Make vs Meson):**
   - Some warnings may appear in Make builds but not Meson builds (or vice versa)
   - This is typically due to differences in how compilers are invoked
   - Example: `safe_strlen()` may warn about string overread checks in Make builds, but not in meson, however this is guarded within the C code properly and may not be detected by the compiler properly in some situations.
   - These are usually false positives - the code is safely handled
   - If Meson builds without warnings but Make shows them, it's likely not a real issue
   - The code is tested with both build systems and compiler warning configurations
   - If you do find an error or bug, or can contribute a way to fix these warnings, please open a pull request!

2. **Case-Sensitive Headers (MinGW cross-compile):**
   - Windows headers in MinGW are lowercase (e.g., `windows.h`, `basetsd.h`)
   - Fixed in codebase but useful to know if adding new Windows code

3. **Missing Compiler:**
   - Install build tools: `apt install build-essential` (Debian/Ubuntu)
   - Install gcc/clang: `pkg install gcc` (Solaris/BSD)

3. **Linker Errors:**
   - The build system automatically tests linker flags
   - Unsupported flags are skipped on incompatible platforms

4. **Submodule Issues:**
   - Ensure submodules are initialized: `git submodule update --init --recursive`

5. **Stale Build:**
   - Clean and rebuild: `make clean-all && make`

**Performance Tuning:**

      # Minimal size (embedded systems)
      make BUILD_TYPE=minsize

      # Maximum optimization (may increase size)
      make CFLAGS="-O3 -march=native"

      # Link-time optimization (LTO)
      make CFLAGS="-flto" LDFLAGS="-flto"


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
