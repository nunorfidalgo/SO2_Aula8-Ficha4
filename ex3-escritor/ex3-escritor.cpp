#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#define BufferSize 100
#define Buffers 10

TCHAR NomeMemoria[] = TEXT("Nome da Memória Partilhada");
TCHAR(*PtrMemoria)[Buffers][BufferSize];

TCHAR NomeSemaforoPodeEscrever[] = TEXT("Semáforo Pode Escrever");
TCHAR NomeSemaforoPodeLer[] = TEXT("Semáforo Pode Ler");

HANDLE PodeEscrever;
HANDLE PodeLer;
HANDLE hMemoria;
HANDLE f1;

typedef struct {
	TCHAR buffer[Buffers][BufferSize];
	int In, Out;
} DADOS;

int _tmain(void)
{

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	DADOS *shm;
	char init = 0;
	int pos;

	PodeEscrever = CreateSemaphore(NULL, Buffers, Buffers, NomeSemaforoPodeEscrever);
	PodeLer = CreateSemaphore(NULL, 0, Buffers, NomeSemaforoPodeLer);

	f1 = CreateFile(TEXT("shm.txt"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() == ERROR_FILE_NOT_FOUND) {
		_tprintf(TEXT("[Erro] Abrir ficheiros (%d)\n"), GetLastError());
		init = 1;
	}

	hMemoria = CreateFileMapping(f1, NULL, PAGE_READWRITE, 0, sizeof(TCHAR[Buffers][BufferSize]), NomeMemoria);

	if (PodeEscrever == NULL || PodeLer == NULL || hMemoria == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		return -1;
	}
	PtrMemoria = (TCHAR(*)[Buffers][BufferSize])MapViewOfFile(hMemoria
		, FILE_MAP_WRITE, init, pos, sizeof(TCHAR[Buffers][BufferSize]));
	if (PtrMemoria == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da memória partilhada(%d)\n"), GetLastError());
		return -1;
	}
	for (int i = 0; i < 100; i++)
	{
		WaitForSingleObject(PodeEscrever, INFINITE);
		_tprintf(TEXT("Escrever para buffer %i\n"), i);
		_stprintf_s((*PtrMemoria)[i%Buffers], BufferSize, TEXT("Escritor-%i\n"), i);
		Sleep(1000);
		ReleaseSemaphore(PodeLer, 1, NULL);
	}
	UnmapViewOfFile(PtrMemoria);
	CloseHandle(PodeEscrever);
	CloseHandle(PodeLer);
	CloseHandle(hMemoria);
	return 0;
}