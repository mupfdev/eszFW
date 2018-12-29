/**
 * @file Main.c
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <stdlib.h>
#include "RainbowJoe.h"

int main()
{
    int sMainStatus = EXIT_SUCCESS;

    if (-1 == InitGame())
    {
        sMainStatus = EXIT_FAILURE;
    }

    QuitGame();
    return sMainStatus;
}
