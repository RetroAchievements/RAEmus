#ifndef FMGEN2203_H
#define FMGEN2203_H

#ifdef __cplusplus
extern "C" {
#endif

READ_HANDLER( FMGEN2203_status_port_0_r );
READ_HANDLER( FMGEN2203_status_port_1_r );
READ_HANDLER( FMGEN2203_status_port_2_r );
READ_HANDLER( FMGEN2203_status_port_3_r );
READ_HANDLER( FMGEN2203_status_port_4_r );

READ_HANDLER( FMGEN2203_read_port_0_r );
READ_HANDLER( FMGEN2203_read_port_1_r );
READ_HANDLER( FMGEN2203_read_port_2_r );
READ_HANDLER( FMGEN2203_read_port_3_r );
READ_HANDLER( FMGEN2203_read_port_4_r );

WRITE_HANDLER( FMGEN2203_control_port_0_w );
WRITE_HANDLER( FMGEN2203_control_port_1_w );
WRITE_HANDLER( FMGEN2203_control_port_2_w );
WRITE_HANDLER( FMGEN2203_control_port_3_w );
WRITE_HANDLER( FMGEN2203_control_port_4_w );

WRITE_HANDLER( FMGEN2203_write_port_0_w );
WRITE_HANDLER( FMGEN2203_write_port_1_w );
WRITE_HANDLER( FMGEN2203_write_port_2_w );
WRITE_HANDLER( FMGEN2203_write_port_3_w );
WRITE_HANDLER( FMGEN2203_write_port_4_w );

WRITE_HANDLER( FMGEN2203_word_0_w );
WRITE_HANDLER( FMGEN2203_word_1_w );

int FMGEN2203_sh_start(const struct MachineSound *msound);
void FMGEN2203_sh_stop(void);
void FMGEN2203_sh_reset(void);

void FMGEN2203UpdateRequest(int chip);

#ifdef __cplusplus
}
#endif

#endif
