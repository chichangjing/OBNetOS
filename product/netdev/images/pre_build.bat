@echo off
cd /d %~dp0
set SVN_PATH=C:\Program Files\TortoiseSVN\bin
"%SVN_PATH%\SubWCRev.exe" ..\..\..\ ..\firmware\svn_revision.template ..\firmware\svn_revision.h

