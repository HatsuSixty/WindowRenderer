#pragma once

#define WR_WINDOW_TITLE_SIZE_MAX 256

typedef struct {
    char title[WR_WINDOW_TITLE_SIZE_MAX];
    int width;
    int height;
} WindowRendererCreateWindow;
