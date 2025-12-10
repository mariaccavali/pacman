#include <iostream>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <cmath>
#include <string>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iomanip>
using namespace std;

// Função para mudar cor no console
void setColor(int corTexto, int corFundo = 0) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, corFundo * 16 + corTexto);
}

const int ROWS = 22;
const int COLS = 25;
const int FRAME_MS = 40;
const int GHOST_MOVE_MS = 160;
const unsigned long GHOST_RELEASE_DELAY = 2000;
const int PACMAN_MOVE_MS = 100;
const int PAC_START_X = 9;
const int PAC_START_Y = 0;



bool gameStarted = false;
bool showGhostMsg = false;
unsigned long ghostMsgStart = 0;
const unsigned long GHOST_MSG_DURATION = 2000;

const int MAP_SCREEN_Y = 2;

int m[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1},
    {2,2,2,2,2,1,0,1,0,0,0,0,0,0,0,1,0,1,2,2,2,2,2,2,2},
    {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
    {2,2,2,2,2,1,0,1,0,1,1,1,1,1,0,1,0,1,2,2,2,2,2,2,2},
    {1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,1,0,1,1,1,0,1,0,1,1,1,1,0,1,1,1,1,1,0,1},
    {1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,0,0,0,1,0,1,1,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// pellets: 0 = none, 1 = normal, 2 = power (frightened), 3 = speed, 4 = freeze, 5 = shield
int pellets[ROWS][COLS];

void place_special_pellets_auto() {

    // Mínimo de distância Manhattan entre Pacman e qualquer power-up especial
    const int MIN_DIST = 6;

    vector<pair<int,int>> freeCells;

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (m[i][j] == 0) {

                int dist = abs(i - PAC_START_X) + abs(j - PAC_START_Y);

                // Ignorar posições muito próximas do spawn do Pacman
                if (dist < MIN_DIST) continue;

                freeCells.emplace_back(i,j);
            }
        }
    }

    if (freeCells.size() >= 6) {
        int n = (int)freeCells.size();

        // PICK SPEED
        auto p = freeCells[n/6];
        if (pellets[p.first][p.second] == 1)
            pellets[p.first][p.second] = 3;

        // PICK FREEZE — AGORA SEMPRE LONGE DO PACMAN
        p = freeCells[n/3];
        if (pellets[p.first][p.second] == 1)
            pellets[p.first][p.second] = 4;

        // PICK SHIELD
        p = freeCells[n/2];
        if (pellets[p.first][p.second] == 1)
            pellets[p.first][p.second] = 5;
    }
}

void fasesFacil(int fase, int m[ROWS][COLS])
{
    static int M1[ROWS][COLS] = {
        // FÁCIL  - fase 1 (área central curta, bordas com paredes)
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,0,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,0,1,1,1,1,1,0,1,0,1,2,2,2,2,2,2,2},
        {1,0,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,1,1,0,1,0,1,1,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1},
        {1,0,1,0,0,1,0,1,1,1,1,1,1,1,1,1,0,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2}

    };

    static int M2[ROWS][COLS] = {
        // FÁCIL - fase 2 (um pouco mais complexo, mas ainda "pequeno")
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},

        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},

        {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},

        {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,0,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,1,0,1,0,0,0,0,0,0,0,1,0,1,0,1,1,1,1,0,1},
        {1,0,1,0,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,1,0,1},
        {1,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1},
        {1,0,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2}

    };

    static int M3[ROWS][COLS] = {
        // FÁCIL - fase 3 (ligeiramente maior que a 1, mas ainda fácil)
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},

        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},

        {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},

        {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,1,0,1,0,0,1,0,1,1,1,1,1,0,1,0,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},


        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2}
    };

    // copia para m
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j) {
            if (fase == 1) m[i][j] = M1[i][j];
            else if (fase == 2) m[i][j] = M2[i][j];
            else if (fase == 3) m[i][j] = M3[i][j];
        }
}

