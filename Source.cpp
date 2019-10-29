#include <stdio.h>
#include "outils.h"
using namespace std;
using namespace outils;

int TX = 0;
int TY = 0;


struct Cell
{
	bool val; //Valeur 0/1 de la cellule
	float color;
};

Cell** PLAN = nullptr;
Cell** MIROIR = nullptr;


void reset(void);
void dimension_console(void);
void create_matrices(void);
void destroy(void);
int  compte_voisins(int y, int x);
void calcul(void);
void copie(void);
void affiche(void);
void mem_affiche(void);

/**********************************************************/
int main(int argc, char* argv[])
{
	int fin = 0;
	int ralenti = 100;
	O_ConsoleCursor(false);

	reset();
	while (!fin)
	{
		if (_kbhit())
		{
			switch (_getch())
			{
			case VK_ESCAPE: fin = 1; break;
			case VK_RETURN: reset(); break;
			case 'p': ralenti += 10; break;
			case 'm': ralenti -= 10; break;
			}
		}
		if (O_Top(ralenti))
		{
			mem_affiche();
			calcul();
			copie();
		}
	}
	destroy();
	return 0;
}
/**********************************************************/

void reset()
{
	destroy();
	dimension_console();
	create_matrices();
}

void destroy()
{
	if (PLAN == nullptr && MIROIR == nullptr)
		return;
	for (int y = 0; y < TY; y++)
	{
		delete[] PLAN[y];
		delete[] MIROIR[y];
	}
}

void dimension_console()
{
	O_Textcolor(15);
	O_Gotoxy(0, 0);

	cout << "Entrez la largeur de la fenêtre :" << endl;
	cin >> TX;

	cout << "Entrez la hauteur de la fenêtre :" << endl;
	cin >> TY;

	//Taille min
	TX = (TX < 10) ? 10 : TX;
	TY = (TY < 10) ? 10 : TY;

	O_ConsoleResize(TX, TY);
}

void create_matrices()
{
	PLAN = new Cell* [TY];
	MIROIR = new Cell* [TY];
	for (int y = 0; y < TY; y++)
	{
		PLAN[y] = new Cell[TX];
		MIROIR[y] = new Cell[TX];
		memset(PLAN[y], 0, sizeof(Cell) * TX);
		memset(MIROIR[y], 0, sizeof(Cell) * TX);
	}

	//Création d'un carré au milieu
	PLAN[TY / 2][TX / 2].val = 1;
	PLAN[TY / 2 + 1][TX / 2].val = 1;
	PLAN[TY / 2][TX / 2 + 1].val = 1;
	PLAN[TY / 2 + 1][TX / 2 + 1].val = 1;

	PLAN[TY / 2][TX / 2].color = 1;
	PLAN[TY / 2 + 1][TX / 2].color = 1;
	PLAN[TY / 2][TX / 2 + 1].color = 1;
	PLAN[TY / 2 + 1][TX / 2 + 1].color = 1;
}

void calcul()
{
	int nb_voisins;

	for (int y = 0; y < TY; y++)
	{
		for (int x = 0; x < TX; x++)
		{
			nb_voisins = compte_voisins(y, x);
			if (nb_voisins < 2 || nb_voisins > 3)
			{
				MIROIR[y][x].val = 0;
				MIROIR[y][x].color = PLAN[y][x].color;
			}
			else
			{
				MIROIR[y][x].val = 1;
				if (PLAN[y][x].color == 0)
				{
					MIROIR[y][x].color = 0.7f;
				}
				else if (PLAN[y][x].color < 15)
				{
					MIROIR[y][x].color = PLAN[y][x].color + 0.1f;
				}
			}
		}
	}
}

int compte_voisins(int y, int x)
{
	int nb = 0;
	int yn, ys, xo, xe;

	yn = (y - 1 + TY) % TY;
	ys = (y + 1) % TY;
	xo = (x - 1 + TX) % TX;
	xe = (x + 1) % TX;

	if (PLAN[y][xe].val == 1)
		nb++;
	if (PLAN[yn][xe].val == 1)
		nb++;
	if (PLAN[yn][x].val == 1)
		nb++;
	if (PLAN[yn][xo].val == 1)
		nb++;
	if (PLAN[y][xo].val == 1)
		nb++;
	if (PLAN[ys][xo].val == 1)
		nb++;
	if (PLAN[ys][x].val == 1)
		nb++;
	if (PLAN[ys][xe].val == 1)
		nb++;

	return nb;
}

void copie()
{
	for (int y = 0; y < TY; y++)
	{
		memcpy(PLAN[y], MIROIR[y], sizeof(Cell) * TY);
	}
}

void mem_affiche()
{
	CHAR_INFO* dat = new CHAR_INFO[TX * TY];
	for (int y = 0; y < TY; y++) {
		for (int x = 0; x < TX; x++) {
			WORD color = (WORD)PLAN[y][x].color;
			// attention : multiplier par 16 pour  
			// passer à la couleur de fond 
			dat[y * TX + x].Attributes = color * 16;
			dat[y * TX + x].Char.AsciiChar = ' ';
		}
	}
	// casts pour SMALL_RECT ensuite 
	SHORT l = 0, t = 0, r = TX, b = TY;
	SMALL_RECT src = { l,t,r,b }, dest = src;
	O_Blit(dat, &src, &dest);

	delete[] dat;
}

void affiche()
{
	for (int y = 0; y < TY; y++) {
		for (int x = 0; x < TX; x++) {
			O_Textcolor(((int)PLAN[y][x].color) * 16);
			O_Gotoxy(x, y);
			putchar(' ');
		}
	}
}