#include "doom.h"

#include "sm64.h"
#include "mod.h"
#include "library.h"
#include "objs/models.h"
#include "game/level_update.h"
#include "game/object_list_processor.h"
#include "game/object_helpers.h"
#include "game/print.h"
#include "game/mario.h"
#include "game/game_init.h"
#include "game/camera.h"
#include <math.h>

#define o gCurrentObject

extern const BehaviorScript bhvCustomTv[];
struct Object* tv;

#include <stdbool.h>
bool gPlayingDoom = false;

extern void bhv_beta_holdable_object_loop(void);

void custom_tv_loop(void) {
    if (gMarioState == NULL) return;

    f32 distX = gMarioState->pos[0] - o->oPosX;
    f32 distY = gMarioState->pos[1] - o->oPosY;
    f32 distZ = gMarioState->pos[2] - o->oPosZ;
    f32 dist = sqrtf(distX * distX + distY * distY + distZ * distZ);

    if (!gPlayingDoom && dist < 300.0f) {
        print_text_centered(160, 40, "PRESS A TO PLAY");
        if (gPlayer1Controller->buttonPressed & A_BUTTON) {
            gPlayingDoom = true;
            gPlayer1Controller->buttonPressed &= ~A_BUTTON;
        }
    }
}

void SpawnDoomTV(void) {
    ModelID modelId = CallUtil(GetModelByName, "ModelTVBox")->id;
    f32 spawnDist = 150.0f;
    f32 spawnX = gMarioState->pos[0] + sins(gMarioState->faceAngle[1]) * spawnDist;
    f32 spawnY = gMarioState->pos[1];
    f32 spawnZ = gMarioState->pos[2] + coss(gMarioState->faceAngle[1]) * spawnDist;

    if(tv == NULL) {
        tv = CallUtil(SpawnObject, modelId, bhvCustomTv, spawnX, spawnY, spawnZ, 0);
    } else {
        tv->oPosX = spawnX;
        tv->oPosY = spawnY;
        tv->oPosZ = spawnZ;
    }
    tv->oMoveAngleYaw = atan2s(gMarioState->pos[0] - tv->oPosX, gMarioState->pos[2] - tv->oPosZ);
    tv->oFaceAngleYaw = tv->oMoveAngleYaw;
}