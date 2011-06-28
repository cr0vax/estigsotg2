// JogoSOClient.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "JogoSOClient.h"
#include <windows.h>
#include <stdio.h>
#include "locale.h"
#include <string.h>

#define PIPE_NAME "\\\\.\\pipe\\jogoso" //cliente e servidor na mesma máquina
#define BUF_SIZE 512
#define MAX_MSG 512
#define MAX_LOADSTRING 100

HANDLE hPipe;
bool   ligado; 
HDC hdc;
static TCHAR ch = TCHAR ( ' ' );

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	setlocale( LC_ALL, "Portuguese" );

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_JOGOSOCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JOGOSOCLIENT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JOGOSOCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_JOGOSOCLIENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}



int liga_pipe()
{
	
	//TextOut( hdc, 0, 100, _T( PIPE_NAME ), 100 ); //A escrita no ecrã é realizada aqui
    
	//Ligação ao named pipe criado pelo servidor
	hPipe = CreateFile( 
         PIPE_NAME,   		// pipe name 
         GENERIC_READ |  	// read and write access 
         GENERIC_WRITE, 
         0,              	// no sharing 
         NULL,           	// default security attributes
         OPEN_EXISTING,  	// opens existing pipe 
         0,              	// default attributes 
         NULL);          	// no template file


	if (hPipe == INVALID_HANDLE_VALUE){ 
		return -1;
	  }

	ligado = true;

	return 0;
}

int envia_mensagem(TCHAR msg[1])
{
	
	//Envio da mensagem ao servidor através do named pipe
	BOOL resSuccess = FALSE;
	DWORD cbWritten;
	resSuccess = WriteFile( 
		 hPipe,        // handle to pipe 
		 msg,     // buffer to write from 
		 MAX_MSG, // number of bytes to write 
		 &cbWritten,   // number of bytes written 
		 NULL);        // not overlapped I/O 

	if( ! resSuccess )
	{
		return -1;
	} 
	else
	{
		return 0;
	}
}

void recebe_mensagem()
{
	
	//Leitura do named pipe
	//TCHAR msg[ MAX_MSG ];
	DWORD cbRead;

	//A função ReadFile só deve ser chamada depois de ambos os processos
	//se encontrarem ligados através do pipe
	BOOL resSuccess = ReadFile( 
			hPipe,    // pipe handle 
			&ch,    // buffer to receive reply 
			BUF_SIZE,  // size of buffer 
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped 

	//return (WPARAM) msg;
}

// menu principal
void menu_principal(HWND hWnd, HDC hdc)
{

	//InvalidateRect(hWnd,0,false);
	// escreve a história do jogo
	TextOut( hdc, 0, 20, _T( "Estamos no ano de 2010 e o Sport Lisboa e Benfica está a sofrer uma das" ), 80 );
	TextOut( hdc, 0, 40, _T( "piores épocas de sempre. A sua única esperança é conseguir contratar o" ), 80 );
	TextOut( hdc, 0, 60, _T( "maior jogador chinês da actualidade de modo a ganhar vantagem na luta" ), 80 );
	TextOut( hdc, 0, 80, _T( "contra os seus mais directos adversários no campeonato. Para isso é" ), 80 );
	TextOut( hdc, 0, 100, _T( "necessário resgatar o jogador que se encontra preso pelo seu agente num" ), 80 );
	TextOut( hdc, 0, 120, _T( "restaurante chinês. A tarefa não vai ser fácil…mas não é impossível!" ), 80 );
	TextOut( hdc, 0, 140, _T( "1 - Novo Jogo" ), 20 );
	TextOut( hdc, 0, 160, _T( "2 - Carregar Jogo" ), 20 );
	TextOut( hdc, 0, 180, _T( "3 - Converter mapa para binário" ), 32 );
	TextOut( hdc, 0, 200, _T( "0 - Sair" ), 10 );
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	int iResultado;

	switch (message)
	{
	case WM_CREATE:
		iResultado = liga_pipe();

		if ( iResultado < 0 )
		{
			//TODO: dá mensagem de erro no ecrã
		}

		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CHAR:

		// variável para guardar o comando inserido pelo utilizador
		ch = (TCHAR) wParam;

		// enviar mensagem para o servidor
		iResultado = envia_mensagem(&ch);

		if ( iResultado < 0 )
		{
			//TODO: dá mensagem de erro no ecrã
		}
		else
		{
			// recebe mensagem do servidor
			recebe_mensagem();
			InvalidateRgn ( hWnd, NULL, TRUE );
		}

		break;
	case WM_PAINT:
		
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...

		//Escreve no ecrã
		

		if ( ligado )
		{
			menu_principal(hWnd, hdc);

			TextOut( hdc, 0, 0, &ch, 1 ); //A escrita no ecrã é realizada aqui
		}
		
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
