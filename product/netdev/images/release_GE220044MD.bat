@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GE220044MD_Bootloader.bin
copy /B Firmware.bin GE220044MD_Firmware.bin
obhead.exe GE220044MD.cfg
