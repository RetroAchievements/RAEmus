#ifndef FMGEN2203_H
#define FMGEN2203_H

#ifdef __cplusplus
extern "C" {
#endif

READ8_HANDLER( FMGEN2203_status_port_0_r );
READ8_HANDLER( FMGEN2203_status_port_1_r );
READ8_HANDLER( FMGEN2203_status_port_2_r );
READ8_HANDLER( FMGEN2203_status_port_3_r );
READ8_HANDLER( FMGEN2203_status_port_4_r );

READ8_HANDLER( FMGEN2203_read_port_0_r );
READ8_HANDLER( FMGEN2203_read_port_1_r );
READ8_HANDLER( FMGEN2203_read_port_2_r );
READ8_HANDLER( FMGEN2203_read_port_3_r );
READ8_HANDLER( FMGEN2203_read_port_4_r );

WRITE8_HANDLER( FMGEN2203_control_port_0_w );
WRITE8_HANDLER( FMGEN2203_control_port_1_w );
WRITE8_HANDLER( FMGEN2203_control_port_2_w );
WRITE8_HANDLER( FMGEN2203_control_port_3_w );
WRITE8_HANDLER( FMGEN2203_control_port_4_w );

WRITE8_HANDLER( FMGEN2203_write_port_0_w );
WRITE8_HANDLER( FMGEN2203_write_port_1_w );
WRITE8_HANDLER( FMGEN2203_write_port_2_w );
WRITE8_HANDLER( FMGEN2203_write_port_3_w );
WRITE8_HANDLER( FMGEN2203_write_port_4_w );

WRITE8_HANDLER( FMGEN2203_word_0_w );
WRITE8_HANDLER( FMGEN2203_word_1_w );

extern void FMGEN2203_set_volume_0(float volume);
extern void FMGEN2203_set_volume_1(float volume);

#ifdef __cplusplus
}
#endif

#endif
