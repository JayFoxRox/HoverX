#include <stdbool.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>

#include "draw.h"
#include "vec3.h"

int main() {

  SDL_SetHint("SDL_HINT_NO_SIGNAL_HANDLERS", "1");

  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Window *window = SDL_CreateWindow("HoverX!", 0, 0, 640, 480, SDL_WINDOW_OPENGL);
    
  // Create an OpenGL context associated with the window.
  SDL_GLContext glcontext = SDL_GL_CreateContext(window);

  while(true) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      //FIXME: Handle?
    }


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw some X pattern
    Vec3 tl = { -1.0f,  1.0f, 0.0f };
    Vec3 tr = {  1.0f,  1.0f, 0.0f };
    Vec3 bl = { -1.0f, -1.0f, 0.0f };
    Vec3 br = {  1.0f, -1.0f, 0.0f };
    drawDebugLine(&tl, &br);
    drawDebugLine(&tr, &bl);

    // Get current time
    float t = SDL_GetTicks() / 1000.0f;

    // Create a fake-player state on a fixed trajectory
    const float radius = 0.1f;
    static Vec3 position = { 0.0f, 0.0f, 0.0f };
    static Vec3 velocity = { 0.0f, 0.0f, 0.0f };
    static float spin = 0.0f;
    static float angle = 0.0f;

    // Get vector for heading
    Vec3 heading = { 0.0f, 0.0f, 0.0f };
    pointOnCircle(&heading.x, &heading.y, angle, 1.0f);

    // Get input
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // Modify spin based on input
    if (state[SDL_SCANCODE_RIGHT]) {
      spin += 0.5f;
    }
    if (state[SDL_SCANCODE_LEFT]) {
      spin -= 0.5f;
    }

    const float top_speed = 0.01f; // Speed clamp
    const float acceleration = top_speed / 60.0f; // Speed addition per frame
    const float drag = acceleration / 10.0f; // How much speed is removed per frame [pre-movement]
    const float momentum = 95.0f / 100.0f; // How much spin is kept [1.0 = 100%]
    const float top_spin = 0.05f; // Spin clamp

    // Limit spin
    if (spin >  top_spin) { spin =  top_spin; }
    if (spin < -top_spin) { spin = -top_spin; }

    // Change angle based on spin
    angle += spin;

    // Reduce spin over time
    spin = spin * momentum;

    // Get acceleration speed based on input
    float speed = 0.0f;
    if (state[SDL_SCANCODE_UP]) {
      speed += acceleration;
    }
    if (state[SDL_SCANCODE_DOWN]) {
      speed -= acceleration;
    }

    // Add acceleration for this frame
    velocity.x += heading.x * speed;
    velocity.y += heading.y * speed;

    // Now get actual speed
    float old_speed = vec3_length(&velocity);
    if (old_speed >= 1.0e-9f) {
      float new_speed = old_speed;

      // Add some drag to limit speed
      if (new_speed > 0.0f) {
        new_speed -= drag;
      }

      // Limit speed
      if (fabsf(new_speed) > top_speed) {
        new_speed = top_speed;
      }

      // Normalize and scale with new speed
      vec3_scale(&velocity, &velocity, 1.0f / old_speed * new_speed);
    }

    // Calculate new position
    vec3_add(&position, &position, &velocity);

    // Draw something like a player 
    drawDebugCircle(&position, radius);

    // Mark direction of heading
    Vec3 tip;
    vec3_add(&tip, &position, &heading);
    drawDebugLine(&position, &tip);

    // Mark direction of travel
    Vec3 scaled_velocity;
    vec3_scale(&scaled_velocity, &velocity, 10.0f);
    vec3_add(&tip, &position, &scaled_velocity);
    drawDebugLine(&position, &tip);


    // Display new image
    SDL_GL_SwapWindow(window);
  }

  // Once finished with OpenGL functions, the SDL_GLContext can be deleted.
  SDL_GL_DeleteContext(glcontext);  
  
  return 0;  
}
