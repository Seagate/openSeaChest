#!/bin/bash

#This script generates all the openSeaChest help documents. It assembles tool unique and general documents into place, and generates the HTML help document.
#Some parts as assembled within temporary folders in order to keep output clean.

if ! command -v enscript > /dev/null 2>&1
then
    echo "You must install enscript to generate documentation"
fi

if [ "$#" -lt 1 ]; then
    echo "You must provide a path to the openSeaChest executables"
fi

exeDir="$1"

generatorDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

#this is a list of ALL of the tools. This is used as a "Base" to generate all of the docs for these tools
openSeaChest_doc_list=("openSeaChest_Basics" "openSeaChest_Configure" "openSeaChest_Erase" "openSeaChest_Firmware" "openSeaChest_Format" "openSeaChest_GenericTests" "openSeaChest_Info" "openSeaChest_Logs" "openSeaChest_NVMe" "openSeaChest_PassthroughTest" "openSeaChest_PowerControl" "openSeaChest_Reservations" "openSeaChest_Security" "openSeaChest_SMART" "openSeaChest_ZBD")

#Now, we need to go through and generate the user guides for each tool.
#While most have a similar structure, some have different parts that are unique to each of them.
#So there are some custom rules depending on the file being generated.

mkdir -p Generated_openSeaChest_Documents

