#include <raylib.h>
#include <stdio.h>

int main(int argc, char **argv) {

    InitWindow(800, 450, "Game Jam Submission");

    Texture gun = LoadTexture("resources/gun.png");

    /* Camera */
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };
    camera.target = (Vector3){ 0.185f, 2.4f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(BLACK);
        DrawTextureEx(gun, (Vector2){800.0f, 500.0f}, 0.0f, 0.5f, WHITE);

        EndDrawing();
    }

    UnloadTexture(gun);

    CloseWindow();

    return 0;
}
