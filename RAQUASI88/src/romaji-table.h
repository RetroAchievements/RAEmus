#define XR__A	RJ(P6,3,0)
#define XR__I	RJ(P2,5,0)
#define XR__U	RJ(P6,4,0)
#define XR__E	RJ(P6,5,0)
#define XR__O	RJ(P6,6,0)
#define XR_KA	RJ(P4,4,0)
#define XR_KI	RJ(P2,7,0)
#define XR_KU	RJ(P3,0,0)
#define XR_KE	RJ(P7,2,0)
#define XR_KO	RJ(P2,2,0)
#define XR_SA	RJ(P5,0,0)
#define XR_SI	RJ(P2,4,0)
#define XR_SU	RJ(P4,2,0)
#define XR_SE	RJ(P4,0,0)
#define XR_SO	RJ(P2,3,0)
#define XR_TA	RJ(P4,1,0)
#define XR_TI	RJ(P2,1,0)
#define XR_TU	RJ(P5,2,0)
#define XR_TE	RJ(P4,7,0)
#define XR_TO	RJ(P4,3,0)
#define XR_NA	RJ(P4,5,0)
#define XR_NI	RJ(P3,1,0)
#define XR_NU	RJ(P6,1,0)
#define XR_NE	RJ(P7,4,0)
#define XR_NO	RJ(P3,3,0)
#define XR_HA	RJ(P2,6,0)
#define XR_HI	RJ(P4,6,0)
#define XR_HU	RJ(P6,2,0)
#define XR_HE	RJ(P5,6,0)
#define XR_HO	RJ(P5,7,0)
#define XR_MA	RJ(P3,2,0)
#define XR_MI	RJ(P3,6,0)
#define XR_MU	RJ(P5,5,0)
#define XR_ME	RJ(P7,6,0)
#define XR_MO	RJ(P3,5,0)
#define XR_YA	RJ(P6,7,0)
#define XR_YU	RJ(P7,0,0)
#define XR_YO	RJ(P7,1,0)
#define XR_RA	RJ(P3,7,0)
#define XR_RI	RJ(P3,4,0)
#define XR_RU	RJ(P7,5,0)
#define XR_RE	RJ(P7,3,0)
#define XR_RO	RJ(P7,7,0)
#define XR_WA	RJ(P6,0,0)
#define XR_WO	RJ(P6,0,1)
#define XR__a	RJ(P6,3,1)
#define XR__i	RJ(P2,5,1)
#define XR__u	RJ(P6,4,1)
#define XR__e	RJ(P6,5,1)
#define XR__o	RJ(P6,6,1)
#define XR_ya	RJ(P6,7,1)
#define XR_yu	RJ(P7,0,1)
#define XR_yo	RJ(P7,1,1)
#define XR_tu	RJ(P5,2,1)
#define	XR__N	RJ(P5,1,0)
#define	XR_sp	RJ(P9,6,0)	/*    */
#define	XR_jj	RJ(P2,0,0)	/* ゛ */
#define	XR_pp	RJ(P5,3,0)	/* ゜ */
#define	XR_oo	RJ(P7,6,1)	/* ・ */
#define	XR_rr	RJ(P5,4,0)	/* ー */
#define	XR_aa	RJ(P5,3,1)	/* 「 */
#define	XR_ee	RJ(P5,5,1)	/* 」 */
#define	XR_xx	RJ(P7,5,1)	/* 。 */
#define	XR_yy	RJ(P7,4,1)	/* 、 */


static const romaji_list list_NN = { "", { XR__N, } };
static const romaji_list list_tu = { "", { XR_tu, } };

static const romaji_list list_mark[] =
{
		     
/*  	*/ { " ",    { XR_sp, } },
/* ゛ 	*/ { "@",    { XR_jj, } },
/* ゜ 	*/ { "[",    { XR_pp, } },
/* ・ 	*/ { "/",    { XR_oo, } },
/* ー 	*/ { "-",    { XR_rr, } },
/* 「 	*/ { "{",    { XR_aa, } },
/* 」 	*/ { "}",    { XR_ee, } },
/* 。 	*/ { ".",    { XR_xx, } },
/* 、 	*/ { ",",    { XR_yy, } },

};

