@echo off
REM =======================================================
REM Post-build script to create BIN file from IAR output (.out)
REM Argument %1 is the full path to the final IAR output file ($TARGET_PATH$)
REM =======================================================

SET "TARGET_FILE=%~1"

REM --- 로그 파일 설정 ---
SET "LOG_FILE=%~dp0post_build.log"

REM --- 1. Set IAR Toolkit Path ---
SET "IAR_BIN_DIR=C:\Program Files (x86)\IAR Systems\Embedded Workbench 8.4\arm\bin"

REM --- 2. Set Output File Path (same name as .out) ---
SET "OUTPUT_BIN_FILE=%TARGET_FILE:.out=.bin%"

REM --- 3. OUTPUT_BIN_FILE 경로에서 파일명과 디렉토리 추출 ---
FOR %%F IN ("%OUTPUT_BIN_FILE%") DO (
  SET "BIN_NAME=%%~nxF"
  SET "BIN_DIR=%%~dpF"
)

REM --- 4. 로그 및 명령 실행 ---

echo =======================================================
echo [BIN Export] Creating final BIN file
echo Input File : %TARGET_FILE%
echo Output File: %OUTPUT_BIN_FILE%
echo Output Dir : "%BIN_DIR%"
echo =======================================================

REM --- 4.1 Execute ielftool (ELF/OUT → BIN) ---
"%IAR_BIN_DIR%\ielftool.exe" "%TARGET_FILE%" --bin "%OUTPUT_BIN_FILE%"

REM --- 4.2 Check ielftool Result ---
IF %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] BIN file created successfully.
) ELSE (
    echo [FAILURE] ielftool.exe failed with ERRORLEVEL %ERRORLEVEL%.
)

REM =======================================================
REM 5. Execute make_fw.exe
REM =======================================================
pushd "%BIN_DIR%"
make_fw.exe "%BIN_NAME%" 0x188
IF %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] make_fw.exe executed successfully.
) ELSE (
    echo [FAILURE] make_fw.exe failed with ERRORLEVEL %ERRORLEVEL%.
)
popd

echo =======================================================
echo [Post-build completed]
echo =======================================================


exit 0
