/**
 * @file Main.c
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <stdlib.h>
#include "Game.h"

int main(int sArgC, char *pacArgV[])
{
    int sExecStatus = SDL_main(sArgC, pacArgV);

    QuitGame();
    return sExecStatus;
}
