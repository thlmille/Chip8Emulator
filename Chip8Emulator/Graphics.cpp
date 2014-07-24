//
//  Graphics.cpp
//  Chip8Emulator
//
//  Created by Thomas Miller on 7/9/14.
//  Copyright (c) 2014 Thomas Miller. All rights reserved.
//

#include "Graphics.h"
#include "Chip8.h"

#define TWO_DIM_TO_ONE(x,y,RowLength) ((x) + (y) * (RowLength))

void Graphics::Initialize()
{
    DbgPrint("Initializing Graphics Class\n");
    
    Window = NULL;
    Renderer = NULL;
    
    Pixels = (Uint32 *) malloc(SCREEN_X_AXIS * SCREEN_Y_AXIS * sizeof(Uint32));
    assert(Pixels != NULL);
    
    //
    // Set screen to all white to begin with, we'll draw with black just because
    // everyone else draws with white.
    //
    
    memset(Pixels, 0xFF, SCREEN_X_AXIS * SCREEN_Y_AXIS * sizeof(Uint32));
    
    SDL_Init(SDL_INIT_VIDEO);
    
    Window = SDL_CreateWindow("Chip8",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_X_AXIS,
                              SCREEN_Y_AXIS,
                              0);
    
    assert(Window != NULL);
    
    Renderer = SDL_CreateRenderer(Window, -1, 0);
    
    assert(Renderer != NULL);
    
    Texture = SDL_CreateTexture(Renderer,
                                SDL_PIXELFORMAT_ABGR8888,
                                SDL_TEXTUREACCESS_STATIC,
                                SCREEN_X_AXIS,
                                SCREEN_Y_AXIS);
    
    assert(Texture != NULL);
    
    SDL_UpdateTexture(Texture, NULL, Pixels, SCREEN_X_AXIS * sizeof(Uint32));
    SDL_RenderClear(Renderer);
    SDL_RenderCopy(Renderer, Texture, NULL, NULL);
    SDL_RenderPresent(Renderer);
}

void Graphics::Draw(const unsigned char *Graphics)
{
    //
    // Update our pixels array
    //
    
    TransferGraphicsToPixels(Graphics);
    
    //
    // Update our texture and get it onto the screen
    //
    
    SDL_UpdateTexture(Texture, NULL, Pixels, SCREEN_X_AXIS * sizeof(Uint32));
    SDL_RenderClear(Renderer);
    SDL_RenderCopy(Renderer, Texture, NULL, NULL);
    SDL_RenderPresent(Renderer);
    
}

void Graphics::TransferGraphicsToPixels(const unsigned char *Graphics)
{
    //
    // Each "pixel" in our graphics array is going to represent an 8x8 pixel block on
    // our actual screen. Do the array translation to map this.
    //
    
    Uint32 Color;
    
    for (int xIndex = 0; xIndex < GRAPHICS_X_AXIS; ++xIndex) {
        
        for (int yIndex = 0; yIndex < GRAPHICS_Y_AXIS; ++yIndex) {
            
            if (Graphics[TWO_DIM_TO_ONE(xIndex, yIndex, GRAPHICS_X_AXIS)] == 1) {
                Color = 0; // black
            } else {
                Color = 0xFF; // white
            }
            
            ColorEightbyEightBlock(xIndex, yIndex, Color);
        }
    }
}

void Graphics::ColorEightbyEightBlock(int xPos, int yPos, Uint32 Color)
{
    int xPixel = xPos * 8;
    int yPixel = yPos * 8;
    
    for (int i = 0; i < 8; ++i) {
        
        memset(&(Pixels[TWO_DIM_TO_ONE(xPixel, yPixel + i, SCREEN_X_AXIS)]), Color, 8 * sizeof(Uint32));
        
    }
}