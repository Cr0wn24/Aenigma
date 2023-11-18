#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#pragma pack(push, 1)
typedef struct World_File_Header
{
    s32 chunk_count_x;
    s32 chunk_count_y;
    u32 chunk_count;
} World_File_Header;

typedef struct Chunk_Header
{
    s32 chunk_pos_x;
    s32 chunk_pos_y;
    u32 tiles[16 * 16];
    u32 entity_count;
    Entity *entitites;
} Chunk_Header;
#pragma pack(pop)

#endif
