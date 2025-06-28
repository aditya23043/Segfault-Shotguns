
#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>

#define MAP_WIDTH 10
#define MAP_HEIGHT 10

// Level layout: '#' = wall, '.' = floor
const char levelMap[MAP_HEIGHT][MAP_WIDTH + 1] = {
    "##########",
    "#........#",
    "#..####..#",
    "#..#..#..#",
    "#..#..#..#",
    "#..####..#",
    "#........#",
    "#..##....#",
    "#........#",
    "##########"
};

// Player collider size
float playerRadius = 0.4f;
float playerHeight = 1.8f;

// Check if player is colliding with wall blocks
bool IsPositionBlocked(Vector3 pos) {
    for (int z = 0; z < MAP_HEIGHT; z++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (levelMap[z][x] == '#') {
                Vector3 cubePos = { x * 2.0f, 1.0f, z * 2.0f };
                Vector3 cubeExtent = { 1.0f, 1.0f, 1.0f };

                BoundingBox wallBox;
                wallBox.min = Vector3Subtract(cubePos, cubeExtent);
                wallBox.max = Vector3Add(cubePos, cubeExtent);

                BoundingBox playerBox;
                playerBox.min.x = pos.x - playerRadius;
                playerBox.min.y = pos.y - playerHeight / 2.0f;
                playerBox.min.z = pos.z - playerRadius;
                playerBox.max.x = pos.x + playerRadius;
                playerBox.max.y = pos.y + playerHeight / 2.0f;
                playerBox.max.z = pos.z + playerRadius;

                if (CheckCollisionBoxes(playerBox, wallBox)) return true;
            }
        }
    }
    return false;
}

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Raylib FPS - Procedural Map with Collision");
    DisableCursor(); // Lock and hide cursor

    Camera3D camera = { 0 };
    Vector3 camPos = { 4.0f, 1.8f, 4.0f };
    Vector3 camTarget = { 4.0f, 1.8f, 3.0f };
    Vector3 camUp = { 0.0f, 1.0f, 0.0f };
    camera.position = camPos;
    camera.target = camTarget;
    camera.up = camUp;
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        Vector3 forward = Vector3Subtract(camera.target, camera.position);
        forward.y = 0;
        forward = Vector3Normalize(forward);
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

        Vector3 newPosition = camera.position;

        if (IsKeyDown(KEY_W)) newPosition = Vector3Add(newPosition, Vector3Scale(forward, 0.1f));
        if (IsKeyDown(KEY_S)) newPosition = Vector3Subtract(newPosition, Vector3Scale(forward, 0.1f));
        if (IsKeyDown(KEY_A)) newPosition = Vector3Subtract(newPosition, Vector3Scale(right, 0.1f));
        if (IsKeyDown(KEY_D)) newPosition = Vector3Add(newPosition, Vector3Scale(right, 0.1f));

        if (!IsPositionBlocked(newPosition)) {
            camera.position = newPosition;
        }

        Vector2 mouseDelta = GetMouseDelta();
        float sensitivity = 0.003f;
        float yaw = -mouseDelta.x * sensitivity;
        float pitch = -mouseDelta.y * sensitivity;

        Matrix rotationYaw = MatrixRotateY(yaw);
        Vector3 direction = Vector3Subtract(camera.target, camera.position);
        direction = Vector3Transform(direction, rotationYaw);

        Vector3 rightAxis = Vector3Normalize(Vector3CrossProduct(direction, camera.up));
        Matrix rotationPitch = MatrixRotate(rightAxis, pitch);
        direction = Vector3Transform(direction, rotationPitch);

        camera.target = Vector3Add(camera.position, direction);

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);

        for (int z = 0; z < MAP_HEIGHT; z++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                Vector3 cubePos = { x * 2.0f, 1.0f, z * 2.0f };
                if (levelMap[z][x] == '#') {
                    DrawCube(cubePos, 2.0f, 2.0f, 2.0f, DARKGRAY); // Wall
                }
                else {
                    DrawCube(cubePos, 2.0f, 0.1f, 2.0f, LIGHTGRAY); // Floor
                }
            }
        }

        EndMode3D();

        DrawText("WASD to move, Mouse to look", 10, 10, 20, BLACK);
        DrawText("ESC to exit", 10, 35, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
