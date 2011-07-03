// JogoSOClienteConsola.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "locale.h"
#include <string.h>

//-------------------------------------------------------------------------------------------------
//	ESTRUTURAS
//-------------------------------------------------------------------------------------------------

//Jogador
//---------------------
struct Jogador
{
	char*	nome[50];			//Nome do jogador
	int		energia;			//Energia do jogador
	int		localizacao;		//Celula onde o jogador se encontra
	int		flg_tem_tesouro;	//Se o jogador tem ou não tesouro
};

struct Celula
{
	int		norte;				//Qual a celula a norte
	int		sul;				//Qual a celula a sul
	int		este;				//Qual a celula a este
	int		oeste;				//Qual a celula a oeste
	char*	descricao[400];		//Descrição da celula
	int		item;				//Item	que existe na celula
};

//Monstro
//---------------------
struct Monstro
{
	char*	nome[50];
	int		energia;			//Energia do jogador
	int		localizacao;		//Celula onde o jogador se encontra

};

#define PIPE_NAME "\\\\portatil\\pipe\\jogoso" //cliente e servidor na mesma máquina
#define BUF_SIZE 1024
#define MAX_MSG 1024
#define MAX_LOADSTRING 100
#define MAX_PARAMS_MSG 6

#define MAX_CELULAS 9			// número máximo de células do mapa
#define MAX_COL 400				// tamanho máximo da linha	

struct Jogador jogador;

HANDLE hPipe;
char msg[ MAX_MSG ];

//-------------------------------
//Imprime a mensagem de erro das comunicações do PIPE
//-------------------------------
void PrintErrorMsg()
{
	
	char errorMsg[80];

	if( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                       0, 
                       GetLastError(),
                       0,
                       errorMsg, 
                       80, 
                       NULL) )
	printf("Erro na comunicação do pipe: %s\n",errorMsg); 
}

int liga_pipe()
{
	printf("A connectar ao pipe...\n");
    
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


	if (hPipe == INVALID_HANDLE_VALUE)
	{ 
		return -1;
	}

	return 0;
}

int envia_mensagem(char msgEnvia [MAX_MSG])
{

	//Envio da mensagem ao servidor através do named pipe
	BOOL resSuccess = FALSE;
	DWORD cbWritten;

	resSuccess = WriteFile( 
		 hPipe,			 // handle to pipe 
		 msgEnvia,		 // buffer to write from 
		 MAX_MSG,		// number of bytes to write 
		 &cbWritten,	 // number of bytes written 
		 NULL);			// not overlapped I/O 

	if( resSuccess != TRUE )
	{
		PrintErrorMsg();
		return -1;
	} 
	else
	{
		return 0;
	}
}

