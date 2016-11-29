@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GE2C400U_Bootloader.bin
copy /B Firmware.bin GE2C400U_Firmware.bin
obhead.exe GE2C400U.cfg
