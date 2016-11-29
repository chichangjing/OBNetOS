
#ifndef __CMD_MISC_H
#define __CMD_MISC_H

void CmdReset(int argc, char **argv);
void CmdDumpMemory(int argc, char **argv);
void CmdDumpEeprom(int argc, char **argv);
void CmdSetBootDelay(int argc, char **argv);
void CmdSetConsoleSwitch(int argc, char **argv);
void CmdClearEeprom(int argc, char **argv);

void RegisterMiscCommand(void);

#endif

