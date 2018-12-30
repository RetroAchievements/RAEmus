#ifndef __2608INTF_H__
#define __2608INTF_H__

#include "fm.h"

struct YM2608interface
{
	read8_handler portAread;
	read8_handler portBread;
	write8_handler portAwrite;
	write8_handler portBwrite;
	void ( *handler )( int irq );	/* IRQ handler for the YM2608 */
	int pcmrom;		/* Delta-T memory region ram/rom */
};

/************************************************/
/* Chip 0 functions             */
/************************************************/
READ8_HANDLER( YM2608_status_port_0_A_r );
READ8_HANDLER( YM2608_status_port_0_B_r );
READ8_HANDLER( YM2608_read_port_0_r );
WRITE8_HANDLER( YM2608_control_port_0_A_w );
WRITE8_HANDLER( YM2608_control_port_0_B_w );
WRITE8_HANDLER( YM2608_data_port_0_A_w );
WRITE8_HANDLER( YM2608_data_port_0_B_w );
#if 1		/* QUASI88 */
extern int	YM2608_timer_over_0(int c);
extern void YM2608_set_volume_0(float volume);
extern void YM2608_AY8910_set_volume_0(float volume);
#endif		/* QUASI88 */

/************************************************/
/* Chip 1 functions             */
/************************************************/
READ8_HANDLER( YM2608_status_port_1_A_r );
READ8_HANDLER( YM2608_status_port_1_B_r );
READ8_HANDLER( YM2608_read_port_1_r );
WRITE8_HANDLER( YM2608_control_port_1_A_w );
WRITE8_HANDLER( YM2608_control_port_1_B_w );
WRITE8_HANDLER( YM2608_data_port_1_A_w );
WRITE8_HANDLER( YM2608_data_port_1_B_w );
#if 1		/* QUASI88 */
extern int	YM2608_timer_over_1(int c);
extern void YM2608_set_volume_1(float volume);
extern void YM2608_AY8910_set_volume_1(float volume);
#endif		/* QUASI88 */

#endif /* __2608INTF_H__ */
