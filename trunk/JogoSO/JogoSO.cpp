//Descritivo:Primeiro trabalho de grupo da disciplina de SO
//Titulo:DLL para o jogo de aventura
//Autores: Luís Costa Nº6032, Bruno Moreira Nº6170


#include "stdafx.h"
#include "string.h"
#include <ctime>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include "windows.h"
#include "stdafx.h"
#include "JogoSO.h"

//include da DLL
#include "JogoSODLL.h"

#define LAST_LINE 24

#define MAX_CELULAS 9			// número máximo de células do mapa
#define MAX_COL 400				// tamanho máximo da linha

#define MAX_THREADS 20
#define MAX_FILE_NAME 100
#define MAX_MSG 512

HANDLE hMutexJogo;					// mutex para controlar o jogo
HANDLE hPipe;						// handle do pipe

#define PIPE_NAME "\\\\.\\pipe\\jogoso" //só é possível criar um named pipe no próprio computador
#define BUF_SIZE 1024
#define MAX_MSG 1024

char msgRecebida[ MAX_MSG ];				// mensagem recebida no buffer
char msgEnviada[ MAX_MSG ];				// mensagem recebida no buffer

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

//Estrutura onde são armazenados os dados de cada thread
struct ThreadData{
	int id;
	char fileName[ MAX_FILE_NAME ];
};

struct argThreadMonstro{
	struct Celula *pMapa;
	struct Monstro *pMonstro;
};

struct argThreadJogador{
	struct Celula *pMapa;
	struct Jogador *pJogador;
	struct Monstro *pMonstro;
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//-------------------------------------------------------------------------------------------------
//	FUNÇÕES
//-------------------------------------------------------------------------------------------------

void PrintErrorMsg(){
	
	char errorMsg[80];

	if( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                       0, 
                       GetLastError(),
                       0,
                       errorMsg, 
                       80, 
                       NULL) )
		printf( "%s\n", errorMsg );
	system("pause");
}

int recebe_mensagem()
{
	//printf("Entrou no recebe mensagem\n");

	printf("Servidor>Espera mensagem ");
	//Leitura do named pipe
	DWORD cbRead;

	//A função ReadFile só deve ser chamada depois de ambos os processos
	//se encontrarem ligados através do pipe
	BOOL resSuccess = ReadFile( 
			hPipe,     // pipe handle 
			msgRecebida,       // buffer to receive reply 
			BUF_SIZE,  // size of buffer 
			&cbRead,   // number of bytes read 
			NULL);     // not overlapped 

	if( resSuccess != TRUE ){
		printf( "Servidor: Erro na leitura do named pipe.\n" );
		PrintErrorMsg();
		return -1;
	}

	printf("Recebi %s\n", msgRecebida);

	return 0;
}

int envia_mensagem(char msg[ MAX_MSG ])
{

	//WaitForSingleObject( hMutexJogo,INFINITE );

	printf("Servidor>Envia mensagem:%s\n", msg);
	DWORD cbWritten;
	BOOL resSuccess = WriteFile( 
		  hPipe,        // handle to pipe 
		  msg,     // buffer to write from 
		  MAX_MSG, // number of bytes to write 
		  &cbWritten,   // number of bytes written 
		  NULL);        // not overlapped I/O 

	 //Libertação do handle do named pipe
	 //DisconnectNamedPipe( hPipe );

	if( ! resSuccess ){ 
		printf( "Servidor: Erro no envio.\n" );
		PrintErrorMsg();
		return -1;
	}

	recebe_mensagem();

	return 0;

	//ReleaseMutex(hMutexJogo);
}

//Inicializa o jogador
void inicializa_jogador(struct Jogador *pJogador, char* nome, bool blnSuperUser)
{
	// associa os dados por defeito ao jogador
	strcpy((char*) pJogador->nome, nome);
	pJogador->localizacao		= 0;
	pJogador->flg_tem_tesouro	= -1;
	
	if (blnSuperUser == true) {
		pJogador->energia			= 9999;			// energia do superuser
	}
	else {
		pJogador->energia			= 100;			// energia do utilizador normal
	}
}

// inicilializa mapa com base num ficheiro
void inicializa_mapa_ficheiro(struct Celula pMapa[], char* pFicheiroMapa)
{
	printf("A inicializar o mapa do ficheiro...\n");
	#define MAX_LIN 80
	#define CAMPOS 7

	FILE *f;
	char l[ MAX_LIN ];

	f = fopen( pFicheiroMapa, "r" );
	
	// inicia a linha
	int iLinha = 0;

	// inicia o contador
	int iIndice = -1;

	// percorre o ficheiro até encontrar o fim
	while( fgets(l, MAX_LIN, f) != NULL ){

		// se o resto da divisão do número da linha pelo número de linhas por célula
		// for igual a 0, quer dizer que é um novo registo
		if (iLinha % CAMPOS == 0)
		{
			// incrementa o indice
			iIndice = iIndice + 1;
		}

		// imprime a linha
		int iResto = iLinha % CAMPOS;

		switch ( iResto )
		{
			case 0:	// Descrição
				strcpy((char*)pMapa[iIndice].descricao, rtrim(l));
				break;
			case 1:	// Norte
				pMapa[iIndice].norte = atoi (l);
				break;
			case 2:	// Sul
				pMapa[iIndice].sul = atoi (l);
				break;
			case 3:	// Este
				pMapa[iIndice].este = atoi (l);
				break;
			case 4:	// Oeste
				pMapa[iIndice].oeste = atoi (l);
				break;
			case 5:	// Item
				pMapa[iIndice].item = atoi (l);
				break;
		}

		// incrementa o número da linha
		iLinha = iLinha + 1;
	}
	
	fclose( f );

	printf("Concluiu a inicialização do mapa com base num ficheiro\n");
}

