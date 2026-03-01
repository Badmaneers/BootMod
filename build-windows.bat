@echo off
setlocal EnableDelayedExpansion

set VCPKG_TOOLCHAIN=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
set VCPKG_TRIPLET=x64-windows

echo [1/4] Building CLI...
cmake -S . -B build-cli-win ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN% ^
    -DVCPKG_TARGET_TRIPLET=%VCPKG_TRIPLET%
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build build-cli-win --config Release --parallel
if %errorlevel% neq 0 exit /b %errorlevel%

echo [2/4] Building GUI...
cmake -S gui -B build-gui-win ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%Qt6_DIR%" ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN% ^
    -DVCPKG_TARGET_TRIPLET=%VCPKG_TRIPLET%
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build build-gui-win --config Release --parallel
if %errorlevel% neq 0 exit /b %errorlevel%

echo [3/4] Staging output...
mkdir dist-win
copy build-cli-win\Release\bootmod.exe dist-win\
copy build-gui-win\Release\bootmod-gui.exe dist-win\
windeployqt --qmldir gui\qml --release dist-win\bootmod-gui.exe
if %errorlevel% neq 0 exit /b %errorlevel%

echo [4/4] Copying vcpkg runtime DLLs...
copy C:\vcpkg\installed\x64-windows\bin\zlib1.dll dist-win\ 2>nul
copy C:\vcpkg\installed\x64-windows\bin\libpng16.dll dist-win\ 2>nul

echo Done.
endlocal