static const romaji_list list_msime[] =
{

/* あ   */ { "A",    { XR__A, } },

/* ば   */ { "BA",   { XR_HA, XR_jj, } },
/* び   */ { "BI",   { XR_HI, XR_jj, } },
/* ぶ   */ { "BU",   { XR_HU, XR_jj, } },
/* べ   */ { "BE",   { XR_HE, XR_jj, } },
/* ぼ   */ { "BO",   { XR_HO, XR_jj, } },

/* か   */ { "CA",   { XR_KA, } },
/* し   */ { "CI",   { XR_SI, } },
/* く   */ { "CU",   { XR_KU, } },
/* せ   */ { "CE",   { XR_SE, } },
/* こ   */ { "CO",   { XR_KO, } },

/* だ   */ { "DA",   { XR_TA, XR_jj, } },
/* ぢ   */ { "DI",   { XR_TI, XR_jj, } },
/* づ   */ { "DU",   { XR_TU, XR_jj, } },
/* で   */ { "DE",   { XR_TE, XR_jj, } },
/* ど   */ { "DO",   { XR_TO, XR_jj, } },

/* え   */ { "E",    { XR__E, } },

/* ふぁ */ { "FA",   { XR_HU, XR__a, } },
/* ふぃ */ { "FI",   { XR_HU, XR__i, } },
/* ふ   */ { "FU",   { XR_HU,        } },
/* ふぇ */ { "FE",   { XR_HU, XR__e, } },
/* ふぉ */ { "FO",   { XR_HU, XR__o, } },

/* が   */ { "GA",   { XR_KA, XR_jj, } },
/* ぎ   */ { "GI",   { XR_KI, XR_jj, } },
/* ぐ   */ { "GU",   { XR_KU, XR_jj, } },
/* げ   */ { "GE",   { XR_KE, XR_jj, } },
/* ご   */ { "GO",   { XR_KO, XR_jj, } },

/* は   */ { "HA",   { XR_HA, } },
/* ひ   */ { "HI",   { XR_HI, } },
/* ふ   */ { "HU",   { XR_HU, } },
/* へ   */ { "HE",   { XR_HE, } },
/* ほ   */ { "HO",   { XR_HO, } },

/* い   */ { "I",    { XR__I, } },

/* じゃ */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* じ   */ { "JI",   { XR_SI, XR_jj,        } },
/* じゅ */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* か   */ { "KA",   { XR_KA, } },
/* き   */ { "KI",   { XR_KI, } },
/* く   */ { "KU",   { XR_KU, } },
/* け   */ { "KE",   { XR_KE, } },
/* こ   */ { "KO",   { XR_KO, } },

/* ぁ   */ { "LA",   { XR__a, } },
/* ぃ   */ { "LI",   { XR__i, } },
/* ぅ   */ { "LU",   { XR__u, } },
/* ぇ   */ { "LE",   { XR__e, } },
/* ぉ   */ { "LO",   { XR__o, } },

/* ま   */ { "MA",   { XR_MA, } },
/* み   */ { "MI",   { XR_MI, } },
/* む   */ { "MU",   { XR_MU, } },
/* め   */ { "ME",   { XR_ME, } },
/* も   */ { "MO",   { XR_MO, } },

/* な   */ { "NA",   { XR_NA, } },
/* に   */ { "NI",   { XR_NI, } },
/* ぬ   */ { "NU",   { XR_NU, } },
/* ね   */ { "NE",   { XR_NE, } },
/* の   */ { "NO",   { XR_NO, } },

/* お   */ { "O",    { XR__O, } },

/* ぱ   */ { "PA",   { XR_HA, XR_pp, } },
/* ぴ   */ { "PI",   { XR_HI, XR_pp, } },
/* ぷ   */ { "PU",   { XR_HU, XR_pp, } },
/* ぺ   */ { "PE",   { XR_HE, XR_pp, } },
/* ぽ   */ { "PO",   { XR_HO, XR_pp, } },

/* くぁ */ { "QA",   { XR_KU, XR__a, } },
/* くぃ */ { "QI",   { XR_KU, XR__i, } },
/* く   */ { "QU",   { XR_KU,        } },
/* くぇ */ { "QE",   { XR_KU, XR__e, } },
/* くぉ */ { "QO",   { XR_KU, XR__o, } },

/* ら   */ { "RA",   { XR_RA, } },
/* り   */ { "RI",   { XR_RI, } },
/* る   */ { "RU",   { XR_RU, } },
/* れ   */ { "RE",   { XR_RE, } },
/* ろ   */ { "RO",   { XR_RO, } },

/* さ   */ { "SA",   { XR_SA, } },
/* し   */ { "SI",   { XR_SI, } },
/* す   */ { "SU",   { XR_SU, } },
/* せ   */ { "SE",   { XR_SE, } },
/* そ   */ { "SO",   { XR_SO, } },

/* た   */ { "TA",   { XR_TA, } },
/* ち   */ { "TI",   { XR_TI, } },
/* つ   */ { "TU",   { XR_TU, } },
/* て   */ { "TE",   { XR_TE, } },
/* と   */ { "TO",   { XR_TO, } },

/* う   */ { "U",    { XR__U, } },

/* ヴぁ */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ヴぃ */ { "VI",   { XR__U, XR_jj, XR__i, } },
/* ヴ   */ { "VU",   { XR__U, XR_jj,        } },
/* ヴぇ */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* ヴぉ */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* わ   */ { "WA",   { XR_WA,        } },
/* うぃ */ { "WI",   { XR__U, XR__i, } },
/* う   */ { "WU",   { XR__U,        } },
/* うぇ */ { "WE",   { XR__U, XR__e, } },
/* を   */ { "WO",   { XR_WO,        } },

/* ぁ   */ { "XA",   { XR__a, } },
/* ぃ   */ { "XI",   { XR__i, } },
/* ぅ   */ { "XU",   { XR__u, } },
/* ぇ   */ { "XE",   { XR__e, } },
/* ぉ   */ { "XO",   { XR__o, } },

/* や   */ { "YA",   { XR_YA,        } },
/* い   */ { "YI",   { XR__I,        } },
/* ゆ   */ { "YU",   { XR_YU,        } },
/* いぇ */ { "YE",   { XR__I, XR__e, } },
/* よ   */ { "YO",   { XR_YO,        } },

/* ざ   */ { "ZA",   { XR_SA, XR_jj, } },
/* じ   */ { "ZI",   { XR_SI, XR_jj, } },
/* ず   */ { "ZU",   { XR_SU, XR_jj, } },
/* ぜ   */ { "ZE",   { XR_SE, XR_jj, } },
/* ぞ   */ { "ZO",   { XR_SO, XR_jj, } },

/* びゃ */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* びぃ */ { "BYI",  { XR_HI, XR_jj, XR__i, } },
/* びゅ */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* びぇ */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* びょ */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* ちゃ */ { "CYA",  { XR_TI, XR_ya, } },
/* ちぃ */ { "CYI",  { XR_TI, XR__i, } },
/* ちゅ */ { "CYU",  { XR_TI, XR_yu, } },
/* ちぇ */ { "CYE",  { XR_TI, XR__e, } },
/* ちょ */ { "CYO",  { XR_TI, XR_yo, } },
		     
/* ぢゃ */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* ぢぃ */ { "DYI",  { XR_TI, XR_jj, XR__i, } },
/* ぢゅ */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* ぢぇ */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* ぢょ */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* ふゃ */ { "FYA",  { XR_HU, XR_ya, } },
/* ふぃ */ { "FYI",  { XR_HU, XR__i, } },
/* ふゅ */ { "FYU",  { XR_HU, XR_yu, } },
/* ふぇ */ { "FYE",  { XR_HU, XR__e, } },
/* ふょ */ { "FYO",  { XR_HU, XR_yo, } },
		     
/* ぎゃ */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ぎぃ */ { "GYI",  { XR_KI, XR_jj, XR__i, } },
/* ぎゅ */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ぎぇ */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ぎょ */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* ひゃ */ { "HYA",  { XR_HI, XR_ya, } },
/* ひぃ */ { "HYI",  { XR_HI, XR__i, } },
/* ひゅ */ { "HYU",  { XR_HI, XR_yu, } },
/* ひぇ */ { "HYE",  { XR_HI, XR__e, } },
/* ひょ */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* じゃ */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* じぃ */ { "JYI",  { XR_SI, XR_jj, XR__i, } },
/* じゅ */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* きゃ */ { "KYA",  { XR_KI, XR_ya, } },
/* きぃ */ { "KYI",  { XR_KI, XR__i, } },
/* きゅ */ { "KYU",  { XR_KI, XR_yu, } },
/* きぇ */ { "KYE",  { XR_KI, XR__e, } },
/* きょ */ { "KYO",  { XR_KI, XR_yo, } },
		     
/* ゃ   */ { "LYA",  { XR_ya, } },
/* ぃ   */ { "LYI",  { XR__i, } },
/* ゅ   */ { "LYU",  { XR_yu, } },
/* ぇ   */ { "LYE",  { XR__e, } },
/* ょ   */ { "LYO",  { XR_yo, } },
		     
/* みゃ */ { "MYA",  { XR_MI, XR_ya, } },
/* みぃ */ { "MYI",  { XR_MI, XR__i, } },
/* みゅ */ { "MYU",  { XR_MI, XR_yu, } },
/* みぇ */ { "MYE",  { XR_MI, XR__e, } },
/* みょ */ { "MYO",  { XR_MI, XR_yo, } },

/* にゃ */ { "NYA",  { XR_NI, XR_ya, } },
/* にぃ */ { "NYI",  { XR_NI, XR__i, } },
/* にゅ */ { "NYU",  { XR_NI, XR_yu, } },
/* にぇ */ { "NYE",  { XR_NI, XR__e, } },
/* にょ */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* ぴゃ */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* ぴぃ */ { "PYI",  { XR_HI, XR_pp, XR__i, } },
/* ぴゅ */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* ぴぇ */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* ぴょ */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* くゃ */ { "QYA",  { XR_KU, XR_ya, } },
/* くぃ */ { "QYI",  { XR_KU, XR__i, } },
/* くゅ */ { "QYU",  { XR_KU, XR_yu, } },
/* くぇ */ { "QYE",  { XR_KU, XR__e, } },
/* くょ */ { "QYO",  { XR_KU, XR_yo, } },
		     
/* りゃ */ { "RYA",  { XR_RI, XR_ya, } },
/* りぃ */ { "RYI",  { XR_RI, XR__i, } },
/* りゅ */ { "RYU",  { XR_RI, XR_yu, } },
/* りぇ */ { "RYE",  { XR_RI, XR__e, } },
/* りょ */ { "RYO",  { XR_RI, XR_yo, } },

/* しゃ */ { "SYA",  { XR_SI, XR_ya, } },
/* しぃ */ { "SYI",  { XR_SI, XR__i, } },
/* しゅ */ { "SYU",  { XR_SI, XR_yu, } },
/* しぇ */ { "SYE",  { XR_SI, XR__e, } },
/* しょ */ { "SYO",  { XR_SI, XR_yo, } },
		     
/* ちゃ */ { "TYA",  { XR_TI, XR_ya, } },
/* ちぃ */ { "TYI",  { XR_TI, XR__i, } },
/* ちゅ */ { "TYU",  { XR_TI, XR_yu, } },
/* ちぇ */ { "TYE",  { XR_TI, XR__e, } },
/* ちょ */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* ヴゃ */ { "VYA",  { XR__U, XR_jj, XR_ya, } },
/* ヴぃ */ { "VYI",  { XR__U, XR_jj, XR__i, } },
/* ヴゅ */ { "VYU",  { XR__U, XR_jj, XR_yu, } },
/* ヴぇ */ { "VYE",  { XR__U, XR_jj, XR__e, } },
/* ヴょ */ { "VYO",  { XR__U, XR_jj, XR_yo, } },
		     
/* ゃ   */ { "XYA",  { XR_ya, } },
/* ぃ   */ { "XYI",  { XR__i, } },
/* ゅ   */ { "XYU",  { XR_yu, } },
/* ぇ   */ { "XYE",  { XR__e, } },
/* ょ   */ { "XYO",  { XR_yo, } },

/* じゃ */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* じぃ */ { "ZYI",  { XR_SI, XR_jj, XR__i, } },
/* じゅ */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ちゃ */ { "CHA",  { XR_TI, XR_ya, } },
/* ち   */ { "CHI",  { XR_TI,        } },
/* ちゅ */ { "CHU",  { XR_TI, XR_yu, } },
/* ちぇ */ { "CHE",  { XR_TI, XR__e, } },
/* ちょ */ { "CHO",  { XR_TI, XR_yo, } },
		     
/* でゃ */ { "DHA",  { XR_TE, XR_jj, XR_ya, } },
/* でぃ */ { "DHI",  { XR_TE, XR_jj, XR__i, } },
/* でゅ */ { "DHU",  { XR_TE, XR_jj, XR_yu, } },
/* でぇ */ { "DHE",  { XR_TE, XR_jj, XR__e, } },
/* でょ */ { "DHO",  { XR_TE, XR_jj, XR_yo, } },
		     
/* しゃ */ { "SHA",  { XR_SI, XR_ya, } },
/* し   */ { "SHI",  { XR_SI,        } },
/* しゅ */ { "SHU",  { XR_SI, XR_yu, } },
/* しぇ */ { "SHE",  { XR_SI, XR__e, } },
/* しょ */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* てゃ */ { "THA",  { XR_TE, XR_ya, } },
/* てぃ */ { "THI",  { XR_TE, XR__i, } },
/* てゅ */ { "THU",  { XR_TE, XR_yu, } },
/* てぇ */ { "THE",  { XR_TE, XR__e, } },
/* てょ */ { "THO",  { XR_TE, XR_yo, } },
		     
/* うぁ */ { "WHA",  { XR__U, XR_ya, } },
/* うぃ */ { "WHI",  { XR__U, XR__i, } },
/* う   */ { "WHU",  { XR__U,        } },
/* うぇ */ { "WHE",  { XR__U, XR__e, } },
/* うぉ */ { "WHO",  { XR__U, XR_yo, } },
		     
/* つぁ */ { "TSA",  { XR_TU, XR_ya, } },
/* つぃ */ { "TSI",  { XR_TU, XR__i, } },
/* つ   */ { "TSU",  { XR_TU,        } },
/* つぇ */ { "TSE",  { XR_TU, XR__e, } },
/* つぉ */ { "TSO",  { XR_TU, XR_yo, } },
		     
/* っ   */ { "XTU",  { XR_tu, } },
		     
/* くぁ */ { "QWA",  { XR_KU, XR__a, } },
/* くぃ */ { "QWI",  { XR_KU, XR__i, } },
/* くぅ */ { "QWU",  { XR_KU, XR__u, } },
/* くぇ */ { "QWE",  { XR_KU, XR__e, } },
/* くぉ */ { "QWO",  { XR_KU, XR__o, } },
		     
/* ん   */ { "NN",   { XR__N, } },
/* ん   */ { "N'",   { XR__N, } },
		     
};

