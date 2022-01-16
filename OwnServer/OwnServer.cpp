// OwnServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <rfb/rfb.h>
#include "ResizeWnd.h"

#define FPS 25
#define UPDATE_INTERVAL (CLOCKS_PER_SEC/FPS) //(CLOCKS_PER_SEC / 10)

void CaptureScreen(rfbScreenInfoPtr rfbScreen, int nWidth, int nHeight)
{
	memset(rfbScreen->frameBuffer, rand() % 256, nWidth * nHeight * 4);
}

int main(int argc, char* argv[])
{
	// path, port, width, height
	if (argc != 4)
	{
		std::cout << "Invalid arguments." << std::endl;
		return 0;
	}

	int		nPort = atoi(argv[1]);
	int		nWidth = atoi(argv[2]);
	int		nHeight = atoi(argv[3]);

	if (nWidth & 3)
		nWidth += 4 - (nWidth & 3);

	printf("Port: %d, Width(rounded): %d, Height: %d\n", nPort, nWidth, nHeight);
	rfbScreenInfoPtr server = rfbGetScreen(NULL, NULL, nWidth, nHeight, 8, 3, 4);
	server->port = nPort;
	server->frameBuffer = (char *)malloc(nWidth * nHeight* 4);

	rfbInitServer(server);
	// rfbRunEventLoop(server, UPDATE_INTERVAL * 1000, FALSE);
	int begin = clock();
	while (rfbIsActive(server))
	{
		int end = clock();
		if (end - begin >= UPDATE_INTERVAL)
		{
			begin = clock() - (end - begin - UPDATE_INTERVAL);

			CaptureScreen(server, nWidth, nHeight);
			
			rfbMarkRectAsModified(server, 0, 0, nWidth, nHeight);
		}
		rfbProcessEvents(server, UPDATE_INTERVAL);
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