// Inicilializa mapa com base num ficheiro binário
void inicializa_mapa_ficheiro_bin(struct Celula pMapa[], char* pFicheiroMapa)
{
	printf("A inicializar o mapa de um ficheiro binário\n");

	#define MAX_LIN 400
	#define CAMPOS 7

	FILE *f;
	char l[ MAX_LIN ];

	f = fopen( pFicheiroMapa, "rb" );
	
	// inicia a linha
	int iLinha = 0;

	// inicia o contador
	int iIndice = -1;

	if (f != NULL)
	{
		// percorre o ficheiro até encontrar o fim
		while( fread(l,sizeof(char), MAX_LIN, f) != NULL ){

			// se o resto da divisão do número da linha pelo número de linhas por célula
			// for igual a 0, quer dizer que é um novo registo
			if (iLinha % CAMPOS == 0)
			{
				// incrementa o indice
				iIndice = iIndice + 1;
			}

			// imprime a linha
			int iResto = iLinha % CAMPOS;

			if (iResto > 0) {int a = atoi(l);}

			switch ( iResto )
			{
				case 0:	// Descrição
					strcpy((char*)pMapa[iIndice].descricao, l);
					break;
				case 1:	// Norte
					pMapa[iIndice].norte = atoi (l);
					break;
				case 2:	// Sul
					pMapa[iIndice].sul = atoi (l);
					break;
				case 3:	// Este
					pMapa[iIndice].este = atoi (l);
					break;
				case 4:	// Oeste
					pMapa[iIndice].oeste = atoi (l);
					break;
				case 5:	// Item
					pMapa[iIndice].item = atoi (l);
					break;
			}

			// incrementa o número da linha
			iLinha = iLinha + 1;
		}
	
		fclose( f );

		printf("Concluiu a inicialização do mapa com base num ficheiro binário\n");
	}
	else
	{
		printf("Erro a abrir o ficheiro %s",pFicheiroMapa);
		system("pause");
	}
}


//Inicializa o mapa da aventura
//--------------------------------
//    +------+------+------+
//    |             |      |
//    |0     |1     |2     |
//    +------+--  --+--  --+
//    |                    |
//    |3     |4     |5    T|
//    +--  --+------+------+
//    |                    |
//    |6     |7     |8     |
//    +------+------+------+
void inicializa_mapa(struct Celula pMapa[], char* pFicheiroMapa)
{
	printf("A inicializar o mapa...\n");
	// se o tamanho da string pFicheiroMapa for superior a 0 então é
	// para carregar o mapa do ficheiro
	if (strlen(pFicheiroMapa) > 1)
	{
		// carrega mapa do ficheiro
		inicializa_mapa_ficheiro_bin(pMapa, pFicheiroMapa);
	}
	else
	{
		//Construção da sala 0
		//----------------------
		strcpy((char*) pMapa[0].descricao, "Encontraste na entrada do restaurante…O ambiente é muito sombrio avistando-se apenas uma pequena luz a ESTE.\n");
		pMapa[0].norte	= -1;
		pMapa[0].sul	= -1;
		pMapa[0].este	= 1;
		pMapa[0].oeste	= -1;
		pMapa[0].item	= -1;

		//Construção da sala 1
		//----------------------
		strcpy((char*) pMapa[1].descricao, "Entraste numa sala que parece ser a sala de espera dos clientes. Para além de algumas cadeiras e uma televisão existem apenas alguns quadros na parede.\n");
		pMapa[1].norte	= -1;
		pMapa[1].sul	= 4;
		pMapa[1].este	= -1;
		pMapa[1].oeste	= 0;
		pMapa[1].item	= -1;

		//Construção da sala 2
		//----------------------
		strcpy((char*) pMapa[2].descricao, "Abriste a porta com dificuldade e entraste no que parece ser o armazém do restaurante. Nas prateleiras encontram-se inúmeros frascos com lagartos, insectos e uma arca frigorífica onde consegues ver alguns gatos…\n");
		pMapa[2].norte	= -1;
		pMapa[2].sul	= 5;
		pMapa[2].este	= -1;
		pMapa[2].oeste	= -1;
		pMapa[2].item	= -1;

		//Construção da sala 3
		//----------------------
		strcpy((char*) pMapa[3].descricao, "Encontras-te numa das salas de jantar do restaurante. As mesas estão todas postas e prontas para a hora de jantar. Olhas ao fundo e vez o que parecem ser umas escadas para o 2º andar.\n");
		pMapa[3].norte	= -1;
		pMapa[3].sul	= 6;
		pMapa[3].este	= 4;
		pMapa[3].oeste	= -1;
		pMapa[3].item	= -1;

		//Construção da sala 4
		//----------------------
		strcpy((char*) pMapa[4].descricao, "Entras no corredor do restaurante…Sentes o ambiente algo pesado muito por culpa de uma música tradicional chinesa que ouves vinda da sala a ESTE.\n");
		pMapa[4].norte	= 1;
		pMapa[4].sul	= -1;
		pMapa[4].este	= 5;
		pMapa[4].oeste	= 3;
		pMapa[4].item	= -1;

		//Construção da sala 5
		//----------------------
		strcpy((char*) pMapa[5].descricao, "Ao entrares da sala deparaste com uma cena macabra e arrepiante…Encontras o grande jogador chinês a ser torturado de uma forma desumana, sendo obrigado a ouvir a entrevista do Paulo Futre. De imediato o desamarras agradecendo-te este por lhe teres salvo a vida.\n");
		pMapa[5].norte	= 2;
		pMapa[5].sul	= -1;
		pMapa[5].este	= -1;
		pMapa[5].oeste	= 4;
		pMapa[5].item	= 0;


		//Construção da sala 6
		//----------------------
		strcpy((char*) pMapa[6].descricao, "Estás no segundo andar do restaurante. Á tua frente tens umas escadas para o 1º andar. Olhas em teu redor e vez apenas alguns vasos chineses.\n");
		pMapa[6].norte	= 3;
		pMapa[6].sul	= -1;
		pMapa[6].este	= 7;
		pMapa[6].oeste	= -1;
		pMapa[6].item	= -1;

		//Construção da sala 7
		//----------------------
		strcpy((char*) pMapa[7].descricao, "Abres uma porta de correr e entras num pequeno quarto. Á tua direita vez algumas camas por fazer e muita roupa espalhada no chão.\n");
		pMapa[7].norte	= -1;
		pMapa[7].sul	= -1;
		pMapa[7].este	= 8;
		pMapa[7].oeste	= 6;
		pMapa[7].item	= -1;

		//Construção da sala 8
		//----------------------
		strcpy((char*) pMapa[8].descricao, "Aproximaste da porta e sentes um cheiro muito intenso a plástico estilo loja dos chineses! Abres a porta e encontras a fonte do forte cheiro…uma enorme quantidade de caixas com letras chinesas no exterior. Algumas delas então abertas evidenciando o seu conteúdo, camisolas da ''adidaas''\n");
		pMapa[8].norte	= -1;
		pMapa[8].sul	= -1;
		pMapa[8].este	= -1;
		pMapa[8].oeste	= 7;
		pMapa[8].item	= -1;
	}
}

// testes da inicialização do mapa
void inicializa_mapa_teste (struct Celula pMapa[])
{
	printf ("inicializa_mapa_teste\n");
	for (int i = 0; i < MAX_CELULAS; i++)
	{
		printf("[%s]\n", pMapa[i].descricao);
		printf("Norte: %d\n", pMapa[i].norte);
		printf("Sul:   %d\n", pMapa[i].sul);
		printf("Este:  %d\n", pMapa[i].este);
		printf("Oeste: %d\n", pMapa[i].oeste);
		printf("Item:  %d\n\n", pMapa[i].item);
	}
	system("pause");
}