static const romaji_list list_atok[] =
{

/* あ   */ { "A",    { XR__A, } },

/* ば   */ { "BA",   { XR_HA, XR_jj, } },
/* び   */ { "BI",   { XR_HI, XR_jj, } },
/* ぶ   */ { "BU",   { XR_HU, XR_jj, } },
/* べ   */ { "BE",   { XR_HE, XR_jj, } },
/* ぼ   */ { "BO",   { XR_HO, XR_jj, } },

/* だ   */ { "DA",   { XR_TA, XR_jj, } },
/* ぢ   */ { "DI",   { XR_TI, XR_jj, } },
/* づ   */ { "DU",   { XR_TU, XR_jj, } },
/* で   */ { "DE",   { XR_TE, XR_jj, } },
/* ど   */ { "DO",   { XR_TO, XR_jj, } },

/* え   */ { "E",    { XR__E, } },

/* ふぁ */ { "FA",   { XR_HU, XR__a, } },
/* ふぃ */ { "FI",   { XR_HU, XR__i, } },
/* ふ   */ { "FU",   { XR_HU,        } },
/* ふぇ */ { "FE",   { XR_HU, XR__e, } },
/* ふぉ */ { "FO",   { XR_HU, XR__o, } },

/* が   */ { "GA",   { XR_KA, XR_jj, } },
/* ぎ   */ { "GI",   { XR_KI, XR_jj, } },
/* ぐ   */ { "GU",   { XR_KU, XR_jj, } },
/* げ   */ { "GE",   { XR_KE, XR_jj, } },
/* ご   */ { "GO",   { XR_KO, XR_jj, } },

/* は   */ { "HA",   { XR_HA, } },
/* ひ   */ { "HI",   { XR_HI, } },
/* ふ   */ { "HU",   { XR_HU, } },
/* へ   */ { "HE",   { XR_HE, } },
/* ほ   */ { "HO",   { XR_HO, } },

/* い   */ { "I",    { XR__I, } },

/* じゃ */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* じ   */ { "JI",   { XR_SI, XR_jj,        } },
/* じゅ */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* か   */ { "KA",   { XR_KA, } },
/* き   */ { "KI",   { XR_KI, } },
/* く   */ { "KU",   { XR_KU, } },
/* け   */ { "KE",   { XR_KE, } },
/* こ   */ { "KO",   { XR_KO, } },

/* ぁ   */ { "LA",   { XR__a, } },
/* ぃ   */ { "LI",   { XR__i, } },
/* ぅ   */ { "LU",   { XR__u, } },
/* ぇ   */ { "LE",   { XR__e, } },
/* ぉ   */ { "LO",   { XR__o, } },

/* ま   */ { "MA",   { XR_MA, } },
/* み   */ { "MI",   { XR_MI, } },
/* む   */ { "MU",   { XR_MU, } },
/* め   */ { "ME",   { XR_ME, } },
/* も   */ { "MO",   { XR_MO, } },

/* な   */ { "NA",   { XR_NA, } },
/* に   */ { "NI",   { XR_NI, } },
/* ぬ   */ { "NU",   { XR_NU, } },
/* ね   */ { "NE",   { XR_NE, } },
/* の   */ { "NO",   { XR_NO, } },

/* お   */ { "O",    { XR__O, } },

/* ぱ   */ { "PA",   { XR_HA, XR_pp, } },
/* ぴ   */ { "PI",   { XR_HI, XR_pp, } },
/* ぷ   */ { "PU",   { XR_HU, XR_pp, } },
/* ぺ   */ { "PE",   { XR_HE, XR_pp, } },
/* ぽ   */ { "PO",   { XR_HO, XR_pp, } },

/* ら   */ { "RA",   { XR_RA, } },
/* り   */ { "RI",   { XR_RI, } },
/* る   */ { "RU",   { XR_RU, } },
/* れ   */ { "RE",   { XR_RE, } },
/* ろ   */ { "RO",   { XR_RO, } },

/* さ   */ { "SA",   { XR_SA, } },
/* し   */ { "SI",   { XR_SI, } },
/* す   */ { "SU",   { XR_SU, } },
/* せ   */ { "SE",   { XR_SE, } },
/* そ   */ { "SO",   { XR_SO, } },

/* た   */ { "TA",   { XR_TA, } },
/* ち   */ { "TI",   { XR_TI, } },
/* つ   */ { "TU",   { XR_TU, } },
/* て   */ { "TE",   { XR_TE, } },
/* と   */ { "TO",   { XR_TO, } },

/* う   */ { "U",    { XR__U, } },

/* ヴぁ */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ヴぃ */ { "VI",   { XR__U, XR_jj, XR__i, } },
/* ヴ   */ { "VU",   { XR__U, XR_jj,        } },
/* ヴぇ */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* ヴぉ */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* わ   */ { "WA",   { XR_WA,        } },
/* うぃ */ { "WI",   { XR__U, XR__i, } },
/* う   */ { "WU",   { XR__U,        } },
/* うぇ */ { "WE",   { XR__U, XR__e, } },
/* を   */ { "WO",   { XR_WO,        } },

/* ぁ   */ { "XA",   { XR__a, } },
/* ぃ   */ { "XI",   { XR__i, } },
/* ぅ   */ { "XU",   { XR__u, } },
/* ぇ   */ { "XE",   { XR__e, } },
/* ぉ   */ { "XO",   { XR__o, } },

/* や   */ { "YA",   { XR_YA,        } },
/* い   */ { "YI",   { XR__I,        } },
/* ゆ   */ { "YU",   { XR_YU,        } },
/* いぇ */ { "YE",   { XR__I, XR__e, } },
/* よ   */ { "YO",   { XR_YO,        } },

/* ざ   */ { "ZA",   { XR_SA, XR_jj, } },
/* じ   */ { "ZI",   { XR_SI, XR_jj, } },
/* ず   */ { "ZU",   { XR_SU, XR_jj, } },
/* ぜ   */ { "ZE",   { XR_SE, XR_jj, } },
/* ぞ   */ { "ZO",   { XR_SO, XR_jj, } },

/* びゃ */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* びぃ */ { "BYI",  { XR_HI, XR_jj, XR__i, } },
/* びゅ */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* びぇ */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* びょ */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* ちゃ */ { "CYA",  { XR_TI, XR_ya, } },
/* ちぃ */ { "CYI",  { XR_TI, XR__i, } },
/* ちゅ */ { "CYU",  { XR_TI, XR_yu, } },
/* ちぇ */ { "CYE",  { XR_TI, XR__e, } },
/* ちょ */ { "CYO",  { XR_TI, XR_yo, } },
		     
/* ぢゃ */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* ぢぃ */ { "DYI",  { XR_TI, XR_jj, XR__i, } },
/* ぢゅ */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* ぢぇ */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* ぢょ */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* ふゃ */ { "FYA",  { XR_HU, XR_ya, } },
/* ふぃ */ { "FYI",  { XR_HU, XR__i, } },
/* ふゅ */ { "FYU",  { XR_HU, XR_yu, } },
/* ふぇ */ { "FYE",  { XR_HU, XR__e, } },
/* ふょ */ { "FYO",  { XR_HU, XR_ya, } },
		     
/* ぎゃ */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ぎぃ */ { "GYI",  { XR_KI, XR_jj, XR__i, } },
/* ぎゅ */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ぎぇ */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ぎょ */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* ひゃ */ { "HYA",  { XR_HI, XR_ya, } },
/* ひぃ */ { "HYI",  { XR_HI, XR__i, } },
/* ひゅ */ { "HYU",  { XR_HI, XR_yu, } },
/* ひぇ */ { "HYE",  { XR_HI, XR__e, } },
/* ひょ */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* じゃ */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* じぃ */ { "JYI",  { XR_SI, XR_jj, XR__i, } },
/* じゅ */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },

