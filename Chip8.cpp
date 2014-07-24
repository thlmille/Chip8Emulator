//
//  Chip8.cpp
//  Chip8Emulator
//
//  Created by Thomas Miller on 6/24/14.
//  Copyright (c) 2014 Thomas Miller. All rights reserved.
//

/*
 
 Chip 8 Emulator class implementation
 
 */


#include "Chip8.h"

#define TWO_DIM_TO_ONE(x,y,RowLength) ((x) + (y) * (RowLength))

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::Initialize()
{
    //
    // Set starting values. All zero except for PC.
    //
    
    ProgramCounter = PROGRAM_START_LOCATION;
    Opcode = 0;
    IndexRegister = 0;
    StackPointer = 0;
    SoundTimer = 0;
    DelayTimer = 0;
    
    //Stack.push_back(ProgramCounter);
    
    memset(VRegisters, 0, 16 * sizeof(char));
    memset(Graphics, 0, 64 * 32 * sizeof(char));
    memset(Key, 0, 16 * sizeof(char));
    memset(Memory, 0, 4096 * sizeof(unsigned char));
    
    //
    // Load fonts into memory.
    //
    
    memcpy(Memory, chip8_fontset, 80 * sizeof(char));
    FontsetLocation = Memory;
    
    //
    // Seed random number generator
    //
    
    std::srand((unsigned int)std::time(0));
    
}