void fasesMedio(int fase, int m[ROWS][COLS])
{
    static int M1[ROWS][COLS] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1},
        {2,2,2,2,2,1,0,1,0,0,0,0,0,0,0,1,0,1,2,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {2,2,2,2,2,1,0,1,0,1,1,1,1,1,0,1,0,1,2,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,0,0,0,1,0,1,1,1,0,1,0,1,1,1,1,0,1,1,1,1,1,0,1},
        {1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,0,0,0,1,0,1,1,1},
        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    static int M2[ROWS][COLS] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1},
        {2,2,2,2,2,1,0,1,0,0,0,0,0,0,0,1,0,1,2,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {2,2,2,2,2,1,0,1,0,1,1,1,1,1,0,1,0,1,2,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,0,0,0,1,0,1,1,1,0,1,0,1,1,1,1,0,1,1,1,1,1,0,1},
        {1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,0,0,0,1,0,1,1,1},
        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    static int M3[ROWS][COLS] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1},
        {1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,1,1,1,1,1,1},
        {2,2,2,2,2,1,0,1,0,0,0,0,0,0,0,1,0,1,2,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {2,2,2,2,2,1,0,1,0,1,1,1,1,1,0,1,0,1,2,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1},
        {1,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,1},
        {1,0,1,0,1,1,0,1,0,0,0,0,0,1,0,1,0,1,0,1,0,1,1,0,1},
        {1,0,1,0,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    // Copia a fase desejada para m
    for(int i = 0; i < ROWS; i++)
        for(int j = 0; j < COLS; j++)
        {
            if (fase == 1) m[i][j] = M1[i][j];
            else if (fase == 2) m[i][j] = M2[i][j];
            else if (fase == 3) m[i][j] = M3[i][j];
        }
}

void fasesDificil(int fase, int m[ROWS][COLS])
{
    static int M1[ROWS][COLS] = {
        // DIFÍCIL - fase 1 (mais caminhos, menos paredes)
               {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {2,2,2,2,2,1,0,1,0,0,0,0,0,0,0,1,0,1,2,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {2,2,2,2,2,1,0,1,0,1,1,1,1,1,0,1,0,1,2,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

    static int M2[ROWS][COLS] = {
        // DIFÍCIL - fase 2 (labirinto mais sinuoso e pontos inacessíveis)
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1},
        {1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,0,0,0,1,0,1,1,1,0,1,1,0,1,0,1,0,1},
        {1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,1},
        {1,0,0,0,0,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1},
        {1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,0,0,1,1,0,1,1,1},
        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    static int M3[ROWS][COLS] = {
        // DIFÍCIL - fase 3 (labirinto bem mais aberto mas com cantos inacessíveis)
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1},
        {1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    // Copia para m
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j) {
            if (fase == 1) m[i][j] = M1[i][j];
            else if (fase == 2) m[i][j] = M2[i][j];
            else if (fase == 3) m[i][j] = M3[i][j];
        }
}

void colocarPowerPellets(int m[ROWS][COLS], int pellets[ROWS][COLS]) {
    int cont = 0;
    for (int i = 0; i < ROWS && cont < 4; ++i) {
        for (int j = 0; j < COLS && cont < 4; ++j) {
            if (m[i][j] == 0) {       // posição livre
                pellets[i][j] = 2;     // coloca power pellet
                cont++;
            }
        }
    }
}


void posXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

string getDateTime();

struct Jogador {
    string nome = "Jogador";
    int pontos = 0;
    string data = "";
    string tempo = "";

    Jogador() = default;

    Jogador(const string& _nome, int _pontos, const string& _data, const string& _tempo)
        : nome(_nome), pontos(_pontos), data(_data), tempo(_tempo) {}

    bool operator<(const Jogador &outro) const {
        if (pontos != outro.pontos)
            return pontos > outro.pontos;
        return nome < outro.nome;
    }

    Jogador operator+(const Jogador &outro) const {
        Jogador soma;
        soma.nome = nome + " & " + outro.nome;
        soma.pontos = pontos + outro.pontos;
        soma.data = getDateTime();
        soma.tempo = "—";
        return soma;
    }
};

string getDateTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", ltm);
    return string(buffer);
}

void salvarPontos(const Jogador &j) {
    ofstream file("rank.txt", ios::app);
    if (file.is_open()) {
        file << j.nome << "," << j.pontos << "," << j.data << "," << j.tempo << "\n";
        file.close();
    }
}

void showRanking() {
    ifstream file("rank.txt");
    vector<Jogador> scores;

    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            Jogador j;
            size_t p1 = line.find(',');
            size_t p2 = line.find(',', p1 + 1);
            size_t p3 = line.find(',', p2 + 1);

            if (p1 != string::npos && p2 != string::npos && p3 != string::npos) {
                j.nome = line.substr(0, p1);
                j.pontos = stoi(line.substr(p1 + 1, p2 - p1 - 1));
                j.data = line.substr(p2 + 1, p3 - p2 - 1);
                j.tempo = line.substr(p3 + 1);
                scores.push_back(j);
            }
        }
        file.close();
    }

    sort(scores.begin(), scores.end(),
        [](const Jogador &a, const Jogador &b) {
            if (a.pontos != b.pontos)
                return a.pontos > b.pontos;
            return a.nome < b.nome;
        });

    cout << "\n========================= RANKING ========================\n";
    cout << left << setw(15) << "Nome"
         << setw(10) << "Score"
         << setw(22) << "Data"
         << "Tempo\n";

    cout << "----------------------------------------------------------\n";

    for (auto &s : scores) {
        cout << left << setw(15) << s.nome
             << setw(10) << s.pontos
             << setw(22) << s.data
             << s.tempo << "\n";
    }

    cout << "==========================================================\n\n";
}

const int dx[4] = {-1, 0, 1, 0};
const int dy[4] = {0, 1, 0, -1};
const int priorityOrder[4] = {0, 3, 2, 1};

enum Mode { SCATTER = 0, CHASE = 1, FRIGHTENED = 2 };

inline bool is_free(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
    return m[r][c] == 0;
}

inline int clamp_r(int v) {
    if (v < 0) return 0;
    if (v >= ROWS) return ROWS - 1;
    return v;
}

inline int clamp_c(int v) {
    if (v < 0) return 0;
    if (v >= COLS) return COLS - 1;
    return v;
}

int count_options(int gx, int gy, int current_dir) {
    int cnt = 0;
    for (int d = 0; d < 4; d++) {
        if (d == (current_dir + 2) % 4) continue;
        int nx = gx + dx[d], ny = gy + dy[d];
        if (is_free(nx, ny)) cnt++;
    }
    return cnt;
}

bool need_decision(int gx, int gy, int current_dir) {
    int nx = gx + dx[current_dir], ny = gy + dy[current_dir];
    if (!is_free(nx, ny)) return true;
    return count_options(gx, gy, current_dir) > 1;
}

int choose_dir(int gx, int gy, int current_dir,
               int target_x, int target_y,
               bool force_reverse = false) {

    int best = -1;
    double bestDist = 1e18;

    for (int d = 0; d < 4; d++) {
        if (!force_reverse && d == (current_dir + 2) % 4)
            continue;

        int nx = gx + dx[d], ny = gy + dy[d];
        if (!is_free(nx, ny)) continue;

        double dist = (nx - target_x)*(nx - target_x) +
                      (ny - target_y)*(ny - target_y);

        if (dist < bestDist - 1e-9) {
            bestDist = dist;
            best = d;
        }
        else if (fabs(dist - bestDist) < 1e-9) {
            int pb = 4, pc = 4;
            for (int p = 0; p < 4; p++) {
                if (priorityOrder[p] == best) pb = p;
                if (priorityOrder[p] == d)    pc = p;
            }
            if (pc < pb) best = d;
        }
    }

    if (best == -1) {
        int rev = (current_dir + 2) % 4;
        int nx = gx + dx[rev], ny = gy + dy[rev];
        if (is_free(nx, ny)) best = rev;
    }

    return best;
}

// efeitos visuais
void drawCharOnMap(HANDLE out, int gridR, int gridC, char ch) {
    COORD p;
    p.X = gridC;
    p.Y = MAP_SCREEN_Y + gridR;
    SetConsoleCursorPosition(out, p);
    cout << ch << flush;
}

void eatEffect(HANDLE out, int gridR, int gridC) {
    const int frames = 6;
    const int frameMs = 80;
    const string msg = "+50";

    for (int f = 0; f < frames; ++f) {
        bool on = (f % 2 == 0);

        drawCharOnMap(out, gridR, gridC,
                      on ? msg[0] : ' ');
        drawCharOnMap(out, gridR, gridC + 1,
                      on ? msg[1] : ' ');
        drawCharOnMap(out, gridR, gridC + 2,
                      on ? msg[2] : ' ');

        Sleep(frameMs);
    }
    drawCharOnMap(out, gridR, gridC, ' ');
    drawCharOnMap(out, gridR, gridC + 1, ' ');
    drawCharOnMap(out, gridR, gridC + 2, ' ');
}

void deathEffect(HANDLE out, int gridR, int gridC) {
    const vector<string> seq = {"AI", "    ", "AI", "    "};
    const int frameMs = 140;

    for (const auto &s : seq) {
        int startC = gridC - 1;
        int r = gridR;

        for (int i = 0; i < 4; ++i) {
            int c = startC + i;
            char ch = (i < (int)s.size() ? s[i] : ' ');
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
                drawCharOnMap(out, r, c, ch);
        }
        Sleep(frameMs);
    }
}

int pontosDaFase(int dificuldade, int fase) {
    if (dificuldade == 1) { // Fácil
        if (fase == 1) return 100;
        if (fase == 2) return 200;
        if (fase == 3) return 300;
    }
    else if (dificuldade == 2) { // Médio
        if (fase == 1) return 500;
        if (fase == 2) return 600;
        if (fase == 3) return 700;
    }
    else if (dificuldade == 3) { // Difícil
        if (fase == 1) return 1000;
        if (fase == 2) return 2000;
        if (fase == 3) return 3000;
    }
    return 0;
}

int movimentos = 0;
int pontosPorMovimento = 1;  // pode mudar depois
enum PowerUpType { PU_NONE, PU_SPEED, PU_FREEZE, PU_SHIELD };
PowerUpType powerPending = PU_NONE;

bool showPowerHUD = false;

int main() {
    unsigned long startTime = GetTickCount();

    int houseX = 9;
    int houseY = 11;
    static unsigned long msgStart = 0;

    int pacX = PAC_START_X;
    int pacY = PAC_START_Y;


    if (!is_free(pacX, pacY)) {
        bool found = false;
        for (int r = 0; r < max(ROWS, COLS) && !found; r++) {
            for (int dxr = -r; dxr <= r && !found; dxr++) {
                for (int dyr = -r; dyr <= r && !found; dyr++) {
                    int nx = ROWS/2 + dxr;
                    int ny = COLS/2 + dyr;
                    if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS && is_free(nx, ny)) {
                        pacX = nx;
                        pacY = ny;
                        found = true;
                    }
                }
            }
        }
    }

    int pacDir = 1;
    int bx = houseX,      by = houseY,      bdir = 1;
    int px = houseX,      py = houseY + 1,  pdir = 1;
    int ix = houseX + 1,  iy = houseY,      idir = 1;
    int cx = houseX + 1,  cy = houseY + 1,  cdir = 1;

    bool ghostsLocked = true;
    unsigned long ghostsReleaseTime = GetTickCount() + GHOST_RELEASE_DELAY;

    const int scatter_bx = 0,       scatter_by = COLS - 1;
    const int scatter_px = 0,       scatter_py = 0;
    const int scatter_ix = ROWS - 1, scatter_iy = COLS - 1;
    const int scatter_cx = ROWS - 1, scatter_cy = 0;

    const int modeDurationsCount = 6;
    int modeDurations[modeDurationsCount] = {
        7000, 20000, 7000, 20000, 5000, 99999999
    };

    int currentModeIndex = 0;
    Mode ghostsMode = SCATTER;
    unsigned long modeStartTime = GetTickCount();

    bool frightenedActive = false;
    Mode prevMode = SCATTER;
    unsigned long frightenedStart = 0;
    const unsigned long frightenedDuration = 6000;

    // power-up states
    bool speedActive = false;
    unsigned long speedStart = 0;

    bool freezeActive = false;
    unsigned long freezeStart = 0;

    bool shieldActive = false;
    string activePower = "";
    unsigned long shieldStart = 0;

    const unsigned long SPEED_DURATION_MS  = 10000;
    const unsigned long FREEZE_DURATION_MS = 10000;
    const unsigned long SHIELD_DURATION_MS = 15000;

    int pontos = 0;
    int vidas = 5;

    srand((unsigned)time(NULL));

    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cinfo;

    GetConsoleCursorInfo(out, &cinfo);
    cinfo.bVisible = false;
    SetConsoleCursorInfo(out, &cinfo);

    COORD coord;
    coord.X = 0;
    coord.Y = 0;

    string buffer;
    buffer.reserve((ROWS+6) * (COLS+6));

    char tecla;
    int dificuldade;
    unsigned long lastGhostMove = GetTickCount();
    unsigned long lastPacMove = GetTickCount();

    system("cls");

    string nome;
    cout << "Digite o nome do jogador: ";
    getline(cin, nome);

    if (nome.empty()) nome = "Jogador";

    system("cls");
    cout << "================================\n";
    cout << "| Qual o nivel de dificuldade? |\n";
    cout << "| 1 - Facil                    |\n";
    cout << "| 2 - Medio                    |\n";
    cout << "| 3 - Dificil                  |\n";
    cout << "================================\n";
    cin >> dificuldade;

    int faseAtual = 1;

    if (dificuldade == 1)
        fasesFacil(faseAtual, m);
    else if (dificuldade == 2)
        fasesMedio(faseAtual, m);
    else
        fasesDificil(faseAtual, m);

    // --- inicializar pellets com a fase escolhida
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            pellets[i][j] = (m[i][j] == 0 ? 1 : 0);
        }
    }

    // --- POWER PELLETS PARA CADA DIFICULDADE
    if (dificuldade == 1) {
        // percorre o mapa e coloca power pellets nos cantos livres
        if (m[5][1] == 0) pellets[5][1] = 2;
        if (m[16][1] == 0) pellets[16][1] = 2;
        if (m[5][23] == 0) pellets[5][23] = 2;
        if (m[16][23] == 0) pellets[16][23] = 2;
    } else {
        // níveis médio/difícil → sistema original
        if (m[1][1] == 0) pellets[1][1] = 2;
        if (m[1][COLS-2] == 0) pellets[1][COLS-2] = 2;
        if (m[ROWS-2][1] == 0) pellets[ROWS-2][1] = 2;
        if (m[ROWS-2][COLS-2] == 0) pellets[ROWS-2][COLS-2] = 2;
    }

    // --- coloca power-ups especiais automaticamente
    place_special_pellets_auto();



    // --- RESET COMPLETO DOS POWERS (CORRIGE O FREEZE AUTOMÁTICO) ---
    powerPending = PU_NONE;
    speedActive = false;
    freezeActive = false;
    shieldActive = false;

    speedStart = 0;
    freezeStart = 0;
    shieldStart = 0;

    activePower = "";
    showPowerHUD = false;
    msgStart = 0;

    // coloca power-ups especiais automaticamente
    place_special_pellets_auto();

    system("cls");
    cout << "Bem-vindo, " << nome << "! O jogo vai comecar...\n";
    Sleep(1000);
    system("cls");

    gameStarted = true;
    showGhostMsg = true;
    ghostMsgStart = GetTickCount();

    ghostsLocked = true;
    unsigned long ghostsStartAfterMsg = ghostMsgStart + GHOST_MSG_DURATION;
    ghostsReleaseTime = ghostsStartAfterMsg;

