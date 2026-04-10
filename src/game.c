#include "mod.h"

#include <stdio.h>
#include <stdlib.h>

#include "doom.h"
#include "sm64.h"
#include "library.h"
#include "objs/models.h"
#include "port/events/Events.h"
#include "game/game_init.h"
#include "port/api/ui.h"

ListenerID gListenerIDs[4];
uint8_t* gDoomScreenBuffer = NULL;

void SetupModels() {
    CallUtil(RegisterModel, "ModelTVBox", tv_model, MODEL_DISPLAY_LIST, MODEL_PRIVATE);
    CallUtil(RegisterModel, "ModelSnes", snes_model, MODEL_DISPLAY_LIST, MODEL_PRIVATE);
    CallUtil(RegisterModel, "ModelSnesController", snes_controller_model, MODEL_DISPLAY_LIST, MODEL_PRIVATE);
}

void SetupUI() {
    C_WidgetConfig chk = {0};
    chk.type = C_WIDGET_CVAR_CHECKBOX;
    chk.cvar = "gDoom.EnableTVSpawn";
    chk.opts.checkbox.tooltip = "Enable TV Spawn on L Button";
    chk.opts.checkbox.default_val = true;
    C_AddSidebarEntry("Doom", 1);
    C_AddWidget("Doom", 1, "Enable TV Spawn", &chk);
}

int StickToKey(f32 axis, int is_y_axis) {
    if (axis > 40.0f) {
        return is_y_axis ? DOOMKEY_UPARROW : DOOMKEY_RIGHTARROW;
    } else if (axis < -40.0f) {
        return is_y_axis ? DOOMKEY_DOWNARROW : DOOMKEY_LEFTARROW;
    }
    return 0;
}

void SendKeyIfNotZero(int key, int pressed) {
    if (key != 0) {
        DoomDLL_Input(pressed, key);
    }
}

void HandleDoomInput() {
    struct Controller *input = gPlayer1Controller;

    static f32 prev_stick_x = 0.0f;
    static f32 prev_stick_y = 0.0f;
    static u16 prev_buttons = 0;

    int prev_x_key = StickToKey(prev_stick_x, 0);
    int cur_x_key  = StickToKey(input->stickX, 0);
    int prev_y_key = StickToKey(prev_stick_y, 1);
    int cur_y_key  = StickToKey(input->stickY, 1);

    if (prev_x_key != cur_x_key) {
        if (prev_x_key != 0) DoomDLL_Input(0, prev_x_key); 
        if (cur_x_key != 0)  DoomDLL_Input(1, cur_x_key);  
    }

    if (prev_y_key != cur_y_key) {
        if (prev_y_key != 0) DoomDLL_Input(0, prev_y_key); 
        if (cur_y_key != 0)  DoomDLL_Input(1, cur_y_key);  
    }

    prev_stick_x = input->stickX;
    prev_stick_y = input->stickY;

    HANDLE_DOOM_BTN(BTN_A,      DOOMKEY_ENTER);    // Menu Select
    HANDLE_DOOM_BTN(BTN_CDOWN,  DOOMKEY_USE);      // Shoot
    HANDLE_DOOM_BTN(BTN_B,      DOOMKEY_FIRE);     // Open Doors
    HANDLE_DOOM_BTN(BTN_CUP,    DOOMKEY_ESCAPE);   // Pause Menu
    HANDLE_DOOM_BTN(BTN_CLEFT,  DOOMKEY_STRAFE_L); // Strafe Left
    HANDLE_DOOM_BTN(BTN_CRIGHT, DOOMKEY_STRAFE_R); // Strafe Right

    prev_buttons = input->buttonDown;
}

void OnGameRender(IEvent* event) {
    if(CVarGetInteger("gDoom.EnableTVSpawn", 0) != 0 && gPlayer1Controller->buttonPressed & BTN_L) {
        SpawnDoomTV();
    }

    DoomDLL_Tick(NULL);
    DoomDLL_ScreenCopy(gDoomScreenBuffer);
    HandleDoomInput();

    gSPInvalidateTexCache(gDisplayListHead++, gDoomScreenBuffer);
    __gSPSegment(gDisplayListHead++, 0x06, gDoomScreenBuffer);
}

MOD_INIT() {
    SetupModels();
    SetupUI();
    DoomDLL_Initialize("doom.wad");

    gDoomScreenBuffer = (uint8_t*) malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
    gListenerIDs[0] = REGISTER_LISTENER(RenderGamePost, EVENT_PRIORITY_NORMAL, OnGameRender);
}

MOD_EXIT() {
    free(gDoomScreenBuffer);
    C_RemoveSidebarEntry("Doom");
    UNREGISTER_LISTENER(RenderGamePost, gListenerIDs[0]);
}