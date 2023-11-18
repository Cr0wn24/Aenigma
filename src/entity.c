function Entity *
GetNextEmptyEntitySlot(MemoryArena *arena, Chunk *chunk) {
    Entity *result = 0;

    for (u32 i = 0; i < 256; ++i) {
        if (!chunk->entities[i]) {
            chunk->entities[i] = PushStruct(arena, Entity);
            result = chunk->entities[i];
            break;
        } else {
            if (!chunk->entities[i]->initialized) {
                result = chunk->entities[i];
            }
        }
    }

    return result;
}

function Entity *
PushEntity(MemoryArena *arena, World *world, EntityType type, v3 abs_tile_pos) {
    TileMap *tile_map = world->tile_map;
    Assert(tile_map);
    TileMapPosition pos = { 0 };

    pos.tile_rel_pos = abs_tile_pos;

    RecanonicalizePosition(tile_map, &pos);

    Chunk *chunk = GetChunk(tile_map, pos.chunk_pos.x, pos.chunk_pos.y);

    if (!chunk->entities) {
        chunk->entities = PushArray(arena, 256, Entity *);
    }

    Entity *entity = GetNextEmptyEntitySlot(arena, chunk);
    entity->type = type;
    entity->pos = pos;
    entity->active = true;
    entity->initialized = true;

    return entity;
}

function void
RemoveEntity(World *world, Entity *entity) {
    Chunk *chunk = GetChunk(world->tile_map, entity->pos.chunk_pos.x, entity->pos.chunk_pos.y);
    for (u32 i = 0; i < 256; ++i) {
        if (chunk->entities[i] == entity) {
            ZeroStruct(*entity);
            break;
        }
    }
}

function inline s32
CompareEntityPos(Entity *entity0, Entity *entity1) {
    v3 tile_pos0 = GetTilePos(entity0->pos);
    v3 tile_pos1 = GetTilePos(entity1->pos);

    f32 value0 = tile_pos0.x - tile_pos0.y + tile_pos0.z;
    f32 value1 = tile_pos1.x - tile_pos1.y + tile_pos1.z;

    if (value0 < value1) {
        return -1;
    } else {
        return 1;
    }
}

function void
SortEntitiesInRegion(Entity **entities, s32 entity_region_count) {
    BEGIN_TIMED_FUNCTION();
    for (s32 i = 1; i < entity_region_count; ++i) {
        s32 j = i;
        while (j > 0 && CompareEntityPos(entities[j - 1], entities[j]) > 0) {
            Swap(entities[j], entities[j - 1], Entity *);
            j--;
        }
    }
    END_TIMED_FUNCTION();
}

function EntityGroup
GetEntities(TileMap *tile_map, TileMapPosition pos) {
    Assert(tile_map);
    EntityGroup result = { 0 };

    Chunk *chunk = GetChunk(tile_map, pos.chunk_pos.x, pos.chunk_pos.y);

    if (chunk) {
        if (chunk->entities) {
            for (u32 entity_index = 0; entity_index < 256; ++entity_index) {
                if (chunk->entities[entity_index]) {
                    if (CompareTileMapPosition(chunk->entities[entity_index]->pos, pos)) {
                        // an entity was found on that position
                        result.entities[result.count++] = chunk->entities[entity_index];
                    }
                }
            }
        }
    }

    return result;
}

function inline void
EndMoveEvent(Entity *entity) {
    Assert(entity->move_event.active);
    entity->move_event.active = false;
    entity->move_event.timer = 0;
}

function inline void
StartMoveEvent(Entity *entity, TileMapPosition target_pos, f32 duration) {
    Assert(!entity->move_event.active);
    entity->move_event.active = true;
    entity->move_event.start_pos = entity->pos;
    entity->move_event.target_pos = target_pos;
    entity->move_event.duration = duration;
}

function void
UpdateEntity(Entity *entity, World *world, f32 dt) {
    Assert(entity);
    Assert(world);
    f32 player_speed = 5.0f;

    if (world->tim_pushing) {
        player_speed = 3.0f;
    }

    if (world->DEBUG_tim_fast) {
        player_speed *= 10.0f;
    }

    TileMap *tile_map = world->tile_map;
    if (entity->move_event.active) {
        entity->move_event.timer += dt * player_speed;
        entity->move_event.timer = Clamp(0, entity->move_event.timer, entity->move_event.duration);
        entity->pos = LerpTileMapPosition(entity->move_event.start_pos, entity->move_event.target_pos,
                                          entity->move_event.timer / entity->move_event.duration);

        RecanonicalizePosition(tile_map, &entity->pos);

        if (entity->move_event.timer >= entity->move_event.duration) {
            EndMoveEvent(entity);
        }
    }
}

