#ifndef AENIGMA_H
#define AENIGMA_H

#include "platform.h"
#include "intrinsics.h"
#include "math.h"

#include <stdio.h>

#define ZeroStruct(val) ZeroMemory_((u8 *)&val, sizeof(val))
#define ZeroArray(array, count, T) ZeroMemory_((u8 *)array, count*sizeof(T))

void ZeroMemory_(u8 *src, memory_index size)
{
    for(memory_index i = 0; i < size; ++i)
    {
        *src++ = 0;
    }
}

typedef struct MemoryArena
{
    memory_index size;
    u8 *base;
    memory_index used;
} MemoryArena;

function void InitializeArena(MemoryArena *arena, memory_index size, u8 *base)
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

function void ZeroArena(MemoryArena *arena)
{
    ZeroArray(arena->base, arena->used, u8);
    arena->used = 0;
}

#define PushStruct(arena, T) (T *)PushSize_(arena, sizeof(T))
#define PushArray(arena, count, T) (T *)PushSize_(arena, (count)*(sizeof(T)))

void *PushSize_(MemoryArena *arena, memory_index size)
{
    Assert((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;
    return result;
}

#include "random.h"
#include "asset.h"
#include "renderer.h"
#include "tile.h"
#include "entity.h"
#include "ui.h"
#include "menu.h"
#include "activator.h"
#include "world_loader.h"
#include "editor.h"

#include "tiles.h"

typedef enum ShadowType
{
    ShadowType_Small,
    ShadowType_Medium,
    ShadowType_Large,
    ShadowType_Tim,
} ShadowType;

typedef enum ProgramMode
{
    ProgramMode_Game = 0,
    ProgramMode_Menu = 1,
    ProgramMode_Editor = 2,
} ProgramMode;

typedef struct World
{
    char name[256];
    TileMap *tile_map;
    
    Entity *portal_emitter;
    
    u32 num_doors;
    // TODO(hampus): Make this dynamic?
    Entity *doors[32];
    
    // TODO(hampus): Make this dynamic?
    Activator activators[32];
    
    b32 tim_pushing;
    b32 tim_running;
    u32 curr_animation_frame;
    
    b32 DEBUG_tim_fast;
    
    Entity *tim;
} World;

typedef struct Camera
{
    v2 pos;
    
    // Movement stuff
    b32 should_move;
    f32 timer;
    v2 start_pos;
    v2 end_pos;
} Camera;


typedef struct GameState
{
    ProgramMode program_mode;
    
    MemoryArena world_arena;
    MemoryArena frame_arena;
    
    World *world;
    
    GameAssets assets;
    
    Camera camera;
    
    f32 seconds_elapsed;
    
    b32 player_has_moved;
    
    EditorState editor_state;
    
    b32 debug_menu_enabled;
    b32 draw_debug_info;
    b32 draw_debug_guides;
} GameState;

#endif