bool Chip8::EmulateCycle(unsigned char* KeyboardState)
{
    unsigned short RegisterNum;
    unsigned short RegisterNum1;
    unsigned short RegisterNum2;
    
    unsigned char RandomNumber;
    
    unsigned short UShortValue;
    unsigned char UCharValue;
    
    bool ValidKeyPressed;
    
    SDL_Event Event;
    
    //
    // Decrement Timers
    //
    
    if (SoundTimer > 0) {
        --SoundTimer;
    }
    
    if (DelayTimer > 0) {
        --DelayTimer;
    }
    
    //
    // Opcodes are 2 bytes long, combine the next two entries of the ProgramCounter
    //
    
    if (Memory + ProgramCounter == ProgramEnd) {
        return false;
    }
    
    DbgPrint("Program Counter: 0x%4X\n", ProgramCounter);
    
    Opcode = Memory[ProgramCounter++] << 8 | Memory[ProgramCounter++];
    
    DbgPrint("Opcode: 0x%4X\n", Opcode);
    
    switch (Opcode & FIRST_FOUR_BITMASK) {
            
        case 0x0000:
            switch (Opcode & LAST_EIGHT_BITMASK) {
                case 0xE0:
                    
                    //
                    // Clear the screen
                    //
                    DbgPrint("0x%4X: Clear the screen\n", Opcode);
                    
                    memset(Graphics, 0, 64 * 32 * sizeof(char));
                    
                    DrawFlag = true;
                    
                    break;
                    
                case 0xEE:
                    
                    //
                    // Return from subroutine
                    //
                    DbgPrint("0x%4x: Return from subroutine\n", Opcode);
                    
                    assert(!Stack.empty());
                    
                    ProgramCounter = Stack.back();
                    Stack.pop_back();
                    
                default:
                    break;
            }
            break;
            
        case 0x1000:
            
            //
            // Jump to address
            //
            DbgPrint("0x%4X: Jump to address\n", Opcode);
            
            UShortValue = Opcode & LAST_TWELVE_BITMASK;
            ProgramCounter = UShortValue;
            
            
            break;
            
        case 0x2000:
            
            //
            // Call Subroutine
            //
            DbgPrint("0x%4X: Call subroutine\n", Opcode);
            
            Stack.push_back(ProgramCounter);
            UShortValue = Opcode & LAST_TWELVE_BITMASK;
            ProgramCounter = UShortValue;
            break;
            
        case 0x3000:
            
            //
            // Skip next instruction if register equals value
            //
            DbgPrint("0x%4X: Skip next instruction if register equals value\n", Opcode);
            
            RegisterNum = GetRegister(First);
            
            if (VRegisters[RegisterNum] == (Opcode & LAST_EIGHT_BITMASK)) {
                SkipNextInstruction();
            }
            break;
            
        case 0x4000:
            
            //
            // Skip next instruction if register doesn't equal value
            //
            DbgPrint("0x%4X: Skip next instruction if register doesn't equal value\n", Opcode);
            
            RegisterNum = GetRegister(First);
            
            if (VRegisters[RegisterNum] != (Opcode & LAST_EIGHT_BITMASK)) {
                SkipNextInstruction();
            }
            break;
            
        case 0x5000:
            
            //
            // Skip next instruction if registers are equal
            //
            DbgPrint("0x%4X: Skip next instruction if registers are equal\n", Opcode);
            
            RegisterNum1 = GetRegister(First);
            RegisterNum2 = GetRegister(Second);
            
            if (VRegisters[RegisterNum1] == VRegisters[RegisterNum2]) {
                SkipNextInstruction();
            }
            break;
            
        case 0x6000:
            
            //
            // Set register to value
            //
            DbgPrint("0x%4X: Set register to value\n", Opcode);
            
            RegisterNum = GetRegister(First);
            VRegisters[RegisterNum] = Opcode & LAST_EIGHT_BITMASK;
            break;
            
        case 0x7000:
            
            //
            // Add value to register
            //
            DbgPrint("0x%4X: Add value to register\n", Opcode);
            
            RegisterNum = GetRegister(First);
            UCharValue = Opcode & LAST_EIGHT_BITMASK;
            
            VRegisters[RegisterNum] += UCharValue;
            break;
            
        case 0x8000:
            
            //
            // Perform register operations
            //
            DbgPrint("0x%4X: Perform register operation\n", Opcode);
            
            RegisterNum1 = GetRegister(First);
            RegisterNum2 = GetRegister(Second);
            
            switch (Opcode & LAST_FOUR_BITMASK) {
                    
                case 0x0:
                    VRegisters[RegisterNum1] = VRegisters[RegisterNum2];
                    break;
                    
                case 0x1:
                    VRegisters[RegisterNum1] = VRegisters[RegisterNum1] | VRegisters[RegisterNum2];
                    break;
                    
                case 0x2:
                    VRegisters[RegisterNum1] = VRegisters[RegisterNum1] & VRegisters[RegisterNum2];
                    break;
                    
                case 0x3:
                    VRegisters[RegisterNum1] = VRegisters[RegisterNum1] ^ VRegisters[RegisterNum2];
                    break;
                    
                case 0x4:
                    CheckAndSetCarry(VRegisters[RegisterNum1],
                                     VRegisters[RegisterNum2],
                                     Add);
                    
                    VRegisters[RegisterNum1] = VRegisters[RegisterNum1] + VRegisters[RegisterNum2];
                    break;
                    
                case 0x5:
                    CheckAndSetCarry(VRegisters[RegisterNum1],
                                     VRegisters[RegisterNum2],
                                     Subtract);
                    
                    VRegisters[RegisterNum1] = VRegisters[RegisterNum1] - VRegisters[RegisterNum2];
                    break;
                    
                default:
                    break;
            }
            
            
            
            break;
            
        case 0x9000:
            
            //
            // Skip next instruction if registers are not equal
            //
            DbgPrint("0x%4X: Skip next instruction if registers are not equal\n", Opcode);
            
            RegisterNum1 = GetRegister(First);
            RegisterNum2 = GetRegister(Second);
            
            if (VRegisters[RegisterNum1] != VRegisters[RegisterNum2]) {
                SkipNextInstruction();
            }
            break;
            
        case 0xA000:
            
            //
            // Set index register to the given address
            //
            DbgPrint("0x%4X: Set index register to the given address\n", Opcode);
            
            IndexRegister = Opcode & LAST_TWELVE_BITMASK;
            break;
            
        case 0xB000:
            
            //
            // Jump to address given plus value in register 0
            //
            DbgPrint("0x%4X: Jump to address given plus value in register 0\n", Opcode);
            
            UShortValue = Opcode & LAST_TWELVE_BITMASK;
            ProgramCounter -= 2;
            ProgramCounter = UShortValue + VRegisters[0];
            break;
            
        case 0xC000:
            
            //
            // Set register to random number and given value
            //
            DbgPrint("0x%4X: Set register to random number and given value\n", Opcode);
            
            RegisterNum = GetRegister(First);
            RandomNumber = (unsigned char) (std::rand() % 255);
            UCharValue = Opcode & LAST_EIGHT_BITMASK;
            
            VRegisters[RegisterNum] = RandomNumber & UCharValue;
            break;
            
        case 0xD000:
            
            //
            // Draw sprites stored at location in index register
            //
            DbgPrint("0x%4X: Draw sprites stored at location in index register\n", Opcode);
            
            DrawSprites();
            
            DrawFlag = true;
            
            break;
            
        case 0xE000:
            
            //
            // Key functions
            //
            
            switch (Opcode & LAST_EIGHT_BITMASK) {
                case 0x9E:
                    
                    //
                    // Skip the next instruction if the key stored in VX is pressed
                    //
                    DbgPrint("0x%4X: Skip the next instruction if the key stored in VX is pressed\n", Opcode);
                    
                    RegisterNum = GetRegister(First);
                    UCharValue = VRegisters[RegisterNum];
                    
                    assert(0 <= UCharValue && UCharValue <= 16);
                    
                    if (KeyboardState[UCharValue] != 0) {
                        //ProgramCounter += 2;
                        SkipNextInstruction();
                    }
                    
                    break;
                    
                case 0xA1:
                    
                    //
                    // Skip the next instruction if the key stored in VX is NOT pressed
                    //
                    DbgPrint("0x%4X: Skip the next instruction if the key stored in VX is NOT pressed\n", Opcode);
                    
                    RegisterNum = GetRegister(First);
                    UCharValue = VRegisters[RegisterNum];
                    
                    assert(0 <= UCharValue && UCharValue <= 16);
                    
                    if (KeyboardState[UCharValue] == 0) {
                        //ProgramCounter += 2;
                        SkipNextInstruction();
                    }                    
                    break;
                    
                default:
                    break;
            }
            
            break;
            
        case 0xF000:
            
            RegisterNum = GetRegister(First);
            
            switch (Opcode & LAST_EIGHT_BITMASK) {
                    
                case 0x07:
                    
                    //
                    // Set register to delay timer value
                    //
                    DbgPrint("0x%4X: Set register to delay timer value\n", Opcode);
                    
                    VRegisters[RegisterNum] = DelayTimer;
                    
                    break;
                    
                case 0x0A:
                    
                    //
                    // A key press is awaited, and then stored in register
                    //
                    DbgPrint("0x%4X: Wait for key press then store value in register\n", Opcode);
                    
                    ValidKeyPressed = false;
                    
                    for (;;) {
                        while (SDL_PollEvent(&Event)) {
                            if (Event.type == SDL_KEYDOWN) {
                                UCharValue = GetKeyNum(Event.key.keysym);
                                
                                if (UCharValue <= 15) {
                                    VRegisters[RegisterNum] = UCharValue;
                                    ValidKeyPressed = true;
                                    break;
                                }
                            }
                        }
                        if (ValidKeyPressed != false) {
                            break;
                        }
                    }
                    
                    break;
                
                case 0x15:
                    
                    //
                    // Set delay timer to register value
                    //
                    DbgPrint("0x%4X: Set delay tmer to register value\n", Opcode);
                    
                    DelayTimer = VRegisters[RegisterNum];
                    break;
                    
                case 0x18:
                    
                    //
                    // Set sound timer to register value
                    //
                    DbgPrint("0x%4X: Set sound timer to register value\n", Opcode);
                    
                    SoundTimer = VRegisters[RegisterNum];
                    break;
                    
                case 0x1E:
                    
                    //
                    // Add register value to index register
                    //
                    DbgPrint("0x%4X: Add register value to index register\n", Opcode);
                    
                    IndexRegister += VRegisters[RegisterNum];
                    break;
                    
                case 0x29:
                    
                    //
                    // Set index register to location of the sprite for the character in VX,
                    //    index register locations are all relevant to where ever memory starts
                    //
                    DbgPrint("0x%4X: Set index register to location of the sprite for the character in VX\n", Opcode);
                    
                    //IndexRegister = VRegisters[RegisterNum] * CHARACTER_SPRITE_SIZE;
                    UCharValue = VRegisters[RegisterNum];
                    Uint32 CharacterIndex;
                    
                    if ('0' <= UCharValue && UCharValue <= '9') {
                        CharacterIndex = UCharValue - '0';
                        
                    } else if ('a' <= UCharValue && UCharValue <= 'f') {
                        CharacterIndex = UCharValue - 'a' + 10;
                        
                    } else if ('A' <= UCharValue && UCharValue <= 'F') {
                        CharacterIndex = UCharValue - 'A' + 10;
                        
                    } else {
                        CharacterIndex = 0;
                    }
                    
                    IndexRegister = CharacterIndex * CHARACTER_SPRITE_SIZE;
                    break;
                    
                case 0x33:
                    
                    //
                    // Put decimal representation of register value into memory at index register
                    //
                    DbgPrint("0x%4X: Put decimal representation of register value into memory at index register\n", Opcode);
                    
                    Memory[IndexRegister] = '0' + ((unsigned char) VRegisters[RegisterNum] / 100);
                    Memory[IndexRegister + 1] = '0' + (((unsigned char) VRegisters[RegisterNum] / 10) % 10);
                    Memory[IndexRegister + 2] = '0' + ((unsigned char) VRegisters[RegisterNum] % 10);
                    break;
                    
                case 0x55:
                    
                    //
                    // Store V0 to VX into memory starting at index register
                    //
                    DbgPrint("0x%4X: Store Vregisters into memory starting at index register\n", Opcode);
                    
                    memcpy(Memory + IndexRegister, VRegisters, (RegisterNum + 1) * sizeof(unsigned char));
                    break;
                    
                case 0x65:
                    
                    //
                    // Put memory starting at index register into V0 to VX
                    //
                    DbgPrint("0x%4X: Put memory starting at index register into Vregisters\n", Opcode);
                    
                    memcpy(VRegisters, Memory + IndexRegister, (RegisterNum + 1) * sizeof(unsigned char));
                    
                default:
                    break;
            }
            
            
        default:
            break;
    }
    
    DebugDumpState();
    
    return true;
}

