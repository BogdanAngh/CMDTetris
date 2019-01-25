#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <thread>
using namespace std;

wstring tetromino[7];
int nFieldWidth = 20;
int nFieldHeight = 18;
unsigned char * pField = nullptr;

int nScreenWidth = 120;		// Console Screen Size width	
int nScreenHeight = 30;		// Console Screen Size height

int nDrawOffset = 2;

int Rotate(int px, int py, int r);
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY);

int main()
{
	//create assets
	tetromino[0].append(L"..X...X...X...X.");
	tetromino[1].append(L".XX...X...X.....");
	tetromino[2].append(L".XX..X...X...X..");
	tetromino[3].append(L".....XX..XX.....");
	tetromino[4].append(L"..X..XX...X.....");
	tetromino[5].append(L"..X..XX..X......");
	tetromino[6].append(L".X...XX...X.....");

	pField = new unsigned char[nFieldHeight * nFieldWidth]; // Create array

	//Fill game field with 9's for border and 0's for spaces
	for (int x = 0; x < nFieldWidth; x++) {
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;


	// Game Logic 
	bool bGameOver = false;

	int nCurrentPiece = 6;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4]; //Right, Left, Down, Z


	//Game loop
	while (!bGameOver) 
	{	
		// GAME TIMING ==============================
		this_thread::sleep_for(50ms);

		// INPUT ====================================
		for (int k = 0; k < 4; k++)
		{														// R   L    D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}
		 
		// GAME LOGIC ===============================

		//Right arrow pressed
		if (bKey[0])
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY))
			{
				nCurrentX = nCurrentX + 1;
			}
		}

		//Left arrow pressed
		if (bKey[1])
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))
			{
				nCurrentX = nCurrentX - 1;
			}
		}

		//Down arrow pressed
		if (bKey[2])
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY - 1))
			{
				nCurrentY = nCurrentY + 1;
			}
		}

		//Z key pressed
		if (bKey[3])
		{
			/*if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))
			{
				nCurrentX = nCurrentX - 1;
			}*/
		}



		// RENDER OUTPUT  ============================

		//Draw field
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				screen[(y + nDrawOffset) * nScreenWidth + (x + nDrawOffset)] = L" ABCDEFG=#"[pField[y*nFieldWidth + x]]; //cool trick
				
			}
		}

		//Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
			{
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + nDrawOffset) * nScreenWidth + (nCurrentX + px + nDrawOffset)] = nCurrentPiece + 65;
			}
		//Display frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

	}

}

int Rotate(int px, int py, int r) 
{
	switch (r % 4) {
	case 0: return py * 4 + px;			// 0 degrees
	case 1: return 12 + py - py * 4;	// 90 degrees
	case 2: return 15 - py * 4 - px;	// 180 degrees
	case 3: return 3 - py + px * 4;		// 270 degrees
	}

	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	// iterate through each position of the tetromino
	for(int px = 0; px < 4; px ++)
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			//Bounds
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY+ py < nFieldHeight)
				{
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false; //fail, piece can't exist on a non-empty space on the field 
				}
			}
		}

	return true;
}
