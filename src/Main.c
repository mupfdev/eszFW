/**
 * @file Main.c
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <stdlib.h>
#include "examples/RainbowJoe.h"

int main()
{
    int sExecStatus;

    sExecStatus = InitRainbowJoe();
    QuitRainbowJoe();

    return sExecStatus;
}
