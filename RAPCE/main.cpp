/******************************************************************************
Ootake
E"ñd<N"®,ğ,·,é,Æ-â'è,ª<N,±,éê?,ª, ,é,Ì,ÅA"ñd<N"®,Í-hZ~,·,é,æ,¤,É,µ,½B
ECOM,Ì?Sú?»,ÆSJ.ú,ğ,±,±,Å,â,é,æ,¤,É,µ,½BVista,Åftf@fCf<f_fCfAffOZg-pZz,É
  ?Sú?»,µ,Ä,¨,©,È,¢,Æ.s^À'è,¾,Á,½Bv1.05
Ef}f<f`ffffBfAf^fCf},Ì¸"x,ğ,±,±,Åã,°,Ä,¨,­,æ,¤,É,µ,½Bv1.55

Copyright(C)2006-2010 Kitao Nakamura.
    Attach the source code when you open the remodeling version and the
    succession version to the public. and, please contact me by E-mail.
    Business use is prohibited.
	Additionally, it applies to "GNU General Public License". 
	?ü'¢"ÅEOãOp"Å,ğOöSJ,È,³,é,Æ,«,Í.K,¸f\[fXfR[fh,ğ"Y.t,µ,Ä,­,¾,³,¢B
	,»,ÌÛ,ÉZ-Oã,Å,©,Ü,¢,Ü,¹,ñ,Ì,ÅA,Ğ,Æ,±,Æ,¨'m,ç,¹,¢,½,¾,¯,é,ÆK,¢,Å,·B
	¤"I,È-~-p,Í<Ö,¶,Ü,·B
	, ,Æ,ÍuGNU General Public License(^ê"ÊOöO-~-p<-'øO_-ñ')v,É?,¶,Ü,·B

*******************************************************************************
	[main]
		-{fvffWfFfNfg,ÌffCf"SÖ",Å,·D

		The main function of the project.

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/

#define _WIN32_DCOM //v2.17XV

#include <objbase.h>
#include "App.h"

// gcc ,Í main ,ª'è<`,³,ê,Ä,¢,é,Æ
// WinMain ,æ,è,àæ,É main ,ğOÄ,ñ,Å,µ,Ü,¤,ç,µ,¢DDD
// __main__ ,Æ,¢,¤,Ì,Í,»,Ì workaround.
int
__main__(
	int			argc,
	char**		argv)
{
	HANDLE		hMutex;
	TIMECAPS	tc;

	//Kitao'Ç?ÁB"ñd<N"®,ğ-hZ~
	hMutex = CreateMutex(NULL, TRUE, "Ootake Emulator"); //f~f.[fefbfNfX,Ìì¬
	if (GetLastError() == ERROR_ALREADY_EXISTS) //,·,Å,ÉOotake,ğ<N"®,µ,Ä,¢,½,ç
		return 0; //<N"®,¹,¸,ÉI-¹

	//CoInitializeEx(NULL, COINIT_MULTITHREADED); //Kitao'Ç?ÁBv2.17XVBZQlFfAfp[fgff"fg(COINIT_APARTMENTTHREADED),¾,Æ?¹,ª­,µd,¢S´,¶,É,È,é(,¨,»,ç,­'¼,ÌMTA,æ,èSTA,Ì,Ù,¤,ª^-SÔSu,ª'·,¢)Bv2.19<L
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); //Kitao'Ç?ÁBv2.17XVBZQlFfAfp[fgff"fg(COINIT_APARTMENTTHREADED),¾,Æ?¹,ª­,µd,¢S´,¶,É,È,é(,¨,»,ç,­'¼,ÌMTA,æ,èSTA,Ì,Ù,¤,ª^-SÔSu,ª'·,¢)Bv2.19<L
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(tc.wPeriodMin); //Kitao'Ç?ÁBf^fCf}¸"x,ğ,±,±,Åã,°,Ä,¨,­,æ,¤,É,µ,½B

	if (!APP_Init(argc, argv))
		return -1;

	while (APP_ProcessEvents() != APP_QUIT);

	APP_Deinit();

	timeEndPeriod(tc.wPeriodMin); //Kitao'Ç?Á
	//CoUninitialize(); //Kitao'Ç?Á

	return 0;
}
