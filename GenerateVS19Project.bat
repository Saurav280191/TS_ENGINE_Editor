@echo off

REM Build Clone and build TS_ENGINE
cd ..
echo Cloning TS_ENGINE...
git clone https://github.com/Saurav280191/TS_ENGINE.git
timeout /t 5 >nul
echo TS_ENGINE cloned complete.
cd TS_ENGINE
git pull origin main
git checkout TS_ENGINE_Editor

REM Build Assimp
echo Building assimp
cd Dependencies\include\assimp
call Build_vs2019_x64.bat
cd ..\..\..\..\

REM Build TS_ENGINE_Editor
cd TS_ENGINE_Editor
REM Generate Solution
cmake -G "Visual Studio 16 2019" -A=x64 -B=./build/x64/Debug -DCMAKE_ARCHITECTURE=x64 -DCMAKE_BUILD_TYPE=Debug 
REM Build Binaries
cmake --build build/x64/Debug --config Debug

REM Post build commands
copy "..\TS_ENGINE\Dependencies\include\assimp\build\x64\bin\Debug\assimp-vc142-mtd.dll" "build\x64\Debug\bin"

REM Copy Assets And Resources
call CopyDataToBin.bat