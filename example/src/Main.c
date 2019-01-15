/**
 * @file Main.c
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <stdlib.h>
#include "Launcher.h"

int main(int sArgC, char *pacArgV[])
{
    int sExecStatus;

    // Avoid 'error: unused parameter':
    (void)sArgC; (void)pacArgV;

    sExecStatus = InitLauncher();
    Quit();

    return sExecStatus;
}
