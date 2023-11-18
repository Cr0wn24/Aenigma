function inline b32
InsideTileMap(TileMap *tile_map, TileMapPosition pos) {
    b32 result;

    result = (pos.chunk_pos.x >= 0 &&
              pos.chunk_pos.y >= 0 &&
              pos.chunk_pos.x < tile_map->chunk_count_x &&
              pos.chunk_pos.y < tile_map->chunk_count_y);

    return result;
}

function void
CenterCameraAroundPoint(Camera *camera, v2 point, f32 old_zoom, f32 new_zoom) {
    v2 iso_mouse_pos_before_zoom = ScreenToIso(point);

    iso_mouse_pos_before_zoom = V2DivF32(iso_mouse_pos_before_zoom, PIXELS_PER_METER * old_zoom);

    v2 iso_mouse_pos_after_zoom = ScreenToIso(point);

    iso_mouse_pos_after_zoom = V2DivF32(iso_mouse_pos_after_zoom, PIXELS_PER_METER * new_zoom);

    camera->pos.x -= (iso_mouse_pos_after_zoom.x - iso_mouse_pos_before_zoom.x);
    camera->pos.y += (iso_mouse_pos_after_zoom.y - iso_mouse_pos_before_zoom.y);
}

function void
CenterCameraAroundTile(Camera *camera, TileMapPosition pos) {
    v3 tile_pos = GetTilePos(pos);
    v2 point = V2(tile_pos.x, tile_pos.y);
    point.x -= 16 / g_zoom;
    camera->pos.x += (point.x - camera->pos.x);
    camera->pos.y += (point.y - camera->pos.y);
}

function void
SimEntitiesInRegionZoom(World *world, Entity **entities, u32 entity_count,
                        GameAssets *assets, RenderBuffer *buffer, Camera *camera,
                        f32 dt, b32 draw_entities, f32 zoom) {
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

        // Entity type-specific stuff
        switch (entity->type) {
            case EntityType_Water: {
                DrawBitmapZoomInPixels(buffer, &assets->water[0], screen_pos, zoom);
            } break;

            case EntityType_Grass: {
                DrawBitmapZoomInPixels(buffer, &assets->grass[0], screen_pos, zoom);
            } break;

            case EntityType_Wall: {
                DrawBitmapZoomInPixels(buffer, &assets->wall, screen_pos, zoom);

                TileType tile_type_right = GetTileTypeRight(tile_map, pos);
                s32 shadow_type = -1;

                switch (tile_type_right) {
                    case TileType_TreePortPier:
                    case TileType_TreePortPierPole:
                    case TileType_Grass1:
                    case TileType_Grass0: {
                        shadow_type = ShadowType_Medium;
                        pos.tile_rel_pos.z -= 0.5f;
                    } break;
                }

                if (shadow_type >= 0) {
                    v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(1, 0));
                    DrawBitmapZoomInPixels(buffer, &assets->shadows[shadow_type], screen_pos, zoom);
                }

            } break;

            case EntityType_MovableWall: {
                DrawBitmapZoomInPixels(buffer, &assets->moveable_wall, screen_pos, zoom);

                TileType tile_type_right = GetTileTypeRight(tile_map, pos);
                s32 shadow_type = -1;
                switch (tile_type_right) {
                    case TileType_TreePortPier:
                    case TileType_TreePortPierPole:
                    case TileType_Grass1:
                    case TileType_Grass0: {
                        shadow_type = ShadowType_Medium;
                        pos.tile_rel_pos.z -= 0.5f;
                    } break;
                }

                if (shadow_type >= 0) {
                    v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(1, 0));
                    DrawBitmapZoomInPixels(buffer, &assets->shadows[shadow_type], screen_pos, zoom);
                }
            } break;

            case EntityType_MossyWall: {
                DrawBitmapZoomInPixels(buffer, &assets->mossy_wall, screen_pos, zoom);

                TileType tile_type_right = GetTileTypeRight(tile_map, pos);
                s32 shadow_type = -1;
                switch (tile_type_right) {
                    case TileType_TreePortPier:
                    case TileType_TreePortPierPole:
                    case TileType_Grass1:
                    case TileType_Grass0: {
                        shadow_type = ShadowType_Medium;
                        pos.tile_rel_pos.z -= 0.5f;
                    } break;
                }

                if (shadow_type >= 0) {
                    v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(1, 0));
                    DrawBitmapZoomInPixels(buffer, &assets->shadows[shadow_type], screen_pos, zoom);
                }
            } break;

            case EntityType_Tim: {
                Bitmap *tim_bitmap = 0;
                tim_bitmap = &assets->tim_bitmaps.still_right;

                Assert(tim_bitmap);
                // @Incomplete: Fix alignment
                screen_pos.x += 20 * zoom * SCALE;
                screen_pos.y += 35 * zoom * SCALE;
                DrawBitmapZoomInPixels(buffer, &assets->shadows[ShadowType_Tim], screen_pos, zoom);
                screen_pos.x -= 20 * zoom * SCALE;
                screen_pos.y -= 35 * zoom * SCALE;
                DrawBitmapZoomInPixels(buffer, tim_bitmap, screen_pos, zoom);
            } break;

            case EntityType_DoorPortalVert: {
                DrawBitmapZoomInPixels(buffer, &assets->door_portal_vertical, screen_pos, zoom);
            } break;

            case EntityType_DoorPortalHori: {
                DrawBitmapZoomInPixels(buffer, &assets->door_portal_horizontal, screen_pos, zoom);
            } break;

            case EntityType_Emitter: {
                Particle *particle = entity->particles;
                for (u32 i = 0; i < entity->num_particles; ++i, ++particle) {
                    // entity->sim_func(entity, particle, dt, camera, buffer);
                }
            } break;

            case EntityType_Door: {
                if (!entity->open) {
                    DrawBitmapZoomInPixels(buffer, &assets->door, screen_pos, zoom);
                }
            } break;

            default: {
                DrawBitmapZoomInPixels(buffer, &assets->missing_bitmap, screen_pos, zoom);
            } break;
        }
    }
}

