
#ifndef __CMD_BDINFO_H
#define __CMD_BDINFO_H

void CmdMacSet(int argc, char **argv);
void CmdBdinfoSet(int argc, char **argv);
void CmdBdinfoGet(int argc, char **argv);
void CmdBdinfoClear(int argc, char **argv);

void RegisterFactoryCommand(void);
#endif

