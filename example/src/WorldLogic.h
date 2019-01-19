/**
 * @file WorldLogic.h
 * @ingroup WorldLogic
 */

#ifndef _WORLD_LOGIC_H_
#define _WORLD_LOGIC_H_

#include <stdbool.h>
#include "GameData.h"

void UpdateWorldStart(GameData **pstGameData);
void UpdateWorldEnd(const double dDeltaTime, GameData **pstGameData);

#endif // _WORLD_LOGIC_H_