function inline b32
CanDrawTopOnly(TileMap *tile_map, TileMapPosition pos) {
    b32 result = false;

    TileType tile_type_right = GetTileTypeRight(tile_map, pos);

    TileType tile_type_down = GetTileTypeDown(tile_map, pos);

    result = ((tile_type_right == TileType_Grass0 || tile_type_right == TileType_Grass1) &&
              (tile_type_down == TileType_Grass0 || tile_type_down == TileType_Grass1));

    return result;
}

function void
DrawEditorTileMap(TileMap *tile_map, RenderBuffer *buffer, Camera *camera, GameAssets *assets, f32 zoom) {
    BEGIN_TIMED_FUNCTION();
    s32 min_tile_y = FloorReal32ToInt32(-17 / zoom) - 1;
    s32 min_tile_x = FloorReal32ToInt32(-2 / zoom) - 1;

    s32 max_tile_y = FloorReal32ToInt32(16 / zoom) + 1;
    s32 max_tile_x = FloorReal32ToInt32(32 / zoom) + 1;

    for (s32 rel_y = max_tile_y; rel_y >= min_tile_y; --rel_y) {
        for (s32 rel_x = min_tile_x; rel_x < max_tile_x; ++rel_x) {
            s32 x = rel_x + (s32)camera->pos.x;
            s32 y = rel_y + (s32)camera->pos.y;
            TileMapPosition pos = { 0 };
            pos.tile_pos.x = x;
            pos.tile_pos.y = y;
            RecanonicalizePosition(tile_map, &pos);
            Chunk *chunk = GetChunk(tile_map, pos.chunk_pos.x, pos.chunk_pos.y);
            if (chunk) {
                if (chunk->tiles) {
                    TileType tile_type = GetTileType(tile_map, pos);

                    if (tile_type) {
                        // Inside map
                        // @Speed: Only the top of the grass needs to be drawn most of the time
                        v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                        switch (tile_type) {
                            case TileType_Grass0: {
                                if (CanDrawTopOnly(tile_map, pos)) {
                                    DrawBitmapZoomInPixels(buffer, &assets->grass_top[0], screen_pos, zoom);
                                } else {
                                    DrawBitmapZoomInPixels(buffer, &assets->grass[0], screen_pos, zoom);
                                }
                            } break;

                            case TileType_Grass1: {
                                if (CanDrawTopOnly(tile_map, pos)) {
                                    DrawBitmapZoomInPixels(buffer, &assets->grass_top[1], screen_pos, zoom);
                                } else {
                                    DrawBitmapZoomInPixels(buffer, &assets->grass[1], screen_pos, zoom);
                                }
                            } break;

                            case TileType_TreePortPier: {
                                // @Incomplete: @Refactoring: Make this cleaner
                                pos.tile_rel_pos.z -= 0.25f;
                                v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                                DrawBitmapZoomLightnessInPixels(buffer, &assets->water[0], screen_pos, zoom, 0.5f);

                                pos.tile_rel_pos.z += 0.25f;
                                screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                                DrawBitmapZoomInPixels(buffer, &assets->tree_port_pier, screen_pos, zoom);
                            } break;

                            case TileType_TreePortPierPole: {
                                // @Incomplete: @Refactoring: Make this cleaner
                                pos.tile_rel_pos.z -= 0.25f;
                                v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                                DrawBitmapZoomLightnessInPixels(buffer, &assets->water[0], screen_pos, zoom, 0.5f);

                                pos.tile_rel_pos.z += 0.25f;
                                screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                                DrawBitmapZoomInPixels(buffer, &assets->tree_port_pier_pole, screen_pos, zoom);
                            } break;

                            default: {
                                DrawBitmapZoomInPixels(buffer, &assets->missing_bitmap, screen_pos, zoom);
                            } break;
                        }
                    } else { // Water
                        pos.tile_rel_pos.z -= 0.25f;
                        v2 screen_pos = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                        DrawBitmapZoomInPixels(buffer, &assets->water[0], screen_pos, zoom);
                        TileMapPosition left_tile_map_pos = pos;
                        left_tile_map_pos.tile_pos.x--;
                        RecanonicalizePosition(tile_map, &left_tile_map_pos);

                        EntityGroup entities = GetEntities(tile_map, left_tile_map_pos);
                        Assert(entities.count <= 1);
                        Entity *entity = entities.entities[0];
                        if (entity) {
                            if (entity->type == EntityType_Wall || entity->type == EntityType_MovableWall ||
                                entity->type == EntityType_MossyWall) {
                                DrawBitmapZoomInPixels(buffer, &assets->shadows[ShadowType_Large], screen_pos, zoom);
                            } else if (entity->type == EntityType_Tim) {
                                DrawBitmapInPixels(buffer, &assets->shadows[ShadowType_Small], screen_pos);
                            }
                        } else {
                            TileType tile_type_left = GetTileType(tile_map, left_tile_map_pos);
                            if (tile_type_left == TileType_Grass0 ||
                                tile_type_left == TileType_Grass1 ||
                                tile_type_left == TileType_TreePortPier ||
                                tile_type_left == TileType_TreePortPierPole) {
                                DrawBitmapZoomInPixels(buffer, &assets->shadows[ShadowType_Small], screen_pos, zoom);
                            }
                        }
                    }
                }
            }
        }
    }

    END_TIMED_FUNCTION();
}