//
// Subroutine for drawing sprites. There's no color in chip 8, so pixels are just bits on or off.
// We have our graphics pixel array represented as unsigned chars though, se we'll only use the first
// bit in each entry and use a mask to iterate bit by bit through the sprites stored in memory in order
// to XOR them into the graphics array.
//
void Chip8::DrawSprites()
{
    unsigned char SpritePixelBitMask;
    unsigned char SpritePixelValue;
    int GraphicsDrawLoc;
    
    unsigned char SpriteRows = Opcode & LAST_FOUR_BITMASK;
    unsigned short RegisterNum1 = GetRegister(First);
    unsigned short RegisterNum2 = GetRegister(Second);
    
    unsigned short DrawLocX = VRegisters[RegisterNum1];
    unsigned short DrawLocY = VRegisters[RegisterNum2];
    
    SetCarry(0);
    
    for (int SpriteRowIndex = 0; SpriteRowIndex < SpriteRows; ++SpriteRowIndex) {
        
        //
        // Get sprite row from memory
        //
        
        unsigned char SpritePixelRow = Memory[IndexRegister + SpriteRowIndex];
        
        //
        // Iterate through pixels in sprite row and XOR them into graphics array
        //
        
        for (int SpritePixelIndex = 0; SpritePixelIndex < 8; ++SpritePixelIndex) {
            
            if (DrawLocY + SpriteRowIndex > 32 || DrawLocX + SpritePixelIndex > 64) {
                continue;
            }
            
            GraphicsDrawLoc = TWO_DIM_TO_ONE(DrawLocX + SpritePixelIndex, DrawLocY + SpriteRowIndex, GRAPHICS_X_AXIS);
            
            SpritePixelBitMask = 1 << (7 - SpritePixelIndex);
            SpritePixelValue = (SpritePixelRow & SpritePixelBitMask) >> (7 - SpritePixelIndex);
            
            if (SpritePixelValue != 1) {
                continue;
            }
            
            if (Graphics[GraphicsDrawLoc] == 1) {
                SetCarry(1);
            }
            
            Graphics[GraphicsDrawLoc] ^= SpritePixelValue;
        }
    }
}