//    static unsigned long msgStart = 0;
    const unsigned long MSG_DURATION = 1500;
    // Ajustado: deslocar todas as mensagens uma linha para baixo (para não sobrescrever o mapa)
    const int MSG_LINE = ROWS + 3;

    while (true) {
        unsigned long now = GetTickCount();
        // liberar fantasmas
        if (ghostsLocked && now >= ghostsReleaseTime) {
            ghostsLocked = false;
            if (m[8][11] == 1) m[8][11] = 0;

            lastGhostMove = now;
            msgStart = now;

            COORD avisoPos;
            avisoPos.X = 0;
            avisoPos.Y = MSG_LINE;
            SetConsoleCursorPosition(out, avisoPos);
            cout << "Fantasmas liberados!" << flush;
        }
        // modos SCATTER/CHASE
        if (!frightenedActive) {
            if (now - modeStartTime >= (unsigned long)modeDurations[currentModeIndex]) {
                currentModeIndex = min(currentModeIndex + 1, modeDurationsCount - 1);
                ghostsMode = (ghostsMode == SCATTER ? CHASE : SCATTER);
                bdir = (bdir + 2) % 4;
                pdir = (pdir + 2) % 4;
                idir = (idir + 2) % 4;
                cdir = (cdir + 2) % 4;
                modeStartTime = now;
            }
        }
        else {
            if (now - frightenedStart >= frightenedDuration) {
                frightenedActive = false;
                ghostsMode = prevMode;
                bdir = (bdir + 2) % 4;
                pdir = (pdir + 2) % 4;
                idir = (idir + 2) % 4;
                cdir = (cdir + 2) % 4;
                modeStartTime = now;
            }
        }

        // power-up timeouts
        if (speedActive && now - speedStart >= SPEED_DURATION_MS) {
            speedActive = false;
            showPowerHUD = false;

            // limpar a linha do HUD
            COORD pos;
            pos.X = 0;
            pos.Y = 1;   // <-- AQUI É A LINHA DO HUD (mude se precisar)
            SetConsoleCursorPosition(out, pos);
            cout << "                                      " << flush;
        }
        if (freezeActive && now - freezeStart >= FREEZE_DURATION_MS) {
            freezeActive = false;
            showPowerHUD = false;
            lastGhostMove = now;

            COORD pos;
            pos.X = 0;
            pos.Y = 1;
            SetConsoleCursorPosition(out, pos);
            cout << "                                      " << flush;
        }

        if (shieldActive && now - shieldStart >= SHIELD_DURATION_MS) {
            shieldActive = false;
            showPowerHUD = false;

            COORD pos;
            pos.X = 0;
            pos.Y = 1;
            SetConsoleCursorPosition(out, pos);
            cout << "                                      " << flush;
        }


        // input
        if (_kbhit()) {
            tecla = getch();
            if (tecla == 'w' || tecla == 'W' || tecla == 72) pacDir = 0;
            else if (tecla == 'd' || tecla == 'D' || tecla == 77) pacDir = 1;
            else if (tecla == 's' || tecla == 'S' || tecla == 80) pacDir = 2;
            else if (tecla == 'a' || tecla == 'A' || tecla == 75) pacDir = 3;
            else if (tecla == 'p' || tecla == 'P') {
                if (!frightenedActive) {
                    prevMode = ghostsMode;
                    frightenedActive = true;
                    frightenedStart = GetTickCount();
                    ghostsMode = FRIGHTENED;

                    bdir = (bdir + 2) % 4;
                    pdir = (pdir + 2) % 4;
                    idir = (idir + 2) % 4;
                    cdir = (cdir + 2) % 4;
                }
            }
        }

        // mover pacman (ajusta velocidade se speedActive)
        int effectivePacMs = speedActive ? (PACMAN_MOVE_MS / 2) : PACMAN_MOVE_MS;
        if (now - lastPacMove >= (unsigned long)effectivePacMs) {
            lastPacMove = now;

            int nx = pacX + dx[pacDir];
            int ny = pacY + dy[pacDir];

            if (ny < 0) ny = COLS - 1;
            else if (ny >= COLS) ny = 0;

            if (is_free(nx, ny)) {
                pacX = nx;
                pacY = ny;

                movimentos++;
            }
        }

        // comer bolinhas / power-ups
        int pelletHere = pellets[pacX][pacY];
        if (pelletHere == 1) {
            pontos += 1;
            pellets[pacX][pacY] = 0;
        }
        else if (pelletHere == 2) {
            // power tradicional -> frightened
            pontos += 5;
            pellets[pacX][pacY] = 0;

            if (!frightenedActive) {
                prevMode = ghostsMode;
                frightenedActive = true;
                frightenedStart = GetTickCount();
                ghostsMode = FRIGHTENED;

                bdir = (bdir + 2) % 4;
                pdir = (pdir + 2) % 4;
                idir = (idir + 2) % 4;
                cdir = (cdir + 2) % 4;
            } else {
                frightenedStart = GetTickCount();
            }
        }
        else if (pelletHere == 3) { // SPEED
            // impede coletar outro power se já existe um ativo ou pendente
            if (speedActive || freezeActive || shieldActive || powerPending != PU_NONE) {
                // ignora e não pega o novo power
               // pellets[pacX][pacY] = 0; // remove o pellet mesmo assim, se quiser
                continue;
            }

            pellets[pacX][pacY] = 0;
            pontos += 10;

            powerPending = PU_SPEED;
            showPowerHUD = false;
            speedActive = false;

            COORD avisoPos; avisoPos.X = 0; avisoPos.Y = MSG_LINE;
            SetConsoleCursorPosition(out, avisoPos);
            cout << "SPEED coletado! (+" << 10 << " pts)" << flush;

            msgStart = now;
        }

        else if (pelletHere == 4) { // FREEZE
            if (speedActive || freezeActive || shieldActive || powerPending != PU_NONE) {
                //pellets[pacX][pacY] = 0;
                continue;
            }

            pellets[pacX][pacY] = 0;
            pontos += 10;

            powerPending = PU_FREEZE;
            showPowerHUD = false;
            freezeActive = false;

            COORD avisoPos; avisoPos.X = 0; avisoPos.Y = MSG_LINE;
            SetConsoleCursorPosition(out, avisoPos);
            cout << "FREEZE coletado! (+10 pts)" << flush;

            msgStart = now;
        }

        else if (pelletHere == 5) { // SHIELD
            if (speedActive || freezeActive || shieldActive || powerPending != PU_NONE) {
                //pellets[pacX][pacY] = 0;
                continue;
            }

            pellets[pacX][pacY] = 0;
            pontos += 10;

            powerPending = PU_SHIELD;
            showPowerHUD = false;
            shieldActive = false;

            COORD avisoPos; avisoPos.X = 0; avisoPos.Y = MSG_LINE;
            SetConsoleCursorPosition(out, avisoPos);
            cout << "ESCUDO coletado! (+10 pts)" << flush;

            msgStart = now;
        }


        // targets
        int chase_bx = pacX;
        int chase_by = pacY;

        int ahead4_x = clamp_r(pacX + dx[pacDir] * 4);
        int ahead4_y = clamp_c(pacY + dy[pacDir] * 4);

        int chase_px = ahead4_x;
        int chase_py = ahead4_y;

        int inter_x = clamp_r(pacX + dx[pacDir] * 2);
        int inter_y = clamp_c(pacY + dy[pacDir] * 2);

        int chase_ix = clamp_r(bx + 2 * (inter_x - bx));
        int chase_iy = clamp_c(by + 2 * (inter_y - by));

        double distClyde = sqrt((cx - pacX)*(cx - pacX) + (cy - pacY)*(cy - pacY));
        int chase_cx = pacX;
        int chase_cy = pacY;

        if (distClyde < 8.0) {
            chase_cx = scatter_cx;
            chase_cy = scatter_cy;
        }

        // mover fantasmas (se não congelados)
        if (!ghostsLocked && !freezeActive && now - lastGhostMove >= (unsigned long)GHOST_MOVE_MS) {
            lastGhostMove = now;

            auto ghost_step = [&](int &gx, int &gy, int &gdir,
                                  int s_tx, int s_ty,
                                  int c_tx, int c_ty) {

                bool decide = need_decision(gx, gy, gdir);

                if (decide) {
                    if (frightenedActive) {
                        int cand[4], cnt = 0;
                        for (int d = 0; d < 4; d++) {
                            int nx = gx + dx[d], ny = gy + dy[d];
                            if (is_free(nx, ny)) cand[cnt++] = d;
                        }
                        if (cnt > 0) gdir = cand[rand() % cnt];
                    }
                    else {
                        int targetx = (ghostsMode == SCATTER ? s_tx : c_tx);
                        int targety = (ghostsMode == SCATTER ? s_ty : c_ty);

                        int nd = choose_dir(gx, gy, gdir, targetx, targety, false);
                        if (nd != -1) gdir = nd;
                    }
                }

                if (is_free(gx + dx[gdir], gy + dy[gdir])) {
                    gx += dx[gdir];
                    gy += dy[gdir];
                }
                else {
                    for (int d = 0; d < 4; d++) {
                        int nx = gx + dx[d], ny = gy + dy[d];
                        if (is_free(nx, ny)) {
                            gdir = d;
                            gx = nx;
                            gy = ny;
                            break;
                        }
                    }
                }
            };

            ghost_step(bx, by, bdir, scatter_bx, scatter_by, chase_bx, chase_by);
            ghost_step(px, py, pdir, scatter_px, scatter_py, chase_px, chase_py);
            ghost_step(ix, iy, idir, scatter_ix, scatter_iy, chase_ix, chase_iy);
            ghost_step(cx, cy, cdir, scatter_cx, scatter_cy, chase_cx, chase_cy);
        }

        // desenhar HUD
        buffer.clear();
        buffer += "Pontos: " + to_string(pontos) + "   Vidas: " + to_string(vidas) + "   Movimentos: " + to_string(movimentos) + "\n";
        buffer += "Power-up ativo: ";

        // Verifica qual power está ativo
        if (speedActive) {
            unsigned long left = (SPEED_DURATION_MS - (now - speedStart)) / 1000;
            if (left > 0) {
                buffer += "SPEED (" + to_string(left) + "s)";
            //} else {
            //    speedActive = false;
            }
        }
        else if (freezeActive) {
            unsigned long left = (FREEZE_DURATION_MS - (now - freezeStart)) / 1000;
            if (left > 0) {
                buffer += "FREEZE (" + to_string(left) + "s)";
            //} else {
              //  freezeActive = false;
                //lastGhostMove = now; // descongela
            }
        }
        else if (shieldActive) {
            unsigned long left = (SHIELD_DURATION_MS - (now - shieldStart)) / 1000;
            if (left > 0) {
                buffer += "ESCUDO (" + to_string(left) + "s)";
            //} else {
              //  shieldActive = false;
            }
        }

        buffer += "\n";   // sempre pula linha


        if (ghostsLocked) {
            unsigned long restante = (ghostsReleaseTime > now ?
                                      ghostsReleaseTime - now : 0);
            buffer += "Fantasmas: TRANCADOS (" + to_string(restante) + " ms)\n";
        }
        else {
            buffer += string(30, ' ') + "\n";
        }

        setColor(7);
        SetConsoleCursorPosition(out, coord);
        cout << buffer << flush;

        // desenhar mapa (deslocado 1 linha para baixo)
        // Observação: aqui substituí chamadas para desenhar na linha i por i+1
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                char ch;
                if (i == pacX && j == pacY) {
                    ch = (pacDir == 0 ? '^' :
                          pacDir == 1 ? '>' :
                          pacDir == 2 ? 'v' : '<');
                }
                else if (i == bx && j == by) {
                    ch = (frightenedActive ? 'f' : 'B');
                }
                else if (i == px && j == py) {
                    ch = (frightenedActive ? 'f' : 'P');
                }
                else if (i == ix && j == iy) {
                    ch = (frightenedActive ? 'f' : 'I');
                }
                else if (i == cx && j == cy) {
                    ch = (frightenedActive ? 'f' : 'C');
                }
                else {
                    if (m[i][j] == 1) ch = char(219);
                    else if (pellets[i][j] == 1) ch = '.';
                    else if (pellets[i][j] == 2) ch = 'O';
                    else if (pellets[i][j] == 3) ch = 'S'; // Speed
                    else if (pellets[i][j] == 4) ch = 'F'; // Freeze
                    else if (pellets[i][j] == 5) ch = 'H'; // sHield
                    else ch = ' ';
                }

                int color = 7;

                if (ch == char(219)) color = 8;
                else if (ch == '.') color = 7;
                else if (ch == 'O') color = 14;
                else if (ch == '^' || ch == '>' || ch == 'v' || ch == '<') color = 14;
                else if (ch == 'B') color = 12;
                else if (ch == 'P') color = 13;
                else if (ch == 'I') color = 11;
                else if (ch == 'C') color = 6;
                else if (ch == 'f') color = 9;
                else if (ch == 'S') color = 10;
                else if (ch == 'F') color = 3;
                else if (ch == 'H') color = 5;

                setColor(color);
                // desenha com deslocamento vertical de +1: i + 1
                drawCharOnMap(out, i + 1, j, ch);
            }
        }

        // desenhar indicador de escudo ao lado do Pacman (também com deslocamento)
        if (shieldActive) {
            setColor(5);
            if (pacY + 1 < COLS) drawCharOnMap(out, pacX + 1, pacY+1, 'U'); // U = shield icon
        }

        setColor(7);

        // limpar aviso
        if (msgStart != 0) {
            if (now - msgStart >= MSG_DURATION) {

                if (powerPending == PU_SPEED) {
                    speedActive = true;
                    speedStart = now;
                    showPowerHUD = true;
                }
                else if (powerPending == PU_FREEZE) {
                    freezeActive = true;
                    freezeStart = now;
                    showPowerHUD = true;
                }
                else if (powerPending == PU_SHIELD) {
                    shieldActive = true;
                    shieldStart = now;
                    showPowerHUD = true;
                }

                powerPending = PU_NONE;

                COORD avisoPos;
                avisoPos.X = 0;
                avisoPos.Y = MSG_LINE;

                SetConsoleCursorPosition(out, avisoPos);
                cout << string(80, ' ') << flush;

                msgStart = 0;
            }
        }

        bool venceu = false;
        bool faseCompleta = true;

        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLS; j++)
                if (pellets[i][j] != 0)
                    faseCompleta = false;
        if (faseCompleta)
        {
            // Primeiro: soma os pontos pela fase que acabou (faseAtual corresponde à fase concluída)
            int recompensa = pontosDaFase(dificuldade, faseAtual);
            pontos += recompensa;
            pontos += movimentos;
            movimentos = 0;

            // Mensagem curta informando o ganho
            COORD infoPos;
            infoPos.X = 0;
            infoPos.Y = MSG_LINE;
            SetConsoleCursorPosition(out, infoPos);
            cout << "Fase " << faseAtual << " concluida! +"
                 << recompensa
                 << " pontos + "
                 << movimentos
                 << " por movimentos (Total: " << pontos << ")      " << flush;

            // Pausa curta para o jogador ver a mensagem
            Sleep(900);

            // Se ainda há fases no mesmo nível -> avança para a próxima fase
            if (faseAtual < 3) {
                faseAtual++;

                // Carrega a fase correta de acordo com a dificuldade selecionada
                if (dificuldade == 1)
                    fasesFacil(faseAtual, m);
                else if (dificuldade == 2)
                    fasesMedio(faseAtual, m);
                else
                    fasesDificil(faseAtual, m);

                // ----------------------------
                // Reset do estado dos fantasmas
                // ----------------------------
                ghostsLocked = true;
                showGhostMsg = true;
                ghostMsgStart = GetTickCount();

                // Fantasmas sairão após a mensagem e o delay normal
                ghostsReleaseTime = ghostMsgStart + GHOST_MSG_DURATION + GHOST_RELEASE_DELAY;

                // Resetar posição dos fantasmas na "casa"
                bx = houseX;      by = houseY;      bdir = 1;
                px = houseX;      py = houseY + 1;  pdir = 1;
                ix = houseX + 1;  iy = houseY;      idir = 1;
                cx = houseX + 1;  cy = houseY + 1;  cdir = 1;

                // Reset modo dos fantasmas
                ghostsMode = SCATTER;
                currentModeIndex = 0;
                modeStartTime = GetTickCount();

                frightenedActive = false;

                // Recria pellets com base no m (m acabou de ser atualizado pela fasesXXX)
                for (int i = 0; i < ROWS; i++)
                    for (int j = 0; j < COLS; j++)
                        pellets[i][j] = (m[i][j] == 0 ? 1 : 0);

                // Coloca power pellets apenas onde realmente é livre (m == 0)
                if (m[1][1] == 0) pellets[1][1] = 2;
                if (m[1][COLS-2] == 0) pellets[1][COLS-2] = 2;
                if (m[ROWS-2][1] == 0) pellets[ROWS-2][1] = 2;
                if (m[ROWS-2][COLS-2] == 0) pellets[ROWS-2][COLS-2] = 2;

                // coloca especiais novamente
                place_special_pellets_auto();
                //pellets[PAC_START_X][PAC_START_Y] = 0;

                // Reseta posição do Pacman
                pacX = PAC_START_X;
                pacY = PAC_START_Y;
                movimentos = 0;
                pacDir = 1;

                // Reseta fantasmas (já feito acima, mas repetido por segurança)
                bx = houseX; by = houseY; bdir = 1;
                px = houseX; py = houseY+1; pdir = 1;
                ix = houseX+1; iy = houseY; idir = 1;
                cx = houseX+1; cy = houseY+1; cdir = 1;

                // limpa a mensagem do MSG_LINE após a pausa
                SetConsoleCursorPosition(out, infoPos);
                cout << string(80, ' ') << flush;

                continue; // volta ao loop com a nova fase carregada
            }
            else {
                // Se não há mais fases -> vitória final
                venceu = true;
            }
        }

        if (venceu) {

            // Pontuação ao completar fase, de acordo com o nível e fase
            if (dificuldade == 1) { // Fácil
                if (faseAtual == 1) pontos += 100;
                else if (faseAtual == 2) pontos += 200;
                else if (faseAtual == 3) pontos += 300;
            }
            else if (dificuldade == 2) { // Médio
                if (faseAtual == 1) pontos += 500;
                else if (faseAtual == 2) pontos += 600;
                else if (faseAtual == 3) pontos += 700;
            }
            else if (dificuldade == 3) { // Difícil
                if (faseAtual == 1) pontos += 1000;
                else if (faseAtual == 2) pontos += 2000;
                else if (faseAtual == 3) pontos += 3000;
            }


            system("cls");
            SetConsoleCursorPosition(out, coord);

            cout << "====================== FIM DE JOGO =======================\n";
            cout << "Parabens, " << nome << "! Voce venceu o jogo! Pontos: "
                 << pontos << "\n";

            unsigned long totalTimeMs = GetTickCount() - startTime;
            int totalSeconds = totalTimeMs / 1000;
            int minutes = totalSeconds / 60;
            int seconds = totalSeconds % 60;

            string tempo = to_string(minutes) + "m " + to_string(seconds) + "s";
            string data = getDateTime();

            Jogador j = { nome, pontos, data, tempo };
            salvarPontos(j);

            cout << "\n\n";
            showRanking();

            cout << "\nPressione qualquer tecla para sair...";
            while (!_kbhit()) Sleep(50);
            getch();

            return 0;
        }

        // colisão com fantasma
        if ((pacX == bx && pacY == by) ||
            (pacX == px && pacY == py) ||
            (pacX == ix && pacY == iy) ||
            (pacX == cx && pacY == cy)) {

            bool collisionWithGhostHandled = false;

            if (frightenedActive) {

                if (pacX == bx && pacY == by) {
                    eatEffect(out, bx, by);
                    bx = houseX; by = houseY;
                    bdir = 1;
                    pontos += 50;
                    collisionWithGhostHandled = true;
                }

                if (pacX == px && pacY == py) {
                    eatEffect(out, px, py);
                    px = houseX; py = houseY + 1;
                    pdir = 1;
                    pontos += 50;
                    collisionWithGhostHandled = true;
                }

                if (pacX == ix && pacY == iy) {
                    eatEffect(out, ix, iy);
                    ix = houseX + 1; iy = houseY;
                    idir = 1;
                    pontos += 50;
                    collisionWithGhostHandled = true;
                }

                if (pacX == cx && pacY == cy) {
                    eatEffect(out, cx, cy);
                    cx = houseX + 1; cy = houseY + 1;
                    cdir = 1;
                    pontos += 50;
                    collisionWithGhostHandled = true;
                }
            }
            else {
                // if shield active, consume shield instead of losing life
                if (shieldActive) {
                    // escudo é consumido pela colisão - comportamento desejado
                    shieldActive = false;

                    // envia o fantasma de volta pra casa
                    if (pacX == bx && pacY == by) { bx = houseX; by = houseY; bdir = 1; }
                    if (pacX == px && pacY == py) { px = houseX; py = houseY+1; pdir = 1; }
                    if (pacX == ix && pacY == iy) { ix = houseX+1; iy = houseY; idir = 1; }
                    if (pacX == cx && pacY == cy) { cx = houseX+1; cy = houseY+1; cdir = 1; }

                    // mensagem de aviso (sua mensagem atual)
                    COORD infoPos; infoPos.X = 0; infoPos.Y = MSG_LINE;
                    SetConsoleCursorPosition(out, infoPos);
                    cout << "Escudo protegido! Escudo perdido.                     " << flush;
                    msgStart = now;
                    collisionWithGhostHandled = true;

                    // --- ADIÇÕES: limpar apenas a parte do HUD que mostra o power-up ---
                    // 1) limpa qualquer estado visível relativo ao power-up
                    activePower = "";        // se você estiver usando essa variável (recomendado)
                    showPowerHUD = false;    // sinaliza que não deve mais mostrar o power no HUD
                    shieldStart = 0;         // zera o timer (opcional, mas seguro)

                    // 2) limpar visualmente a linha do HUD onde aparece "Power-up ativo: ..."
                    //    - no seu código o buffer é impresso a partir da linha 0; a linha do power-up é a linha 1
                    COORD clearPos;
                    clearPos.X = 0;
                    clearPos.Y = 1; // se sua linha do HUD for outra, ajuste aqui (ex: 0, 1, etc.)
                    SetConsoleCursorPosition(out, clearPos);

                    // sobrescreve a área do power-up com "Power-up ativo: " + espaços para apagar resto
                    cout << "Power-up ativo: " << string(40, ' ') << flush;
                    // ---------------------------------------------------------------------
                }else {
                    if (pacX == bx && pacY == by) deathEffect(out, pacX, pacY);
                    else if (pacX == px && pacY == py) deathEffect(out, pacX, pacY);
                    else if (pacX == ix && pacY == iy) deathEffect(out, pacX, pacY);
                    else if (pacX == cx && pacY == cy) deathEffect(out, pacX, pacY);

                    vidas--;
                    pacX = PAC_START_X;
                    pacY = PAC_START_Y;
                    pacDir = 1;

                    Sleep(800);

                    if (vidas <= 0) {

                        SetConsoleCursorPosition(out, coord);
                        cout << "GAME OVER! Pontos: " << pontos << "\n";

                        unsigned long totalTimeMs = GetTickCount() - startTime;
                        int totalSeconds = totalTimeMs / 1000;
                        int minutes = totalSeconds / 60;
                        int seconds = totalSeconds % 60;

                        string tempo = to_string(minutes) + "m " + to_string(seconds) + "s";
                        string data = getDateTime();

                        Jogador j = { nome, pontos, data, tempo };
                        salvarPontos(j);

                        system("cls");
                        showRanking();

                        cout << "\nPressione qualquer tecla para jogar novamente ou ESC para sair...";

                        while (true) {
                            if (_kbhit()) {
                                int tecla = _getch();

                                if (tecla == 27) {
                                    cout << "\nEncerrando o jogo...";
                                    Sleep(1000);
                                    return 0;
                                }
                                else break;
                            }
                            Sleep(50);
                        }

                        system("cls");

                        for (int i = 0; i < ROWS; i++)
                            for (int j = 0; j < COLS; j++)
                                pellets[i][j] = (m[i][j] == 0 ? 1 : 0);

                        if (is_free(1,1)) pellets[1][1] = 2;
                        if (is_free(1,COLS-2)) pellets[1][COLS-2] = 2;
                        if (is_free(ROWS-2,1)) pellets[ROWS-2][1] = 2;
                        if (is_free(ROWS-2,COLS-2)) pellets[ROWS-2][COLS-2] = 2;

                        // recoloca especiais
                        place_special_pellets_auto();

                        pontos = 0;
                        vidas = 3;

                        pacX = PAC_START_X;
                        pacY = PAC_START_Y;
                        pacDir = 1;

                        bx = houseX;      by = houseY;      bdir = 1;
                        px = houseX;      py = houseY + 1;  pdir = 1;
                        ix = houseX + 1;  iy = houseY;      idir = 1;
                        cx = houseX + 1;  cy = houseY + 1;  cdir = 1;

                        ghostsLocked = true;
                        ghostsReleaseTime = GetTickCount() + GHOST_RELEASE_DELAY;

                        ghostsMode = SCATTER;
                        currentModeIndex = 0;
                        modeStartTime = GetTickCount();
                        frightenedActive = false;

                        COORD avisoPos;
                        avisoPos.X = 0;
                        avisoPos.Y = MSG_LINE;
                        SetConsoleCursorPosition(out, avisoPos);
                        cout << string(80, ' ') << flush;

                        msgStart = 0;
                    }
                    collisionWithGhostHandled = true;
                }
            }
        }

        Sleep(FRAME_MS);
    }

    return 0;
}