// testes da inicialização do jogador
void inicializa_jogador_teste ( struct Jogador *pJogador )
{
	printf ("inicializa_jogador_teste\n");
	printf("Nome: %s\n", pJogador->nome);
	printf("Energia:   %d\n", pJogador->energia);
	printf("Localização:  %d\n", pJogador->localizacao);
	printf("Tesouro: %d\n", pJogador->flg_tem_tesouro);
	system("pause");
}

// testes da inicialização do monstro
void inicializa_monstro_teste ( struct Monstro *pMonstro )
{
	printf ("inicializa_monstro_teste\n");
	printf("Nome: %s\n", pMonstro->nome);
	printf("Energia:   %d\n", pMonstro->energia);
	printf("Localização:  %d\n", pMonstro->localizacao);
	system("pause");
}

// inicializa o monstro
void inicializa_monstro(struct Monstro *pMonstro)
{
	// associa os dados por defeito ao monstro
	strcpy((char*) pMonstro->nome, "Paulo Futre");
	pMonstro->energia		= 50;
	pMonstro->localizacao	= 4;
}

// Aceita comando do jogador
int aceita_comando_jogador(char *sComando, struct Jogador *pJogador, struct Celula pMapa[])
{
	// valida a localização do jogador
	int iLocalizacaoJogador = pJogador->localizacao;
	int iAccao = -1;

	// validar comandos disponíveis
	sComando = strupr(sComando);

	if ( strcmp(sComando, "N") == 0 ) {
		iAccao = pMapa[iLocalizacaoJogador].norte;
	}
	
	if ( strcmp(sComando, "S") == 0 ) {
		iAccao = pMapa[iLocalizacaoJogador].sul;
	}
	
	if ( strcmp(sComando, "E") == 0 ) {
		iAccao = pMapa[iLocalizacaoJogador].este;
	}
	
	if ( strcmp(sComando, "O") == 0 ) {
		iAccao = pMapa[iLocalizacaoJogador].oeste;
	}

	if ( strcmp(sComando, "G") == 0 ) {
		iAccao = 100;
	}

	if ( strcmp(sComando, "0") == 0 ) {
		iAccao = 101;
	}

	if ( strcmp(sComando, "OK") == 0 ) {
		iAccao = 200;
	}


	// retorna o movimento que foi feito
	return iAccao;
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
void descreve_monstro(struct Monstro *pMonstro, struct Celula pMapa[], bool blnSuperUser)
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
			printf("Localização: %d", pMonstro->localizacao);
		}
	}
	else
	{
		SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );
		COORD pos6 = {nColunaInicio, nLinhaInicio + 5};
		SetConsoleCursorPosition( hStdout, pos6 );
		printf("*", pMapa[pMonstro->localizacao].descricao);
	}

	SetConsoleTextAttribute( hStdout, strConsoleInfo.wAttributes );

	COORD pos7 = {nColunaInicio, nLinhaInicio + 6};
	SetConsoleCursorPosition( hStdout, pos7 );
	printf("*------------------------------*");
	//-----------------------------------------

	COORD pos8 = {nColunaInicio, nLinhaInicio + 7};
	SetConsoleCursorPosition( hStdout, pos8 );
	printf("********************************\n");

	// repõe o cursor na posição de inserção de comando
	COORD pos9 = {19, LAST_LINE};
	SetConsoleCursorPosition( hStdout, pos9 );
}

// Desenha no ecrã o que ocorre no jogo
void descreve_status(struct Jogador *pJogador, struct Celula pMapa[], bool blnSuperUser)
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

	strcpy((char*) sLocalizacao, (char*) pMapa[pJogador->localizacao].descricao);

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

	for (x; x <= (MAX_LARGURA - ((i) % MAX_LARGURA)); x++)
	{
		printf(" ");
	}

	printf("|\n");

	printf("+---------------------------------------------------------------------------+\n");

	// se o modo superUser estiver activado mostra o status do monstro

	/* TODO
	if (blnSuperUser == true)
	{
		descreve_monstro(pMonstro, pMapa, blnSuperUser);
	}

	printf("\n\n\n\n\n\n");
	*/
}

// Simula a luta entre jogador e monstro
int lutar(struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula pMapa[], bool blnSuperUser)
{
	int MAX_DANO = 30;
	int iReturn = 1;
	int iHandyCap = 0;				// desvantagem do monstro

	// valida se o modo é superUser
	if (blnSuperUser == true)
	{
		iHandyCap = 10;
	}

	/*
	// descreve status
		system("cls");

		descreve_jogador(pJogador);
		descreve_monstro(pMonstro, pMapa, blnSuperUser);

		printf("+------------------------------\n");
		printf("| COMBATE\n");
		printf("+------------------------------\n");

	// imprime legenda
		printf("     @  - 1\n");
		printf("3 - /|\\ - 4\n");
		printf("    / \\ - 2\n");
	*/	
	// solicita a posição para atacar (cima, baixo, esquerda, direita, foge)
		int iJogadorAtaque;
		//printf("Qual a posição em que quer atacar?:");
		//scanf( "%d", &iJogadorAtaque );

		//	Aceitar Comando do Jogador
		//"»C|descricao|norte|sul|este|oeste"
		sprintf(msgEnviada, "»I|%s", "Qual a posição em que quer atacar?:");
		envia_mensagem(msgEnviada);
		recebe_mensagem();
		iJogadorAtaque = atoi(msgRecebida);


	// solicita a posição para defender
		int iJogadorDefesa;
		//printf("Qual a posição em que quer defender?:");
		//scanf( "%d", &iJogadorDefesa );
		sprintf(msgEnviada, "»I|%s", "Qual a posição em que quer defender?:");
		envia_mensagem(msgEnviada);
		recebe_mensagem();
		iJogadorDefesa = atoi(msgRecebida);

	// random de da posição onde o monstro defende/ataca
	// (cima, baixo, esquerda, direita, foge)
		int iMonstroAtaque = GetRandomNumber(0, 4);
		int iMonstroDefesa = GetRandomNumber(0, 4 + iHandyCap);
	
	// calcula dano
		int iDanoJogador = GetRandomNumber(1, MAX_DANO);
		int iDanoMonstro = GetRandomNumber(1, MAX_DANO - iHandyCap);

	// Simula a luta
		// ATAQUE
		if (iJogadorAtaque == iMonstroDefesa)
		{
			//printf("O monstro defendeu o ataque!\n");
			//envia_mensagem("O monstro defendeu o ataque!\n");
			sprintf(msgEnviada, "O monstro defendeu o ataque!\n");
			envia_mensagem(msgEnviada);
		}
		else
		{
			// desconta o dano no monstro
			//printf("Causou %d pontos de dano no monstro %s\n", iDanoJogador, pMonstro->nome);
			pMonstro->energia = pMonstro->energia - iDanoJogador;
			sprintf(msgEnviada, "Causou %d pontos de dano no monstro %s\n", iDanoJogador, pMonstro->nome);
			envia_mensagem(msgEnviada);
		}

		// DEFESA
		if (iMonstroAtaque == iJogadorDefesa)
		{
			//printf("O jogador defendeu o ataque!\n");
			envia_mensagem("O jogador defendeu o ataque!\n");
		}
		else
		{
			// desconta o dano no jogador
			//printf("O monstro %s causou-lhe %d pontos de dano\n", pMonstro->nome, iDanoMonstro);
			pJogador->energia = pJogador->energia - iDanoMonstro;
			sprintf(msgEnviada, "O monstro %s causou-lhe %d pontos de dano\n", pMonstro->nome, iDanoMonstro);
			envia_mensagem(msgEnviada);
		}

	// Valida se a luta terminou
		if (pJogador->energia <= 0)
		{
			// o monstro matou o jogador
			//printf("Infelizmente o monstro %s matou-o!\n", pMonstro->nome);
			sprintf(msgEnviada, "»L|1|1");
			envia_mensagem(msgEnviada);

			sprintf(msgEnviada, "Infelizmente o monstro %s matou-o!\n", pMonstro->nome);
			envia_mensagem(msgEnviada);
			iReturn = 0;
		}

		if (pMonstro->energia <= 0)
		{
			// remove o monstro do mapa
			pMonstro->localizacao = -1;
			
			sprintf(msgEnviada, "»L|1|0");
			envia_mensagem(msgEnviada);

			// o jogador matou o monstro
			//printf("Parabéns! Matou o monstro %s!\n", pMonstro->nome);
			sprintf(msgEnviada, "Parabéns! Matou o monstro %s!\n", pMonstro->nome);
			envia_mensagem(msgEnviada);

			iReturn = 0;
		}

		// pausa se a luta terminou
		//system("pause");

		return iReturn;
}

