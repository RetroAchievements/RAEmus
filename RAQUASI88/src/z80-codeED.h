/************************************************************************/
/*									*/
/* オペコード別処理 ( ED XX )						*/
/*									*/
/************************************************************************/


      /* 8ビット転送命令 */

    case LD_A_I:
      z80->ACC  = z80->I;
      z80->FLAG = SZ_table[z80->ACC] |
                  (z80->IFF==INT_DISABLE? 0:P_FLAG)|(z80->FLAG&C_FLAG);
      break;
    case LD_A_R:
      z80->ACC = (z80->R & 0x7f) | (z80->R_saved & 0x80);
      z80->FLAG = SZ_table[z80->ACC] |
                  (z80->IFF==INT_DISABLE? 0:P_FLAG)|(z80->FLAG&C_FLAG);
      break;

    case LD_I_A:   z80->I = z80->ACC;    break;
    case LD_R_A:   z80->R = z80->ACC;    break;


      /* 16ビット転送命令 */

    case LD_x16x_HL:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      M_WRMEM(J.W++,z80->HL.B.l);
      M_WRMEM(J.W,  z80->HL.B.h);
      break;
    case LD_x16x_DE:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      M_WRMEM(J.W++,z80->DE.B.l);
      M_WRMEM(J.W,  z80->DE.B.h);
      break;
    case LD_x16x_BC:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      M_WRMEM(J.W++,z80->BC.B.l);
      M_WRMEM(J.W,  z80->BC.B.h);
      break;
    case LD_x16x_SP:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      M_WRMEM(J.W++,z80->SP.B.l);
      M_WRMEM(J.W,  z80->SP.B.h);
      break;

    case LD_HL_x16x:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      z80->HL.B.l = M_RDMEM(J.W++);
      z80->HL.B.h = M_RDMEM(J.W  );
      break;
    case LD_DE_x16x:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      z80->DE.B.l = M_RDMEM(J.W++);
      z80->DE.B.h = M_RDMEM(J.W  );
      break;
    case LD_BC_x16x:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      z80->BC.B.l = M_RDMEM(J.W++);
      z80->BC.B.h = M_RDMEM(J.W  );
      break;
    case LD_SP_x16x:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      z80->SP.B.l = M_RDMEM(J.W++);
      z80->SP.B.h = M_RDMEM(J.W  );
      break;
      
      /* 16ビット算術演算命令 */

    case ADC_HL_BC:  M_ADCW(z80->BC.W);  break;
    case ADC_HL_DE:  M_ADCW(z80->DE.W);  break;
    case ADC_HL_HL:  M_ADCW(z80->HL.W);  break;
    case ADC_HL_SP:  M_ADCW(z80->SP.W);  break;

    case SBC_HL_BC:  M_SBCW(z80->BC.W);  break;
    case SBC_HL_DE:  M_SBCW(z80->DE.W);  break;
    case SBC_HL_HL:  M_SBCW(z80->HL.W);  break;
    case SBC_HL_SP:  M_SBCW(z80->SP.W);  break;


      /* ローテート・シフト命令 */

    case RLD:
      I = M_RDMEM(z80->HL.W);
      J.B.l = (I<<4)|(z80->ACC&0x0f);
      M_WRMEM(z80->HL.W,J.B.l);
      z80->ACC  = (I>>4)|(z80->ACC&0xf0);
      z80->FLAG = SZP_table[z80->ACC]|(z80->FLAG&C_FLAG);
      break;
    case RRD:
      I = M_RDMEM(z80->HL.W);
      J.B.l = (I>>4)|(z80->ACC<<4);
      M_WRMEM(z80->HL.W,J.B.l);
      z80->ACC  = (I&0x0f)|(z80->ACC&0xf0);
      z80->FLAG = SZP_table[z80->ACC]|(z80->FLAG&C_FLAG);
      break;

      /* ＣＰＵ制御命令 */

    case IM_0:
    case IM_0_4E:
    case IM_0_66:
    case IM_0_6E:  z80->IM = 0;  break;

    case IM_1:
    case IM_1_76:  z80->IM = 1;  break;

    case IM_2:
    case IM_2_7E:  z80->IM = 2;  break;

      /* アキュムレータ操作命令 */

    case NEG:
    case NEG_4C:
    case NEG_54:
    case NEG_5C:
    case NEG_64:
    case NEG_6C:
    case NEG_74:
    case NEG_7C:   I=z80->ACC;  z80->ACC=0;  M_SUB(I);  break;

      /* 分岐命令 */

    case RETI:      M_RET();                      break;

    case RETN:
    case RETN_55:
    case RETN_5D:
    case RETN_65:
    case RETN_6D:
    case RETN_7D:  
    case RETN_75:   M_RET();                      break;

      /* 入出力命令 */

    case IN_B_xC:   M_IN_C(z80->BC.B.h);   break;
    case IN_C_xC:   M_IN_C(z80->BC.B.l);   break;
    case IN_D_xC:   M_IN_C(z80->DE.B.h);   break;
    case IN_E_xC:   M_IN_C(z80->DE.B.l);   break;
    case IN_H_xC:   M_IN_C(z80->HL.B.h);   break;
    case IN_L_xC:   M_IN_C(z80->HL.B.l);   break;
    case IN_A_xC:   M_IN_C(z80->ACC);      break;
    case IN_F_xC:   M_IN_C(J.B.l);         break;

    case OUT_xC_B:  M_OUT_C(z80->BC.B.h);  break;
    case OUT_xC_C:  M_OUT_C(z80->BC.B.l);  break;
    case OUT_xC_D:  M_OUT_C(z80->DE.B.h);  break;
    case OUT_xC_E:  M_OUT_C(z80->DE.B.l);  break;
    case OUT_xC_H:  M_OUT_C(z80->HL.B.h);  break;
    case OUT_xC_L:  M_OUT_C(z80->HL.B.l);  break;
    case OUT_xC_A:  M_OUT_C(z80->ACC);     break;
    case OUT_xC_F:  M_OUT_C(0);            break;

    case INI:
      I = M_RDIO(z80->BC.B.l);
      M_WRMEM(z80->HL.W++,I);
      z80->BC.B.h--;
      z80->FLAG = (z80->BC.B.h? 0:Z_FLAG)|N_FLAG|(z80->FLAG&C_FLAG);
      break;
    case INIR:
      I = M_RDIO(z80->BC.B.l);
      M_WRMEM(z80->HL.W++,I);
      z80->BC.B.h--;
      z80->FLAG = (z80->BC.B.h? 0:Z_FLAG)|N_FLAG|(z80->FLAG&C_FLAG);
      if( z80->BC.B.h ){
	z80->state0 += 5;
	z80->PC.W -= 2;
      }
      break;
    case IND:
      I = M_RDIO(z80->BC.B.l);
      M_WRMEM(z80->HL.W--,I);
      z80->BC.B.h--;
      z80->FLAG = (z80->BC.B.h? 0:Z_FLAG)|N_FLAG|(z80->FLAG&C_FLAG);
      break;
    case INDR:
      I = M_RDIO(z80->BC.B.l);
      M_WRMEM(z80->HL.W--,I);
      z80->BC.B.h--;
      z80->FLAG = (z80->BC.B.h? 0:Z_FLAG)|N_FLAG|(z80->FLAG&C_FLAG);
      if( z80->BC.B.h ){
	z80->state0 += 5;
	z80->PC.W -= 2;
      }
      break;

    case OUTI:
      M_WRIO(z80->BC.B.l,M_RDMEM(z80->HL.W));
      z80->HL.W++;
      z80->BC.B.h--;
      z80->FLAG = (z80->BC.B.h? 0:Z_FLAG)|N_FLAG|(z80->FLAG&C_FLAG);
      break;
    case OTIR:
      M_WRIO(z80->BC.B.l,M_RDMEM(z80->HL.W));
      z80->HL.W++;
      z80->BC.B.h--;
      z80->FLAG = (z80->BC.B.h? 0:Z_FLAG)|N_FLAG|(z80->FLAG&C_FLAG);
      if( z80->BC.B.h ){
	z80->state0 += 5;
	z80->PC.W -= 2;
      }
      break;
    case OUTD:
      M_WRIO(z80->BC.B.l,M_RDMEM(z80->HL.W));
      z80->HL.W--;
      z80->BC.B.h--;
      z80->FLAG = (z80->BC.B.h? 0:Z_FLAG)|N_FLAG|(z80->FLAG&C_FLAG);
      break;
    case OTDR:
      M_WRIO(z80->BC.B.l,M_RDMEM(z80->HL.W));
      z80->HL.W--;
      z80->BC.B.h--;
      z80->FLAG = (z80->BC.B.h? 0:Z_FLAG)|N_FLAG|(z80->FLAG&C_FLAG);
      if( z80->BC.B.h ){
	z80->state0 += 5;
	z80->PC.W -= 2;
      }
      break;

      /* ブロック転送命令 */

    case LDI:
      {
	M_WRMEM(z80->DE.W++,M_RDMEM(z80->HL.W++));
	z80->BC.W--;
	z80->FLAG = (z80->FLAG&~(N_FLAG|H_FLAG|P_FLAG))|(z80->BC.W? P_FLAG:0);
      }
      break;
    case LDIR:
      {
	M_WRMEM(z80->DE.W++,M_RDMEM(z80->HL.W++));
	z80->BC.W--;
	z80->FLAG = (z80->FLAG&~(N_FLAG|H_FLAG|P_FLAG))|(z80->BC.W? P_FLAG:0);
      }
      if( z80->BC.W ){
	z80->state0 += 5;
	z80->PC.W -= 2;
      }
      break;
    case LDD:
      {
	M_WRMEM(z80->DE.W--,M_RDMEM(z80->HL.W--));
	z80->BC.W--;
	z80->FLAG = (z80->FLAG&~(N_FLAG|H_FLAG|P_FLAG))|(z80->BC.W? P_FLAG:0);
      }
      break;
    case LDDR:
      {
	M_WRMEM(z80->DE.W--,M_RDMEM(z80->HL.W--));
	z80->BC.W--;
	z80->FLAG = (z80->FLAG&~(N_FLAG|H_FLAG|P_FLAG))|(z80->BC.W? P_FLAG:0);
      }
      if( z80->BC.W ){
	z80->state0 += 5;
	z80->PC.W -= 2;
      }
      break;

      /* ブロックサーチ命令 */

    case CPI:
      {
	I = M_RDMEM(z80->HL.W++);
	J.B.l = z80->ACC-I;
	z80->BC.W--;
	z80->FLAG = SZ_table[J.B.l] | ((z80->ACC^I^J.B.l)&H_FLAG) |
                    (z80->BC.W? P_FLAG:0) | N_FLAG | (z80->FLAG&C_FLAG);
      }
      break;
    case CPIR:
      {
	I = M_RDMEM(z80->HL.W++);
	J.B.l = z80->ACC-I;
	z80->BC.W--;
	z80->FLAG = SZ_table[J.B.l] | ((z80->ACC^I^J.B.l)&H_FLAG) |
                    (z80->BC.W? P_FLAG:0) | N_FLAG | (z80->FLAG&C_FLAG);
      }
      if( z80->BC.W && J.B.l ){
	z80->state0 += 5;
	z80->PC.W -= 2;
      }
      break;  
    case CPD:
      {
	I = M_RDMEM(z80->HL.W--);
	J.B.l = z80->ACC-I;
	z80->BC.W--;
	z80->FLAG = SZ_table[J.B.l] | ((z80->ACC^I^J.B.l)&H_FLAG) |
                    (z80->BC.W? P_FLAG:0) | N_FLAG | (z80->FLAG&C_FLAG);
      }
      break;
    case CPDR:
      {
	I = M_RDMEM(z80->HL.W--);
	J.B.l = z80->ACC-I;
	z80->BC.W--;
	z80->FLAG = SZ_table[J.B.l] | ((z80->ACC^I^J.B.l)&H_FLAG) |
                    (z80->BC.W? P_FLAG:0) | N_FLAG | (z80->FLAG&C_FLAG);
      }
      if( z80->BC.W && J.B.l ){
	z80->state0 += 5;
	z80->PC.W -= 2;
      }
      break;
