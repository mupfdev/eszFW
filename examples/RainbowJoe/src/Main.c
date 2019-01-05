/**
 * @file Main.c
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <stdlib.h>
#include "Game.h"

int main()
{
    int sExecStatus;

    sExecStatus = InitGame();
    QuitGame();

    return sExecStatus;
}