// Tenta apanhar o tesouro
bool apanha_tesouro(struct Jogador *pJogador, struct Celula pMapa[])
{
	if (pMapa[pJogador->localizacao].item == 0)
	{
		// define o jogador como tendo o tesouro
		pJogador->flg_tem_tesouro = 0;

		// retira o tesouro da célula
		pMapa[pJogador->localizacao].item = -1;

		// o tesouro foi apanhado
		return true;
	}

	// o tesouro não foi apanhado
	return false;
}

// valida os comandos disponíveis para o jogador
char* valida_comandos_disponiveis(struct Celula *pMapa)
{
	int MAX_CHARS = 200;
	char* sComandosDisponiveis;
	sComandosDisponiveis = (char *) malloc( MAX_CHARS );
	strcpy((char*) sComandosDisponiveis, "+------------------------------\n");

	// valida Norte
		if (pMapa->norte >= 0){	strcat((char*) sComandosDisponiveis, "| N - Norte\n");}
	// valida Sul
		if (pMapa->sul >= 0)  {	strcat((char*) sComandosDisponiveis, "| S - Sul\n");  }
	// valida Este
		if (pMapa->este >= 0) {	strcat((char*) sComandosDisponiveis, "| E - Este\n"); }
	// valida Oeste
		if (pMapa->oeste >= 0){	strcat((char*) sComandosDisponiveis, "| O - Oeste\n");}

	// Opções funcionais
	// adiciona return
		strcat((char*) sComandosDisponiveis, "|\n");
	// Gravar Jogo
		strcat((char*) sComandosDisponiveis, "| G - Gravar Jogo\n");
	// sai do Jogo
		strcat((char*) sComandosDisponiveis, "| 0 - Sair do Jogo\n");

	return (char*) sComandosDisponiveis;
}

// valida switches
void validaSwitches(int argc, char* args[], bool *pSuperUser, char *pFicheiroMapa)
{
	bool godMode = false;

	for (int i = 1; i < argc; i++)
	{
		if ( strcmp(strupr(args[i]), "SU") == 0 ) { *pSuperUser = true; }			     // modo SuperUser
		if ( strcmp(strupr(args[i]), "M") == 0  ) { strcpy(pFicheiroMapa, args[i+1]); }  // faz load do mapa
	}
}

//	Se a Localização do Jogador for igual à do Monstro
void valida_condicoes_luta(struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula pMapa[], bool blnSuperUser)
{
	if (pJogador->localizacao == pMonstro->localizacao)
	{

		// avisa o jogador que se encontra na mesma localização que o monstro e que tem de lutar
		//printf("Encontraste o agente do jogador ao entrares na sala! Inicia-se uma violenta batalha...\n");
		//system("pause");
		int iResultado = 1;

		// envia dados para o cliente
		envia_mensagem("Encontraste o agente do jogador ao entrares na sala! Inicia-se uma violenta batalha...\n");

		// Lutar
		while (iResultado != 0)
		{
			sprintf(msgEnviada, "»L|0|0"); // Tipo de comando|0 inicio, 1 fim| 0 vivo, 1 morto
			envia_mensagem(msgEnviada);

			//"»M|nome|energia|localizacao"
			sprintf(msgEnviada, "»M|%s|%d|%d", pMonstro->nome, pMonstro->energia, pMonstro->localizacao);
			envia_mensagem(msgEnviada);
	
			//"»J|nome|energia|localizacao|flg_tem_tesouro"
			sprintf(msgEnviada, "»J|%s|%d|%d|%d", pJogador->nome, pJogador->energia, pJogador->localizacao, pJogador->flg_tem_tesouro);
			envia_mensagem(msgEnviada);

			//"»S|1"  0 - normal, 1 - luta
			sprintf(msgEnviada, "»S|%d", 1);
			envia_mensagem(msgEnviada);

			// descreve status
			iResultado = lutar(pJogador, pMonstro, pMapa, blnSuperUser);
		}
	}
}

