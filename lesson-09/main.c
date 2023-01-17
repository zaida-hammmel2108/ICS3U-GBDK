// Copyright (c) 2023 Zaida Hammel
//
// Created by: Zaida Hammel
// Created on: Jan 2023
// This program is the Space Aliens game for the GameBoy

#include <gb/gb.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "GBDK_Constants.h"
#include "MetaSprites.c"
#include "SpaceAliens.c"
#include "SpaceAliensData.c"
#include "SpaceAliensMap.c"
#include "Background.c"

typedef enum {
    SPLASH,
    START,
    GAME,
    GAME_OVER
} screen_t;

// splash scene
screen_t splash() {
    // user input
    uint8_t joypadData;
    screen_t next_screen = START;

    // time
    unsigned int timeCounter = 0;

    printf("\n\n\n\n\n");
    printf("   MT Game Studios");
    printf("        Presents");
    printf("\n\n\n\n\n\n");
    printf("    Space Aliens");

    DISPLAY_ON;

    // game loop
    while (1) {
        // get d-pad input
        joypadData = joypad();

        // if start button is pressed
        if (joypadData & J_START) {
            return next_screen;
        }

        // move even if start is not pressed after 2 seconds
        timeCounter++;
        if (timeCounter > (3 * 60)) {
            return next_screen;
        }

        // wait until end of frame (1/60th second)
        wait_vbl_done();
    }
}


// start scene
screen_t start() {
    screen_t next_screen = GAME;

    set_bkg_data(0, 215, space_aliens_data);
    set_bkg_tiles(0, 0, 20, 18, space_aliens_map);

    SHOW_BKG; // turn background on
    DISPLAY_ON;

    waitpad(J_START);
    return next_screen;
}


// game scene
screen_t game() {
    // user input
    uint8_t joypadData;

    // check if A button is single pressed
    bool aButtonJustPressed = false;
    bool aButtonStillPressed = false;

    // these registers must be in this specific order
    NR52_REG = 0x80; // is 1000 0000 in binary and turns on sound
    NR50_REG = 0x77; // sets the volume for both left and right channel just set to max 0x77
    NR51_REG = 0xFF; // is 1111 1111 in binary, selects which channels we want to use

    // keep track of meta sprite position, in a variable
    const int FIXED_Y_POSITION_OF_SHIP = 144;
    int shipXPosition = 76;
    int shipYPosition = FIXED_Y_POSITION_OF_SHIP;
    int missiles[5][3] = {
                            {5, 255, 255},
                            {6, 255, 255},
                            {7, 255, 255},
                            {8, 255, 255},
                            {9, 255, 255},
    }; // missile [sprite number, x position, y position]

    // load spritesheet reference
    set_sprite_data(0, 16, SpaceAliens);

    // load sprites for eta sprite
    set_meta_sprite_tile(0, 1, 2, 3, 4);

    // move meta sprite on screen
    move_meta_sprite(0, shipXPosition, shipYPosition);

    // load sprites for the 5 missiles
    // load sprite index #06, missile
    for (int laserCounter = 0; laserCounter < 5; laserCounter++) {
        set_sprite_tile(missiles[laserCounter][0], 6);
    }

    // move the missiles off screen
    for (int laserCounter = 0; laserCounter < 5; laserCounter++) {
        move_sprite(missiles[laserCounter][0],
                    missiles[laserCounter][1],
                    missiles[laserCounter][2]);
    }

    //set background tile sheet 0 to the same tile sheet
    set_bkg_data(0, 16, SpaceAliens);
    // load tiles from (0, 0) to (20, 36) form the tile map called background
    set_bkg_tiles(0, 0, 20, 36, Background);

    SHOW_BKG; // turn background on
    SHOW_SPRITES;
    DISPLAY_ON;

        // game loop
    while (1) {
        // get D-pad input
        joypadData = joypad();

        // if LEFT D-pad is pressed
        if (joypadData & J_LEFT) {
            // move sprite in the index 0 by (x, y) (-1, 0)
            scroll_meta_sprite(0, -1, 0);
            shipXPosition -= 1;
            if (shipXPosition < 8) {
                shipXPosition = 8;
                move_meta_sprite(0, 8, FIXED_Y_POSITION_OF_SHIP);
            }
        }

        // if right d-pad is pressed
        if (joypadData & J_RIGHT) {
            // move sprite in the index 0 by (x, y) (+1, 0)
            scroll_meta_sprite(0, 1, 0);
            shipXPosition += 1;
            if (shipXPosition > SCREEN_WIDTH - 8) {
                shipXPosition = SCREEN_WIDTH - 8;
                move_meta_sprite(0, (uint8_t)(SCREEN_WIDTH - 8), FIXED_Y_POSITION_OF_SHIP);
            }
        }

        // if A button is pressed
        if (joypadData & J_A) {
            if (aButtonJustPressed == true) {
                aButtonStillPressed = true;
            } else {
                // check to see if there are any of the 5 missiles left
                for (int laserCounter = 0; laserCounter < 5; laserCounter++) {
                    if (missiles[laserCounter][1] >= 255) {
                        //fire this laser
                        missiles[laserCounter][1] = shipXPosition + 4;
                        missiles[laserCounter][2] = 136;
                        move_sprite(missiles[laserCounter][0],
                                    missiles[laserCounter][1],
                                    missiles[laserCounter][2]);

                        aButtonJustPressed = true;
                        // play a sound
                        NR10_REG = 0x16;
                        NR11_REG = 0x82;
                        NR12_REG = 0x69;
                        NR13_REG = 0x59;
                        NR14_REG = 0xC6;

                        break;
                    }
                }
            }
        }
        else {
            aButtonJustPressed = false;
            aButtonStillPressed = false;
        }

        // if tehre are any missiles on the screen, move them up
        for (int laserCounter = 0; laserCounter < 5; laserCounter++) {
            if (missiles[laserCounter][1] < 255) {
                // move up
                missiles[laserCounter][2]--;
                move_sprite(missiles[laserCounter][0],
                            missiles[laserCounter][1],
                            missiles[laserCounter][2]);

                // check to see if missile of off the tip of the screen
                if (missiles[laserCounter][2] < 0) {
                    // move off screen again
                    missiles[laserCounter][1] = 255;
                    missiles[laserCounter][2] = 255;
                    move_sprite(missiles[laserCounter][0],
                                missiles[laserCounter][1],
                                missiles[laserCounter][2]);
                }
            }
        }

        // scroll background 0 in the x and -1 in teh y
        scroll_bkg(0, -1);

        // wait until end of frame (1/60th second)
        wait_vbl_done();
    }
}


void main(){
    // Current screen, defaults to TITLE
    screen_t current_screen = SPLASH;

    // these registers must be in this specific order
    NR52_REG = 0x80; // is 1000 0000 in binary and turns on sound
    NR50_REG = 0x77; // sets the volume for both left and right channel just set to max 0x77
    NR51_REG = 0xFF; // is 1111 1111 in binary, selects which channels we want to use

    while(1) {
        if (current_screen == SPLASH) {
            current_screen = splash();
        }
        else if (current_screen == START) {
            current_screen = start();
        }
        else if (current_screen == GAME) {
            current_screen = game();
        }
        else if (current_screen == GAME_OVER) {
            printf("game_over");
            // current_screen = game_over();
        }
    }
}
