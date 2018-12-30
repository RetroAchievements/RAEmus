/************************************************************************/
/*									*/
/* オペコード別処理							*/
/*									*/
/************************************************************************/


      /* 8ビット転送命令 */

    case LD_A_A:   z80->ACC=z80->ACC;               break;
    case LD_A_B:   z80->ACC=z80->BC.B.h;            break;
    case LD_A_C:   z80->ACC=z80->BC.B.l;            break;
    case LD_A_D:   z80->ACC=z80->DE.B.h;            break;
    case LD_A_E:   z80->ACC=z80->DE.B.l;            break;
    case LD_A_H:   z80->ACC=z80->HL.B.h;            break;
    case LD_A_L:   z80->ACC=z80->HL.B.l;            break;
    case LD_A_xHL: z80->ACC=M_RDMEM(z80->HL.W);     break;
    case LD_A_8:   z80->ACC=M_RDMEM(z80->PC.W++);   break;

    case LD_B_A:   z80->BC.B.h=z80->ACC;             break;
    case LD_B_B:   z80->BC.B.h=z80->BC.B.h;          break;
    case LD_B_C:   z80->BC.B.h=z80->BC.B.l;          break;
    case LD_B_D:   z80->BC.B.h=z80->DE.B.h;          break;
    case LD_B_E:   z80->BC.B.h=z80->DE.B.l;          break;
    case LD_B_H:   z80->BC.B.h=z80->HL.B.h;          break;
    case LD_B_L:   z80->BC.B.h=z80->HL.B.l;          break;
    case LD_B_xHL: z80->BC.B.h=M_RDMEM(z80->HL.W);   break;
    case LD_B_8:   z80->BC.B.h=M_RDMEM(z80->PC.W++); break;

    case LD_C_A:   z80->BC.B.l=z80->ACC;             break;
    case LD_C_B:   z80->BC.B.l=z80->BC.B.h;          break;
    case LD_C_C:   z80->BC.B.l=z80->BC.B.l;          break;
    case LD_C_D:   z80->BC.B.l=z80->DE.B.h;          break;
    case LD_C_E:   z80->BC.B.l=z80->DE.B.l;          break;
    case LD_C_H:   z80->BC.B.l=z80->HL.B.h;          break;
    case LD_C_L:   z80->BC.B.l=z80->HL.B.l;          break;
    case LD_C_xHL: z80->BC.B.l=M_RDMEM(z80->HL.W);   break;
    case LD_C_8:   z80->BC.B.l=M_RDMEM(z80->PC.W++); break;

    case LD_D_A:   z80->DE.B.h=z80->ACC;             break;
    case LD_D_B:   z80->DE.B.h=z80->BC.B.h;          break;
    case LD_D_C:   z80->DE.B.h=z80->BC.B.l;          break;
    case LD_D_D:   z80->DE.B.h=z80->DE.B.h;          break;
    case LD_D_E:   z80->DE.B.h=z80->DE.B.l;          break;
    case LD_D_H:   z80->DE.B.h=z80->HL.B.h;          break;
    case LD_D_L:   z80->DE.B.h=z80->HL.B.l;          break;
    case LD_D_xHL: z80->DE.B.h=M_RDMEM(z80->HL.W);   break;
    case LD_D_8:   z80->DE.B.h=M_RDMEM(z80->PC.W++); break;

    case LD_E_A:   z80->DE.B.l=z80->ACC;             break;
    case LD_E_B:   z80->DE.B.l=z80->BC.B.h;          break;
    case LD_E_C:   z80->DE.B.l=z80->BC.B.l;          break;
    case LD_E_D:   z80->DE.B.l=z80->DE.B.h;          break;
    case LD_E_E:   z80->DE.B.l=z80->DE.B.l;          break;
    case LD_E_H:   z80->DE.B.l=z80->HL.B.h;          break;
    case LD_E_L:   z80->DE.B.l=z80->HL.B.l;          break;
    case LD_E_xHL: z80->DE.B.l=M_RDMEM(z80->HL.W);   break;
    case LD_E_8:   z80->DE.B.l=M_RDMEM(z80->PC.W++); break;

    case LD_H_A:   z80->HL.B.h=z80->ACC;             break;
    case LD_H_B:   z80->HL.B.h=z80->BC.B.h;          break;
    case LD_H_C:   z80->HL.B.h=z80->BC.B.l;          break;
    case LD_H_D:   z80->HL.B.h=z80->DE.B.h;          break;
    case LD_H_E:   z80->HL.B.h=z80->DE.B.l;          break;
    case LD_H_H:   z80->HL.B.h=z80->HL.B.h;          break;
    case LD_H_L:   z80->HL.B.h=z80->HL.B.l;          break;
    case LD_H_xHL: z80->HL.B.h=M_RDMEM(z80->HL.W);   break;
    case LD_H_8:   z80->HL.B.h=M_RDMEM(z80->PC.W++); break;

    case LD_L_A:   z80->HL.B.l=z80->ACC;             break;
    case LD_L_B:   z80->HL.B.l=z80->BC.B.h;          break;
    case LD_L_C:   z80->HL.B.l=z80->BC.B.l;          break;
    case LD_L_D:   z80->HL.B.l=z80->DE.B.h;          break;
    case LD_L_E:   z80->HL.B.l=z80->DE.B.l;          break;
    case LD_L_H:   z80->HL.B.l=z80->HL.B.h;          break;
    case LD_L_L:   z80->HL.B.l=z80->HL.B.l;          break;
    case LD_L_xHL: z80->HL.B.l=M_RDMEM(z80->HL.W);   break;
    case LD_L_8:   z80->HL.B.l=M_RDMEM(z80->PC.W++); break;

    case LD_xHL_A: M_WRMEM(z80->HL.W,z80->ACC);             break;
    case LD_xHL_B: M_WRMEM(z80->HL.W,z80->BC.B.h);          break;
    case LD_xHL_C: M_WRMEM(z80->HL.W,z80->BC.B.l);          break;
    case LD_xHL_D: M_WRMEM(z80->HL.W,z80->DE.B.h);          break;
    case LD_xHL_E: M_WRMEM(z80->HL.W,z80->DE.B.l);          break;
    case LD_xHL_H: M_WRMEM(z80->HL.W,z80->HL.B.h);          break;
    case LD_xHL_L: M_WRMEM(z80->HL.W,z80->HL.B.l);          break;
    case LD_xHL_8: M_WRMEM(z80->HL.W,M_RDMEM(z80->PC.W++)); break;

    case LD_A_xBC: z80->ACC=M_RDMEM(z80->BC.W);  break;
    case LD_A_xDE: z80->ACC=M_RDMEM(z80->DE.W);  break;
    case LD_A_x16:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++); 
      z80->ACC = M_RDMEM(J.W);
      break;

    case LD_xBC_A: M_WRMEM(z80->BC.W,z80->ACC);  break;
    case LD_xDE_A: M_WRMEM(z80->DE.W,z80->ACC);  break;
    case LD_x16_A:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      M_WRMEM(J.W,z80->ACC);
      break;


      /* 16ビット転送命令 */

    case LD_BC_16:  M_LDWORD(BC);  break;
    case LD_DE_16:  M_LDWORD(DE);  break;
    case LD_HL_16:  M_LDWORD(HL);  break;
    case LD_SP_16:  M_LDWORD(SP);  break;

    case LD_SP_HL:  z80->SP.W=z80->HL.W;  break;

    case LD_x16_HL:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      M_WRMEM(J.W++,z80->HL.B.l);
      M_WRMEM(J.W,  z80->HL.B.h);
      break;
    case LD_HL_x16:
      J.B.l = M_RDMEM(z80->PC.W++);
      J.B.h = M_RDMEM(z80->PC.W++);
      z80->HL.B.l = M_RDMEM(J.W++);
      z80->HL.B.h = M_RDMEM(J.W);
      break;

    case PUSH_BC:  M_PUSH(BC);  break;
    case PUSH_DE:  M_PUSH(DE);  break;
    case PUSH_HL:  M_PUSH(HL);  break;
    case PUSH_AF:  M_PUSH(AF);  break;

    case POP_BC:   M_POP(BC);   break;
    case POP_DE:   M_POP(DE);   break;
    case POP_HL:   M_POP(HL);   break;
    case POP_AF:   M_POP(AF);   break;


      /* 8ビット算術論理演算命令 */

    case ADD_A_A:  M_ADD_A(z80->ACC);     break;
    case ADD_A_B:  M_ADD_A(z80->BC.B.h);  break;
    case ADD_A_C:  M_ADD_A(z80->BC.B.l);  break;
    case ADD_A_D:  M_ADD_A(z80->DE.B.h);  break;
    case ADD_A_E:  M_ADD_A(z80->DE.B.l);  break;
    case ADD_A_H:  M_ADD_A(z80->HL.B.h);  break;
    case ADD_A_L:  M_ADD_A(z80->HL.B.l);  break;
    case ADD_A_xHL:I=M_RDMEM(z80->HL.W);   M_ADD_A(I);  break;
    case ADD_A_8:  I=M_RDMEM(z80->PC.W++); M_ADD_A(I);  break;

    case ADC_A_A:  M_ADC_A(z80->ACC);     break;
    case ADC_A_B:  M_ADC_A(z80->BC.B.h);  break;
    case ADC_A_C:  M_ADC_A(z80->BC.B.l);  break;
    case ADC_A_D:  M_ADC_A(z80->DE.B.h);  break;
    case ADC_A_E:  M_ADC_A(z80->DE.B.l);  break;
    case ADC_A_H:  M_ADC_A(z80->HL.B.h);  break;
    case ADC_A_L:  M_ADC_A(z80->HL.B.l);  break;
    case ADC_A_xHL:I=M_RDMEM(z80->HL.W);   M_ADC_A(I);  break;
    case ADC_A_8:  I=M_RDMEM(z80->PC.W++); M_ADC_A(I);  break;

    case SUB_A:    M_SUB(z80->ACC);     break;
    case SUB_B:    M_SUB(z80->BC.B.h);  break;
    case SUB_C:    M_SUB(z80->BC.B.l);  break;
    case SUB_D:    M_SUB(z80->DE.B.h);  break;
    case SUB_E:    M_SUB(z80->DE.B.l);  break;
    case SUB_H:    M_SUB(z80->HL.B.h);  break;
    case SUB_L:    M_SUB(z80->HL.B.l);  break;
    case SUB_xHL:  I=M_RDMEM(z80->HL.W);   M_SUB(I);  break;
    case SUB_8:    I=M_RDMEM(z80->PC.W++); M_SUB(I);  break;

    case SBC_A_A:  M_SBC_A(z80->ACC);     break;
    case SBC_A_B:  M_SBC_A(z80->BC.B.h);  break;
    case SBC_A_C:  M_SBC_A(z80->BC.B.l);  break;
    case SBC_A_D:  M_SBC_A(z80->DE.B.h);  break;
    case SBC_A_E:  M_SBC_A(z80->DE.B.l);  break;
    case SBC_A_H:  M_SBC_A(z80->HL.B.h);  break;
    case SBC_A_L:  M_SBC_A(z80->HL.B.l);  break;
    case SBC_A_xHL:I=M_RDMEM(z80->HL.W);   M_SBC_A(I);  break;
    case SBC_A_8:  I=M_RDMEM(z80->PC.W++); M_SBC_A(I);  break;

    case AND_A:    M_AND(z80->ACC);     break;
    case AND_B:    M_AND(z80->BC.B.h);  break;
    case AND_C:    M_AND(z80->BC.B.l);  break;
    case AND_D:    M_AND(z80->DE.B.h);  break;
    case AND_E:    M_AND(z80->DE.B.l);  break;
    case AND_H:    M_AND(z80->HL.B.h);  break;
    case AND_L:    M_AND(z80->HL.B.l);  break;
    case AND_xHL:  I=M_RDMEM(z80->HL.W);   M_AND(I);  break;
    case AND_8:    I=M_RDMEM(z80->PC.W++); M_AND(I);  break;

    case OR_A:     M_OR(z80->ACC);     break;
    case OR_B:     M_OR(z80->BC.B.h);  break;
    case OR_C:     M_OR(z80->BC.B.l);  break;
    case OR_D:     M_OR(z80->DE.B.h);  break;
    case OR_E:     M_OR(z80->DE.B.l);  break;
    case OR_H:     M_OR(z80->HL.B.h);  break;
    case OR_L:     M_OR(z80->HL.B.l);  break;
    case OR_xHL:   I=M_RDMEM(z80->HL.W);   M_OR(I);  break;
    case OR_8:     I=M_RDMEM(z80->PC.W++); M_OR(I);  break;

    case XOR_A:    M_XOR(z80->ACC);     break;
    case XOR_B:    M_XOR(z80->BC.B.h);  break;
    case XOR_C:    M_XOR(z80->BC.B.l);  break;
    case XOR_D:    M_XOR(z80->DE.B.h);  break;
    case XOR_E:    M_XOR(z80->DE.B.l);  break;
    case XOR_H:    M_XOR(z80->HL.B.h);  break;
    case XOR_L:    M_XOR(z80->HL.B.l);  break;
    case XOR_xHL:  I=M_RDMEM(z80->HL.W);   M_XOR(I);  break;
    case XOR_8:    I=M_RDMEM(z80->PC.W++); M_XOR(I);  break;

    case CP_A:     M_CP(z80->ACC);     break;
    case CP_B:     M_CP(z80->BC.B.h);  break;
    case CP_C:     M_CP(z80->BC.B.l);  break;
    case CP_D:     M_CP(z80->DE.B.h);  break;
    case CP_E:     M_CP(z80->DE.B.l);  break;
    case CP_H:     M_CP(z80->HL.B.h);  break;
    case CP_L:     M_CP(z80->HL.B.l);  break;
    case CP_xHL:   I=M_RDMEM(z80->HL.W);   M_CP(I);  break;
    case CP_8:     I=M_RDMEM(z80->PC.W++); M_CP(I);  break;

    case INC_A:    M_INC(z80->ACC);     break;
    case INC_B:    M_INC(z80->BC.B.h);  break;
    case INC_C:    M_INC(z80->BC.B.l);  break;
    case INC_D:    M_INC(z80->DE.B.h);  break;
    case INC_E:    M_INC(z80->DE.B.l);  break;
    case INC_H:    M_INC(z80->HL.B.h);  break;
    case INC_L:    M_INC(z80->HL.B.l);  break;
    case INC_xHL:
      I=M_RDMEM(z80->HL.W); M_INC(I); M_WRMEM(z80->HL.W,I);  break;

    case DEC_A:    M_DEC(z80->ACC);     break;
    case DEC_B:    M_DEC(z80->BC.B.h);  break;
    case DEC_C:    M_DEC(z80->BC.B.l);  break;
    case DEC_D:    M_DEC(z80->DE.B.h);  break;
    case DEC_E:    M_DEC(z80->DE.B.l);  break;
    case DEC_H:    M_DEC(z80->HL.B.h);  break;
    case DEC_L:    M_DEC(z80->HL.B.l);  break;
    case DEC_xHL:
      I=M_RDMEM(z80->HL.W); M_DEC(I); M_WRMEM(z80->HL.W,I);  break;

      /* 16ビット算術演算命令 */

    case ADD_HL_BC:  M_ADDW(z80->HL.W,z80->BC.W);  break;
    case ADD_HL_DE:  M_ADDW(z80->HL.W,z80->DE.W);  break;
    case ADD_HL_HL:  M_ADDW(z80->HL.W,z80->HL.W);  break;
    case ADD_HL_SP:  M_ADDW(z80->HL.W,z80->SP.W);  break;

    case INC_BC:   z80->BC.W++;  break;
    case INC_DE:   z80->DE.W++;  break;
    case INC_HL:   z80->HL.W++;  break;
    case INC_SP:   z80->SP.W++;  break;

    case DEC_BC:   z80->BC.W--;  break;
    case DEC_DE:   z80->DE.W--;  break;
    case DEC_HL:   z80->HL.W--;  break;
    case DEC_SP:   z80->SP.W--;  break;


      /* レジスタ交換命令 */

    case EX_AF_AF:
      J.W=z80->AF.W; z80->AF.W=z80->AF1.W; z80->AF1.W=J.W;
      break;
    case EX_DE_HL:
      J.W=z80->DE.W; z80->DE.W=z80->HL.W;  z80->HL.W=J.W;
      break;
    case EX_xSP_HL:
      J.B.l = M_RDMEM(z80->SP.W); M_WRMEM(z80->SP.W++,z80->HL.B.l);
      J.B.h = M_RDMEM(z80->SP.W); M_WRMEM(z80->SP.W--,z80->HL.B.h);
      z80->HL.W = J.W;
      break;
    case EXX:
      J.W=z80->BC.W; z80->BC.W=z80->BC1.W; z80->BC1.W=J.W;
      J.W=z80->DE.W; z80->DE.W=z80->DE1.W; z80->DE1.W=J.W;
      J.W=z80->HL.W; z80->HL.W=z80->HL1.W; z80->HL1.W=J.W;
      break;


      /* 分岐命令 */

    case JP:       M_JP();                     break;
    case JP_NZ:
      if( M_NZ() ) M_JP();  else M_JP_SKIP();  break;
    case JP_NC:
      if( M_NC() ) M_JP();  else M_JP_SKIP();  break;
    case JP_PO:
      if( M_PO() ) M_JP();  else M_JP_SKIP();  break;
    case JP_P:
      if( M_P()  ) M_JP();  else M_JP_SKIP();  break;
    case JP_Z:
      if( M_Z()  ) M_JP();  else M_JP_SKIP();  break;
    case JP_C:
      if( M_C()  ) M_JP();  else M_JP_SKIP();  break;
    case JP_PE:
      if( M_PE() ) M_JP();  else M_JP_SKIP();  break;
    case JP_M:
      if( M_M()  ) M_JP();  else M_JP_SKIP();  break;

    case JR:       M_JR();                     break;
    case JR_NZ:
      if( M_NZ() ) M_JR();  else M_JR_SKIP();  break;
    case JR_NC:
      if( M_NC() ) M_JR();  else M_JR_SKIP();  break;
    case JR_Z:
      if( M_Z()  ) M_JR();  else M_JR_SKIP();  break;
    case JR_C:
      if( M_C()  ) M_JR();  else M_JR_SKIP();  break;

    case CALL:     M_CALL();                       break;
    case CALL_NZ:
      if( M_NZ() ) M_CALL();  else M_CALL_SKIP();  break;
    case CALL_NC:
      if( M_NC() ) M_CALL();  else M_CALL_SKIP();  break;
    case CALL_PO:
      if( M_PO() ) M_CALL();  else M_CALL_SKIP();  break;
    case CALL_P:
      if( M_P()  ) M_CALL();  else M_CALL_SKIP();  break;
    case CALL_Z:
      if( M_Z()  ) M_CALL();  else M_CALL_SKIP();  break;
    case CALL_C:
      if( M_C()  ) M_CALL();  else M_CALL_SKIP();  break;
    case CALL_PE:
      if( M_PE() ) M_CALL();  else M_CALL_SKIP();  break;
    case CALL_M:
      if( M_M()  ) M_CALL();  else M_CALL_SKIP();  break;

    case RET:      M_RET();                      break;
    case RET_NZ:
      if( M_NZ() ) M_RET();  else M_RET_SKIP();  break;
    case RET_NC:
      if( M_NC() ) M_RET();  else M_RET_SKIP();  break;
    case RET_PO:
      if( M_PO() ) M_RET();  else M_RET_SKIP();  break;
    case RET_P:
      if( M_P()  ) M_RET();  else M_RET_SKIP();  break;
    case RET_Z:
      if( M_Z()  ) M_RET();  else M_RET_SKIP();  break;
    case RET_C:
      if( M_C()  ) M_RET();  else M_RET_SKIP();  break;
    case RET_PE:
      if( M_PE() ) M_RET();  else M_RET_SKIP();  break;
    case RET_M:
      if( M_M()  ) M_RET();  else M_RET_SKIP();  break;

    case JP_xHL:   z80->PC.W = z80->HL.W;   break;
    case DJNZ:
      if( --z80->BC.B.h ) M_JR();
      else                M_JR_SKIP();      break;

    case RST00:    M_RST(0x0000);   break;
    case RST08:    M_RST(0x0008);   break;
    case RST10:    M_RST(0x0010);   break;
    case RST18:    M_RST(0x0018);   break;
    case RST20:    M_RST(0x0020);   break;
    case RST28:    M_RST(0x0028);   break;
    case RST30:    M_RST(0x0030);   break;
    case RST38:    M_RST(0x0038);   break;


      /* ローテート／シフト命令 */

    case RLCA:
      I = z80->ACC>>7;
      z80->ACC  = (z80->ACC<<1)|I;
      z80->FLAG = (z80->FLAG&~(H_FLAG|N_FLAG|C_FLAG))|I;
      break;
    case RLA:
      I = z80->ACC>>7;
      z80->ACC  = (z80->ACC<<1)|(z80->FLAG&C_FLAG);
      z80->FLAG = (z80->FLAG&~(H_FLAG|N_FLAG|C_FLAG))|I;
      break;
    case RRCA:
      I = z80->ACC&0x01;
      z80->ACC  = (z80->ACC>>1)|(I<<7);
      z80->FLAG = (z80->FLAG&~(H_FLAG|N_FLAG|C_FLAG))|I;
      break;
    case RRA:
      I = z80->ACC&0x01;
      z80->ACC  = (z80->ACC>>1)|(z80->FLAG<<7);
      z80->FLAG = (z80->FLAG&~(H_FLAG|N_FLAG|C_FLAG))|I;
      break;


      /* 入出力命令 */

    case IN_A_x8:
      I = M_RDIO( M_RDMEM(z80->PC.W++) );
      z80->ACC = I;
      break;
    case OUT_x8_A:
      M_WRIO( M_RDMEM(z80->PC.W++), z80->ACC );
      break;


      /* その他の命令 */

    case NOP:  break;

    case DI:
      z80->IFF = INT_DISABLE;
      break;
    case EI:
      z80->IFF = INT_ENABLE;
      if( z80->state0 < z80_state_intchk ){	/* まだ内側ループ抜けない場合*/
	if( z80->INT_active ){				/* 保留割込があれば  */
	  z80->skip_intr_chk = TRUE;			/* ここで抜ける      */
	  z80_state_intchk = 0;
	}
      }else{					/* もう抜ける場合 */
	z80->skip_intr_chk = TRUE;
	z80_state_intchk = 0;
      }
      break;

    case SCF:
      z80->FLAG = (z80->FLAG&~(H_FLAG|N_FLAG))|C_FLAG;
      break;
    case CCF:
      z80->FLAG ^= C_FLAG;
      z80->FLAG  = (z80->FLAG&~(H_FLAG|N_FLAG))|(z80->FLAG&C_FLAG? 0:H_FLAG);
      break;

    case CPL:
      z80->ACC   = ~z80->ACC;
      z80->FLAG |= (H_FLAG|N_FLAG);
      break;
    case DAA:
      J.W = z80->ACC;
      if( z80->FLAG & C_FLAG ) J.W |= 256;
      if( z80->FLAG & H_FLAG ) J.W |= 512;
      if( z80->FLAG & N_FLAG ) J.W |= 1024;
      z80->AF.W = DAA_table[ J.W ];
      break;

    case HALT: 
      z80->HALT = TRUE;
      z80->PC.W --;
      if( z80->INT_active )    z80_state_intchk = 0;
      if( z80->break_if_halt ) z80_state_intchk = 0;
      break;


