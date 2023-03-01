Name:           OpenSeaChest
Version:        1
Release:        1%{?dist}
Summary:        Storage Command Line Tool. 
License:        GPL  
URL:            http://www.seagate.com  
Source0:        OpenSeaChest-1.0.tar.gz 
BuildArch:      noarch
#BuildRequires:  
#Requires:       
Vendor:         Seagate Technologies
Packager:       Lingaraj 


%description
OpenSeaChest is used to manage storage controllers.

%prep
%setup -q
%install
install -m 0755 -d $RPM_BUILD_ROOT/opt/OpenSeaChest
install -m 0755 openSeaChest_Info  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_Info
install -m 0755 openSeaChest_Basics  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_Basics
install -m 0755 openSeaChest_Configure  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_Configure
install -m 0755 openSeaChest_Erase  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_Erase
install -m 0755 openSeaChest_Firmware  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_Firmware
install -m 0755 openSeaChest_Format  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_Format
install -m 0755 openSeaChest_GenericTests  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_GenericTests
install -m 0755 openSeaChest_NVMe  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_NVMe
install -m 0755 openSeaChest_PowerControl  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_PowerControl
install -m 0755 openSeaChest_SMART  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_SMART
install -m 0755 openSeaChest_ZBD  $RPM_BUILD_ROOT/opt/OpenSeaChest/openSeaChest_ZBD



%files
/opt/OpenSeaChest
/opt/OpenSeaChest/openSeaChest_Info
/opt/OpenSeaChest/openSeaChest_Basics
/opt/OpenSeaChest/openSeaChest_Configure
/opt/OpenSeaChest/openSeaChest_Erase
/opt/OpenSeaChest/openSeaChest_Firmware
/opt/OpenSeaChest/openSeaChest_Format
/opt/OpenSeaChest/openSeaChest_GenericTests
/opt/OpenSeaChest/openSeaChest_NVMe
/opt/OpenSeaChest/openSeaChest_PowerControl
/opt/OpenSeaChest/openSeaChest_SMART
/opt/OpenSeaChest/openSeaChest_ZBD

%doc

%changelog
* Mon Jul 23 2018 Lingaraj Marapli 1.0.0
  - Initial rpm release

