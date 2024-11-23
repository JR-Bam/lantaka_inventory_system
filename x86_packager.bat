@echo off
set build_folder=Win32\Release\

set assets_folder=assets
set public_folder=public

set ims_exe_file=%build_folder%lantaka_inventory_system_x86.exe
set shutdown_exe_file=%build_folder%server_shutdown_x86.exe

set libcrypto_dll=%build_folder%libcrypto-3.dll
set libssl_dll=%build_folder%libssl-3.dll
set mysql_dll=%build_folder%mysqlcppconn8-2-vs14.dll
set libprotobuf_dll=%build_folder%libprotobuf-lite.dll
set lz4_dll=%build_folder%lz4.dll
set zlib_dll=%build_folder%zlib1.dll
set zstd_dll=%build_folder%zstd.dll


set all_files_exist=true

for %%f in (
    "%ims_exe_file%" 
    "%shutdown_exe_file%" 
    "%libcrypto_dll%" 
    "%libssl_dll%" 
    "%mysql_dll%"
    "%libprotobuf_dll%"
    "%lz4_dll%"
    "%zlib_dll%"
    "%zstd_dll%"
    ) do (
    if not exist %%f (
        echo File missing: %%f
        set all_files_exist=false
    )
)

if "%all_files_exist%" == "true" (
    echo All files exist.
) else (
    echo Some files are missing.
    goto exit
)

:prompt
echo Enter destination folder name:
set /p name=

set dest=%build_folder%%name%

if exist %dest% (
    echo Folder already exists.
    goto prompt
)

mkdir %dest%
echo Folder Created Successfully

xcopy "%assets_folder%" "%dest%\assets" /E /I /H /Y
xcopy "%public_folder%" "%dest%\public" /E /I /H /Y
xcopy "%ims_exe_file%" "%dest%" /Y
xcopy "%shutdown_exe_file%" "%dest%" /Y
xcopy "%libcrypto_dll%" "%dest%" /Y
xcopy "%libssl_dll%" "%dest%" /Y
xcopy "%mysql_dll%" "%dest%" /Y
xcopy "%libprotobuf_dll%" "%dest%" /Y
xcopy "%lz4_dll%" "%dest%" /Y
xcopy "%zlib_dll%" "%dest%" /Y
xcopy "%zstd_dll%" "%dest%" /Y

echo Files Copied Successfully

:exit
pause
