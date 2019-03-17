/*
	Skelton for retropc emulator

	Origin : MESS UPD7810 Core
	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD7801 ]
*/

#include "upd7801.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define PRESCALER	16

#define VA	regs[0].w.l
#define BC	regs[1].w.l
#define DE 	regs[2].w.l
#define HL	regs[3].w.l

#define _V	regs[0].b.h
#define _A	regs[0].b.l
#define _B	regs[1].b.h
#define _C	regs[1].b.l
#define _D	regs[2].b.h
#define _E	regs[2].b.l
#define _H	regs[3].b.h
#define _L	regs[3].b.l

#define altVA	regs[4].w.l
#define altBC	regs[5].w.l
#define altDE 	regs[6].w.l
#define altHL	regs[7].w.l

#define altV	regs[4].b.h
#define altA	regs[4].b.l
#define altB	regs[5].b.h
#define altC	regs[5].b.l
#define altD	regs[6].b.h
#define altE	regs[6].b.l
#define altH	regs[7].b.h
#define altL	regs[7].b.l

#define F_CY	0x01
#define F_L0	0x04
#define F_L1	0x08
#define F_HC	0x10
#define F_SK	0x20
#define F_Z	0x40

#define INTF0	0x01
#define INTFT	0x02
#define INTF1	0x04
#define INTF2	0x08
#define INTFS	0x10

#define SIO_DISABLED	(!(MC & 4) && (IN8(P_C) & 4))
#define SIO_EXTCLOCK	(MC & 0x80)

static const uint8_t irq_bits[5] = {
	INTF0, INTFT, INTF1, INTF2, INTFS
};

static const uint16_t irq_addr[5] = {
	0x0004, 0x0008, 0x0010, 0x0020, 0x0040
};

typedef struct {
	int oplen;	// bytes of opecode
	int clock;	// clock
} op_t;

static const op_t op[256] = {
	{1, 4}, {1, 6}, {1, 7}, {1, 7}, {3,10}, {3,16}, {1, 4}, {2, 7}, {1,11}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4},
	{1, 4}, {1, 4}, {1, 7}, {1, 7}, {3,10}, {3,16}, {2, 7}, {2, 7}, {1,11}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4},
	{2,13}, {1,19}, {1, 7}, {1, 7}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7},
	{2,13}, {1,13}, {1, 7}, {1, 7}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7},
	{1, 4}, {1, 4}, {1, 4}, {1, 4}, {3,16}, {3,13}, {2, 7}, {2, 7}, {0, 0}, {2,10}, {2,10}, {2,10}, {0, 0}, {0, 0}, {2,17}, {2,17},
	{1, 4}, {1, 4}, {1, 4}, {1, 4}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{0, 0}, {1, 4}, {1,15}, {1,13}, {0, 0}, {3,13}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7},
	{0, 0}, {3,13}, {1,19}, {1, 4}, {0, 0}, {3,13}, {2, 7}, {2, 7}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}

};
static const op_t op48[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,11}, {2,11}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,17}, {2, 8}, {2, 8}, {2,11}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op4c[256] = {
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op4d[256] = {
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op60[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op64[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op70[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}
};
static const op_t op74[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};

// flag control

#define ZHC_ADD(a, b, c) { \
	if(a) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z; \
	} \
	if(a == b) { \
		PSW = (PSW & ~F_CY) | (c); \
	} else if(a < b) { \
		PSW |= F_CY; \
	} else { \
		PSW &= ~F_CY; \
	} \
	if((a & 15) < (b & 15)) { \
		PSW |= F_HC; \
	} else { \
		PSW &= ~F_HC; \
	} \
}
#define ZHC_SUB(a, b, c) { \
	if(a) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z; \
	} \
	if(a == b) { \
		PSW = (PSW & ~F_CY) | (c); \
	} else if(a > b) { \
		PSW |= F_CY; \
	} else { \
		PSW &= ~F_CY; \
	} \
	if((a & 15) > (b & 15)) { \
		PSW |= F_HC; \
	} else { \
		PSW &= ~F_HC; \
	} \
}
#define SET_Z(n) { \
	if(n) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define SKIP_CY { \
	if(PSW & F_CY) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_NC { \
	if(!(PSW & F_CY)) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_Z { \
	if(PSW & F_Z) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_NZ { \
	if(!(PSW & F_Z)) { \
		PSW |= F_SK; \
	} \
}

// memory

inline uint8_t UPD7801::RM8(uint16_t addr)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint8_t val = d_mem->read_data8w(addr, &wait);
	period += wait;
	return val;
#else
	return d_mem->read_data8(addr);
#endif
}

inline void UPD7801::WM8(uint16_t addr, uint8_t val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data8w(addr, val, &wait);
	period += wait;
#else
	d_mem->write_data8(addr, val);
#endif
}

inline uint16_t UPD7801::RM16(uint16_t addr)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint16_t val = d_mem->read_data16w(addr, &wait);
	period += wait;
	return val;
#else
	return d_mem->read_data16(addr);
#endif
}

inline void UPD7801::WM16(uint16_t addr, uint16_t val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data16w(addr, val, &wait);
	period += wait;
#else
	d_mem->write_data16(addr, val);
#endif
}

inline uint8_t UPD7801::FETCH8()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint8_t val = d_mem->read_data8w(PC++, &wait);
	period += wait;
	return val;
#else
	return d_mem->read_data8(PC++);
#endif
}

inline uint16_t UPD7801::FETCH16()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint16_t val = d_mem->read_data16w(PC, &wait);
	period += wait;
#else
	uint16_t val = d_mem->read_data16(PC);
#endif
	PC += 2;
	return val;
}

inline uint16_t UPD7801::FETCHWA()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint16_t val = (_V << 8) | d_mem->read_data8w(PC++, &wait);
	period += wait;
	return val;
#else
	return (_V << 8) | d_mem->read_data8(PC++);
#endif
}

inline uint8_t UPD7801::POP8()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint8_t val = d_mem->read_data8w(SP++, &wait);
	period += wait;
	return val;
#else
	return d_mem->read_data8(SP++);
#endif
}

inline void UPD7801::PUSH8(uint8_t val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data8w(--SP, val, &wait);
	period += wait;
#else
	d_mem->write_data8(--SP, val);
#endif
}

inline uint16_t UPD7801::POP16()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint16_t val = d_mem->read_data16w(SP, &wait);
	period += wait;
#else
	uint16_t val = d_mem->read_data16(SP);
#endif
	SP += 2;
	return val;
}

inline void UPD7801::PUSH16(uint16_t val)
{
	SP -= 2;
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data16w(SP, val, &wait);
	period += wait;
#else
	d_mem->write_data16(SP, val);
#endif
}

// io

inline uint8_t UPD7801::IN8(int port)
{
	if(port == P_C) {
		return (d_io->read_io8(P_C) & 0x87) | (PORTC & 0x78);
	}
	return d_io->read_io8(port);
}

inline void UPD7801::OUT8(int port, uint8_t val)
{
	if(port == P_C) {
		PORTC = val;
	}
	d_io->write_io8(port, val);
}

// IOM : 0x20 = I/O, 0 = MEMORY
inline void UPD7801::UPDATE_PORTC(uint8_t IOM)
{
	uint8_t MC2 = (MC & 0x7f) | ((MC & 0x40) << 1);
	d_io->write_io8(P_C, (PORTC & MC2) | ((SAK | TO | IOM | HLDA) & ~MC2));
}

// opecode

#define ACI(r) { \
	uint8_t tmp = r + FETCH8() + (PSW & F_CY); \
	ZHC_ADD(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define ACI_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old + FETCH8() + (PSW & F_CY); \
	ZHC_ADD(tmp, old, (PSW & F_CY)); \
	OUT8(p, tmp); \
}
#define ADC(r, n) { \
	uint8_t tmp = r + n + (PSW & F_CY); \
	ZHC_ADD(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define ADCW() { \
	uint8_t tmp = _A + RM8(FETCHWA()) + (PSW & F_CY); \
	ZHC_ADD(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define ADCX(r) { \
	uint8_t tmp = _A + RM8(r) + (PSW & F_CY); \
	ZHC_ADD(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define ADD(r, n) { \
	uint8_t tmp = r + n; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
}
#define ADDNC(r, n) { \
	uint8_t tmp = r + n; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define ADDNCW() { \
	uint8_t tmp = _A + RM8(FETCHWA()); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define ADDNCX(r) { \
	uint8_t tmp = _A + RM8(r); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define ADDW() { \
	uint8_t tmp = _A + RM8(FETCHWA()); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
}
#define ADDX(r) { \
	uint8_t tmp = _A + RM8(r); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
}
#define ADI(r) { \
	uint8_t tmp = r + FETCH8(); \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
}
#define ADI_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old + FETCH8(); \
	ZHC_ADD(tmp, old, 0); \
	OUT8(p, tmp); \
}
#define ADINC(r) { \
	uint8_t tmp = r + FETCH8(); \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define ADINC_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old + FETCH8(); \
	ZHC_ADD(tmp, old, 0); \
	OUT8(p, tmp); \
	SKIP_NC; \
}
#define ANA(r, n) { \
	r &= n; \
	SET_Z(r); \
}
#define ANAW() { \
	_A &= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define ANAX(r) { \
	_A &= RM8(r); \
	SET_Z(_A); \
}
#define ANI(r) { \
	r &= FETCH8(); \
	SET_Z(r); \
}
#define ANI_IO(p) { \
	uint8_t tmp = IN8(p) & FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}
#define ANIW() { \
	uint16_t dst = FETCHWA(); \
	uint8_t tmp = RM8(dst) & FETCH8(); \
	WM8(dst, tmp); \
	SET_Z(tmp); \
}
#define BIT(b) { \
	if(RM8(FETCHWA()) & (1 << b)) { \
		PSW |= F_SK; \
	} \
}
#define BLOCK() { \
	WM8(DE++, RM8(HL++)); \
	if(_C--) { \
		PSW &= ~F_CY; \
		PC--; \
	} else { \
		PSW |= F_CY; \
	} \
}
#define CALF(o) { \
	uint16_t dst = 0x800 + ((o & 7) << 8) + FETCH8(); \
	PUSH16(PC); \
	PC = dst; \
}
#define CALL() { \
	uint16_t dst = FETCH16(); \
	PUSH16(PC); \
	PC = dst; \
}
#define CALT(o) { \
	uint16_t dst = RM16(0x80 + ((o & 0x3f) << 1)); \
	PUSH16(PC); \
	PC = dst; \
}
#define DAA() { \
	uint8_t lo = _A & 0xf, hi = _A >> 4, diff = 0; \
	if(lo <= 9 && !(PSW & F_HC)) { \
		diff = (hi >= 10 || (PSW & F_CY)) ? 0x60 : 0x00; \
	} else if(lo >= 10 && !(PSW & F_HC)) { \
		diff = (hi >= 9 || (PSW & F_CY)) ? 0x66 : 0x06; \
	} else if(lo <= 2 && (PSW & F_HC)) { \
		diff = (hi >= 10 || (PSW & F_CY)) ? 0x66 : 0x06; \
	} \
	_A += diff; \
	if(_A) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z; \
	} \
	if((PSW & F_CY) || (lo <= 9 ? hi >= 10 : hi >= 9)) { \
		PSW |= F_CY; \
	} else { \
		PSW &= ~F_CY; \
	} \
	if(lo >= 10) { \
		PSW |= F_HC; \
	} else { \
		PSW &= ~F_HC; \
	} \
}
#define DCR(r) { \
	uint8_t carry = PSW & F_CY; \
	uint8_t tmp = r - 1; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define DCRW() { \
	uint8_t carry = PSW & F_CY; \
	uint16_t dst = FETCHWA(); \
	uint8_t old = RM8(dst); \
	uint8_t tmp = old - 1; \
	ZHC_SUB(tmp, old, 0); \
	WM8(dst, tmp); \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define EQA(r, n) { \
	uint8_t tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_Z; \
}
#define EQAW() { \
	uint8_t tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_Z; \
}
#define EQAX(r) { \
	uint8_t tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_Z; \
}
#define EQI(r) { \
	uint8_t tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_Z; \
}
#define EQI_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_Z; \
}
#define EQIW() { \
	uint8_t old = RM8(FETCHWA()); \
	uint8_t tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_Z; \
}
#define EX() { \
	uint16_t tmp; \
	tmp = VA; VA = altVA; altVA = tmp; \
}
#define EXX() { \
	uint16_t tmp; \
	tmp = BC; BC = altBC; altBC = tmp; \
	tmp = DE; DE = altDE; altDE = tmp; \
	tmp = HL; HL = altHL; altHL = tmp; \
}
#define GTA(r, n) { \
	uint8_t tmp = r - n - 1; \
	ZHC_SUB(tmp, r, 1); \
	SKIP_NC; \
}
#define GTAW() { \
	uint8_t tmp = _A - RM8(FETCHWA()) - 1; \
	ZHC_SUB(tmp, _A, 1); \
	SKIP_NC; \
}
#define GTAX(r) { \
	uint8_t tmp = _A - RM8(r) - 1; \
	ZHC_SUB(tmp, _A, 1); \
	SKIP_NC; \
}
#define GTI(r) { \
	uint8_t tmp = r - FETCH8() - 1; \
	ZHC_SUB(tmp, r, 1); \
	SKIP_NC; \
}
#define GTI_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old - FETCH8() - 1; \
	ZHC_SUB(tmp, old, 1); \
	SKIP_NC; \
}
#define GTIW() { \
	uint8_t old = RM8(FETCHWA()); \
	uint8_t tmp = old - FETCH8() - 1; \
	ZHC_SUB(tmp, old, 1); \
	SKIP_NC; \
}
#define INR(r) { \
	uint8_t carry = PSW & F_CY; \
	uint8_t tmp = r + 1; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define INRW() { \
	uint8_t carry = PSW & F_CY; \
	uint16_t dst = FETCHWA(); \
	uint8_t old = RM8(dst); \
	uint8_t tmp = old + 1; \
	ZHC_ADD(tmp, old, 0); \
	WM8(dst, tmp); \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define JRE(o) { \
	uint8_t tmp = FETCH8(); \
	if(o & 1) { \
		PC -= 256 - tmp; \
	} else { \
		PC += tmp; \
	} \
}
#define LTA(r, n) { \
	uint8_t tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_CY; \
}
#define LTAW() { \
	uint8_t tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_CY; \
}
#define LTAX(r) { \
	uint8_t tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_CY; \
}
#define LTI(r) { \
	uint8_t tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_CY; \
}
#define LTI_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_CY; \
}
#define LTIW() { \
	uint8_t old = RM8(FETCHWA()); \
	uint8_t tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_CY; \
}
#define MVIW() { \
	uint16_t dst = FETCHWA(); \
	WM8(dst, FETCH8()); \
}
#define NEA(r, n) { \
	uint8_t tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_NZ; \
}
#define NEAW() { \
	uint8_t tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_NZ; \
}
#define NEAX(r) { \
	uint8_t tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_NZ; \
}
#define NEI(r) { \
	uint8_t tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_NZ; \
}
#define NEI_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_NZ; \
}
#define NEIW() { \
	uint8_t old = RM8(FETCHWA()); \
	uint8_t tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_NZ; \
}
#define OFFA(r, n) { \
	if(r & n) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFAW() { \
	if(_A & RM8(FETCHWA())) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFAX(r) { \
	if(_A & RM8(r)) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFI(r) { \
	if(r & FETCH8()) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFI_IO(p) { \
	if(IN8(p) & FETCH8()) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFIW() { \
	uint8_t tmp = RM8(FETCHWA()); \
	if(tmp & FETCH8()) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define ONA(r, n) { \
	if(r & n) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONAW() { \
	if(_A & RM8(FETCHWA())) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONAX(r) { \
	if(_A & RM8(r)) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONI(r) { \
	if(r & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONI_IO(p) { \
	if(IN8(p) & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONIW() { \
	uint8_t tmp = RM8(FETCHWA()); \
	if(tmp & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ORA(r, n) { \
	r |= n; \
	SET_Z(r); \
}
#define ORAW() { \
	_A |= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define ORAX(r) { \
	_A |= RM8(r); \
	SET_Z(_A); \
}
#define ORI(r) { \
	r |= FETCH8(); \
	SET_Z(r); \
}
#define ORI_IO(p) { \
	uint8_t tmp = IN8(p) | FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}
#define ORIW() { \
	uint16_t dst = FETCHWA(); \
	uint8_t tmp = RM8(dst) | FETCH8(); \
	WM8(dst, tmp); \
	SET_Z(tmp); \
}
#define PEN() { \
}
#define PER() { \
}
#define PEX() { \
}
#define RLD() { \
	uint8_t old = RM8(HL); \
	uint8_t tmp = (old << 4) | (_A & 0x0f); \
	_A = (_A & 0xf0) | (old >> 4); \
	WM8(HL, tmp); \
}
#define RLL(r) { \
	uint8_t carry = PSW & F_CY; \
	PSW = (PSW & ~F_CY) | ((r >> 7) & F_CY); \
	r = (r << 1) | carry; \
}
#define RLR(r) { \
	uint8_t carry = (PSW & F_CY) << 7; \
	PSW = (PSW & ~F_CY) | (r & F_CY); \
	r = (r >> 1) | carry; \
}
#define RRD() { \
	uint8_t old = RM8(HL); \
	uint8_t tmp = (_A << 4) | (old >> 4); \
	_A = (_A & 0xf0) | (old & 0x0f); \
	WM8(HL, tmp); \
}
#define SBB(r, n) { \
	uint8_t tmp = r - n - (PSW & F_CY); \
	ZHC_SUB(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define SBBW() { \
	uint8_t tmp = _A - RM8(FETCHWA()) - (PSW & F_CY); \
	ZHC_SUB(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define SBBX(r) { \
	uint8_t tmp = _A - RM8(r) - (PSW & F_CY); \
	ZHC_SUB(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define SBI(r) { \
	uint8_t tmp = r - FETCH8() - (PSW & F_CY); \
	ZHC_SUB(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define SBI_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old - FETCH8() - (PSW & F_CY); \
	ZHC_SUB(tmp, old, (PSW & F_CY)); \
	OUT8(p, tmp); \
}
#define SIO() { \
	scount = 4; \
	sio_count = 0; \
}
#define SK(f) { \
	if(PSW & f) { \
		PSW |= F_SK; \
	} \
}
#define SKIT(f) { \
	if(IRR & f) { \
		PSW |= F_SK; \
	} \
	IRR &= ~f; \
}
#define SKN(f) { \
	if(!(PSW & f)) { \
		PSW |= F_SK; \
	} \
}
#define SKNIT(f) { \
	if(!(IRR & f)) { \
		PSW |= F_SK; \
	} \
	IRR &= ~f; \
}
#define SLL(r) { \
	PSW = (PSW & ~F_CY) | ((r >> 7) & F_CY); \
	r <<= 1; \
}
#define SLR(r) { \
	PSW = (PSW & ~F_CY) | (r & F_CY); \
	r >>= 1; \
}
#define STM() { \
	tcount = (((TM0 | (TM1 << 8)) & 0xfff) + 1) * PRESCALER; \
}
#define SUB(r, n) { \
	uint8_t tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
}
#define SUBNB(r, n) { \
	uint8_t tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define SUBNBW() { \
	uint8_t tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define SUBNBX(r) { \
	uint8_t tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define SUBW() { \
	uint8_t tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
}
#define SUBX(r) { \
	uint8_t tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
}
#define SUI(r) { \
	uint8_t tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
}
#define SUI_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	OUT8(p, tmp); \
}
#define SUINB(r) { \
	uint8_t tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define SUINB_IO(p) { \
	uint8_t old = IN8(p); \
	uint8_t tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	OUT8(p, tmp); \
	SKIP_NC; \
}
#define XRA(r, n) { \
	r ^= n; \
	SET_Z(r); \
}
#define XRAW() { \
	_A ^= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define XRAX(r) { \
	_A ^= RM8(r); \
	SET_Z(_A); \
}
#define XRI(r) { \
	r ^= FETCH8(); \
	SET_Z(r); \
}
#define XRI_IO(p) { \
	uint8_t tmp = IN8(p) ^ FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}

