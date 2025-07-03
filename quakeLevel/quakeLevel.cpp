#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define MAP_WIDTH  30
#define MAP_HEIGHT 30

char levelMap[MAP_HEIGHT][MAP_WIDTH];

float playerRadius = 0.4f;
float playerHeight = 1.8f;

void GenerateProceduralMap() {
    srand(time(NULL));
    for (int z = 0; z < MAP_HEIGHT; z++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // Walls on edges
            if (x == 0 || z == 0 || x == MAP_WIDTH - 1 || z == MAP_HEIGHT - 1) {
                levelMap[z][x] = '#';
            }
            else {
                // Random wall or floor
                levelMap[z][x] = (rand() % 100 < 20) ? '#' : '.';
            }
        }
    }
    // Make player start position open
    levelMap[1][1] = '.';
}

bool IsPositionBlocked(Vector3 pos) {
    for (int z = 0; z < MAP_HEIGHT; z++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (levelMap[z][x] == '#') {
                Vector3 cubePos = Vector3{ x * 2.0f, 1.0f, z * 2.0f };
                Vector3 cubeExtent = Vector3{ 1.0f, 1.0f, 1.0f };

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

    InitWindow(screenWidth, screenHeight, "Raylib FPS - Procedural Map + Minimap");
    DisableCursor();

    GenerateProceduralMap();

    Camera3D camera = { 0 };
    camera.position = Vector3{ 2.0f, 1.8f, 2.0f };
    camera.target = Vector3Add(camera.position, Vector3{ 0.0f, 0.0f, -1.0f });
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);
    float pitchAngle = 0.0f;

    while (!WindowShouldClose()) {
        Vector3 direction = Vector3Subtract(camera.target, camera.position);
        direction = Vector3Normalize(direction);

        Vector3 flatForward = direction;
        flatForward.y = 0;
        flatForward = Vector3Normalize(flatForward);

        Vector3 right = Vector3Normalize(Vector3CrossProduct(flatForward, camera.up));

        Vector3 newPosition = camera.position;
        float speed = 0.1f;

        if (IsKeyDown(KEY_W)) newPosition = Vector3Add(newPosition, Vector3Scale(flatForward, speed));
        if (IsKeyDown(KEY_S)) newPosition = Vector3Subtract(newPosition, Vector3Scale(flatForward, speed));
        if (IsKeyDown(KEY_A)) newPosition = Vector3Subtract(newPosition, Vector3Scale(right, speed));
        if (IsKeyDown(KEY_D)) newPosition = Vector3Add(newPosition, Vector3Scale(right, speed));

        if (!IsPositionBlocked(newPosition)) {
            camera.position = newPosition;
        }

        Vector2 mouseDelta = GetMouseDelta();
        float sensitivity = 0.003f;
        float yaw = -mouseDelta.x * sensitivity;
        float pitch = -mouseDelta.y * sensitivity;

        pitchAngle += pitch;
        float pitchLimit = DEG2RAD * 89.0f;
        if (pitchAngle > pitchLimit) {
            pitch = pitchLimit - (pitchAngle - pitch);
            pitchAngle = pitchLimit;
        }
        if (pitchAngle < -pitchLimit) {
            pitch = -pitchLimit - (pitchAngle - pitch);
            pitchAngle = -pitchLimit;
        }

        Matrix rotYaw = MatrixRotateY(yaw);
        direction = Vector3Transform(direction, rotYaw);
        Vector3 rightAxis = Vector3Normalize(Vector3CrossProduct(direction, camera.up));
        Matrix rotPitch = MatrixRotate(rightAxis, pitch);
        direction = Vector3Transform(direction, rotPitch);

        camera.target = Vector3Add(camera.position, direction);

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);

        for (int z = 0; z < MAP_HEIGHT; z++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                Vector3 cubePos = Vector3{ x * 2.0f, 1.0f, z * 2.0f };
                if (levelMap[z][x] == '#') {
                    DrawCube(cubePos, 2.0f, 4.0f, 2.0f, DARKGRAY);
                }
                else {
                    DrawCube(cubePos, 2.0f, 0.1f, 2.0f, LIGHTGRAY);
                }
            }
        }

        EndMode3D();

        // Minimap
        for (int z = 0; z < MAP_HEIGHT; z++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                Color color = (levelMap[z][x] == '#') ? DARKGRAY : LIGHTGRAY;
                DrawRectangle(10 + x * 4, 500 + z * 4, 4, 4, color);
            }
        }

        // Player marker on minimap
        int px = (int)(camera.position.x / 2.0f);
        int pz = (int)(camera.position.z / 2.0f);
        DrawRectangle(10 + px * 4, 500 + pz * 4, 4, 4, RED);

        DrawText("WASD to move, Mouse to look", 10, 10, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