// grava o estado actual do jogo
void grava_jogo(struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula pMapa[])
{
	printf("A gravar o jogo...\n");

	FILE *f;						// ficheiro
	char l[ MAX_COL ];				// linha
	char* sResposta[2];
	strcpy((char*) sResposta, "");
	
	// valida se já existe uma gravação
	char* sNomeFicheiro[50];
	strcpy((char*) sNomeFicheiro, (char*) pJogador->nome);
	
	// tenta abrir um ficheiro de gravação já existente
	f = fopen( strcat((char*) sNomeFicheiro, ".txt"), "rb");

	if (f != NULL )
	{
		// se existir pergunta se deseja gravar por cima
		while (strcmp(strupr((char*) sResposta), "S") != 0 && strcmp(strupr((char*) sResposta), "N") != 0)
		{
			printf("Já existe um ficheiro de gravação para este utilizador, deseja gravar por cima?(S/N):");
			scanf( "%s", sResposta );
		}
	}

	if (strcmp(strupr((char*) sResposta), "S") == 0 || f == NULL )
	{
		// abre o ficheiro para escrita
		f = fopen((char*) sNomeFicheiro, "wb");

		// grava estrutura do jogador
		//---------------------------
		fwrite(":JOGADOR", sizeof(char), MAX_COL, f);					// cabeçalho :JOGADOR
		
		strcpy(l, trim((char*) pJogador->nome));						// nome
		fwrite(l, sizeof(char), MAX_COL, f);

		itoa(pJogador->energia, l, 10);									// energia
		fwrite(trim(l), sizeof(char), MAX_COL, f);
		
		itoa(pJogador->localizacao, l, 10);								// localização
		fwrite(trim(l), sizeof(char), MAX_COL, f);
		
		itoa(pJogador->flg_tem_tesouro, l, 10);							// tesouro
		fwrite(trim(l), sizeof(char), MAX_COL, f);

		// grava estrutura do monstro
		//---------------------------
		fwrite(":MONSTRO", sizeof(char), MAX_COL, f);					// cabeçalho :MONSTRO
		
		strcpy(l, trim((char*) pMonstro->nome));						// nome
		fwrite(l, sizeof(char), MAX_COL, f);
		
		itoa(pMonstro->energia, l, 10);									// energia
		fwrite(trim(l), sizeof(char), MAX_COL, f);
		
		itoa(pMonstro->localizacao, l, 10);								// localização
		fwrite(trim(l), sizeof(char), MAX_COL, f);
		
		// grava estrutura do mapa
		//------------------------
		fwrite(":MAPA", sizeof(char), MAX_COL, f);						// cabeçalho :MAPA

		// grava cada uma das células do mapa
		for (int i = 0; i < MAX_CELULAS; i++)
		{
			strcpy(l, (char*) pMapa[i].descricao);						// nome da célula
			fwrite(l, sizeof(char), MAX_COL, f);
			
			itoa(pMapa[i].norte, l, 10);								// norte
			fwrite(l, sizeof(char), MAX_COL, f);
			
			itoa(pMapa[i].sul, l, 10);									// sul
			fwrite(l, sizeof(char), MAX_COL, f);
			
			itoa(pMapa[i].este, l, 10);									// este
			fwrite(trim(l), sizeof(char), MAX_COL, f);
			
			itoa(pMapa[i].oeste, l, 10);								// oeste
			fwrite(l, sizeof(char), MAX_COL, f);
			
			itoa(pMapa[i].item, l, 10);									// item
			fwrite(l, sizeof(char), MAX_COL, f);
		}

		// fecha o ficheiro
		fclose(f);

		printf("Jogo gravado com sucesso!");
		system("pause");
	}
}

// testa o final do jogo
bool testa_fim_jogo(struct Jogador *pJogador)
{
	return !(pJogador->energia > 0 && !(pJogador->flg_tem_tesouro == 0 && pJogador->localizacao == 0));
}

// novo jogo
void novo_jogo(struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula pMapa[], bool blnSuperUser, char* pFicheiroMapa)
{
	//--------------------------------
	//Chama a função para inicializar o jogador
	//--------------------------------

	//jogador = ( struct Jogador )malloc( sizeof ( struct Jogador ) );
	//struct jsw_node *rn = ( struct jsw_node*)malloc(sizeof(struct jsw_node));
	char * nomeJogador[50];
	// solicita o nome do jogador
	//printf( "Indique o nome do jogador:" );
	//scanf( "%s", &nomeJogador );
	sprintf(msgEnviada, "»I|%s", "Indique o nome do jogador:");
	envia_mensagem(msgEnviada);
	recebe_mensagem();

	strcpy((char*) nomeJogador, msgRecebida);

	inicializa_jogador(pJogador, (char *) nomeJogador, blnSuperUser );

	//--------------------------------
	//Chama a função para inicializar o mapa
	//--------------------------------
	inicializa_mapa( pMapa, (char *) pFicheiroMapa );
	//inicializa_mapa_teste (pMapa);
	//system("pause");

	//--------------------------------
	//Chama a função para inicializar o monstro
	//--------------------------------
	inicializa_monstro( pMonstro );
}

void convert_file()
{
	char fBinario[100];
	char fOriginal[100];

	strcpy(fBinario, "");
	strcpy(fOriginal, "");

	sprintf(msgEnviada, "»I|%s", "Qual o nome do ficheiro a converter?:");
	envia_mensagem(msgEnviada);
	recebe_mensagem();

	strcat((char*) fOriginal, msgRecebida);
	strcat((char*) fBinario, msgRecebida);

	strcat((char*) fOriginal, ".txt");
	strcat((char*) fBinario, "b.txt");

	//1 converte para binário, nome do ficheiro original, nome do ficheiro binário
	sprintf(msgEnviada, "»O|1|%s|%s", fOriginal, fBinario);
	envia_mensagem(msgEnviada);
}

