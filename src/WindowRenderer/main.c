#include <stdio.h>

#include <raylib.h>

#include "server.h"

#define DEBUG_ENABLE_FPS

int main(void)
{
    Server* server = server_create();
    if (server_run(server) < 0) {
        return 1;
    }

    InitWindow(800, 600, "WindowRenderer");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

#ifdef DEBUG_ENABLE_FPS
        DrawFPS(0, 0);
#endif

        Texture window_textures[server->windows_count];

        server_lock_windows(server);
        for (size_t wi = 0; wi < server->windows_count; ++wi) {
            Window* w = server->windows[wi];

            Image image = {
                .data = w->pixels,
                .width = (int)w->width,
                .height = (int)w->height,
                .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            };

            SetTraceLogLevel(LOG_WARNING);
            window_textures[wi] = LoadTextureFromImage(image);
            SetTraceLogLevel(LOG_INFO);

            DrawTexture(window_textures[wi], 0, 0, WHITE);
        }
        server_unlock_windows(server);

        EndDrawing();

        SetTraceLogLevel(LOG_WARNING);
        for (size_t i = 0; i < server->windows_count; ++i) {
            UnloadTexture(window_textures[i]);
        }
        SetTraceLogLevel(LOG_INFO);
    }

    server_destroy(server);

    CloseWindow();

    return 0;
}
