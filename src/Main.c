/**
 * @file Main.c
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <stdlib.h>
#include "RainbowJoe.h"

int main()
{
    int sExecStatus;
    sExecStatus = InitGame();

    QuitGame();
    return sExecStatus;
}
