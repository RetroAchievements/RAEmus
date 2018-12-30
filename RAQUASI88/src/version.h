#ifndef	VERSION_H
#define	VERSION_H

#ifndef	Q_TITLE
#define	Q_TITLE		"RAQUASI88"
#endif

#ifndef	Q_VERSION
#define	Q_VERSION	"1.0.0"
#endif

#ifndef	Q_COMMENT
#define	Q_COMMENT	""
#endif




#ifndef	Q_COPYRIGHT
#define	Q_COPYRIGHT		"(c) 1998-2018 S.Fukunaga, R.Zumer"
#endif

#ifdef	USE_SOUND
#ifndef	Q_MAME_COPYRIGHT
#define	Q_MAME_COPYRIGHT	"Copyright (c) 1997-2007, Nicola Salmoria and the MAME team"
#endif

#ifdef	USE_FMGEN
#ifndef	Q_FMGEN_COPYRIGHT
#define	Q_FMGEN_COPYRIGHT	"Copyright (C) by cisc 1998, 2003."
#endif
#endif
#endif

#endif	/* VERSION_H */
