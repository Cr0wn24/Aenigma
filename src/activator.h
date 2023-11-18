#ifndef ACTIVATOR_H
#define ACTIVATOR_H

typedef enum ActivatorType {
    ActivatorType_Green = 0,
    ActivatorType_Blue = 1,
    ActivatorType_Yellow = 2,
    ActivatorType_Red = 3,
} ActivatorType;

typedef struct ActivatorLine {
    b32 active;

    TileMapPosition pos;
    FacingDirection start_dir;
    FacingDirection end_dir;
} ActivatorLine;

typedef struct Activator {
    ActivatorType type;

    u32 num_lines;
    ActivatorLine lines[32];

    b32 started;
    b32 ended;
    Entity *doors[2];

    f32 timer;
    b32 increment_timer;
    b32 decrement_timer;
} Activator;

#endif