//
// Parse the registers out of the opcode
//
unsigned short Chip8::GetRegister(RegisterLocationInOpcode RegLoc)
{
    unsigned char RegisterNum;
    assert (RegLoc == First || RegLoc == Second);
    
    if (RegLoc == First) {
        RegisterNum = (Opcode & REGISTER_ONE_BITMASK) >> 8;
    }
    else {
        RegisterNum = (Opcode & REGISTER_TWO_BITMASK) >> 4;
    }
    
    assert(0 <= RegisterNum && RegisterNum <= 16);
    
    return RegisterNum;
}


//
// Check if values result in carry or borrow
// If operation is subtract, takes form of Value1 - Value2
//
void Chip8::CheckAndSetCarry(unsigned short Value1,
                             unsigned short Value2,
                             RegisterOperation Operator)
{
    assert (Operator == Add || Operator == Subtract);
    
    if (Operator == Add) {
        
        if ((int)((int)Value1 + (int)Value2) > 255) {
            SetCarry(1);
        } else {
            SetCarry(0);
        }
    }
    else if (Operator == Subtract) {
        
        if ((int)((int)Value1 - (int)Value2) < 0) {
            SetCarry(0);
        }
        else {
            SetCarry(1);
        }
    }
}


//
// Set the carry register to 1
//
void Chip8::SetCarry(int OneOrZero)
{
    assert(OneOrZero == 0 || OneOrZero == 1);
    VRegisters[0xF] = OneOrZero;
}