/* きゃ */ { "KYA",  { XR_KI, XR_ya, } },
/* きぃ */ { "KYI",  { XR_KI, XR__i, } },
/* きゅ */ { "KYU",  { XR_KI, XR_yu, } },
/* きぇ */ { "KYE",  { XR_KI, XR__e, } },
/* きょ */ { "KYO",  { XR_KI, XR_yo, } },
		     
/* ゃ   */ { "LYA",  { XR_ya, } },
/* ぃ   */ { "LYI",  { XR__i, } },
/* ゅ   */ { "LYU",  { XR_yu, } },
/* ぇ   */ { "LYE",  { XR__e, } },
/* ょ   */ { "LYO",  { XR_yo, } },
		     
/* みゃ */ { "MYA",  { XR_MI, XR_ya, } },
/* みぃ */ { "MYI",  { XR_MI, XR__i, } },
/* みゅ */ { "MYU",  { XR_MI, XR_yu, } },
/* みぇ */ { "MYE",  { XR_MI, XR__e, } },
/* みょ */ { "MYO",  { XR_MI, XR_yo, } },
		     
/* にゃ */ { "NYA",  { XR_NI, XR_ya, } },
/* にぃ */ { "NYI",  { XR_NI, XR__i, } },
/* にゅ */ { "NYU",  { XR_NI, XR_yu, } },
/* にぇ */ { "NYE",  { XR_NI, XR__e, } },
/* にょ */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* ぴゃ */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* ぴぃ */ { "PYI",  { XR_HI, XR_pp, XR__i, } },
/* ぴゅ */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* ぴぇ */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* ぴょ */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* りゃ */ { "RYA",  { XR_RI, XR_ya, } },
/* りぃ */ { "RYI",  { XR_RI, XR__i, } },
/* りゅ */ { "RYU",  { XR_RI, XR_yu, } },
/* りぇ */ { "RYE",  { XR_RI, XR__e, } },
/* りょ */ { "RYO",  { XR_RI, XR_yo, } },
		     
