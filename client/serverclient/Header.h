#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WIndows.h>

#define SERVERCALL _declspec(dllexport)

#pragma pack(push, 8)

typedef struct _TAGSERVERINSTANCE
{
	HANDLE hOutputPipe;
	HANDLE hInputPipe;
} SERVERINSTANCE;

#pragma pack(pop)

#if defined(__cplusplus)
extern "C" {
#endif

	SERVERCALL BOOL InstantiateServer(SERVERINSTANCE*,
		PROCESS_INFORMATION*);


	SERVERCALL void WriteMessage(SERVERINSTANCE*,
		LPWSTR,
		int);

	SERVERCALL void TerminateServer(SERVERINSTANCE*);

#if defined(__cplusplus)
}
#endif