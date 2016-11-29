@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GE204P0U_Bootloader.bin
copy /B Firmware.bin GE204P0U_Firmware.bin
obhead.exe GE204P0U.cfg