void recebe_mensagem()
{
	printf("A espera de mensagem...\n");
	//Leitura do named pipe
	DWORD cbRead;

	//A função ReadFile só deve ser chamada depois de ambos os processos
	//se encontrarem ligados através do pipe
	BOOL resSuccess = ReadFile( 
			hPipe,		// pipe handle 
			msg,		// buffer to receive reply 
			BUF_SIZE,  // size of buffer 
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped

	// envia mensagem de retorno para o server
	envia_mensagem("OK");
}


// menu principal
void menu_principal()
{

	system("cls");

	// escreve a história do jogo
	printf("+-------------------------------------------------------------------------+\n");
	printf("| Estamos no ano de 2010 e o Sport Lisboa e Benfica está a sofrer uma das |\n");
	printf("| piores épocas de sempre. A sua única esperança é conseguir contratar o  |\n");
	printf("| maior jogador chinês da actualidade de modo a ganhar vantagem na luta   |\n");
	printf("| contra os seus mais directos adversários no campeonato. Para isso é     |\n");
	printf("| necessário resgatar o jogador que se encontra preso pelo seu agente num |\n");
	printf("| restaurante chinês. A tarefa não vai ser fácil…mas não é impossível!    |\n");
	printf("+-------------------------------------------------------------------------+\n");
	
	// adiciona uns espaços antes do menú
	printf("\n\n");

	// escreve o menu
	printf("+---------------------------------+\n");
	printf("| 1 - Novo Jogo                   |\n");
	printf("| 2 - Carregar Jogo               |\n");
	printf("| 3 - Converter mapa para binário |\n");
	printf("|                                 |\n");
	printf("| 0 - Sair                        |\n");
	printf("+---------------------------------+\n");
}

// Descreve jogador
void descreve_jogador(struct Jogador *pJogador)
{
	int nLinhaInicio = 0;				//linha da consola
	int nColunaInicio = 7;				//coluna da consola

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	/*Guarda os atributos currentes do texto para reposiçãoo no final*/
	CONSOLE_SCREEN_BUFFER_INFO strConsoleInfo;
	GetConsoleScreenBufferInfo( hStdout, &strConsoleInfo );

	//define a posição do cursor na consula e imprime naquela posição
	COORD pos1 = {nColunaInicio, nLinhaInicio};
	SetConsoleCursorPosition( hStdout, pos1 );
	printf("********************************");
	//-----------------------------
	
	COORD pos2 = {nColunaInicio + 8, nLinhaInicio + 1};
	SetConsoleCursorPosition( hStdout, pos2 );

	/*Texto e fundo com cores*/
	SetConsoleTextAttribute( hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY  | BACKGROUND_INTENSITY );

	printf("DADOS DO JOGADOR");
	//-----------------------------

	COORD pos3 = {nColunaInicio, nLinhaInicio + 2};
	SetConsoleCursorPosition( hStdout, pos3 );

	SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );

	printf("*------------------------------*");
	//-----------------------------

	COORD pos4 = {nColunaInicio, nLinhaInicio + 3};
	SetConsoleCursorPosition( hStdout, pos4 );
	
	printf("* ");

	SetConsoleTextAttribute( hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
	printf("Nome      : %s", pJogador->nome);
	//----------------------------

	COORD pos5 = {nColunaInicio, nLinhaInicio + 4};
	SetConsoleCursorPosition( hStdout, pos5 );
	SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );

	printf("* ");

	SetConsoleTextAttribute( hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
	printf("Energia   : %d", pJogador->energia);
	//----------------------------

	COORD pos6 = {nColunaInicio, nLinhaInicio + 5};
	SetConsoleCursorPosition( hStdout, pos6 );

	SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );

	printf("*");
	//----------------------------

	COORD pos7 = {nColunaInicio, nLinhaInicio + 6};
	SetConsoleCursorPosition( hStdout, pos7 );
	printf("*------------------------------*");

	COORD pos8 = {nColunaInicio, nLinhaInicio + 7};
	SetConsoleCursorPosition( hStdout, pos8 );
	printf("********************************");

	/*Repoe os atributos iniciais*/
	SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );
}

// Descreve monstro
void descreve_monstro(struct Monstro *pMonstro, bool blnSuperUser)
{
	int nLinhaInicio = 0;				//linha da consola
	int nColunaInicio = 38;				//coluna da consola

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	/*Guarda os atributos currentes do texto para reposiçãoo no final*/
	CONSOLE_SCREEN_BUFFER_INFO strConsoleInfo;
	GetConsoleScreenBufferInfo( hStdout, &strConsoleInfo );

	//define a posição do cursor na consula e imprime naquela posição
	COORD pos1 = {nColunaInicio, nLinhaInicio};
	SetConsoleCursorPosition( hStdout, pos1 );
	printf("********************************");
	//---------------------------------

	COORD pos2 = {nColunaInicio + 4, nLinhaInicio + 1};
	SetConsoleCursorPosition( hStdout, pos2 );

	SetConsoleTextAttribute( hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY  | BACKGROUND_INTENSITY );

	printf("DESCRIÇÃO DOS MONSTROS");
	//---------------------------------

	SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );

	COORD pos3 = {nColunaInicio, nLinhaInicio + 2};
	SetConsoleCursorPosition( hStdout, pos3 );
	printf("*------------------------------*");
	//---------------------------------

	COORD pos4 = {nColunaInicio, nLinhaInicio + 3};
	SetConsoleCursorPosition( hStdout, pos4 );

	printf("* ");

	SetConsoleTextAttribute( hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
	printf("Nome       : %s", pMonstro->nome);
	//----------------------------------

	COORD pos5 = {nColunaInicio, nLinhaInicio + 4};
	SetConsoleCursorPosition( hStdout, pos5 );

	SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );
	printf("* ");

	SetConsoleTextAttribute( hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);

	// se o monstro estiver morto informa que morreu
	if (pMonstro->energia <= 0)
	{
		printf("Energia    : Morto");
	}
	else
	{
		printf("Energia    : %d", pMonstro->energia);
	}

	// se o modo super user estiver activado mostra a sala do monstro
	if ( blnSuperUser == true )
	{
		// escreve a designação da sala se o monstro estiver vivo
		if (pMonstro->energia >= 0)
		{
			COORD pos6 = {nColunaInicio, nLinhaInicio + 5};
			SetConsoleCursorPosition( hStdout, pos6 );

			SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );
			printf("* ");

			SetConsoleTextAttribute( hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
			printf("Localização: Sala %d", pMonstro->localizacao);
		}
	}
	/*else
	{
		SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );
		COORD pos6 = {nColunaInicio, nLinhaInicio + 5};
		SetConsoleCursorPosition( hStdout, pos6 );
		printf("*", pMapa[pMonstro->localizacao].descricao);
	}
*/
	SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );

	COORD pos7 = {nColunaInicio, nLinhaInicio + 6};
	SetConsoleCursorPosition( hStdout, pos7 );
	printf("*------------------------------*");
	//-----------------------------------------

	COORD pos8 = {nColunaInicio, nLinhaInicio + 7};
	SetConsoleCursorPosition( hStdout, pos8 );
	printf("********************************\n");
}

