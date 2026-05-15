@echo off
setlocal

if "%VCPKG_ROOT%"=="" (
    echo ERROR: VCPKG_ROOT is not set.
    echo Example:
    echo setx VCPKG_ROOT C:\vcpkg
    exit /b 1
)

cmake --preset vs2022-x64-static-debug
