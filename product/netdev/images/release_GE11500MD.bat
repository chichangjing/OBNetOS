@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GE11500MD_Bootloader.bin
copy /B Firmware.bin GE11500MD_Firmware.bin
obhead.exe GE11500MD.cfg