function void
AddPattern(TilePatterns *patterns, s32 radius) {
    TilePattern *tile_pattern = &patterns->pattern[patterns->num_patterns++];
    tile_pattern->dim = V2s(radius * 2, radius * 2);
    for (s32 y = -radius; y <= radius; ++y) {
        for (s32 x = -radius; x <= radius; ++x) {
            if (Square(x) + Square(y) < Square(radius)) {
                tile_pattern->pattern[(y + radius) * 32 + (x + radius)] = 1;
            }
        }
    }
}

function Bitmap *
GetEditorBitmap(GameAssets *assets, TileSelection selection) {
    Bitmap *result = 0;

    switch (selection) {
        case EditorSelection_Water: {
            result = &assets->water[0];
        } break;

        case EditorSelection_Grass0: {
            result = &assets->grass[0];
        } break;

        case EditorSelection_TreePortPier: {
            result = &assets->tree_port_pier;
        } break;

        case EditorSelection_TreePortPierPole: {
            result = &assets->tree_port_pier_pole;
        } break;

        case EditorSelection_Wall: {
            result = &assets->wall;
        } break;

        case EditorSelection_MovableWall: {
            result = &assets->moveable_wall;
        } break;

        case EditorSelection_Door: {
            result = &assets->door;
        } break;

        case EditorSelection_MossyWall: {
            result = &assets->mossy_wall;
        } break;

        case EditorSelection_Tim: {
            result = &assets->tim_bitmaps.still_down;
        } break;

        default: {
            result = &assets->missing_bitmap;
        } break;
    }

    return result;
}

function TileType
GetEditorTileType(TileSelection selection) {
    TileType result = 0;

    switch (selection) {
        case EditorSelection_Water: {
            result = TileType_Water;
        } break;

        case EditorSelection_Grass0: {
            result = TileType_Grass0;
        } break;

        case EditorSelection_TreePortPier: {
            result = TileType_TreePortPier;
        } break;

        case EditorSelection_TreePortPierPole: {
            result = TileType_TreePortPierPole;
        } break;

        default: {
            result = 0;
        } break;
    }

    return result;
}

function EntityType
GetEditorEntityType(TileSelection selection) {
    EntityType result = 0;

    switch (selection) {
        case EditorSelection_Wall: {
            result = EntityType_Wall;
        } break;

        case EditorSelection_MovableWall: {
            result = EntityType_MovableWall;
        } break;

        case EditorSelection_Tim: {
            result = EntityType_Tim;
        } break;

        case EditorSelection_Door: {
            result = EntityType_Door;
        } break;

        case EditorSelection_MossyWall: {
            result = EntityType_MossyWall;
        } break;

        default: {
            result = 0;
        } break;
    }

    return result;
}

function inline b32
IsEditorSelectionEntity(TileSelection selection) {
    b32 result;

    result = !(selection >= EditorSelection_Water && selection <= EditorSelection_TreePortPierPole);

    return result;
}

