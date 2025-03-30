@echo off
@rem This file finds and adds the msvc values to env

for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set InstallDir=%%i
)

if exist "%InstallDir%\VC\Auxiliary\Build\vcvarsall.bat" (
  call "%InstallDir%\VC\Auxiliary\Build\vcvarsall.bat" amd64
  @echo on
  echo "Exporting PATH to ADDPATH_ENV"
  echo ADDPATH_ENV=%PATH% >> %GITHUB_ENV%
  set ADDPATH_ENV=%PATH%
  echo "Exporting finished"
)