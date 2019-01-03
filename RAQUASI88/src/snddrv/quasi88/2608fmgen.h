#ifndef __FMGEN2608_H__
#define __FMGEN2608_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "2608intf.h"


/************************************************/
/* Chip 0 functions				*/
/************************************************/
READ8_HANDLER( FMGEN2608_status_port_0_A_r );
READ8_HANDLER( FMGEN2608_status_port_0_B_r );
READ8_HANDLER( FMGEN2608_read_port_0_r );
WRITE8_HANDLER( FMGEN2608_control_port_0_A_w );
WRITE8_HANDLER( FMGEN2608_control_port_0_B_w );
WRITE8_HANDLER( FMGEN2608_data_port_0_A_w );
WRITE8_HANDLER( FMGEN2608_data_port_0_B_w );
extern void FMGEN2608_set_volume_0(float volume);

/************************************************/
/* Chip 1 functions				*/
/************************************************/
READ8_HANDLER( FMGEN2608_status_port_1_A_r );
READ8_HANDLER( FMGEN2608_status_port_1_B_r );
READ8_HANDLER( FMGEN2608_read_port_1_r );
WRITE8_HANDLER( FMGEN2608_control_port_1_A_w );
WRITE8_HANDLER( FMGEN2608_control_port_1_B_w );
WRITE8_HANDLER( FMGEN2608_data_port_1_A_w );
WRITE8_HANDLER( FMGEN2608_data_port_1_B_w );
extern void FMGEN2608_set_volume_1(float volume);

#ifdef __cplusplus
}
#endif

#endif /* __2608cINTF_H__ */
