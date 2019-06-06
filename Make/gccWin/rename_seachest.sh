#!/bin/bash
#
#=============== Global Variables ============================================
# Date last edit
EDIT_STAMP="02-Feb-2019"
TITLE="SeaChest Utilities Rename Script"

#example below of ANSI quoting
WHITEonBLUE=$'\e[1;37m\e[44m'
WHITEonGREEN=$'\e[1;37m\e[42m'
WHITEonRED=$'\e[1;37m\e[41m'
BLACKonBLUE=$'\e[1;30m\e[44m'
BLACKonGREEN=$'\e[1;30m\e[42m'
BLACKonRED=$'\e[1;30m\e[41m'
YELLOW=$'\e[1;33m'

RESETCOLOR=$'\e[0m'

# Details for current app to check
CUR_APP_ARRAY=""
CUR_APP_NAME=""
CUR_APP_VER=""
CUR_APP_LIB=""
CUR_APP_DATE=""
CUR_APP_REV=""
CUR_APP_ARCH=""
CUR_APP_OS=""

#=============== Function Declarations ========================================
function repecho() {
  # draws the separator line
  local rep_count=$1;
  local rep_char=$2;

  for ((i=1; i <= $rep_count; i++)); do
    echo -n "$rep_char";
  done
  echo;
}