void UPD7801::initialize()
{
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_io_stored = d_io;
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void UPD7801::reset()
{
	PC = SP = 0;
//	VA = BC = DE = HL = altVA = altBC = altDE = altHL = 0;
	PSW = IRR = IFF = SIRQ = HALT = 0;
	_V = MB = MC = TM0 = TM1 = SR = 0xff;
	altVA = VA;
	MK = 0x1f;
	PORTC = TO = SAK = HLDA = 0;
	count = 0;
	scount = tcount = 0;
	wait = false;
	sio_count = 0;
}

int UPD7801::run(int clock)
{
	// run cpu
	if(clock == -1) {
		// run only one opcode
		count = 0;
#ifdef USE_DEBUGGER
		run_one_opecode_debugger();
#else
		run_one_opecode();
#endif
		return -count;
	} else {
		// run cpu while given clocks
		count += clock;
		int first_count = count;
		
		while(count > 0) {
#ifdef USE_DEBUGGER
			run_one_opecode_debugger();
#else
			run_one_opecode();
#endif
		}
		return first_count - count;
	}
}

void UPD7801::run_one_opecode()
{
	if(!(MC & 0x40) && (IN8(P_C) & 0x80)) {
		if(!HLDA) {
			HLDA = 0x40;
			UPDATE_PORTC(0);
		}
	} else {
		if(HLDA) {
			HLDA = 0;
			UPDATE_PORTC(0);
		}
	}
	if(HLDA || wait) {
		period = 1;
	} else {
		// interrupt is enabled after next opecode of ei
		if(IFF & 2) {
			IFF--;
		}
		
		// run 1 opecode
#ifdef USE_DEBUGGER
		d_debugger->add_cpu_trace(PC);
#endif
		period = 0;
		prevPC = PC;
		OP();
	}
#ifdef USE_DEBUGGER
	total_count += period;
#endif
	count -= period;
	
	// update serial count
	if(scount) {
		scount -= period;
		while(scount <= 0) {
			if(!SIO_DISABLED && !SIO_EXTCLOCK) {
				write_signals(&outputs_so, (SR & 0x80) ? 0xffffffff : 0);
				SR <<= 1;
				if(SI) SR |= 1;
				if(++sio_count == 8) {
					IRR |= INTFS;
					if(SAK) {
						SAK = 0;
						UPDATE_PORTC(0);
					}
					scount = sio_count = 0;
					break;
				}
			}
			scount += 4;
		}
	}
	
	// update timer
	if(tcount && (tcount -= period) <= 0) {
		tcount += (((TM0 | (TM1 << 8)) & 0xfff) + 1) * PRESCALER;
		IRR |= INTFT;
		if(TO) {
			TO = 0;
			UPDATE_PORTC(0);
		}
	}
	
	// check interrupt
	if(IFF == 1 && !SIRQ) {
		for(int i = 0; i < 5; i++) {
			uint8_t bit = irq_bits[i];
			if((IRR & bit) && !(MK & bit)) {
				if(HALT) {
					HALT = 0;
					PC++;
				}
				PUSH8(PSW);
				PUSH16(PC);
				
				PC = irq_addr[i];
				PSW &= ~(F_SK | F_L0 | F_L1);
				IFF = 0;
				IRR &= ~bit;
				break;
			}
		}
	}
}

#ifdef USE_DEBUGGER
void UPD7801::run_one_opecode_debugger()
{
	bool now_debugging = d_debugger->now_debugging;
	if(now_debugging) {
		d_debugger->check_break_points(PC);
		if(d_debugger->now_suspended) {
			d_debugger->now_waiting = true;
			emu->start_waiting_in_debugger();
			while(d_debugger->now_debugging && d_debugger->now_suspended) {
				emu->process_waiting_in_debugger();
			}
			emu->finish_waiting_in_debugger();
			d_debugger->now_waiting = false;
		}
		if(d_debugger->now_debugging) {
			d_mem = d_io = d_debugger;
		} else {
			now_debugging = false;
		}
		run_one_opecode();
		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended = true;
			}
			d_mem = d_mem_stored;
			d_io = d_io_stored;
		}
	} else {
		run_one_opecode();
	}
}

void UPD7801::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32_t UPD7801::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

void UPD7801::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io_stored->write_io8w(addr, data, &wait);
}

uint32_t UPD7801::read_debug_io8(uint32_t addr) {
	int wait;
	return d_io_stored->read_io8w(addr, &wait);
}

bool UPD7801::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		SP = data;
	} else if(_tcsicmp(reg, _T("VA")) == 0) {
		VA = data;
	} else if(_tcsicmp(reg, _T("BC")) == 0) {
		BC = data;
	} else if(_tcsicmp(reg, _T("DE")) == 0) {
		DE = data;
	} else if(_tcsicmp(reg, _T("HL")) == 0) {
		HL = data;
	} else if(_tcsicmp(reg, _T("V")) == 0) {
		_V = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		_A = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		_B = data;
	} else if(_tcsicmp(reg, _T("C")) == 0) {
		_C = data;
	} else if(_tcsicmp(reg, _T("D")) == 0) {
		_D = data;
	} else if(_tcsicmp(reg, _T("E")) == 0) {
		_E = data;
	} else if(_tcsicmp(reg, _T("H")) == 0) {
		_H = data;
	} else if(_tcsicmp(reg, _T("L")) == 0) {
		_L = data;
	} else if(_tcsicmp(reg, _T("VA'")) == 0) {
		altVA = data;
	} else if(_tcsicmp(reg, _T("BC'")) == 0) {
		altBC = data;
	} else if(_tcsicmp(reg, _T("DE'")) == 0) {
		altDE = data;
	} else if(_tcsicmp(reg, _T("HL'")) == 0) {
		altHL = data;
	} else if(_tcsicmp(reg, _T("V'")) == 0) {
		altV = data;
	} else if(_tcsicmp(reg, _T("A'")) == 0) {
		altA = data;
	} else if(_tcsicmp(reg, _T("B'")) == 0) {
		altB = data;
	} else if(_tcsicmp(reg, _T("C'")) == 0) {
		altC = data;
	} else if(_tcsicmp(reg, _T("D'")) == 0) {
		altD = data;
	} else if(_tcsicmp(reg, _T("E'")) == 0) {
		altE = data;
	} else if(_tcsicmp(reg, _T("H'")) == 0) {
		altH = data;
	} else if(_tcsicmp(reg, _T("L'")) == 0) {
		altL = data;
	} else {
		return false;
	}
	return true;
}

bool UPD7801::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
VA = 0000  BC = 0000  DE = 0000 HL = 0000  PSW= 00 [Z SK HC L1 L0 CY]
VA'= 0000  BC'= 0000  DE'= 0000 HL'= 0000  SP = 0000  PC = 0000
          (BC)= 0000 (DE)=0000 (HL)= 0000 (SP)= 0000 <DI>