// carrega um jogo previamente gravado
void carrega_jogo(struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula pMapa[])
{
	printf("A carregar o jogo...\n");
	
	sprintf(msgEnviada, "»I|%s", "Qual o nome do jogo?:");
	envia_mensagem(msgEnviada);
	recebe_mensagem();

	sprintf(msgEnviada, "»O|0|%s", strcat((char*) msgRecebida, ".txt"));
	envia_mensagem(msgEnviada);
	recebe_mensagem();

	//FILE *f;
	char l[ MAX_COL ];

	// carrega o jogo com o nome do jogador
	//char* sNomeFicheiro[50];
	//strcpy((char*) sNomeFicheiro, (char*) pJogador->nome);
	//f = fopen( strcat((char*) sNomeFicheiro, ".txt"), "rb" );


	//if ( f != NULL )
	//{
		// lê o conteúdo do ficheiro
		//while( fread(l, sizeof(char), MAX_COL, f) != NULL ){
		while( strcmp(msgRecebida, "FIM" ) != 0 )
		{
			/*
			sprintf(msgEnviada, "»O|1", strcat((char*) msgEnviada, ".txt"));
			envia_mensagem(msgEnviada);
			recebe_mensagem();
			*/
			// se a linha for igual a mapa
			if (strcmp(strupr(msgRecebida),":MAPA") == 0)
			{

				// carrega cada uma das células do mapa
				for (int i = 0; i < MAX_CELULAS; i++)
				{
					//fread(l, sizeof(char), MAX_COL, f);
					envia_mensagem(msgEnviada);
					recebe_mensagem();
					//strcpy((char*) pMapa[i].descricao, l);		// nome da célula
					strcpy((char*) pMapa[i].descricao, msgRecebida);		// nome da célula
					
					//fread(l, sizeof(char), MAX_COL, f);
					//pMapa[i].norte = atoi(l);							// norte
					envia_mensagem(msgEnviada);
					recebe_mensagem();
					pMapa[i].norte = atoi(msgRecebida);

					//fread(l, sizeof(char), MAX_COL, f);
					//pMapa[i].sul = atoi(l);								// sul
					envia_mensagem(msgEnviada);
					recebe_mensagem();
					pMapa[i].sul = atoi(msgRecebida);								// sul
					
					//fread(l, sizeof(char), MAX_COL, f);
					//pMapa[i].este = atoi(l);							// este
					envia_mensagem(msgEnviada);
					recebe_mensagem();
					pMapa[i].este = atoi(msgRecebida);							// este
					
					//fread(l, sizeof(char), MAX_COL, f);
					//pMapa[i].oeste = atoi(l);							// oeste
					envia_mensagem(msgEnviada);
					recebe_mensagem();
					pMapa[i].oeste = atoi(msgRecebida);							// oeste
					
					//fread(l, sizeof(char), MAX_COL, f);
					//pMapa[i].item = atoi(l);							// item
					envia_mensagem(msgEnviada);
					recebe_mensagem();
					pMapa[i].item = atoi(msgRecebida);							// item
					
				}
			}

			if (strcmp(strupr(msgRecebida),":JOGADOR") == 0)
			{
				//fread(l, sizeof(char), MAX_COL, f);
				//strcpy((char*) pJogador->nome, l);				// nome do jogador
				envia_mensagem(msgEnviada);
				recebe_mensagem();
				strcpy((char*) pJogador->nome, msgRecebida);				// nome do jogador

				//fread(l, sizeof(char), MAX_COL, f);
				//pJogador->energia = atoi (l);							// energia
				envia_mensagem(msgEnviada);
				recebe_mensagem();
				pJogador->energia = atoi (msgRecebida);							// energia
				

				//fread(l, sizeof(char), MAX_COL, f);
				//pJogador->localizacao = atoi (l);						// localizacao
				envia_mensagem(msgEnviada);
				recebe_mensagem();
				pJogador->localizacao = atoi (msgRecebida);						// localizacao

				//fread(l, sizeof(char), MAX_COL, f);
				//pJogador->flg_tem_tesouro = atoi (l);					// tesouro
				envia_mensagem(msgEnviada);
				recebe_mensagem();
				pJogador->flg_tem_tesouro = atoi (msgRecebida);					// tesouro

			}

			if (strcmp(strupr(msgRecebida),":MONSTRO") == 0)
			{
				//fread(l, sizeof(char), MAX_COL, f);
				//strcpy((char*) pMonstro->nome, l);				// nome do monstro
				envia_mensagem(msgEnviada);
				recebe_mensagem();
				strcpy((char*) pMonstro->nome, msgRecebida);				// nome do monstro

				//fread(l, sizeof(char), MAX_COL, f);
				//pMonstro->energia = atoi (l);							// energia
				envia_mensagem(msgEnviada);
				recebe_mensagem();
				pMonstro->energia = atoi (msgRecebida);							// energia

				//fread(l, sizeof(char), MAX_COL, f);
				//pMonstro->localizacao = atoi (l);						// localizacao
				envia_mensagem(msgEnviada);
				recebe_mensagem();
				pMonstro->localizacao = atoi (msgRecebida);						// localizacao

			}
			
			envia_mensagem(msgEnviada);
			recebe_mensagem();

		}
		//fclose( f );

	printf("Jogo carregado com sucesso!\n");
	system("pause");
}

// executa os comandos funcionais
void comandos_funcionais(int iAccao, struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula pMapa[])
{
	// grava jogo
	if (iAccao == 100) {
		grava_jogo(pJogador, pMonstro, pMapa); 
	}
	// sai do jogo
	if (iAccao == 101) {
		printf("Jogo terminado!\n");
		system("pause");
		exit(EXIT_SUCCESS);
	}
}

// movimenta o monstro
void movimenta_monstro(struct Monstro *pMonstro, struct Celula pMapa[])
{

	// faz random para decidir se movimenta ou não
	int iMovimenta = GetRandomNumber(0, 1);
	
	// é para movimentar
	if ( iMovimenta == 1 && pMonstro->localizacao != -1)
	{
		// verifica quais as saídas existentes na localização actual do monstro
		int iLocalizacaoActual = pMonstro->localizacao;
		
		int iEste = pMapa[iLocalizacaoActual].este;
		int iNorte = pMapa[iLocalizacaoActual].norte;
		int iSul = pMapa[iLocalizacaoActual].sul;
		int iOeste = pMapa[iLocalizacaoActual].oeste;

		// adiciona as entradas com valor maior ou igual a 0 a um array
		unsigned int iSaidas[4];
		int iPos = -1;

		if (iEste  >= 0) { iPos++; iSaidas[iPos]=iEste; }
		if (iNorte >= 0) { iPos++; iSaidas[iPos]=iNorte; }
		if (iSul   >= 0) { iPos++; iSaidas[iPos]=iSul; }
		if (iOeste >= 0) { iPos++; iSaidas[iPos]=iOeste; }

		// faz random das saídas existentes
		int iNovaLocalizacao = iSaidas[GetRandomNumber(0, iPos)];

		// altera a localização do monstro
		pMonstro->localizacao = iNovaLocalizacao;
	}

}

// Movimenta ao jogador para a localização pretendida
void movimenta_jogador(int iLocalizacao, struct Jogador *pJogador)
{
	WaitForSingleObject( hMutexJogo,INFINITE );

	// validar se o comando recebido está nos comandos disponíveis
	if (iLocalizacao >= 0) {
		// movimenta jogador
		pJogador->localizacao = iLocalizacao;
	}

	ReleaseMutex(hMutexJogo);
}

DWORD WINAPI threadMonstro( LPVOID lpParam ) 
{
	struct Monstro *ppMonstro;
	struct Celula *ppMapa;
	
	ppMonstro = ((struct argThreadMonstro*) lpParam)->pMonstro;
	ppMapa = ((struct argThreadMonstro*) lpParam)->pMapa;

	while (ppMonstro->energia > 0)
	{
		WaitForSingleObject( hMutexJogo,INFINITE );

		movimenta_monstro(ppMonstro, ppMapa);

		//descreve_monstro(ppMonstro, ppMapa, true);

		ReleaseMutex(hMutexJogo);

		//printf("Aqui-------------------------\n");
		//Sleep(1000);
	}

	return 0;
}

