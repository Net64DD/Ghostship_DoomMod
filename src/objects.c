#include "doom.h"

#define OBJECT_FIELDS_INDEX_DIRECTLY

#include "object_constants.h"
#include "game/object_list_processor.h"
#include "game/interaction.h"
#include "game/behavior_actions.h"
#include "game/mario_actions_cutscene.h"
#include "game/mario_misc.h"
#include "game/object_helpers.h"
#include "engine/surface_load.h"
#include "behavior_data.h"
#include "behavior_macros.h"
#include "geo_commands.h"
#include "actors/common0.h"
#include "game/level_update.h"
#include "assets/actors/breakable_box.h"

extern struct AllocOnlyPool *sLevelPool;

#define o gCurrentObject
#define MODEL_TVBOX 0xE2

struct ObjectHitbox sTVHitbox = {
    /* interactType:      */ INTERACT_BREAKABLE,
    /* downOffset:        */ 20,
    /* damageOrCoinValue: */ 0,
    /* health:            */ 1,
    /* numLootCoins:      */ 0,
    /* radius:            */ 150,
    /* height:            */ 200,
    /* hurtboxRadius:     */ 150,
    /* hurtboxHeight:     */ 200,
};

static void bhv_tv_loop(void) {
    obj_set_hitbox(o, &sTVHitbox);
    cur_obj_set_model(MODEL_TVBOX);
}

static const BehaviorScript bhvCustomTv[] = {
    BEGIN(OBJ_LIST_SURFACE),
    OR_INT(oFlags, OBJ_FLAG_UPDATE_GFX_POS_AND_ANGLE),
    LOAD_COLLISION_DATA(breakable_box_seg8_collision_08012D70),
    SET_FLOAT(oCollisionDistance, 500),
    CALL_NATIVE(bhv_init_room),
    BEGIN_LOOP(),
        CALL_NATIVE(bhv_tv_loop),
        CALL_NATIVE(load_object_collision_model),
    END_LOOP(),
    BREAK(),
};

static const GeoLayout tv_model_geo[] = {
   GEO_CULLING_RADIUS(500),
   GEO_OPEN_NODE(),
      GEO_SWITCH_CASE(2, geo_switch_anim_state),
      GEO_OPEN_NODE(),
         GEO_DISPLAY_LIST(LAYER_OPAQUE, tv_model),
         GEO_DISPLAY_LIST(LAYER_OPAQUE, tv_model),
      GEO_CLOSE_NODE(),
   GEO_CLOSE_NODE(),
   GEO_END(),
};

void OnLoadArea(void* _) {
    gLoadedGraphNodes[MODEL_TVBOX] = (struct GraphNode *) init_graph_node_display_list(sLevelPool, 0, LAYER_OPAQUE, tv_model);
    printf("Loaded TV model\n");
}

void SpawnObject(u32 modelId, const BehaviorScript* behavior, s16 x, s16 y, s16 z, s32 param) {
    struct Object* object =
        spawn_object_abs_with_rot(&gMacroObjectDefaultParent, 0, modelId, behavior, x, y, z, 0, 0, 0);
    object->custom = true;
}

void SpawnTV(void) {
    SpawnObject(MODEL_TVBOX, bhvCustomTv, gMarioState->pos[0], gMarioState->pos[1], gMarioState->pos[2], 0);
}