/* しゃ */ { "SYA",  { XR_SI, XR_ya, } },
/* しぃ */ { "SYI",  { XR_SI, XR__i, } },
/* しゅ */ { "SYU",  { XR_SI, XR_yu, } },
/* しぇ */ { "SYE",  { XR_SI, XR__e, } },
/* しょ */ { "SYO",  { XR_SI, XR_yo, } },

/* ちゃ */ { "TYA",  { XR_TI, XR_ya, } },
/* ちぃ */ { "TYI",  { XR_TI, XR__i, } },
/* ちゅ */ { "TYU",  { XR_TI, XR_yu, } },
/* ちぇ */ { "TYE",  { XR_TI, XR__e, } },
/* ちょ */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* ゃ   */ { "XYA",  { XR_ya, } },
/* ぃ   */ { "XYI",  { XR__i, } },
/* ゅ   */ { "XYU",  { XR_yu, } },
/* ぇ   */ { "XYE",  { XR__e, } },
/* ょ   */ { "XYO",  { XR_yo, } },
		     
/* じゃ */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* じぃ */ { "ZYI",  { XR_SI, XR_jj, XR__i, } },
/* じゅ */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ちゃ */ { "CHA",  { XR_TI, XR_ya, } },
/* ち   */ { "CHI",  { XR_TI,        } },
/* ちゅ */ { "CHU",  { XR_TI, XR_yu, } },
/* ちぇ */ { "CHE",  { XR_TI, XR__e, } },
/* ちょ */ { "CHO",  { XR_TI, XR_yo, } },

/* でゃ */ { "DHA",  { XR_TE, XR_jj, XR_ya, } },
/* でぃ */ { "DHI",  { XR_TE, XR_jj, XR__i, } },
/* でゅ */ { "DHU",  { XR_TE, XR_jj, XR_yu, } },
/* でぇ */ { "DHE",  { XR_TE, XR_jj, XR__e, } },
/* でょ */ { "DHO",  { XR_TE, XR_jj, XR_yo, } },
		     
/* しゃ */ { "SHA",  { XR_SI, XR_ya, } },
/* し   */ { "SHI",  { XR_SI,        } },
/* しゅ */ { "SHU",  { XR_SI, XR_yu, } },
/* しぇ */ { "SHE",  { XR_SI, XR__e, } },
/* しょ */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* てゃ */ { "THA",  { XR_TE, XR_ya, } },
/* てぃ */ { "THI",  { XR_TE, XR__i, } },
/* てゅ */ { "THU",  { XR_TE, XR_yu, } },
/* てぇ */ { "THE",  { XR_TE, XR__e, } },
/* てょ */ { "THO",  { XR_TE, XR_yo, } },
		     
