/* $Id: userCmds.h,v 1.6 2004/03/04 09:44:21 tksoh Exp $ */

#ifndef NEDIT_USERCMDS_H_INCLUDED
#define NEDIT_USERCMDS_H_INCLUDED

#include "nedit.h"

void EditShellMenu(WindowInfo *window);
void EditMacroMenu(WindowInfo *window);
void EditBGMenu(WindowInfo *window);
void UpdateUserMenus(WindowInfo *window);
char *WriteShellCmdsString(void);
char *WriteMacroCmdsString(void);
char *WriteBGMenuCmdsString(void);
int LoadShellCmdsString(char *inString);
int LoadMacroCmdsString(char *inString);
int LoadBGMenuCmdsString(char *inString);
int DoNamedShellMenuCmd(WindowInfo *window, const char *itemName, int fromMacro);
int DoNamedMacroMenuCmd(WindowInfo *window, const char *itemName);
int DoNamedBGMenuCmd(WindowInfo *window, const char *itemName);
void RebuildAllMenus(WindowInfo *window);
void SetBGMenuUndoSensitivity(WindowInfo *window, int sensitive);
void SetBGMenuRedoSensitivity(WindowInfo *window, int sensitive);
void DimSelectionDepUserMenuItems(WindowInfo *window, int sensitive);
void DimPasteReplayBtns(int sensitive);
UserMenuCache *CreateUserMenuCache();
void FreeUserMenuCache(UserMenuCache *cache);
void InitUserBGMenuCache(UserBGMenuCache *cache);
void FreeUserBGMenuCache(UserBGMenuCache *cache);
void SetupUserMenuInfo();

#endif /* NEDIT_USERCMDS_H_INCLUDED */
