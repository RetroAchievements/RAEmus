/************************************************************************/
/*									*/
/* オペコード別処理 ( CB XX )						*/
/*									*/
/************************************************************************/


      /* ローテート・シフト命令 */

    case RLC_B:   M_RLC(z80->BC.B.h);  break;
    case RLC_C:   M_RLC(z80->BC.B.l);  break;
    case RLC_D:   M_RLC(z80->DE.B.h);  break;
    case RLC_E:   M_RLC(z80->DE.B.l);  break;
    case RLC_H:   M_RLC(z80->HL.B.h);  break;
    case RLC_L:   M_RLC(z80->HL.B.l);  break;
    case RLC_xHL:
      I=M_RDMEM(z80->HL.W); M_RLC(I); M_WRMEM(z80->HL.W,I);  break;
    case RLC_A:   M_RLC(z80->AF.B.h);  break;
      
    case RRC_B:   M_RRC(z80->BC.B.h);  break;
    case RRC_C:   M_RRC(z80->BC.B.l);  break;
    case RRC_D:   M_RRC(z80->DE.B.h);  break;
    case RRC_E:   M_RRC(z80->DE.B.l);  break;
    case RRC_H:   M_RRC(z80->HL.B.h);  break;
    case RRC_L:   M_RRC(z80->HL.B.l);  break;
    case RRC_xHL:
      I=M_RDMEM(z80->HL.W); M_RRC(I); M_WRMEM(z80->HL.W,I);  break;
    case RRC_A:   M_RRC(z80->AF.B.h);  break;
      
    case RL_B:   M_RL(z80->BC.B.h);  break;
    case RL_C:   M_RL(z80->BC.B.l);  break;
    case RL_D:   M_RL(z80->DE.B.h);  break;
    case RL_E:   M_RL(z80->DE.B.l);  break;
    case RL_H:   M_RL(z80->HL.B.h);  break;
    case RL_L:   M_RL(z80->HL.B.l);  break;
    case RL_xHL:
      I=M_RDMEM(z80->HL.W); M_RL(I); M_WRMEM(z80->HL.W,I);  break;
    case RL_A:   M_RL(z80->AF.B.h);  break;
      
    case RR_B:   M_RR(z80->BC.B.h);  break;
    case RR_C:   M_RR(z80->BC.B.l);  break;
    case RR_D:   M_RR(z80->DE.B.h);  break;
    case RR_E:   M_RR(z80->DE.B.l);  break;
    case RR_H:   M_RR(z80->HL.B.h);  break;
    case RR_L:   M_RR(z80->HL.B.l);  break;
    case RR_xHL:
      I=M_RDMEM(z80->HL.W); M_RR(I); M_WRMEM(z80->HL.W,I);  break;
    case RR_A:   M_RR(z80->AF.B.h);  break;
      
    case SLA_B:   M_SLA(z80->BC.B.h);  break;
    case SLA_C:   M_SLA(z80->BC.B.l);  break;
    case SLA_D:   M_SLA(z80->DE.B.h);  break;
    case SLA_E:   M_SLA(z80->DE.B.l);  break;
    case SLA_H:   M_SLA(z80->HL.B.h);  break;
    case SLA_L:   M_SLA(z80->HL.B.l);  break;
    case SLA_xHL:
      I=M_RDMEM(z80->HL.W); M_SLA(I); M_WRMEM(z80->HL.W,I);  break;
    case SLA_A:   M_SLA(z80->AF.B.h);  break;
      
    case SRA_B:   M_SRA(z80->BC.B.h);  break;
    case SRA_C:   M_SRA(z80->BC.B.l);  break;
    case SRA_D:   M_SRA(z80->DE.B.h);  break;
    case SRA_E:   M_SRA(z80->DE.B.l);  break;
    case SRA_H:   M_SRA(z80->HL.B.h);  break;
    case SRA_L:   M_SRA(z80->HL.B.l);  break;
    case SRA_xHL:
      I=M_RDMEM(z80->HL.W); M_SRA(I); M_WRMEM(z80->HL.W,I);  break;
    case SRA_A:   M_SRA(z80->AF.B.h);  break;
      
    case SLL_B:   M_SLL(z80->BC.B.h);  break;
    case SLL_C:   M_SLL(z80->BC.B.l);  break;
    case SLL_D:   M_SLL(z80->DE.B.h);  break;
    case SLL_E:   M_SLL(z80->DE.B.l);  break;
    case SLL_H:   M_SLL(z80->HL.B.h);  break;
    case SLL_L:   M_SLL(z80->HL.B.l);  break;
    case SLL_xHL:
      I=M_RDMEM(z80->HL.W); M_SLL(I); M_WRMEM(z80->HL.W,I);  break;
    case SLL_A:   M_SLL(z80->AF.B.h);  break;
      
    case SRL_B:   M_SRL(z80->BC.B.h);  break;
    case SRL_C:   M_SRL(z80->BC.B.l);  break;
    case SRL_D:   M_SRL(z80->DE.B.h);  break;
    case SRL_E:   M_SRL(z80->DE.B.l);  break;
    case SRL_H:   M_SRL(z80->HL.B.h);  break;
    case SRL_L:   M_SRL(z80->HL.B.l);  break;
    case SRL_xHL:
      I=M_RDMEM(z80->HL.W); M_SRL(I); M_WRMEM(z80->HL.W,I);  break;
    case SRL_A:   M_SRL(z80->AF.B.h);  break;
      
      /* ビット操作命令 */

    case BIT_0_B:   M_BIT(0,z80->BC.B.h);  break;
    case BIT_0_C:   M_BIT(0,z80->BC.B.l);  break;
    case BIT_0_D:   M_BIT(0,z80->DE.B.h);  break;
    case BIT_0_E:   M_BIT(0,z80->DE.B.l);  break;
    case BIT_0_H:   M_BIT(0,z80->HL.B.h);  break;
    case BIT_0_L:   M_BIT(0,z80->HL.B.l);  break;
    case BIT_0_xHL: I=M_RDMEM(z80->HL.W); M_BIT(0,I);  break;
    case BIT_0_A:   M_BIT(0,z80->AF.B.h);  break;
      
    case BIT_1_B:   M_BIT(1,z80->BC.B.h);  break;
    case BIT_1_C:   M_BIT(1,z80->BC.B.l);  break;
    case BIT_1_D:   M_BIT(1,z80->DE.B.h);  break;
    case BIT_1_E:   M_BIT(1,z80->DE.B.l);  break;
    case BIT_1_H:   M_BIT(1,z80->HL.B.h);  break;
    case BIT_1_L:   M_BIT(1,z80->HL.B.l);  break;
    case BIT_1_xHL: I=M_RDMEM(z80->HL.W); M_BIT(1,I);  break;
    case BIT_1_A:   M_BIT(1,z80->AF.B.h);  break;
      
    case BIT_2_B:   M_BIT(2,z80->BC.B.h);  break;
    case BIT_2_C:   M_BIT(2,z80->BC.B.l);  break;
    case BIT_2_D:   M_BIT(2,z80->DE.B.h);  break;
    case BIT_2_E:   M_BIT(2,z80->DE.B.l);  break;
    case BIT_2_H:   M_BIT(2,z80->HL.B.h);  break;
    case BIT_2_L:   M_BIT(2,z80->HL.B.l);  break;
    case BIT_2_xHL: I=M_RDMEM(z80->HL.W); M_BIT(2,I);  break;
    case BIT_2_A:   M_BIT(2,z80->AF.B.h);  break;
      
    case BIT_3_B:   M_BIT(3,z80->BC.B.h);  break;
    case BIT_3_C:   M_BIT(3,z80->BC.B.l);  break;
    case BIT_3_D:   M_BIT(3,z80->DE.B.h);  break;
    case BIT_3_E:   M_BIT(3,z80->DE.B.l);  break;
    case BIT_3_H:   M_BIT(3,z80->HL.B.h);  break;
    case BIT_3_L:   M_BIT(3,z80->HL.B.l);  break;
    case BIT_3_xHL: I=M_RDMEM(z80->HL.W); M_BIT(3,I);  break;
    case BIT_3_A:   M_BIT(3,z80->AF.B.h);  break;
      
    case BIT_4_B:   M_BIT(4,z80->BC.B.h);  break;
    case BIT_4_C:   M_BIT(4,z80->BC.B.l);  break;
    case BIT_4_D:   M_BIT(4,z80->DE.B.h);  break;
    case BIT_4_E:   M_BIT(4,z80->DE.B.l);  break;
    case BIT_4_H:   M_BIT(4,z80->HL.B.h);  break;
    case BIT_4_L:   M_BIT(4,z80->HL.B.l);  break;
    case BIT_4_xHL: I=M_RDMEM(z80->HL.W); M_BIT(4,I);  break;
    case BIT_4_A:   M_BIT(4,z80->AF.B.h);  break;

    case BIT_5_B:   M_BIT(5,z80->BC.B.h);  break;
    case BIT_5_C:   M_BIT(5,z80->BC.B.l);  break;
    case BIT_5_D:   M_BIT(5,z80->DE.B.h);  break;
    case BIT_5_E:   M_BIT(5,z80->DE.B.l);  break;
    case BIT_5_H:   M_BIT(5,z80->HL.B.h);  break;
    case BIT_5_L:   M_BIT(5,z80->HL.B.l);  break;
    case BIT_5_xHL: I=M_RDMEM(z80->HL.W); M_BIT(5,I);  break;
    case BIT_5_A:   M_BIT(5,z80->AF.B.h);  break;

    case BIT_6_B:   M_BIT(6,z80->BC.B.h);  break;
    case BIT_6_C:   M_BIT(6,z80->BC.B.l);  break;
    case BIT_6_D:   M_BIT(6,z80->DE.B.h);  break;
    case BIT_6_E:   M_BIT(6,z80->DE.B.l);  break;
    case BIT_6_H:   M_BIT(6,z80->HL.B.h);  break;
    case BIT_6_L:   M_BIT(6,z80->HL.B.l);  break;
    case BIT_6_xHL: I=M_RDMEM(z80->HL.W); M_BIT(6,I);  break;
    case BIT_6_A:   M_BIT(6,z80->AF.B.h);  break;

    case BIT_7_B:   M_BIT(7,z80->BC.B.h);  break;
    case BIT_7_C:   M_BIT(7,z80->BC.B.l);  break;
    case BIT_7_D:   M_BIT(7,z80->DE.B.h);  break;
    case BIT_7_E:   M_BIT(7,z80->DE.B.l);  break;
    case BIT_7_H:   M_BIT(7,z80->HL.B.h);  break;
    case BIT_7_L:   M_BIT(7,z80->HL.B.l);  break;
    case BIT_7_xHL: I=M_RDMEM(z80->HL.W); M_BIT(7,I);  break;
    case BIT_7_A:   M_BIT(7,z80->AF.B.h);  break;

    case RES_0_B:   M_RES(0,z80->BC.B.h);  break;
    case RES_0_C:   M_RES(0,z80->BC.B.l);  break;
    case RES_0_D:   M_RES(0,z80->DE.B.h);  break;
    case RES_0_E:   M_RES(0,z80->DE.B.l);  break;
    case RES_0_H:   M_RES(0,z80->HL.B.h);  break;
    case RES_0_L:   M_RES(0,z80->HL.B.l);  break;
    case RES_0_xHL:
      I=M_RDMEM(z80->HL.W); M_RES(0,I); M_WRMEM(z80->HL.W,I); break;
    case RES_0_A:   M_RES(0,z80->AF.B.h);  break;
      
    case RES_1_B:   M_RES(1,z80->BC.B.h);  break;
    case RES_1_C:   M_RES(1,z80->BC.B.l);  break;
    case RES_1_D:   M_RES(1,z80->DE.B.h);  break;
    case RES_1_E:   M_RES(1,z80->DE.B.l);  break;
    case RES_1_H:   M_RES(1,z80->HL.B.h);  break;
    case RES_1_L:   M_RES(1,z80->HL.B.l);  break;
    case RES_1_xHL:
      I=M_RDMEM(z80->HL.W); M_RES(1,I); M_WRMEM(z80->HL.W,I); break;
    case RES_1_A:   M_RES(1,z80->AF.B.h);  break;
      
    case RES_2_B:   M_RES(2,z80->BC.B.h);  break;
    case RES_2_C:   M_RES(2,z80->BC.B.l);  break;
    case RES_2_D:   M_RES(2,z80->DE.B.h);  break;
    case RES_2_E:   M_RES(2,z80->DE.B.l);  break;
    case RES_2_H:   M_RES(2,z80->HL.B.h);  break;
    case RES_2_L:   M_RES(2,z80->HL.B.l);  break;
    case RES_2_xHL:
      I=M_RDMEM(z80->HL.W); M_RES(2,I); M_WRMEM(z80->HL.W,I); break;
    case RES_2_A:   M_RES(2,z80->AF.B.h);  break;
      
    case RES_3_B:   M_RES(3,z80->BC.B.h);  break;
    case RES_3_C:   M_RES(3,z80->BC.B.l);  break;
    case RES_3_D:   M_RES(3,z80->DE.B.h);  break;
    case RES_3_E:   M_RES(3,z80->DE.B.l);  break;
    case RES_3_H:   M_RES(3,z80->HL.B.h);  break;
    case RES_3_L:   M_RES(3,z80->HL.B.l);  break;
    case RES_3_xHL:
      I=M_RDMEM(z80->HL.W); M_RES(3,I); M_WRMEM(z80->HL.W,I); break;
    case RES_3_A:   M_RES(3,z80->AF.B.h);  break;
      
    case RES_4_B:   M_RES(4,z80->BC.B.h);  break;
    case RES_4_C:   M_RES(4,z80->BC.B.l);  break;
    case RES_4_D:   M_RES(4,z80->DE.B.h);  break;
    case RES_4_E:   M_RES(4,z80->DE.B.l);  break;
    case RES_4_H:   M_RES(4,z80->HL.B.h);  break;
    case RES_4_L:   M_RES(4,z80->HL.B.l);  break;
    case RES_4_xHL:
      I=M_RDMEM(z80->HL.W); M_RES(4,I); M_WRMEM(z80->HL.W,I); break;
    case RES_4_A:   M_RES(4,z80->AF.B.h);  break;
      
    case RES_5_B:   M_RES(5,z80->BC.B.h);  break;
    case RES_5_C:   M_RES(5,z80->BC.B.l);  break;
    case RES_5_D:   M_RES(5,z80->DE.B.h);  break;
    case RES_5_E:   M_RES(5,z80->DE.B.l);  break;
    case RES_5_H:   M_RES(5,z80->HL.B.h);  break;
    case RES_5_L:   M_RES(5,z80->HL.B.l);  break;
    case RES_5_xHL:
      I=M_RDMEM(z80->HL.W); M_RES(5,I); M_WRMEM(z80->HL.W,I); break;
    case RES_5_A:   M_RES(5,z80->AF.B.h);  break;
      
    case RES_6_B:   M_RES(6,z80->BC.B.h);  break;
    case RES_6_C:   M_RES(6,z80->BC.B.l);  break;
    case RES_6_D:   M_RES(6,z80->DE.B.h);  break;
    case RES_6_E:   M_RES(6,z80->DE.B.l);  break;
    case RES_6_H:   M_RES(6,z80->HL.B.h);  break;
    case RES_6_L:   M_RES(6,z80->HL.B.l);  break;
    case RES_6_xHL:
      I=M_RDMEM(z80->HL.W); M_RES(6,I); M_WRMEM(z80->HL.W,I); break;
    case RES_6_A:   M_RES(6,z80->AF.B.h);  break;
      
    case RES_7_B:   M_RES(7,z80->BC.B.h);  break;
    case RES_7_C:   M_RES(7,z80->BC.B.l);  break;
    case RES_7_D:   M_RES(7,z80->DE.B.h);  break;
    case RES_7_E:   M_RES(7,z80->DE.B.l);  break;
    case RES_7_H:   M_RES(7,z80->HL.B.h);  break;
    case RES_7_L:   M_RES(7,z80->HL.B.l);  break;
    case RES_7_xHL:
      I=M_RDMEM(z80->HL.W); M_RES(7,I); M_WRMEM(z80->HL.W,I); break;
    case RES_7_A:   M_RES(7,z80->AF.B.h);  break;

    case SET_0_B:   M_SET(0,z80->BC.B.h);  break;
    case SET_0_C:   M_SET(0,z80->BC.B.l);  break;
    case SET_0_D:   M_SET(0,z80->DE.B.h);  break;
    case SET_0_E:   M_SET(0,z80->DE.B.l);  break;
    case SET_0_H:   M_SET(0,z80->HL.B.h);  break;
    case SET_0_L:   M_SET(0,z80->HL.B.l);  break;
    case SET_0_xHL:
      I=M_RDMEM(z80->HL.W); M_SET(0,I); M_WRMEM(z80->HL.W,I); break;
    case SET_0_A:   M_SET(0,z80->AF.B.h);  break;
      
    case SET_1_B:   M_SET(1,z80->BC.B.h);  break;
    case SET_1_C:   M_SET(1,z80->BC.B.l);  break;
    case SET_1_D:   M_SET(1,z80->DE.B.h);  break;
    case SET_1_E:   M_SET(1,z80->DE.B.l);  break;
    case SET_1_H:   M_SET(1,z80->HL.B.h);  break;
    case SET_1_L:   M_SET(1,z80->HL.B.l);  break;
    case SET_1_xHL:
      I=M_RDMEM(z80->HL.W); M_SET(1,I); M_WRMEM(z80->HL.W,I); break;
    case SET_1_A:   M_SET(1,z80->AF.B.h);  break;
      
    case SET_2_B:   M_SET(2,z80->BC.B.h);  break;
    case SET_2_C:   M_SET(2,z80->BC.B.l);  break;
    case SET_2_D:   M_SET(2,z80->DE.B.h);  break;
    case SET_2_E:   M_SET(2,z80->DE.B.l);  break;
    case SET_2_H:   M_SET(2,z80->HL.B.h);  break;
    case SET_2_L:   M_SET(2,z80->HL.B.l);  break;
    case SET_2_xHL:
      I=M_RDMEM(z80->HL.W); M_SET(2,I); M_WRMEM(z80->HL.W,I); break;
    case SET_2_A:   M_SET(2,z80->AF.B.h);  break;
      
    case SET_3_B:   M_SET(3,z80->BC.B.h);  break;
    case SET_3_C:   M_SET(3,z80->BC.B.l);  break;
    case SET_3_D:   M_SET(3,z80->DE.B.h);  break;
    case SET_3_E:   M_SET(3,z80->DE.B.l);  break;
    case SET_3_H:   M_SET(3,z80->HL.B.h);  break;
    case SET_3_L:   M_SET(3,z80->HL.B.l);  break;
    case SET_3_xHL:
      I=M_RDMEM(z80->HL.W); M_SET(3,I); M_WRMEM(z80->HL.W,I); break;
    case SET_3_A:   M_SET(3,z80->AF.B.h);  break;
      
    case SET_4_B:   M_SET(4,z80->BC.B.h);  break;
    case SET_4_C:   M_SET(4,z80->BC.B.l);  break;
    case SET_4_D:   M_SET(4,z80->DE.B.h);  break;
    case SET_4_E:   M_SET(4,z80->DE.B.l);  break;
    case SET_4_H:   M_SET(4,z80->HL.B.h);  break;
    case SET_4_L:   M_SET(4,z80->HL.B.l);  break;
    case SET_4_xHL:
      I=M_RDMEM(z80->HL.W); M_SET(4,I); M_WRMEM(z80->HL.W,I); break;
    case SET_4_A:   M_SET(4,z80->AF.B.h);  break;
      
    case SET_5_B:   M_SET(5,z80->BC.B.h);  break;
    case SET_5_C:   M_SET(5,z80->BC.B.l);  break;
    case SET_5_D:   M_SET(5,z80->DE.B.h);  break;
    case SET_5_E:   M_SET(5,z80->DE.B.l);  break;
    case SET_5_H:   M_SET(5,z80->HL.B.h);  break;
    case SET_5_L:   M_SET(5,z80->HL.B.l);  break;
    case SET_5_xHL:
      I=M_RDMEM(z80->HL.W); M_SET(5,I); M_WRMEM(z80->HL.W,I); break;
    case SET_5_A:   M_SET(5,z80->AF.B.h);  break;
      
    case SET_6_B:   M_SET(6,z80->BC.B.h);  break;
    case SET_6_C:   M_SET(6,z80->BC.B.l);  break;
    case SET_6_D:   M_SET(6,z80->DE.B.h);  break;
    case SET_6_E:   M_SET(6,z80->DE.B.l);  break;
    case SET_6_H:   M_SET(6,z80->HL.B.h);  break;
    case SET_6_L:   M_SET(6,z80->HL.B.l);  break;
    case SET_6_xHL:
      I=M_RDMEM(z80->HL.W); M_SET(6,I); M_WRMEM(z80->HL.W,I); break;
    case SET_6_A:   M_SET(6,z80->AF.B.h);  break;
      
    case SET_7_B:   M_SET(7,z80->BC.B.h);  break;
    case SET_7_C:   M_SET(7,z80->BC.B.l);  break;
    case SET_7_D:   M_SET(7,z80->DE.B.h);  break;
    case SET_7_E:   M_SET(7,z80->DE.B.l);  break;
    case SET_7_H:   M_SET(7,z80->HL.B.h);  break;
    case SET_7_L:   M_SET(7,z80->HL.B.l);  break;
    case SET_7_xHL:
      I=M_RDMEM(z80->HL.W); M_SET(7,I); M_WRMEM(z80->HL.W,I); break;
    case SET_7_A:   M_SET(7,z80->AF.B.h);  break;
