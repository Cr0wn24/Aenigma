function void
RecanonicalizePosition(TileMap *tile_map, TileMapPosition *pos)
{
    Assert(tile_map);

    s32 tile_offset_x = RoundReal32ToInt32(pos->tile_rel_pos.x / tile_map->tile_side_in_meters);
    s32 tile_offset_y = RoundReal32ToInt32(pos->tile_rel_pos.y / tile_map->tile_side_in_meters);
    s32 tile_offset_z = RoundReal32ToInt32(pos->tile_rel_pos.z / tile_map->tile_side_in_meters);

    pos->tile_pos.x += tile_offset_x;
    pos->tile_pos.y += tile_offset_y;
    pos->tile_pos.z += tile_offset_z;

    pos->tile_rel_pos.x -= tile_offset_x * tile_map->tile_side_in_meters;
    pos->tile_rel_pos.y -= tile_offset_y * tile_map->tile_side_in_meters;
    pos->tile_rel_pos.z -= tile_offset_z * tile_map->tile_side_in_meters;

    Assert(pos->tile_rel_pos.x >= -0.5f * tile_map->tile_side_in_meters && pos->tile_rel_pos.x <= 0.5f * tile_map->tile_side_in_meters);
    Assert(pos->tile_rel_pos.y >= -0.5f * tile_map->tile_side_in_meters && pos->tile_rel_pos.y <= 0.5f * tile_map->tile_side_in_meters);
    Assert(pos->tile_rel_pos.z >= -0.5f * tile_map->tile_side_in_meters && pos->tile_rel_pos.z <= 0.5f * tile_map->tile_side_in_meters);

    s32 chunk_offset_x = FloorReal32ToInt32((f32)pos->tile_pos.x / tile_map->chunk_dim);
    s32 chunk_offset_y = FloorReal32ToInt32((f32)pos->tile_pos.y / tile_map->chunk_dim);

    pos->chunk_pos.x += chunk_offset_x;
    pos->chunk_pos.y += chunk_offset_y;

    pos->tile_pos.x -= chunk_offset_x * tile_map->chunk_dim;
    pos->tile_pos.y -= chunk_offset_y * tile_map->chunk_dim;

    Assert(pos->tile_pos.x >= 0 && pos->tile_pos.x < (s32)tile_map->chunk_dim);
    Assert(pos->tile_pos.y >= 0 && pos->tile_pos.y < (s32)tile_map->chunk_dim);
}

function inline v3
GetTilePos(TileMapPosition pos)
{
    v3 result = { 0 };

    result.x = (f32)pos.chunk_pos.x * 16 + pos.tile_pos.x + pos.tile_rel_pos.x;
    result.y = (f32)pos.chunk_pos.y * 16 + pos.tile_pos.y + pos.tile_rel_pos.y;
    result.z = pos.tile_pos.z + pos.tile_rel_pos.z;

    return result;
}

function inline Chunk *
GetChunk(TileMap *tile_map, s32 chunk_x, s32 chunk_y)
{
    Assert(tile_map);
    Chunk *result = 0;

    if(chunk_x >= 0 && chunk_y >= 0 && chunk_x < tile_map->chunk_count_x && chunk_y < tile_map->chunk_count_y)
    {
        result = &tile_map->chunks[chunk_x + chunk_y * tile_map->chunk_count_x];
    }

    return result;
}


function inline TileType
GetChunkTileType(TileMap *tile_map, Chunk *chunk, s32 x, s32 y)
{
    Assert(chunk);
    TileType result = 0;

    if(x >= 0 && y >= 0 && x < tile_map->chunk_dim && y < tile_map->chunk_dim)
    {
        if(chunk->tiles)
        {
            result = chunk->tiles[x + y * tile_map->chunk_dim];
        }
    }

    return result;
}

function TileType
GetTileMapTileType(TileMap *tile_map, TileMapPosition pos)
{
    Assert(tile_map);
    TileType result = 0;

    if(pos.chunk_pos.x >= 0 && pos.chunk_pos.y >= 0 && pos.chunk_pos.x < tile_map->chunk_count_x && pos.chunk_pos.y < tile_map->chunk_count_y)
    {
        Chunk *chunk = GetChunk(tile_map, pos.chunk_pos.x, pos.chunk_pos.y);
        if(chunk)
        {
            result = GetChunkTileType(tile_map, chunk, pos.tile_pos.x, pos.tile_pos.y);
        }
    }

    return result;
}