/* つぁ */ { "TSA",  { XR_TU, XR__a, } },
/* つぃ */ { "TSI",  { XR_TU, XR__i, } },
/* つ   */ { "TSU",  { XR_TU,        } },
/* つぇ */ { "TSE",  { XR_TU, XR__e, } },
/* つぉ */ { "TSO",  { XR_TU, XR__o, } },
		     
/* っ   */ { "XTU",  { XR_tu, } },
/* っ   */ { "XTSU", { XR_tu, } },
		     
/* ん   */ { "NN",   { XR__N, } },
/* ん   */ { "N'",   { XR__N, } },

};

static const romaji_list list_egg[] =
{

/* あ   */ { "A",    { XR__A, } },

/* ば   */ { "BA",   { XR_HA, XR_jj, } },
/* び   */ { "BI",   { XR_HI, XR_jj, } },
/* ぶ   */ { "BU",   { XR_HU, XR_jj, } },
/* べ   */ { "BE",   { XR_HE, XR_jj, } },
/* ぼ   */ { "BO",   { XR_HO, XR_jj, } },

/* だ   */ { "DA",   { XR_TA, XR_jj, } },
/* ぢ   */ { "DI",   { XR_TI, XR_jj, } },
/* づ   */ { "DU",   { XR_TU, XR_jj, } },
/* で   */ { "DE",   { XR_TE, XR_jj, } },
/* ど   */ { "DO",   { XR_TO, XR_jj, } },

/* え   */ { "E",    { XR__E, } },

/* ふぁ */ { "FA",   { XR_HU, XR__a, } },
/* ふぃ */ { "FI",   { XR_HU, XR__i, } },
/* ふ   */ { "FU",   { XR_HU,        } },
/* ふぇ */ { "FE",   { XR_HU, XR__e, } },
/* ふぉ */ { "FO",   { XR_HU, XR__o, } },

/* が   */ { "GA",   { XR_KA, XR_jj, } },
/* ぎ   */ { "GI",   { XR_KI, XR_jj, } },
/* ぐ   */ { "GU",   { XR_KU, XR_jj, } },
/* げ   */ { "GE",   { XR_KE, XR_jj, } },
/* ご   */ { "GO",   { XR_KO, XR_jj, } },

/* は   */ { "HA",   { XR_HA, } },
/* ひ   */ { "HI",   { XR_HI, } },
/* ふ   */ { "HU",   { XR_HU, } },
/* へ   */ { "HE",   { XR_HE, } },
/* ほ   */ { "HO",   { XR_HO, } },

/* い   */ { "I",    { XR__I, } },

/* じゃ */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* じ   */ { "JI",   { XR_SI, XR_jj,        } },
/* じゅ */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* か   */ { "KA",   { XR_KA, } },
/* き   */ { "KI",   { XR_KI, } },
/* く   */ { "KU",   { XR_KU, } },
/* け   */ { "KE",   { XR_KE, } },
/* こ   */ { "KO",   { XR_KO, } },

/* ら   */ { "LA",   { XR_RA, } },
/* り   */ { "LI",   { XR_RI, } },
/* る   */ { "LU",   { XR_RU, } },
/* れ   */ { "LE",   { XR_RE, } },
/* ろ   */ { "LO",   { XR_RO, } },

/* ま   */ { "MA",   { XR_MA, } },
/* み   */ { "MI",   { XR_MI, } },
/* む   */ { "MU",   { XR_MU, } },
/* め   */ { "ME",   { XR_ME, } },
/* も   */ { "MO",   { XR_MO, } },

/* な   */ { "NA",   { XR_NA, } },
/* に   */ { "NI",   { XR_NI, } },
/* ぬ   */ { "NU",   { XR_NU, } },
/* ね   */ { "NE",   { XR_NE, } },
/* の   */ { "NO",   { XR_NO, } },

/* お   */ { "O",    { XR__O, } },

/* ぱ   */ { "PA",   { XR_HA, XR_pp, } },
/* ぴ   */ { "PI",   { XR_HI, XR_pp, } },
/* ぷ   */ { "PU",   { XR_HU, XR_pp, } },
/* ぺ   */ { "PE",   { XR_HE, XR_pp, } },
/* ぽ   */ { "PO",   { XR_HO, XR_pp, } },

/* ら   */ { "RA",   { XR_RA, } },
/* り   */ { "RI",   { XR_RI, } },
/* る   */ { "RU",   { XR_RU, } },
/* れ   */ { "RE",   { XR_RE, } },
/* ろ   */ { "RO",   { XR_RO, } },

/* さ   */ { "SA",   { XR_SA, } },
/* し   */ { "SI",   { XR_SI, } },
/* す   */ { "SU",   { XR_SU, } },
/* せ   */ { "SE",   { XR_SE, } },
/* そ   */ { "SO",   { XR_SO, } },

/* た   */ { "TA",   { XR_TA, } },
/* ち   */ { "TI",   { XR_TI, } },
/* つ   */ { "TU",   { XR_TU, } },
/* て   */ { "TE",   { XR_TE, } },
/* と   */ { "TO",   { XR_TO, } },

/* う   */ { "U",    { XR__U, } },

/* ヴぁ */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ヴぃ */ { "VI",   { XR__U, XR_jj, XR__i, } },
/* ヴ   */ { "VU",   { XR__U, XR_jj,        } },
/* ヴぇ */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* ヴぉ */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* わ   */ { "WA",   { XR_WA,        } },
/* ゐ   */ { "WI",   { XR__I,        } },
/* う   */ { "WU",   { XR__U,        } },
/* ゑ   */ { "WE",   { XR__E,        } },
/* を   */ { "WO",   { XR_WO,        } },

/* ぁ   */ { "XA",   { XR__a, } },
/* ぃ   */ { "XI",   { XR__i, } },
/* ぅ   */ { "XU",   { XR__u, } },
/* ぇ   */ { "XE",   { XR__e, } },
/* ぉ   */ { "XO",   { XR__o, } },

/* や   */ { "YA",   { XR_YA,        } },
/* い   */ { "YI",   { XR__I,        } },
/* ゆ   */ { "YU",   { XR_YU,        } },
/* いぇ */ { "YE",   { XR__I, XR__e, } },
/* よ   */ { "YO",   { XR_YO,        } },

/* ざ   */ { "ZA",   { XR_SA, XR_jj, } },
/* じ   */ { "ZI",   { XR_SI, XR_jj, } },
/* ず   */ { "ZU",   { XR_SU, XR_jj, } },
/* ぜ   */ { "ZE",   { XR_SE, XR_jj, } },
/* ぞ   */ { "ZO",   { XR_SO, XR_jj, } },

/* びゃ */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* びゅ */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* びぇ */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* びょ */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* ぢゃ */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* でぃ */ { "DYI",  { XR_TE, XR_jj, XR__i, } },
/* ぢゅ */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* ぢぇ */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* ぢょ */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* ぎゃ */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ぎゅ */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ぎぇ */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ぎょ */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* ひゃ */ { "HYA",  { XR_HI, XR_ya, } },
/* ひゅ */ { "HYU",  { XR_HI, XR_yu, } },
/* ひぇ */ { "HYE",  { XR_HI, XR__e, } },
/* ひょ */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* じゃ */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* じゅ */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* きゃ */ { "KYA",  { XR_KI, XR_ya, } },
/* きゅ */ { "KYU",  { XR_KI, XR_yu, } },
/* きぇ */ { "KYE",  { XR_KI, XR__e, } },
/* きょ */ { "KYO",  { XR_KI, XR_yo, } },

/* りゃ */ { "LYA",  { XR_RI, XR_ya, } },
/* りゅ */ { "LYU",  { XR_RI, XR_yu, } },
/* りぇ */ { "LYE",  { XR_RI, XR__e, } },
/* りょ */ { "LYO",  { XR_RI, XR_yo, } },
		     
/* みゃ */ { "MYA",  { XR_MI, XR_ya, } },
/* みゅ */ { "MYU",  { XR_MI, XR_yu, } },
/* みぇ */ { "MYE",  { XR_MI, XR__e, } },
/* みょ */ { "MYO",  { XR_MI, XR_yo, } },
		     
/* にゃ */ { "NYA",  { XR_NI, XR_ya, } },
/* にゅ */ { "NYU",  { XR_NI, XR_yu, } },
/* にぇ */ { "NYE",  { XR_NI, XR__e, } },
/* にょ */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* ぴゃ */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* ぴゅ */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* ぴぇ */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* ぴょ */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* りゃ */ { "RYA",  { XR_RI, XR_ya, } },
/* りゅ */ { "RYU",  { XR_RI, XR_yu, } },
/* りぇ */ { "RYE",  { XR_RI, XR__e, } },
/* りょ */ { "RYO",  { XR_RI, XR_yo, } },

/* しゃ */ { "SYA",  { XR_SI, XR_ya, } },
/* しゅ */ { "SYU",  { XR_SI, XR_yu, } },
/* しぇ */ { "SYE",  { XR_SI, XR__e, } },
/* しょ */ { "SYO",  { XR_SI, XR_yo, } },
		     
/* ちゃ */ { "TYA",  { XR_TI, XR_ya, } },
/* てぃ */ { "TYI",  { XR_TE, XR__i, } },
/* ちゅ */ { "TYU",  { XR_TI, XR_yu, } },
/* ちぇ */ { "TYE",  { XR_TI, XR__e, } },
/* ちょ */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* ゃ   */ { "XYA",  { XR_ya, } },
/* ゅ   */ { "XYU",  { XR_yu, } },
/* ょ   */ { "XYO",  { XR_yo, } },
		     
/* じゃ */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* じゅ */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* じぇ */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* じょ */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ちゃ */ { "CHA",  { XR_TI, XR_ya, } },
/* ち   */ { "CHI",  { XR_TI,        } },
/* ちゅ */ { "CHU",  { XR_TI, XR_yu, } },
/* ちぇ */ { "CHE",  { XR_TI, XR__e, } },
/* ちょ */ { "CHO",  { XR_TI, XR_yo, } },

/* しゃ */ { "SHA",  { XR_SI, XR_ya, } },
/* し   */ { "SHI",  { XR_SI,        } },
/* しゅ */ { "SHU",  { XR_SI, XR_yu, } },
/* しぇ */ { "SHE",  { XR_SI, XR__e, } },
/* しょ */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* つぁ */ { "TSA",  { XR_TU, XR__a, } },
/* つぃ */ { "TSI",  { XR_TU, XR__i, } },
/* つ   */ { "TSU",  { XR_TU,        } },
/* つぇ */ { "TSE",  { XR_TU, XR__e, } },
/* つぉ */ { "TSO",  { XR_TU, XR__o, } },
		     
/* てぃ */ { "XTI",  { XR_TE, XR__i, } },
/* っ   */ { "XTU",  { XR_tu, } },
/* っ   */ { "XTSU", { XR_tu, } },
		     
/* ん   */ { "N'",   { XR__N, } },
		     
};


