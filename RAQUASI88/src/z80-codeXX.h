/************************************************************************/
/*									*/
/* オペコード別処理 ( DD/FD XX )					*/
/*									*/
/************************************************************************/


      /* 8ビット転送命令 */

    case LD_A_H:   z80->ACC=z80->XX.B.h;            break;
    case LD_A_L:   z80->ACC=z80->XX.B.l;            break;
    case LD_A_xHL: z80->ACC=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
		   break;

    case LD_B_H:   z80->BC.B.h=z80->XX.B.h;          break;
    case LD_B_L:   z80->BC.B.h=z80->XX.B.l;          break;
    case LD_B_xHL: z80->BC.B.h=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
		   break;

    case LD_C_H:   z80->BC.B.l=z80->XX.B.h;          break;
    case LD_C_L:   z80->BC.B.l=z80->XX.B.l;          break;
    case LD_C_xHL: z80->BC.B.l=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
		   break;

    case LD_D_H:   z80->DE.B.h=z80->XX.B.h;          break;
    case LD_D_L:   z80->DE.B.h=z80->XX.B.l;          break;
    case LD_D_xHL: z80->DE.B.h=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
		   break;

    case LD_E_H:   z80->DE.B.l=z80->XX.B.h;          break;
    case LD_E_L:   z80->DE.B.l=z80->XX.B.l;          break;
    case LD_E_xHL: z80->DE.B.l=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
		   break;

    case LD_H_A:   z80->XX.B.h=z80->ACC;             break;
    case LD_H_B:   z80->XX.B.h=z80->BC.B.h;          break;
    case LD_H_C:   z80->XX.B.h=z80->BC.B.l;          break;
    case LD_H_D:   z80->XX.B.h=z80->DE.B.h;          break;
    case LD_H_E:   z80->XX.B.h=z80->DE.B.l;          break;
    case LD_H_H:   z80->XX.B.h=z80->XX.B.h;          break;
    case LD_H_L:   z80->XX.B.h=z80->XX.B.l;          break;
    case LD_H_xHL: z80->HL.B.h=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                   break;
    case LD_H_8:   z80->XX.B.h=M_RDMEM(z80->PC.W++); break;

    case LD_L_A:   z80->XX.B.l=z80->ACC;             break;
    case LD_L_B:   z80->XX.B.l=z80->BC.B.h;          break;
    case LD_L_C:   z80->XX.B.l=z80->BC.B.l;          break;
    case LD_L_D:   z80->XX.B.l=z80->DE.B.h;          break;
    case LD_L_E:   z80->XX.B.l=z80->DE.B.l;          break;
    case LD_L_H:   z80->XX.B.l=z80->XX.B.h;          break;
    case LD_L_L:   z80->XX.B.l=z80->XX.B.l;          break;
    case LD_L_xHL: z80->HL.B.l=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                   break;
    case LD_L_8:   z80->XX.B.l=M_RDMEM(z80->PC.W++); break;

    case LD_xHL_A: J.W=z80->XX.W+(offset)M_RDMEM(z80->PC.W++);
                   M_WRMEM(J.W,z80->ACC);
                   break;
    case LD_xHL_B: J.W=z80->XX.W+(offset)M_RDMEM(z80->PC.W++);
                   M_WRMEM(J.W,z80->BC.B.h);
                   break;
    case LD_xHL_C: J.W=z80->XX.W+(offset)M_RDMEM(z80->PC.W++);
                   M_WRMEM(J.W,z80->BC.B.l);
                   break;
    case LD_xHL_D: J.W=z80->XX.W+(offset)M_RDMEM(z80->PC.W++);
                   M_WRMEM(J.W,z80->DE.B.h);
                   break;
    case LD_xHL_E: J.W=z80->XX.W+(offset)M_RDMEM(z80->PC.W++);
                   M_WRMEM(J.W,z80->DE.B.l);
                   break;
    case LD_xHL_H: J.W=z80->XX.W+(offset)M_RDMEM(z80->PC.W++);
                   M_WRMEM(J.W,z80->HL.B.h);
                   break;
    case LD_xHL_L: J.W=z80->XX.W+(offset)M_RDMEM(z80->PC.W++);
                   M_WRMEM(J.W,z80->HL.B.l);
                   break;
    case LD_xHL_8: J.W=z80->XX.W+(offset)M_RDMEM(z80->PC.W++);
                   M_WRMEM(J.W,M_RDMEM(z80->PC.W++));
                   break;


      /* 16ビット転送命令 */

    case LD_HL_16:  M_LDWORD(XX);  break;

    case LD_SP_HL:  z80->SP.W=z80->XX.W;  break;

    case LD_x16_HL:
      J.B.l=M_RDMEM(z80->PC.W++);
      J.B.h=M_RDMEM(z80->PC.W++);
      M_WRMEM(J.W++,z80->XX.B.l);
      M_WRMEM(J.W,  z80->XX.B.h);
      break;
    case LD_HL_x16:
      J.B.l=M_RDMEM(z80->PC.W++);
      J.B.h=M_RDMEM(z80->PC.W++);
      z80->XX.B.l=M_RDMEM(J.W++);
      z80->XX.B.h=M_RDMEM(J.W);
      break;

    case PUSH_HL:  M_PUSH(XX);  break;
    case POP_HL:   M_POP(XX);   break;


      /* 8ビット算術論理演算命令 */

    case ADD_A_H:   M_ADD_A(z80->XX.B.h);  break;
    case ADD_A_L:   M_ADD_A(z80->XX.B.l);  break;
    case ADD_A_xHL: I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                    M_ADD_A(I);
                    break;

    case ADC_A_H:   M_ADC_A(z80->XX.B.h);  break;
    case ADC_A_L:   M_ADC_A(z80->XX.B.l);  break;
    case ADC_A_xHL: I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                    M_ADC_A(I);
                    break;

    case SUB_H:     M_SUB(z80->XX.B.h);  break;
    case SUB_L:     M_SUB(z80->XX.B.l);  break;
    case SUB_xHL:   I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                    M_SUB(I);
                    break;

    case SBC_A_H:   M_SBC_A(z80->XX.B.h);  break;
    case SBC_A_L:   M_SBC_A(z80->XX.B.l);  break;
    case SBC_A_xHL: I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                    M_SBC_A(I);
                    break;

    case AND_H:     M_AND(z80->XX.B.h);  break;
    case AND_L:     M_AND(z80->XX.B.l);  break;
    case AND_xHL:   I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                    M_AND(I);
                    break;

    case OR_H:      M_OR(z80->XX.B.h);  break;
    case OR_L:      M_OR(z80->XX.B.l);  break;
    case OR_xHL:    I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                    M_OR(I);
                    break;

    case XOR_H:     M_XOR(z80->XX.B.h);  break;
    case XOR_L:     M_XOR(z80->XX.B.l);  break;
    case XOR_xHL:   I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                    M_XOR(I);
                    break;

    case CP_H:      M_CP(z80->XX.B.h);  break;
    case CP_L:      M_CP(z80->XX.B.l);  break;
    case CP_xHL:    I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++));
                    M_CP(I);
                    break;

    case INC_H:     M_INC(z80->XX.B.h);  break;
    case INC_L:     M_INC(z80->XX.B.l);  break;
    case INC_xHL:   I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W));
                    M_INC(I);
                    M_WRMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++),I);
                    break;

    case DEC_H:     M_DEC(z80->XX.B.h);  break;
    case DEC_L:     M_DEC(z80->XX.B.l);  break;
    case DEC_xHL:   I=M_RDMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W));
                    M_DEC(I);
                    M_WRMEM(z80->XX.W+(offset)M_RDMEM(z80->PC.W++),I);
                    break;


      /* 16ビット算術演算命令 */

    case ADD_HL_BC:  M_ADDW(z80->XX.W,z80->BC.W);  break;
    case ADD_HL_DE:  M_ADDW(z80->XX.W,z80->DE.W);  break;
    case ADD_HL_HL:  M_ADDW(z80->XX.W,z80->XX.W);  break;
    case ADD_HL_SP:  M_ADDW(z80->XX.W,z80->SP.W);  break;

    case INC_HL:   z80->XX.W++;  break;
    case DEC_HL:   z80->XX.W--;  break;


      /* レジスタ交換命令 */

    case EX_xSP_HL:
      J.B.l=M_RDMEM(z80->SP.W); M_WRMEM(z80->SP.W++,z80->XX.B.l);
      J.B.h=M_RDMEM(z80->SP.W); M_WRMEM(z80->SP.W--,z80->XX.B.h);
      z80->XX.W=J.W;
      break;


      /* 分岐命令 */

    case JP_xHL:   z80->PC.W = z80->XX.W;   break;
