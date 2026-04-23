#include "mod.h"

#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

#include "doom.h"
#include "sm64.h"
#include "draw.h"
#include "library.h"
#include "objs/models.h"
#include "port/events/Events.h"
#include "game/game_init.h"
#include "port/api/ui.h"
#include "game/level_update.h"
#include "game/camera.h"

Gfx gPool[0x1024] = { 0 };
ListenerID gListenerIDs[4];
OSContPad gControllerInput[4];
uint8_t* gDoomScreenBuffer = NULL;

void SpawnDoomTV(void);
extern bool gPlayingDoom;

void SetupModels() {
    CallUtil(RegisterModel, "ModelTVBox", tv_model, MODEL_DISPLAY_LIST, MODEL_PRIVATE);
    CallUtil(RegisterModel, "ModelSnes", snes_model, MODEL_DISPLAY_LIST, MODEL_PRIVATE);
    CallUtil(RegisterModel, "ModelSnesController", snes_controller_model, MODEL_DISPLAY_LIST, MODEL_PRIVATE);
}

void SetupUI() {
    C_WidgetConfig chk = {0};
    chk.type = C_WIDGET_CVAR_CHECKBOX;
    chk.cvar = "gDoom.EnableTVSpawn";
    chk.opts.checkbox.tooltip = "Enable TV Spawn on L & Z Button";
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

struct Controller OSPadToController(OSContPad *pad) {
    struct Controller c = {0};
    c.stickX = (f32)pad->stick_x;
    c.stickY = (f32)pad->stick_y;
    c.buttonDown = pad->button;
    return c;
}

void HandleDoomInput() {
    struct Controller input = OSPadToController(&gControllerInput[0]);

    static f32 prev_stick_x = 0.0f;
    static f32 prev_stick_y = 0.0f;
    static u16 prev_buttons = 0;

    int prev_x_key = StickToKey(prev_stick_x, 0);
    int cur_x_key  = StickToKey(input.stickX, 0);
    int prev_y_key = StickToKey(prev_stick_y, 1);
    int cur_y_key  = StickToKey(input.stickY, 1);

    if (prev_x_key != cur_x_key) {
        if (prev_x_key != 0) DoomDLL_Input(0, prev_x_key); 
        if (cur_x_key != 0)  DoomDLL_Input(1, cur_x_key);  
    }

    if (prev_y_key != cur_y_key) {
        if (prev_y_key != 0) DoomDLL_Input(0, prev_y_key); 
        if (cur_y_key != 0)  DoomDLL_Input(1, cur_y_key);  
    }

    prev_stick_x = input.stickX;
    prev_stick_y = input.stickY;

    HANDLE_DOOM_BTN(BTN_A,      DOOMKEY_ENTER);    // Menu Select
    HANDLE_DOOM_BTN(BTN_CDOWN,  DOOMKEY_USE);      // Shoot
    HANDLE_DOOM_BTN(BTN_B,      DOOMKEY_FIRE);     // Open Doors
    HANDLE_DOOM_BTN(BTN_CUP,    DOOMKEY_ESCAPE);   // Pause Menu
    HANDLE_DOOM_BTN(BTN_CLEFT,  DOOMKEY_STRAFE_L); // Strafe Left
    HANDLE_DOOM_BTN(BTN_CRIGHT, DOOMKEY_STRAFE_R); // Strafe Right

    if(input.buttonDown & BTN_L) {
        gPlayingDoom = false;
    }

    prev_buttons = input.buttonDown;
}

void OnGameInput(IEvent* event) {
    if(!gPlayingDoom) {
        gControllerInput[0] = (OSContPad){0};
        return;
    }

    event->Cancelled = true;
    osContGetReadData(gControllerInput);
}

void Graphics_FillRectangle(Gfx** gfxPtr, s32 ulx, s32 uly, s32 lrx, s32 lry, u8 r, u8 g, u8 b, u8 a) {
    gDPPipeSync((*gfxPtr)++);
    gDPSetCycleType((*gfxPtr)++, G_CYC_FILL);
    gDPSetFillColor((*gfxPtr)++, (r << 24) | (g << 16) | (b << 8) | a);
    gDPFillWideRectangle((*gfxPtr)++, ulx, uly, lrx, lry);
    gDPPipeSync((*gfxPtr)++);
}

void RenderToScreen() {
    if(!gPlayingDoom) {
        return;
    }

    Gfx* head = &gPool[0];

    gDPPipeSync(head++);
    gDPSetOtherMode(head++,
                    G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_POINT | G_TT_IA16 | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | G_RM_XLU_SURF | G_RM_XLU_SURF2);
    gDPSetCombineMode(head++, G_CC_DECALRGBA, G_CC_DECALRGBA);
    gDPSetColor(head++, G_SETPRIMCOLOR, 0xFFFFFFFF);

    u32 rowsPerSlice = 3; 
    u32 totalSlices = (DOOMGENERIC_RESY + rowsPerSlice - 1) / rowsPerSlice;

    u32 borderSize = 3;
    float xPos = borderSize;
    float yPos = (SCREEN_HEIGHT / 2 - (DOOMGENERIC_RESY / 2));

    Graphics_FillRectangle(&head, 0, yPos - borderSize, DOOMGENERIC_RESX + borderSize * 2, yPos + DOOMGENERIC_RESY + borderSize, 0, 0, 0, 255);

    for (u32 i = 0; i < totalSlices; i++) {
        u32 currentY = i * rowsPerSlice;
        u32 rowsToDraw = (currentY + rowsPerSlice > DOOMGENERIC_RESY) ? (DOOMGENERIC_RESY - currentY) : rowsPerSlice;

        u32* slicePtr = (u32*)((u8*)gDoomScreenBuffer + (currentY * DOOMGENERIC_RESX * 4));
        gSPInvalidateTexCache(head++, slicePtr);
        CallUtil(Lib_TextureRect_RGBA32, &head, slicePtr, DOOMGENERIC_RESX, rowsToDraw, xPos, (f32)yPos + currentY, 1.0f, 1.0f);
    }

    gSPEndDisplayList(head);
    gSPDisplayList(gDisplayListHead++, gPool);
}

void OnGameRender(IEvent* event) {
    if(CVarGetInteger("gDoom.EnableTVSpawn", 0) != 0 && gPlayer1Controller->buttonDown & BTN_Z && gPlayer1Controller->buttonPressed & BTN_L) {
        SpawnDoomTV();
    }

    DoomDLL_Tick(NULL);
    DoomDLL_ScreenCopy(gDoomScreenBuffer);

    HandleDoomInput();

    gSPInvalidateTexCache(gDisplayListHead++, gDoomScreenBuffer);
    __gSPSegment(gDisplayListHead++, 0x06, gDoomScreenBuffer);

    RenderToScreen();
}

MOD_INIT() {
    SetupModels();
    SetupUI();
    DoomDLL_Initialize("doom.wad");

    gDoomScreenBuffer = (uint8_t*) malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
    gListenerIDs[0] = REGISTER_LISTENER(RenderGamePost, EVENT_PRIORITY_NORMAL, OnGameRender);
    gListenerIDs[1] = REGISTER_LISTENER(GameReadInput, EVENT_PRIORITY_NORMAL, OnGameInput);
}

MOD_EXIT() {
    free(gDoomScreenBuffer);
    C_RemoveSidebarEntry("Doom");
    UNREGISTER_LISTENER(RenderGamePost, gListenerIDs[0]);
    UNREGISTER_LISTENER(GameReadInput, gListenerIDs[1]);
}