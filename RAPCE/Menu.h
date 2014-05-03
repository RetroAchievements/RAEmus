/*------------------------------------------------------------------------------
	[Menu.h]

		Defines interface for the menu.

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
------------------------------------------------------------------------------*/
#ifndef		MENU_H_INCLUDED
#define		MENU_H_INCLUDED

#include "TypeDefs.h"


HANDLE
MENU_Init();

HANDLE
MENU_CreateSubMenu();

BOOL
MENU_AddItem(
	HANDLE			hMenu,
	HANDLE			hSubMenu,
	char*			pText,
	Uint32			id);


HANDLE
MENU_GetSubMenu(
	HANDLE			hMenu,
	Uint32			nPos);

BOOL
MENU_ChangeItemText(
	HANDLE			hMenu,
	Uint32			id,
	char*			pText);

BOOL
MENU_CheckItem(
	HANDLE		hMenu,
	Uint32		id,
	BOOL		bChecked);

BOOL
MENU_CheckRadioItem(
	HANDLE		hMenu,
	Uint32		idFrom,
	Uint32		idTo,
	Uint32		idRadio);

BOOL
MENU_EnableItem(
	HANDLE			hMenu,
	Uint32			id,
	BOOL			bEnabled);

void
MENU_Deinit(
	HANDLE			hMenu);

void
MENU_Show(
	HANDLE			hMenu);

void
MENU_ShowSubMenu(
	HANDLE		hSubMenu,
	Uint32		x,
	Uint32		y);

//KitaoçXêV
BOOL
MENU_RemoveItem(
	HANDLE			hMenu,
	Uint32			id);

//Kitaoí«â¡
BOOL
MENU_InsertItem(
	HANDLE			hMenu,
	HANDLE			hSubMenu,
	char*			pText,
	Uint32			id,
	DWORD			pos);

#endif /* MENU_H_INCLUDED */
