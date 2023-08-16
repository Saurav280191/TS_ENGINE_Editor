@echo off

REM Copying Assets folder to build 

xcopy "Assets" "build\x64\debug\Assets" /E /I /Y
xcopy "Assets" "build\x64\debug\bin\Assets" /E /I /Y

REM Copying Resources folder to build 
xcopy "Resources" "build\x64\debug\Resources" /E /I /Y
xcopy "Resources" "build\x64\debug\bin\Resources" /E /I /Y

PAUSE