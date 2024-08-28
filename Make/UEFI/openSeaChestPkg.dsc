## @file
#   openSeaChest development file for EDKII
#   This package contains applications which depend upon Standard Libraries
#   from the StdLib package.
#
#   See the comments in the [LibraryClasses.IA32] and [BuildOptions] sections
#   for important information about configuring this package for your
#   environment.
#
##
# SPDX-License-Identifier: MPL-2.0

[Defines]
  PLATFORM_NAME                  = openSeaChestPkg
  PLATFORM_GUID                  = dd2c189f-a31a-473d-bfe4-de8500b3bdff
  PLATFORM_VERSION               = 0.01
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/openSeaChestPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64|ARM|AARCH64|IPF
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

#
#  Debug output control
#
  DEFINE DEBUG_ENABLE_OUTPUT      = FALSE       # Set to TRUE to enable debug output
  DEFINE DEBUG_PRINT_ERROR_LEVEL  = 0x80000040  # Flags to control amount of debug output
  DEFINE DEBUG_PROPERTY_MASK      = 0

[PcdsFeatureFlag]

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|$(DEBUG_PROPERTY_MASK)
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|$(DEBUG_PRINT_ERROR_LEVEL)

[PcdsFixedAtBuild.IPF]

[LibraryClasses]
  #
  # Entry Point Libraries
  #
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  #
  # Common Libraries
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  !if $(DEBUG_ENABLE_OUTPUT)
    DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
    DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  !else   ## DEBUG_ENABLE_OUTPUT
    DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  !endif  ## DEBUG_ENABLE_OUTPUT

  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf

  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf

  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf

  LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library

  opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
  opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
  opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf

###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################

DEFINE  DISABLE_TCG_SUPPORT = 1

[Components]

####Each Utility needs a separate inf file
openSeaChestPkg/openSeaChest_Basics.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_Security.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_Configure.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_Erase.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_Firmware.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_Format.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_GenericTests.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_Logs.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_NVMe.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_PowerControl.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_Sample.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_SMART.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}
openSeaChestPkg/openSeaChest_ZBD.inf {
  <LibraryClasses>
    opensea-commonlib|opensea-libs/opensea-common/opensea-common.inf
    opensea-transportlib|opensea-libs/opensea-transport/opensea-transport.inf
    opensea-operationslib|opensea-libs/opensea-operations/opensea-operations.inf
    LibPosix|StdLib/PosixLib/PosixLib.inf   # Combines LibErr, LibGen, LibGlob, LibStringlist, GetPass into one library
}

##############################################################################
#
# Specify whether we are running in an emulation environment, or not.
# Define EMULATE if we are, else keep the DEFINE commented out.
#
# DEFINE  EMULATE = 1

##############################################################################
#
#  Include Boilerplate text required for building with the Standard Libraries.
#
##############################################################################
!include StdLib/StdLib.inc
