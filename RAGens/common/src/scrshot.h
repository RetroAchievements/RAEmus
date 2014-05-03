#ifndef SCRSHOT_H
#define SCRSHOT_H

extern char ScrShot_Dir[1024];

int Save_Shot(unsigned char *Screen, int mode, int X, int Y, int Pitch);

#endif