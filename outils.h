#ifndef OUTILS_H 
#define OUTILS_H 

// regroupement des biblioth�ques utiles dans le projet 
#include <cstdio> 
#include<cstdlib> 
#include <conio.h> 

// biblioth�ques n�cessaires pour outils.h 
#include<ctime> 
#include <iostream> 
#include<Windows.h> 
#include <string> 
#include <exception> 
using namespace std;

// �ventuellement possibilit� de r�partir le code  
// sur plusieurs fichiers 
namespace outils
{
	HANDLE O_GetStdout();
	void O_ConsoleResize(int& width, int& height);
	COORD O_ConsoleMinSize(HANDLE h);
	int O_Top(int dur);
	void O_Gotoxy(int x, int y);
	void O_Textcolor(int color);
	void O_ConsoleCursor(int val);
	void O_Blit(CHAR_INFO* dat, SMALL_RECT* datsrc, SMALL_RECT* consdest);
	void O_ClearToColor(int color, char lettre);
}

namespace outils
{
	/**************************************************
	ConsoleGetStdout fonction
	R�cup�re le handle pour les sorties sur la fen�tre
	console
	*/
	HANDLE O_GetStdout()
	{
		static HANDLE h = nullptr;
		if (h == nullptr) {
			h = GetStdHandle(STD_OUTPUT_HANDLE);
			if (h == INVALID_HANDLE_VALUE)
				throw exception("Error GetStdHandle");
		}
		return h;
	}
	/**************************************************
	ConsoleResize fonction
	redimensionne la fen�tre console, partie visible et
	partie m�moire sont ajust�es.
	*/
	void O_ConsoleResize(int& width, int& height)
	{
		HANDLE h = O_GetStdout();

		// r�cup�ration des tailles maximum et minimum    
		// support�es par la console 
		COORD max = GetLargestConsoleWindowSize(h);
		COORD min = O_ConsoleMinSize(h);
		width = (width > max.X) ? max.X :
			((width < min.X) ? min.X : width);
		height = (height > max.Y) ? max.Y :
			((height < min.Y) ? min.Y : height);

		CONSOLE_SCREEN_BUFFER_INFO info;
		if (!GetConsoleScreenBufferInfo(h, &info))
			throw exception("Error "
				"GetConsoleScreenBufferInfo");

		// si la hauteur  demand�e est inf�rieure � la  
		// hauteur actuelle 
		if (height < info.dwSize.Y) {

			// diminuer d�abord le rectangle fen�tre 
			info.srWindow.Bottom = height - 1;
			if (!SetConsoleWindowInfo(h, TRUE, &info.srWindow))
				throw exception("Error "
					"SetConsoleWindowInfo");

			// ensuite le buffer correspondant 
			info.dwSize.Y = height;
			if (!SetConsoleScreenBufferSize(h, info.dwSize))
				throw exception("Error "
					"SetConsoleScreenBufferSize");

		}
		// si la taille demand�e est sup�rieure �  
		// la taille actuelle 
		else if (height > info.dwSize.Y) {
			// d�abord augmenter la taille du buffer 
			info.dwSize.Y = height;
			if (!SetConsoleScreenBufferSize(h, info.dwSize))
				throw exception("Error "
					"SetConsoleScreenBufferSize");

			// ensuite le rectangle correspondant de la fen�tre 
			info.srWindow.Bottom = height - 1;
			if (!SetConsoleWindowInfo(h, TRUE, &info.srWindow))
				throw exception("Error SetConsoleWindowInfo");
		}
		// idem pour la largeur 
		if (width < info.dwSize.X) {
			info.srWindow.Right = width - 1;
			if (!SetConsoleWindowInfo(h, TRUE, &info.srWindow))
				throw exception("Error SetConsoleWindowInfo");

			info.dwSize.X = width;
			if (!SetConsoleScreenBufferSize(h, info.dwSize))
				throw exception("Error "
					"SetConsoleScreenBufferSize");

		}
		else if (width > info.dwSize.X) {
			info.dwSize.X = width;
			if (!SetConsoleScreenBufferSize(h, info.dwSize))
				throw exception("Error "
					"SetConsoleScreenBufferSize");

			info.srWindow.Right = width - 1;
			if (!SetConsoleWindowInfo(h, TRUE, &info.srWindow))
				throw exception("Error "
					"SetConsoleWindowInfo");
		}
	}
	/**************************************************
   ConsoleMinSize fonction
   R�cup�re une taille minimum pour la fen�tre console.
   Le principe n�est pas tout � fait juste car la mesure
   minimum de la fen�tre est donn�e en pixels alors que
   celle des lettres ne l�est pas. Cependant le r�sultat
   obtenu est acceptable.
   */
	COORD O_ConsoleMinSize(HANDLE h)
	{
		// taille min de la fen�tre en pixel 
		COORD wsize;
		wsize.X = GetSystemMetrics(SM_CXMIN);
		wsize.Y = GetSystemMetrics(SM_CYMIN);

		// taille des caract�res en pixel 
		CONSOLE_FONT_INFO font;
		COORD fsize;
		GetCurrentConsoleFont(h, TRUE, &font);
		fsize = GetConsoleFontSize(h, font.nFont);

		// nombre de caract�res minimum  
		COORD min = { wsize.X / fsize.X , wsize.Y / fsize.Y };
		return min;
	};
	/**************************************************
   chronom�trage des espaces de temps pour controler la
   vitesse des animations
   */
	int O_Top(int dur)
	{
		static int start = 0;
		int res = 0;
		if (clock() > start + dur) {
			start = clock();
			res = 1;
		}
		return res;
	}
	/**************************************************
	d�placer le curseur en �criture
	*/
	void O_Gotoxy(int x, int y)
	{
		COORD c;
		c.X = x;
		c.Y = y;
		SetConsoleCursorPosition(O_GetStdout(), c);
	}
	/**************************************************
	donner une couleur (lettre et fond simultan�ment)
	*/
	void O_Textcolor(int color)
	{
		SetConsoleTextAttribute(O_GetStdout(), color);
	}
	/**************************************************
   Masquer le curseur en �criture lors des affichages
   - invisible avec val = FALSE (0),
   - visible avec val = TRUE (ou autre valeur que 0)
   */
	void O_ConsoleCursor(int val)
	{
		CONSOLE_CURSOR_INFO cursorInfo;

		GetConsoleCursorInfo(O_GetStdout(), &cursorInfo);
		cursorInfo.bVisible = val; //  
		SetConsoleCursorInfo(O_GetStdout(), &cursorInfo);
	}
	/**************************************************
	Afficher directement dans la console un block dat de
	CHAR_INFO depuis le rectangle source pris dans dat et �
	destination du rectangle dest visible dans la console.
	Les champs right et bottom correspondent respectivement
	� la taille de la largeur et celle de la hauteur.
	*/
	void O_Blit(CHAR_INFO* dat, SMALL_RECT* datsrc,
		SMALL_RECT* consdest)
	{
		COORD taille = { datsrc->Right,datsrc->Bottom };
		COORD pos = { datsrc->Left, datsrc->Top };
		if (!WriteConsoleOutput(O_GetStdout(), dat, taille,
			pos, consdest))
			throw exception("Error WriteConsoleOutput");
	}
	/**************************************************
  Fonction pour remplir la fen�tre console avec une couleur
  et une lettre. La couleur couvre � la fois le fond (back :
  4 bits de gauche) et la lettre (le for : les quatre bits
  de droite)
  */
	void O_ClearToColor(int color, char lettre)
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		if (!GetConsoleScreenBufferInfo(O_GetStdout(), &info))
			throw exception("Error GetConsoleScreenBufferInfo");

		int nb = info.dwSize.X * info.dwSize.Y;
		CHAR_INFO* dat = new CHAR_INFO[nb];
		for (int i = 0; i < nb; i++) {
			dat[i].Attributes = color;
			dat[i].Char.AsciiChar = lettre;
		}

		SMALL_RECT src = { 0 };
		src.Right = info.srWindow.Right + 1;
		src.Bottom = info.srWindow.Bottom + 1;

		SMALL_RECT dest = src;
		O_Blit(dat, &src, &dest);
	}
}
#endif

