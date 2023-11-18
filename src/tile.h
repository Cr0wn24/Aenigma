#ifndef TILE_H
#define TILE_H

typedef enum TileType
{
    TileType_Water = 0,
    TileType_Grass0 = 1,
    TileType_Grass1 = 2,
    TileType_TreePortPier = 3,
    TileType_TreePortPierPole = 4,
} TileType;

typedef struct TileMapPosition
{
    v2s chunk_pos;

    v3s tile_pos;

    v3 tile_rel_pos;
} TileMapPosition;

typedef struct Particle
{
    TileMapPosition pos;
    v3 vel;
    v4 color;
} Particle;

typedef struct Entity Entity;

typedef struct Chunk
{
    u32 *tiles;
    Entity **entities;
} Chunk;

typedef struct TileMap
{
    Chunk *chunks;

    s32 chunk_count_x;
    s32 chunk_count_y;

    s32 chunk_dim;

    f32 tile_side_in_meters;
} TileMap;

#endif