#include "pch.h"
#include "Global.h"

WCHAR		g_szAppPath[MAX_PATH];
PARAM		g_param;

void GetAppPath()
{
	GetModuleFileName(GetModuleHandle(0), g_szAppPath, MAX_PATH);

	for (int i = wcslen(g_szAppPath) - 1; i >= 0; i--)
	{
		if (g_szAppPath[i] == L'\\')
		{
			g_szAppPath[i] = 0;
			break;
		}
	}
}

void LoadSettings()
{
	WCHAR		szPath[MAX_PATH];
	WCHAR		szTemp[MAX_PATH];
	
	swprintf(szPath, L"%s\\Settings.ini", g_szAppPath);

	HDC hdcDesk = ::GetDC(HWND_DESKTOP);
	int nScreenWidth = ::GetDeviceCaps(hdcDesk, HORZRES);
	int nScreenHeight = ::GetDeviceCaps(hdcDesk, VERTRES);
	::ReleaseDC(HWND_DESKTOP, hdcDesk);

	g_param.nPort = GetPrivateProfileInt(L"OwnServer", L"Port", 5900, szPath);
	g_param.nWidth = GetPrivateProfileInt(L"OwnServer", L"Width", nScreenWidth, szPath);
	if (g_param.nWidth & 3)
		g_param.nWidth += 4 - (g_param.nWidth & 3);
	g_param.nHeight = GetPrivateProfileInt(L"OwnServer", L"Height", nScreenHeight, szPath);
	g_param.szPassword = (char*)calloc(1, MAX_PATH);
	GetPrivateProfileString(L"OwnServer", L"Password", L"", szTemp, MAX_PATH, szPath);
	sprintf(g_param.szPassword, "%S", szTemp);

	GetPrivateProfileString(L"OwnServer", L"Name", L"VUBIG_VNC", szTemp, MAX_PATH, szPath);
	sprintf(g_param.szName, "%S", szTemp);
}

void SaveSettings()
{
	WCHAR		szPath[MAX_PATH];
	WCHAR		szTemp[MAX_PATH];

	swprintf(szPath, L"%s\\Settings.ini", g_szAppPath);

	swprintf(szTemp, L"%d", g_param.nPort);
	WritePrivateProfileString(L"OwnServer", L"Port", szTemp, szPath);

	swprintf(szTemp, L"%d", g_param.nWidth);
	WritePrivateProfileString(L"OwnServer", L"Width", szTemp, szPath);

	swprintf(szTemp, L"%d", g_param.nHeight);
	WritePrivateProfileString(L"OwnServer", L"Height", szTemp, szPath);

	swprintf(szTemp, L"%S", g_param.szPassword);
	WritePrivateProfileString(L"OwnServer", L"Password", szTemp, szPath);

	swprintf(szTemp, L"%S", g_param.szName);
	WritePrivateProfileString(L"OwnServer", L"Name", szTemp, szPath);
}