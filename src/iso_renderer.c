function inline v2
IsoToScreen(v2 pos)
{
    v2 result;

    result.x = (pos.x * 0.5f + pos.y * -0.5f);
    result.y = (pos.x * 0.25f + pos.y * 0.25f);

    return result;
}

function inline v2
ScreenToIso(v2 pos)
{
    v2 result;

    result.x = pos.x * 1 + pos.y * 2;
    result.y = pos.x * -1 + pos.y * 2;

    return result;
}

f32 g_zoom = 1.0f;

function v2
ConvertTileMapPositionToScreen(TileMapPosition pos, Camera *camera, v2 offset)
{

    f32 pixels_per_meter = PIXELS_PER_METER * g_zoom;
    f32 tile_side_in_pixels = pixels_per_meter;

    v2 screen_pos = { 0 };
    screen_pos.x = (f32)(pos.chunk_pos.x * 16.0f + pos.tile_pos.x + pos.tile_rel_pos.x + offset.x) * tile_side_in_pixels;
    screen_pos.y = (f32)(pos.chunk_pos.y * 16.0f + pos.tile_pos.y + pos.tile_rel_pos.y + offset.y) * tile_side_in_pixels;
    screen_pos = V2SubV2(screen_pos, V2MulF32(camera->pos, pixels_per_meter));
    screen_pos.y = -screen_pos.y;
    screen_pos = IsoToScreen(screen_pos);

    screen_pos.y -= (pos.tile_rel_pos.z + pos.tile_pos.z) * tile_side_in_pixels;

    return screen_pos;
}

function void
DrawBorderAroundTile(RenderBuffer *buffer, v2 min, v2 max, v4 color)
{
    f32 pixels_per_meter = PIXELS_PER_METER * g_zoom;
    v2 top_left = V2(min.x, min.y);
    v2 top_right = V2(max.x, min.y);
    v2 bottom_left = V2(min.x, max.y);
    v2 bottom_right = V2(max.x, max.y);

    top_left.y -= 0.5f;
    top_left.x += 0.5f;

    top_right.x += 0.5f;
    top_right.y -= 0.5f;

    bottom_left.x += 0.5f;
    bottom_left.y -= 0.5f;

    bottom_right.x += 0.5f;
    bottom_right.y -= 0.5f;

    top_left = V2MulF32(top_left, pixels_per_meter);
    top_right = V2MulF32(top_right, pixels_per_meter);
    bottom_left = V2MulF32(bottom_left, pixels_per_meter);
    bottom_right = V2MulF32(bottom_right, pixels_per_meter);

    top_left.y = -top_left.y;
    top_right.y = -top_right.y;
    bottom_left.y = -bottom_left.y;
    bottom_right.y = -bottom_right.y;

    top_left = IsoToScreen(top_left);
    top_right = IsoToScreen(top_right);
    bottom_right = IsoToScreen(bottom_right);
    bottom_left = IsoToScreen(bottom_left);

    DrawLineInPixels(buffer, top_right, top_left, color);
    DrawLineInPixels(buffer, top_right, bottom_right, color);
    DrawLineInPixels(buffer, bottom_left, bottom_right, color);
    DrawLineInPixels(buffer, top_left, bottom_left, color);
}

function void
DrawLineBetweenTiles(RenderBuffer *buffer, TileMapPosition pos0, TileMapPosition pos1, Camera *camera, v4 color, s32 thickness)
{
    f32 pixels_per_meter = PIXELS_PER_METER;

    v3 tile_pos0 = GetTilePos(pos0);
    v3 tile_pos1 = GetTilePos(pos1);

    tile_pos0.x += 1.0f;
    tile_pos1.x += 1.0f;

    v2 point0 = V2(tile_pos0.x, tile_pos0.y);
    v2 point1 = V2(tile_pos1.x, tile_pos1.y);

    point0 = V2SubV2(point0, camera->pos);
    point1 = V2SubV2(point1, camera->pos);

    point0 = V2MulF32(point0, pixels_per_meter);
    point1 = V2MulF32(point1, pixels_per_meter);

    point0.y = -point0.y;
    point1.y = -point1.y;

    point0 = IsoToScreen(point0);
    point1 = IsoToScreen(point1);

    point0 = V2MulF32(point0, g_zoom);
    point1 = V2MulF32(point1, g_zoom);

    DrawThickLineInPixels(buffer, point0, point1, thickness, color);
}