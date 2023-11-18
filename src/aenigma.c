#include "aenigma.h"

#if AENIGMA_INTERNAL
#include <stdio.h>
#include <stdlib.h>
#endif

Platform *platform;

#include "random.c"
#include "asset.c"
#include "tile.c"
#include "renderer.c"
#include "iso_renderer.c"
#include "entity.c"
#include "ui.c"
#include "menu.c"
#include "activator.c"

function void
ResetWorldArena(GameState *game_state, MemoryArena *arena) {
    ZeroArena(arena);
    random_index = 0;
    game_state->world = PushStruct(&game_state->world_arena, World);
    World *world = game_state->world;

    world->tile_map = PushStruct(&game_state->world_arena, TileMap);
    TileMap *tile_map = world->tile_map;

    tile_map->chunk_count_x = 4096;
    tile_map->chunk_count_y = 4096;
    tile_map->chunk_dim = 16;
    tile_map->tile_side_in_meters = 1.0f;

    tile_map->chunks = PushArray(&game_state->world_arena, tile_map->chunk_count_x * tile_map->chunk_count_y, Chunk);
}

#include "world_loader.c"
#include "editor.c"

function void
DrawTileMap(TileMap *tile_map, RenderBuffer *buffer, Camera *camera, GameAssets *assets) {
    BEGIN_TIMED_FUNCTION();
    s32 min_x = (s32)(-2 + camera->pos.x);
    s32 max_x = (s32)(33 + camera->pos.x);

    s32 min_y = (s32)(16 + camera->pos.y);
    s32 max_y = (s32)(-17 + camera->pos.y);

    for (s32 rel_y = min_y; rel_y >= max_y; --rel_y) {
        for (s32 rel_x = min_x; rel_x < max_x; ++rel_x) {
            TileMapPosition pos = { 0 };
            pos.tile_pos.x = rel_x;
            pos.tile_pos.y = rel_y;
            RecanonicalizePosition(tile_map, &pos);
            TileType tile_type = GetTileType(tile_map, pos);

            if (tile_type) {
                // Inside map
                v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                switch (tile_type) {
                    case TileType_Grass0: {
                        if (CanDrawTopOnly(tile_map, pos)) {
                            DrawBitmapInPixels(buffer, &assets->grass_top[0], screen_pos);
                        } else {
                            DrawBitmapInPixels(buffer, &assets->grass[0], screen_pos);
                        }
                    } break;

                    case TileType_Grass1: {
                        if (CanDrawTopOnly(tile_map, pos)) {
                            DrawBitmapInPixels(buffer, &assets->grass_top[1], screen_pos);
                        } else {
                            DrawBitmapInPixels(buffer, &assets->grass[1], screen_pos);
                        }
                    } break;

                    case TileType_TreePortPier: {
                        // @Incomplete: @Refactoring: Make this cleaner
                        pos.tile_rel_pos.z -= 0.25f;
                        v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                        DrawBitmapLightnessInPixels(buffer, &assets->water[0], screen_pos, 0.5f);

                        pos.tile_rel_pos.z += 0.25f;
                        screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                        DrawBitmapInPixels(buffer, &assets->tree_port_pier, screen_pos);
                    } break;

                    case TileType_TreePortPierPole: {
                        // @Incomplete: @Refactoring: Make this cleaner
                        pos.tile_rel_pos.z -= 0.25f;
                        v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                        DrawBitmapLightnessInPixels(buffer, &assets->water[0], screen_pos, 0.5f);

                        pos.tile_rel_pos.z += 0.25f;
                        screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                        DrawBitmapInPixels(buffer, &assets->tree_port_pier_pole, screen_pos);
                    } break;

                    default: {
                        DrawBitmapInPixels(buffer, &assets->missing_bitmap, screen_pos);
                    } break;
                }
            } else { // Water
                pos.tile_rel_pos.z -= 0.25f;
                v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                DrawBitmapInPixels(buffer, &assets->water[0], screen_pos);
                TileMapPosition left_tile_map_pos = pos;
                left_tile_map_pos.tile_pos.x--;
                RecanonicalizePosition(tile_map, &left_tile_map_pos);

                EntityGroup entities = GetEntities(tile_map, left_tile_map_pos);
                Entity *entity = entities.entities[0];

                if (entity) {
                    if (entity->type == EntityType_Wall || entity->type == EntityType_MovableWall ||
                        entity->type == EntityType_MossyWall) {
                        DrawBitmapInPixels(buffer, &assets->shadows[ShadowType_Large], screen_pos);
                    } else if (entity->type == EntityType_Tim) {
                        DrawBitmapInPixels(buffer, &assets->shadows[ShadowType_Small], screen_pos);
                    }
                } else {
                    TileType tile_type_left = GetTileType(tile_map, left_tile_map_pos);
                    if (tile_type_left == TileType_Grass0 ||
                        tile_type_left == TileType_Grass1 ||
                        tile_type_left == TileType_TreePortPier ||
                        tile_type_left == TileType_TreePortPierPole) {
                        DrawBitmapInPixels(buffer, &assets->shadows[ShadowType_Small], screen_pos);
                    }
                }
            }
        }
    }
    END_TIMED_FUNCTION();
}

