#include "pch.h"
#include <iostream>
#include <vector>
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
    tetromino[0].append(L"..X...X...X...X."); // I
    tetromino[1].append(L".XX...X...X....."); // L
    tetromino[2].append(L".XX..X...X...X.."); // J
    tetromino[3].append(L".....XX..XX....."); // O
    tetromino[4].append(L"..X..XX...X....."); // T
    tetromino[5].append(L"..X..XX..X......"); // Z
    tetromino[6].append(L".X...XX...X....."); // S

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

    int nCurrentPiece = 5;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    bool bKey[4]; // Right, Left, Down, Z
	bool bRotateHold = false;

	int nSpeed = 20; // Game ticks until pieace falls 1 space 
	int nSpeedCounter = 0; // Game ticks counter
	bool bForceDown = false;

	int nPieceCount = 0;
	int nScore = 0;

	vector<int> vLines;

    //Game loop
    while (!bGameOver) 
    {	
        // GAME TIMING ==============================
        this_thread::sleep_for(50ms); //Game Tick
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);
        // INPUT ====================================
        for (int k = 0; k < 4; k++)
        {														// R   L    D Z 
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
        }
         
        // GAME LOGIC ===============================

        //Right arrow pressed
        nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0; // cool trick for adding 0 or 1 depending on the result 
                                                                                                                   // of the conditional

        //Left arrow pressed
        nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        

        //Down arrow pressed
        nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
		 
        //Z key pressed
        //nCurrentRotation += (bKey[3] && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY )) ? 1 : 0;

		if (bKey[3]) {
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else {
			bRotateHold = false;
		}

		if (bForceDown) 
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++; // piece continues to fall
			else
			{
				// Lock the current piece in the field
				for(int px = 0; px < 4; px ++)
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
						{
							pField[(nCurrentY + py)*nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}


				//Every 10th piece, the difficulty increases
				nPieceCount++;
				if (nPieceCount % 2 == 0)
				{
					if (nSpeed >= 10) nSpeed--;
				}

				// Check if we have lines
				for (int py = 0; py < 4; py++)
				{
					if (nCurrentY + py < nFieldHeight - 1)// boundry check
					{
						bool bLine = true;
						// Check the width of the field for existence of a line
						for (int px = 1; px < nFieldWidth; px++)
						{
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
						}

						if (bLine) {
							//Remove line, set to =
							for (int px = 1; px < nFieldWidth; px++)
							{
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							}

							vLines.push_back(nCurrentY + py); 
						}

					}
				}

				nScore += 20; // For every piece, give the player 20 points
				if (!vLines.empty()) {
					nScore += (1 << vLines.size()) * 100; //If the players has n lines at one time, give him (2^n) * 100 points
				}

				// Choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// If piece doesn't fit => GAME OVER
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY );
			}
			nSpeedCounter = 0;
		}


        // RENDER OUTPUT  ============================

        //Draw field
        for (int x = 0; x < nFieldWidth; x++) {
            for (int y = 0; y < nFieldHeight; y++) {
                screen[(y + nDrawOffset) * nScreenWidth + (x + nDrawOffset)] = L" ABCDEFG=#"[pField[y*nFieldWidth + x]]; //cool trick
                
            }
        }

		if (!vLines.empty())
		{	
			//Display Frame
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); // delay

			//move the lines above the '=' line on space down
			for (auto &v : vLines) {
				for (int px = 1; px < nFieldWidth; px++)
				{
					for (int py = v; py > 0; py--)
					{
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					pField[px] = 0;
				}
			}

			vLines.clear();
		}

        //Draw Current Piece
        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++)
            {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                    screen[(nCurrentY + py + nDrawOffset) * nScreenWidth + (nCurrentX + px + nDrawOffset)] = nCurrentPiece + 65;
            }
		
		//Draw Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        //Display frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

    }
	//GAME OVER
	CloseHandle(hConsole);
	cout << "GAME OVER! SCORE: " << nScore << "\n";
	system("pause");
}

int Rotate(int px, int py, int r) 
{
    switch (r % 4) {
    case 0: return py * 4 + px;			// 0 degrees
    case 1: return 12 + py - px * 4;	// 90 degrees
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
                if (nPosY + py > 0 && nPosY+ py <= nFieldHeight)
                {
                    if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
                        return false; //fail, piece can't exist on a non-empty space on the field 
                }
            }
        }

    return true;
}