DWORD WINAPI threadJogador( LPVOID lpParam ) 
{
	struct Jogador *ppJogador;
	struct Monstro *ppMonstro;
	struct Celula *ppMapa;
	
	ppJogador = ((struct argThreadJogador*) lpParam)->pJogador;
	ppMonstro = ((struct argThreadJogador*) lpParam)->pMonstro;
	ppMapa = ((struct argThreadJogador*) lpParam)->pMapa;

	int iAccao = -1;

	while ( testa_fim_jogo(ppJogador) == false )
	{
		WaitForSingleObject( hMutexJogo,INFINITE );

		// envia dados para o cliente
		sprintf(msgEnviada, "»L|1|0"); // Tipo de comando|0 inicio, 1 fim| 0 vivo, 1 morto
		envia_mensagem(msgEnviada);

		//"»M|nome|energia|localizacao"
		sprintf(msgEnviada, "»M|%s|%d|%d", ppMonstro->nome, ppMonstro->energia, ppMonstro->localizacao);
		envia_mensagem(msgEnviada);
	
		//"»J|nome|energia|localizacao|flg_tem_tesouro"
		sprintf(msgEnviada, "»J|%s|%d|%d|%d", ppJogador->nome, ppJogador->energia, ppJogador->localizacao, ppJogador->flg_tem_tesouro);
		envia_mensagem(msgEnviada);

		//"»C|descricao|norte|sul|este|oeste"
		sprintf(msgEnviada, "»C|%s|%d|%d|%d|%d", ppMapa[ppJogador->localizacao].descricao, ppMapa[ppJogador->localizacao].norte, ppMapa[ppJogador->localizacao].sul, ppMapa[ppJogador->localizacao].este, ppMapa[ppJogador->localizacao].oeste);
		envia_mensagem(msgEnviada);

		//"»S|0"  0 - normal, 1 - luta
		sprintf(msgEnviada, "»S|%d", 0);
		envia_mensagem(msgEnviada);

		//	Aceitar Comando do Jogador
		//"»C|descricao|norte|sul|este|oeste"
		sprintf(msgEnviada, "»I|%s", "Insira um comando:");
		envia_mensagem(msgEnviada);
		recebe_mensagem();

		iAccao = aceita_comando_jogador(msgRecebida, ppJogador, ppMapa);

		// se o comando for inválido dá mensagem de erro
		if (iAccao < 0)
		{
			printf("Servidor>Comando inválido!: %s\n", msgRecebida);
			envia_mensagem("Comando inválido!");
		}

		// TODO: se a acção for igual ou superior a 100 é uma acção funcional
		if (iAccao >= 100)
		{
			switch (iAccao)
			{
				// Grava jogo
				case 100:
					//"»C|descricao|norte|sul|este|oeste"
					sprintf(msgEnviada, "»O|0");
					envia_mensagem(msgEnviada);

					break;
				// Sai do Jogo
				case 101:
					break;
			}
			//comandos_funcionais(iAccao, ppJogador, pMonstro, ppMapa);
		}
		else
		{
			movimenta_jogador(iAccao, ppJogador);

			//  apanha o tesouro
			bool tesouro = apanha_tesouro(ppJogador, ppMapa);

			if (tesouro == true)
			{
				//printf("Apanhou o tesouro! %d\n", jogador.flg_tem_tesouro);
				//system("pause");
			}

		}

		ReleaseMutex(hMutexJogo);
	}
	
	//printf("Aqui-------------------------Jogador\n");
	//Sleep(100000);

	return 0;
}

int cria_pipe()
{
	//Inicialização das variáveis que irão permitir criar o named pipe
	SECURITY_ATTRIBUTES secAttrib;
	SECURITY_DESCRIPTOR* pSecDesc;
	
	pSecDesc = (SECURITY_DESCRIPTOR*)LocalAlloc(LPTR, 
		SECURITY_DESCRIPTOR_MIN_LENGTH);
	
	InitializeSecurityDescriptor(pSecDesc,
		SECURITY_DESCRIPTOR_REVISION);
	
	//(PACL) NULL permite o acesso de todos ao named pipe
	SetSecurityDescriptorDacl(pSecDesc,TRUE,(PACL)NULL,FALSE);
	
	secAttrib.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttrib.bInheritHandle = TRUE;
	secAttrib.lpSecurityDescriptor = pSecDesc;

	//Criação do named pipe
	//Nota alterar em Project Properties o Character Set para Not Set (ASCII)
	//caso contrário irá utilizar a versão UNICODE da função CreateNamedPipe 
	hPipe = CreateNamedPipe( 
          PIPE_NAME,				// pipe name 
          PIPE_ACCESS_DUPLEX,       // read/write access 
          PIPE_TYPE_BYTE |			// byte type pipe 
          PIPE_READMODE_BYTE |		// byte-read mode 
          PIPE_WAIT,                // blocking mode 
          PIPE_UNLIMITED_INSTANCES, // max. instances  
          BUF_SIZE,                 // output buffer size 
          BUF_SIZE,                 // input buffer size 
          0,                        // client time-out 
          &secAttrib );            // access to everyone

      if (hPipe == INVALID_HANDLE_VALUE){ 
          printf( "Servidor: Erro na criação do named pipe.\n" );
		  PrintErrorMsg();
		return -1;
	  }

	  
	  /*DWORD availableData = 0;
	  do{
		  PeekNamedPipe( hPipe, NULL, 0, NULL, &availableData, NULL );
	  }while( availableData == 0 );*/

	  printf( "Servidor: vou conectar-me ao named pipe para esperar por uma ligação.\n" );

	  ConnectNamedPipe( hPipe, NULL );
      printf( "Servidor: Conexão estabelecida, vou ler do named pipe.\n" );

	  /*
	  //Leitura do named pipe
	  char msg[ MAX_MSG ];
	  DWORD cbRead;

	  //A função ReadFile só deve ser chamada depois de ambos os processos
	  //se encontrarem ligados através do pipe
	  BOOL resSuccess = ReadFile( 
			 hPipe,    // pipe handle 
			 msg,    // buffer to receive reply 
			 BUF_SIZE,  // size of buffer 
			 &cbRead,  // number of bytes read 
			 NULL);    // not overlapped 

	  if( resSuccess != TRUE ){
		  printf( "Servidor: Erro na leitura do named pipe.\n" );
          PrintErrorMsg();
		  return -1;
	  }
	  
	  printf( "Servidor: Recebi >%s<\n", msg );
	  system("Pause");


	  DWORD cbWritten;
	  resSuccess = WriteFile( 
		  hPipe,        // handle to pipe 
		  msg,     // buffer to write from 
		  MAX_MSG, // number of bytes to write 
		  &cbWritten,   // number of bytes written 
		  NULL);        // not overlapped I/O 

	if( ! resSuccess ){ 
		printf( "Servidor: Erro no envio." );
		PrintErrorMsg();
		return -1;
	}

	 //Libertação do handle do named pipe
	 CloseHandle( hPipe );

	return 0;
	*/
}