function void
DrawTileWireframe(RenderBuffer *buffer, World *world, Camera *camera) {
    for (s32 rel_y = 16; rel_y >= -17; --rel_y) {
        for (s32 rel_x = -2; rel_x < 32; ++rel_x) {
            s32 x = rel_x + (s32)camera->pos.x;
            s32 y = rel_y + (s32)camera->pos.y;
            TileMapPosition pos = { 0 };
            pos.tile_pos.x = x;
            pos.tile_pos.y = y;

            v2 border_min = V2(pos.tile_pos.x + pos.chunk_pos.x * 16.0f, pos.tile_pos.y + pos.chunk_pos.y * 16.0f);
            border_min = V2SubV2(border_min, camera->pos);

            v2 border_max = V2AddV2(border_min, V2(1, 1));

            DrawBorderAroundTile(buffer, border_min, border_max, V4(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }
}

function void
DrawEntityWireframe(RenderBuffer *buffer, World *world, Camera *camera) {
    Entity *tim = world->tim;

    for (s32 chunk_y = -1; chunk_y < 2; ++chunk_y) {
        for (s32 chunk_x = -1; chunk_x < 2; ++chunk_x) {
            v2 border_min = V2((tim->pos.chunk_pos.x + chunk_x) * 16.0f, (tim->pos.chunk_pos.y + chunk_y) * 16.0f);
            border_min = V2SubV2(border_min, camera->pos);

            v2 border_max = V2AddV2(border_min, V2(16, 16));

            DrawBorderAroundTile(buffer, border_min, border_max, V4(0.0f, 0.0f, 1.0f, 1.0f));
        }
    }

    v2 border_min = V2(tim->pos.chunk_pos.x * 16.0f, tim->pos.chunk_pos.y * 16.0f);
    border_min = V2SubV2(border_min, camera->pos);
    v2 border_max = V2AddV2(border_min, V2(16, 16));

    DrawBorderAroundTile(buffer, border_min, border_max, V4(1.0f, 1.0f, 0.0f, 1.0f));
}

function void
DrawMovementGuide(RenderBuffer *buffer, Bitmap *bitmap, f32 dt) {
    local_persist f32 guide_timer = 0.0f;
    local_persist b32 increment = true;
    local_persist b32 decrement = false;
    f32 width = (f32)bitmap->width;
    f32 height = (f32)bitmap->height;
    v2 top_left;
    top_left.x = (buffer->width - width) / 2;
    top_left.y = buffer->height - height;
    top_left.y -= 100;
    DrawBitmapAlphaInPixels(buffer, bitmap, top_left, QuadraticEaseOut(guide_timer));

    if (increment) {
        guide_timer += dt / 2;
        if (guide_timer >= 0.5f) {
            decrement = true;
            increment = false;
        }
    } else if (decrement) {
        guide_timer -= dt / 2;
        if (guide_timer <= 0.0f) {
            increment = true;
            decrement = false;
        }
    }

    guide_timer = Clamp(0, guide_timer, 1.0f);
}

function void
SimulateGame(RenderBuffer *buffer, ThreadContext *thread,
             Input *input, GameState *game_state) {
    World *world = game_state->world;
    TileMap *tile_map = world->tile_map;
    GameAssets *assets = &game_state->assets;
    Entity *tim = world->tim;
    Camera *camera = &game_state->camera;

    if (!tim->move_event.active) {
        world->tim_pushing = false;
        world->tim_running = false;
        TileMapPosition new_player_pos = tim->pos;
        if (KeyDown(input, KeyCode_W)) {
            tim->facing_direction = Direction_Up;
            new_player_pos.tile_pos.y++;
        } else if (KeyDown(input, KeyCode_S)) {
            tim->facing_direction = Direction_Down;
            new_player_pos.tile_pos.y--;
        } else if (KeyDown(input, KeyCode_A)) {
            tim->facing_direction = Direction_Left;
            new_player_pos.tile_pos.x--;
        } else if (KeyDown(input, KeyCode_D)) {
            tim->facing_direction = Direction_Right;
            new_player_pos.tile_pos.x++;
        }

        RecanonicalizePosition(tile_map, &new_player_pos);

        b32 valid_move = false;

        if (TimCanMove(world, new_player_pos, tim->facing_direction)) {
            valid_move = true;
        }

        if (valid_move) {
            if (!CompareTileMapPosition(tim->pos, new_player_pos)) {
                game_state->player_has_moved = true;
                world->tim_running = true;
                if (valid_move) {
                    // The player should move
                    StartMoveEvent(tim, new_player_pos, 1.0f);
                }
            }
        }

    }

    if (KeyPressed(input, KeyCode_Escape)) {
        game_state->program_mode = ProgramMode_Menu;
    }

    if (KeyDown(input, KeyCode_Space)) {
        world->DEBUG_tim_fast = true;
    } else {
        world->DEBUG_tim_fast = false;
    }

    if (KeyPressed(input, KeyCode_F1)) {
        game_state->debug_menu_enabled = !game_state->debug_menu_enabled;
    }

    local_persist f32 animation_timer;
    if (world->tim_running) {
        if (animation_timer > 0.2f) {
            world->curr_animation_frame++;
            animation_timer = 0;
        }
        world->curr_animation_frame = Cycle(0, world->curr_animation_frame, 3);
        animation_timer += input->dt;
    } else {
        animation_timer = 0;
        world->curr_animation_frame = 0;
    }

#if 1 
    v2 target_camera_pos = V2(tim->pos.chunk_pos.x * 16.0f, tim->pos.chunk_pos.y * 16.0f);
    target_camera_pos = V2AddV2(target_camera_pos, V2(-7.5f, 8.5f));
    if (!camera->should_move) {
        if (target_camera_pos.x != camera->pos.x || target_camera_pos.y != camera->pos.y) {
            camera->should_move = true;
            camera->start_pos = camera->pos;
            camera->end_pos = target_camera_pos;
        }
    }

    if (camera->should_move) {
        if (camera->timer >= 1.0f) {
            camera->timer = 0;
            camera->should_move = false;
            camera->pos = camera->end_pos;
        } else {
            camera->pos = SmoothStepV2(camera->start_pos, camera->end_pos, camera->timer);
            camera->timer += input->dt * 1.5f;
        }
    }
#else
    camera->pos = V2(tim->pos.chunk_pos.x * 16.0f + tim->pos.tile_pos.x + tim->pos.tile_rel_pos.x,
                     tim->pos.chunk_pos.y * 16.0f + tim->pos.tile_pos.y + tim->pos.tile_rel_pos.y);
    camera->pos = V2AddV2(camera->pos, V2(-14.5f, 1.5f));
#endif

    DrawTileMap(tile_map, buffer, camera, assets);

    Activator *activator = world->activators;
    for (u32 activator_index = 0; activator_index < 32; ++activator_index, ++activator) {
        SimActivator(buffer, world, camera, input->dt, activator);
    }

    s32 region_size = 1;
    u32 entity_region_count = 0;
    Entity *entities_in_region[256 * 9] = { 0 };

    u32 num_chunks = 0;
    Chunk *chunks[9];

    for (s32 chunk_y = -region_size; chunk_y <= region_size; ++chunk_y) {
        for (s32 chunk_x = -region_size; chunk_x <= region_size; ++chunk_x) {
            Chunk *chunk = GetChunk(tile_map, tim->pos.chunk_pos.x + chunk_x, tim->pos.chunk_pos.y + chunk_y);
            if (chunk) {
                if (chunk->entities) {
                    UpdateChunkEntities(world, chunk, input->dt);
                }
                chunks[num_chunks++] = chunk;
            }
        }
    }

    for (u32 i = 0; i < num_chunks; ++i) {
        Chunk *chunk = chunks[i];
        if (chunk) {
            if (chunk->entities) {
                for (u32 i = 0; i < 256; ++i) {
                    if (chunk->entities[i]) {
                        entities_in_region[entity_region_count++] = chunk->entities[i];
                    }
                }
            }
        }
    }

    SortEntitiesInRegion(entities_in_region, entity_region_count);

    SimEntitiesInRegion(world, entities_in_region, entity_region_count, assets, buffer, camera, input->dt, true);

    if (!game_state->player_has_moved) {
        DrawMovementGuide(buffer, &assets->wsad_guide, input->dt);
    }
}

function void
LoadTileMap(GameState *game_state, u32 *tiles, u32 tile_map_width, u32 tile_map_height, f32 dt) {
    World *world = game_state->world;
    TileMap *tile_map = world->tile_map;

    for (s32 tile_y = 0; tile_y < 48; ++tile_y) {
        for (s32 tile_x = 0; tile_x < 48; ++tile_x) {
            TileMapPosition pos = { 0 };
            pos.tile_pos.x = tile_x;
            pos.tile_pos.y = tile_y;
            RecanonicalizePosition(tile_map, &pos);

            pos.chunk_pos.x += tile_map->chunk_count_x / 2;
            pos.chunk_pos.y += tile_map->chunk_count_y / 2;

            u32 value = tiles[Abs(tile_y - 47) * tile_map_width + tile_x];
            switch (value) {
                case 0: { // water
                    // SetTileType(&game_state->world_arena, tile_map, pos, TileType_Water);
                } break;

                case 1: { // grass
                    u32 random_number = GetNextRandom();
                    u32 grass_index = random_number % 2;
                    SetTileType(&game_state->world_arena, tile_map, pos, grass_index == 0 ? TileType_Grass0 : TileType_Grass1);
                } break;

                case 2: { // wall
                    u32 random_number = GetNextRandom();
                    u32 grass_index = random_number % 2;
                    SetTileType(&game_state->world_arena, tile_map, pos, grass_index == 0 ? TileType_Grass0 : TileType_Grass1);
                    PushEntity(&game_state->world_arena, world, EntityType_Wall, V3((f32)pos.tile_pos.x + pos.chunk_pos.x * 16,
                                                                                    (f32)pos.tile_pos.y + pos.chunk_pos.y * 16,
                                                                                    0.5f));
                } break;

                case 3: { // moveable wall
                    u32 random_number = GetNextRandom();
                    u32 grass_index = random_number % 2;
                    SetTileType(&game_state->world_arena, tile_map, pos, grass_index == 0 ? TileType_Grass0 : TileType_Grass1);
                    PushEntity(&game_state->world_arena, world, EntityType_MovableWall, V3((f32)pos.tile_pos.x + pos.chunk_pos.x * 16,
                                                                                           (f32)pos.tile_pos.y + pos.chunk_pos.y * 16,
                                                                                           0.5f));
                } break;

                case 4: { // tree port pier
                    SetTileType(&game_state->world_arena, tile_map, pos, TileType_TreePortPier);
                } break;

                case 5: { // tree port pier pole
                    SetTileType(&game_state->world_arena, tile_map, pos, TileType_TreePortPierPole);
                } break;

                case 6: { // vertical portal
                    world->portal_emitter = PushEntity(&game_state->world_arena, world, EntityType_Emitter, V3((f32)pos.tile_pos.x + pos.chunk_pos.x * 16,
                                                                                                               (f32)pos.tile_pos.y + pos.chunk_pos.y * 16,
                                                                                                               0));

                    world->portal_emitter->num_particles = 64;
                    world->portal_emitter->particles = PushArray(&game_state->world_arena, world->portal_emitter->num_particles, Particle);

                    u32 random_number = GetNextRandom();
                    u32 grass_index = random_number % 2;
                    SetTileType(&game_state->world_arena, tile_map, pos, grass_index == 0 ? TileType_Grass0 : TileType_Grass1);
                    PushEntity(&game_state->world_arena, world, EntityType_DoorPortalVert, V3((f32)pos.tile_pos.x + pos.chunk_pos.x * 16,
                                                                                              (f32)pos.tile_pos.y + pos.chunk_pos.y * 16,
                                                                                              1.5f));
                } break;

                case 8: {

                } break;

                case 9: { // door
                    SetTileType(&game_state->world_arena, tile_map, pos, TileType_Grass0);
                    world->doors[world->num_doors++] = PushEntity(&game_state->world_arena, world, EntityType_Door, V3((f32)pos.tile_pos.x + pos.chunk_pos.x * 16,
                                                                                                                       (f32)pos.tile_pos.y + pos.chunk_pos.y * 16,
                                                                                                                       0.5f));
                } break;

                case 10: { // Pressure plate
                    SetTileType(&game_state->world_arena, tile_map, pos, TileType_Grass0);
                    pos.tile_rel_pos.z = 0.5f;
                } break;

                case 11: { // Mossy wall
                    SetTileType(&game_state->world_arena, tile_map, pos, TileType_Grass0);
                    PushEntity(&game_state->world_arena, world, EntityType_MossyWall, V3((f32)pos.tile_pos.x + pos.chunk_pos.x * 16,
                                                                                         (f32)pos.tile_pos.y + pos.chunk_pos.y * 16,
                                                                                         0.5f));
                } break;

                case 12: {
                    u32 random_number = GetNextRandom();
                    u32 grass_index = random_number % 2;
                    SetTileType(&game_state->world_arena, tile_map, pos, grass_index == 0 ? TileType_Grass0 : TileType_Grass1);
                } break;

                default: {
                    u32 random_number = GetNextRandom();
                    u32 grass_index = random_number % 2;
                    SetTileType(&game_state->world_arena, tile_map, pos, grass_index == 0 ? TileType_Grass0 : TileType_Grass1);

                    u32 activator_index = (value - (value % 10)) / 10;
                    u32 activator_line_index = value % 10;
                    Activator *activator = &world->activators[activator_index];
                    if (activator_index == 3) {
                        activator->type = ActivatorType_Green;
                    } else if (activator_index == 4) {
                        activator->type = ActivatorType_Blue;
                    }

                    world->activators[activator_index].lines[activator_line_index].pos = pos;

                    world->activators[activator_index].num_lines++;
                } break;
            }
        }
    }

    // NOTE(hampus): set the directions of all the activator lines
    Activator *activator = world->activators;
    for (u32 i = 0; i < 32; ++i, ++activator) {
        if (activator->num_lines) {
            SetLineDirections(activator);
        }
    }

    world->activators[3].doors[0] = world->doors[0];
    world->activators[3].doors[1] = world->doors[1];

    world->activators[4].doors[0] = world->doors[2];
    world->activators[4].doors[1] = world->doors[3];

    Entity *portal_emitter = world->portal_emitter;
    if (portal_emitter) {
        Particle *particle = portal_emitter->particles;
        for (u32 i = 0; i < portal_emitter->num_particles; ++i, ++particle) {
            particle->pos = portal_emitter->pos;
            particle->pos.tile_rel_pos.x = GetRandomBetween(portal_emitter->pos.tile_rel_pos.x + 0.5f, portal_emitter->pos.tile_rel_pos.x + 1.5f);
            particle->pos.tile_rel_pos.z = GetRandomBetween(0, 2.0f);
            f32 x_vel = GetRandomBetween(-0.3f, 0.3f);
            f32 y_vel = GetRandomBetween(-0.3f, 0.3f);
            f32 z_vel = GetRandomBetween(0.5f, 1.0f);
            // @Refactoring @Incomplete: Should we really start animating the particles here?
            x_vel *= dt;
            y_vel *= dt;
            z_vel *= dt;
            particle->vel = V3(x_vel, y_vel, z_vel);
            f32 color = GetRandomBetween(0.0f, 1.0f);
            particle->color = V4(0.0f, color, color, 1.0f);
        }
    }

    world->tim = PushEntity(&game_state->world_arena, world, EntityType_Tim, V3((f32)5 + tile_map->chunk_count_x / 2 * 16,
                                                                                (f32)7 + tile_map->chunk_count_y / 2 * 16,
                                                                                0.5f));
    world->tim->facing_direction = Direction_Right;
}

function void
CenterCameraAroundEntity(Camera *camera, Entity *entity) {
    camera->pos = V2(entity->pos.chunk_pos.x * 16.0f,
                     entity->pos.chunk_pos.y * 16.0f);
    camera->pos = V2AddV2(camera->pos, V2(-7.5f, 8.5f));
}

function void
ToggleEditor(GameState *game_state) {
    if (game_state->program_mode == ProgramMode_Editor && game_state->world->tim) {
        game_state->program_mode = ProgramMode_Game;
        CenterCameraAroundEntity(&game_state->camera, game_state->world->tim);
        g_zoom = 1.0f;
    } else if (game_state->program_mode == ProgramMode_Game) {
        game_state->program_mode = ProgramMode_Editor;
        CenterCameraAroundEntity(&game_state->camera, game_state->world->tim);
    }
}

GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
#if AENIGMA_INTERNAL
    g_memory = memory;
#endif

    BEGIN_TIMED_FUNCTION();

    Assert((&input->terminator - &input->key_states[0]) <= ArrayCount(input->key_states));
    Assert(sizeof(GameState) <= memory->permanent_storage_size);

    GameState *game_state = (GameState *)memory->permanent_storage;
    platform = &memory->platform;

    if (!memory->is_initialized) {
        InitializeArena(&game_state->world_arena, memory->permanent_storage_size - sizeof(GameState), (u8 *)memory->permanent_storage + sizeof(GameState));

        ResetWorldArena(game_state, &game_state->world_arena);

#if AENIGMA_INTERNAL
        DEBUGLoadGameAssets(thread, &game_state->assets, platform->ReadEntireFile);
#endif

#if AENIGMA_RELEASE
        LoadGameAssets(&game_state->assets);
#endif

#if 1
        LoadTileMap(game_state, (u32 *)tiles_demo, 48, 48, input->dt);
#else
        LoadWorldSave(game_state, &game_state->world_arena, "default");
#endif
        World *world = game_state->world;

        game_state->camera.pos = V2(world->tim->pos.chunk_pos.x * 16.0f, world->tim->pos.chunk_pos.y * 16.0f);
        game_state->camera.pos = V2AddV2(game_state->camera.pos, V2(-7.5f, 8.5f));

        memory->is_initialized = true;
    }
    World *world = game_state->world;

    ClearScreen(buffer, V4(0.2f, 0.2f, 0.2f, 1.0f));

    switch (game_state->program_mode) {
        case ProgramMode_Menu:
        {
            memory->exit_requested = SimulateMenu(buffer, thread, input, game_state);
        } break;

        case ProgramMode_Game:
        {
            SimulateGame(buffer, thread, input, game_state);
        } break;

        case ProgramMode_Editor:
        {
            SimulateEditor(memory, buffer, game_state, input);
        } break;
    }

    if (memory->assets_folder_changed) {
        DEBUGLoadGameAssets(thread, &game_state->assets, platform->ReadEntireFile);
        memory->assets_folder_changed = false;
    }

    if (KeyPressed(input, KeyCode_F1)) {
        game_state->draw_debug_info = !game_state->draw_debug_info;
    } else if (KeyPressed(input, KeyCode_F2)) {
        ToggleEditor(game_state);

    } else if (KeyPressed(input, KeyCode_F3)) {
        // reserved for compilation
    } else if (KeyPressed(input, KeyCode_F4)) {
        game_state->draw_debug_guides = !game_state->draw_debug_guides;
    } else if (KeyPressed(input, KeyCode_F5)) {
        // reserved for looping
    } else if (KeyPressed(input, KeyCode_F6)) {
    } else if (KeyPressed(input, KeyCode_F7)) {
    } else if (KeyPressed(input, KeyCode_F8)) {
    } else if (KeyPressed(input, KeyCode_F9)) {
    } else if (KeyPressed(input, KeyCode_F10)) {
    } else if (KeyPressed(input, KeyCode_F11)) {
        // reserved for fullscreen
    } else if (KeyPressed(input, KeyCode_F12)) {
    }

    if (game_state->draw_debug_guides) {
        DrawLine(buffer, V2(0.5f, 0.0f), V2(0.5f, 1.0f), white);
        DrawLine(buffer, V2(0.0f, 0.5f), V2(1.0f, 0.5f), white);
    }

    END_TIMED_FUNCTION();

#if 0
    if (game_state->draw_debug_info) {
        local_persist f32 y = 0.05f;
        DrawRect(buffer, V2(0.0f, 0.0f), V2(0.5f, y), V4(0.2f, 0.2f, 0.2f, 0.5f));
        char fps_text[256];

        y = 0.05f;

        char test[256];

        for (u32 i = 0; i < 16; ++i) {
            if (g_memory->counter[i].call_count) {
                Rect text_rect = {
                    .min = V2(0, y),
                    .max = V2(0.5f, y + 0.03f),
                };
                StringFormat(test, 256, "%-25s %10llucy (%6.02f%%)",
                             g_memory->counter[i].function_name, g_memory->counter[i].cycle_count,
                             (f32)g_memory->counter[i].cycle_count / (f32)g_memory->counter[id].cycle_count * 100.0f);
                v2 text_pos = GetCenterAlignedPosOfText(test, 1.0f, text_rect);
                DrawText(buffer, test, 1.0f, text_pos, V4(1.0f, 1.0f, 1.0f, 1.0f));
                DrawBorderAroundRect(buffer, text_rect.min, text_rect.max, V4(1.0f, 1.0f, 1.0f, 1.0f), 1);
                y += 0.03f;
            }
        }
    }
#endif

    for (s32 i = 0; i < ArrayCount(g_memory->counter); ++i) {
        g_memory->counter[i].call_count = 0;
        g_memory->counter[i].cycle_count = 0;
    }

    game_state->seconds_elapsed += input->dt;
}