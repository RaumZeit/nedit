/* $Id: userCmds.h,v 1.5 2002/07/11 21:18:12 slobasso Exp $ */

#ifndef NEDIT_USERCMDS_H_INCLUDED
#define NEDIT_USERCMDS_H_INCLUDED

#include "nedit.h"

void EditShellMenu(WindowInfo *window);
void EditMacroMenu(WindowInfo *window);
void EditBGMenu(WindowInfo *window);
void UpdateShellMenu(WindowInfo *window);
void UpdateMacroMenu(WindowInfo *window);
void UpdateBGMenu(WindowInfo *window);
char *WriteShellCmdsString(void);
char *WriteMacroCmdsString(void);
char *WriteBGMenuCmdsString(void);
int LoadShellCmdsString(char *inString);
int LoadMacroCmdsString(char *inString);
int LoadBGMenuCmdsString(char *inString);
int DoNamedShellMenuCmd(WindowInfo *window, const char *itemName, int fromMacro);
int DoNamedMacroMenuCmd(WindowInfo *window, const char *itemName);
int DoNamedBGMenuCmd(WindowInfo *window, const char *itemName);
void SetBGMenuUndoSensitivity(WindowInfo *window, int sensitive);
void SetBGMenuRedoSensitivity(WindowInfo *window, int sensitive);
void DimSelectionDepUserMenuItems(WindowInfo *window, int sensitive);
void DimPasteReplayBtns(int sensitive);

#endif /* NEDIT_USERCMDS_H_INCLUDED */
