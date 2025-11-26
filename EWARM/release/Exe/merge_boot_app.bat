@echo off
setlocal enabledelayedexpansion

rem Find hex file in HJIR-610W_Boot folder
set BOOT_HEX=
for %%f in (HJIR-610W_Boot\*.hex) do (
    set BOOT_HEX=%%f
    goto :found
)

:found
if not defined BOOT_HEX (
    echo Error: No hex file found in HJIR-610W_Boot folder
    exit /b 1
)

rem Extract filename from path and remove HJIR-610W_ prefix
for %%i in ("%BOOT_HEX%") do set BOOT_FILENAME=%%~nxi
set OUTPUT_NAME=%BOOT_FILENAME:HJIR-610W_=merge_%

echo Boot hex file: %BOOT_HEX%
echo App hex file: aws_app.hex
echo Output file: %OUTPUT_NAME%
echo.

srec_cat.exe "%BOOT_HEX%" -Intel aws_app.hex -Intel -o "%OUTPUT_NAME%" -Intel -Output_BlockSize=16

if %errorlevel% equ 0 (
    echo.
    echo Merge succeeded: %OUTPUT_NAME% created
) else (
    echo.
    echo Merge failed
    exit /b 1
)

endlocal
