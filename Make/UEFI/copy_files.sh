#!/bin/bash
# SPDX-License-Identifier: MPL-2.0
function usage {
    echo "This script will copy all files required to build openSeaChest to the edk2/UDK path specified."
    echo "Once the script is done copying, change the target.txt to build one of the following:"
    echo "    OpenSeaChestPkg/openSeaChestPkg.dsc"
    echo
    echo "How to use: copy_files.sh <path to edk2 directory>"
}

#check that we were given the correct number of parameters
if [ "$#" -lt 1 ]; then
    usage
fi
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
openSeaChestBaseDir=$(dirname "$(dirname "$scriptDir")")

#check that the directory exists
if [ -d "$1" ]; then
    mkdir -p "$1/openSeaChestPkg"
    openSeaChestPkgDir="$1/openSeaChestPkg"
    #start by copying the files in openSeaChest
    cp -r "$openSeaChestBaseDir/utils" "$openSeaChestPkgDir"
    cp -r "$openSeaChestBaseDir/include" "$openSeaChestPkgDir"
    cp -r "$openSeaChestBaseDir/src" "$openSeaChestPkgDir"
    #copy UEFI makefiles for openSeaChest
    cp -r "$openSeaChestBaseDir/Make/UEFI/openSeaChest"* "$openSeaChestPkgDir"
    #now copy opensea-libs
    "$openSeaChestBaseDir"/subprojects/opensea-common/Make/UEFI/copy_files.sh "$1"
    "$openSeaChestBaseDir"/subprojects/opensea-transport/Make/UEFI/copy_files.sh "$1"
    "$openSeaChestBaseDir"/subprojects/opensea-operations/Make/UEFI/copy_files.sh "$1"

else
    echo "Cannot find specified path: $1"
fi
