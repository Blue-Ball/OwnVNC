#pragma once

typedef struct _PARAM_
{
	int		nPort;
	int		nWidth;
	int		nHeight;
	char* szPassword;
	char	szName[MAX_PATH];
} PARAM;

extern WCHAR			g_szAppPath[MAX_PATH];
extern PARAM			g_param;

void GetAppPath();
void LoadSettings();
void SaveSettings();