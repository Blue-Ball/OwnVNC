// OwnServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <rfb/rfb.h>
#include "ximage.h"

#define FPS 25
#define UPDATE_INTERVAL (CLOCKS_PER_SEC/FPS) //(CLOCKS_PER_SEC / 10)

void CaptureScreen(rfbScreenInfoPtr rfbScreen, int nWidth, int nHeight)
{
	HDC hdcDesk = GetDC(HWND_DESKTOP);
	int nScreenWidth = GetDeviceCaps(hdcDesk, HORZRES);
	int nScreenHeight = GetDeviceCaps(hdcDesk, VERTRES);
	HDC hdcCopy = CreateCompatibleDC(hdcDesk);
	HBITMAP hBm = CreateCompatibleBitmap(hdcDesk, nScreenWidth, nScreenHeight);
	SelectObject(hdcCopy, hBm);
	BitBlt(hdcCopy, 0, 0, nScreenWidth, nScreenHeight, hdcDesk, 0, 0, SRCCOPY);

	// create a CxImage from the screen grab
	CxImage* image = new CxImage(nScreenWidth, nScreenHeight, 24);
	GetDIBits(hdcDesk, hBm, 0, nScreenHeight, image->GetBits(),
		(LPBITMAPINFO)image->GetDIB(), DIB_RGB_COLORS);
	// image->CreateFromHBITMAP(hBm);

	// clean up the bitmap and dcs
	ReleaseDC(HWND_DESKTOP, hdcDesk);
	DeleteDC(hdcCopy);
	DeleteObject(hBm);

	image->Resample(nWidth, nHeight);
	int				i, j;
	for (i = 0; i < nHeight; i++)
	{
		// memcpy(rfbScreen->frameBuffer + i * nWidth * 3, image->GetBits(nHeight - i - 1), nWidth * 3);
		for (j = 0; j < nWidth; j++)
		{
			memcpy(rfbScreen->frameBuffer + i * nWidth * 4 + j * 4, 
				image->GetBits() + (nHeight - i - 1) * nWidth * 3 + j * 3, 3);
		}
	}

	delete image;
}

int main(int argc, char* argv[])
{
	// path, port, width, height
	if (argc != 4)
	{
		std::cout << "Invalid arguments." << std::endl;
		return 0;
	}

	HDC hdcDesk = GetDC(HWND_DESKTOP);
	int nScreenWidth = GetDeviceCaps(hdcDesk, HORZRES);
	int nScreenHeight = GetDeviceCaps(hdcDesk, VERTRES);
	ReleaseDC(HWND_DESKTOP, hdcDesk);

	int		nPort = atoi(argv[1]);
	int		nWidth = atoi(argv[2]);
	int		nHeight = atoi(argv[3]);

	if (nWidth == -1)
		nWidth = nScreenWidth;
	if (nHeight == -1)
		nHeight = nScreenHeight;

	if (nWidth & 3)
		nWidth += 4 - (nWidth & 3);

	printf("Port: %d, Width(rounded): %d, Height: %d\n", nPort, nWidth, nHeight);
	rfbScreenInfoPtr server = rfbGetScreen(NULL, NULL, nWidth, nHeight, 8, 3, 4);
	server->port = nPort;
	server->frameBuffer = (char*)malloc(nWidth * nHeight * 4);
	server->alwaysShared = TRUE;

	rfbInitServer(server);
// 	while (1) {
// 		rfbProcessEvents(server, UPDATE_INTERVAL*1000);
// 	}
	rfbRunEventLoop(server, UPDATE_INTERVAL * 1000, TRUE);

	int begin = clock();
	while (rfbIsActive(server))
	{
		int end = clock();
		if (end - begin >= UPDATE_INTERVAL)
		{
			begin = clock() - (end - begin - UPDATE_INTERVAL);

			CaptureScreen(server, nWidth, nHeight);

			rfbClientPtr cl;
			rfbClientIteratorPtr iter = rfbGetClientIterator(server);
			while ((cl = rfbClientIteratorNext(iter)))
			{
				rfbMarkRectAsModified(cl->screen, 0, 0, cl->screen->width-1, cl->screen->height-1);
			}
			rfbReleaseClientIterator(iter);
		}

		rfbProcessEvents(server, UPDATE_INTERVAL * 1000);
	}

    std::cout << "Good Bye!\n";

	free(server->frameBuffer);
	return 0;
}
// int main()
// {
// 	rfbScreenInfoPtr server = rfbGetScreen(NULL, NULL, 400, 300, 8, 3, 4);
// 	server->frameBuffer = (char *)malloc(400 * 300 * 4);
// 	rfbInitServer(server);
// 	rfbRunEventLoop(server, -1, FALSE);
//     std::cout << "Hello World!\n";
// }

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
