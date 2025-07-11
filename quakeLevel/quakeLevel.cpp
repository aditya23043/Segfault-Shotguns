#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAP_WIDTH  30
#define MAP_HEIGHT 30
#define ROOM_ROWS  3
#define ROOM_COLS  3
#define ROOM_W     8
#define ROOM_H     8

char levelMap[MAP_HEIGHT][MAP_WIDTH];

float playerRadius = 0.4f;
float playerHeight = 1.8f;

int currentRoomX = 0;
int currentRoomZ = 0;

float fadeAlpha = 0.0f;
bool fading = false;
bool fadeIn = true;
float fadeSpeed = 0.05f;

void ClearMap() {
    for (int z = 0; z < MAP_HEIGHT; z++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            levelMap[z][x] = '#';
        }
    }
}

void CarveRoom(int roomX, int roomZ) {
    int offsetX = 1 + roomX * (ROOM_W + 1);
    int offsetZ = 1 + roomZ * (ROOM_H + 1);

    for (int z = 0; z < ROOM_H; z++) {
        for (int x = 0; x < ROOM_W; x++) {
            int mapX = offsetX + x;
            int mapZ = offsetZ + z;
            if (mapX < MAP_WIDTH - 1 && mapZ < MAP_HEIGHT - 1) {
                levelMap[mapZ][mapX] = '.';
            }
        }
    }
}

void ConnectRooms(int x1, int z1, int x2, int z2) {
    int cx1 = 1 + x1 * (ROOM_W + 1) + ROOM_W / 2;
    int cz1 = 1 + z1 * (ROOM_H + 1) + ROOM_H / 2;
    int cx2 = 1 + x2 * (ROOM_W + 1) + ROOM_W / 2;
    int cz2 = 1 + z2 * (ROOM_H + 1) + ROOM_H / 2;

    while (cx1 != cx2) {
        levelMap[cz1][cx1] = '.';
        cx1 += (cx1 < cx2) ? 1 : -1;
    }
    while (cz1 != cz2) {
        levelMap[cz1][cx1] = '.';
        cz1 += (cz1 < cz2) ? 1 : -1;
    }
}

void GenerateRoomBasedMap() {
    srand(time(NULL));
    ClearMap();

    bool roomPlaced[ROOM_ROWS][ROOM_COLS] = { 0 };

    int startX = rand() % ROOM_COLS;
    int startZ = rand() % ROOM_ROWS;
    roomPlaced[startZ][startX] = true;
    CarveRoom(startX, startZ);

    currentRoomX = startX;
    currentRoomZ = startZ;

    int numRooms = 6 + rand() % 3;
    int placed = 1;

    while (placed < numRooms) {
        int rx = rand() % ROOM_COLS;
        int rz = rand() % ROOM_ROWS;

        if (roomPlaced[rz][rx]) continue;

        if ((rx > 0 && roomPlaced[rz][rx - 1]) ||
            (rx < ROOM_COLS - 1 && roomPlaced[rz][rx + 1]) ||
            (rz > 0 && roomPlaced[rz - 1][rx]) ||
            (rz < ROOM_ROWS - 1 && roomPlaced[rz + 1][rx])) {

            roomPlaced[rz][rx] = true;
            CarveRoom(rx, rz);

            int dx = 0, dz = 0;
            do {
                dx = (rand() % 3) - 1;
                dz = (rand() % 3) - 1;
            } while (abs(dx) + abs(dz) != 1 ||
                rx + dx < 0 || rx + dx >= ROOM_COLS ||
                rz + dz < 0 || rz + dz >= ROOM_ROWS ||
                !roomPlaced[rz + dz][rx + dx]);

            ConnectRooms(rx, rz, rx + dx, rz + dz);
            placed++;
        }
    }
}

void HandleRoomTransition(Vector3 playerPos) {
    int roomX = (int)(playerPos.x / (ROOM_W * 2 + 2));
    int roomZ = (int)(playerPos.z / (ROOM_H * 2 + 2));

    if ((roomX != currentRoomX || roomZ != currentRoomZ) && !fading) {
        fading = true;
        fadeIn = false;
    }

    if (fading) {
        fadeAlpha += fadeIn ? -fadeSpeed : fadeSpeed;
        if (fadeAlpha >= 1.0f) {
            currentRoomX = roomX;
            currentRoomZ = roomZ;
            fadeIn = true;
        }
        else if (fadeAlpha <= 0.0f) {
            fadeAlpha = 0.0f;
            fading = false;
        }
    }
}

