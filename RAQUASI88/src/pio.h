#ifndef PIO_H_INCLUDED
#define PIO_H_INCLUDED


#define PIO_SIDE_M	(0)
#define PIO_SIDE_S	(1)

#define PIO_READ	(0)
#define PIO_WRITE	(1)

#define PIO_PORT_A	(0)
#define PIO_PORT_B	(1)

#define PIO_PORT_CH	(0)
#define PIO_PORT_CL	(1)

#define PIO_EMPTY	(0)
#define PIO_EXIST	(1)

typedef	struct{		
  int	type;	      /* PORT の 機能   READ か WRITE か                     */
  int	exist;	      /* PA/PB …データの有無				     */
  int	cont_f;	      /* PC 連続アクセスフラグ (CPU 切替えフラグ)	     */
  byte	data;	      /* PA/PB …入力するデータ PC …入力する/出力したデータ */
} pio_work;		

extern	pio_work	pio_AB[2][2], pio_C[2][2];



void	pio_init( void );

byte	pio_read_AB( int side, int port );
void	pio_write_AB( int side, int port, byte data );
byte	pio_read_C( int side );
void	pio_write_C( int side, byte data );
void	pio_write_C_direct( int side, byte data );
void	pio_set_mode( int side, byte data );


#endif	/* PIO_H_INCLUDED */
