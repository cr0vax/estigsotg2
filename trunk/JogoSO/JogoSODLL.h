//Descritivo:Primeiro trabalho de grupo da disciplina de SO
//Titulo:DLL para o jogo de aventura
//Autores: Luís Costa Nº6032, Bruno Moreira Nº6170

#pragma once
//#include "stdafx.h"
#ifndef __AFXWIN_H__
	//#include "stdafx.h"
#endif

#include "resource.h"		// main symbols

__declspec( dllexport ) int WINAPI MultiplyByTwo( int number );

//left trim
__declspec( dllexport ) char * WINAPI ltrim(char *s);
//right trim			int
__declspec( dllexport ) char * WINAPI rtrim(char *s);
//trim					int
__declspec( dllexport ) char * WINAPI trim(char *s);
//converter ficheiro em binário
__declspec( dllexport ) void WINAPI converte_ficheiro();

__declspec( dllexport ) unsigned int WINAPI GetRandomNumber(int nLow, int nHigh);

// CJogoSODLLApp
// See JogoSODLL.cpp for the implementation of this class
//

class CJogoSODLLApp : public CWinApp
{
public:
	CJogoSODLLApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
