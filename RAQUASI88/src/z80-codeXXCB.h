/************************************************************************/
/*									*/
/* オペコード別処理 ( DD/FD CB XX XX )					*/
/*									*/
/************************************************************************/


      /* ローテート・シフト命令 */

    case RLC_B: I=M_RDMEM(J.W); M_RLC(I); M_WRMEM(J.W,I); z80->BC.B.h=I; break;
    case RLC_C: I=M_RDMEM(J.W); M_RLC(I); M_WRMEM(J.W,I); z80->BC.B.l=I; break;
    case RLC_D: I=M_RDMEM(J.W); M_RLC(I); M_WRMEM(J.W,I); z80->DE.B.h=I; break;
    case RLC_E: I=M_RDMEM(J.W); M_RLC(I); M_WRMEM(J.W,I); z80->DE.B.l=I; break;
    case RLC_H: I=M_RDMEM(J.W); M_RLC(I); M_WRMEM(J.W,I); z80->HL.B.h=I; break;
    case RLC_L: I=M_RDMEM(J.W); M_RLC(I); M_WRMEM(J.W,I); z80->HL.B.l=I; break;
    case RLC_xHL: I=M_RDMEM(J.W); M_RLC(I); M_WRMEM(J.W,I); break;
    case RLC_A: I=M_RDMEM(J.W); M_RLC(I); M_WRMEM(J.W,I); z80->AF.B.h=I; break;

    case RRC_B: I=M_RDMEM(J.W); M_RRC(I); M_WRMEM(J.W,I); z80->BC.B.h=I; break;
    case RRC_C: I=M_RDMEM(J.W); M_RRC(I); M_WRMEM(J.W,I); z80->BC.B.l=I; break;
    case RRC_D: I=M_RDMEM(J.W); M_RRC(I); M_WRMEM(J.W,I); z80->DE.B.h=I; break;
    case RRC_E: I=M_RDMEM(J.W); M_RRC(I); M_WRMEM(J.W,I); z80->DE.B.l=I; break;
    case RRC_H: I=M_RDMEM(J.W); M_RRC(I); M_WRMEM(J.W,I); z80->HL.B.h=I; break;
    case RRC_L: I=M_RDMEM(J.W); M_RRC(I); M_WRMEM(J.W,I); z80->HL.B.l=I; break;
    case RRC_xHL: I=M_RDMEM(J.W); M_RRC(I); M_WRMEM(J.W,I); break;
    case RRC_A: I=M_RDMEM(J.W); M_RRC(I); M_WRMEM(J.W,I); z80->AF.B.h=I; break;

    case RL_B:  I=M_RDMEM(J.W); M_RL(I);  M_WRMEM(J.W,I); z80->BC.B.h=I; break;
    case RL_C:  I=M_RDMEM(J.W); M_RL(I);  M_WRMEM(J.W,I); z80->BC.B.l=I; break;
    case RL_D:  I=M_RDMEM(J.W); M_RL(I);  M_WRMEM(J.W,I); z80->DE.B.h=I; break;
    case RL_E:  I=M_RDMEM(J.W); M_RL(I);  M_WRMEM(J.W,I); z80->DE.B.l=I; break;
    case RL_H:  I=M_RDMEM(J.W); M_RL(I);  M_WRMEM(J.W,I); z80->HL.B.h=I; break;
    case RL_L:  I=M_RDMEM(J.W); M_RL(I);  M_WRMEM(J.W,I); z80->HL.B.l=I; break;
    case RL_xHL:  I=M_RDMEM(J.W); M_RL(I);  M_WRMEM(J.W,I); break;
    case RL_A:  I=M_RDMEM(J.W); M_RL(I);  M_WRMEM(J.W,I); z80->AF.B.h=I; break;

    case RR_B:  I=M_RDMEM(J.W); M_RR(I);  M_WRMEM(J.W,I); z80->BC.B.h=I; break;
    case RR_C:  I=M_RDMEM(J.W); M_RR(I);  M_WRMEM(J.W,I); z80->BC.B.l=I; break;
    case RR_D:  I=M_RDMEM(J.W); M_RR(I);  M_WRMEM(J.W,I); z80->DE.B.h=I; break;
    case RR_E:  I=M_RDMEM(J.W); M_RR(I);  M_WRMEM(J.W,I); z80->DE.B.l=I; break;
    case RR_H:  I=M_RDMEM(J.W); M_RR(I);  M_WRMEM(J.W,I); z80->HL.B.h=I; break;
    case RR_L:  I=M_RDMEM(J.W); M_RR(I);  M_WRMEM(J.W,I); z80->HL.B.l=I; break;
    case RR_xHL:  I=M_RDMEM(J.W); M_RR(I);  M_WRMEM(J.W,I); break;
    case RR_A:  I=M_RDMEM(J.W); M_RR(I);  M_WRMEM(J.W,I); z80->AF.B.h=I; break;

    case SLA_B: I=M_RDMEM(J.W); M_SLA(I); M_WRMEM(J.W,I); z80->BC.B.h=I; break;
    case SLA_C: I=M_RDMEM(J.W); M_SLA(I); M_WRMEM(J.W,I); z80->BC.B.l=I; break;
    case SLA_D: I=M_RDMEM(J.W); M_SLA(I); M_WRMEM(J.W,I); z80->DE.B.h=I; break;
    case SLA_E: I=M_RDMEM(J.W); M_SLA(I); M_WRMEM(J.W,I); z80->DE.B.l=I; break;
    case SLA_H: I=M_RDMEM(J.W); M_SLA(I); M_WRMEM(J.W,I); z80->HL.B.h=I; break;
    case SLA_L: I=M_RDMEM(J.W); M_SLA(I); M_WRMEM(J.W,I); z80->HL.B.l=I; break;
    case SLA_xHL: I=M_RDMEM(J.W); M_SLA(I); M_WRMEM(J.W,I); break;
    case SLA_A: I=M_RDMEM(J.W); M_SLA(I); M_WRMEM(J.W,I); z80->AF.B.h=I; break;

    case SRA_B: I=M_RDMEM(J.W); M_SRA(I); M_WRMEM(J.W,I); z80->BC.B.h=I; break;
    case SRA_C: I=M_RDMEM(J.W); M_SRA(I); M_WRMEM(J.W,I); z80->BC.B.l=I; break;
    case SRA_D: I=M_RDMEM(J.W); M_SRA(I); M_WRMEM(J.W,I); z80->DE.B.h=I; break;
    case SRA_E: I=M_RDMEM(J.W); M_SRA(I); M_WRMEM(J.W,I); z80->DE.B.l=I; break;
    case SRA_H: I=M_RDMEM(J.W); M_SRA(I); M_WRMEM(J.W,I); z80->HL.B.h=I; break;
    case SRA_L: I=M_RDMEM(J.W); M_SRA(I); M_WRMEM(J.W,I); z80->HL.B.l=I; break;
    case SRA_xHL: I=M_RDMEM(J.W); M_SRA(I); M_WRMEM(J.W,I); break;
    case SRA_A: I=M_RDMEM(J.W); M_SRA(I); M_WRMEM(J.W,I); z80->AF.B.h=I; break;

    case SLL_B: I=M_RDMEM(J.W); M_SLL(I); M_WRMEM(J.W,I); z80->BC.B.h=I; break;
    case SLL_C: I=M_RDMEM(J.W); M_SLL(I); M_WRMEM(J.W,I); z80->BC.B.l=I; break;
    case SLL_D: I=M_RDMEM(J.W); M_SLL(I); M_WRMEM(J.W,I); z80->DE.B.h=I; break;
    case SLL_E: I=M_RDMEM(J.W); M_SLL(I); M_WRMEM(J.W,I); z80->DE.B.l=I; break;
    case SLL_H: I=M_RDMEM(J.W); M_SLL(I); M_WRMEM(J.W,I); z80->HL.B.h=I; break;
    case SLL_L: I=M_RDMEM(J.W); M_SLL(I); M_WRMEM(J.W,I); z80->HL.B.l=I; break;
    case SLL_xHL: I=M_RDMEM(J.W); M_SLL(I); M_WRMEM(J.W,I); break;
    case SLL_A: I=M_RDMEM(J.W); M_SLL(I); M_WRMEM(J.W,I); z80->AF.B.h=I; break;

    case SRL_B: I=M_RDMEM(J.W); M_SRL(I); M_WRMEM(J.W,I); z80->BC.B.h=I; break;
    case SRL_C: I=M_RDMEM(J.W); M_SRL(I); M_WRMEM(J.W,I); z80->BC.B.l=I; break;
    case SRL_D: I=M_RDMEM(J.W); M_SRL(I); M_WRMEM(J.W,I); z80->DE.B.h=I; break;
    case SRL_E: I=M_RDMEM(J.W); M_SRL(I); M_WRMEM(J.W,I); z80->DE.B.l=I; break;
    case SRL_H: I=M_RDMEM(J.W); M_SRL(I); M_WRMEM(J.W,I); z80->HL.B.h=I; break;
    case SRL_L: I=M_RDMEM(J.W); M_SRL(I); M_WRMEM(J.W,I); z80->HL.B.l=I; break;
    case SRL_xHL: I=M_RDMEM(J.W); M_SRL(I); M_WRMEM(J.W,I); break;
    case SRL_A: I=M_RDMEM(J.W); M_SRL(I); M_WRMEM(J.W,I); z80->AF.B.h=I; break;


      /* ビット演算命令 */

    case BIT_0_B:
    case BIT_0_C:
    case BIT_0_D:
    case BIT_0_E:
    case BIT_0_H:
    case BIT_0_L:
    case BIT_0_xHL:
    case BIT_0_A:  I=M_RDMEM(J.W); M_BIT(0,I); break;

    case BIT_1_B:
    case BIT_1_C:
    case BIT_1_D:
    case BIT_1_E:
    case BIT_1_H:
    case BIT_1_L:
    case BIT_1_xHL:
    case BIT_1_A: I=M_RDMEM(J.W); M_BIT(1,I); break;

    case BIT_2_B:
    case BIT_2_C:
    case BIT_2_D:
    case BIT_2_E:
    case BIT_2_H:
    case BIT_2_L:
    case BIT_2_xHL:
    case BIT_2_A:  I=M_RDMEM(J.W); M_BIT(2,I); break;

    case BIT_3_B:
    case BIT_3_C:
    case BIT_3_D:
    case BIT_3_E:
    case BIT_3_H:
    case BIT_3_L:
    case BIT_3_xHL:
    case BIT_3_A:  I=M_RDMEM(J.W); M_BIT(3,I); break;

    case BIT_4_B:
    case BIT_4_C:
    case BIT_4_D:
    case BIT_4_E:
    case BIT_4_H:
    case BIT_4_L:
    case BIT_4_xHL:
    case BIT_4_A: I=M_RDMEM(J.W); M_BIT(4,I); break;

    case BIT_5_B:
    case BIT_5_C:
    case BIT_5_D:
    case BIT_5_E:
    case BIT_5_H:
    case BIT_5_L:
    case BIT_5_xHL:
    case BIT_5_A:  I=M_RDMEM(J.W); M_BIT(5,I); break;

    case BIT_6_B:
    case BIT_6_C:
    case BIT_6_D:
    case BIT_6_E:
    case BIT_6_H:
    case BIT_6_L:
    case BIT_6_xHL:
    case BIT_6_A:  I=M_RDMEM(J.W); M_BIT(6,I); break;

    case BIT_7_B:
    case BIT_7_C:
    case BIT_7_D:
    case BIT_7_E:
    case BIT_7_H:
    case BIT_7_L:
    case BIT_7_xHL:
    case BIT_7_A:  I=M_RDMEM(J.W); M_BIT(7,I); break;

    case RES_0_B: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case RES_0_C: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case RES_0_D: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case RES_0_E: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case RES_0_H: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case RES_0_L: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case RES_0_xHL: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I); break;
    case RES_0_A: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case RES_1_B: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case RES_1_C: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case RES_1_D: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case RES_1_E: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case RES_1_H: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case RES_1_L: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case RES_1_xHL: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I); break;
    case RES_1_A: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case RES_2_B: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case RES_2_C: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case RES_2_D: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case RES_2_E: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case RES_2_H: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case RES_2_L: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case RES_2_xHL: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I); break;
    case RES_2_A: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case RES_3_B: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case RES_3_C: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case RES_3_D: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case RES_3_E: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case RES_3_H: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case RES_3_L: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case RES_3_xHL: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I); break;
    case RES_3_A: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case RES_4_B: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case RES_4_C: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case RES_4_D: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case RES_4_E: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case RES_4_H: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case RES_4_L: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case RES_4_xHL: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I); break;
    case RES_4_A: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case RES_5_B: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case RES_5_C: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case RES_5_D: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case RES_5_E: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case RES_5_H: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case RES_5_L: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case RES_5_xHL: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I); break;
    case RES_5_A: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case RES_6_B: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case RES_6_C: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case RES_6_D: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case RES_6_E: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case RES_6_H: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case RES_6_L: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case RES_6_xHL: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I); break;
    case RES_6_A: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case RES_7_B: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case RES_7_C: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case RES_7_D: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case RES_7_E: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case RES_7_H: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case RES_7_L: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case RES_7_xHL: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I); break;
    case RES_7_A: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case SET_0_B: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case SET_0_C: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case SET_0_D: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case SET_0_E: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case SET_0_H: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case SET_0_L: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case SET_0_xHL: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I); break;
    case SET_0_A: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case SET_1_B: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case SET_1_C: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case SET_1_D: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case SET_1_E: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case SET_1_H: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case SET_1_L: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case SET_1_xHL: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I); break;
    case SET_1_A: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case SET_2_B: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case SET_2_C: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case SET_2_D: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case SET_2_E: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case SET_2_H: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case SET_2_L: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case SET_2_xHL: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I); break;
    case SET_2_A: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case SET_3_B: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case SET_3_C: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case SET_3_D: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case SET_3_E: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case SET_3_H: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case SET_3_L: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case SET_3_xHL: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I); break;
    case SET_3_A: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case SET_4_B: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case SET_4_C: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case SET_4_D: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case SET_4_E: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case SET_4_H: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case SET_4_L: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case SET_4_xHL: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I); break;
    case SET_4_A: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case SET_5_B: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case SET_5_C: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case SET_5_D: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case SET_5_E: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case SET_5_H: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case SET_5_L: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case SET_5_xHL: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I); break;
    case SET_5_A: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case SET_6_B: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case SET_6_C: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case SET_6_D: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case SET_6_E: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case SET_6_H: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case SET_6_L: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case SET_6_xHL: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I); break;
    case SET_6_A: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;

    case SET_7_B: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I);z80->BC.B.h=I;break;
    case SET_7_C: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I);z80->BC.B.l=I;break;
    case SET_7_D: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I);z80->DE.B.h=I;break;
    case SET_7_E: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I);z80->DE.B.l=I;break;
    case SET_7_H: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I);z80->HL.B.h=I;break;
    case SET_7_L: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I);z80->HL.B.l=I;break;
    case SET_7_xHL: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I); break;
    case SET_7_A: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I);z80->AF.B.h=I;break;