/*
MS-IME の ローマ字変換

	あ   い   う   え   お
B	ば   び   ぶ   べ   ぼ
C	か   し   く   せ   こ
D	だ   ぢ   づ   で   ど
F	ふぁ ふぃ ふ   ふぇ ふぉ
G	が   ぎ   ぐ   げ   ご
H	は   ひ   ふ   へ   ほ
J	じゃ じ   じゅ じぇ じょ
K	か   き   く   け   こ
L	ぁ   ぃ   ぅ   ぇ   ぉ
M	ま   み   む   め   も
N	な   に   ぬ   ね   の
P	ぱ   ぴ   ぷ   ぺ   ぽ
Q	くぁ くぃ く   くぇ くぉ
R	ら   り   る   れ   ろ
S	さ   し   す   せ   そ
T	た   ち   つ   て   と
V	ヴぁ ヴぃ ヴ   ヴぇ ヴぉ
W	わ   うぃ う   うぇ を
X	ぁ   ぃ   ぅ   ぇ   ぉ
Y	や   い   ゆ   いぇ よ
Z	ざ   じ   ず   ぜ   ぞ

BY	びゃ びぃ びゅ びぇ びょ
CY	ちゃ ちぃ ちゅ ちぇ ちょ
DY	ぢゃ ぢぃ ぢゅ ぢぇ ぢょ
FY	ふゃ ふぃ ふゅ ふぇ ふょ
GY	ぎゃ ぎぃ ぎゅ ぎぇ ぎょ
HY	ひゃ ひぃ ひゅ ひぇ ひょ
JY	じゃ じぃ じゅ じぇ じょ
KY	きゃ きぃ きゅ きぇ きょ
LY	ゃ   ぃ   ゅ   ぇ   ょ
MY	みゃ みぃ みゅ みぇ みょ
NY	にゃ にぃ にゅ にぇ にょ
PY	ぴゃ ぴぃ ぴゅ ぴぇ ぴょ
QY	くゃ くぃ くゅ くぇ くょ
RY	りゃ りぃ りゅ りぇ りょ
SY	しゃ しぃ しゅ しぇ しょ
TY	ちゃ ちぃ ちゅ ちぇ ちょ
VY	ヴゃ ヴぃ ヴゅ ヴぇ ヴょ
WY
XY	ゃ   ぃ   ゅ   ぇ   ょ
ZY	じゃ じぃ じゅ じぇ じょ

CH	ちゃ ち   ちゅ ちぇ ちょ
DH	でゃ でぃ でゅ でぇ でょ
SH	しゃ し   しゅ しぇ しょ
TH	てゃ てぃ てゅ てぇ てょ
WH	うぁ うぃ う   うぇ うぉ

TS	つぁ つぃ つ   つぇ つぉ

XK	ヵ             ヶ
XT	          っ
XTS
LW	ゎ
QW	くぁ くぃ くぅ くぇ くぉ
LK	ヵ             ヶ

NN	ん
N'	ん


ATOKのローマ字変換

	あ   い   う  え   お
B	ば   び   ぶ  べ   ぼ
C	
D	だ   ぢ   づ   で   ど
F	ふぁ ふぃ ふ   ふぇ ふぉ
G	が   ぎ   ぐ   げ   ご
H	は   ひ   ふ   へ   ほ
J	じゃ じ   じゅ じぇ じょ
K	か   き   く   け   こ
L	ぁ   ぃ   ぅ   ぇ   ぉ
M	ま   み   む   め   も
N	な   に   ぬ   ね   の
P	ぱ   ぴ   ぷ   ぺ   ぽ
Q	
R	ら   り   る   れ   ろ
S	さ   し   す   せ   そ
T	た   ち   つ   て   と
V	う゛ぁう゛ぃう゛う゛ぇう゛ぉ
W	わ   うぃ う   うぇ を
X	ぁ   ぃ   ぅ   ぇ   ぉ
Y	や   い   ゆ   いぇ よ
Z	ざ   じ   ず   ぜ   ぞ

BY	びゃ びぃ びゅ びぇ びょ
CY	ちゃ ちぃ ちゅ ちぇ ちょ
DY	ぢゃ ぢぃ ぢゅ ぢぇ ぢょ
FY	ふゃ ふぃ ふゅ ふぇ ふょ
GY	ぎゃ ぎぃ ぎゅ ぎぇ ぎょ
HY	ひゃ ひぃ ひゅ ひぇ ひょ
JY	じゃ じぃ じゅ じぇ じょ
KY	きゃ きぃ きゅ きぇ きょ
LY	ゃ   ぃ   ゅ   ぇ   ょ
MY	みゃ みぃ みゅ みぇ みょ
NY	にゃ にぃ にゅ にぇ にょ
PY	ぴゃ ぴぃ ぴゅ ぴぇ ぴょ
QY	
RY	りゃ りぃ りゅ りぇ りょ
SY	しゃ しぃ しゅ しぇ しょ
TY	ちゃ ちぃ ちゅ ちぇ ちょ
VY	
WY	
XY	ゃ   ぃ   ゅ   ぇ   ょ
ZY	じゃ じぃ じゅ じぇ じょ

CH	ちゃ ち   ちゅ ちぇ ちょ
DH	でゃ でぃ でゅ でぇ でょ
SH	しゃ し   しゅ しぇ しょ
TH	てゃ てぃ てゅ てぇ てょ
WH	

TS	つぁ つぃ つ   つぇ つぉ

XK	ヵ             ヶ
XT	          っ
XTS	          っ
LW	ゎ
QW	
LK	ヵ             ヶ

NN	ん
N'	ん


egg の ローマ字変換

	あ   い   う   え   お
B	ば   び   ぶ   べ   ぼ
C	
D	だ   ぢ   づ   で   ど
F	ふぁ ふぃ ふ   ふぇ ふぉ
G	が   ぎ   ぐ   げ   ご
H	は   ひ   ふ   へ   ほ
J	じゃ じ   じゅ じぇ じょ
K	か   き   く   け   こ
L	ら   り   る   れ   ろ
M	ま   み   む   め   も
N	な   に   ぬ   ね   の
P	ぱ   ぴ   ぷ   ぺ   ぽ
Q	
R	ら   り   る   れ   ろ
S	さ   し   す   せ   そ
T	た   ち   つ   て   と
V	ヴぁ ヴぃ ヴ   ヴぇ ヴぉ
W	わ   ゐ   う   ゑ   を
X	ぁ   ぃ   ぅ   ぇ   ぉ
Y	や   い   ゆ   いぇ よ
Z	ざ   じ   ず   ぜ   ぞ

BY	びゃ      びゅ びぇ びょ
CY	
DY	ぢゃ でぃ ぢゅ ぢぇ ぢょ
FY	
GY	ぎゃ      ぎゅ ぎぇ ぎょ
HY	ひゃ      ひゅ ひぇ ひょ
JY	じゃ      じゅ じぇ じょ
KY	きゃ      きゅ きぇ きょ
LY	りゃ      りゅ りぇ りょ
MY	みゃ      みゅ みぇ みょ
NY	にゃ      にゅ にぇ にょ
PY	ぴゃ      ぴゅ ぴぇ ぴょ
QY	
RY	りゃ      りゅ りぇ りょ
SY	しゃ      しゅ しぇ しょ
TY	ちゃ てぃ ちゅ ちぇ ちょ
VY	
WY	
XY	ゃ        ゅ        ょ
ZY	じゃ      じゅ じぇ じょ

CH	ちゃ ち   ちゅ ちぇ ちょ
DH	
SH	しゃ し   しゅ しぇ しょ
TH	
WH	

TS	つぁ つぃ つ   つぇ つぉ

XK	ヵ             ヶ
XT	     てぃ っ
XTS	          っ
LW	
QW	
LK	

N	ん
N'	ん
*/
