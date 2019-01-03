#if !defined(win32_types_h)
#define win32_types_h

//  固定長型とか
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;

typedef signed char sint8;
typedef signed short sint16;
typedef signed int sint32;

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

// forQUASI88
#define	__stdcall

// forQUASI88 ... MPW not support bool ???
#if	defined(macintosh)
#if	  defined(__SC__) || defined(__MRC__)

typedef	int	bool;
//enum{ false, true };

#endif
#endif

#endif // win32_types_h
