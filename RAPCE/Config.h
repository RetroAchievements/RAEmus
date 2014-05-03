/*-----------------------------------------------------------------------------
	[Config.h]

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
-----------------------------------------------------------------------------*/
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED


#include "TypeDefs.h"


BOOL
CONFIG_Init();

BOOL
CONFIG_Load(const char*		pPathName);

BOOL
CONFIG_Deinit();

BOOL
CONFIG_Save(const char*		pPathName);

BOOL
CONFIG_Set(	const char*		pName,
			const void*		pValue,
			const Sint32	valueSize);

BOOL
CONFIG_Get(	const char*		pName,
			      void*		pValue,
			const Sint32	valueSize);


#endif // CONFIG_H_INCLUDED
