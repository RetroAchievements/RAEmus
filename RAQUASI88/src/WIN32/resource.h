#ifndef RESOURCE_H_INCLUDED
#define RESOURCE_H_INCLUDED

/*---------------------------------------------------------------------------*/
#define M_SYS           10000
/*---------------------------------------------------------------------------*/
#define M_SYS_RESET     10100

#define M_SYS_MODE      10200
#define M_SYS_MODE_V2       10201
#define M_SYS_MODE_V1H      10202
#define M_SYS_MODE_V1S      10203
#define M_SYS_MODE_N        10204
#define M_SYS_MODE_4MH      10205
#define M_SYS_MODE_8MH      10206
#define M_SYS_MODE_SB       10207
#define M_SYS_MODE_SB2      10208

#define M_SYS_RESET_V2      10300
#define M_SYS_RESET_V1H     10400
#define M_SYS_RESET_V1S     10500

#define M_SYS_MENU      10600

#define M_SYS_SAVE      10700

#define M_SYS_EXIT      10800

/*---------------------------------------------------------------------------*/
#define M_SET           20000
/*---------------------------------------------------------------------------*/
#define M_SET_SPD       20100
#define M_SET_SPD_25        20101
#define M_SET_SPD_50        20102
#define M_SET_SPD_100       20103
#define M_SET_SPD_200       20104
#define M_SET_SPD_400       20105
#define M_SET_SPD_MAX       20199

#define M_SET_SUB       20200
#define M_SET_SUB_SOME      20201
#define M_SET_SUB_OFT       20202
#define M_SET_SUB_ALL       20203

#define M_SET_FDCWAIT       20300

#define M_SET_REF       20400
#define M_SET_REF_60        20401
#define M_SET_REF_30        20402
#define M_SET_REF_20        20403
#define M_SET_REF_15        20404

#define M_SET_INT       20500
#define M_SET_INT_NO        20501
#define M_SET_INT_SKIP      20502
#define M_SET_INT_YES       20503

#define M_SET_SIZ       20600
#define M_SET_SIZ_FULL      20601
#define M_SET_SIZ_HALF      20602
#define M_SET_SIZ_DOUBLE    20603

#define M_SET_FULLSCREEN    20650

#define M_SET_PCG       20700

#define M_SET_MO        20800
#define M_SET_MO_NO     20801
#define M_SET_MO_MOUSE      20802
#define M_SET_MO_JOYMO      20803
#define M_SET_MO_JOY        20804

#define M_SET_CUR       20900
#define M_SET_CUR_DEF       20901
#define M_SET_CUR_TEN       20902

#define M_SET_NUMLOCK       21000

#define M_SET_ROMAJI        21100

#define M_SET_FM        21200
#define M_SET_FM_MAME       21201
#define M_SET_FM_FMGEN      21202

#define M_SET_FRQ       21300
#define M_SET_FRQ_48        21301
#define M_SET_FRQ_44        21302
#define M_SET_FRQ_22        21303
#define M_SET_FRQ_11        21304

#define M_SET_BUF       21400
#define M_SET_BUF_800       21401
#define M_SET_BUF_400       21402
#define M_SET_BUF_200       21403
#define M_SET_BUF_100       21404

/*---------------------------------------------------------------------------*/
#define M_DRV           30000
/*---------------------------------------------------------------------------*/
#define M_DRV_DRV1      30100
#define M_DRV_DRV1_1        30101
#define M_DRV_DRV1_2        30102
#define M_DRV_DRV1_3        30103
#define M_DRV_DRV1_4        30104
#define M_DRV_DRV1_5        30105
#define M_DRV_DRV1_6        30106
#define M_DRV_DRV1_7        30107
#define M_DRV_DRV1_8        30108
#define M_DRV_DRV1_9        30109
#define M_DRV_DRV1_NO       30110
#define M_DRV_DRV1_CHG      30111

#define M_DRV_DRV2      30200
#define M_DRV_DRV2_1        30201
#define M_DRV_DRV2_2        30202
#define M_DRV_DRV2_3        30203
#define M_DRV_DRV2_4        30204
#define M_DRV_DRV2_5        30205
#define M_DRV_DRV2_6        30206
#define M_DRV_DRV2_7        30207
#define M_DRV_DRV2_8        30208
#define M_DRV_DRV2_9        30209
#define M_DRV_DRV2_NO       30210
#define M_DRV_DRV2_CHG      30211

#define M_DRV_CHG       30300
#define M_DRV_UNSET     30400

/*---------------------------------------------------------------------------*/
#define M_MISC          40000
/*---------------------------------------------------------------------------*/
#define M_MISC_CAPTURE      40100

#define M_MISC_RECORD       40200

#define M_MISC_CLOAD        40300
#define M_MISC_CLOAD_S      40301
#define M_MISC_CLOAD_U      40302

#define M_MISC_CSAVE        40400
#define M_MISC_CSAVE_S      40401
#define M_MISC_CSAVE_U      40402

#define M_MISC_SLOAD        40500
#define M_MISC_SLOAD_1      40501
#define M_MISC_SLOAD_2      40502
#define M_MISC_SLOAD_3      40503
#define M_MISC_SLOAD_4      40504
#define M_MISC_SLOAD_5      40505
#define M_MISC_SLOAD_6      40506
#define M_MISC_SLOAD_7      40507
#define M_MISC_SLOAD_8      40508
#define M_MISC_SLOAD_9      40509

#define M_MISC_SSAVE        40600
#define M_MISC_SSAVE_1      40601
#define M_MISC_SSAVE_2      40602
#define M_MISC_SSAVE_3      40603
#define M_MISC_SSAVE_4      40604
#define M_MISC_SSAVE_5      40605
#define M_MISC_SSAVE_6      40606
#define M_MISC_SSAVE_7      40607
#define M_MISC_SSAVE_8      40608
#define M_MISC_SSAVE_9      40609

#define M_MISC_STATUS       40700

/*---------------------------------------------------------------------------*/
#define M_HELP          50000
/*---------------------------------------------------------------------------*/
#define M_HELP_ABOUT        50100


#endif  /* RESOURCE_H_INCLUDED */
