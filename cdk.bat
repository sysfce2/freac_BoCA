@echo off
set CDK_INSTALL_PATH=%CD%
set PATH=%CDK_INSTALL_PATH%\system\bin
set C_INCLUDE_PATH=%CDK_INSTALL_PATH%/system/include
set CPLUS_INCLUDE_PATH=%CDK_INSTALL_PATH%/system/include
set LIBRARY_PATH=%CDK_INSTALL_PATH%/system/lib

echo BonkEnc Component Development Kit v1.1 Beta 1
echo Copyright (C) 2001-2008 Robert Kausch

bash