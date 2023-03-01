#!/bin/sh

#This script will install a musl cross compiler from http://musl.cc/#refs
#This will only install within the meson_crosscompile directory at this time.
#NOTE: Not all cross-compilers listed will be installable by this script
#      Expand this list as necessary for additional compilers needed
#      Some CPUs have variations between softfloat (sf) and hardfloat (hf)
#      This currently installs only a couple variations that openSeaChest has
#      been used with in the past (before it was opensourced)

#possible improvements to this script:
#     selecting multiple CPU architectures to download at once
#     selectable directory to install to
#     other websites for cross-compilers (IBM has one for powerpc)

usage() { 
    echo "Usage: $0 [-a <cpu architecture>]" 1>&2; 
    echo "    use -a to specify a CPU architecture for musl cross compiler to install."
    echo "    All compilers will be installed under /opt/muslcc/<compiler_full_package_name>"
    echo "    Below is a list of accepted cpu architecutres for this script"
    echo "         aarch64"
    #echo "         aarch64_be"
    echo "         armv5l"
    echo "         armv5lhf"
    echo "         armv6"
    echo "         armv6hf"
    echo "         armv7l"
    #echo "         armv7m"
    echo "         armv7r"
    echo "         i486"
    echo "         i686"
    echo "         mips"
    echo "         mips64"
    echo "         mipsel"
    echo "         mips64el"
    echo "         powerpc"
    echo "         powerpcle"
    echo "         powerpc64"
    echo "         powerpc64le"
    echo "         riscv64"
    echo "         s390x"
    echo "         x86_64"
}

if [ "$(id -u)" -ne 0 ]; then
    echo "You must run this script as root"
    exit 1
fi

installdir=/opt/muslcc
cpuarch=""

while getopts ":a:h" o; do
    case "${o}" in
        a)
            cpuarch=${OPTARG}
            ;;
        h)
            usage
            exit 0
            ;;
        *)
            usage
            exit 2
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${cpuarch}" ] ; then
    usage
    exit 3
fi

mkdir -p "$installdir"

#Use curl to get the requested compiler
#NOTE: This is the most common form of cross-compiler name
requestedcompiler="$cpuarch"-linux-musl-cross.tgz

#handle special cases for cpu arch to modify the requested compiler name to match the website
#this was done to keep arguments to this script simpler
if [ "${cpuarch#*arm}" != "$cpuarch" ]; then
    #Check the requested ARM type and change the requested compiler to the correct name
    if [ "$cpuarch" = "armv5l" ]; then
        requestedcompiler=armv5l-linux-musleabi-cross.tgz
    elif [ "$cpuarch" = "armv5lhf" ]; then
        requestedcompiler=armv5l-linux-musleabihf-cross.tgz
    elif [ "$cpuarch" = "armv6" ]; then
        requestedcompiler=armv6-linux-musleabi-cross.tgz
    elif [ "$cpuarch" = "armv6hf" ]; then
        requestedcompiler=armv6-linux-musleabihf-cross.tgz
    elif [ "$cpuarch" = "armv7l" ]; then
        requestedcompiler=armv7l-linux-musleabihf-cross.tgz
    elif [ "$cpuarch" = "armv7r" ]; then
        requestedcompiler=armv7r-linux-musleabihf-cross.tgz
    fi
fi

#TODO: check for curl and wget and error if neither are found
cd /opt/muslcc && curl -O https://musl.cc/"$requestedcompiler"

#use tar to extract the compiler
tar zxf "$requestedcompiler"

#cleanup the tar.gz since it has been extracted
rm -f "$requestedcompiler"