// Desenha no ecrã o que ocorre no jogo
void descreve_status(struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula *pCelula, bool blnSuperUser)
{
	int MAX_LARGURA = 73;
	system("cls");							// limpa ecrã

	descreve_jogador(pJogador);

	printf("\n");
	printf("+---------------------------------------------------------------------------+\n");
	printf("|                       DESCRIÇÃO DA LOCALIZAÇÃO                            |\n");
	printf("|                                                                           |\n");
	printf("| ");
	
	// imprime a descrição em conjuntos de 73 caracteres

	int i = 0;
	char sLocalizacao[ MAX_COL ];

	strcpy((char*) sLocalizacao, (char*) pCelula->descricao);

	while( sLocalizacao[i] != '\n')
	{
		printf("%c", sLocalizacao[i]);

		i++;

		if ( (i % MAX_LARGURA) == 0)
		{
			printf(" |\n| ");
		}
	}

	int x = 0;
	//printf("i:%d, largura:%d, conta:%d", i, MAX_LARGURA, (i % MAX_LARGURA));

	for (x; x <= (MAX_LARGURA - ((i) % MAX_LARGURA)); x++)
	{
		printf(" ");
	}

	printf("|\n");

	printf("+---------------------------------------------------------------------------+\n");

	// se o modo superUser estiver activado mostra o status do monstro
	if (blnSuperUser == true)
	{
		descreve_monstro(pMonstro, blnSuperUser);
	}

	printf("\n\n\n\n\n\n");
	
}

// inicializa o monstro
void inicializa_monstro(struct Monstro *pMonstro, char* strArr[MAX_PARAMS_MSG])
{
	// associa os dados passados para o monstro
	strcpy((char*) pMonstro->nome, strArr[1]);
	pMonstro->energia		= atoi(strArr[2]);
	pMonstro->localizacao	= atoi(strArr[3]);
}


//Inicializa o jogador
void inicializa_jogador(struct Jogador *pJogador, char* strArr[MAX_PARAMS_MSG])
{
	// associa os passados ao jogador
	strcpy((char*) pJogador->nome,  strArr[1]);
	pJogador->localizacao		= atoi(strArr[2]);
	pJogador->flg_tem_tesouro	= atoi(strArr[3]);
	
	//TODO: if (blnSuperUser == true) {
	//	pJogador->energia			= 9999;			// energia do superuser
	//}
	//else {
		pJogador->energia			= 100;			// energia do utilizador normal
	//}
}

// inicializa a celula
void inicializa_celula(struct Celula *pCelula, char* strArr[MAX_PARAMS_MSG])
{
	// associa os dados passados à celula
	strcpy((char*) pCelula->descricao, strArr[1]);
	pCelula->norte			= atoi(strArr[2]);
	pCelula->sul			= atoi(strArr[3]);
	pCelula->este			= atoi(strArr[4]);
	pCelula->oeste			= atoi(strArr[5]);
}

