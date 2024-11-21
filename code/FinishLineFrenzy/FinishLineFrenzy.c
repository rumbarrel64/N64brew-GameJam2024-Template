#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
// For debugging in console
#include <stdio.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

// Global Variable
surface_t *depthBuffer;
T3DViewport viewport;
T3DModel *sphere;
T3DModel *flatplane;
// Camera Postition
//                 X     Y       Z
T3DVec3 camPos = {{0.0f, 125.0f, -50.0f}};
// Set to origin
T3DVec3 camTarget = {{0.0f, 0.0f, 0.0f}};

const MinigameDef minigame_def = {
    .gamename = "Finish Line Frenzy",
    .developername = "rumbarrel64",
    .description = "Get to the finish line",
    .instructions = "Press A to win."
};

float timer;

void minigame_init()
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    //Initilaize Tiny3d object
    t3d_init((T3DInitParams){});

    depthBuffer = display_get_zbuf();
    viewport = t3d_viewport_create();
    
    //Load in graphics from blender
    sphere = t3d_model_load("rom:/FinishLineFrenzy/sphere.t3dm");
    flatplane = t3d_model_load("rom:/FinishLineFrenzy/flatplane.t3dm");
    
    //Set time to 10 seconds 
    timer = 10.0f;

}

void minigame_fixedloop(float deltatime)
{
    
    timer -= deltatime;

    // use stdio to print out timeleft as error
    fprintf(stderr, "Time left %f\n", timer);

    if(timer <= 0.0f) {
        minigame_end();
    }

}

void minigame_loop(float deltatime)
{

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

    T3DVec3 lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);
    //Setup camera matrix
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 160.0f);
    // Setup View Matrix
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});
    
    // Wait until framebuffer available. Slow down the loop
    rdpq_attach(display_get(), depthBuffer);

    //Unknown
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    // Clears the entire screen with provided color
    //                            R G B
    t3d_screen_clear_color(RGBA32(0,0,102, 0XFF));
    t3d_screen_clear_depth();
    
    // Set Lights
    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);
    
    
    // Draw the flatplane aka map
    t3d_model_draw(flatplane);

    // draw the sphere
    t3d_model_draw(sphere);
    rdpq_detach_show();
    

}

void minigame_cleanup()
{

    // Remove tiny3d object (unregister)
    t3d_destroy();

}