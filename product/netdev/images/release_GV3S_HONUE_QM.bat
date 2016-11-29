@echo off
cd /d %~dp0
rm -f *_Firmware.* *_Bootloader.*
copy /B Bootloader.bin GV3S_HONUE_QM_Bootloader.bin
copy /B Firmware.bin GV3S_HONUE_QM_Firmware.bin
obhead.exe GV3S_HONUE_QM.cfg
