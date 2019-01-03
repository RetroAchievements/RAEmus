#ifndef __FMGEN2608_H__
#define __FMGEN2608_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "2608intf.h"


int FMGEN2608_sh_start(const struct MachineSound *msound);
void FMGEN2608_sh_stop(void);
void FMGEN2608_sh_reset(void);

/************************************************/
/* Chip 0 functions				*/
/************************************************/
READ_HANDLER( FMGEN2608_status_port_0_A_r );
READ_HANDLER( FMGEN2608_status_port_0_B_r );
READ_HANDLER( FMGEN2608_read_port_0_r );
WRITE_HANDLER( FMGEN2608_control_port_0_A_w );
WRITE_HANDLER( FMGEN2608_control_port_0_B_w );
WRITE_HANDLER( FMGEN2608_data_port_0_A_w );
WRITE_HANDLER( FMGEN2608_data_port_0_B_w );

/************************************************/
/* Chip 1 functions				*/
/************************************************/
READ_HANDLER( FMGEN2608_status_port_1_A_r );
READ_HANDLER( FMGEN2608_status_port_1_B_r );
READ_HANDLER( FMGEN2608_read_port_1_r );
WRITE_HANDLER( FMGEN2608_control_port_1_A_w );
WRITE_HANDLER( FMGEN2608_control_port_1_B_w );
WRITE_HANDLER( FMGEN2608_data_port_1_A_w );
WRITE_HANDLER( FMGEN2608_data_port_1_B_w );

#ifdef __cplusplus
}
#endif

#endif /* __2608cINTF_H__ */
