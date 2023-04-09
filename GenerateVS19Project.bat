@echo off
 
echo Cloning TS_ENGINE...
git clone https://github.com/Saurav280191/TS_ENGINE.git
echo TS_ENGINE cloned complete.
timeout /t 5 >nul
cd TS_ENGINE
git checkout tags/v1.1
cd TS_ENGINE
call GenerateVS19Project.bat
cd ..

REM Generate Solution
cmake -G "Visual Studio 16 2019" -A=x64 -B=./build/x64/Debug -DCMAKE_ARCHITECTURE=x64 -DCMAKE_BUILD_TYPE=Debug
REM Build Binaries
cmake --build build/x64/Debug --config Debug

copy "TS_ENGINE\Dependencies\include\assimp\build\x64\bin\Debug\assimp-vc143-mtd.dll" "build\x64\Debug\bin"

xcopy "Assets" "build\x64\debug\Assets" /E /I /Y
xcopy "Assets" "build\x64\debug\bin\Assets" /E /I /Y

PAUSE