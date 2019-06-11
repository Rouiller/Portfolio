#include <iostream>
#include <vector>
#include <thread>
using namespace std;

#include <Windows.h>

wstring tetromino[7];
int fieldWidth = 12;
int fieldHeight = 18;
unsigned char* field = nullptr;


int screenWidth = 120;
int screenHeight = 40;


/* Clockwise Rotation

	  0°		   90°			 180°		   270°
+--+--+--+--+ +--+--+--+--+ +--+--+--+--+ +--+--+--+--+
|00|01|02|03| |12|08|04|00| |15|14|13|12| |03|07|11|15|
+--+--+--+--+ +--+--+--+--+ +--+--+--+--+ +--+--+--+--+
|04|05|06|07| |13|09|05|01| |11|10|09|08| |02|06|10|14|
+--+--+--+--+ +--+--+--+--+ +--+--+--+--+ +--+--+--+--+
|08|09|10|11| |14|10|06|02| |07|06|05|04| |01|05|09|13|
+--+--+--+--+ +--+--+--+--+ +--+--+--+--+ +--+--+--+--+
|12|13|14|15| |15|11|07|03| |03|02|01|00| |00|04|08|12|
+--+--+--+--+ +--+--+--+--+ +--+--+--+--+ +--+--+--+--+

*/

int Rotate(int x, int y, int r)
{
	switch (r % 4)
	{
	case 0: return y * 4 + x;
	case 1: return 12 + y - (x * 4);
	case 2: return 15 - (y * 4) - x;
	case 3: return 3 - y + (x * 4);
	}

	return 0;
}


bool DoesPiecefit(int tetrominoType, int rotation, int posX, int posY)
{
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			int pi = Rotate(px, py, rotation);

			int fi = (posY + py) * fieldWidth + (posX + px);

			if (posX + px >= 0 && posX + px < fieldWidth)
			{
				if (posY + py >= 0 && posY + py < fieldHeight)
				{
					if (tetromino[tetrominoType][pi] == L'X' && field[fi] != 0 )
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}


int main()
{
	// Screen buffer
	wchar_t* screen = new wchar_t[screenWidth * screenHeight];
	for (int i = 0; i < screenWidth * screenHeight; i++) { screen[i] = L' '; }
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;

	// Assets 
	tetromino[0].append(L"X...");
	tetromino[0].append(L"X...");
	tetromino[0].append(L"X...");
	tetromino[0].append(L"X...");

	tetromino[1].append(L".X..");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"XX..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"....");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"XX..");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".X..");
	tetromino[5].append(L"XXX.");
	tetromino[5].append(L"....");

	tetromino[6].append(L"....");
	tetromino[6].append(L"XX..");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L"....");

	// Game playing field
	field = new unsigned char[fieldWidth * fieldHeight];
	for (int x = 0; x < fieldWidth; x++)
	{
		for (int y = 0; y < fieldHeight; y++)
		{
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;
		}
	}

	// Game variables
	bool gameOver = false;

	int currentPiece = 6;
	int currentRotation = 0;
	int currentX = fieldWidth / 2;
	int currentY = 0;

	bool rotateHold = false;
	bool key[4];

	int speed = 20;
	int speedCounter = 0;
	bool forceDown = false;
	int pieceCount = 0;
	int score = 0;

	vector<int> lines;

	// Game loop
	while (!gameOver)
	{
		// Game timing
		this_thread::sleep_for(50ms);
		speedCounter++;
		forceDown = (speedCounter == speed);

		// Input
		for (int k = 0; k < 4; k++)
		{													   // R    L   D Z
			key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		// Game logic
		currentX += (key[0] && DoesPiecefit(currentPiece, currentRotation, currentX + 1, currentY)) ? 1 : 0;
		currentX -= (key[1] && DoesPiecefit(currentPiece, currentRotation, currentX - 1, currentY)) ? 1 : 0;
		currentY += (key[2] && DoesPiecefit(currentPiece, currentRotation, currentX, currentY + 1)) ? 1 : 0;

		if (key[3])
		{
			currentRotation += (!rotateHold && DoesPiecefit(currentPiece, currentRotation + 1, currentX, currentY)) ? 1 : 0;
			rotateHold = true;
		}
		else
		{
			rotateHold = false;
		}

		if (forceDown)
		{
			if (DoesPiecefit(currentPiece, currentRotation, currentX, currentY + 1))
			{
				currentY++;
			}
			else
			{
				// Lock piece
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[currentPiece][Rotate(px, py, currentRotation)] == L'X')
						{
							field[(currentY + py) * fieldWidth + (currentX + px)] = currentPiece + 1;
						}
					}
				}

				pieceCount++;
				if (pieceCount % 10 == 0)
				{
					if (speed >= 10)
					{
						speed--;
					}
				}

				// Check full lines
				for (int py = 0; py < 4; py++)
				{
					if (currentY + py < fieldHeight - 1)
					{
						bool line = true;
						for (int px = 1; px < fieldWidth - 1; px++)
						{
							line &= (field[(currentY + py * fieldWidth + px)]) != 0;
						}
						
						if (line)
						{
							for (int px = 1; px < fieldWidth - 1; px++)
							{
								field[(currentY + py) * fieldWidth + px] = 8;
							}

							lines.push_back(currentY + py);
						}
					}
				}

				score += 25;
				if (!lines.empty())
				{
					score += (1 << lines.size()) * 100;
				}

				// Choose next piece
				currentX = fieldWidth / 2;
				currentY = 0;
				currentRotation = 0;
				currentPiece = rand() % 7;

				// If piece does not fit
				gameOver = !DoesPiecefit(currentPiece, currentRotation, currentX, currentY);
			}

			speedCounter = 0;
		}

		// Render 
		// Field
		for (int x = 0; x < fieldWidth; x++)
		{
			for (int y = 0; y < fieldHeight; y++)
			{
				screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[field[y * fieldWidth + x]];
				
			}
		}
		// Current piece
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				if (tetromino[currentPiece][Rotate(px, py, currentRotation)] == L'X')
				{
					// + 65 to make letters ABCDEFG
					screen[(currentY + py + 2) * screenWidth + (currentX + px + 2)] = currentPiece + 65;
				}				
			}
		}
		// Score
		swprintf(&screen[2 * screenWidth + fieldWidth + 6], 16, L"SCORE: %8d", score);


		if (!lines.empty())
		{
			WriteConsoleOutputCharacter(console, screen, screenWidth* screenHeight, { 0, 0 }, & bytesWritten);
			this_thread::sleep_for(400ms);

			for (auto& v : lines)
			{
				for (int px = 1; px < fieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
					{
						field[py * fieldWidth + px] = field[(py - 1) * fieldWidth + px];
					}
					field[px] = 0;
				}
			}
		}

		WriteConsoleOutputCharacter(console, screen, screenWidth * screenHeight, { 0, 0 }, &bytesWritten);

	}

	CloseHandle(console);
	cout << "Game Over!! Score:" << score << endl;
	system("pause");
}