function void
UpdateChunkEntities(World *world, Chunk *chunk, f32 dt) {
    Assert(world);
    Assert(chunk);

    TileMap *tile_map = world->tile_map;

    for (u32 entity_index = 0; entity_index < 256; ++entity_index) {
        Entity *entity = chunk->entities[entity_index];
        if (entity) {
            if (entity->initialized) {
                Chunk *check_chunk = GetChunk(tile_map, entity->pos.chunk_pos.x, entity->pos.chunk_pos.y);

                if (check_chunk != chunk) {
                    // the entity has moved onto a new chunk
                    for (u32 i = 0; i < 256; ++i) {
                        if (!check_chunk->entities[i]) {
                            // found an empty slot to place the entity
                            chunk->entities[entity_index] = 0;
                            check_chunk->entities[i] = entity;
                            break;
                        } else {
                            // not empty
                        }
                    }
                }
            }
        }
    }
}

function b32
CanWallMove(World *world, Entity *entity, FacingDirection direction) {
    Assert(world);
    TileMap *tile_map = world->tile_map;
    Assert(tile_map);
    b32 result = false;

    // DO NOT REMOVE. The solves a bug when moving walls between chunks
    if (entity->move_event.active) {
        return false;
    }

    TileMapPosition new_target_pos = entity->pos;
    switch (direction) {
        case Direction_Up: {
            new_target_pos.tile_pos.y++;
        } break;

        case Direction_Down: {
            new_target_pos.tile_pos.y--;
        } break;

        case Direction_Left: {
            new_target_pos.tile_pos.x--;
        } break;

        case Direction_Right: {
            new_target_pos.tile_pos.x++;
        } break;
    }
    RecanonicalizePosition(tile_map, &new_target_pos);

    if (IsTileMapPointEmpty(tile_map, new_target_pos)) {
        // the tile is empty, now we need to check the entities on that tile
        EntityGroup colliding_entities = GetEntities(tile_map, new_target_pos);

        if (colliding_entities.count) {
            for (u32 i = 0; i < colliding_entities.count; ++i) {
                Entity *colliding_entity = colliding_entities.entities[i];
                if (colliding_entity != entity) {
                    switch (colliding_entity->type) {
                        case EntityType_MovableWall: {
                            result = CanWallMove(world, colliding_entity, direction);
                        } break;

                        case EntityType_Door: {
                            result = colliding_entity->open;
                        } break;

                        default: {
                            result = false;
                        } break;
                    }
                }
            }
        } else {
            // There was no entity
            result = true;
        }
    } else {
        result = false;
    }

    if (result) {
        StartMoveEvent(entity, new_target_pos, 1.0f);
    }

    return result;
}

function b32
TimCanMove(World *world, TileMapPosition new_pos, FacingDirection direction) {
    b32 result = true;
    TileMap *tile_map = world->tile_map;
    if (IsTileMapPointEmpty(tile_map, new_pos)) {
        // the tile is empty, now we need to check the entities on that tile
        EntityGroup colliding_entities = GetEntities(tile_map, new_pos);

        if (colliding_entities.count) {
            for (u32 i = 0; i < colliding_entities.count; ++i) {
                Entity *colliding_entity = colliding_entities.entities[i];
                if (colliding_entity != world->tim) {
                    switch (colliding_entity->type) {
                        case EntityType_Water: {
                            result = false;
                        } break;

                        case EntityType_Grass: {
                            result = true;
                        } break;

                        case EntityType_MovableWall: {
                            result = CanWallMove(world, colliding_entity, direction);
                            if (result) {
                                world->tim_pushing = true;
                            }
                        } break;

                        case EntityType_Door: {
                            result = colliding_entity->open;
                        } break;

                        default: {
                            result = false;
                        } break;
                    }
                }
            }

        } else {
            // There was no entity
            result = true;
        }
    } else {
        result = false;
    }

    return result;
}

