#ifndef EDITOR_H
#define EDITOR_H

typedef enum TileSelection {
    // all tiles here
    EditorSelection_Water = 0,
    EditorSelection_Grass0 = 1,
    EditorSelection_Grass1 = 2,
    EditorSelection_TreePortPier = 3,
    EditorSelection_TreePortPierPole = 4,

    // all entities here
    EditorSelection_Wall = 5,
    EditorSelection_MossyWall = 6,
    EditorSelection_MovableWall = 7,
    EditorSelection_Tim = 8,
    EditorSelection_Door = 9,
} TileSelection;

typedef struct TilePattern {
    v2s dim;
    u8 pattern[32 * 32];
} TilePattern;

typedef struct TilePatterns {
    s32 num_patterns;
    TilePattern pattern[32];
} TilePatterns;

#define MAX_WORLD_NAME_LENGTH 256

typedef struct EditorState {
    s32 selected_pattern;
    TilePatterns patterns;
    TileSelection selected_tile;

    u32 num_worlds_name;
    char worlds_name[32][MAX_WORLD_NAME_LENGTH];

    s32 current_world_index;

    b32 currently_loading;
    b32 currently_saving;

    b32 invalid_save_name;

    b32 world_does_not_exist;
    char invalid_world_name[MAX_WORLD_NAME_LENGTH];

    char save_input_buffer[MAX_WORLD_NAME_LENGTH];
    char load_input_buffer[MAX_WORLD_NAME_LENGTH];

    b32 placing_activator;
    Activator *current_activator;

    b32 is_initialized;
} EditorState;

#endif