//
//  Graphics.h
//  Chip8Emulator
//
//  Created by Thomas Miller on 7/9/14.
//  Copyright (c) 2014 Thomas Miller. All rights reserved.
//

#ifndef __Chip8Emulator__Graphics__
#define __Chip8Emulator__Graphics__

#include <iostream>
#include <SDL2/SDL.h>

#define SCREEN_X_AXIS (64 * 8)
#define SCREEN_Y_AXIS (32 * 8)

class Graphics {
    
private:
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_Texture *Texture;
    Uint32 *Pixels; //[SCREEN_X_AXIS * SCREEN_Y_AXIS];
    
    void TransferGraphicsToPixels(const unsigned char *Graphics);
    void ColorEightbyEightBlock(int xPos, int yPos, Uint32 Color);
    
public:
    void Initialize();
    void Draw(const unsigned char *Graphics);
    
    
};

#endif /* defined(__Chip8Emulator__Graphics__) */