// inicia jogo
void inicia_jogo(struct Jogador *pJogador, struct Monstro *pMonstro, struct Celula pMapa[], bool blnSuperUser)
{
	//--------------------------------
	// Inicia o jogo
	//--------------------------------

	hMutexJogo= CreateMutex( 
		NULL,                       // default security attributes
		FALSE,                      // initially not owned
		NULL);                      // unnamed mutex

	// variaveis do monstro
	struct argThreadMonstro argTMonstro;
	argTMonstro.pMapa = pMapa;
	argTMonstro.pMonstro = pMonstro;
	
	// variaveis do jogador
	struct argThreadJogador argTJogador;
	argTJogador.pMapa = pMapa;
	argTJogador.pJogador = pJogador;
	argTJogador.pMonstro = pMonstro;

	//	Movimentar Monstro
	HANDLE tMonstro = CreateThread( 
		NULL,              // default security attributes
		0,                 // use default stack size  
		threadMonstro,     // thread function 
		&argTMonstro,      // argument to thread function 
		0,                 // use default creation flags 
		NULL);   // returns the thread identifier

	//	Movimentar Jogador
	HANDLE tJogador = CreateThread( 
		NULL,              // default security attributes
		0,                 // use default stack size  
		threadJogador,     // thread function 
		&argTJogador,      // argument to thread function 
		0,                 // use default creation flags 
		NULL);   // returns the thread identifier
	
	//Enquanto não for Fim de Jogo 
	while (testa_fim_jogo(pJogador) == false)
	{
		WaitForSingleObject( hMutexJogo,INFINITE );

		// valida se o monstro encontrou o jogador, se encontrou inicia a luta
		valida_condicoes_luta(pJogador, pMonstro, pMapa, blnSuperUser);

		ReleaseMutex(hMutexJogo);
	}

	// terminou o jogo
	system("cls");

	if (pJogador->energia > 0)
	{

		//"»F|0"
		sprintf(msgEnviada, "»F|0");

		// ganhou
		printf("+-------------------------------------------------------------------------+\n");
		printf("| Sais rapidamente do restaurante levando contigo o grandioso jogador. A  |\n");
		printf("| notícia espalhou-se rapidamente por tudo o mundo e o seu já se encontra |\n");
		printf("| inundado de charters…Cumpriste o teu objectivo e salvastes a época do   |\n");
		printf("| clube!                                                                  |\n");
		printf("+-------------------------------------------------------------------------+\n");
		
	}
	else
	{

		//"»F|0"
		sprintf(msgEnviada, "»F|1");
		
		// perdeu
		printf("+-------------------------------------------------------------------------+\n");
		printf("| Infelizmente não foste capaz de salvar o grande jogador chinês!         |\n");
		printf("| Os charters de chineses nunca virão a ser o que se esperava!            |\n");
		printf("+-------------------------------------------------------------------------+\n");
	}

	envia_mensagem(msgEnviada);

	printf("\n\n");
	printf("____ _ _  _    ___  ____     _ ____ ____ ____   /\n");
	printf("|___ | |\\/|    |  \\ |  |     | |  | | __ |  |  / \n");
	printf("|    | |  |    |__/ |__|    _| |__| |__] |__| .  \n");
                                                 
	system( "pause" );
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

// The one and only application object

CWinApp theApp;

using namespace std;

int main(int argc, char * argv[], char * envp[])
{

	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			int iResultado;
			setlocale( LC_ALL, "Portuguese" );
			srand(time(0));

			//--------------------------------
			//Chama a função para validar os switches
			//--------------------------------
			bool blnSuperUser = false;						// define se está ou não no modo superuser
			char* ficheiroMapa[100];						// nome do ficheiro do mapa
			strcpy((char*) ficheiroMapa, "");

			validaSwitches(argc, argv, &blnSuperUser, (char*) ficheiroMapa);

			//--------------------------------
			// Define as estruturas a serem utilizadas no jogo
			//--------------------------------
			struct Celula mapa[MAX_CELULAS];				//Cria um array de células para o mapa do jogo
			struct Jogador jogador;
			struct Monstro monstro;
	
			//--------------------------------
			// Chama a função que imprime o menu principal
			//--------------------------------
			int iOpcao = -1;
			bool blnOpcaoMenu = false;

			//--------------------------------
			// Chama a função que cria o pipe para permitir ligações de clientes
			//--------------------------------
			cria_pipe();

			while (iOpcao != 0)
			{
				// solicita a opção ao jogador
			
				//"»S|1"  0 - normal, 1 - luta, 2 - menu principal
				sprintf(msgEnviada, "»S|%d", 2);
				envia_mensagem(msgEnviada);

				sprintf(msgEnviada, "»I|%s", "Escolha uma opção:");
				envia_mensagem(msgEnviada);
				recebe_mensagem();
				
				/*
				printf("opção: %d", (int) msg[0]);
				system("pause");

				if ((int) msg != 32)
				{
					iOpcao = atoi(msg);
				}

				*/
				iOpcao = atoi(msgRecebida);
				printf("Servidor>recebi opção: %d\n", iOpcao);

				// Valida a opção escolhida
				switch ( iOpcao )
				{
					case 0:	// Sai do jogo
						printf("Saiu do jogo");
						break;

					case 1:	// Novo jogo
						//novo_jogo(&jogador, &monstro, mapa, blnSuperUser, (char*) ficheiroMapa);
						novo_jogo(&jogador, &monstro, mapa, blnSuperUser, (char*) ficheiroMapa);
						inicia_jogo(&jogador, &monstro, mapa, blnSuperUser);
						break;

					case 2:	// Carregar jogo
						// solicita o nome do jogo
						//printf("Qual o nome do jogo?:");
						//scanf( "%s", &jogador.nome );
						carrega_jogo(&jogador, &monstro, mapa);
						inicia_jogo(&jogador, &monstro, mapa, blnSuperUser);
						break;

					case 3:	// Converter mapa para binário
						//converte_ficheiro();
						convert_file();
						break;
				}
			}

			return 0;
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