void DrawFadeOverlay() {
    if (fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, fadeAlpha));
    }
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
int GetRoomCenterX(int roomX) {
    return (1 + roomX * (ROOM_W + 1) + ROOM_W / 2) * 2;
}

int GetRoomCenterZ(int roomZ) {
    return (1 + roomZ * (ROOM_H + 1) + ROOM_H / 2) * 2;
}



int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;


    //wall texturing
    InitWindow(screenWidth, screenHeight, "Raylib FPS - Procedural Map + Minimap");
    DisableCursor();
    Texture2D wallTex1 = LoadTexture("assets/textures/Brick_11-128x128.png");
    Texture2D wallTex2 = LoadTexture("assets/textures/Brick_12-128x128.png");
    Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f); // Size of cube

    Material wallMat1 = LoadMaterialDefault();
    wallMat1.maps[MATERIAL_MAP_ALBEDO].texture = wallTex1;

    Material wallMat2 = LoadMaterialDefault();
    wallMat2.maps[MATERIAL_MAP_ALBEDO].texture = wallTex2;

    //floor texturing


    Texture2D floorTex1 = LoadTexture("assets/textures/Tile_01-128x128.png");
    Texture2D floorTex2 = LoadTexture("assets/textures/Tile_01-128x128.png");

    Material floorMat1 = LoadMaterialDefault();
    floorMat1.maps[MATERIAL_MAP_ALBEDO].texture = floorTex1;

    Material floorMat2 = LoadMaterialDefault();
    floorMat2.maps[MATERIAL_MAP_ALBEDO].texture = floorTex2;

    Mesh floorMesh = GenMeshCube(2.0f, 0.1f, 2.0f); // Flat floor tile

    GenerateRoomBasedMap();

    Camera3D camera = { 0 };
    int playerX = GetRoomCenterX(currentRoomX);
    int playerZ = GetRoomCenterZ(currentRoomZ);
    camera.position = Vector3{ (float)playerX, 1.8f, (float)playerZ };
    camera.target = Vector3Add(camera.position, Vector3{ 0.0f, 0.0f, -1.0f });

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
        HandleRoomTransition(camera.position);

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);

        for (int z = 0; z < MAP_HEIGHT; z++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                Vector3 cubePos = Vector3{ x * 2.0f, 1.0f, z * 2.0f };
                if (levelMap[z][x] == '#') {
                    // Alternate between two wall textures
                    Material mat = ((x + z) % 2 == 0) ? wallMat1 : wallMat2;
                    // Bottom wall block
                    Matrix bottom = MatrixTranslate(cubePos.x, cubePos.y, cubePos.z);
                    DrawMesh(cubeMesh, mat, bottom);

                    // Top wall block (2.0 units higher)
                    Matrix top = MatrixTranslate(cubePos.x, cubePos.y + 2.0f, cubePos.z);
                    DrawMesh(cubeMesh, mat, top);



                }
                else {
                    if (levelMap[z][x] == '.') {
                        Material mat = ((x + z) % 2 == 0) ? floorMat1 : floorMat2;
                        Matrix floorMatrix = MatrixTranslate(cubePos.x, cubePos.y - 0.95f, cubePos.z);
                        DrawMesh(floorMesh, mat, floorMatrix);
                    }

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

        DrawFadeOverlay();
        EndDrawing();
    }

    //unloading the assets
    UnloadTexture(wallTex1);
    UnloadTexture(wallTex2);
    UnloadMaterial(wallMat1);
    UnloadMaterial(wallMat2);
    UnloadMesh(cubeMesh);

    UnloadTexture(floorTex1);
    UnloadTexture(floorTex2);
    UnloadMaterial(floorMat1);
    UnloadMaterial(floorMat2);
    UnloadMesh(floorMesh);



    CloseWindow();
    return 0;
}