//
// Increment the program counter to skip the next instruction
//
void Chip8::SkipNextInstruction()
{
    ProgramCounter += 2;
}


//
// Load ROM into memory.
//
bool Chip8::LoadRom (char* FileName)
{
    std::ifstream RomFile;
    char *MemoryLocation;
    
    RomFile.open(FileName, std::ios::binary | std::ios::in);
    
    if (!RomFile.is_open()) {
        assert(false);
    }
    
    MemoryLocation = (char *) (Memory + PROGRAM_START_LOCATION);
    
    while (RomFile.read(MemoryLocation, 1024)) {
        MemoryLocation += 1024;
    }
    
    MemoryLocation--;
    ProgramEnd = (unsigned char *) MemoryLocation;
    
    return true;
    
}

unsigned char Chip8::GetKeyNum(SDL_Keysym Symbol)
{
    switch (Symbol.sym) {
            
        case SDLK_1:
            return 0;
        case SDLK_2:
            return 1;
        case SDLK_3:
            return 2;
        case SDLK_4:
            return 3;
            
        case SDLK_q:
            return 4;
        case SDLK_w:
            return 5;
        case SDLK_e:
            return 6;
        case SDLK_r:
            return 7;
            
        case SDLK_a:
            return 8;
        case SDLK_s:
            return 9;
        case SDLK_d:
            return 10;
        case SDLK_f:
            return 11;
            
        case SDLK_z:
            return 12;
        case SDLK_x:
            return 13;
        case SDLK_c:
            return 14;
        case SDLK_v:
            return 15;
        
        //
        // No valid key was touched
        //
            
        default:
            return 0xFF;
    }
}

void Chip8::HandleKeyboard(unsigned char Key, int x, int y)
{
    return;
}

//
// Dump the state of the machine
//
void Chip8::DebugDumpState()
{
    if (!DEBUG) {
        return;
    }
    
    printf("Registers:\n"
           "    V0: %4X\n"
           "    V1: %4X\n"
           "    V2: %4X\n"
           "    V3: %4X\n"
           "    V4: %4X\n"
           "    V5: %4X\n"
           "    V6: %4X\n"
           "    V7: %4X\n"
           "    V8: %4X\n"
           "    V9: %4X\n"
           "    VA: %4X\n"
           "    VB: %4X\n"
           "    VC: %4X\n"
           "    VD: %4X\n"
           "    VE: %4X\n"
           "    VF: %4X\n",
           VRegisters[0],
           VRegisters[1],
           VRegisters[2],
           VRegisters[3],
           VRegisters[4],
           VRegisters[5],
           VRegisters[6],
           VRegisters[7],
           VRegisters[8],
           VRegisters[9],
           VRegisters[10],
           VRegisters[11],
           VRegisters[12],
           VRegisters[13],
           VRegisters[14],
           VRegisters[15]);
    
    printf("IndexRegister:  %4X\n"
           "ProgramCounter: %4X\n"
           "Opcode:         %4X\n",
           IndexRegister,
           ProgramCounter,
           Opcode);
    
    printf("Stack:\n");
    
    for (auto i = Stack.rbegin(); i != Stack.rend(); ++i) {
        printf("%4X\n", *i);
    }
    
    printf("\n\n");
}






