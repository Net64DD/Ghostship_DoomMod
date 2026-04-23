#pragma once

#include <stdint.h>

#include "macros.h"
#include "align_asset_macro.h"

extern uint32_t* DG_ScreenBuffer;

#define DOOMKEY_RIGHTARROW  0xae
#define DOOMKEY_LEFTARROW   0xac
#define DOOMKEY_UPARROW     0xad
#define DOOMKEY_DOWNARROW   0xaf
#define DOOMKEY_STRAFE_L    0xa0
#define DOOMKEY_STRAFE_R    0xa1
#define DOOMKEY_USE         0xa2
#define DOOMKEY_FIRE        0xa3
#define DOOMKEY_ESCAPE      27
#define DOOMKEY_ENTER       13
#define DOOMGENERIC_RESX 320
#define DOOMGENERIC_RESY 200

#define CHECK_BTN_ALL(state, combo) (~((state) | ~(combo)) == 0)

#define HANDLE_DOOM_BTN(sm64_mask, doom_key) { \
    if ((input.buttonDown & (sm64_mask)) && !(prev_buttons & (sm64_mask))) { \
        DoomDLL_Input(1, doom_key); \
    } \
    if (!(input.buttonDown & (sm64_mask)) && (prev_buttons & (sm64_mask))) { \
        DoomDLL_Input(0, doom_key); \
    } \
}

extern void DoomDLL_Initialize(const char* wad);
extern void DoomDLL_Tick(uint8_t *rdram);
extern void DoomDLL_Input(int pressed, unsigned char doomKey);
extern void DoomDLL_ScreenCopy(uint8_t *rdram);
extern void DoomDLL_ScreenWidth(uint8_t *rdram);
extern void DoomDLL_ScreenHeight(uint8_t *rdram);

// Objects
extern void SpawnDoomTV(void);