// valida os comandos disponíveis para o jogador
char* valida_comandos_disponiveis(struct Celula *pCelula)
{
	int MAX_CHARS = 200;
	char* sComandosDisponiveis;
	sComandosDisponiveis = (char *) malloc( MAX_CHARS );
	strcpy((char*) sComandosDisponiveis, "+------------------------------\n");

	// valida Norte
		if (pCelula->norte >= 0){	strcat((char*) sComandosDisponiveis, "| N - Norte\n");}
	// valida Sul
		if (pCelula->sul >= 0)  {	strcat((char*) sComandosDisponiveis, "| S - Sul\n");  }
	// valida Este
		if (pCelula->este >= 0) {	strcat((char*) sComandosDisponiveis, "| E - Este\n"); }
	// valida Oeste
		if (pCelula->oeste >= 0){	strcat((char*) sComandosDisponiveis, "| O - Oeste\n");}

	// Opções funcionais
	// adiciona return
		strcat((char*) sComandosDisponiveis, "|\n");
	// Gravar Jogo
		strcat((char*) sComandosDisponiveis, "| G - Gravar Jogo\n");
	// sai do Jogo
		strcat((char*) sComandosDisponiveis, "| 0 - Sair do Jogo\n");

	return (char*) sComandosDisponiveis;
}

// Imprimir status no ecra
//---------------------------
void imprimir_status (int iStatus, struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula *pCelula)
{
	char* sComandosDisponiveis;

	switch (iStatus)
	{
		case 0: //status normal
				sComandosDisponiveis = valida_comandos_disponiveis(pCelula);
				descreve_status(pJogador,pMonstro,pCelula,TRUE);
				printf("%s\n", sComandosDisponiveis);
				break;

		case 1: //status luta
				// descreve status
				system("cls");

				descreve_jogador(pJogador);
				descreve_monstro(pMonstro, pCelula);

				printf("+------------------------------\n");
				printf("| COMBATE\n");
				printf("+------------------------------\n");

			// imprime legenda
				printf("     @  - 1\n");
				printf("3 - /|\\ - 4\n");
				printf("    / \\ - 2\n");
				break;
	}
}

//Verifica que tipo de mensagem foi enviada segundo o protocolo
//------------------------------------
int trata_mensagem_luta (struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula *pCelula) 
{
	//vai dividir a mensagem enviada segundo o protocolo
	char strSeparet[2];
	char strTOsplit[MAX_MSG];
	int nArr = MAX_PARAMS_MSG;
	char* strArr[MAX_PARAMS_MSG];

	strcpy((char*) strSeparet, "|");
	strcpy((char*) strTOsplit, msg);

	int i = 0;
	int nParams = 0;

	char * pch;

	pch = strtok (strTOsplit, strSeparet);
	for(i = 0;i < nArr;i++)
	{
		printf ("%s\n",pch);

		strArr[i] = pch;
		pch = strtok (NULL,strSeparet);

		if (pch != NULL)
		{
			nParams ++;
		}
	}

	//Inicia o tratamento das partes da mensagem
	//--------------------
	if (strcmp(strArr[0], "»M") == 0)
	{
		//Carrega a estrutura do monstro
		inicializa_monstro(pMonstro, strArr);
		return 0;
	}

	else if (strcmp(strArr[0], "»J") == 0)
	{
		//Carrega a estrutura do jogador
		inicializa_jogador(pJogador, strArr);
		return 0;
	}

	else if (strcmp(strArr[0], "»C") == 0)
	{
		//Carrega a estrutura da celula
		inicializa_celula(pCelula, strArr);
		return 0;
	}

	else if (strcmp(strArr[0], "»S") == 0)
	{
		//Imprime status luta
		imprimir_status(atoi(strArr[1]),pJogador, pMonstro, pCelula);
		return 0;
	}

	else
	{
		printf("%s", strArr[0]);
	}
}

//Faz o tratamento da luta entre o monstro e o jogador
void luta(char* strArr[MAX_PARAMS_MSG], struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula *pCelula)
{	
	int fimLuta = 0;

	while(fimLuta != 0)
	{
		recebe_mensagem();
		fimLuta = trata_mensagem_luta(pJogador, pMonstro, pCelula);
	}
}

