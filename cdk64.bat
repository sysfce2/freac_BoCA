@echo off
set CDK_INSTALL_PATH=%CD%
set PATH=%CDK_INSTALL_PATH%\system64\bin
set C_INCLUDE_PATH=%CDK_INSTALL_PATH%/system64/include
set CPLUS_INCLUDE_PATH=%CDK_INSTALL_PATH%/system64/include
set LIBRARY_PATH=%CDK_INSTALL_PATH%/system64/lib

set BUILD_WIN32=True
set BUILD_X64=True

echo fre:ac Component Development Kit v1.1 Beta 1 (x64)
echo Copyright (C) 2001-2010 Robert Kausch

bash