function void
SimEntitiesInRegion(World *world, Entity **entities, u32 entity_count,
                    GameAssets *assets, RenderBuffer *buffer, Camera *camera,
                    f32 dt, b32 draw_entities) {
    BEGIN_TIMED_FUNCTION();
    Assert(world);
    Assert(entities);
    Assert(assets);
    Assert(camera);

    TileMap *tile_map = world->tile_map;
    Entity *tim = world->tim;

    for (u32 entity_index = 0; entity_index < entity_count; ++entity_index) {
        Entity *entity = entities[entity_index];
        TileMapPosition pos = entity->pos;

        v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));

        UpdateEntity(entity, world, dt);
        // Entity type-specific stuff
        switch (entity->type) {
            case EntityType_Wall: {
                DrawBitmapInPixels(buffer, &assets->wall, screen_pos);

                TileType tile_type_right = GetTileTypeRight(tile_map, pos);
                s32 shadow_type = -1;
                switch (tile_type_right) {
                    case TileType_TreePortPier:
                    case TileType_TreePortPierPole:
                    case TileType_Grass1:
                    case TileType_Grass0:
                    {
                        shadow_type = ShadowType_Medium;
                        pos.tile_rel_pos.z -= 0.5f;
                    } break;
                }

                if (shadow_type >= 0) {
                    v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(1, 0));
                    DrawBitmapInPixels(buffer, &assets->shadows[shadow_type], screen_pos);
                }

            } break;

            case EntityType_MovableWall: {
                DrawBitmapInPixels(buffer, &assets->moveable_wall, screen_pos);

                TileType tile_type_right = GetTileTypeRight(tile_map, pos);
                s32 shadow_type = -1;
                switch (tile_type_right) {
                    case TileType_TreePortPier:
                    case TileType_TreePortPierPole:
                    case TileType_Grass1:
                    case TileType_Grass0:
                    {
                        shadow_type = ShadowType_Medium;
                        pos.tile_rel_pos.z -= 0.5f;
                    } break;
                }

                if (shadow_type >= 0) {
                    v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(1, 0));
                    DrawBitmapInPixels(buffer, &assets->shadows[shadow_type], screen_pos);
                }
            } break;

            case EntityType_MossyWall: {
                DrawBitmapInPixels(buffer, &assets->mossy_wall, screen_pos);

                TileType tile_type_right = GetTileTypeRight(tile_map, pos);
                s32 shadow_type = -1;
                switch (tile_type_right) {
                    case TileType_TreePortPier:
                    case TileType_TreePortPierPole:
                    case TileType_Grass1:
                    case TileType_Grass0:
                    {
                        shadow_type = ShadowType_Medium;
                        pos.tile_rel_pos.z -= 0.5f;
                    } break;
                }

                if (shadow_type >= 0) {
                    v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(1, 0));
                    DrawBitmapInPixels(buffer, &assets->shadows[shadow_type], screen_pos);
                }
            } break;

            case EntityType_Tim: {
                Bitmap *tim_bitmap = 0;
                switch (tim->facing_direction) {
                    case Direction_Right: {
                        if (world->tim_running) {
                            if (world->tim_pushing) {
                                tim_bitmap = &assets->tim_bitmaps.pushing_right[world->curr_animation_frame];
                            } else {
                                tim_bitmap = &assets->tim_bitmaps.run_right[world->curr_animation_frame];
                            }
                        } else {
                            tim_bitmap = &assets->tim_bitmaps.still_right;
                        }
                    } break;

                    case Direction_Left: {
                        if (world->tim_running) {
                            if (world->tim_pushing) {
                                tim_bitmap = &assets->tim_bitmaps.pushing_left[world->curr_animation_frame];
                            } else {
                                tim_bitmap = &assets->tim_bitmaps.run_left[world->curr_animation_frame];
                            }
                        } else {
                            tim_bitmap = &assets->tim_bitmaps.still_left;
                        }
                    } break;

                    case Direction_Up: {
                        if (world->tim_running) {
                            if (world->tim_pushing) {
                                tim_bitmap = &assets->tim_bitmaps.pushing_up[world->curr_animation_frame];
                            } else {
                                tim_bitmap = &assets->tim_bitmaps.run_up[world->curr_animation_frame];
                            }
                        } else {
                            tim_bitmap = &assets->tim_bitmaps.still_up;
                        }
                    } break;

                    case Direction_Down: {
                        if (world->tim_running) {
                            if (world->tim_pushing) {
                                tim_bitmap = &assets->tim_bitmaps.pushing_down[world->curr_animation_frame];
                            } else {
                                tim_bitmap = &assets->tim_bitmaps.run_down[world->curr_animation_frame];
                            }
                        } else {
                            tim_bitmap = &assets->tim_bitmaps.still_down;
                        }
                    } break;
                }

                Assert(tim_bitmap);

                // @Incomplete: Fix alignment
                DrawBitmapAlignedInPixels(buffer, &assets->shadows[ShadowType_Tim], screen_pos, V2(-20 * SCALE, -35 * SCALE));
                DrawBitmapInPixels(buffer, tim_bitmap, screen_pos);
            } break;

            case EntityType_DoorPortalVert: {
                DrawBitmapInPixels(buffer, &assets->door_portal_vertical, screen_pos);
            } break;

            case EntityType_DoorPortalHori: {
                DrawBitmapInPixels(buffer, &assets->door_portal_horizontal, screen_pos);
            } break;

            case EntityType_Emitter: {
                Particle *particle = entity->particles;
                for (u32 i = 0; i < entity->num_particles; ++i, ++particle) {
                    // entity->sim_func(entity, particle, dt, camera, buffer);
                }
            } break;

            case EntityType_Door: {
                if (!entity->open) {
                    DrawBitmapInPixels(buffer, &assets->door, screen_pos);
                }
            } break;

                InvalidCase;
        }
    }
    END_TIMED_FUNCTION();
}