for tool in "${openSeaChest_doc_list[@]}"; do
    echo "-------------$tool-----------"
    if [ -z "$tool" ]; then
        continue
    fi

    textDoc="$generatorDir/Generated_openSeaChest_Documents/$tool.txt"
    
    if [ -f "$textDoc" ];then
        #remove the file if it already exists so we get a new clean one
        rm -f "$textDoc"
    fi
    
    touch "$textDoc"

    {
        #1. Add header
        printf "%-58s Revision: %s\r\n" "$tool" "$(date +"%d-%b-%Y")"
        printf "===============================================================================\r\n"
        printf " %s - drive utilities\r\n" "$tool"
        printf " Copyright (c) 2014-%s Seagate Technology LLC and/or its Affiliates, All Rights Reserved\r\n" "$(date +"%Y")"
        printf " See Version History below.\r\n"
        printf "===============================================================================\r\n"
        #2. Add $tool_About.txt
        cat "$generatorDir/openSeaChest/$tool""_About.txt"
        #3. Add result of $tool* --help (TODO: This causes an error in shell check. Need to rewrite this at some point to minimize any issues we might have in the future.
        "$exeDir/$tool"* --help
        #4. Add general usage hints for Windows and Linux
        cat "$generatorDir"/General_Docs/Tool_Usage_Hints.txt
        #6. Add generic usage hints
        cat "$generatorDir"/General_Docs/General_Usage_Hints_Linux.txt
        cat "$generatorDir"/General_Docs/General_Usage_Hints_Windows.txt
    } >> "$textDoc"
    
    #7. Add specific features (generated in if's below)
    specificHelp=()
    additionalLicenses=()

    #check tool name to handle custom tools
    if [ "$tool" = "openSeaChest_Basics" ]; then
        #need to add:
        # Win FWDL Restrictions
        specificHelp+=("Windows_Firmware_Download_Restrictions.txt")
    elif [ "$tool" = "openSeaChest_Erase" ]; then
        #need to add:
        # About format Unit
        # about fast format
        # interpretting head health status
        # about ATA security frozen
        # enabling TCG in linux
        specificHelp+=("About_Format_Unit.txt")
        specificHelp+=("About_Fast_Format.txt")
        specificHelp+=("Interpretting_Head_Health.txt")
        specificHelp+=("About_ATA_Security_Frozen.txt")
        specificHelp+=("Enabling_TCG_Commands_In_Linux.txt")
    elif [ "$tool" = "openSeaChest_Firmware" ]; then
        #need to add:
        # Win FWDL Restrictions
        specificHelp+=("Windows_Firmware_Download_Restrictions.txt")
    elif [ "$tool" = "openSeaChest_Format" ]; then
        #need to add:
        # About format Unit
        # about fast format
        specificHelp+=("About_Format_Unit.txt")
        specificHelp+=("About_Fast_Format.txt")
        specificHelp+=("Interpretting_Head_Health.txt")
    elif [ "$tool" = "openSeaChest_GenericTests" ]; then
        #need to add:
        # About bad sector
        specificHelp+=("About_Bad_LBAs.txt")
    elif [ "$tool" = "openSeaChest_PowerControl" ]; then
        #need to add:
        # About Power Choice
        specificHelp+=("About_PowerChoice.txt")
    elif [ "$tool" = "openSeaChest_Security" ]; then
        #need to add:
        # About ATA frozen
        # enable TCG
        specificHelp+=("About_ATA_Security_Frozen.txt")
        specificHelp+=("Enabling_TCG_Commands_In_Linux.txt")
    elif [ "$tool" = "openSeaChest_SMART" ]; then
        #need to add:
        # About bad sector
        specificHelp+=("About_Bad_LBAs.txt")
    fi

    if [ ${#specificHelp[@]} -gt 0 ]; then
        for help in "${specificHelp[@]}"; do
            if [ -z "$help" ]; then
                #this is needed because the index is left empty after deletions which is annoying.
                continue
            fi
            cat "$generatorDir/Feature_Docs/$help" >> "$textDoc"
            
            #cleanup/remove from the list once processed. If this is not done, then it continually grows and will not work as expected
            specificHelp=("${specificHelp[@]/$help}")
        done
    fi

    {
        #8. Add sample output (TODO: Tool unique sample output)
        cat "$generatorDir"/General_Docs/Sample_Output.txt
        #9. Add $tool_version_history
        cat "$generatorDir/openSeaChest/$tool""_Version_History.txt"
        #10. Add About openSeaChest Tools
        cat "$generatorDir"/General_Docs/About_openSeaChest_Tools.txt
        #11. Add Opensource Licenses
        cat "$generatorDir"/General_Docs/Open_Source_Licenses.txt
    } >> "$textDoc"
    
    
    #12. Add additional licenses (generated from above)
    if [ ${#additionalLicenses[@]} -gt 0 ]; then
        for license in "${additionalLicenses[@]}"; do
            if [ -z "$license" ]; then
                #this is needed because the index is left empty after deletions which is annoying.
                continue
            fi
            #insert a separator before each new license
            printf "===========================================================================\r\n" >> "$textDoc"
            cat "$generatorDir/General_Docs/$license" >> "$textDoc"
            
            #cleanup/remove from the list once processed. If this is not done, then it continually grows and will not work as expected
            additionalLicenses=("${additionalLicenses[@]/$license}")
        done
    fi
    
    #run unix2dos on the text file to make sure it is readable in both linux and windows

done

#Now we need to generate the combined text help
#First do the intro text/header
combinedDoc="$generatorDir/Generated_openSeaChest_Documents/openSeaChest_Combo_UserGuides.txt"
if [ -f "$combinedDoc" ];then
    #remove the file if it already exists so we get a new clean one
    rm -f "$combinedDoc"
fi

touch "$combinedDoc"
{
    printf "All openSeaChest Utilities User Guides\r\n\r\n"
    printf "Date last edit: %s\r\n\r\n" "$(date +"%d-%b-%Y")"
    printf "About openSeaChest Utilities Command Line Diagnostics and Open Source Statement.\r\n\r\n"
    printf "openSeaChest_Basics\r\n"
    printf "openSeaChest_Configure\r\n"
    printf "openSeaChest_Erase\r\n"
    printf "openSeaChest_Firmware\r\n"
    printf "openSeaChest_Format\r\n"
    printf "openSeaChest_GenericTests\r\n"
    printf "openSeaChest_Info\r\n"
    printf "openSeaChest_Logs\r\n"
    printf "openSeaChest_NVMe\r\n"
    printf "openSeaChest_PassthroughTest\r\n"
    printf "openSeaChest_PowerControl\r\n"
    printf "openSeaChest_Reservations\r\n"
    printf "openSeaChest_Security\r\n"
    printf "openSeaChest_SMART\r\n\r\n"
    printf "openSeaChest_ZBD\r\n\r\n"
    printf "Tool Usage Hints\r\n"
    printf "Linux General Usage Hints\r\n"
    printf "Windows General Usage Hints\r\n\r\n"
    printf "About ATA Security Frozen\r\n"
    printf "About Bad LBAs (Sectors)\r\n"
    printf "About Format Unit\r\n"
    printf "About Fast Format\r\n"
    printf "About PowerChoice\r\n"
    printf "Enabling TCG Commands In Linux\r\n"
    printf "Interpretting Head Health\r\n\r\n"
    printf "Windows Firmware Download Restrictions\r\n"
    printf "Sample Output\r\n"
    printf "About openSeaChest Diagnostics\r\n\r\n"
    printf "END USER LICENSE AGREEMENT\r\n"
    printf "Various Open Source Licenses\r\n\r\n"
} >> "$combinedDoc"

#now, need to go through the tools and output the: header, about, help, version_history
openSeaChest_combodoc_list=("openSeaChest_Basics" "openSeaChest_Configure" "openSeaChest_Erase" "openSeaChest_Firmware" "openSeaChest_Format" "openSeaChest_GenericTests" "openSeaChest_Info" "openSeaChest_Logs" "openSeaChest_NVMe" "openSeaChest_PassthroughTest" "openSeaChest_PowerControl" "openSeaChest_Reservations" "openSeaChest_Security" "openSeaChest_SMART" "openSeaChest_ZBD")
#additionally, create some files in a temporary place that will be used by enscript
enscriptFolder="enscriptFiles"
mkdir -p "$enscriptFolder"
for combotool in "${openSeaChest_combodoc_list[@]}"; do
    if [ -z "$combotool" ]; then
        continue
    fi
    enscriptToolDoc="enscriptFiles/$combotool"
    touch "$enscriptToolDoc"
    {
        printf "%-58s Revision: %s\r\n" "$combotool" "$(date +"%d-%b-%Y")"
        printf "===============================================================================\r\n"
        printf " %s - Seagate drive utilities\r\n" "$combotool"
        printf " Copyright (c) 2014-%s Seagate Technology LLC and/or its Affiliates, All Rights Reserved\r\n" "$(date +"%Y")"
        printf " See Version History below.\r\n"
        printf "===============================================================================\r\n"
        cat "$generatorDir/openSeaChest/$combotool""_About.txt"
        #3. Add result of $tool* --help (TODO: This causes an error in shell check. Need to rewrite this at some point to minimize any issues we might have in the future.
        "$exeDir/$combotool"* --help
        cat "$generatorDir/openSeaChest/$combotool""_Version_History.txt"
    } | tee "$enscriptToolDoc" >> "$combinedDoc"
    #the pip to tee above should split the output to the 2 different files we need...one for enscript and one for the combined text document
done
#now append the other general help files and specific features files and licenses
{
    cat "$generatorDir"/General_Docs/Tool_Usage_Hints.txt
    cat "$generatorDir"/General_Docs/General_Usage_Hints_Linux.txt
    cat "$generatorDir"/General_Docs/General_Usage_Hints_Windows.txt
    
    cat "$generatorDir"/Feature_Docs/About_ATA_Security_Frozen.txt
    cat "$generatorDir"/Feature_Docs/About_Bad_LBAs.txt
    cat "$generatorDir"/Feature_Docs/About_Format_Unit.txt
    cat "$generatorDir"/Feature_Docs/About_Fast_Format.txt
    cat "$generatorDir"/Feature_Docs/About_PowerChoice.txt
    cat "$generatorDir"/Feature_Docs/Enabling_TCG_Commands_In_Linux.txt
    cat "$generatorDir"/Feature_Docs/Interpretting_Head_Health.txt
    cat "$generatorDir"/Feature_Docs/Windows_Firmware_Download_Restrictions.txt

    cat "$generatorDir"/General_Docs/Sample_Output.txt
    cat "$generatorDir"/General_Docs/About_openSeaChest_Tools.txt
    cat "$generatorDir"/General_Docs/Open_Source_Licenses.txt
} >> "$combinedDoc"

#copy all the other files to the enscript folder
#this command is comlicated as we are removing file extensions and changing the underscores to spaces all at the same time!
for file in "$generatorDir/General_Docs/"*; do
    #file will contain the fully qualitied path, so need to keep that in mind for adjusting the copy command.
    #NOTE: This command may be able to be simplified. It may also be possible to use bash substitution to simplify this...but I am leaving this as it may work better if this
    #      script ever needs converting to /bin/sh versus currently using /bin/bash - TJE
    #      https://github.com/koalaman/shellcheck/wiki/SC2001
    cp "$file" "$enscriptFolder/$(echo "$(echo "$(basename "$file")" | cut -f 1 -d '.')" | sed 's/_/ /g')"
done

for file in "$generatorDir/Feature_Docs/"*; do
    #file will contain the fully qualitied path, so need to keep that in mind for adjusting the copy command.
    #NOTE: This command may be able to be simplified. It may also be possible to use bash substitution to simplify this...but I am leaving this as it may work better if this
    #      script ever needs converting to /bin/sh versus currently using /bin/bash - TJE
    #      https://github.com/koalaman/shellcheck/wiki/SC2001
    cp "$file" "$enscriptFolder/$(echo "$(echo "$(basename "$file")" | cut -f 1 -d '.')" | sed 's/_/ /g')"
done

#need to concatenate extra licenses to the file in the enscript folder before continuing to renaming
{
    cat "$generatorDir"/General_Docs/Open_Source_Licenses.txt
} >> "$enscriptFolder/ Open Source Licenses"

#We are almost ready to call enscript...we need to create the list of files in an array to give to enscript
cd "$enscriptFolder" || exit
enscriptFileList=("About openSeaChest Tools")
#enumerate and add all created openSeaChest documents
for openSeaChestDoc in "openSeaChest_"*; do
    enscriptFileList+=("$(basename "$openSeaChestDoc")")
done
#Now add remaining known documents
#NOTE: There is probably a better way to do this by using the loops above to create lists, but then they may not be in order....-TJE
enscriptFileList+=("Tool Usage Hints" "General Usage Hints Linux" "General Usage Hints Windows" "About ATA Security Frozen" "About Bad LBAs" "About Format Unit" "About Fast Format" "About PowerChoice" "Enabling TCG Commands In Linux" "Interpretting Head Health" "Windows Firmware Download Restrictions" "Sample Output" "Open Source Licenses")

#Finally generate the combined html help
enscript --toc -whtml -p"$generatorDir"/Generated_openSeaChest_Documents/openSeaChest_Combo_UserGuides.html "${enscriptFileList[@]}"

cd "$generatorDir" || exit

#cleanup temp enscript directory
rm -rf "$enscriptFolder"