function void
DrawAndUpdateMouseSelection(EditorState *editor_state, World *world, Camera *camera, TileMapPosition selected_pos, RenderBuffer *buffer, GameAssets *assets, Input *input, MemoryArena *world_arena) {
    TileMap *tile_map = world->tile_map;

    TilePatterns *patterns = &editor_state->patterns;

    TilePattern *selected_pattern = &patterns->pattern[editor_state->selected_pattern];

    v2s half_pattern_dim = V2s(selected_pattern->dim.x / 2, selected_pattern->dim.y / 2);

    b32 is_tim = editor_state->selected_tile == EditorSelection_Tim;
    if (!is_tim) {
        for (s32 y = (selected_pattern->dim.y - 1); y >= 0; --y) {
            for (s32 x = 0; x < selected_pattern->dim.x; ++x) {
                if (selected_pattern->pattern[y * 32 + x] == 1) {
                    TileMapPosition pos = selected_pos;
                    pos.tile_pos.x += (x - half_pattern_dim.x);
                    pos.tile_pos.y += (y - half_pattern_dim.y);
                    if (IsEditorSelectionEntity(editor_state->selected_tile)) {
                        pos.tile_rel_pos.z += 0.5f;
                    }
                    RecanonicalizePosition(tile_map, &pos);
                    v2 screenp = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
                    if (InsideTileMap(tile_map, pos)) {
                        DrawBitmapZoomInPixels(buffer, GetEditorBitmap(assets, editor_state->selected_tile), screenp, g_zoom);
                    }
                }
            }
        }
    } else {
        TileMapPosition pos = selected_pos;
        pos.tile_rel_pos.z += 0.5f;
        RecanonicalizePosition(tile_map, &pos);
        v2 screenp = ConvertTileMapPositionToScreen(pos, camera, V2(0, 0));
        if (InsideTileMap(tile_map, pos)) {
            DrawBitmapZoomInPixels(buffer, GetEditorBitmap(assets, editor_state->selected_tile), screenp, g_zoom);
        }
    }

    if (KeyDown(input, KeyCode_MouseLeft)) {
        if (IsEditorSelectionEntity(editor_state->selected_tile)) {
            if (is_tim) {
                selected_pos.tile_rel_pos.z = 0.5f;
                RecanonicalizePosition(tile_map, &selected_pos);
                if (!world->tim) {
                    world->tim = PushEntity(world_arena, world, EntityType_Tim, GetTilePos(selected_pos));
                } else {
                    // MoveEntity(world_arena, world, world->tim, selected_pos);
                }
            } else {
                for (s32 y = (selected_pattern->dim.y - 1); y >= 0; --y) {
                    for (s32 x = 0; x < selected_pattern->dim.x; ++x) {
                        if (selected_pattern->pattern[y * 32 + x] == 1) {
                            TileMapPosition pos = selected_pos;
                            pos.tile_pos.x += (x - half_pattern_dim.x);
                            pos.tile_pos.y += (y - half_pattern_dim.y);
                            RecanonicalizePosition(tile_map, &pos);
                            EntityGroup entities = GetEntities(tile_map, pos);
                            if (InsideTileMap(tile_map, pos) && !entities.count) {
                                u32 random_number = GetNextRandom();
                                u32 grass_index = random_number % 2;
                                SetTileType(world_arena, tile_map, pos, grass_index == 0 ? TileType_Grass0 : TileType_Grass1);
                                pos.tile_rel_pos.z = 0.5f;
                                EntityType type = GetEditorEntityType(editor_state->selected_tile);
                                Entity *e = PushEntity(world_arena, world, type, GetTilePos(pos));

                                if (type == EntityType_Door) {
                                    world->doors[world->num_doors++] = e;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            for (s32 y = (selected_pattern->dim.y - 1); y >= 0; --y) {
                for (s32 x = 0; x < selected_pattern->dim.x; ++x) {
                    if (selected_pattern->pattern[y * 32 + x] == 1) {
                        TileMapPosition pos = selected_pos;
                        pos.tile_pos.x += (x - half_pattern_dim.x);
                        pos.tile_pos.y += (y - half_pattern_dim.y);
                        RecanonicalizePosition(tile_map, &pos);
                        if (InsideTileMap(tile_map, pos)) {
                            if (editor_state->selected_tile == EditorSelection_Grass0 ||
                                editor_state->selected_tile == EditorSelection_Grass0) {
                                u32 random_number = GetNextRandom();
                                u32 grass_index = random_number % 2;
                                SetTileType(world_arena, tile_map, pos, grass_index == 0 ? TileType_Grass0 : TileType_Grass1);
                            } else {
                                SetTileType(world_arena, tile_map, pos, editor_state->selected_tile);
                            }
                        }
                    }
                }
            }
        }
    } else if (KeyDown(input, KeyCode_MouseRight)) {
        EntityGroup entities = GetEntities(tile_map, selected_pos);
        if (entities.count) {
            Entity *entity = entities.entities[0];
            RemoveEntity(world, entity);
        }
    }
}

function void
QueryWorldsInDirectory(EditorState *editor_state, PlatformFindFilesInDirectory *FindFilesInDirectory, char *name) {
    char files_in_directory[32][MAX_WORLD_NAME_LENGTH];

    FindFilesInDirectory(files_in_directory, MAX_WORLD_NAME_LENGTH, name);

    ZeroArray(editor_state->worlds_name, 32, char[MAX_WORLD_NAME_LENGTH]);
    editor_state->num_worlds_name = 0;

    for (u32 i = 0; i < 32; ++i) {
        if (StringLength(files_in_directory[i]) > 5) {
            u32 count = 0;
            for (u32 j = 0; files_in_directory[i][j] != '.'; ++j) {
                editor_state->worlds_name[editor_state->num_worlds_name][count++] = files_in_directory[i][j];
            }
            editor_state->num_worlds_name++;
        }
    }

}

function s32
GetIndexOfStringInArray(char string[MAX_WORLD_NAME_LENGTH], char array[32][MAX_WORLD_NAME_LENGTH]) {
    s32 result = -1;
    for (s32 i = 0; i < 32; ++i) {
        if (StringCompare(string, array[i])) {
            result = i;
        }
    }

    return result;
}

function b32
ValidWorldName(char name[MAX_WORLD_NAME_LENGTH]) {
    b32 result = true;

    s32 length = StringLength(name);
    if (StringEmpty(name)) {
        result = false;
    } else {
        for (s32 i = 0; i < length; ++i) {
            char ch = name[i];

            if (!((ch >= 'a' && ch <= 'z') ||
                  (ch >= 'A' && ch <= 'Z') ||
                  (ch >= '0' && ch <= '9') ||
                  (ch == '_'))) {
                result = false;
            }
        }
    }

    return result;
}

function void
SimulateEditor(GameMemory *memory, RenderBuffer *buffer, GameState *game_state, Input *input) {
    Camera *camera = &game_state->camera;
    GameAssets *assets = &game_state->assets;
    EditorState *editor_state = &game_state->editor_state;
    MemoryArena *world_arena = &game_state->world_arena;
    World *world = game_state->world;
    TileMap *tile_map = world->tile_map;

    if (!editor_state->is_initialized) {
        for (u32 i = 1; i < 10; ++i) {
            AddPattern(&editor_state->patterns, i);
        }

        QueryWorldsInDirectory(editor_state, memory->platform.FindFilesInDirectory, "*");

        editor_state->is_initialized = true;
    }

    v2 mouse_pos_in_pixels = V2(input->mouse_x * buffer->width, input->mouse_y * buffer->height);

    // TODO(hampus): Move this into the editor state
    local_persist v2 last_mouse_pos_in_pixels;

    v2 start_mouse_pos_in_pixels = mouse_pos_in_pixels;

    if (KeyDown(input, KeyCode_MouseMiddle)) {
        v2 mouse_delta = V2SubV2(last_mouse_pos_in_pixels, start_mouse_pos_in_pixels);

        mouse_delta = ScreenToIso(mouse_delta);

        camera->pos.x += mouse_delta.x * input->dt * 2 / g_zoom / SCALE;
        camera->pos.y -= mouse_delta.y * input->dt * 2 / g_zoom / SCALE;
    }

    last_mouse_pos_in_pixels = mouse_pos_in_pixels;

    v2 iso_mouse_pos = ScreenToIso(mouse_pos_in_pixels);

    iso_mouse_pos.y = -iso_mouse_pos.y;

    iso_mouse_pos = V2DivF32(iso_mouse_pos, SCALE * 64.0f * g_zoom);

    s32 tile_x = FloorReal32ToInt32(iso_mouse_pos.x);
    s32 tile_y = FloorReal32ToInt32(iso_mouse_pos.y);

    tile_x += FloorReal32ToInt32(camera->pos.x);
    tile_y += FloorReal32ToInt32(camera->pos.y);

    f32 new_zoom = g_zoom;
    f32 old_zoom = g_zoom;
    if (!currently_typing) {
        if (KeyPressed(input, KeyCode_Q)) {
            if (new_zoom <= 1.0f) {
                new_zoom /= 2;
            } else {
                new_zoom -= 1.0f;
            }
        } else if (KeyPressed(input, KeyCode_E)) {
            if (new_zoom <= 1.0f) {
                new_zoom *= 2;
            } else {
                new_zoom += 1.0f;
            }
        }
    }

    b32 zoomed = new_zoom != old_zoom;
    if (zoomed) {
        CenterCameraAroundPoint(camera, mouse_pos_in_pixels, old_zoom, new_zoom);
        g_zoom = new_zoom;
    }

    DrawEditorTileMap(tile_map, buffer, camera, assets, g_zoom);

    TileMapPosition selected_pos = { 0 };
    selected_pos.tile_pos.x = tile_x;
    selected_pos.tile_pos.y = tile_y + 1;
    RecanonicalizePosition(tile_map, &selected_pos);

    editor_state->selected_pattern += input->mouse_wheel;
    editor_state->selected_pattern = Cycle(0, editor_state->selected_pattern, editor_state->patterns.num_patterns - 1);

    Activator *activator = world->activators;
    for (u32 activator_index = 0; activator_index < 32; ++activator_index) {
        Activator *activator = world->activators;
        for (u32 activator_index = 0; activator_index < 32; ++activator_index) {
            SimActivator(buffer, world, camera, input->dt, activator++);
        }
    }

    s32 region_size = RoundReal32ToInt32(2 / g_zoom);

    for (s32 chunk_y = -region_size; chunk_y <= region_size; ++chunk_y) {
        for (s32 chunk_x = -region_size; chunk_x <= region_size; ++chunk_x) {
            Chunk *chunk = GetChunk(tile_map, (s32)(camera->pos.x / 16) + chunk_x, (s32)(camera->pos.y / 16) + chunk_y);
            if (chunk) {
                if (chunk->entities) {
                    UpdateChunkEntities(world, chunk, input->dt);
                }
            }
        }
    }

    u32 entity_region_count = 0;
    Entity *entities_in_region[256 * 9] = { 0 };

    s32 min_tile_y = RoundReal32ToInt32(-17 / g_zoom) - 1;
    s32 min_tile_x = RoundReal32ToInt32(-2 / g_zoom) - 1;

    s32 max_tile_y = RoundReal32ToInt32(16 / g_zoom) + 1;
    s32 max_tile_x = RoundReal32ToInt32(32 / g_zoom) + 1;

    for (s32 rel_y = max_tile_y; rel_y >= min_tile_y; --rel_y) {
        for (s32 rel_x = min_tile_x; rel_x < max_tile_x; ++rel_x) {
            TileMapPosition pos = { 0 };
            pos.tile_pos.x += (s32)(camera->pos.x + rel_x);
            pos.tile_pos.y += (s32)(camera->pos.y + rel_y);

            RecanonicalizePosition(tile_map, &pos);

            EntityGroup entities = GetEntities(tile_map, pos);

            for (u32 i = 0; i < entities.count; ++i) {
                entities_in_region[entity_region_count++] = entities.entities[i];
            }
        }
    }

    SortEntitiesInRegion(entities_in_region, entity_region_count);

    SimEntitiesInRegionZoom(world, entities_in_region, entity_region_count, assets, buffer, camera, input->dt, true, g_zoom);

    Rect ui_rect = {
        .min = V2(0.01f, 0.05f),
        .max = V2(0.21f, 0.95f),
    };

    b32 mouse_inside_ui = PointInsideRect(V2(input->mouse_x, input->mouse_y), ui_rect);

    // @Refactoring: Move these into the editor state
    b32 highlight_doors = false;

    if (editor_state->placing_activator) {
        highlight_doors = true;
        if (!mouse_inside_ui && !currently_typing) {
            if (KeyDown(input, KeyCode_MouseLeft)) {
                ActivatorLine *line = &editor_state->current_activator->lines[editor_state->current_activator->num_lines];
                b32 is_first_line = editor_state->current_activator->num_lines == 0;
                b32 valid_placement = false;
                if (!is_first_line) {
                    ActivatorLine *prev_line = line - 1;

                    TileMapPosition end_pos_prev_line = prev_line->pos;

                    TileMapPosition valid_pos0 = end_pos_prev_line;
                    TileMapPosition valid_pos1 = end_pos_prev_line;
                    TileMapPosition valid_pos2 = end_pos_prev_line;
                    TileMapPosition valid_pos3 = end_pos_prev_line;

                    valid_pos0.tile_pos.y--;
                    valid_pos1.tile_pos.y++;

                    valid_pos2.tile_pos.x--;
                    valid_pos3.tile_pos.x++;

                    if (CompareTileMapPosition(selected_pos, valid_pos0) ||
                        CompareTileMapPosition(selected_pos, valid_pos1) ||
                        CompareTileMapPosition(selected_pos, valid_pos2) ||
                        CompareTileMapPosition(selected_pos, valid_pos3)) {
                        valid_placement = true;
                    }
                    // check that the line placed is actually close to the prev line
                } else {
                    valid_placement = true;
                }

                if (valid_placement) {
                    line->pos = selected_pos;
                    editor_state->current_activator->num_lines++;
                }

                SetLineDirections(editor_state->current_activator);
            }

        } else {
            DrawAndUpdateMouseSelection(editor_state, world, camera, selected_pos, buffer, assets, input, world_arena);
        }
    }

    // NOTE(hampus): Highlight doors

    if (highlight_doors) {
        for (u32 i = 0; i < 2; ++i) {
            Entity *activator_door = editor_state->current_activator->doors[i];
            if (activator_door) {
                v2 screen_pos = ConvertTileMapPositionToScreen(activator_door->pos, camera, V2(0, 0));

                screen_pos.x += 32;
                screen_pos.y += 32;
                screen_pos.x /= buffer->width;
                screen_pos.y /= buffer->height;

                Rect rect = {
                    .min = V2SubV2(screen_pos, V2(0.01f, 0.01f * buffer->width / buffer->height)),
                    .max = V2AddV2(screen_pos, V2(0.01f, 0.01f * buffer->width / buffer->height)),
                };

                DrawRect(buffer,
                         rect.min,
                         rect.max,
                         white);
            }
        }

        for (u32 i = 0; i < 32; ++i) {
            Entity *door = world->doors[i];
            if (door) {
                v2 screen_pos = ConvertTileMapPositionToScreen(door->pos, camera, V2(0, 0));
                screen_pos.x += 32;
                screen_pos.y += 32;
                screen_pos.x /= buffer->width;
                screen_pos.y /= buffer->height;

                Rect rect = {
                    .min = V2SubV2(screen_pos, V2(0.01f, 0.01f * buffer->width / buffer->height)),
                    .max = V2AddV2(screen_pos, V2(0.01f, 0.01f * buffer->width / buffer->height)),
                };

                if (PointInsideRect(V2(input->mouse_x, input->mouse_y), rect)) {

                    if (KeyPressed(input, KeyCode_MouseLeft)) {
                        if (!editor_state->current_activator->doors[0]) {
                            editor_state->current_activator->doors[0] = door;
                        } else if (!editor_state->current_activator->doors[1]) {
                            editor_state->current_activator->doors[1] = door;
                        }
                    } else if (KeyPressed(input, KeyCode_MouseRight)) {
                        if (editor_state->current_activator->doors[0] == door) {
                            editor_state->current_activator->doors[0] = 0;
                        } else if (editor_state->current_activator->doors[1] == door) {
                            editor_state->current_activator->doors[1] = 0;
                        }
                    }
                }
                DrawBorderAroundRect(buffer,
                                     rect.min,
                                     rect.max,
                                     white, 1);
            }
        }
    }

    // NOTE(hampus): Highlight activators

    for (u32 i = 0; i < 32; ++i) {
        Activator *activator = &world->activators[i];
        if (activator->num_lines) {
            ActivatorLine *first_line = &activator->lines[0];
            v2 screen_pos = ConvertTileMapPositionToScreen(first_line->pos, camera, V2(0, 0));

            screen_pos.x += 32;
            screen_pos.y += 16;
            screen_pos.x /= buffer->width;
            screen_pos.y /= buffer->height;

            Rect rect = {
                .min = V2SubV2(screen_pos, V2(0.01f, 0.01f * buffer->width / buffer->height)),
                .max = V2AddV2(screen_pos, V2(0.01f, 0.01f * buffer->width / buffer->height)),
            };

            if (KeyPressed(input, KeyCode_MouseLeft)) {
                if (PointInsideRect(V2(input->mouse_x, input->mouse_y), rect)) {
                    editor_state->current_activator = activator;
                    editor_state->placing_activator = true;
                }
            }

            DrawBorderAroundRect(buffer,
                                 rect.min,
                                 rect.max,
                                 white, 1);

            if (editor_state->current_activator == activator) {
                DrawRect(buffer,
                         rect.min,
                         rect.max,
                         white);
            }

        }
    }

    local_persist f32 zoom_text_timer = 0;
    if (zoomed) {
        zoom_text_timer = 1.0f;
    }
    zoom_text_timer -= input->dt;

    if (zoom_text_timer > 0.0f) {
        char zoom_text[64];
        StringFormat(zoom_text, 64, "ZOOM: %d%%", (s32)(g_zoom * 100.0f));
        Rect zoom_text_rect = GetTextRect(zoom_text, 2.0f);
        CenterRect(&zoom_text_rect, MakeRect(V2(0, 0), V2((f32)buffer->width, (f32)buffer->height)));
        DrawTextInPixels(buffer, zoom_text, 2.0f, zoom_text_rect.min, V4(1.0f, 1.0f, 1.0f, zoom_text_timer));
    }

    // ui

    f32 width = ui_rect.max.x - ui_rect.min.x;

    DrawRoundedRect(buffer, ui_rect.min, ui_rect.max, 25, V4(0.5f, 0.5f, 0.5f, 1.0f));

    Rect tiles_rect[10];

    for (u32 y = 0; y < 5; ++y) {
        for (u32 x = 0; x < 2; ++x) {
            f32 offset_x = 0.05f;
            f32 offset_y = 0.01f;

            f32 width = 0.05f;
            f32 height = 0.05f * (16.0f / 9.0f);

            v2 min = V2(offset_x + x * width,
                        offset_y + y * height);

            min = V2AddV2(min, ui_rect.min);

            v2 max = V2AddV2(min, V2(width, height));

            Rect rect =
            {
                .min = min,
                .max = max,
            };

            tiles_rect[y * 2 + x] = rect;
        }
    }

    UI_Layout editor_ui = { 0 };

    UI_BeginLayout(&editor_ui, buffer, input);

    for (u32 i = 0; i < 10; ++i) {
        if (BitmapButton(GetEditorBitmap(assets, i), tiles_rect[i].min, tiles_rect[i].max)) {
            editor_state->selected_tile = i;
        }
        DrawBorderAroundRect(buffer, tiles_rect[i].min, tiles_rect[i].max, white, 1);
    }

    if (editor_state->currently_loading || editor_state->currently_saving) {
        DrawRectAlpha(buffer, V2(0, 0), V2(1, 1), V4(0, 0, 0, 0.5f));
    }

    b32 loaded_new_world = false;
    b32 saved_world = false;

    Rect input_rect = { 0 };
    f32 rect_height = 16.0f / buffer->height;
    input_rect.max = V2(0.1f, rect_height);
    CenterRect(&input_rect, MakeRect(V2(0, 0), V2(1.0f, 1.0f)));
    if (editor_state->currently_loading) {

        // TODO(hampus): Move this into the editor state
        local_persist s32 curr_selected_save = 0;
        local_persist f32 timer = 0;
        local_persist b32 holding = false;
        local_persist f32 action_rect_timer = 0.0f;
        local_persist s32 selected_action = 1;

        if (KeyDown(input, KeyCode_Down)) {
            if (timer == 0) curr_selected_save++;
            holding = true;
            action_rect_timer = 0;
            selected_action = 1;
        } else if (KeyDown(input, KeyCode_Up)) {
            if (timer == 0) curr_selected_save--;
            holding = true;
            action_rect_timer = 0;
            selected_action = 1;
        } else {
            holding = false;
        }

        if (holding) {
            timer += input->dt / 2;
        } else {
            timer = 0;
        }

        if (timer >= 0.1f) {
            timer = 0;
        }

        if (KeyPressed(input, KeyCode_Right)) {
            selected_action++;
        }

        if (KeyPressed(input, KeyCode_Left)) {
            selected_action--;
        }

        selected_action = Clamp(0, selected_action, 2);

        curr_selected_save = Cycle(0, curr_selected_save, (s32)editor_state->num_worlds_name);

        if (KeyPressed(input, KeyCode_Enter) && curr_selected_save != 0 && selected_action == 1) {
            char *selected_save = editor_state->worlds_name[curr_selected_save - 1];
            if (LoadWorldSave(game_state, world_arena, selected_save)) {
                loaded_new_world = true;
                StringCopy(world->name, MAX_WORLD_NAME_LENGTH, selected_save);
            }
        }

        for (s32 i = 0; i < (s32)editor_state->num_worlds_name; ++i) {
            b32 selected = false;
            if (curr_selected_save) {
                if (i == (curr_selected_save - 1)) selected = true;
            }

            if (selected && KeyPressed(input, KeyCode_Delete)) {
                char path[MAX_WORLD_NAME_LENGTH];
                StringFormat(path, MAX_WORLD_NAME_LENGTH, "worlds\\%s.world", editor_state->worlds_name[i]);
                memory->platform.DeleteFile(path);
                QueryWorldsInDirectory(editor_state, memory->platform.FindFilesInDirectory, editor_state->load_input_buffer);
            }

            Rect file_rect = input_rect;
            file_rect.min.y += rect_height * (i + 1);
            file_rect.max.y += rect_height * (i + 1);
            v2 text_min = GetCenterAlignedPosOfText(editor_state->worlds_name[i], 1.0f, file_rect);

            v4 rect_color = V4(0.5f, 0.5f, 0.5f, 1.0f);
            v4 edit_color = V4(0.5f, 0.5f, 0.5f, 1.0f);
            v4 delete_color = V4(0.5f, 0.5f, 0.5f, 1.0f);
            v4 select_color = V4(0.75, 0.75, 0.75f, 1.0f);
            v4 border_color = V4(0.2f, 0.2f, 0.2f, 1.0f);

            if (selected) {
                Rect action_rect = file_rect;
                action_rect.min.x -= rect_height / 1.5f * QuadraticEaseOut(action_rect_timer);
                action_rect.max.x = action_rect.min.x + rect_height * buffer->height / buffer->width;

                if (selected_action == 0) {
                    DrawRect(buffer, action_rect.min, action_rect.max, select_color);
                } else {
                    DrawRect(buffer, action_rect.min, action_rect.max, edit_color);
                }

                DrawBitmapStretch(buffer, &assets->pen, action_rect.min, action_rect.max);

                DrawBorderAroundRect(buffer, action_rect.min, action_rect.max, border_color, 1);

                action_rect.min.x = file_rect.max.x - +rect_height * buffer->height / buffer->width + rect_height / 1.5f * QuadraticEaseOut(action_rect_timer);
                action_rect.max.x = action_rect.min.x + +rect_height * buffer->height / buffer->width;

                if (selected_action == 2) {
                    DrawRect(buffer, action_rect.min, action_rect.max, select_color);
                } else {
                    DrawRect(buffer, action_rect.min, action_rect.max, delete_color);
                }

                DrawBitmapStretch(buffer, &assets->bin, action_rect.min, action_rect.max);

                DrawBorderAroundRect(buffer, action_rect.min, action_rect.max, border_color, 1);

                action_rect_timer += input->dt * 3;

                action_rect_timer = Clamp(0.0f, action_rect_timer, 1.0f);
            }

            if (selected && selected_action == 1) {
                rect_color = select_color;
            }

            DrawRect(buffer, file_rect.min, file_rect.max, rect_color);
            DrawBorderAroundRect(buffer, file_rect.min, file_rect.max, border_color, 1);

            DrawText(buffer, editor_state->worlds_name[i], 1.0f, text_min, V4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        char *text = "Load:";
        v2 text_pos = GetCenterAlignedPosOfText(text, 1.0f, input_rect);
        text_pos.y -= 0.05f;
        DrawText(buffer, text, 1.0f, text_pos, white);

        b32 typed = false;
        if (!loaded_new_world) {
            if (InputButton(input_rect.min, input_rect.max, editor_state->load_input_buffer, MAX_WORLD_NAME_LENGTH, &typed)) {
                if (ValidWorldName(editor_state->load_input_buffer)) {
                    // valid world-name, load the world save
                    if (LoadWorldSave(game_state, world_arena, editor_state->load_input_buffer)) {
                        loaded_new_world = true;
                        StringCopy(world->name, MAX_WORLD_NAME_LENGTH, editor_state->load_input_buffer);
                    } else {
                        StringCopy(editor_state->invalid_world_name, MAX_WORLD_NAME_LENGTH, editor_state->load_input_buffer);
                        editor_state->world_does_not_exist = true;
                    }
                } else {
                    // invalid world-name, keep the load-screen up
                }
            }

            if (typed) {
                curr_selected_save = 0;
                if (StringEmpty(editor_state->load_input_buffer)) {
                    QueryWorldsInDirectory(editor_state, memory->platform.FindFilesInDirectory, "*");
                } else {
                    QueryWorldsInDirectory(editor_state, memory->platform.FindFilesInDirectory, editor_state->load_input_buffer);
                }
            }

            if (!editor_state->currently_loading) {
                QueryWorldsInDirectory(editor_state, memory->platform.FindFilesInDirectory, "*");
            }
        }

    } else if (editor_state->currently_saving) {
        char *text = "Save:";
        v2 text_pos = GetCenterAlignedPosOfText(text, 1.0f, input_rect);
        text_pos.y -= 20;
        DrawTextInPixels(buffer, text, 1.0f, text_pos, white);
        if (InputButton(input_rect.min, input_rect.max, editor_state->save_input_buffer, MAX_WORLD_NAME_LENGTH, 0)) {
            if (ValidWorldName(editor_state->save_input_buffer)) {
                if (world->tim) {
                    saved_world = true;
                }
            } else {
                editor_state->invalid_save_name = true;
            }
        }
    }

    if (TextButton("Start activator", V2(ui_rect.min.x, ui_rect.max.y - 0.05f), 1.0f)) {
        if (!editor_state->placing_activator) {
            editor_state->placing_activator = true;
            b32 found_activator = false;
            for (u32 i = 0; i < 32; ++i) {
                b32 empty_activator_index = world->activators[i].num_lines == 0;
                if (empty_activator_index) {
                    editor_state->current_activator = &world->activators[i];
                    found_activator = true;
                }
            }
            Assert(found_activator);
        }

    } else if (TextButton("End activator", V2(ui_rect.min.x + (ui_rect.max.x - ui_rect.min.x) / 2, ui_rect.max.y), 1.0f)) {
        if (editor_state->placing_activator) {
            editor_state->placing_activator = false;
            editor_state->current_activator = 0;
        }
    }

    UI_EndLayout();

    if (loaded_new_world) {
        game_state->camera.pos = V2(world->tim->pos.chunk_pos.x * 16.0f, world->tim->pos.chunk_pos.y * 16.0f);
        game_state->camera.pos = V2AddV2(game_state->camera.pos, V2(-7.5f, 8.5f));
        ZeroArray(editor_state->load_input_buffer, MAX_WORLD_NAME_LENGTH, char);
        QueryWorldsInDirectory(editor_state, memory->platform.FindFilesInDirectory, "*");
        editor_state->current_world_index = GetIndexOfStringInArray(world->name, editor_state->worlds_name);
        editor_state->currently_loading = false;
    }

    if (saved_world) {
        SaveWorld(world, editor_state->save_input_buffer);
        StringCopy(world->name, MAX_WORLD_NAME_LENGTH, editor_state->save_input_buffer);
        ZeroArray(editor_state->save_input_buffer, MAX_WORLD_NAME_LENGTH, char);
        QueryWorldsInDirectory(editor_state, memory->platform.FindFilesInDirectory, "*");
        editor_state->currently_saving = false;
    }

    if (editor_state->currently_loading || editor_state->currently_saving) {
        currently_typing = true;
        if (editor_state->invalid_save_name && editor_state->currently_saving) {
            char *text = "Invalid save name";
            v2 text_pos = AlignTextInRectInPixels(text, 1.0f, input_rect, AlignFlag_CenterHeight);
            text_pos.x += 120;
            DrawTextInPixels(buffer, text, 1.0f, text_pos, V4(1.0f, 0, 0, 1.0f));
        } else if (editor_state->world_does_not_exist && editor_state->currently_loading) {
            char text[256];
            StringFormat(text, 256, "World '%s' was not found", editor_state->invalid_world_name);
            v2 text_pos = AlignTextInRect(text, 1.0f, input_rect, AlignFlag_CenterHeight);
            text_pos.x += 0.12f;
            DrawText(buffer, text, 1.0f, text_pos, V4(1.0f, 0, 0, 1.0f));
        }

        if (KeyPressed(input, KeyCode_Escape)) {
            editor_state->currently_loading = false;
            editor_state->currently_saving = false;
            currently_typing = false;
        }
    } else {
        editor_state->invalid_save_name = false;
        editor_state->world_does_not_exist = false;
        if (StringLength(editor_state->invalid_world_name)) {
            ZeroArray(editor_state->invalid_world_name, MAX_WORLD_NAME_LENGTH, 255);
        }
        currently_typing = false;
    }

    if (KeyPressed(input, KeyCode_L) && !currently_typing) {
        editor_state->currently_loading = !editor_state->currently_loading;
    } else if (KeyPressed(input, KeyCode_S) && !currently_typing) {
        editor_state->currently_saving = !editor_state->currently_saving;
    }

    if (StringLength(world->name)) {
        Rect title_rect = { 0 };
        title_rect.max = V2(120, 30);
        CenterRect(&title_rect, MakeRect(V2(0, 0), V2((f32)buffer->width, (f32)buffer->height)));
        title_rect.min.y -= 250;
        title_rect.max.y -= 250;
        v2 text_min = GetCenterAlignedPosOfText(world->name, 2.0f, title_rect);
        DrawTextInPixels(buffer, world->name, 2.0f, title_rect.min, V4(1.0f, 1.0f, 1.0f, 1.0f));
    }
}