// solicita comando ao jogador
//----------------------------------
void pede_comando (char* strArr[MAX_PARAMS_MSG])
{
	char * iOpcao [MAX_MSG];
	strcpy((char*) iOpcao, "");
	
	printf("%s", strArr[1]);
	scanf( "%s", iOpcao );
	envia_mensagem((char*)iOpcao);
}

//Verifica que tipo de mensagem foi enviada segundo o protocolo
//------------------------------------
int trata_mensagem (struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula *pCelula) 
{
	//vai dividir a mensagem enviada segundo o protocolo
	char strSeparet[2];
	char strTOsplit[MAX_MSG];
	int nArr = MAX_PARAMS_MSG;
	char* strArr[MAX_PARAMS_MSG];

	strcpy((char*) strSeparet, "|");
	strcpy((char*) strTOsplit, msg);

	int i = 0;
	int nParams = 0;

	char * pch;

	pch = strtok (strTOsplit, strSeparet);
	for(i = 0;i < nArr;i++)
	{
		//printf ("%s\n",pch);

		strArr[i] = pch;
		pch = strtok (NULL,strSeparet);

		if (pch != NULL)
		{
			nParams ++;
		}
	}

	//Inicia o tratamento das partes da mensagem
	//--------------------
	if (strcmp(strArr[0], "»M") == 0)
	{
		//Carrega a estrutura do monstro
		inicializa_monstro(pMonstro, strArr);
		return 0;
	}

	else if (strcmp(strArr[0], "»J") == 0)
	{
		//Carrega a estrutura do jogador
		inicializa_jogador(pJogador, strArr);
		return 0;
	}

	else if (strcmp(strArr[0], "»C") == 0)
	{
		//Carrega a estrutura da celula
		inicializa_celula(pCelula, strArr);
		return 0;
	}

	//else if (strcmp(strArr[0], "»L") == 0)
	//{
	//	// Posicao 1 -> 0 = inicio da luta
	//	//				1 = termina luta
	//	// Posicao 2 -> 0 = vivo
	//	//			    1 = morto

	//	//faz o tratamento da luta
	//	if (strcmp(strArr[1], "0") == 0)
	//	{
	//		luta(strArr, pJogador, pMonstro, pCelula);
	//		return 0;
	//	}
	//}

	else if (strcmp(strArr[0], "»I") == 0)
	{
		//Pede um comando ao utilizador
		pede_comando(strArr);
		return 0;
	}

	else if (strcmp(strArr[0], "»S") == 0)
	{
		//Pede um comando ao utilizador
		imprimir_status(atoi(strArr[1]), pJogador, pMonstro, pCelula);
		return 0;
	}

		else if (strcmp(strArr[0], "»F") == 0)
	{
		//Verifica se ganhou o jogo
		if (atoi(strArr[1]) == 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	else
	{
		printf("%s",strArr[0]);
		return 0;
	}
}

int main(int argc, _TCHAR* argv[])
{
	int iResultado;

	//Estruturas
	struct Jogador jogador;
	struct Monstro monstro;
	struct Celula celula;

	char * iOpcao [MAX_MSG];
	int fimJogo = 0;

	strcpy((char*) iOpcao, "");
	strcpy(msg, "");

	setlocale( LC_ALL, "Portuguese" );

	iResultado = liga_pipe();
	
	//Valida se a ligação ao pipe foi efectuada
	//----------------------------
	if ( iResultado < 0 )
	{
		printf("Erro na ligação do pipe!\n");
	}

	else
	{
		printf("Ligação ao pipe efectuada!\n");

		menu_principal();

		// solicita a opção ao jogador
		printf("Escolha uma opção:");
		scanf( "%s", iOpcao );
		envia_mensagem((char*)iOpcao);

		// recebe a mensagem
		recebe_mensagem();
		system("CLS");

		printf(msg);

		// envia mensagem
		scanf( "%s", iOpcao );
		envia_mensagem((char*)iOpcao);

		while (fimJogo == 0)
		{
			// recebe a mensagem para saber se vai lutar
			recebe_mensagem();
			fimJogo = trata_mensagem(&jogador, &monstro, &celula);
		}
	}

	getchar();
	system("PAUSE");

	return 0;
	
}