Clocks = 0 (0)  Since Scanline = 0/0 (0/0)
*/
	int wait;
	my_stprintf_s(buffer, buffer_len,
	_T("VA = %04X  BC = %04X  DE = %04X HL = %04X  PSW= %02x [%s %s %s %s %s %s]\n")
	_T("VA'= %04X  BC'= %04X  DE'= %04X HL'= %04X  SP = %04X  PC = %04X\n")
	_T("          (BC)= %04X (DE)=%04X (HL)= %04X (SP)= %04X <%s>\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
	VA, BC, DE, HL, PSW,
	(PSW & F_Z) ? _T("Z") : _T("-"), (PSW & F_SK) ? _T("SK") : _T("--"), (PSW & F_HC) ? _T("HC") : _T("--"), (PSW & F_L1) ? _T("L1") : _T("--"), (PSW & F_L0) ? _T("L0") : _T("--"), (PSW & F_CY) ? _T("CY") : _T("--"),
	altVA, altBC, altDE, altHL, SP, PC,
	d_mem_stored->read_data16w(BC, &wait), d_mem_stored->read_data16w(DE, &wait), d_mem_stored->read_data16w(HL, &wait), d_mem_stored->read_data16w(SP, &wait),
	IFF ? _T("EI") : _T("DI"),
	total_count, total_count - prev_total_count,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	prev_total_count = total_count;
	return true;
}

// disassembler

uint8_t upd7801_dasm_ops[4];
int upd7801_dasm_ptr;

uint8_t getb()
{
	return upd7801_dasm_ops[upd7801_dasm_ptr++];
}

uint16_t getw()
{
	uint16_t l = getb();
	return l | (getb() << 8);
}

uint8_t getwa()
{
	return getb();
}

int UPD7801::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	for(int i = 0; i < 4; i++) {
		int wait;
		upd7801_dasm_ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
	}
	upd7801_dasm_ptr = 0;
	
	uint8_t b;
	uint16_t wa;
	
	switch(b = getb()) {
	case 0x00: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
	case 0x01: my_stprintf_s(buffer, buffer_len, _T("hlt")); break;
	case 0x02: my_stprintf_s(buffer, buffer_len, _T("inx sp")); break;
	case 0x03: my_stprintf_s(buffer, buffer_len, _T("dcx sp")); break;
	case 0x04: my_stprintf_s(buffer, buffer_len, _T("lxi sp,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
	case 0x05: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("aniw v.%02xh,%02xh"), wa, getb()); break;
//	case 0x06:
	case 0x07: my_stprintf_s(buffer, buffer_len, _T("ani a,%02xh"), getb()); break;
	case 0x08: my_stprintf_s(buffer, buffer_len, _T("ret")); break;
	case 0x09: my_stprintf_s(buffer, buffer_len, _T("sio")); break;
	case 0x0a: my_stprintf_s(buffer, buffer_len, _T("mov a,b")); break;
	case 0x0b: my_stprintf_s(buffer, buffer_len, _T("mov a,c")); break;
	case 0x0c: my_stprintf_s(buffer, buffer_len, _T("mov a,d")); break;
	case 0x0d: my_stprintf_s(buffer, buffer_len, _T("mov a,e")); break;
	case 0x0e: my_stprintf_s(buffer, buffer_len, _T("mov a,h")); break;
	case 0x0f: my_stprintf_s(buffer, buffer_len, _T("mov a,l")); break;
	
	case 0x10: my_stprintf_s(buffer, buffer_len, _T("ex")); break;
	case 0x11: my_stprintf_s(buffer, buffer_len, _T("exx")); break;
	case 0x12: my_stprintf_s(buffer, buffer_len, _T("inx b")); break;
	case 0x13: my_stprintf_s(buffer, buffer_len, _T("dcx b")); break;
	case 0x14: my_stprintf_s(buffer, buffer_len, _T("lxi b,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
	case 0x15: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("oriw v.%02xh,%02xh"), wa, getb()); break;
	case 0x16: my_stprintf_s(buffer, buffer_len, _T("xri a,%02xh"), getb()); break;
	case 0x17: my_stprintf_s(buffer, buffer_len, _T("ori a,%02xh"), getb()); break;
	case 0x18: my_stprintf_s(buffer, buffer_len, _T("rets")); break;
	case 0x19: my_stprintf_s(buffer, buffer_len, _T("stm")); break;
	case 0x1a: my_stprintf_s(buffer, buffer_len, _T("mov b,a")); break;
	case 0x1b: my_stprintf_s(buffer, buffer_len, _T("mov c,a")); break;
	case 0x1c: my_stprintf_s(buffer, buffer_len, _T("mov d,a")); break;
	case 0x1d: my_stprintf_s(buffer, buffer_len, _T("mov e,a")); break;
	case 0x1e: my_stprintf_s(buffer, buffer_len, _T("mov h,a")); break;
	case 0x1f: my_stprintf_s(buffer, buffer_len, _T("mov l,a")); break;
	
	case 0x20: my_stprintf_s(buffer, buffer_len, _T("inrw v.%02xh"), getwa()); break;
	case 0x21: my_stprintf_s(buffer, buffer_len, _T("table")); break;
	case 0x22: my_stprintf_s(buffer, buffer_len, _T("inx d")); break;
	case 0x23: my_stprintf_s(buffer, buffer_len, _T("dcx d")); break;
	case 0x24: my_stprintf_s(buffer, buffer_len, _T("lxi d,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
	case 0x25: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("gtiw v.%02xh,%02xh"), wa, getb()); break;
	case 0x26: my_stprintf_s(buffer, buffer_len, _T("adinc a,%02xh"), getb()); break;
	case 0x27: my_stprintf_s(buffer, buffer_len, _T("gti a,%02xh"), getb()); break;
	case 0x28: my_stprintf_s(buffer, buffer_len, _T("ldaw v.%02xh"), getwa()); break;
	case 0x29: my_stprintf_s(buffer, buffer_len, _T("ldax b")); break;
	case 0x2a: my_stprintf_s(buffer, buffer_len, _T("ldax d")); break;
	case 0x2b: my_stprintf_s(buffer, buffer_len, _T("ldax h")); break;
	case 0x2c: my_stprintf_s(buffer, buffer_len, _T("ldax d+")); break;
	case 0x2d: my_stprintf_s(buffer, buffer_len, _T("ldax h+")); break;
	case 0x2e: my_stprintf_s(buffer, buffer_len, _T("ldax d-")); break;
	case 0x2f: my_stprintf_s(buffer, buffer_len, _T("ldax h-")); break;
	
	case 0x30: my_stprintf_s(buffer, buffer_len, _T("dcrw v.%02xh"), getwa()); break;
	case 0x31: my_stprintf_s(buffer, buffer_len, _T("block")); break;
	case 0x32: my_stprintf_s(buffer, buffer_len, _T("inx h")); break;
	case 0x33: my_stprintf_s(buffer, buffer_len, _T("dcx h")); break;
	case 0x34: my_stprintf_s(buffer, buffer_len, _T("lxi h,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
	case 0x35: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("ltiw v.%02xh,%02xh"), wa, getb()); break;
	case 0x36: my_stprintf_s(buffer, buffer_len, _T("suinb a,%02xh"), getb()); break;
	case 0x37: my_stprintf_s(buffer, buffer_len, _T("lti a,%02xh"), getb()); break;
	case 0x38: my_stprintf_s(buffer, buffer_len, _T("staw v.%02xh"), getwa()); break;
	case 0x39: my_stprintf_s(buffer, buffer_len, _T("stax b")); break;
	case 0x3a: my_stprintf_s(buffer, buffer_len, _T("stax d")); break;
	case 0x3b: my_stprintf_s(buffer, buffer_len, _T("stax h")); break;
	case 0x3c: my_stprintf_s(buffer, buffer_len, _T("stax d+")); break;
	case 0x3d: my_stprintf_s(buffer, buffer_len, _T("stax h+")); break;
	case 0x3e: my_stprintf_s(buffer, buffer_len, _T("stax d-")); break;
	case 0x3f: my_stprintf_s(buffer, buffer_len, _T("stax h-")); break;
	
//	case 0x40:
	case 0x41: my_stprintf_s(buffer, buffer_len, _T("inr a")); break;
	case 0x42: my_stprintf_s(buffer, buffer_len, _T("inr b")); break;
	case 0x43: my_stprintf_s(buffer, buffer_len, _T("inr c")); break;
	case 0x44: my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
	case 0x45: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("oniw v.%02xh,%02xh"), wa, getb()); break;
	case 0x46: my_stprintf_s(buffer, buffer_len, _T("adi a,%02xh"), getb()); break;
	case 0x47: my_stprintf_s(buffer, buffer_len, _T("oni a,%02xh"), getb()); break;
	case 0x48:
		switch(b = getb()) {
		case 0x00: my_stprintf_s(buffer, buffer_len, _T("skit intf0")); break;
		case 0x01: my_stprintf_s(buffer, buffer_len, _T("skit intft")); break;
		case 0x02: my_stprintf_s(buffer, buffer_len, _T("skit intf1")); break;
		case 0x03: my_stprintf_s(buffer, buffer_len, _T("skit intf2")); break;
		case 0x04: my_stprintf_s(buffer, buffer_len, _T("skit intfs")); break;
		case 0x0a: my_stprintf_s(buffer, buffer_len, _T("sk cy")); break;
		case 0x0c: my_stprintf_s(buffer, buffer_len, _T("sk z")); break;
		case 0x0e: my_stprintf_s(buffer, buffer_len, _T("push v")); break;
		case 0x0f: my_stprintf_s(buffer, buffer_len, _T("pop v")); break;
		case 0x10: my_stprintf_s(buffer, buffer_len, _T("sknit f0")); break;
		case 0x11: my_stprintf_s(buffer, buffer_len, _T("sknit ft")); break;
		case 0x12: my_stprintf_s(buffer, buffer_len, _T("sknit f1")); break;
		case 0x13: my_stprintf_s(buffer, buffer_len, _T("sknit f2")); break;
		case 0x14: my_stprintf_s(buffer, buffer_len, _T("sknit fs")); break;
		case 0x1a: my_stprintf_s(buffer, buffer_len, _T("skn cy")); break;
		case 0x1c: my_stprintf_s(buffer, buffer_len, _T("skn z")); break;
		case 0x1e: my_stprintf_s(buffer, buffer_len, _T("push b")); break;
		case 0x1f: my_stprintf_s(buffer, buffer_len, _T("pop b")); break;
		case 0x20: my_stprintf_s(buffer, buffer_len, _T("ei")); break;
		case 0x24: my_stprintf_s(buffer, buffer_len, _T("di")); break;
		case 0x2a: my_stprintf_s(buffer, buffer_len, _T("clc")); break;
		case 0x2b: my_stprintf_s(buffer, buffer_len, _T("stc")); break;
		case 0x2c: my_stprintf_s(buffer, buffer_len, _T("pen")); break;
		case 0x2d: my_stprintf_s(buffer, buffer_len, _T("pex")); break;
		case 0x2e: my_stprintf_s(buffer, buffer_len, _T("push d")); break;
		case 0x2f: my_stprintf_s(buffer, buffer_len, _T("pop d")); break;
		case 0x30: my_stprintf_s(buffer, buffer_len, _T("rll a")); break;
		case 0x31: my_stprintf_s(buffer, buffer_len, _T("rlr a")); break;
		case 0x32: my_stprintf_s(buffer, buffer_len, _T("rll c")); break;
		case 0x33: my_stprintf_s(buffer, buffer_len, _T("rlr c")); break;
		case 0x34: my_stprintf_s(buffer, buffer_len, _T("sll a")); break;
		case 0x35: my_stprintf_s(buffer, buffer_len, _T("slr a")); break;
		case 0x36: my_stprintf_s(buffer, buffer_len, _T("sll c")); break;
		case 0x37: my_stprintf_s(buffer, buffer_len, _T("sll c")); break;
		case 0x38: my_stprintf_s(buffer, buffer_len, _T("rld")); break;
		case 0x39: my_stprintf_s(buffer, buffer_len, _T("rrd")); break;
		case 0x3c: my_stprintf_s(buffer, buffer_len, _T("per")); break;
		case 0x3e: my_stprintf_s(buffer, buffer_len, _T("push h")); break;
		case 0x3f: my_stprintf_s(buffer, buffer_len, _T("pop h")); break;
		default: my_stprintf_s(buffer, buffer_len, _T("db 48h,%02xh"), b);
		}
		break;
	case 0x49: my_stprintf_s(buffer, buffer_len, _T("mvix b,%02xh"), getb()); break;
	case 0x4a: my_stprintf_s(buffer, buffer_len, _T("mvix d,%02xh"), getb()); break;
	case 0x4b: my_stprintf_s(buffer, buffer_len, _T("mvix h,%02xh"), getb()); break;
	case 0x4c:
		switch(b = getb()) {
		case 0xc0: my_stprintf_s(buffer, buffer_len, _T("mov a,pa")); break;
		case 0xc1: my_stprintf_s(buffer, buffer_len, _T("mov a,pb")); break;
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("mov a,pc")); break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("mov a,mk")); break;
		case 0xc4: my_stprintf_s(buffer, buffer_len, _T("mov a,mb")); break;	// 未定義?
		case 0xc5: my_stprintf_s(buffer, buffer_len, _T("mov a,mc")); break;	// 未定義?
		case 0xc6: my_stprintf_s(buffer, buffer_len, _T("mov a,tm0")); break;	// 未定義?
		case 0xc7: my_stprintf_s(buffer, buffer_len, _T("mov a,tm1")); break;	// 未定義?
		case 0xc8: my_stprintf_s(buffer, buffer_len, _T("mov a,s")); break;
		default:
			if(b < 0xc0) {
				my_stprintf_s(buffer, buffer_len, _T("in %02xh"), getb()); break;
			}
			my_stprintf_s(buffer, buffer_len, _T("db 4ch,%02xh"), b);
		}
		break;
	case 0x4d:
		switch(b = getb()) {
		case 0xc0: my_stprintf_s(buffer, buffer_len, _T("mov pa,a")); break;
		case 0xc1: my_stprintf_s(buffer, buffer_len, _T("mov pb,a")); break;
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("mov pc,a")); break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("mov mk,a")); break;
		case 0xc4: my_stprintf_s(buffer, buffer_len, _T("mov mb,a")); break;
		case 0xc5: my_stprintf_s(buffer, buffer_len, _T("mov mc,a")); break;
		case 0xc6: my_stprintf_s(buffer, buffer_len, _T("mov tm0,a")); break;
		case 0xc7: my_stprintf_s(buffer, buffer_len, _T("mov tm1,a")); break;
		case 0xc8: my_stprintf_s(buffer, buffer_len, _T("mov s,a")); break;
		default:
			if(b < 0xc0) {
				my_stprintf_s(buffer, buffer_len, _T("out %02xh"), getb()); break;
			}
			my_stprintf_s(buffer, buffer_len, _T("db 4dh,%02xh"), b);
		}
		break;
	case 0x4e: b = getb(); my_stprintf_s(buffer, buffer_len, _T("jre %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), pc + upd7801_dasm_ptr + b)); break;
	case 0x4f: b = getb(); my_stprintf_s(buffer, buffer_len, _T("jre %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), (pc + upd7801_dasm_ptr + b - 256) & 0xffff)); break;
	
//	case 0x50:
	case 0x51: my_stprintf_s(buffer, buffer_len, _T("dcr a")); break;
	case 0x52: my_stprintf_s(buffer, buffer_len, _T("dcr b")); break;
	case 0x53: my_stprintf_s(buffer, buffer_len, _T("dcr c")); break;
	case 0x54: my_stprintf_s(buffer, buffer_len, _T("jmp %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
	case 0x55: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("offiw v.%02xh,%02xh"), wa, getb()); break;
	case 0x56: my_stprintf_s(buffer, buffer_len, _T("aci a,%02xh"), getb()); break;
	case 0x57: my_stprintf_s(buffer, buffer_len, _T("offi a,%02xh"), getb()); break;
	case 0x58: my_stprintf_s(buffer, buffer_len, _T("bit 0,v.%02xh"), getwa()); break;
	case 0x59: my_stprintf_s(buffer, buffer_len, _T("bit 1,v.%02xh"), getwa()); break;
	case 0x5a: my_stprintf_s(buffer, buffer_len, _T("bit 2,v.%02xh"), getwa()); break;
	case 0x5b: my_stprintf_s(buffer, buffer_len, _T("bit 3,v.%02xh"), getwa()); break;
	case 0x5c: my_stprintf_s(buffer, buffer_len, _T("bit 4,v.%02xh"), getwa()); break;
	case 0x5d: my_stprintf_s(buffer, buffer_len, _T("bit 5,v.%02xh"), getwa()); break;
	case 0x5e: my_stprintf_s(buffer, buffer_len, _T("bit 6,v.%02xh"), getwa()); break;
	case 0x5f: my_stprintf_s(buffer, buffer_len, _T("bit 7,v.%02xh"), getwa()); break;
	
	case 0x60:
		switch(b = getb()) {
		case 0x08: my_stprintf_s(buffer, buffer_len, _T("ana v,a")); break;
		case 0x09: my_stprintf_s(buffer, buffer_len, _T("ana a,a")); break;
		case 0x0a: my_stprintf_s(buffer, buffer_len, _T("ana b,a")); break;
		case 0x0b: my_stprintf_s(buffer, buffer_len, _T("ana c,a")); break;
		case 0x0c: my_stprintf_s(buffer, buffer_len, _T("ana d,a")); break;
		case 0x0d: my_stprintf_s(buffer, buffer_len, _T("ana e,a")); break;
		case 0x0e: my_stprintf_s(buffer, buffer_len, _T("ana h,a")); break;
		case 0x0f: my_stprintf_s(buffer, buffer_len, _T("ana l,a")); break;
		case 0x10: my_stprintf_s(buffer, buffer_len, _T("xra v,a")); break;
		case 0x11: my_stprintf_s(buffer, buffer_len, _T("xra a,a")); break;
		case 0x12: my_stprintf_s(buffer, buffer_len, _T("xra b,a")); break;
		case 0x13: my_stprintf_s(buffer, buffer_len, _T("xra c,a")); break;
		case 0x14: my_stprintf_s(buffer, buffer_len, _T("xra d,a")); break;
		case 0x15: my_stprintf_s(buffer, buffer_len, _T("xra e,a")); break;
		case 0x16: my_stprintf_s(buffer, buffer_len, _T("xra h,a")); break;
		case 0x17: my_stprintf_s(buffer, buffer_len, _T("xra l,a")); break;
		case 0x18: my_stprintf_s(buffer, buffer_len, _T("ora v,a")); break;
		case 0x19: my_stprintf_s(buffer, buffer_len, _T("ora a,a")); break;
		case 0x1a: my_stprintf_s(buffer, buffer_len, _T("ora b,a")); break;
		case 0x1b: my_stprintf_s(buffer, buffer_len, _T("ora c,a")); break;
		case 0x1c: my_stprintf_s(buffer, buffer_len, _T("ora d,a")); break;
		case 0x1d: my_stprintf_s(buffer, buffer_len, _T("ora e,a")); break;
		case 0x1e: my_stprintf_s(buffer, buffer_len, _T("ora h,a")); break;
		case 0x1f: my_stprintf_s(buffer, buffer_len, _T("ora l,a")); break;
		case 0x20: my_stprintf_s(buffer, buffer_len, _T("addnc v,a")); break;
		case 0x21: my_stprintf_s(buffer, buffer_len, _T("addnc a,a")); break;
		case 0x22: my_stprintf_s(buffer, buffer_len, _T("addnc b,a")); break;
		case 0x23: my_stprintf_s(buffer, buffer_len, _T("addnc c,a")); break;
		case 0x24: my_stprintf_s(buffer, buffer_len, _T("addnc d,a")); break;
		case 0x25: my_stprintf_s(buffer, buffer_len, _T("addnc e,a")); break;
		case 0x26: my_stprintf_s(buffer, buffer_len, _T("addnc h,a")); break;
		case 0x27: my_stprintf_s(buffer, buffer_len, _T("addnc l,a")); break;
		case 0x28: my_stprintf_s(buffer, buffer_len, _T("gta v,a")); break;
		case 0x29: my_stprintf_s(buffer, buffer_len, _T("gta a,a")); break;
		case 0x2a: my_stprintf_s(buffer, buffer_len, _T("gta b,a")); break;
		case 0x2b: my_stprintf_s(buffer, buffer_len, _T("gta c,a")); break;
		case 0x2c: my_stprintf_s(buffer, buffer_len, _T("gta d,a")); break;
		case 0x2d: my_stprintf_s(buffer, buffer_len, _T("gta e,a")); break;
		case 0x2e: my_stprintf_s(buffer, buffer_len, _T("gta h,a")); break;
		case 0x2f: my_stprintf_s(buffer, buffer_len, _T("gta l,a")); break;
		case 0x30: my_stprintf_s(buffer, buffer_len, _T("subnb v,a")); break;
		case 0x31: my_stprintf_s(buffer, buffer_len, _T("subnb a,a")); break;
		case 0x32: my_stprintf_s(buffer, buffer_len, _T("subnb b,a")); break;
		case 0x33: my_stprintf_s(buffer, buffer_len, _T("subnb c,a")); break;
		case 0x34: my_stprintf_s(buffer, buffer_len, _T("subnb d,a")); break;
		case 0x35: my_stprintf_s(buffer, buffer_len, _T("subnb e,a")); break;
		case 0x36: my_stprintf_s(buffer, buffer_len, _T("subnb h,a")); break;
		case 0x37: my_stprintf_s(buffer, buffer_len, _T("subnb l,a")); break;
		case 0x38: my_stprintf_s(buffer, buffer_len, _T("lta v,a")); break;
		case 0x39: my_stprintf_s(buffer, buffer_len, _T("lta a,a")); break;
		case 0x3a: my_stprintf_s(buffer, buffer_len, _T("lta b,a")); break;
		case 0x3b: my_stprintf_s(buffer, buffer_len, _T("lta c,a")); break;
		case 0x3c: my_stprintf_s(buffer, buffer_len, _T("lta d,a")); break;
		case 0x3d: my_stprintf_s(buffer, buffer_len, _T("lta e,a")); break;
		case 0x3e: my_stprintf_s(buffer, buffer_len, _T("lta h,a")); break;
		case 0x3f: my_stprintf_s(buffer, buffer_len, _T("lta l,a")); break;
		case 0x40: my_stprintf_s(buffer, buffer_len, _T("add v,a")); break;
		case 0x41: my_stprintf_s(buffer, buffer_len, _T("add a,a")); break;
		case 0x42: my_stprintf_s(buffer, buffer_len, _T("add b,a")); break;
		case 0x43: my_stprintf_s(buffer, buffer_len, _T("add c,a")); break;
		case 0x44: my_stprintf_s(buffer, buffer_len, _T("add d,a")); break;
		case 0x45: my_stprintf_s(buffer, buffer_len, _T("add e,a")); break;
		case 0x46: my_stprintf_s(buffer, buffer_len, _T("add h,a")); break;
		case 0x47: my_stprintf_s(buffer, buffer_len, _T("add l,a")); break;
		case 0x50: my_stprintf_s(buffer, buffer_len, _T("adc v,a")); break;
		case 0x51: my_stprintf_s(buffer, buffer_len, _T("adc a,a")); break;
		case 0x52: my_stprintf_s(buffer, buffer_len, _T("adc b,a")); break;
		case 0x53: my_stprintf_s(buffer, buffer_len, _T("adc c,a")); break;
		case 0x54: my_stprintf_s(buffer, buffer_len, _T("adc d,a")); break;
		case 0x55: my_stprintf_s(buffer, buffer_len, _T("adc e,a")); break;
		case 0x56: my_stprintf_s(buffer, buffer_len, _T("adc h,a")); break;
		case 0x57: my_stprintf_s(buffer, buffer_len, _T("adc l,a")); break;
		case 0x60: my_stprintf_s(buffer, buffer_len, _T("sub v,a")); break;
		case 0x61: my_stprintf_s(buffer, buffer_len, _T("sub a,a")); break;
		case 0x62: my_stprintf_s(buffer, buffer_len, _T("sub b,a")); break;
		case 0x63: my_stprintf_s(buffer, buffer_len, _T("sub c,a")); break;
		case 0x64: my_stprintf_s(buffer, buffer_len, _T("sub d,a")); break;
		case 0x65: my_stprintf_s(buffer, buffer_len, _T("sub e,a")); break;
		case 0x66: my_stprintf_s(buffer, buffer_len, _T("sub h,a")); break;
		case 0x67: my_stprintf_s(buffer, buffer_len, _T("sub l,a")); break;
		case 0x68: my_stprintf_s(buffer, buffer_len, _T("nea v,a")); break;
		case 0x69: my_stprintf_s(buffer, buffer_len, _T("nea a,a")); break;
		case 0x6a: my_stprintf_s(buffer, buffer_len, _T("nea b,a")); break;
		case 0x6b: my_stprintf_s(buffer, buffer_len, _T("nea c,a")); break;
		case 0x6c: my_stprintf_s(buffer, buffer_len, _T("nea d,a")); break;
		case 0x6d: my_stprintf_s(buffer, buffer_len, _T("nea e,a")); break;
		case 0x6e: my_stprintf_s(buffer, buffer_len, _T("nea h,a")); break;
		case 0x6f: my_stprintf_s(buffer, buffer_len, _T("nea l,a")); break;
		case 0x70: my_stprintf_s(buffer, buffer_len, _T("sbb v,a")); break;
		case 0x71: my_stprintf_s(buffer, buffer_len, _T("sbb a,a")); break;
		case 0x72: my_stprintf_s(buffer, buffer_len, _T("sbb b,a")); break;
		case 0x73: my_stprintf_s(buffer, buffer_len, _T("sbb c,a")); break;
		case 0x74: my_stprintf_s(buffer, buffer_len, _T("sbb d,a")); break;
		case 0x75: my_stprintf_s(buffer, buffer_len, _T("sbb e,a")); break;
		case 0x76: my_stprintf_s(buffer, buffer_len, _T("sbb h,a")); break;
		case 0x77: my_stprintf_s(buffer, buffer_len, _T("sbb l,a")); break;
		case 0x78: my_stprintf_s(buffer, buffer_len, _T("eqa v,a")); break;
		case 0x79: my_stprintf_s(buffer, buffer_len, _T("eqa a,a")); break;
		case 0x7a: my_stprintf_s(buffer, buffer_len, _T("eqa b,a")); break;
		case 0x7b: my_stprintf_s(buffer, buffer_len, _T("eqa c,a")); break;
		case 0x7c: my_stprintf_s(buffer, buffer_len, _T("eqa d,a")); break;
		case 0x7d: my_stprintf_s(buffer, buffer_len, _T("eqa e,a")); break;
		case 0x7e: my_stprintf_s(buffer, buffer_len, _T("eqa h,a")); break;
		case 0x7f: my_stprintf_s(buffer, buffer_len, _T("eqa l,a")); break;
		case 0x88: my_stprintf_s(buffer, buffer_len, _T("ana a,v")); break;
		case 0x89: my_stprintf_s(buffer, buffer_len, _T("ana a,a")); break;
		case 0x8a: my_stprintf_s(buffer, buffer_len, _T("ana a,b")); break;
		case 0x8b: my_stprintf_s(buffer, buffer_len, _T("ana a,c")); break;
		case 0x8c: my_stprintf_s(buffer, buffer_len, _T("ana a,d")); break;
		case 0x8d: my_stprintf_s(buffer, buffer_len, _T("ana a,e")); break;
		case 0x8e: my_stprintf_s(buffer, buffer_len, _T("ana a,h")); break;
		case 0x8f: my_stprintf_s(buffer, buffer_len, _T("ana a,l")); break;
		case 0x90: my_stprintf_s(buffer, buffer_len, _T("xra a,v")); break;
		case 0x91: my_stprintf_s(buffer, buffer_len, _T("xra a,a")); break;
		case 0x92: my_stprintf_s(buffer, buffer_len, _T("xra a,b")); break;
		case 0x93: my_stprintf_s(buffer, buffer_len, _T("xra a,c")); break;
		case 0x94: my_stprintf_s(buffer, buffer_len, _T("xra a,d")); break;
		case 0x95: my_stprintf_s(buffer, buffer_len, _T("xra a,e")); break;
		case 0x96: my_stprintf_s(buffer, buffer_len, _T("xra a,h")); break;
		case 0x97: my_stprintf_s(buffer, buffer_len, _T("xra a,l")); break;
		case 0x98: my_stprintf_s(buffer, buffer_len, _T("ora a,v")); break;
		case 0x99: my_stprintf_s(buffer, buffer_len, _T("ora a,a")); break;
		case 0x9a: my_stprintf_s(buffer, buffer_len, _T("ora a,b")); break;
		case 0x9b: my_stprintf_s(buffer, buffer_len, _T("ora a,c")); break;
		case 0x9c: my_stprintf_s(buffer, buffer_len, _T("ora a,d")); break;
		case 0x9d: my_stprintf_s(buffer, buffer_len, _T("ora a,e")); break;
		case 0x9e: my_stprintf_s(buffer, buffer_len, _T("ora a,h")); break;
		case 0x9f: my_stprintf_s(buffer, buffer_len, _T("ora a,l")); break;
		case 0xa0: my_stprintf_s(buffer, buffer_len, _T("addnc a,v")); break;
		case 0xa1: my_stprintf_s(buffer, buffer_len, _T("addnc a,a")); break;
		case 0xa2: my_stprintf_s(buffer, buffer_len, _T("addnc a,b")); break;
		case 0xa3: my_stprintf_s(buffer, buffer_len, _T("addnc a,c")); break;
		case 0xa4: my_stprintf_s(buffer, buffer_len, _T("addnc a,d")); break;
		case 0xa5: my_stprintf_s(buffer, buffer_len, _T("addnc a,e")); break;
		case 0xa6: my_stprintf_s(buffer, buffer_len, _T("addnc a,h")); break;
		case 0xa7: my_stprintf_s(buffer, buffer_len, _T("addnc a,l")); break;
		case 0xa8: my_stprintf_s(buffer, buffer_len, _T("gta a,v")); break;
		case 0xa9: my_stprintf_s(buffer, buffer_len, _T("gta a,a")); break;
		case 0xaa: my_stprintf_s(buffer, buffer_len, _T("gta a,b")); break;
		case 0xab: my_stprintf_s(buffer, buffer_len, _T("gta a,c")); break;
		case 0xac: my_stprintf_s(buffer, buffer_len, _T("gta a,d")); break;
		case 0xad: my_stprintf_s(buffer, buffer_len, _T("gta a,e")); break;
		case 0xae: my_stprintf_s(buffer, buffer_len, _T("gta a,h")); break;
		case 0xaf: my_stprintf_s(buffer, buffer_len, _T("gta a,l")); break;
		case 0xb0: my_stprintf_s(buffer, buffer_len, _T("subnb a,v")); break;
		case 0xb1: my_stprintf_s(buffer, buffer_len, _T("subnb a,a")); break;
		case 0xb2: my_stprintf_s(buffer, buffer_len, _T("subnb a,b")); break;
		case 0xb3: my_stprintf_s(buffer, buffer_len, _T("subnb a,c")); break;
		case 0xb4: my_stprintf_s(buffer, buffer_len, _T("subnb a,d")); break;
		case 0xb5: my_stprintf_s(buffer, buffer_len, _T("subnb a,e")); break;
		case 0xb6: my_stprintf_s(buffer, buffer_len, _T("subnb a,h")); break;
		case 0xb7: my_stprintf_s(buffer, buffer_len, _T("subnb a,l")); break;
		case 0xb8: my_stprintf_s(buffer, buffer_len, _T("lta a,v")); break;
		case 0xb9: my_stprintf_s(buffer, buffer_len, _T("lta a,a")); break;
		case 0xba: my_stprintf_s(buffer, buffer_len, _T("lta a,b")); break;
		case 0xbb: my_stprintf_s(buffer, buffer_len, _T("lta a,c")); break;
		case 0xbc: my_stprintf_s(buffer, buffer_len, _T("lta a,d")); break;
		case 0xbd: my_stprintf_s(buffer, buffer_len, _T("lta a,e")); break;
		case 0xbe: my_stprintf_s(buffer, buffer_len, _T("lta a,h")); break;
		case 0xbf: my_stprintf_s(buffer, buffer_len, _T("lta a,l")); break;
		case 0xc0: my_stprintf_s(buffer, buffer_len, _T("add a,v")); break;
		case 0xc1: my_stprintf_s(buffer, buffer_len, _T("add a,a")); break;
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("add a,b")); break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("add a,c")); break;
		case 0xc4: my_stprintf_s(buffer, buffer_len, _T("add a,d")); break;
		case 0xc5: my_stprintf_s(buffer, buffer_len, _T("add a,e")); break;
		case 0xc6: my_stprintf_s(buffer, buffer_len, _T("add a,h")); break;
		case 0xc7: my_stprintf_s(buffer, buffer_len, _T("add a,l")); break;
		case 0xc8: my_stprintf_s(buffer, buffer_len, _T("ona a,v")); break;
		case 0xc9: my_stprintf_s(buffer, buffer_len, _T("ona a,a")); break;
		case 0xca: my_stprintf_s(buffer, buffer_len, _T("ona a,b")); break;
		case 0xcb: my_stprintf_s(buffer, buffer_len, _T("ona a,c")); break;
		case 0xcc: my_stprintf_s(buffer, buffer_len, _T("ona a,d")); break;
		case 0xcd: my_stprintf_s(buffer, buffer_len, _T("ona a,e")); break;
		case 0xce: my_stprintf_s(buffer, buffer_len, _T("ona a,h")); break;
		case 0xcf: my_stprintf_s(buffer, buffer_len, _T("ona a,l")); break;
		case 0xd0: my_stprintf_s(buffer, buffer_len, _T("adc a,v")); break;
		case 0xd1: my_stprintf_s(buffer, buffer_len, _T("adc a,a")); break;
		case 0xd2: my_stprintf_s(buffer, buffer_len, _T("adc a,b")); break;
		case 0xd3: my_stprintf_s(buffer, buffer_len, _T("adc a,c")); break;
		case 0xd4: my_stprintf_s(buffer, buffer_len, _T("adc a,d")); break;
		case 0xd5: my_stprintf_s(buffer, buffer_len, _T("adc a,e")); break;
		case 0xd6: my_stprintf_s(buffer, buffer_len, _T("adc a,h")); break;
		case 0xd7: my_stprintf_s(buffer, buffer_len, _T("adc a,l")); break;
		case 0xd8: my_stprintf_s(buffer, buffer_len, _T("offa a,v")); break;
		case 0xd9: my_stprintf_s(buffer, buffer_len, _T("offa a,a")); break;
		case 0xda: my_stprintf_s(buffer, buffer_len, _T("offa a,b")); break;
		case 0xdb: my_stprintf_s(buffer, buffer_len, _T("offa a,c")); break;
		case 0xdc: my_stprintf_s(buffer, buffer_len, _T("offa a,d")); break;
		case 0xdd: my_stprintf_s(buffer, buffer_len, _T("offa a,e")); break;
		case 0xde: my_stprintf_s(buffer, buffer_len, _T("offa a,h")); break;
		case 0xdf: my_stprintf_s(buffer, buffer_len, _T("offa a,l")); break;
		case 0xe0: my_stprintf_s(buffer, buffer_len, _T("sub a,v")); break;
		case 0xe1: my_stprintf_s(buffer, buffer_len, _T("sub a,a")); break;
		case 0xe2: my_stprintf_s(buffer, buffer_len, _T("sub a,b")); break;
		case 0xe3: my_stprintf_s(buffer, buffer_len, _T("sub a,c")); break;
		case 0xe4: my_stprintf_s(buffer, buffer_len, _T("sub a,d")); break;
		case 0xe5: my_stprintf_s(buffer, buffer_len, _T("sub a,e")); break;
		case 0xe6: my_stprintf_s(buffer, buffer_len, _T("sub a,h")); break;
		case 0xe7: my_stprintf_s(buffer, buffer_len, _T("sub a,l")); break;
		case 0xe8: my_stprintf_s(buffer, buffer_len, _T("nea a,v")); break;
		case 0xe9: my_stprintf_s(buffer, buffer_len, _T("nea a,a")); break;
		case 0xea: my_stprintf_s(buffer, buffer_len, _T("nea a,b")); break;
		case 0xeb: my_stprintf_s(buffer, buffer_len, _T("nea a,c")); break;
		case 0xec: my_stprintf_s(buffer, buffer_len, _T("nea a,d")); break;
		case 0xed: my_stprintf_s(buffer, buffer_len, _T("nea a,e")); break;
		case 0xee: my_stprintf_s(buffer, buffer_len, _T("nea a,h")); break;
		case 0xef: my_stprintf_s(buffer, buffer_len, _T("nea a,l")); break;
		case 0xf0: my_stprintf_s(buffer, buffer_len, _T("sbb a,v")); break;
		case 0xf1: my_stprintf_s(buffer, buffer_len, _T("sbb a,a")); break;
		case 0xf2: my_stprintf_s(buffer, buffer_len, _T("sbb a,b")); break;
		case 0xf3: my_stprintf_s(buffer, buffer_len, _T("sbb a,c")); break;
		case 0xf4: my_stprintf_s(buffer, buffer_len, _T("sbb a,d")); break;
		case 0xf5: my_stprintf_s(buffer, buffer_len, _T("sbb a,e")); break;
		case 0xf6: my_stprintf_s(buffer, buffer_len, _T("sbb a,h")); break;
		case 0xf7: my_stprintf_s(buffer, buffer_len, _T("sbb a,l")); break;
		case 0xf8: my_stprintf_s(buffer, buffer_len, _T("eqa a,v")); break;
		case 0xf9: my_stprintf_s(buffer, buffer_len, _T("eqa a,a")); break;
		case 0xfa: my_stprintf_s(buffer, buffer_len, _T("eqa a,b")); break;
		case 0xfb: my_stprintf_s(buffer, buffer_len, _T("eqa a,c")); break;
		case 0xfc: my_stprintf_s(buffer, buffer_len, _T("eqa a,d")); break;
		case 0xfd: my_stprintf_s(buffer, buffer_len, _T("eqa a,e")); break;
		case 0xfe: my_stprintf_s(buffer, buffer_len, _T("eqa a,h")); break;
		case 0xff: my_stprintf_s(buffer, buffer_len, _T("eqa a,l")); break;
		default: my_stprintf_s(buffer, buffer_len, _T("db 60h,%02xh"), b);
		}
		break;
	case 0x61: my_stprintf_s(buffer, buffer_len, _T("daa")); break;
	case 0x62: my_stprintf_s(buffer, buffer_len, _T("reti")); break;
	case 0x63: my_stprintf_s(buffer, buffer_len, _T("calb")); break;
	case 0x64:
		switch(b = getb()) {
		case 0x08: my_stprintf_s(buffer, buffer_len, _T("ani v,%02xh"), getb()); break;
		case 0x09: my_stprintf_s(buffer, buffer_len, _T("ani a,%02xh"), getb()); break;
		case 0x0a: my_stprintf_s(buffer, buffer_len, _T("ani b,%02xh"), getb()); break;
		case 0x0b: my_stprintf_s(buffer, buffer_len, _T("ani c,%02xh"), getb()); break;
		case 0x0c: my_stprintf_s(buffer, buffer_len, _T("ani d,%02xh"), getb()); break;
		case 0x0d: my_stprintf_s(buffer, buffer_len, _T("ani e,%02xh"), getb()); break;
		case 0x0e: my_stprintf_s(buffer, buffer_len, _T("ani h,%02xh"), getb()); break;
		case 0x0f: my_stprintf_s(buffer, buffer_len, _T("ani l,%02xh"), getb()); break;
		case 0x10: my_stprintf_s(buffer, buffer_len, _T("xri v,%02xh"), getb()); break;
		case 0x11: my_stprintf_s(buffer, buffer_len, _T("xri a,%02xh"), getb()); break;
		case 0x12: my_stprintf_s(buffer, buffer_len, _T("xri b,%02xh"), getb()); break;
		case 0x13: my_stprintf_s(buffer, buffer_len, _T("xri c,%02xh"), getb()); break;
		case 0x14: my_stprintf_s(buffer, buffer_len, _T("xri d,%02xh"), getb()); break;
		case 0x15: my_stprintf_s(buffer, buffer_len, _T("xri e,%02xh"), getb()); break;
		case 0x16: my_stprintf_s(buffer, buffer_len, _T("xri h,%02xh"), getb()); break;
		case 0x17: my_stprintf_s(buffer, buffer_len, _T("xri l,%02xh"), getb()); break;
		case 0x18: my_stprintf_s(buffer, buffer_len, _T("ori v,%02xh"), getb()); break;
		case 0x19: my_stprintf_s(buffer, buffer_len, _T("ori a,%02xh"), getb()); break;
		case 0x1a: my_stprintf_s(buffer, buffer_len, _T("ori b,%02xh"), getb()); break;
		case 0x1b: my_stprintf_s(buffer, buffer_len, _T("ori c,%02xh"), getb()); break;
		case 0x1c: my_stprintf_s(buffer, buffer_len, _T("ori d,%02xh"), getb()); break;
		case 0x1d: my_stprintf_s(buffer, buffer_len, _T("ori e,%02xh"), getb()); break;
		case 0x1e: my_stprintf_s(buffer, buffer_len, _T("ori h,%02xh"), getb()); break;
		case 0x1f: my_stprintf_s(buffer, buffer_len, _T("ori l,%02xh"), getb()); break;
		case 0x20: my_stprintf_s(buffer, buffer_len, _T("adinc v,%02xh"), getb()); break;
		case 0x21: my_stprintf_s(buffer, buffer_len, _T("adinc a,%02xh"), getb()); break;
		case 0x22: my_stprintf_s(buffer, buffer_len, _T("adinc b,%02xh"), getb()); break;
		case 0x23: my_stprintf_s(buffer, buffer_len, _T("adinc c,%02xh"), getb()); break;
		case 0x24: my_stprintf_s(buffer, buffer_len, _T("adinc d,%02xh"), getb()); break;
		case 0x25: my_stprintf_s(buffer, buffer_len, _T("adinc e,%02xh"), getb()); break;
		case 0x26: my_stprintf_s(buffer, buffer_len, _T("adinc h,%02xh"), getb()); break;
		case 0x27: my_stprintf_s(buffer, buffer_len, _T("adinc l,%02xh"), getb()); break;
		case 0x28: my_stprintf_s(buffer, buffer_len, _T("gti v,%02xh"), getb()); break;
		case 0x29: my_stprintf_s(buffer, buffer_len, _T("gti a,%02xh"), getb()); break;
		case 0x2a: my_stprintf_s(buffer, buffer_len, _T("gti b,%02xh"), getb()); break;
		case 0x2b: my_stprintf_s(buffer, buffer_len, _T("gti c,%02xh"), getb()); break;
		case 0x2c: my_stprintf_s(buffer, buffer_len, _T("gti d,%02xh"), getb()); break;
		case 0x2d: my_stprintf_s(buffer, buffer_len, _T("gti e,%02xh"), getb()); break;
		case 0x2e: my_stprintf_s(buffer, buffer_len, _T("gti h,%02xh"), getb()); break;
		case 0x2f: my_stprintf_s(buffer, buffer_len, _T("gti l,%02xh"), getb()); break;
		case 0x30: my_stprintf_s(buffer, buffer_len, _T("suinb v,%02xh"), getb()); break;
		case 0x31: my_stprintf_s(buffer, buffer_len, _T("suinb a,%02xh"), getb()); break;
		case 0x32: my_stprintf_s(buffer, buffer_len, _T("suinb b,%02xh"), getb()); break;
		case 0x33: my_stprintf_s(buffer, buffer_len, _T("suinb c,%02xh"), getb()); break;
		case 0x34: my_stprintf_s(buffer, buffer_len, _T("suinb d,%02xh"), getb()); break;
		case 0x35: my_stprintf_s(buffer, buffer_len, _T("suinb e,%02xh"), getb()); break;
		case 0x36: my_stprintf_s(buffer, buffer_len, _T("suinb h,%02xh"), getb()); break;
		case 0x37: my_stprintf_s(buffer, buffer_len, _T("suinb l,%02xh"), getb()); break;
		case 0x38: my_stprintf_s(buffer, buffer_len, _T("lti v,%02xh"), getb()); break;
		case 0x39: my_stprintf_s(buffer, buffer_len, _T("lti a,%02xh"), getb()); break;
		case 0x3a: my_stprintf_s(buffer, buffer_len, _T("lti b,%02xh"), getb()); break;
		case 0x3b: my_stprintf_s(buffer, buffer_len, _T("lti c,%02xh"), getb()); break;
		case 0x3c: my_stprintf_s(buffer, buffer_len, _T("lti d,%02xh"), getb()); break;
		case 0x3d: my_stprintf_s(buffer, buffer_len, _T("lti e,%02xh"), getb()); break;
		case 0x3e: my_stprintf_s(buffer, buffer_len, _T("lti h,%02xh"), getb()); break;
		case 0x3f: my_stprintf_s(buffer, buffer_len, _T("lti l,%02xh"), getb()); break;
		case 0x40: my_stprintf_s(buffer, buffer_len, _T("adi v,%02xh"), getb()); break;
		case 0x41: my_stprintf_s(buffer, buffer_len, _T("adi a,%02xh"), getb()); break;
		case 0x42: my_stprintf_s(buffer, buffer_len, _T("adi b,%02xh"), getb()); break;
		case 0x43: my_stprintf_s(buffer, buffer_len, _T("adi c,%02xh"), getb()); break;
		case 0x44: my_stprintf_s(buffer, buffer_len, _T("adi d,%02xh"), getb()); break;
		case 0x45: my_stprintf_s(buffer, buffer_len, _T("adi e,%02xh"), getb()); break;
		case 0x46: my_stprintf_s(buffer, buffer_len, _T("adi h,%02xh"), getb()); break;
		case 0x47: my_stprintf_s(buffer, buffer_len, _T("adi l,%02xh"), getb()); break;
		case 0x48: my_stprintf_s(buffer, buffer_len, _T("oni v,%02xh"), getb()); break;
		case 0x49: my_stprintf_s(buffer, buffer_len, _T("oni a,%02xh"), getb()); break;
		case 0x4a: my_stprintf_s(buffer, buffer_len, _T("oni b,%02xh"), getb()); break;
		case 0x4b: my_stprintf_s(buffer, buffer_len, _T("oni c,%02xh"), getb()); break;
		case 0x4c: my_stprintf_s(buffer, buffer_len, _T("oni d,%02xh"), getb()); break;
		case 0x4d: my_stprintf_s(buffer, buffer_len, _T("oni e,%02xh"), getb()); break;
		case 0x4e: my_stprintf_s(buffer, buffer_len, _T("oni h,%02xh"), getb()); break;
		case 0x4f: my_stprintf_s(buffer, buffer_len, _T("oni l,%02xh"), getb()); break;
		case 0x50: my_stprintf_s(buffer, buffer_len, _T("aci v,%02xh"), getb()); break;
		case 0x51: my_stprintf_s(buffer, buffer_len, _T("aci a,%02xh"), getb()); break;
		case 0x52: my_stprintf_s(buffer, buffer_len, _T("aci b,%02xh"), getb()); break;
		case 0x53: my_stprintf_s(buffer, buffer_len, _T("aci c,%02xh"), getb()); break;
		case 0x54: my_stprintf_s(buffer, buffer_len, _T("aci d,%02xh"), getb()); break;
		case 0x55: my_stprintf_s(buffer, buffer_len, _T("aci e,%02xh"), getb()); break;
		case 0x56: my_stprintf_s(buffer, buffer_len, _T("aci h,%02xh"), getb()); break;
		case 0x57: my_stprintf_s(buffer, buffer_len, _T("aci l,%02xh"), getb()); break;
		case 0x58: my_stprintf_s(buffer, buffer_len, _T("offi v,%02xh"), getb()); break;
		case 0x59: my_stprintf_s(buffer, buffer_len, _T("offi a,%02xh"), getb()); break;
		case 0x5a: my_stprintf_s(buffer, buffer_len, _T("offi b,%02xh"), getb()); break;
		case 0x5b: my_stprintf_s(buffer, buffer_len, _T("offi c,%02xh"), getb()); break;
		case 0x5c: my_stprintf_s(buffer, buffer_len, _T("offi d,%02xh"), getb()); break;
		case 0x5d: my_stprintf_s(buffer, buffer_len, _T("offi e,%02xh"), getb()); break;
		case 0x5e: my_stprintf_s(buffer, buffer_len, _T("offi h,%02xh"), getb()); break;
		case 0x5f: my_stprintf_s(buffer, buffer_len, _T("offi l,%02xh"), getb()); break;
		case 0x60: my_stprintf_s(buffer, buffer_len, _T("sui v,%02xh"), getb()); break;
		case 0x61: my_stprintf_s(buffer, buffer_len, _T("sui a,%02xh"), getb()); break;
		case 0x62: my_stprintf_s(buffer, buffer_len, _T("sui b,%02xh"), getb()); break;
		case 0x63: my_stprintf_s(buffer, buffer_len, _T("sui c,%02xh"), getb()); break;
		case 0x64: my_stprintf_s(buffer, buffer_len, _T("sui d,%02xh"), getb()); break;
		case 0x65: my_stprintf_s(buffer, buffer_len, _T("sui e,%02xh"), getb()); break;
		case 0x66: my_stprintf_s(buffer, buffer_len, _T("sui h,%02xh"), getb()); break;
		case 0x67: my_stprintf_s(buffer, buffer_len, _T("sui l,%02xh"), getb()); break;
		case 0x68: my_stprintf_s(buffer, buffer_len, _T("nei v,%02xh"), getb()); break;
		case 0x69: my_stprintf_s(buffer, buffer_len, _T("nei a,%02xh"), getb()); break;
		case 0x6a: my_stprintf_s(buffer, buffer_len, _T("nei b,%02xh"), getb()); break;
		case 0x6b: my_stprintf_s(buffer, buffer_len, _T("nei c,%02xh"), getb()); break;
		case 0x6c: my_stprintf_s(buffer, buffer_len, _T("nei d,%02xh"), getb()); break;
		case 0x6d: my_stprintf_s(buffer, buffer_len, _T("nei e,%02xh"), getb()); break;
		case 0x6e: my_stprintf_s(buffer, buffer_len, _T("nei h,%02xh"), getb()); break;
		case 0x6f: my_stprintf_s(buffer, buffer_len, _T("nei l,%02xh"), getb()); break;
		case 0x70: my_stprintf_s(buffer, buffer_len, _T("sbi v,%02xh"), getb()); break;
		case 0x71: my_stprintf_s(buffer, buffer_len, _T("sbi a,%02xh"), getb()); break;
		case 0x72: my_stprintf_s(buffer, buffer_len, _T("sbi b,%02xh"), getb()); break;
		case 0x73: my_stprintf_s(buffer, buffer_len, _T("sbi c,%02xh"), getb()); break;
		case 0x74: my_stprintf_s(buffer, buffer_len, _T("sbi d,%02xh"), getb()); break;
		case 0x75: my_stprintf_s(buffer, buffer_len, _T("sbi e,%02xh"), getb()); break;
		case 0x76: my_stprintf_s(buffer, buffer_len, _T("sbi h,%02xh"), getb()); break;
		case 0x77: my_stprintf_s(buffer, buffer_len, _T("sbi l,%02xh"), getb()); break;
		case 0x78: my_stprintf_s(buffer, buffer_len, _T("eqi v,%02xh"), getb()); break;
		case 0x79: my_stprintf_s(buffer, buffer_len, _T("eqi a,%02xh"), getb()); break;
		case 0x7a: my_stprintf_s(buffer, buffer_len, _T("eqi b,%02xh"), getb()); break;
		case 0x7b: my_stprintf_s(buffer, buffer_len, _T("eqi c,%02xh"), getb()); break;
		case 0x7c: my_stprintf_s(buffer, buffer_len, _T("eqi d,%02xh"), getb()); break;
		case 0x7d: my_stprintf_s(buffer, buffer_len, _T("eqi e,%02xh"), getb()); break;
		case 0x7e: my_stprintf_s(buffer, buffer_len, _T("eqi h,%02xh"), getb()); break;
		case 0x7f: my_stprintf_s(buffer, buffer_len, _T("eqi l,%02xh"), getb()); break;
		case 0x88: my_stprintf_s(buffer, buffer_len, _T("ani pa,%02xh"), getb()); break;
		case 0x89: my_stprintf_s(buffer, buffer_len, _T("ani pb,%02xh"), getb()); break;
		case 0x8a: my_stprintf_s(buffer, buffer_len, _T("ani pc,%02xh"), getb()); break;
		case 0x8b: my_stprintf_s(buffer, buffer_len, _T("ani mk,%02xh"), getb()); break;
		case 0x90: my_stprintf_s(buffer, buffer_len, _T("xri pa,%02xh"), getb()); break;
		case 0x91: my_stprintf_s(buffer, buffer_len, _T("xri pb,%02xh"), getb()); break;
		case 0x92: my_stprintf_s(buffer, buffer_len, _T("xri pc,%02xh"), getb()); break;
		case 0x93: my_stprintf_s(buffer, buffer_len, _T("xri mk,%02xh"), getb()); break;
		case 0x98: my_stprintf_s(buffer, buffer_len, _T("ori pa,%02xh"), getb()); break;
		case 0x99: my_stprintf_s(buffer, buffer_len, _T("ori pb,%02xh"), getb()); break;
		case 0x9a: my_stprintf_s(buffer, buffer_len, _T("ori pc,%02xh"), getb()); break;
		case 0x9b: my_stprintf_s(buffer, buffer_len, _T("ori mk,%02xh"), getb()); break;
		case 0xa0: my_stprintf_s(buffer, buffer_len, _T("adinc pa,%02xh"), getb()); break;
		case 0xa1: my_stprintf_s(buffer, buffer_len, _T("adinc pb,%02xh"), getb()); break;
		case 0xa2: my_stprintf_s(buffer, buffer_len, _T("adinc pc,%02xh"), getb()); break;
		case 0xa3: my_stprintf_s(buffer, buffer_len, _T("adinc mk,%02xh"), getb()); break;
		case 0xa8: my_stprintf_s(buffer, buffer_len, _T("gti pa,%02xh"), getb()); break;
		case 0xa9: my_stprintf_s(buffer, buffer_len, _T("gti pb,%02xh"), getb()); break;
		case 0xaa: my_stprintf_s(buffer, buffer_len, _T("gti pc,%02xh"), getb()); break;
		case 0xab: my_stprintf_s(buffer, buffer_len, _T("gti mk,%02xh"), getb()); break;
		case 0xb0: my_stprintf_s(buffer, buffer_len, _T("suinb pa,%02xh"), getb()); break;
		case 0xb1: my_stprintf_s(buffer, buffer_len, _T("suinb pb,%02xh"), getb()); break;
		case 0xb2: my_stprintf_s(buffer, buffer_len, _T("suinb pc,%02xh"), getb()); break;
		case 0xb3: my_stprintf_s(buffer, buffer_len, _T("suinb mk,%02xh"), getb()); break;
		case 0xb8: my_stprintf_s(buffer, buffer_len, _T("lti pa,%02xh"), getb()); break;
		case 0xb9: my_stprintf_s(buffer, buffer_len, _T("lti pb,%02xh"), getb()); break;
		case 0xba: my_stprintf_s(buffer, buffer_len, _T("lti pc,%02xh"), getb()); break;
		case 0xbb: my_stprintf_s(buffer, buffer_len, _T("lti mk,%02xh"), getb()); break;
		case 0xc0: my_stprintf_s(buffer, buffer_len, _T("adi pa,%02xh"), getb()); break;
		case 0xc1: my_stprintf_s(buffer, buffer_len, _T("adi pb,%02xh"), getb()); break;
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("adi pc,%02xh"), getb()); break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("adi mk,%02xh"), getb()); break;
		case 0xc8: my_stprintf_s(buffer, buffer_len, _T("oni pa,%02xh"), getb()); break;
		case 0xc9: my_stprintf_s(buffer, buffer_len, _T("oni pb,%02xh"), getb()); break;
		case 0xca: my_stprintf_s(buffer, buffer_len, _T("oni pc,%02xh"), getb()); break;
		case 0xcb: my_stprintf_s(buffer, buffer_len, _T("oni mk,%02xh"), getb()); break;
		case 0xd0: my_stprintf_s(buffer, buffer_len, _T("aci pa,%02xh"), getb()); break;
		case 0xd1: my_stprintf_s(buffer, buffer_len, _T("aci pb,%02xh"), getb()); break;
		case 0xd2: my_stprintf_s(buffer, buffer_len, _T("aci pc,%02xh"), getb()); break;
		case 0xd3: my_stprintf_s(buffer, buffer_len, _T("aci mk,%02xh"), getb()); break;
		case 0xd8: my_stprintf_s(buffer, buffer_len, _T("offi pa,%02xh"), getb()); break;
		case 0xd9: my_stprintf_s(buffer, buffer_len, _T("offi pb,%02xh"), getb()); break;
		case 0xda: my_stprintf_s(buffer, buffer_len, _T("offi pc,%02xh"), getb()); break;
		case 0xdb: my_stprintf_s(buffer, buffer_len, _T("offi mk,%02xh"), getb()); break;
		case 0xe0: my_stprintf_s(buffer, buffer_len, _T("sui pa,%02xh"), getb()); break;
		case 0xe1: my_stprintf_s(buffer, buffer_len, _T("sui pb,%02xh"), getb()); break;
		case 0xe2: my_stprintf_s(buffer, buffer_len, _T("sui pc,%02xh"), getb()); break;
		case 0xe3: my_stprintf_s(buffer, buffer_len, _T("sui mk,%02xh"), getb()); break;
		case 0xe8: my_stprintf_s(buffer, buffer_len, _T("nei pa,%02xh"), getb()); break;
		case 0xe9: my_stprintf_s(buffer, buffer_len, _T("nei pb,%02xh"), getb()); break;
		case 0xea: my_stprintf_s(buffer, buffer_len, _T("nei pc,%02xh"), getb()); break;
		case 0xeb: my_stprintf_s(buffer, buffer_len, _T("nei mk,%02xh"), getb()); break;
		case 0xf0: my_stprintf_s(buffer, buffer_len, _T("sbi pa,%02xh"), getb()); break;
		case 0xf1: my_stprintf_s(buffer, buffer_len, _T("sbi pb,%02xh"), getb()); break;
		case 0xf2: my_stprintf_s(buffer, buffer_len, _T("sbi pc,%02xh"), getb()); break;
		case 0xf3: my_stprintf_s(buffer, buffer_len, _T("sbi mk,%02xh"), getb()); break;
		case 0xf8: my_stprintf_s(buffer, buffer_len, _T("eqi pa,%02xh"), getb()); break;
		case 0xf9: my_stprintf_s(buffer, buffer_len, _T("eqi pb,%02xh"), getb()); break;
		case 0xfa: my_stprintf_s(buffer, buffer_len, _T("eqi pc,%02xh"), getb()); break;
		case 0xfb: my_stprintf_s(buffer, buffer_len, _T("eqi mk,%02xh"), getb()); break;
		default: my_stprintf_s(buffer, buffer_len, _T("db 64h,%02xh"), b);
		}
		break;
	case 0x65: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("neiw v.%02xh,%02xh"), wa, getb()); break;
	case 0x66: my_stprintf_s(buffer, buffer_len, _T("sui a,%02xh"), getb()); break;
	case 0x67: my_stprintf_s(buffer, buffer_len, _T("nei a,%02xh"), getb()); break;
	case 0x68: my_stprintf_s(buffer, buffer_len, _T("mvi v,%02xh"), getb()); break;
	case 0x69: my_stprintf_s(buffer, buffer_len, _T("mvi a,%02xh"), getb()); break;
	case 0x6a: my_stprintf_s(buffer, buffer_len, _T("mvi b,%02xh"), getb()); break;
	case 0x6b: my_stprintf_s(buffer, buffer_len, _T("mvi c,%02xh"), getb()); break;
	case 0x6c: my_stprintf_s(buffer, buffer_len, _T("mvi d,%02xh"), getb()); break;
	case 0x6d: my_stprintf_s(buffer, buffer_len, _T("mvi e,%02xh"), getb()); break;
	case 0x6e: my_stprintf_s(buffer, buffer_len, _T("mvi h,%02xh"), getb()); break;
	case 0x6f: my_stprintf_s(buffer, buffer_len, _T("mvi l,%02xh"), getb()); break;
	
	case 0x70:
		switch(b = getb()) {
		case 0x0e: my_stprintf_s(buffer, buffer_len, _T("sspd %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x0f: my_stprintf_s(buffer, buffer_len, _T("lspd %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x1e: my_stprintf_s(buffer, buffer_len, _T("sbcd %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x1f: my_stprintf_s(buffer, buffer_len, _T("lbcd %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x2e: my_stprintf_s(buffer, buffer_len, _T("sded %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x2f: my_stprintf_s(buffer, buffer_len, _T("lded %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x3e: my_stprintf_s(buffer, buffer_len, _T("shld %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x3f: my_stprintf_s(buffer, buffer_len, _T("lhld %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x68: my_stprintf_s(buffer, buffer_len, _T("mov v,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x69: my_stprintf_s(buffer, buffer_len, _T("mov a,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x6a: my_stprintf_s(buffer, buffer_len, _T("mov b,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x6b: my_stprintf_s(buffer, buffer_len, _T("mov c,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x6c: my_stprintf_s(buffer, buffer_len, _T("mov d,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x6d: my_stprintf_s(buffer, buffer_len, _T("mov e,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x6e: my_stprintf_s(buffer, buffer_len, _T("mov h,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x6f: my_stprintf_s(buffer, buffer_len, _T("mov l,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x78: my_stprintf_s(buffer, buffer_len, _T("mov %s,v"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x79: my_stprintf_s(buffer, buffer_len, _T("mov %s,a"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x7a: my_stprintf_s(buffer, buffer_len, _T("mov %s,b"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x7b: my_stprintf_s(buffer, buffer_len, _T("mov %s,c"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x7c: my_stprintf_s(buffer, buffer_len, _T("mov %s,d"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x7d: my_stprintf_s(buffer, buffer_len, _T("mov %s,e"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x7e: my_stprintf_s(buffer, buffer_len, _T("mov %s,h"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x7f: my_stprintf_s(buffer, buffer_len, _T("mov %s,l"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), getw())); break;
		case 0x89: my_stprintf_s(buffer, buffer_len, _T("anax b")); break;
		case 0x8a: my_stprintf_s(buffer, buffer_len, _T("anax d")); break;
		case 0x8b: my_stprintf_s(buffer, buffer_len, _T("anax h")); break;
		case 0x8c: my_stprintf_s(buffer, buffer_len, _T("anax d+")); break;
		case 0x8d: my_stprintf_s(buffer, buffer_len, _T("anax h+")); break;
		case 0x8e: my_stprintf_s(buffer, buffer_len, _T("anax d-")); break;
		case 0x8f: my_stprintf_s(buffer, buffer_len, _T("anax h-")); break;
		case 0x91: my_stprintf_s(buffer, buffer_len, _T("xrax b")); break;
		case 0x92: my_stprintf_s(buffer, buffer_len, _T("xrax d")); break;
		case 0x93: my_stprintf_s(buffer, buffer_len, _T("xrax h")); break;
		case 0x94: my_stprintf_s(buffer, buffer_len, _T("xrax d+")); break;
		case 0x95: my_stprintf_s(buffer, buffer_len, _T("xrax h+")); break;
		case 0x96: my_stprintf_s(buffer, buffer_len, _T("xrax d-")); break;
		case 0x97: my_stprintf_s(buffer, buffer_len, _T("xrax h-")); break;
		case 0x99: my_stprintf_s(buffer, buffer_len, _T("orax b")); break;
		case 0x9a: my_stprintf_s(buffer, buffer_len, _T("orax d")); break;
		case 0x9b: my_stprintf_s(buffer, buffer_len, _T("orax h")); break;
		case 0x9c: my_stprintf_s(buffer, buffer_len, _T("orax d+")); break;
		case 0x9d: my_stprintf_s(buffer, buffer_len, _T("orax h+")); break;
		case 0x9e: my_stprintf_s(buffer, buffer_len, _T("orax d-")); break;
		case 0x9f: my_stprintf_s(buffer, buffer_len, _T("orax h-")); break;
		case 0xa1: my_stprintf_s(buffer, buffer_len, _T("addncx b")); break;
		case 0xa2: my_stprintf_s(buffer, buffer_len, _T("addncx d")); break;
		case 0xa3: my_stprintf_s(buffer, buffer_len, _T("addncx h")); break;
		case 0xa4: my_stprintf_s(buffer, buffer_len, _T("addncx d+")); break;
		case 0xa5: my_stprintf_s(buffer, buffer_len, _T("addncx h+")); break;
		case 0xa6: my_stprintf_s(buffer, buffer_len, _T("addncx d-")); break;
		case 0xa7: my_stprintf_s(buffer, buffer_len, _T("addncx h-")); break;
		case 0xa9: my_stprintf_s(buffer, buffer_len, _T("gtax b")); break;
		case 0xaa: my_stprintf_s(buffer, buffer_len, _T("gtax d")); break;
		case 0xab: my_stprintf_s(buffer, buffer_len, _T("gtax h")); break;
		case 0xac: my_stprintf_s(buffer, buffer_len, _T("gtax d+")); break;
		case 0xad: my_stprintf_s(buffer, buffer_len, _T("gtax h+")); break;
		case 0xae: my_stprintf_s(buffer, buffer_len, _T("gtax d-")); break;
		case 0xaf: my_stprintf_s(buffer, buffer_len, _T("gtax h-")); break;
		case 0xb1: my_stprintf_s(buffer, buffer_len, _T("subnbx b")); break;
		case 0xb2: my_stprintf_s(buffer, buffer_len, _T("subnbx d")); break;
		case 0xb3: my_stprintf_s(buffer, buffer_len, _T("subnbx h")); break;
		case 0xb4: my_stprintf_s(buffer, buffer_len, _T("subnbx d+")); break;
		case 0xb5: my_stprintf_s(buffer, buffer_len, _T("subnbx h+")); break;
		case 0xb6: my_stprintf_s(buffer, buffer_len, _T("subnbx d-")); break;
		case 0xb7: my_stprintf_s(buffer, buffer_len, _T("subnbx h-")); break;
		case 0xb9: my_stprintf_s(buffer, buffer_len, _T("ltax b")); break;
		case 0xba: my_stprintf_s(buffer, buffer_len, _T("ltax d")); break;
		case 0xbb: my_stprintf_s(buffer, buffer_len, _T("ltax h")); break;
		case 0xbc: my_stprintf_s(buffer, buffer_len, _T("ltax d+")); break;
		case 0xbd: my_stprintf_s(buffer, buffer_len, _T("ltax h+")); break;
		case 0xbe: my_stprintf_s(buffer, buffer_len, _T("ltax d-")); break;
		case 0xbf: my_stprintf_s(buffer, buffer_len, _T("ltax h-")); break;
		case 0xc1: my_stprintf_s(buffer, buffer_len, _T("addx b")); break;
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("addx d")); break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("addx h")); break;
		case 0xc4: my_stprintf_s(buffer, buffer_len, _T("addx d+")); break;
		case 0xc5: my_stprintf_s(buffer, buffer_len, _T("addx h+")); break;
		case 0xc6: my_stprintf_s(buffer, buffer_len, _T("addx d-")); break;
		case 0xc7: my_stprintf_s(buffer, buffer_len, _T("addx h-")); break;
		case 0xc9: my_stprintf_s(buffer, buffer_len, _T("onax b")); break;
		case 0xca: my_stprintf_s(buffer, buffer_len, _T("onax d")); break;
		case 0xcb: my_stprintf_s(buffer, buffer_len, _T("onax h")); break;
		case 0xcc: my_stprintf_s(buffer, buffer_len, _T("onax d+")); break;
		case 0xcd: my_stprintf_s(buffer, buffer_len, _T("onax h+")); break;
		case 0xce: my_stprintf_s(buffer, buffer_len, _T("onax d-")); break;
		case 0xcf: my_stprintf_s(buffer, buffer_len, _T("onax h-")); break;
		case 0xd1: my_stprintf_s(buffer, buffer_len, _T("adcx b")); break;
		case 0xd2: my_stprintf_s(buffer, buffer_len, _T("adcx d")); break;
		case 0xd3: my_stprintf_s(buffer, buffer_len, _T("adcx h")); break;
		case 0xd4: my_stprintf_s(buffer, buffer_len, _T("adcx d+")); break;
		case 0xd5: my_stprintf_s(buffer, buffer_len, _T("adcx h+")); break;
		case 0xd6: my_stprintf_s(buffer, buffer_len, _T("adcx d-")); break;
		case 0xd7: my_stprintf_s(buffer, buffer_len, _T("adcx h-")); break;
		case 0xd9: my_stprintf_s(buffer, buffer_len, _T("offax b")); break;
		case 0xda: my_stprintf_s(buffer, buffer_len, _T("offax d")); break;
		case 0xdb: my_stprintf_s(buffer, buffer_len, _T("offax h")); break;
		case 0xdc: my_stprintf_s(buffer, buffer_len, _T("offax d+")); break;
		case 0xdd: my_stprintf_s(buffer, buffer_len, _T("offax h+")); break;
		case 0xde: my_stprintf_s(buffer, buffer_len, _T("offax d-")); break;
		case 0xdf: my_stprintf_s(buffer, buffer_len, _T("offax h-")); break;
		case 0xe1: my_stprintf_s(buffer, buffer_len, _T("subx b")); break;
		case 0xe2: my_stprintf_s(buffer, buffer_len, _T("subx d")); break;
		case 0xe3: my_stprintf_s(buffer, buffer_len, _T("subx h")); break;
		case 0xe4: my_stprintf_s(buffer, buffer_len, _T("subx d+")); break;
		case 0xe5: my_stprintf_s(buffer, buffer_len, _T("subx h+")); break;
		case 0xe6: my_stprintf_s(buffer, buffer_len, _T("subx d-")); break;
		case 0xe7: my_stprintf_s(buffer, buffer_len, _T("subx h-")); break;
		case 0xe9: my_stprintf_s(buffer, buffer_len, _T("neax b")); break;
		case 0xea: my_stprintf_s(buffer, buffer_len, _T("neax d")); break;
		case 0xeb: my_stprintf_s(buffer, buffer_len, _T("neax h")); break;
		case 0xec: my_stprintf_s(buffer, buffer_len, _T("neax d+")); break;
		case 0xed: my_stprintf_s(buffer, buffer_len, _T("neax h+")); break;
		case 0xee: my_stprintf_s(buffer, buffer_len, _T("neax d-")); break;
		case 0xef: my_stprintf_s(buffer, buffer_len, _T("neax h-")); break;
		case 0xf1: my_stprintf_s(buffer, buffer_len, _T("sbbx b")); break;
		case 0xf2: my_stprintf_s(buffer, buffer_len, _T("sbbx d")); break;
		case 0xf3: my_stprintf_s(buffer, buffer_len, _T("sbbx h")); break;
		case 0xf4: my_stprintf_s(buffer, buffer_len, _T("sbbx d+")); break;
		case 0xf5: my_stprintf_s(buffer, buffer_len, _T("sbbx h+")); break;
		case 0xf6: my_stprintf_s(buffer, buffer_len, _T("sbbx d-")); break;
		case 0xf7: my_stprintf_s(buffer, buffer_len, _T("sbbx h-")); break;
		case 0xf9: my_stprintf_s(buffer, buffer_len, _T("eqax b")); break;
		case 0xfa: my_stprintf_s(buffer, buffer_len, _T("eqax d")); break;
		case 0xfb: my_stprintf_s(buffer, buffer_len, _T("eqax h")); break;
		case 0xfc: my_stprintf_s(buffer, buffer_len, _T("eqax d+")); break;
		case 0xfd: my_stprintf_s(buffer, buffer_len, _T("eqax h+")); break;
		case 0xfe: my_stprintf_s(buffer, buffer_len, _T("eqax d-")); break;
		case 0xff: my_stprintf_s(buffer, buffer_len, _T("eqax h-")); break;
		default: my_stprintf_s(buffer, buffer_len, _T("db 70h,%02xh"), b);
		}
		break;
	case 0x71: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("mviw v.%02xh,%02xh"), wa, getb()); break;
	case 0x72: my_stprintf_s(buffer, buffer_len, _T("softi")); break;
	case 0x73: my_stprintf_s(buffer, buffer_len, _T("jb")); break;
	case 0x74:
		switch(b = getb()) {
		case 0x88: my_stprintf_s(buffer, buffer_len, _T("anaw v.%02xh"), getwa()); break;
		case 0x90: my_stprintf_s(buffer, buffer_len, _T("xraw v.%02xh"), getwa()); break;
		case 0x98: my_stprintf_s(buffer, buffer_len, _T("oraw v.%02xh"), getwa()); break;
		case 0xa0: my_stprintf_s(buffer, buffer_len, _T("addncw v.%02xh"), getwa()); break;
		case 0xa8: my_stprintf_s(buffer, buffer_len, _T("gtaw v.%02xh"), getwa()); break;
		case 0xb0: my_stprintf_s(buffer, buffer_len, _T("subnbw v.%02xh"), getwa()); break;
		case 0xb8: my_stprintf_s(buffer, buffer_len, _T("ltaw v.%02xh"), getwa()); break;
		case 0xc0: my_stprintf_s(buffer, buffer_len, _T("addw v.%02xh"), getwa()); break;
		case 0xc8: my_stprintf_s(buffer, buffer_len, _T("onaw v.%02xh"), getwa()); break;
		case 0xd0: my_stprintf_s(buffer, buffer_len, _T("adcw v.%02xh"), getwa()); break;
		case 0xd8: my_stprintf_s(buffer, buffer_len, _T("offaw v.%02xh"), getwa()); break;
		case 0xe0: my_stprintf_s(buffer, buffer_len, _T("subw v.%02xh"), getwa()); break;
		case 0xe8: my_stprintf_s(buffer, buffer_len, _T("neaw v.%02xh"), getwa()); break;
		case 0xf0: my_stprintf_s(buffer, buffer_len, _T("sbbw v.%02xh"), getwa()); break;
		case 0xf8: my_stprintf_s(buffer, buffer_len, _T("eqaw v.%02xh"), getwa()); break;
		default: my_stprintf_s(buffer, buffer_len, _T("db 74h,%02xh"), b);
		}
		break;
	case 0x75: wa = getwa(); my_stprintf_s(buffer, buffer_len, _T("eqiw v.%02xh,%02xh"), wa, getb()); break;
	case 0x76: my_stprintf_s(buffer, buffer_len, _T("sbi a,%02xh"), getb()); break;
	case 0x77: my_stprintf_s(buffer, buffer_len, _T("eqi a,%02xh"), getb()); break;
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		my_stprintf_s(buffer, buffer_len, _T("calf %3x"), 0x800 | ((b & 7) << 8) | getb()); break;
	
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		my_stprintf_s(buffer, buffer_len, _T("calt %02xh"), 0x80 | ((b & 0x3f) << 1)); break;
		
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		my_stprintf_s(buffer, buffer_len, _T("jr %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), pc + upd7801_dasm_ptr + (b & 0x1f))); break;
	
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		my_stprintf_s(buffer, buffer_len, _T("jr %s"), get_value_or_symbol(d_debugger->first_symbol, _T("%04xh"), pc + upd7801_dasm_ptr + ((b & 0x1f) - 0x20))); break;
	
	default: my_stprintf_s(buffer, buffer_len, _T("db %02xh"), b); break;
	}
	return upd7801_dasm_ptr;
}
#endif

void UPD7801::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_UPD7801_INTF0) {
		if(data & mask) {
			IRR |= INTF0;
		} else {
			IRR &= ~INTF0;
		}
	} else if(id == SIG_UPD7801_INTF1) {
		if(data & mask) {
			IRR |= INTF1;
		} else {
			IRR &= ~INTF1;
		}
	} else if(id == SIG_UPD7801_INTF2) {
		if((data & mask) && (MK & 0x20)) {
			IRR |= INTF2;
		} else if(!(data & mask) && !(MK & 0x20)) {
			IRR |= INTF2;
		}
	} else if(id == SIG_UPD7801_WAIT) {
		wait = ((data & mask) != 0);
	} else if(id == SIG_UPD7801_SI) {
		SI = ((data & mask) != 0);
	} else if(id == SIG_UPD7801_SCK) {
		bool newSCK = ((data & mask) != 0);
		if(SCK != newSCK) {
			if(!SIO_DISABLED && SIO_EXTCLOCK) {
				if(SCK && !newSCK) {
					write_signals(&outputs_so, (SR & 0x80) ? 0xffffffff : 0);
				} else if(!SCK && newSCK) {
					SR <<= 1;
					if(SI) SR |= 1;
					if(++sio_count == 8) {
						IRR |= INTFS;
						if(SAK) {
							SAK = 0;
							UPDATE_PORTC(0);
						}
						scount = sio_count = 0;
					}
				}
			}
			SCK = newSCK;
		}
	}
}

void UPD7801::OP()
{
	uint8_t ope = FETCH8();
	
	if((PSW & F_SK) && ope != 0x72) {
		// skip this mnemonic
		switch(ope) {
		case 0x48: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op48[ope].oplen - 2; period += op48[ope].clock; break;
		case 0x4c: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op4c[ope].oplen - 2; period += op4c[ope].clock; break;
		case 0x4d: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op4d[ope].oplen - 2; period += op4d[ope].clock; break;
		case 0x60: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op60[ope].oplen - 2; period += op60[ope].clock; break;
		case 0x64: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op64[ope].oplen - 2; period += op64[ope].clock; break;
		case 0x70: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op70[ope].oplen - 2; period += op70[ope].clock; break;
		case 0x74: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op74[ope].oplen - 2; period += op74[ope].clock; break;
		case 0x34: PSW &= ~(F_SK        | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		case 0x69: PSW &= ~(F_SK | F_L0       );                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		case 0x6f: PSW &= ~(F_SK        | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		default:   PSW &= ~(F_SK | F_L0 | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		}
		return;
	}
	period += op[ope].clock;
	
	switch(ope) {
	case 0x00:	// nop
		break;
	case 0x01:	// hlt
		HALT = 1; PC--; break;
	case 0x02:	// inx sp
		SP++; break;
	case 0x03:	// dcx sp
		SP--; break;
	case 0x04:	// lxi sp,word
		SP = FETCH16(); break;
	case 0x05:	// aniw wa,byte
		ANIW(); break;
	case 0x06:
		break;
	case 0x07:	// ani a,byte
		ANI(_A); break;
	case 0x08:	// ret
		PC = POP16(); break;
	case 0x09:	// sio
		SIO(); break;
	case 0x0a:	// mov a,b
		_A = _B; break;
	case 0x0b:	// mov a,c
		_A = _C; break;
	case 0x0c:	// mov a,d
		_A = _D; break;
	case 0x0d:	// mov a,e
		_A = _E; break;
	case 0x0e:	// mov a,h
		_A = _H; break;
	case 0x0f:	// mov a,l
		_A = _L; break;
	case 0x10:	// ex
		EX(); break;
	case 0x11:	// exx
		EXX(); break;
	case 0x12:	// inx b
		BC++; break;
	case 0x13:	// dcx b
		BC--; break;
	case 0x14:	// lxi b,word
		BC = FETCH16(); break;
	case 0x15:	// oriw wa,byte
		ORIW(); break;
	case 0x16:	// xri a,byte
		XRI(_A); break;
	case 0x17:	// ori a,byte
		ORI(_A); break;
	case 0x18:	// rets
		PC = POP16(); PSW |= F_SK; break;
	case 0x19:	// stm
		if(!TO) {
			TO = 0x10; UPDATE_PORTC(0);
		}
		STM(); break;
	case 0x1a:	// mov b, a
		_B = _A; break;
	case 0x1b:	// mov c, a
		_C = _A; break;
	case 0x1c:	// mov d, a
		_D = _A; break;
	case 0x1d:	// mov e, a
		_E = _A; break;
	case 0x1e:	// mov h, a
		_H = _A; break;
	case 0x1f:	// mov l, a
		_L = _A; break;
	case 0x20:	// inrw wa
		INRW(); break;
	case 0x21:	// table
		BC = RM16(PC + _A + 1); break;
	case 0x22:	// inx d
		DE++; break;
	case 0x23:	// dcx d
		DE--; break;
	case 0x24:	// lxi d,word
		DE = FETCH16(); break;
	case 0x25:	// gtiw wa,byte
		GTIW(); break;
	case 0x26:	// adinc a,byte
		ADINC(_A); break;
	case 0x27:	// gti a,byte
		GTI(_A); break;
	case 0x28:	// ldaw wa
		_A = RM8(FETCHWA()); break;
	case 0x29:	// ldax b
		_A = RM8(BC); break;
	case 0x2a:	// ldax d
		_A = RM8(DE); break;
	case 0x2b:	// ldax h
		_A = RM8(HL); break;
	case 0x2c:	// ldax d+
		_A = RM8(DE++);; break;
	case 0x2d:	// ldax h+
		_A = RM8(HL++); break;
	case 0x2e:	// ldax d-
		_A = RM8(DE--); break;
	case 0x2f:	// ldax h-
		_A = RM8(HL--); break;
	case 0x30:	// dcrw wa
		DCRW(); break;
	case 0x31:	// block
		BLOCK(); break;
	case 0x32:	// inx h
		HL++; break;
	case 0x33:	// dcx h
		HL--; break;
	case 0x34:	// lxi h,word
		if(PSW & F_L0) {
			PC += 2;
		} else {
			HL = FETCH16();
		}
		PSW = (PSW & ~F_L1) | F_L0; return;
	case 0x35:	// ltiw wa,byte
		LTIW(); break;
	case 0x36:	// suinb a,byte
		SUINB(_A); break;
	case 0x37:	// lti a,byte
		LTI(_A); break;
	case 0x38:	// staw wa
		WM8(FETCHWA(), _A); break;
	case 0x39:	// stax b
		WM8(BC, _A); break;
	case 0x3a:	// stax d
		WM8(DE, _A); break;
	case 0x3b:	// stax h
		WM8(HL, _A); break;
	case 0x3c:	// stax d+
		WM8(DE++, _A); break;
	case 0x3d:	// stax h+
		WM8(HL++, _A); break;
	case 0x3e:	// stax d-
		WM8(DE--, _A); break;
	case 0x3f:	// stax h-
		WM8(HL--, _A); break;
	case 0x40:
		break;
	case 0x41:	// inr a
		INR(_A); break;
	case 0x42:	// inr b
		INR(_B); break;
	case 0x43:	// inr c
		INR(_C); break;
	case 0x44:	// call word
		CALL(); break;
	case 0x45:	// oniw wa,byte
		ONIW(); break;
	case 0x46:	// adi a,byte
		ADI(_A); break;
	case 0x47:	// oni a,byte
		ONI(_A); break;
	case 0x48:	// 48 xx
		OP48(); break;
	case 0x49:	// mvix b,byte
		WM8(BC, FETCH8()); break;
	case 0x4a:	// mvix d,byte
		WM8(DE, FETCH8()); break;
	case 0x4b:	// mvix h,byte
		WM8(HL, FETCH8()); break;
	case 0x4c:	// 4c xx
		OP4C(); break;
	case 0x4d:	// 4d xx
		OP4D(); break;
	case 0x4e:	// jre
	case 0x4f:	// jre
		JRE(ope); break;
	case 0x50:
		break;
	case 0x51:	// dcr a
		DCR(_A); break;
	case 0x52:	// dcr b
		DCR(_B); break;
	case 0x53:	// dcr c
		DCR(_C); break;
	case 0x54:	// jmp word
		PC = FETCH16(); break;
	case 0x55:	// offiw wa,byte
		OFFIW(); break;
	case 0x56:	// aci a,byte
		ACI(_A); break;
	case 0x57:	// offi a,byte
		OFFI(_A); break;
	case 0x58:	// bit 0,wa
		BIT(0); break;
	case 0x59:	// bit 1,wa
		BIT(1); break;
	case 0x5a:	// bit 2,wa
		BIT(2); break;
	case 0x5b:	// bit 3,wa
		BIT(3); break;
	case 0x5c:	// bit 4,wa
		BIT(4); break;
	case 0x5d:	// bit 5,wa
		BIT(5); break;
	case 0x5e:	// bit 6,wa
		BIT(6); break;
	case 0x5f:	// bit 7,wa
		BIT(7); break;
	case 0x60:	// 60 xx
		OP60(); break;
	case 0x61:	// daa
		DAA(); break;
	case 0x62:	// reti
		PC = POP16(); PSW = POP8(); SIRQ = 0; return;
	case 0x63:	// calb
		PUSH16(PC); PC = BC; break;
	case 0x64:	// 64 xx
		OP64(); break;
	case 0x65:	// neiw wa,byte
		NEIW(); break;
	case 0x66:	// sui a,byte
		SUI(_A); break;
	case 0x67:	// nei a,byte
		NEI(_A); break;
	case 0x68:	// mvi v,byte
		_V = FETCH8(); 
		break;
	case 0x69:	// mvi a,byte
		if(PSW & F_L1) {
			PC++;
		} else {
			_A = FETCH8();
		}
		PSW = (PSW & ~F_L0) | F_L1; return;
	case 0x6a:	// mvi b,byte
		_B = FETCH8(); break;
	case 0x6b:	// mvi c,byte
		_C = FETCH8(); break;
	case 0x6c:	// mvi d,byte
		_D = FETCH8(); break;
	case 0x6d:	// mvi e,byte
		_E = FETCH8(); break;
	case 0x6e:	// mvi h,byte
		_H = FETCH8(); break;
	case 0x6f:	// mvi l,byte
		if(PSW & F_L0) {
			PC++;
		} else {
			_L = FETCH8();
		}
		PSW = (PSW & ~F_L1) | F_L0; return;
	case 0x70:	// 70 xx
		OP70(); break;
	case 0x71:	// mviw wa,byte
		MVIW(); break;
	case 0x72:	// softi
		PUSH8(PSW); PUSH16(PC); PSW &= ~F_SK; SIRQ = 1; PC = 0x0060; break;
	case 0x73:	// jb
		PC = BC; break;
	case 0x74:	// 74xx
		OP74(); break;
	case 0x75:	// eqiw wa,byte
		EQIW(); break;
	case 0x76:	// sbi a,byte
		SBI(_A); break;
	case 0x77:	// eqi a,byte
		EQI(_A); break;
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:	// calf
		CALF(ope); break;
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:	// calt
		CALT(ope); break;
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:	// jr
		PC += ope & 0x1f; break;
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:	// jr
		PC -= 0x20 - (ope & 0x1f); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
	PSW &= ~(F_L0 | F_L1);
}

void UPD7801::OP48()
{
	uint8_t ope = FETCH8();
	period += op48[ope].clock;
	
	switch(ope) {
	case 0x00:	// skit intf0
		SKIT(INTF0); break;
	case 0x01:	// skit intft
		SKIT(INTFT); break;
	case 0x02:	// skit intf1
		SKIT(INTF1); break;
	case 0x03:	// skit intf2
		SKIT(INTF2); break;
	case 0x04:	// skit intfs
		SKIT(INTFS); break;
	case 0x0a:	// sk cy
		SK(F_CY); break;
	case 0x0c:	// sk z
		SK(F_Z); break;
	case 0x0e:	// push v
		PUSH16(VA); break;
	case 0x0f:	// pop v
		VA = POP16(); break;
	case 0x10:	// sknit intf0
		SKNIT(INTF0); break;
	case 0x11:	// sknit intft
		SKNIT(INTFT); break;
	case 0x12:	// sknit intf1
		SKNIT(INTF1); break;
	case 0x13:	// sknit intf2
		SKNIT(INTF2); break;
	case 0x14:	// sknit intfs
		SKNIT(INTFS); break;
	case 0x1a:	// skn cy
		SKN(F_CY); break;
	case 0x1c:	// skn z
		SKN(F_Z); break;
	case 0x1e:	// push b
		PUSH16(BC); break;
	case 0x1f:	// pop b
		BC = POP16(); break;
	case 0x20:	// ei
		IFF = 3; break;
	case 0x24:	// di
		IFF = 0; break;
	case 0x2a:	// clc
		PSW &= ~F_CY; break;
	case 0x2b:	// stc
		PSW |= F_CY; break;
	case 0x2c:	// pen
		PEN(); break;
	case 0x2d:	// pex
		PEX(); break;
	case 0x2e:	// push d
		PUSH16(DE); break;
	case 0x2f:	// pop d
		DE = POP16(); break;
	case 0x30:	// rll a
		RLL(_A); break;
	case 0x31:	// rlr a
		RLR(_A); break;
	case 0x32:	// rll c
		RLL(_C); break;
	case 0x33:	// rlr c
		RLR(_C); break;
	case 0x34:	// sll a
		SLL(_A); break;
	case 0x35:	// slr a
		SLR(_A); break;
	case 0x36:	// sll c
		SLL(_C); break;
	case 0x37:	// slr c
		SLR(_C); break;
	case 0x38:	// rld
		RLD(); break;
	case 0x39:	// rrd
		RRD(); break;
	case 0x3c:	// per
		PER(); break;
	case 0x3e:	// push h
		PUSH16(HL); break;
	case 0x3f:	// pop h
		HL = POP16(); break;
	default:
		this->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 48 %2x\n"), prevPC, ope);
	}
}

void UPD7801::OP4C()
{
	uint8_t ope = FETCH8();
	period += op4c[ope].clock;
	
	switch(ope) {
	case 0xc0:	// mov a,pa
		_A = IN8(P_A); break;
	case 0xc1:	// mov a,pb
		_A = IN8(P_B); break;
	case 0xc2:	// mov a,pc
		_A = IN8(P_C); break;
	case 0xc3:	// mov a,mk
		_A = MK; break;
	case 0xc4:	// mov a,mb ?
		_A = MB; break;
	case 0xc5:	// mov a,mc ?
		_A = MC; break;
	case 0xc6:	// mov a,tm0 ?
		_A = TM0; break;
	case 0xc7:	// mov a,tm1 ?
		_A = TM1; break;
	case 0xc8:	// mov a,s
		if(!SAK) {
			SAK = 8; UPDATE_PORTC(0);
		}
		_A = SR; break;
	default:
		if(ope < 0xc0) {
			// in byte
			UPDATE_PORTC(0x20);
			_A = RM8((_B << 8) | ope);
			UPDATE_PORTC(0);
		} else {
			this->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 4c %2x\n"), prevPC, ope);
		}
	}
}

void UPD7801::OP4D()
{
	uint8_t ope = FETCH8();
	period += op4d[ope].clock;
	
	switch(ope) {
	case 0xc0:	// mov pa,a
		OUT8(P_A, _A); break;
	case 0xc1:	// mov pb,a
		OUT8(P_B, _A); break;
	case 0xc2:	// mov pc,a
		OUT8(P_C, _A); break;
	case 0xc3:	// mov mk,a
		MK = _A; break;
	case 0xc4:	// mov mb,a
		MB = _A; break;
	case 0xc5:	// mov mc,a
		if(MC != _A) {
			MC = _A; UPDATE_PORTC(0); break;
		}
		MC = _A; break;
	case 0xc6:	// mov tm0,a
		TM0 = _A; break;
	case 0xc7:	// mov tm1,a
		TM1 = _A; break;
	case 0xc8:	// mov s,a
		if(!SAK) {
			SAK = 8; UPDATE_PORTC(0);
		}
		SR = _A; break;
	default:
		if(ope < 0xc0) {
			// out byte
			UPDATE_PORTC(0x20);
			WM8((_B << 8) | ope, _A);
			UPDATE_PORTC(0);
		} else {
			this->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 4d %2x\n"), prevPC, ope);
		}
	}
}

void UPD7801::OP60()
{
	uint8_t ope = FETCH8();
	period += op60[ope].clock;
	
	switch(ope) {
	case 0x08:	// ana v,a
		ANA(_V, _A); break;
	case 0x09:	// ana a,a
		ANA(_A, _A); break;
	case 0x0a:	// ana b,a
		ANA(_B, _A); break;
	case 0x0b:	// ana c,a
		ANA(_C, _A); break;
	case 0x0c:	// ana d,a
		ANA(_D, _A); break;
	case 0x0d:	// ana e,a
		ANA(_E, _A); break;
	case 0x0e:	// ana h,a
		ANA(_H, _A); break;
	case 0x0f:	// ana l,a
		ANA(_L, _A); break;
	case 0x10:	// xra v,a
		XRA(_V, _A); break;
	case 0x11:	// xra a,a
		XRA(_A, _A); break;
	case 0x12:	// xra b,a
		XRA(_B, _A); break;
	case 0x13:	// xra c,a
		XRA(_C, _A); break;
	case 0x14:	// xra d,a
		XRA(_D, _A); break;
	case 0x15:	// xra e,a
		XRA(_E, _A); break;
	case 0x16:	// xra h,a
		XRA(_H, _A); break;
	case 0x17:	// xra l,a
		XRA(_L, _A); break;
	case 0x18:	// ora v,a
		ORA(_V, _A); break;
	case 0x19:	// ora a,a
		ORA(_A, _A); break;
	case 0x1a:	// ora b,a
		ORA(_B, _A); break;
	case 0x1b:	// ora c,a
		ORA(_C, _A); break;
	case 0x1c:	// ora d,a
		ORA(_D, _A); break;
	case 0x1d:	// ora e,a
		ORA(_E, _A); break;
	case 0x1e:	// ora h,a
		ORA(_H, _A); break;
	case 0x1f:	// ora l,a
		ORA(_L, _A); break;
	case 0x20:	// addnc v,a
		ADDNC(_V, _A); break;
	case 0x21:	// addnc a,a
		ADDNC(_A, _A); break;
	case 0x22:	// addnc b,a
		ADDNC(_B, _A); break;
	case 0x23:	// addnc c,a
		ADDNC(_C, _A); break;
	case 0x24:	// addnc d,a
		ADDNC(_D, _A); break;
	case 0x25:	// addnc e,a
		ADDNC(_E, _A); break;
	case 0x26:	// addnc h,a
		ADDNC(_H, _A); break;
	case 0x27:	// addnc l,a
		ADDNC(_L, _A); break;
	case 0x28:	// gta v,a
		GTA(_V, _A); break;
	case 0x29:	// gta a,a
		GTA(_A, _A); break;
	case 0x2a:	// gta b,a
		GTA(_B, _A); break;
	case 0x2b:	// gta c,a
		GTA(_C, _A); break;
	case 0x2c:	// gta d,a
		GTA(_D, _A); break;
	case 0x2d:	// gta e,a
		GTA(_E, _A); break;
	case 0x2e:	// gta h,a
		GTA(_H, _A); break;
	case 0x2f:	// gta l,a
		GTA(_L, _A); break;
	case 0x30:	// subnb v,a
		SUBNB(_V, _A); break;
	case 0x31:	// subnb a,a
		SUBNB(_A, _A); break;
	case 0x32:	// subnb b,a
		SUBNB(_B, _A); break;
	case 0x33:	// subnb c,a
		SUBNB(_C, _A); break;
	case 0x34:	// subnb d,a
		SUBNB(_D, _A); break;
	case 0x35:	// subnb e,a
		SUBNB(_E, _A); break;
	case 0x36:	// subnb h,a
		SUBNB(_H, _A); break;
	case 0x37:	// subnb l,a
		SUBNB(_L, _A); break;
	case 0x38:	// lta v,a
		LTA(_V, _A); break;
	case 0x39:	// lta a,a
		LTA(_A, _A); break;
	case 0x3a:	// lta b,a
		LTA(_B, _A); break;
	case 0x3b:	// lta c,a
		LTA(_C, _A); break;
	case 0x3c:	// lta d,a
		LTA(_D, _A); break;
	case 0x3d:	// lta e,a
		LTA(_E, _A); break;
	case 0x3e:	// lta h,a
		LTA(_H, _A); break;
	case 0x3f:	// lta l,a
		LTA(_L, _A); break;
	case 0x40:	// add v,a
		ADD(_V, _A); break;
	case 0x41:	// add a,a
		ADD(_A, _A); break;
	case 0x42:	// add b,a
		ADD(_B, _A); break;
	case 0x43:	// add c,a
		ADD(_C, _A); break;
	case 0x44:	// add d,a
		ADD(_D, _A); break;
	case 0x45:	// add e,a
		ADD(_E, _A); break;
	case 0x46:	// add h,a
		ADD(_H, _A); break;
	case 0x47:	// add l,a
		ADD(_L, _A); break;
	case 0x50:	// adc v,a
		ADC(_V, _A); break;
	case 0x51:	// adc a,a
		ADC(_A, _A); break;
	case 0x52:	// adc b,a
		ADC(_B, _A); break;
	case 0x53:	// adc c,a
		ADC(_C, _A); break;
	case 0x54:	// adc d,a
		ADC(_D, _A); break;
	case 0x55:	// adc e,a
		ADC(_E, _A); break;
	case 0x56:	// adc h,a
		ADC(_H, _A); break;
	case 0x57:	// adc l,a
		ADC(_L, _A); break;
	case 0x60:	// sub v,a
		SUB(_V, _A); break;
	case 0x61:	// sub a,a
		SUB(_A, _A); break;
	case 0x62:	// sub b,a
		SUB(_B, _A); break;
	case 0x63:	// sub c,a
		SUB(_C, _A); break;
	case 0x64:	// sub d,a
		SUB(_D, _A); break;
	case 0x65:	// sub e,a
		SUB(_E, _A); break;
	case 0x66:	// sub h,a
		SUB(_H, _A); break;
	case 0x67:	// sub l,a
		SUB(_L, _A); break;
	case 0x68:	// nea v,a
		NEA(_V, _A); break;
	case 0x69:	// nea a,a
		NEA(_A, _A); break;
	case 0x6a:	// nea b,a
		NEA(_B, _A); break;
	case 0x6b:	// nea c,a
		NEA(_C, _A); break;
	case 0x6c:	// nea d,a
		NEA(_D, _A); break;
	case 0x6d:	// nea e,a
		NEA(_E, _A); break;
	case 0x6e:	// nea h,a
		NEA(_H, _A); break;
	case 0x6f:	// nea l,a
		NEA(_L, _A); break;
	case 0x70:	// sbb v,a
		SBB(_V, _A); break;
	case 0x71:	// sbb a,a
		SBB(_A, _A); break;
	case 0x72:	// sbb b,a
		SBB(_B, _A); break;
	case 0x73:	// sbb c,a
		SBB(_C, _A); break;
	case 0x74:	// sbb d,a
		SBB(_D, _A); break;
	case 0x75:	// sbb e,a
		SBB(_E, _A); break;
	case 0x76:	// sbb h,a
		SBB(_H, _A); break;
	case 0x77:	// sbb l,a
		SBB(_L, _A); break;
	case 0x78:	// eqa v,a
		EQA(_V, _A); break;
	case 0x79:	// eqa a,a
		EQA(_A, _A); break;
	case 0x7a:	// eqa b,a
		EQA(_B, _A); break;
	case 0x7b:	// eqa c,a
		EQA(_C, _A); break;
	case 0x7c:	// eqa d,a
		EQA(_D, _A); break;
	case 0x7d:	// eqa e,a
		EQA(_E, _A); break;
	case 0x7e:	// eqa h,a
		EQA(_H, _A); break;
	case 0x7f:	// eqa l,a
		EQA(_L, _A); break;
	case 0x88:	// ana a,v
		ANA(_A, _V); break;
	case 0x89:	// ana a,a
		ANA(_A, _A); break;
	case 0x8a:	// ana a,b
		ANA(_A, _B); break;
	case 0x8b:	// ana a,c
		ANA(_A, _C); break;
	case 0x8c:	// ana a,d
		ANA(_A, _D); break;
	case 0x8d:	// ana a,e
		ANA(_A, _E); break;
	case 0x8e:	// ana a,h
		ANA(_A, _H); break;
	case 0x8f:	// ana a,l
		ANA(_A, _L); break;
	case 0x90:	// xra a,v
		XRA(_A, _V); break;
	case 0x91:	// xra a,a
		XRA(_A, _A); break;
	case 0x92:	// xra a,b
		XRA(_A, _B); break;
	case 0x93:	// xra a,c
		XRA(_A, _C); break;
	case 0x94:	// xra a,d
		XRA(_A, _D); break;
	case 0x95:	// xra a,e
		XRA(_A, _E); break;
	case 0x96:	// xra a,h
		XRA(_A, _H); break;
	case 0x97:	// xra a,l
		XRA(_A, _L); break;
	case 0x98:	// ora a,v
		ORA(_A, _V); break;
	case 0x99:	// ora a,a
		ORA(_A, _A); break;
	case 0x9a:	// ora a,b
		ORA(_A, _B); break;
	case 0x9b:	// ora a,c
		ORA(_A, _C); break;
	case 0x9c:	// ora a,d
		ORA(_A, _D); break;
	case 0x9d:	// ora a,e
		ORA(_A, _E); break;
	case 0x9e:	// ora a,h
		ORA(_A, _H); break;
	case 0x9f:	// ora a,l
		ORA(_A, _L); break;
	case 0xa0:	// addnc a,v
		ADDNC(_A, _V); break;
	case 0xa1:	// addnc a,a
		ADDNC(_A, _A); break;
	case 0xa2:	// addnc a,b
		ADDNC(_A, _B); break;
	case 0xa3:	// addnc a,c
		ADDNC(_A, _C); break;
	case 0xa4:	// addnc a,d
		ADDNC(_A, _D); break;
	case 0xa5:	// addnc a,e
		ADDNC(_A, _E); break;
	case 0xa6:	// addnc a,h
		ADDNC(_A, _H); break;
	case 0xa7:	// addnc a,l
		ADDNC(_A, _L); break;
	case 0xa8:	// gta a,v
		GTA(_A, _V); break;
	case 0xa9:	// gta a,a
		GTA(_A, _A); break;
	case 0xaa:	// gta a,b
		GTA(_A, _B); break;
	case 0xab:	// gta a,c
		GTA(_A, _C); break;
	case 0xac:	// gta a,d
		GTA(_A, _D); break;
	case 0xad:	// gta a,e
		GTA(_A, _E); break;
	case 0xae:	// gta a,h
		GTA(_A, _H); break;
	case 0xaf:	// gta a,l
		GTA(_A, _L); break;
	case 0xb0:	// subnb a,v
		SUBNB(_A, _V); break;
	case 0xb1:	// subnb a,a
		SUBNB(_A, _A); break;
	case 0xb2:	// subnb a,b
		SUBNB(_A, _B); break;
	case 0xb3:	// subnb a,c
		SUBNB(_A, _C); break;
	case 0xb4:	// subnb a,d
		SUBNB(_A, _D); break;
	case 0xb5:	// subnb a,e
		SUBNB(_A, _E); break;
	case 0xb6:	// subnb a,h
		SUBNB(_A, _H); break;
	case 0xb7:	// subnb a,l
		SUBNB(_A, _L); break;
	case 0xb8:	// lta a,v
		LTA(_A, _V); break;
	case 0xb9:	// lta a,a
		LTA(_A, _A); break;
	case 0xba:	// lta a,b
		LTA(_A, _B); break;
	case 0xbb:	// lta a,c
		LTA(_A, _C); break;
	case 0xbc:	// lta a,d
		LTA(_A, _D); break;
	case 0xbd:	// lta a,e
		LTA(_A, _E); break;
	case 0xbe:	// lta a,h
		LTA(_A, _H); break;
	case 0xbf:	// lta a,l
		LTA(_A, _L); break;
	case 0xc0:	// add a,v
		ADD(_A, _V); break;
	case 0xc1:	// add a,a
		ADD(_A, _A); break;
	case 0xc2:	// add a,b
		ADD(_A, _B); break;
	case 0xc3:	// add a,c
		ADD(_A, _C); break;
	case 0xc4:	// add a,d
		ADD(_A, _D); break;
	case 0xc5:	// add a,e
		ADD(_A, _E); break;
	case 0xc6:	// add a,h
		ADD(_A, _H); break;
	case 0xc7:	// add a,l
		ADD(_A, _L); break;
	case 0xc8:	// ona a,v
		ONA(_A, _V); break;
	case 0xc9:	// ona a,a
		ONA(_A, _A); break;
	case 0xca:	// ona a,b
		ONA(_A, _B); break;
	case 0xcb:	// ona a,c
		ONA(_A, _C); break;
	case 0xcc:	// ona a,d
		ONA(_A, _D); break;
	case 0xcd:	// ona a,e
		ONA(_A, _E); break;
	case 0xce:	// ona a,h
		ONA(_A, _H); break;
	case 0xcf:	// ona a,l
		ONA(_A, _L); break;
	case 0xd0:	// adc a,v
		ADC(_A, _V); break;
	case 0xd1:	// adc a,a
		ADC(_A, _A); break;
	case 0xd2:	// adc a,b
		ADC(_A, _B); break;
	case 0xd3:	// adc a,c
		ADC(_A, _C); break;
	case 0xd4:	// adc a,d
		ADC(_A, _D); break;
	case 0xd5:	// adc a,e
		ADC(_A, _E); break;
	case 0xd6:	// adc a,h
		ADC(_A, _H); break;
	case 0xd7:	// adc a,l
		ADC(_A, _L); break;
	case 0xd8:	// offa a,v
		OFFA(_A, _V); break;
	case 0xd9:	// offa a,a
		OFFA(_A, _A); break;
	case 0xda:	// offa a,b
		OFFA(_A, _B); break;
	case 0xdb:	// offa a,c
		OFFA(_A, _C); break;
	case 0xdc:	// offa a,d
		OFFA(_A, _D); break;
	case 0xdd:	// offa a,e
		OFFA(_A, _E); break;
	case 0xde:	// offa a,h
		OFFA(_A, _H); break;
	case 0xdf:	// offa a,l
		OFFA(_A, _L); break;
	case 0xe0:	// sub a,v
		SUB(_A, _V); break;
	case 0xe1:	// sub a,a
		SUB(_A, _A); break;
	case 0xe2:	// sub a,b
		SUB(_A, _B); break;
	case 0xe3:	// sub a,c
		SUB(_A, _C); break;
	case 0xe4:	// sub a,d
		SUB(_A, _D); break;
	case 0xe5:	// sub a,e
		SUB(_A, _E); break;
	case 0xe6:	// sub a,h
		SUB(_A, _H); break;
	case 0xe7:	// sub a,l
		SUB(_A, _L); break;
	case 0xe8:	// nea a,v
		NEA(_A, _V); break;
	case 0xe9:	// nea a,a
		NEA(_A, _A); break;
	case 0xea:	// nea a,b
		NEA(_A, _B); break;
	case 0xeb:	// nea a,c
		NEA(_A, _C); break;
	case 0xec:	// nea a,d
		NEA(_A, _D); break;
	case 0xed:	// nea a,e
		NEA(_A, _E); break;
	case 0xee:	// nea a,h
		NEA(_A, _H); break;
	case 0xef:	// nea a,l
		NEA(_A, _L); break;
	case 0xf0:	// sbb a,v
		SBB(_A, _V); break;
	case 0xf1:	// sbb a,a
		SBB(_A, _A); break;
	case 0xf2:	// sbb a,b
		SBB(_A, _B); break;
	case 0xf3:	// sbb a,c
		SBB(_A, _C); break;
	case 0xf4:	// sbb a,d
		SBB(_A, _D); break;
	case 0xf5:	// sbb a,e
		SBB(_A, _E); break;
	case 0xf6:	// sbb a,h
		SBB(_A, _H); break;
	case 0xf7:	// sbb a,l
		SBB(_A, _L); break;
	case 0xf8:	// eqa a,v
		EQA(_A, _V); break;
	case 0xf9:	// eqa a,a
		EQA(_A, _A); break;
	case 0xfa:	// eqa a,b
		EQA(_A, _B); break;
	case 0xfb:	// eqa a,c
		EQA(_A, _C); break;
	case 0xfc:	// eqa a,d
		EQA(_A, _D); break;
	case 0xfd:	// eqa a,e
		EQA(_A, _E); break;
	case 0xfe:	// eqa a,h
		EQA(_A, _H); break;
	case 0xff:	// eqa a,l
		EQA(_A, _L); break;
	default:
		this->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 60 %2x\n"), prevPC, ope);
	}
}

void UPD7801::OP64()
{
	uint8_t ope = FETCH8();
	period += op64[ope].clock;
	
	switch(ope) {
	case 0x08:	// ani v,byte
		ANI(_V); break;
	case 0x09:	// ani a,byte
		ANI(_A); break;
	case 0x0a:	// ani b,byte
		ANI(_B); break;
	case 0x0b:	// ani c,byte
		ANI(_C); break;
	case 0x0c:	// ani d,byte
		ANI(_D); break;
	case 0x0d:	// ani e,byte
		ANI(_E); break;
	case 0x0e:	// ani h,byte
		ANI(_H); break;
	case 0x0f:	// ani l,byte
		ANI(_L); break;
	case 0x10:	// xri v,byte
		XRI(_V); break;
	case 0x11:	// xri a,byte
		XRI(_A); break;
	case 0x12:	// xri b,byte
		XRI(_B); break;
	case 0x13:	// xri c,byte
		XRI(_C); break;
	case 0x14:	// xri d,byte
		XRI(_D); break;
	case 0x15:	// xri e,byte
		XRI(_E); break;
	case 0x16:	// xri h,byte
		XRI(_H); break;
	case 0x17:	// xri l,byte
		XRI(_L); break;
	case 0x18:	// ori v,byte
		ORI(_V); break;
	case 0x19:	// ori a,byte
		ORI(_A); break;
	case 0x1a:	// ori b,byte
		ORI(_B); break;
	case 0x1b:	// ori c,byte
		ORI(_C); break;
	case 0x1c:	// ori d,byte
		ORI(_D); break;
	case 0x1d:	// ori e,byte
		ORI(_E); break;
	case 0x1e:	// ori h,byte
		ORI(_H); break;
	case 0x1f:	// ori l,byte
		ORI(_L); break;
	case 0x20:	// adinc v,byte
		ADINC(_V); break;
	case 0x21:	// adinc a,byte
		ADINC(_A); break;
	case 0x22:	// adinc b,byte
		ADINC(_B); break;
	case 0x23:	// adinc c,byte
		ADINC(_C); break;
	case 0x24:	// adinc d,byte
		ADINC(_D); break;
	case 0x25:	// adinc e,byte
		ADINC(_E); break;
	case 0x26:	// adinc h,byte
		ADINC(_H); break;
	case 0x27:	// adinc l,byte
		ADINC(_L); break;
	case 0x28:	// gti v,byte
		GTI(_V); break;
	case 0x29:	// gti a,byte
		GTI(_A); break;
	case 0x2a:	// gti b,byte
		GTI(_B); break;
	case 0x2b:	// gti c,byte
		GTI(_C); break;
	case 0x2c:	// gti d,byte
		GTI(_D); break;
	case 0x2d:	// gti e,byte
		GTI(_E); break;
	case 0x2e:	// gti h,byte
		GTI(_H); break;
	case 0x2f:	// gti l,byte
		GTI(_L); break;
	case 0x30:	// suinb v,byte
		SUINB(_V); break;
	case 0x31:	// suinb a,byte
		SUINB(_A); break;
	case 0x32:	// suinb b,byte
		SUINB(_B); break;
	case 0x33:	// suinb c,byte
		SUINB(_C); break;
	case 0x34:	// suinb d,byte
		SUINB(_D); break;
	case 0x35:	// suinb e,byte
		SUINB(_E); break;
	case 0x36:	// suinb h,byte
		SUINB(_H); break;
	case 0x37:	// suinb l,byte
		SUINB(_L); break;
	case 0x38:	// lti v,byte
		LTI(_V); break;
	case 0x39:	// lti a,byte
		LTI(_A); break;
	case 0x3a:	// lti b,byte
		LTI(_B); break;
	case 0x3b:	// lti c,byte
		LTI(_C); break;
	case 0x3c:	// lti d,byte
		LTI(_D); break;
	case 0x3d:	// lti e,byte
		LTI(_E); break;
	case 0x3e:	// lti h,byte
		LTI(_H); break;
	case 0x3f:	// lti l,byte
		LTI(_L); break;
	case 0x40:	// adi v,byte
		ADI(_V); break;
	case 0x41:	// adi a,byte
		ADI(_A); break;
	case 0x42:	// adi b,byte
		ADI(_B); break;
	case 0x43:	// adi c,byte
		ADI(_C); break;
	case 0x44:	// adi d,byte
		ADI(_D); break;
	case 0x45:	// adi e,byte
		ADI(_E); break;
	case 0x46:	// adi h,byte
		ADI(_H); break;
	case 0x47:	// adi l,byte
		ADI(_L); break;
	case 0x48:	// oni v,byte
		ONI(_V); break;
	case 0x49:	// oni a,byte
		ONI(_A); break;
	case 0x4a:	// oni b,byte
		ONI(_B); break;
	case 0x4b:	// oni c,byte
		ONI(_C); break;
	case 0x4c:	// oni d,byte
		ONI(_D); break;
	case 0x4d:	// oni e,byte
		ONI(_E); break;
	case 0x4e:	// oni h,byte
		ONI(_H); break;
	case 0x4f:	// oni l,byte
		ONI(_L); break;
	case 0x50:	// aci v,byte
		ACI(_V); break;
	case 0x51:	// aci a,byte
		ACI(_A); break;
	case 0x52:	// aci b,byte
		ACI(_B); break;
	case 0x53:	// aci c,byte
		ACI(_C); break;
	case 0x54:	// aci d,byte
		ACI(_D); break;
	case 0x55:	// aci e,byte
		ACI(_E); break;
	case 0x56:	// aci h,byte
		ACI(_H); break;
	case 0x57:	// aci l,byte
		ACI(_L); break;
	case 0x58:	// offi v,byte
		OFFI(_V); break;
	case 0x59:	// offi a,byte
		OFFI(_A); break;
	case 0x5a:	// offi b,byte
		OFFI(_B); break;
	case 0x5b:	// offi c,byte
		OFFI(_C); break;
	case 0x5c:	// offi d,byte
		OFFI(_D); break;
	case 0x5d:	// offi e,byte
		OFFI(_E); break;
	case 0x5e:	// offi h,byte
		OFFI(_H); break;
	case 0x5f:	// offi l,byte
		OFFI(_L); break;
	case 0x60:	// sui v,byte
		SUI(_V); break;
	case 0x61:	// sui a,byte
		SUI(_A); break;
	case 0x62:	// sui b,byte
		SUI(_B); break;
	case 0x63:	// sui c,byte
		SUI(_C); break;
	case 0x64:	// sui d,byte
		SUI(_D); break;
	case 0x65:	// sui e,byte
		SUI(_E); break;
	case 0x66:	// sui h,byte
		SUI(_H); break;
	case 0x67:	// sui l,byte
		SUI(_L); break;
	case 0x68:	// nei v,byte
		NEI(_V); break;
	case 0x69:	// nei a,byte
		NEI(_A); break;
	case 0x6a:	// nei b,byte
		NEI(_B); break;
	case 0x6b:	// nei c,byte
		NEI(_C); break;
	case 0x6c:	// nei d,byte
		NEI(_D); break;
	case 0x6d:	// nei e,byte
		NEI(_E); break;
	case 0x6e:	// nei h,byte
		NEI(_H); break;
	case 0x6f:	// nei l,byte
		NEI(_L); break;
	case 0x70:	// sbi v,byte
		SBI(_V); break;
	case 0x71:	// sbi a,byte
		SBI(_A); break;
	case 0x72:	// sbi b,byte
		SBI(_B); break;
	case 0x73:	// sbi c,byte
		SBI(_C); break;
	case 0x74:	// sbi d,byte
		SBI(_D); break;
	case 0x75:	// sbi e,byte
		SBI(_E); break;
	case 0x76:	// sbi h,byte
		SBI(_H); break;
	case 0x77:	// sbi l,byte
		SBI(_L); break;
	case 0x78:	// eqi v,byte
		EQI(_V); break;
	case 0x79:	// eqi a,byte
		EQI(_A); break;
	case 0x7a:	// eqi b,byte
		EQI(_B); break;
	case 0x7b:	// eqi c,byte
		EQI(_C); break;
	case 0x7c:	// eqi d,byte
		EQI(_D); break;
	case 0x7d:	// eqi e,byte
		EQI(_E); break;
	case 0x7e:	// eqi h,byte
		EQI(_H); break;
	case 0x7f:	// eqi l,byte
		EQI(_L); break;
	case 0x88:	// ani pa,byte
		ANI_IO(P_A); break;
	case 0x89:	// ani pb,byte
		ANI_IO(P_B); break;
	case 0x8a:	// ani pc,byte
		ANI_IO(P_C); break;
	case 0x8b:	// ani mk,byte
		ANI(MK); break;
	case 0x90:	// xri pa,byte
		XRI_IO(P_A); break;
	case 0x91:	// xri pb,byte
		XRI_IO(P_B); break;
	case 0x92:	// xri pc,byte
		XRI_IO(P_C); break;
	case 0x93:	// xri mk,byte
		XRI(MK); break;
	case 0x98:	// ori pa,byte
		ORI_IO(P_A); break;
	case 0x99:	// ori pb,byte
		ORI_IO(P_B); break;
	case 0x9a:	// ori pc,byte
		ORI_IO(P_C); break;
	case 0x9b:	// ori mk,byte
		ORI(MK); break;
	case 0xa0:	// adinc pa,byte
		ADINC_IO(P_A); break;
	case 0xa1:	// adinc pb,byte
		ADINC_IO(P_B); break;
	case 0xa2:	// adinc pc,byte
		ADINC_IO(P_C); break;
	case 0xa3:	// adinc mk,byte
		ADINC(MK); break;
	case 0xa8:	// gti pa,byte
		GTI_IO(P_A); break;
	case 0xa9:	// gti pb,byte
		GTI_IO(P_B); break;
	case 0xaa:	// gti pc,byte
		GTI_IO(P_C); break;
	case 0xab:	// gti mk,byte
		GTI(MK); break;
	case 0xb0:	// suinb pa,byte
		SUINB_IO(P_A); break;
	case 0xb1:	// suinb pb,byte
		SUINB_IO(P_B); break;
	case 0xb2:	// suinb pc,byte
		SUINB_IO(P_C); break;
	case 0xb3:	// suinb mk,byte
		SUINB(MK); break;
	case 0xb8:	// lti pa,byte
		LTI_IO(P_A); break;
	case 0xb9:	// lti pb,byte
		LTI_IO(P_B); break;
	case 0xba:	// lti pc,byte
		LTI_IO(P_C); break;
	case 0xbb:	// lti mk,byte
		LTI(MK); break;
	case 0xc0:	// adi pa,byte
		ADI_IO(P_A); break;
	case 0xc1:	// adi pb,byte
		ADI_IO(P_B); break;
	case 0xc2:	// adi pc,byte
		ADI_IO(P_C); break;
	case 0xc3:	// adi mk,byte
		ADI(MK); break;
	case 0xc8:	// oni pa,byte
		ONI_IO(P_A); break;
	case 0xc9:	// oni pb,byte
		ONI_IO(P_B); break;
	case 0xca:	// oni pc,byte
		ONI_IO(P_C); break;
	case 0xcb:	// oni mk,byte
		ONI(MK); break;
	case 0xd0:	// aci pa,byte
		ACI_IO(P_A); break;
	case 0xd1:	// aci pb,byte
		ACI_IO(P_B); break;
	case 0xd2:	// aci pc,byte
		ACI_IO(P_C); break;
	case 0xd3:	// aci mk,byte
		ACI(MK); break;
	case 0xd8:	// offi pa,byte
		OFFI_IO(P_A); break;
	case 0xd9:	// offi pb,byte
		OFFI_IO(P_B); break;
	case 0xda:	// offi pc,byte
		OFFI_IO(P_C); break;
	case 0xdb:	// offi mk,byte
		OFFI(MK); break;
	case 0xe0:	// sui pa,byte
		SUI_IO(P_A); break;
	case 0xe1:	// sui pb,byte
		SUI_IO(P_B); break;
	case 0xe2:	// sui pc,byte
		SUI_IO(P_C); break;
	case 0xe3:	// sui mk,byte
		SUI(MK); break;
	case 0xe8:	// nei pa,byte
		NEI_IO(P_A); break;
	case 0xe9:	// nei pb,byte
		NEI_IO(P_B); break;
	case 0xea:	// nei pc,byte
		NEI_IO(P_C); break;
	case 0xeb:	// nei mk,byte
		NEI(MK); break;
	case 0xf0:	// sbi pa,byte
		SBI_IO(P_A); break;
	case 0xf1:	// sbi pb,byte
		SBI_IO(P_B); break;
	case 0xf2:	// sbi pc,byte
		SBI_IO(P_C); break;
	case 0xf3:	// sbi mk,byte
		SBI(MK); break;
	case 0xf8:	// eqi pa,byte
		EQI_IO(P_A); break;
	case 0xf9:	// eqi pb,byte
		EQI_IO(P_B); break;
	case 0xfa:	// eqi pc,byte
		EQI_IO(P_C); break;
	case 0xfb:	// eqi mk,byte
		EQI(MK); break;
	default:
		this->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 64 %2x\n"), prevPC, ope);
	}
}

void UPD7801::OP70()
{
	uint8_t ope = FETCH8();
	period += op70[ope].clock;
	
	switch(ope) {
	case 0x0e:	// sspd word
		WM16(FETCH16(), SP); break;
	case 0x0f:	// lspd word
		SP = RM16(FETCH16()); break;
	case 0x1e:	// sbcd word
		WM16(FETCH16(), BC); break;
	case 0x1f:	// lbcd word
		BC = RM16(FETCH16()); break;
	case 0x2e:	// sded word
		WM16(FETCH16(), DE); break;
	case 0x2f:	// lded word
		DE = RM16(FETCH16()); break;
	case 0x3e:	// shld word
		WM16(FETCH16(), HL); break;
	case 0x3f:	// lhld word
		HL = RM16(FETCH16()); break;
	case 0x68:	// mov v,word
		_V = RM8(FETCH16()); 
		break;
	case 0x69:	// mov a,word
		_A = RM8(FETCH16()); break;
	case 0x6a:	// mov b,word
		_B = RM8(FETCH16()); break;
	case 0x6b:	// mov c,word
		_C = RM8(FETCH16()); break;
	case 0x6c:	// mov d,word
		_D = RM8(FETCH16()); break;
	case 0x6d:	// mov e,word
		_E = RM8(FETCH16()); break;
	case 0x6e:	// mov h,word
		_H = RM8(FETCH16()); break;
	case 0x6f:	// mov l,word
		_L = RM8(FETCH16()); break;
	case 0x78:	// mov word,v
		WM8(FETCH16(), _V); break;
	case 0x79:	// mov word,a
		WM8(FETCH16(), _A); break;
	case 0x7a:	// mov word,b
		WM8(FETCH16(), _B); break;
	case 0x7b:	// mov word,c
		WM8(FETCH16(), _C); break;
	case 0x7c:	// mov word,d
		WM8(FETCH16(), _D); break;
	case 0x7d:	// mov word,e
		WM8(FETCH16(), _E); break;
	case 0x7e:	// mov word,h
		WM8(FETCH16(), _H); break;
	case 0x7f:	// mov word,l
		WM8(FETCH16(), _L); break;
	case 0x89:	// anax b
		ANAX(BC); break;
	case 0x8a:	// anax d
		ANAX(DE); break;
	case 0x8b:	// anax h
		ANAX(HL); break;
	case 0x8c:	// anax d+
		ANAX(DE++); break;
	case 0x8d:	// anax h+
		ANAX(HL++); break;
	case 0x8e:	// anax d-
		ANAX(DE--); break;
	case 0x8f:	// anax h-
		ANAX(HL--); break;
	case 0x91:	// xrax b
		XRAX(BC); break;
	case 0x92:	// xrax d
		XRAX(DE); break;
	case 0x93:	// xrax h
		XRAX(HL); break;
	case 0x94:	// xrax d+
		XRAX(DE++); break;
	case 0x95:	// xrax h+
		XRAX(HL++); break;
	case 0x96:	// xrax d-
		XRAX(DE--); break;
	case 0x97:	// xrax h-
		XRAX(HL--); break;
	case 0x99:	// orax b
		ORAX(BC); break;
	case 0x9a:	// orax d
		ORAX(DE); break;
	case 0x9b:	// orax h
		ORAX(HL); break;
	case 0x9c:	// orax d+
		ORAX(DE++); break;
	case 0x9d:	// orax h+
		ORAX(HL++); break;
	case 0x9e:	// orax d-
		ORAX(DE--); break;
	case 0x9f:	// orax h-
		ORAX(HL--); break;
	case 0xa1:	// addncx b
		ADDNCX(BC); break;
	case 0xa2:	// addncx d
		ADDNCX(DE); break;
	case 0xa3:	// addncx h
		ADDNCX(HL); break;
	case 0xa4:	// addncx d+
		ADDNCX(DE++); break;
	case 0xa5:	// addncx h+
		ADDNCX(HL++); break;
	case 0xa6:	// addncx d-
		ADDNCX(DE--); break;
	case 0xa7:	// addncx h-
		ADDNCX(HL--); break;
	case 0xa9:	// gtax b
		GTAX(BC); break;
	case 0xaa:	// gtax d
		GTAX(DE); break;
	case 0xab:	// gtax h
		GTAX(HL); break;
	case 0xac:	// gtax d+
		GTAX(DE++); break;
	case 0xad:	// gtax h+
		GTAX(HL++); break;
	case 0xae:	// gtax d-
		GTAX(DE--); break;
	case 0xaf:	// gtax h-
		GTAX(HL--); break;
	case 0xb1:	// subnbx b
		SUBNBX(BC); break;
	case 0xb2:	// subnbx d
		SUBNBX(DE); break;
	case 0xb3:	// subnbx h
		SUBNBX(HL); break;
	case 0xb4:	// subnbx d+
		SUBNBX(DE++); break;
	case 0xb5:	// subnbx h+
		SUBNBX(HL++); break;
	case 0xb6:	// subnbx d-
		SUBNBX(DE--); break;
	case 0xb7:	// subnbx h-
		SUBNBX(HL--); break;
	case 0xb9:	// ltax b
		LTAX(BC); break;
	case 0xba:	// ltax d
		LTAX(DE); break;
	case 0xbb:	// ltax h
		LTAX(HL); break;
	case 0xbc:	// ltax d+
		LTAX(DE++); break;
	case 0xbd:	// ltax h+
		LTAX(HL++); break;
	case 0xbe:	// ltax d-
		LTAX(DE--); break;
	case 0xbf:	// ltax h-
		LTAX(HL--); break;
	case 0xc1:	// addx b
		ADDX(BC); break;
	case 0xc2:	// addx d
		ADDX(DE); break;
	case 0xc3:	// addx h
		ADDX(HL); break;
	case 0xc4:	// addx d+
		ADDX(DE++); break;
	case 0xc5:	// addx h+
		ADDX(HL++); break;
	case 0xc6:	// addx d-
		ADDX(DE--); break;
	case 0xc7:	// addx h-
		ADDX(HL--); break;
	case 0xc9:	// onax b
		ONAX(BC); break;
	case 0xca:	// onax d
		ONAX(DE); break;
	case 0xcb:	// onax h
		ONAX(HL); break;
	case 0xcc:	// onax d+
		ONAX(DE++); break;
	case 0xcd:	// onax h+
		ONAX(HL++); break;
	case 0xce:	// onax d-
		ONAX(DE--); break;
	case 0xcf:	// onax h-
		ONAX(HL--); break;
	case 0xd1:	// adcx b
		ADCX(BC); break;
	case 0xd2:	// adcx d
		ADCX(DE); break;
	case 0xd3:	// adcx h
		ADCX(HL); break;
	case 0xd4:	// adcx d+
		ADCX(DE++); break;
	case 0xd5:	// adcx h+
		ADCX(HL++); break;
	case 0xd6:	// adcx d-
		ADCX(DE--); break;
	case 0xd7:	// adcx h-
		ADCX(HL--); break;
	case 0xd9:	// offax b
		OFFAX(BC); break;
	case 0xda:	// offax d
		OFFAX(DE); break;
	case 0xdb:	// offax h
		OFFAX(HL); break;
	case 0xdc:	// offax d+
		OFFAX(DE++); break;
	case 0xdd:	// offax h+
		OFFAX(HL++); break;
	case 0xde:	// offax d-
		OFFAX(DE--); break;
	case 0xdf:	// offax h-
		OFFAX(HL--); break;
	case 0xe1:	// subx b
		SUBX(BC); break;
	case 0xe2:	// subx d
		SUBX(DE); break;
	case 0xe3:	// subx h
		SUBX(HL); break;
	case 0xe4:	// subx d+
		SUBX(DE++); break;
	case 0xe5:	// subx h+
		SUBX(HL++); break;
	case 0xe6:	// subx d-
		SUBX(DE--); break;
	case 0xe7:	// subx h-
		SUBX(HL--); break;
	case 0xe9:	// neax b
		NEAX(BC); break;
	case 0xea:	// neax d
		NEAX(DE); break;
	case 0xeb:	// neax h
		NEAX(HL); break;
	case 0xec:	// neax d+
		NEAX(DE++); break;
	case 0xed:	// neax h+
		NEAX(HL++); break;
	case 0xee:	// neax d-
		NEAX(DE--); break;
	case 0xef:	// neax h-
		NEAX(HL--); break;
	case 0xf1:	// sbbx b
		SBBX(BC); break;
	case 0xf2:	// sbbx d
		SBBX(DE); break;
	case 0xf3:	// sbbx h
		SBBX(HL); break;
	case 0xf4:	// sbbx d+
		SBBX(DE++); break;
	case 0xf5:	// sbbx h+
		SBBX(HL++); break;
	case 0xf6:	// sbbx d-
		SBBX(DE--); break;
	case 0xf7:	// sbbx h-
		SBBX(HL--); break;
	case 0xf9:	// eqax b
		EQAX(BC); break;
	case 0xfa:	// eqax d
		EQAX(DE); break;
	case 0xfb:	// eqax h
		EQAX(HL); break;
	case 0xfc:	// eqax d+
		EQAX(DE++); break;
	case 0xfd:	// eqax h+
		EQAX(HL++); break;
	case 0xfe:	// eqax d-
		EQAX(DE--); break;
	case 0xff:	// eqax h-
		EQAX(HL--); break;
	default:
		this->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 70 %2x\n"), prevPC, ope);
	}
}

void UPD7801::OP74()
{
	uint8_t ope = FETCH8();
	period += op74[ope].clock;
	
	switch(ope) {
	case 0x88:	// anaw wa
		ANAW(); break;
	case 0x90:	// xraw wa
		XRAW(); break;
	case 0x98:	// oraw wa
		ORAW(); break;
	case 0xa0:	// addncw wa
		ADDNCW(); break;
	case 0xa8:	// gtaw wa
		GTAW(); break;
	case 0xb0:	// subnbw wa
		SUBNBW(); break;
	case 0xb8:	// ltaw wa
		LTAW(); break;
	case 0xc0:	// addw wa
		ADDW(); break;
	case 0xc8:	// onaw wa
		ONAW(); break;
	case 0xd0:	// adcw wa
		ADCW(); break;
	case 0xd8:	// offaw wa
		OFFAW(); break;
	case 0xe0:	// subw wa
		SUBW(); break;
	case 0xe8:	// neaw wa
		NEAW(); break;
	case 0xf0:	// sbbw wa
		SBBW(); break;
	case 0xf8:	// eqaw wa
		EQAW(); break;
	default:
		this->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 74 %2x\n"), prevPC, ope);
	}
}

#define STATE_VERSION	4

bool UPD7801::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#ifdef USE_DEBUGGER
	state_fio->StateValue(total_count);
#endif
	state_fio->StateValue(count);
	state_fio->StateValue(period);
	state_fio->StateValue(scount);
	state_fio->StateValue(tcount);
	state_fio->StateValue(wait);
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateValue(SP);
	state_fio->StateValue(PC);
	state_fio->StateValue(prevPC);
	state_fio->StateValue(PSW);
	state_fio->StateValue(IRR);
	state_fio->StateValue(IFF);
	state_fio->StateValue(SIRQ);
	state_fio->StateValue(HALT);
	state_fio->StateValue(MK);
	state_fio->StateValue(MB);
	state_fio->StateValue(MC);
	state_fio->StateValue(TM0);
	state_fio->StateValue(TM1);
	state_fio->StateValue(SR);
	state_fio->StateValue(SAK);
	state_fio->StateValue(TO);
	state_fio->StateValue(HLDA);
	state_fio->StateValue(PORTC);
	state_fio->StateValue(SI);
	state_fio->StateValue(SCK);
	state_fio->StateValue(sio_count);
	
#ifdef USE_DEBUGGER
	// post process
	if(loading) {
		prev_total_count = total_count;
	}
#endif
	return true;
}

