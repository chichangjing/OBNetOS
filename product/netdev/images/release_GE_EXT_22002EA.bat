@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GE_EXT_22002EA_Bootloader.bin
copy /B Firmware.bin GE_EXT_22002EA_Firmware.bin
obhead.exe GE_EXT_22002EA.cfg