#------------------------------------------------------
function populate_current_app_data() {
  local app=$1;
  local verbose_ver="";

  if [ "$UNAME" == "Linux" ] && [ -n "$WINE" ] ; then
    CUR_APP_ARRAY=($(wine $app --version --verbose 0));
  else
    CUR_APP_ARRAY=($($app --version --verbose 0));
  fi
  
  if [[ $? -eq 127 ]]; then
     echo "Application $app not found";
     exit 5;
  fi

  #printf '%s\n' "${CUR_APP_ARRAY[@]}"

  ((n_elements=${#CUR_APP_ARRAY[@]}, max_index=n_elements - 1));
  for ((i = 0; i <= max_index; i++)); do
     if [ ${CUR_APP_ARRAY[i]} = "Library" ]; then
        verbose_ver=OLD;
     fi
  done

  if [[ "$verbose_ver" = "OLD" ]]; then
     #echo "Library found!! OLD version";
     CUR_APP_NAME=${CUR_APP_ARRAY[3]};
     CUR_APP_NAME=($(echo "${CUR_APP_NAME//:}"));  #two slashes is parameter expansion, if a pattern begins with / all matches are replaced with string, if string is null then matches are deleted and the / following pattern may be omitted.
     CUR_APP_VER=${CUR_APP_ARRAY[6]};
     CUR_APP_LIB=${CUR_APP_ARRAY[9]};
     CUR_APP_LIB=($(echo "$CUR_APP_LIB" | tr '.' '_'));
     CUR_APP_DATE=${CUR_APP_ARRAY[12]}"-"${CUR_APP_ARRAY[13]}"-"${CUR_APP_ARRAY[14]};
     CUR_APP_REV=$CUR_APP_VER-$CUR_APP_LIB;
     CUR_APP_ARCH=${CUR_APP_ARRAY[17]};
     CUR_APP_OS=${CUR_APP_ARRAY[31]};
  else
     #echo "Library not found!! NEW version";
     CUR_APP_NAME=${CUR_APP_ARRAY[3]};
	 #echo "$CUR_APP_NAME";
     CUR_APP_NAME=($(echo "${CUR_APP_NAME//:}"));
	 #echo "$CUR_APP_NAME";
     CUR_APP_VER=${CUR_APP_ARRAY[6]};
     CUR_APP_LIB=${CUR_APP_ARRAY[12]};
     #echo "------$CUR_APP_LIB";
     CUR_APP_LIB=($(echo "$CUR_APP_LIB" | tr '.' '_'));
     #echo "------$CUR_APP_LIB";
     CUR_APP_DATE=${CUR_APP_ARRAY[18]}"-"${CUR_APP_ARRAY[19]}"-"${CUR_APP_ARRAY[20]};
     CUR_APP_REV=$CUR_APP_VER-$CUR_APP_LIB;
     CUR_APP_ARCH=${CUR_APP_ARRAY[23]};
     CUR_APP_OS=${CUR_APP_ARRAY[37]};
  fi
}

#=============== Main =========================================================
#clear;
#echo $0 $1;
#echo "";
#echo "$TITLE                               ver: $EDIT_STAMP";
#repecho 40 -=;
#echo "";

# Don't do rename if we're cross-compiling under Linux and wine is not installed
UNAME="$(uname)"
WINE="$(command -v wine)"
if [ "$UNAME" == "Linux" ] && [ -z "$WINE" ] ; then
  exit 0
fi

if [[ $# -ne 1  || ( $1 == "--help" || $1 == "-help" || $1 == "-h") ]]; then
  echo -ne "Usage:   "$WHITEonBLUE"./rename_seachest.sh"$RESETCOLOR" "$BLACKonGREEN"<SeaChest app name>"; echo -e $RESETCOLOR
  echo "";
  echo -ne "Example: "$WHITEonBLUE"./rename_seachest.sh"$RESETCOLOR" "$WHITEonRED"SeaChest_Basics"; echo -e $RESETCOLOR
  echo "";
  exit 1;
fi

sc_app=$1;
#echo $sc_app;
dir_sc_app=$(echo "${sc_app%/*}"); #some fancy parameter expansion to extract relative path only
name_sc_app=$(echo "${sc_app##*/}");
#echo $dir_sc_app;
#echo $name_sc_app;

populate_current_app_data $sc_app;

#   nothing works.. sad
#MYTAB='       ';
#echo "      .$CUR_APP_REV.      .$CUR_APP_OS.      .$CUR_APP_ARCH.      .$CUR_APP_DATE";
#echo -e "$MYTAB"."$CUR_APP_REV"."$MYTAB"."$CUR_APP_OS"."$MYTAB"."$CUR_APP_ARCH"."$MYTAB"."$CUR_APP_DATE";
#echo -e \t.$CUR_APP_REV.\t.$CUR_APP_OS.\t.$CUR_APP_ARCH.\t.$CUR_APP_DATE;
#APP_INFO="'      '+${CUR_APP_REV}+'      '+$CUR_APP_OS+'      '+$CUR_APP_ARCH+'      '+$CUR_APP_DATE";

echo $CUR_APP_NAME;
#echo $CUR_APP_OS;
#echo $CUR_APP_ARCH;
#echo $CUR_APP_VER;
#echo $CUR_APP_LIB;
#echo ------------------------

#NAME_APP_NAME=($(echo "${sc_app//.exe}")); #remove .exe if there
NAME_APP_NAME=$dir_sc_app/$CUR_APP_NAME;
NAME_APP_REV=($(echo "${CUR_APP_VER//.}")); #remove periods
NAME_APP_LIB=($(echo "${CUR_APP_LIB//_}")); #remove underscores
if [[ "$CUR_APP_ARCH" =~ "_64" ]]; then
   #echo "64";
   NAME_APP_ARCH="64"
elif [[ "$CUR_APP_ARCH" =~ "X86" ]]; then
   #echo "32";
   NAME_APP_ARCH="32"
fi
#echo $NAME_APP_ARCH;
NEW_APP_NAME="$NAME_APP_NAME"_"$NAME_APP_REV"_"$NAME_APP_LIB"_"$NAME_APP_ARCH";
if [[ "$CUR_APP_OS" =~ "Windows" ]]; then
   #echo "Windows";
   NEW_APP_NAME+=.exe
fi
UNAME=`uname -s`
if [[ "$UNAME" == "Linux" ]] ; then
	NEWER_APP_NAME=`echo $NEW_APP_NAME | sed 's/\\r//g'`;
	NEW_APP_NAME="$NEWER_APP_NAME"
fi
echo "old file name: $sc_app";
echo "new file name: $NEW_APP_NAME"
cp -vp $sc_app $NEW_APP_NAME

