//
//  Chip8.h
//  Chip8Emulator
//
//  Created by Thomas Miller on 6/24/14.
//  Copyright (c) 2014 Thomas Miller. All rights reserved.
//

#ifndef __Chip8Emulator__Chip8__
#define __Chip8Emulator__Chip8__

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <assert.h>
#include <vector>
#include <stdarg.h>
#include <SDL2/SDL.h>

#define DEBUG 0

#define LOGFILE stdout

#define DbgPrint(...) if(DEBUG){ fprintf (LOGFILE, __VA_ARGS__); }

//
// Useful BitMasks
//

#define FIRST_FOUR_BITMASK (0xF000)
#define LAST_FOUR_BITMASK (0x000F)
#define FIRST_EIGHT_BITMASK (0xFF00)
#define LAST_EIGHT_BITMASK (0x00FF)
#define LAST_TWELVE_BITMASK (0x0FFF)

#define REGISTER_ONE_BITMASK (0x0F00)
#define REGISTER_TWO_BITMASK (0x00F0)

#define CHARACTER_SPRITE_SIZE (5)

#define PROGRAM_START_LOCATION (0x200)

#define GRAPHICS_X_AXIS (64)
#define GRAPHICS_Y_AXIS (32)


class Chip8 {
    
private:
    
    unsigned char Memory[4096];
    
    unsigned char *ProgramEnd;
    
    unsigned char *FontsetLocation;
    
    unsigned char VRegisters[16];
    
    unsigned short IndexRegister;
    
    unsigned short ProgramCounter;
    
    unsigned char DelayTimer;
    unsigned char SoundTimer;
    
    std::vector<short> Stack;
    
    unsigned short StackPointer;
    
    unsigned char Key[16];
    
    unsigned short Opcode;
    
    bool DrawFlag;
    
    typedef enum RegisterLocationInOpcode {
        First,
        Second
    };
    
    typedef enum RegisterOperation {
        Add,
        Subtract
    };
    
    unsigned short GetRegister(RegisterLocationInOpcode RegLoc);
    void CheckAndSetCarry(unsigned short Value1,
                          unsigned short Value2,
                          RegisterOperation Operator);
    void SetCarry(int OneOrZero);
    void SkipNextInstruction();
    void DrawSprites();
    
    unsigned char GetKeyNum(SDL_Keysym Symbol);
    
    
public:
    void Initialize();
    bool EmulateCycle(unsigned char *KeyboardState);
    void DebugDumpState();
    bool LoadRom (char* FileName);
    void HandleKeyboard (unsigned char Key, int x, int y);
    bool Draw() {return DrawFlag;};
    
    unsigned char Graphics[64 * 32];
    
};


#endif /* defined(__Chip8Emulator__Chip8__) */
