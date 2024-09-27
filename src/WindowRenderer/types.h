#ifndef TYPES_H_
#define TYPES_H_

typedef struct {
    float x;
    float y;
} Vector2;
#define V2X(vec) vec.x, vec.y

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vector4;
#define V4X(vec) vec.x, vec.y, vec.z, vec.w

#endif // TYPES_H_
