// Include files
#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include <stdio.h>// For debugging in console
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

// Global Variable
surface_t *depthBuffer;
T3DViewport viewport;
T3DModel *flatplane;
T3DModel *sphere_model;
rspq_block_t *sphere_dl;
T3DMat4FP* sphere_matrix;
joypad_port_t port; // Needed for controls
float speed = 0.0f; // For updating speed of mesh
T3DVec3 newDir = {0}; // For updating postion of mesh
T3DVec3 moveDir;
float currSpeed;
T3DVec3 spherePos;

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
    sphere_model = t3d_model_load("rom:/FinishLineFrenzy/sphere.t3dm");
    flatplane = t3d_model_load("rom:/FinishLineFrenzy/flatplane.t3dm");

    // Draw the flatplane aka map
    //t3d_model_draw(flatplane);

    //UNKNOWN
    sphere_matrix = (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP));

    t3d_matrix_set(sphere_matrix, true);

    // Initialize the starting point of the sphere mesh matrix
    t3d_mat4fp_from_srt_euler(
        sphere_matrix,
        (float[3]){1, 1, 1}, // Scale
        (float[3]){0, 0, 0}, // Rotation
        //Position X  Y  Z
        (float[3]){0, // X (Positive: Left, Max: 265 Negative: Right Min: -265. Measured from middle of object? Based on Blender Origin
                   0, // Z (Closer or further away from camera)
                   360  // Y (Positive: Up, Max: 360 / Negative: Down, Min: -360 Image Warped. Measured from middle of object? Based on Blender Origin
        }  

    );

    // draw the sphere aka actor
    rspq_block_begin();
    t3d_matrix_set(sphere_matrix, true);
    t3d_model_draw(sphere_model);
    sphere_dl = rspq_block_end();
    
    //Set time to 1 seconds. When time runs out game ends 
    timer = 100.0f;

}

void minigame_fixedloop(float deltatime)
{
    
    timer -= deltatime;

    // use stdio to print out timeleft as error
    //fprintf(stderr, "Time left %f\n", timer);

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
    t3d_viewport_set_projection(&viewport, 
                                T3D_DEG_TO_RAD(90.0f), // Feild of View
                                20.0f,  // Near Plane distance (i.e. Near Clipping)
                                160.0f //Far plane distance >= 40 (i.e. Far Clipping)
    );
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
    
	t3d_matrix_push_pos(1);

    rspq_block_run(sphere_dl);

	t3d_matrix_pop(1);

    // For detecting if a player hit start
    joypad_buttons_t btn = joypad_get_buttons_pressed(port);

    // If the player presses start end game
	if (btn.start) minigame_end();

    // For detecting if the player uses the joystick
    joypad_inputs_t joypad = joypad_get_inputs(port);

    newDir.v[0] = (float)joypad.stick_x * 0.05f;
    newDir.v[2] = -(float)joypad.stick_y * 0.05f;
    
    // For tracking joystick movements
    //fprintf(stderr, "Joystick X %f\n", newDir);
    
    // Only update speed if a non-zero value is detected. i.e. when inputs are pushed for controler 1
    if (newDir.v[0] != 0 || newDir.v[2] != 0) {
        speed = sqrtf(t3d_vec3_len2(&newDir));
    };

    // Player movement
    if(speed > 0.15f) {
    newDir.v[0] /= speed;
    newDir.v[2] /= speed;
    moveDir = newDir;

    currSpeed = t3d_lerp(currSpeed, speed * 0.3f, 0.15f);
  } else {
    currSpeed *= 0.64f;
  }

    // move player...
    spherePos.v[0] += moveDir.v[0] * currSpeed;
    spherePos.v[2] += moveDir.v[2] * currSpeed;


    // For tracking joystick movements
    fprintf(stderr, "Check Speed %f\n", spherePos.v[0]);
    
    // Update player matrix. Based on user inputs
    t3d_mat4fp_from_srt_euler(
    sphere_matrix,
    (float[3]){1, 1, 1}, // Scale
    (float[3]){0, 0, 0}, // Rotation
    //Position X  Y  Z
    (float[3]){spherePos.v[0], // X (Positive: Left, Max: 265 Negative: Right Min: -265. Measured from middle of object? Based on Blender Origin
               0, // Z (Closer or further away from camera)
               spherePos.v[2] // Y (Positive: Up, Max: 360 / Negative: Down, Min: -360 Image Warped. Measured from middle of object? Based on Blender Origin
        }  

    ); 
    
    
    rdpq_detach_show();
    

}

void minigame_cleanup()
{

    // Remove tiny3d object (unregister)
    t3d_destroy();

    // Removed graphics from blender 
    t3d_model_free(sphere_model);
    t3d_model_free(flatplane);

    // Remove rspq block???
    rspq_block_free(sphere_dl);

    // Remove minigame display object 
    display_close();

    free_uncached(sphere_matrix);

}
