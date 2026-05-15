@echo off
setlocal

if "%VCPKG_ROOT%"=="" (
    echo ERROR: VCPKG_ROOT is not set.
    exit /b 1
)

cmake --preset vs2022-x64-static-release
