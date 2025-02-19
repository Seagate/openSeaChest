::This is a prebuild script to execute the cmake build for json-c
:: script inputs: 
::	1. generator name
:: 	2. platform architecture
::	3. FieldDiagUtils root folder
::  4. Visual Studio version number (14.0, 15.0, 16.0, etc). NOTE: If using VS2015 build tools from VS2019 the macro used does not work! May need to hardcode this in the project files :(
::  5. Visual Studio project config
:: this is to echo the pwd to the screen if you are debugging using this batch file-> echo %cd%
:: -DCMAKE_POLICY_DEFAULT_CMP0091=NEW is NECESSARY for static builds to do the correct build. Do not remove this.
::
:: Things to improve: Detect when json-c build is already built and up to date to skip building again
:: Figure out how to use  -DCMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION_MAXIMUM=ver to match win10 API in vs 2017 and 2015 to match those projects
:: 
set string=%~1
:: Replace every hyphen in string variable with a space
set string=%string:-= %
:: Detecting static build
set staticstring=%~5
echo %staticstring%|find "Static" > nul
if errorlevel 1 (
	:: Performing regular build 
	echo "regular build"
	call cmake -G "%string%" -A "%~2" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DDISABLE_EXTRA_LIBS=ON -DBUILD_TESTING=OFF -DDISABLE_BSYMBOLIC=ON -S %~3\subprojects\json-c\ -B "%~3\subprojects\json-c\build-%~4-%~2-%~5"
) else (
	:: Performing static build 
	echo "static build"
	call cmake -G "%string%" -A "%~2" -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" -DBUILD_SHARED_LIBS=OFF -DDISABLE_EXTRA_LIBS=ON -DBUILD_TESTING=OFF -DDISABLE_BSYMBOLIC=ON -S %~3\subprojects\json-c\ -B "%~3\subprojects\json-c\build-%~4-%~2-%~5"
)
call cmake --build "%~3\subprojects\json-c\build-%~4-%~2-%~5" --config Release
