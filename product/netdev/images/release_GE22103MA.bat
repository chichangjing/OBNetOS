@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GE22103MA_Bootloader.bin
copy /B Firmware.bin GE22103MA_Firmware.bin
obhead.exe GE22103MA.cfg
