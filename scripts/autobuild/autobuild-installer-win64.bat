@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"

mkdir "%TRAKTOR_HOME%/build/installer"
pushd "%TRAKTOR_HOME%/build/installer"

:: Create a file list of "android binaries".
::%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/bin/latest/android/releasestatic" -gg -sfrag -sreg -cg cmp_bin_latest_android -dr bin_latest_android -var var.TRAKTOR_WHERE -template fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Bin_Latest_Android_ReleaseStatic.wxs
::%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/bin/latest/android/releasestatic" Traktor_Bin_Latest_Android_ReleaseStatic.wxs

:: Create a file list of "linux binaries".
::%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/bin/latest/linux/releaseshared" -gg -sfrag -sreg -cg cmp_bin_latest_linux -dr bin_latest_linux -var var.TRAKTOR_WHERE -template fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Bin_Latest_Linux_ReleaseShared.wxs
::%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/bin/latest/linux/releaseshared" Traktor_Bin_Latest_Linux_ReleaseShared.wxs

:: Create a file list of "ios binaries".
::%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/bin/latest/ios/releasestatic" -gg -sfrag -sreg -cg cmp_bin_latest_ios -dr bin_latest_ios -var var.TRAKTOR_WHERE -template fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Bin_Latest_Ios_ReleaseStatic.wxs
::%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/bin/latest/ios/releasestatic" Traktor_Bin_Latest_Ios_ReleaseStatic.wxs

:: Create a file list of "osx binaries".
::%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/bin/latest/osx/releaseshared" -gg -sfrag -sreg -cg cmp_bin_latest_osx -dr bin_latest_osx -var var.TRAKTOR_WHERE -template fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Bin_Latest_Osx_ReleaseShared.wxs
::%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/bin/latest/osx/releaseshared" Traktor_Bin_Latest_Osx_ReleaseShared.wxs

:: Create a file list of "rpi binaries".
::%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/bin/latest/rpi/releaseshared" -gg -sfrag -sreg -cg cmp_bin_latest_rpi -dr bin_latest_rpi -var var.TRAKTOR_WHERE -template fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Bin_Latest_Rpi_ReleaseShared.wxs
::%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/bin/latest/rpi/releaseshared" Traktor_Bin_Latest_Rpi_ReleaseShared.wxs

:: Create a file list of "win64 binaries".
%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/bin/latest/win64/releaseshared" -gg -sfrag -sreg -cg cmp_bin_latest_win64 -dr bin_latest_win64 -var var.TRAKTOR_WHERE -template fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Bin_Latest_Win64_ReleaseShared.wxs
%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/bin/latest/win64/releaseshared" Traktor_Bin_Latest_Win64_ReleaseShared.wxs

:: Create a file list of "data assets".
%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/data/Assets" -gg -sfrag -sreg -cg cmp_data_assets -dr data -var var.TRAKTOR_WHERE -template fragment fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Data_Assets.wxs
%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/data/Assets" Traktor_Data_Assets.wxs

:: Create a file list of "data source".
%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/data/Source" -gg -sfrag -sreg -cg cmp_data_source -dr data -var var.TRAKTOR_WHERE -template fragment fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Data_Source.wxs
%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/data/Source" Traktor_Data_Source.wxs

:: Create a file list of "resources runtime".
%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/resources/runtime" -gg -sfrag -sreg -cg cmp_resources_runtime -dr resources -var var.TRAKTOR_WHERE -template fragment fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Resources_Runtime.wxs
%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/resources/runtime" Traktor_Resources_Runtime.wxs

:: Create a file list of "scripts runtime".
%TRAKTOR_HOME%/3rdp/wix-windows/heat dir "%TRAKTOR_HOME%/scripts/runtime" -gg -sfrag -sreg -cg cmp_scripts_runtime -dr scripts -var var.TRAKTOR_WHERE -template fragment fragment -t "%TRAKTOR_HOME%/code/Installer/Filter.xslt" -out Traktor_Scripts_Runtime.wxs
%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 -dTRAKTOR_WHERE="%TRAKTOR_HOME%/scripts/runtime" Traktor_Scripts_Runtime.wxs

:: Compile master install script.
%TRAKTOR_HOME%/3rdp/wix-windows/candle -arch x64 %TRAKTOR_HOME%/code/Installer/Traktor.wxs

:: Link as an installer.
%TRAKTOR_HOME%/3rdp/wix-windows/light ^
	-ext WixUIExtension ^
	Traktor.wixobj ^
	Traktor_Bin_Latest_Win64_ReleaseShared.wixobj ^
	Traktor_Data_Assets.wixobj ^
	Traktor_Data_Source.wixobj ^
	Traktor_Resources_Runtime.wixobj ^
	Traktor_Scripts_Runtime.wixobj ^
	-out Traktor.msi

popd
