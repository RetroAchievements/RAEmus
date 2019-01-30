/*
**  Oricutron
**  Copyright (C) 2009-2014 Peter Gordon
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation, version 2
**  of the License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

#ifndef MAIN_H
#define MAIN_H

extern SDL_bool warpspeed, soundon;
extern char mon_bpmsg[];
extern struct avi_handle *vidcap;
extern char tapepath[], diskpath[], telediskpath[], pravdiskpath[];
extern char atmosromfile[];
extern char oric1romfile[];
extern char mdiscromfile[];
extern char jasmnromfile[];
extern char pravetzromfile[2][1024];
extern char telebankfiles[8][1024];

SDL_bool read_config_string( char *buf, char *token, char *dest, Sint32 maxlen );
SDL_bool read_config_bool( char *buf, char *token, SDL_bool *dest );
SDL_bool read_config_option( char *buf, char *token, Sint32 *dest, char **options );
SDL_bool read_config_int( char *buf, char *token, int *dest, int min, int max );

#endif /* MAIN_H */
