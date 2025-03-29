@echo off
rem This file finds and adds the msvc values to env

for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set InstallDir=%%i
)

if exist "%InstallDir%\VC\Auxiliary\Build\vcvars64.bat" (
  "%InstallDir%VC\Auxiliary\Build\vcvars64.bat"
)