//
//  main.cpp
//  Chip8Emulator
//
//  Created by Thomas Miller on 6/24/14.
//  Copyright (c) 2014 Thomas Miller. All rights reserved.
//

#include <iostream>

#include <openGl/gl.h>
#include <GLUT/GLUT.h>
#include <SDL2/SDL.h>
#include <ctime>

#include "Chip8.h"
#include "Graphics.h"

//
// Cpu is 60Hz. So clocks/cycle = (clocks/sec) / 60
//

#define CPU_HZ (100)
#define CLOCKS_PER_CYCLE (CLOCKS_PER_SEC/60)

//
// Keyboard State Array
// Keys are: 1234
//            qwer
//             asdf
//              zxcv
//

unsigned char Keyboard[16];

//
// Helper Functions
//

void TranslateKeyboardStates (const Uint8 *SdlKeyStates, unsigned char *Keyboard);
int AdjustSpeed (const Uint8 *SdlKeyStates, int CurrentSpeed);
void WaitForNextCycle (Uint32 TargetHz, Uint32 PreviousTicks, int SpeedLevel);

int main(int argc, char * argv[])
{
    Chip8 *Cpu;
    Graphics *Display;
    bool Quit = false;
    SDL_Event Event;
    const Uint8* CurrentKeyStates;
    unsigned char Keyboard[16];
    Uint32 Ticks;
    int SpeedLevel;
    
    std::cout << "Emulatin' shit\n";
    
    //
    // Initialize Cpu and Graphics
    //
    
    Cpu = new Chip8();
    Cpu->Initialize();
    
    Display = new Graphics();
    Display->Initialize();
    
    memset(Keyboard, 0, 16 * sizeof(unsigned char));
    
    //
    // Load ROM
    //
    
    if (!Cpu->LoadRom("/Users/thomasmiller/cmps/emulation/Chip8Emulator/Roms/BRIX")) {
        assert(false);
    };
    
    //
    // Main emulator loop. Simluate 60 Hz.
    //
    SpeedLevel = 3;
    
    while (!Quit) {
        
        Ticks = SDL_GetTicks();
        
        //
        // Check for quit event
        //
        
        while (SDL_PollEvent(&Event) != 0) {
            if (Event.type == SDL_QUIT) {
                Quit = true;
            }
        }
        
        CurrentKeyStates = SDL_GetKeyboardState(NULL);
        TranslateKeyboardStates (CurrentKeyStates, Keyboard);
        
        Cpu->EmulateCycle(Keyboard);
        
        if (Cpu->Draw()) {
            Display->Draw(Cpu->Graphics);
        }
        
        SpeedLevel = AdjustSpeed (CurrentKeyStates, SpeedLevel);
     
        memset(Keyboard, 0, 16 * sizeof(unsigned char));
        WaitForNextCycle(CPU_HZ, Ticks, SpeedLevel);
    }
    
    return 0;
}

int AdjustSpeed (const Uint8 *SdlKeyStates, int CurrentSpeed) {
    
    assert(SdlKeyStates != NULL);
    
    if (SdlKeyStates[SDL_SCANCODE_J] && CurrentSpeed > 1) {
        return CurrentSpeed - 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_K] && CurrentSpeed < 10) {
        return CurrentSpeed + 1;
    }
    return CurrentSpeed;
}

void WaitForNextCycle (Uint32 TargetHz, Uint32 PreviousTicks, int SpeedLevel)
{
    assert(0 < SpeedLevel && SpeedLevel < 11);
    
    Uint32 TicksPerCycle = 1000 / (TargetHz + CPU_HZ * SpeedLevel);
    Uint32 TicksPassed;
    
    if (SDL_GetTicks() > PreviousTicks) {
        TicksPassed = SDL_GetTicks() - PreviousTicks;
        
        if (TicksPerCycle > TicksPassed) {
            SDL_Delay(TicksPerCycle - TicksPassed);
        }
    }
}

void TranslateKeyboardStates (const Uint8 *SdlKeyStates, unsigned char *Keyboard)
{
    assert(SdlKeyStates != NULL && Keyboard != NULL);
    
    memset(Keyboard, 0, 16 * sizeof(unsigned char));
    
    if (SdlKeyStates[SDL_SCANCODE_1]) {
        Keyboard[0] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_2]) {
        Keyboard[1] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_3]) {
        Keyboard[2] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_4]) {
        Keyboard[3] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_Q]) {
        Keyboard[4] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_W]) {
        Keyboard[5] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_E]) {
        Keyboard[6] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_R]) {
        Keyboard[7] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_A]) {
        Keyboard[8] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_S]) {
        Keyboard[9] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_D]) {
        Keyboard[10] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_F]) {
        Keyboard[11] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_Z]) {
        Keyboard[12] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_X]) {
        Keyboard[13] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_C]) {
        Keyboard[14] = 1;
    }
    if (SdlKeyStates[SDL_SCANCODE_V]) {
        Keyboard[15] = 1;
    }
}