function b32
IsChunkPointEmpty(TileMap *tile_map, Chunk *chunk, v3s tile_pos)
{
    Assert(tile_map);
    Assert(chunk);
    b32 is_empty = false;

    u32 tile_value = GetChunkTileType(tile_map, chunk, tile_pos.x, tile_pos.y);

    is_empty = (tile_value == TileType_Grass0 || tile_value == TileType_Grass1 || tile_value == TileType_TreePortPier || tile_value == TileType_TreePortPierPole);

    return is_empty;
}

function b32
IsTileMapPointEmpty(TileMap *tile_map, TileMapPosition pos)
{
    Assert(tile_map);
    b32 is_empty = false;

    Chunk *chunk = GetChunk(tile_map, pos.chunk_pos.x, pos.chunk_pos.y);
    if(chunk)
    {
        is_empty = IsChunkPointEmpty(tile_map, chunk, pos.tile_pos);
    }

    return is_empty;
}

function inline TileType
GetTileType(TileMap *tile_map, TileMapPosition pos)
{
    Assert(tile_map);

    TileType result = GetTileMapTileType(tile_map, pos);

    return result;
}

function void
SetTileType(MemoryArena *arena, TileMap *tile_map, TileMapPosition pos, u32 tile_value)
{
    Assert(tile_map);
    Chunk *chunk = GetChunk(tile_map, pos.chunk_pos.x, pos.chunk_pos.y);

    if(!chunk->entities)
    {
        chunk->entities = PushArray(arena, 256, Entity *);
    }

    if(!chunk->tiles)
    {
        u32 tile_count = Square(tile_map->chunk_dim);
        chunk->tiles = PushArray(arena, tile_count, u32);

        for(u32 i = 0; i < tile_count; ++i)
        {
            chunk->tiles[i] = 0;
        }
    }

    chunk->tiles[pos.tile_pos.x + pos.tile_pos.y * tile_map->chunk_dim] = tile_value;
}

// returns true if they're the same, else false
function inline b32
CompareTileMapPosition(TileMapPosition a, TileMapPosition b)
{
    b32 result;

    result = (a.chunk_pos.x == b.chunk_pos.x && a.chunk_pos.y == b.chunk_pos.y &&
              a.tile_pos.x == b.tile_pos.x && a.tile_pos.y == b.tile_pos.y);

    return result;
}

function inline TileMapPosition
LerpTileMapPosition(TileMapPosition a, TileMapPosition b, f32 t)
{

    TileMapPosition result = { 0 };

    v3 abs_tile_a = GetTilePos(a);
    v3 abs_tile_b = GetTilePos(b);

    result.tile_rel_pos = LerpV3(abs_tile_a, abs_tile_b, t);

    return result;
}

function inline TileType
GetTileTypeRight(TileMap *tile_map, TileMapPosition pos)
{
    Assert(tile_map);
    TileType result = 0;
    TileMapPosition tile_pos_right = pos;
    tile_pos_right.tile_pos.x += 1;
    RecanonicalizePosition(tile_map, &tile_pos_right);
    result = GetTileType(tile_map, tile_pos_right);
    return result;
}

function inline TileType
GetTileTypeLeft(TileMap *tile_map, TileMapPosition pos)
{
    Assert(tile_map);
    TileType result = 0;
    TileMapPosition tile_pos_right = pos;
    tile_pos_right.tile_pos.x -= 1;
    RecanonicalizePosition(tile_map, &tile_pos_right);
    result = GetTileType(tile_map, tile_pos_right);
    return result;
}

function inline TileType
GetTileTypeDown(TileMap *tile_map, TileMapPosition pos)
{
    Assert(tile_map);
    TileType result = 0;
    TileMapPosition tile_pos_right = pos;
    tile_pos_right.tile_pos.y -= 1;
    RecanonicalizePosition(tile_map, &tile_pos_right);
    result = GetTileType(tile_map, tile_pos_right);
    return result;
}

function inline TileType
GetTileTypeUp(TileMap *tile_map, TileMapPosition pos)
{
    Assert(tile_map);
    TileType result = 0;
    TileMapPosition tile_pos_right = pos;
    tile_pos_right.tile_pos.y += 1;
    RecanonicalizePosition(tile_map, &tile_pos_right);
    result = GetTileType(tile_map, tile_pos_right);
    return result;
}