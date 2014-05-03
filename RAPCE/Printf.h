/*-----------------------------------------------------------------------------
	[Printf.h]

		Define things needed for implementing a customized printf function.

	Copyright (C) 2005 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
-----------------------------------------------------------------------------*/
#ifndef PRINTF_H_INCLUDED
#define PRINTF_H_INCLUDED

#include "TypeDefs.h"

void
PRINTF(
	const char*		pMessage, ...);

BOOL
PRINTF_Init();

void
PRINTF_Deinit();

void
PRINTF_Update();

void
PRINTF_SetSaveLoadMessage();


#endif /* PRINTF_H_INCLUDED */
