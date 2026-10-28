// In the Name of God, the Most Gracious, Most Merciful

#include "raylib.h"
#include "raymath.h"

// Simulation constants
const int MAX_RAYS = 30;
const int SPACING_BETWEEN_RAYS = 10;
const float LIGHT_SPEED = 6.0f;
const float GRAVITY_STRENGTH = 1600.0f;
const float CLOSE_RANGE_BOOST_STRENGTH = 50000.0f;
const float RAY_TAIL_LENGTH = 2.0f;
const int MAX_BACKGROUND_STARS = 100;

// Light ray struct
typedef struct {
    Vector2 position;
    Vector2 oldPosition;
    Vector2 velocity;
    Vector2 tail;
    float currentSpeed;
    bool active;
} LightRay;

// A struct containing black hole data
typedef struct {
    Vector2 position;
    Color color;
    float eventHorizon;
    float photonSphere;
} BlackHole;

// A struct for background stars
typedef struct {
    Vector2 position;
    Color color;
} BackgroundStar;

int main(void) {
    // Window properties
    const int screenWidth = 800;
    const int screenHeight = 500;
    char projectName[] = "Retro Black Hole Simulation";

    // Window initialization
    InitWindow(screenWidth, screenHeight, projectName);

    // A canvas for texture drawing
    RenderTexture2D rayTrailCanvas = LoadRenderTexture(screenWidth, screenHeight);

    // Makes pixels sharp and crisp
    SetTextureFilter(rayTrailCanvas.texture, TEXTURE_FILTER_POINT);

    // Stops Raylib from pulling color data from off screen points
    SetTextureWrap(rayTrailCanvas.texture, TEXTURE_WRAP_CLAMP);

    // Initialize the canvas
    BeginTextureMode(rayTrailCanvas);
    ClearBackground(BLACK);
    EndTextureMode();

    // Background space color
    Color spaceColor = (Color){ 27, 12, 12, 255 };

    // Bloom shader for glowing ray trails
    // The path is relative to the location of the compiled binary
    Shader bloomShader = LoadShader(0, "../../../../resources/shaders/bloom.fs");

    // An array of rays
    LightRay rays[MAX_RAYS];

    // Initialization of rays
    for (int i = 0; i < MAX_RAYS; i++) {
        rays[i] = (LightRay){ 0 };

        float rayPositionX = 0.0f;
        // The SPACING_BETWEEN_RAYS constant and the hard-coded 105 can be changed
        // for different visual effects
        float rayPositionY = (i * SPACING_BETWEEN_RAYS) + 105;

        // Ray's starting position
        rays[i].position = (Vector2){ rayPositionX, rayPositionY };
        // Ray's initial velocity
        rays[i].velocity = (Vector2){ LIGHT_SPEED, 0.0f };
        // Each ray is active by default
        rays[i].active = true;
    }

    // Color of the ray
    Color rayColor = (Color){ 254, 98, 68, 255 };
    // Color of the rays' trails of light
    Color rayTrailColor = (Color){ 255, 0, 0, 255 };

    // Black hole
    BlackHole blackHole = { 0 };
    blackHole.position = (Vector2){ screenWidth / 2, screenHeight / 2 };
    blackHole.color = (Color){ 150, 150, 150, 255 };
    blackHole.eventHorizon = 20.0f;
    blackHole.photonSphere = blackHole.eventHorizon * 1.5f;
    
    // Background stars initialization
    BackgroundStar backgroundStars[MAX_BACKGROUND_STARS];
    for (int i = 0; i < MAX_BACKGROUND_STARS; i++) {
        backgroundStars[i].position = (Vector2){ 
            (float) GetRandomValue(0, screenWidth),
            (float) GetRandomValue(0, screenHeight) 
        };
        int starBrightness = GetRandomValue(120, 220);
        backgroundStars[i].color = (Color){ starBrightness, starBrightness, starBrightness + 35, 255};
    }

    // Target frame-per-second
    SetTargetFPS(60);

    // Main loop
    while (!WindowShouldClose()) {
        // Physics calculations on each individual ray
        for (int i = 0; i < MAX_RAYS; i++) {
            // Difference between the black hole position and the ray position
            Vector2 difference = Vector2Subtract(blackHole.position, rays[i].position);
            // Actual distance between the black hole and the ray
            float distance = sqrtf(difference.x * difference.x + difference.y * difference.y);

            // Checking if the ray goes off screen or into the black hole
            if (distance <= blackHole.eventHorizon) {
                rays[i].active = false;
            } else if (rays[i].position.x < 0 || rays[i].position.x > screenWidth) {
                rays[i].active = false;
            } else if (rays[i].position.y < 0 || rays[i].position.y > screenHeight) {
                rays[i].active = false;
            }

            // Only do movement calculations if the ray is active
            if (rays[i].active) {
                // Direction towards the black hole
                Vector2 directionToBlackHole = (Vector2){ difference.x / distance, difference.y / distance };
                // Standard gravity pull on the ray
                float standardGravityPull = GRAVITY_STRENGTH / (distance * distance);
                // Gravity pull once the ray enters the black hole's photon sphere
                // Calculated in the below if statement
                float closeRangeBoost = 0.0f;

                if (distance < blackHole.photonSphere) {
                    // Increase in gravity pull if the ray enters the black hole's photon sphere
                    closeRangeBoost = CLOSE_RANGE_BOOST_STRENGTH / (distance * distance * distance);
                }

                // Total gravity pull on the ray
                float totalGravity = standardGravityPull + closeRangeBoost;

                // Updating the ray's position
                rays[i].velocity.x += directionToBlackHole.x * totalGravity;
                rays[i].velocity.y += directionToBlackHole.y * totalGravity;

                // Magnitude of the velocity vector
                // This is used further down to normalize the speed of the ray
                // So it does not speed up breaking physics
                rays[i].currentSpeed = sqrtf(
                    rays[i].velocity.x * rays[i].velocity.x
                    +
                    rays[i].velocity.y * rays[i].velocity.y
                );

                // Ray's velocity update while making sure it does not speed up
                rays[i].velocity.x = (rays[i].velocity.x / rays[i].currentSpeed) * LIGHT_SPEED;
                rays[i].velocity.y = (rays[i].velocity.y / rays[i].currentSpeed) * LIGHT_SPEED;
            }

            // Storing the ray's old position needed for the trail to work
            rays[i].oldPosition = rays[i].position;
            // Ray's position update
            rays[i].position = Vector2Add(rays[i].position, rays[i].velocity);
            // Calculations for the ray's tail so it is not a tiny dot
            rays[i].tail = (Vector2){
                rays[i].position.x - rays[i].velocity.x * RAY_TAIL_LENGTH,
                rays[i].position.y - rays[i].velocity.y * RAY_TAIL_LENGTH
            };
        }

        // Drawing
        BeginDrawing();

        ClearBackground(spaceColor);
        
        DrawText(projectName, 20, 20, 20, (Color){ 255, 122, 48, 255 });

        // Drawing background stars
        for (int i = 0; i < MAX_BACKGROUND_STARS; i++) {
            DrawPixelV(backgroundStars[i].position, backgroundStars[i].color);
        }

        // Draw rays and their trails only if the ray is active
        for (int i = 0; i < MAX_RAYS; i++) {
            if (rays[i].active) {
                // Drawing the ray
                DrawLineV(rays[i].tail, rays[i].position, rayColor);
                // Drawing the ray's trail texture on the trail canvas
                if (rays[i].oldPosition.x != rays[i].position.x || rays[i].oldPosition.y != rays[i].position.y) {
                    BeginTextureMode(rayTrailCanvas);
                    DrawLineEx(rays[i].oldPosition, rays[i].position, 1.0f, rayTrailColor);
                    EndTextureMode();
                }
            }
        }

        // Rendering the texture, using negative height because Raylib textures are vertically flipped by default
        BeginBlendMode(BLEND_ADDITIVE);

        BeginShaderMode(bloomShader);

        DrawTextureRec(
            rayTrailCanvas.texture,
            (Rectangle){ 0.0f, 0.0f, (float) rayTrailCanvas.texture.width, (float) -rayTrailCanvas.texture.height },
            (Vector2){ 0.0f, 0.0f },
            WHITE
        );

        EndShaderMode();

        EndBlendMode();

        // Drawing black hole
        DrawCircleV(blackHole.position, blackHole.eventHorizon, BLACK);

        EndDrawing();
    }

    // De-initialization and cleanup
    UnloadShader(bloomShader);
    UnloadRenderTexture(rayTrailCanvas);
    CloseWindow();
    return 0;
}
