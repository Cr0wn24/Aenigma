function Entity *
GetEntityFromPosAndType(TileMap *tile_map, EntityPointer *entity_pointer)
{
    Entity *result = 0;

    EntityGroup entity_group = GetEntities(tile_map, entity_pointer->pos);

    for(u32 entity_index = 0; entity_index < entity_group.count; ++entity_index)
    {
        Entity *entity = entity_group.entities[entity_index];
        if(entity->type == entity_pointer->type)
        {
            result = entity;
            break;
        }
    }

    return result;
}

function b32
LoadWorldSave(GameState *game_state, MemoryArena *world_arena, char *world_name)
{
    char full_path[256] = { 0 };
    StringFormat(full_path, 256, "worlds\\%s.world", world_name);
#if 0
    ReadFileResult file = platform->ReadEntireFile(0, full_path);
    u8 *contents = file.contents;

    MemoryArena temp = { 0 };
    InitializeArena(&temp, file.content_size, contents);
#else
    FILE *save_file = fopen(full_path, "r");

    if(!save_file)
    {
        return false;
    }

    fseek(save_file, 0L, SEEK_END);
    u32 size = ftell(save_file);
    fseek(save_file, 0L, SEEK_SET);

    u8 *contents = (u8 *)malloc(size);
    fread(contents, 1, size, save_file);

    MemoryArena temp = { 0 };
    InitializeArena(&temp, size, contents);
#endif

    ZeroArena(world_arena);
    random_index = 0;

    // printf("Read world_header at %zu\n", temp.used);
    World_File_Header *world_header = PushStruct(&temp, World_File_Header);

    game_state->world = PushStruct(world_arena, World);
    World *world = game_state->world;

    world->tile_map = PushStruct(world_arena, TileMap);
    TileMap *tile_map = world->tile_map;

    tile_map->chunk_count_x = world_header->chunk_count_x;
    tile_map->chunk_count_y = world_header->chunk_count_y;
    tile_map->chunk_dim = 16;
    tile_map->tile_side_in_meters = 1.0f;

    tile_map->chunks = PushArray(world_arena, tile_map->chunk_count_x * tile_map->chunk_count_y, Chunk);

    for(u32 chunk_index = 0; chunk_index < world_header->chunk_count; ++chunk_index)
    {
        // printf("Read chunk%d at %zu\n", chunk_index, temp.used);
        Chunk_Header *chunk_header = PushStruct(&temp, Chunk_Header);

        Chunk *chunk = GetChunk(tile_map, chunk_header->chunk_pos_x, chunk_header->chunk_pos_y);

        // since we are only saving valid chunks, this assert should always succeed
        Assert(chunk);
        if(chunk)
        {

            chunk->tiles = PushArray(world_arena, 16 * 16, u32);
            chunk->entities = PushArray(world_arena, 256, Entity *);

            u32 *src_tiles = chunk_header->tiles;

            if(chunk->tiles)
            {
                u32 *dest_tiles = chunk->tiles;
                for(u32 i = 0; i < 16 * 16; ++i)
                {
                    *dest_tiles++ = *src_tiles++;
                }
            }

            for(u32 i = 0; i < chunk_header->entity_count; ++i)
            {
                chunk->entities[i] = PushStruct(world_arena, Entity);
                // printf("Read entity%d from chunk%d at %zu\n", i, chunk_index, temp.used);
                Entity *entity = PushStruct(&temp, Entity);
                *chunk->entities[i] = *entity;

                if(chunk->entities[i]->type == EntityType_Tim)
                {
                    world->tim = chunk->entities[i];
                }
            }
        }
    }

    u32 *num_doors = PushStruct(&temp, u32);

    for(u32 door_index = 0; door_index < *num_doors; ++door_index)
    {
        EntityPointer *entity_pointer = PushStruct(&temp, EntityPointer);
        world->doors[world->num_doors++] = GetEntityFromPosAndType(tile_map, entity_pointer);
    }

    u32 *num_activators = PushStruct(&temp, u32);

    for(u32 activator_index = 0; activator_index < *num_activators; ++activator_index)
    {
        Activator *dest = &world->activators[activator_index];
        Activator *src = PushStruct(&temp, Activator);
        *dest = *src;
        for(u32 i = 0; i < 2; ++i)
        {
            EntityPointer *entity_pointer = PushStruct(&temp, EntityPointer);
            dest->doors[i] = GetEntityFromPosAndType(tile_map, entity_pointer);
        }
    }

    fclose(save_file);

    return true;
}

