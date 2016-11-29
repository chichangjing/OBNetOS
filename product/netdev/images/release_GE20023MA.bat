@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GE20023MA_Bootloader.bin
copy /B Firmware.bin GE20023MA_Firmware.bin
obhead.exe GE20023MA.cfg
