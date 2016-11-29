@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GE1040PU_Bootloader.bin
copy /B Firmware.bin GE1040PU_Firmware.bin
obhead.exe GE1040PU.cfg