function void
SaveWorld(World *world, char *world_name)
{
    TileMap *tile_map = world->tile_map;
    u8 *buffer = malloc(1024 * 1024 * 1024);
    ZeroArray(buffer, 1024 * 1024 * 1024, u8);

    // @Incomplete: Temp arenas
    MemoryArena save_arena = { 0 };
    InitializeArena(&save_arena, 1024 * 1024 * 1024, buffer);

    World_File_Header *world_header = PushStruct(&save_arena, World_File_Header);
    world_header->chunk_count_x = tile_map->chunk_count_x;
    world_header->chunk_count_y = tile_map->chunk_count_y;

    for(s32 chunk_y = 0; chunk_y < tile_map->chunk_count_y; ++chunk_y)
    {
        for(s32 chunk_x = 0; chunk_x < tile_map->chunk_count_x; ++chunk_x)
        {
            s32 absolute_chunk_pos_x = chunk_x;
            s32 absolute_chunk_pos_y = chunk_y;
            Chunk *chunk = GetChunk(tile_map, absolute_chunk_pos_x, absolute_chunk_pos_y);

            if(chunk)
            {
                // no need to save the chunk if its empty
                if(chunk->tiles)
                {

                    Chunk_Header *chunk_header = PushStruct(&save_arena, Chunk_Header);

                    chunk_header->chunk_pos_x = absolute_chunk_pos_x;
                    chunk_header->chunk_pos_y = absolute_chunk_pos_y;

                    if(chunk->tiles)
                    {
                        u32 *tiles = chunk->tiles;
                        for(u32 y = 0; y < 16; ++y)
                        {
                            for(u32 x = 0; x < 16; ++x)
                            {
                                chunk_header->tiles[y * 16 + x] = *tiles++;
                            }
                        }
                    }

                    if(chunk->entities)
                    {
                        for(s32 i = 0; i < 256; ++i)
                        {
                            Entity *entity = chunk->entities[i];
                            if(entity)
                            {
                                if(entity->initialized)
                                {
                                    Entity *save_entity = PushStruct(&save_arena, Entity);
                                    *save_entity = *entity;
                                    chunk_header->entity_count++;
                                }
                            }
                        }
                    }
                    world_header->chunk_count++;
                }
            }
        }
    }

    u32 *num_doors = PushStruct(&save_arena, u32);

    for(u32 door_index = 0; door_index < 32; ++door_index)
    {
        if(world->doors[door_index])
        {
            EntityPointer *entity_pointer = PushStruct(&save_arena, EntityPointer);
            entity_pointer->pos = world->doors[door_index]->pos;
            entity_pointer->type = world->doors[door_index]->type;
            *num_doors += 1;
        }
    }

    u32 *num_activators = PushStruct(&save_arena, u32);

    for(u32 activator_index = 0; activator_index < 32; ++activator_index)
    {
        Activator *activator = &world->activators[activator_index];
        b32 active = activator->num_lines != 0;
        if(active)
        {
            Activator *dest = PushStruct(&save_arena, Activator);
            *dest = *activator;
            EntityPointer *entity_pointer = PushStruct(&save_arena, EntityPointer);
            entity_pointer->pos = activator->doors[0]->pos;
            entity_pointer->type = activator->doors[0]->type;
            entity_pointer = PushStruct(&save_arena, EntityPointer);
            entity_pointer->pos = activator->doors[1]->pos;
            entity_pointer->type = activator->doors[1]->type;
            *num_activators += 1;
        }
    }

    char full_path[256] = { 0 };
    StringFormat(full_path, 256, "worlds\\%s.world", world_name);
    FILE *save_file = fopen(full_path, "w");

    if(!save_file)
    {
        Assert(false);
    }

    fwrite(save_arena.base, save_arena.used, 1, save_file);

    fclose(save_file);

    free(buffer);
}