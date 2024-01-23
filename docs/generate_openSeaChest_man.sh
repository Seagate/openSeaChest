#!/bin/bash

#This script generates all the openSeaChest man pages. 

if ! command -v help2man > /dev/null 2>&1
then
    echo "You must install help2man to generate man pages"
fi

if [ "$#" -lt 1 ]; then
    echo "You must provide a path to the openSeaChest executables"
fi

exeDir="$1"

generatorDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

#this is a list of ALL of the tools. This is used as a "Base" to generate all of the docs for these tools
openSeaChest_doc_list=("openSeaChest_Basics" "openSeaChest_Configure" "openSeaChest_Erase" "openSeaChest_Firmware" "openSeaChest_Format" "openSeaChest_GenericTests" "openSeaChest_Info" "openSeaChest_Logs" "openSeaChest_NVMe" "openSeaChest_PassthroughTest" "openSeaChest_PowerControl" "openSeaChest_Reservations" "openSeaChest_Security" "openSeaChest_SMART" "openSeaChest_ZBD")

for tool in "${openSeaChest_doc_list[@]}"; do
    echo "-------------$tool-----------"
    if [ -z "$tool" ]; then
        continue
    fi

    textDoc="$generatorDir/man/man8/$tool.8"
    
    if [ -f "$textDoc" ];then
        #remove the file if it already exists so we get a new clean one
        rm -f "$textDoc"
    fi
    
    #run help2man for each utility
    help2man -s8 -N -n="drive utilities" -o "$textDoc" "$exeDir/$tool* --noBanner"

done
