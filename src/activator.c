function void
SetLineDirections(Activator *activator)
{
    ActivatorLine *line = activator->lines;
    for(u32 line_index = 0; line_index < activator->num_lines; ++line_index, ++line)
    {
        b32 first_line = line_index == 0;
        b32 last_line = line_index == (activator->num_lines - 1);
        ActivatorLine *prev_line = 0;
        if(!first_line)
        {
            // NOTE(hampus): now we want to set the end pos of the previous 
            // line and the start pos of the current line 
            prev_line = line - 1;

            TileMapPosition end_pos_prev_line = prev_line->pos;
            if(end_pos_prev_line.tile_pos.x == (line->pos.tile_pos.x + 1))
            {
                line->start_dir = Direction_Right;
                prev_line->end_dir = Direction_Left;
            }
            else if(end_pos_prev_line.tile_pos.x == (line->pos.tile_pos.x - 1))
            {
                line->start_dir = Direction_Left;
                prev_line->end_dir = Direction_Right;
            }
            else if(end_pos_prev_line.tile_pos.y == (line->pos.tile_pos.y + 1))
            {
                line->start_dir = Direction_Up;
                prev_line->end_dir = Direction_Down;
            }
            else if(end_pos_prev_line.tile_pos.y == (line->pos.tile_pos.y - 1))
            {
                line->start_dir = Direction_Down;
                prev_line->end_dir = Direction_Up;
            }

            if(last_line)
            {
                switch(line->start_dir)
                {
                    case Direction_Up:    line->end_dir = Direction_Down;  break;
                    case Direction_Down:  line->end_dir = Direction_Up;    break;
                    case Direction_Right: line->end_dir = Direction_Left;  break;
                    case Direction_Left:  line->end_dir = Direction_Right; break;
                        InvalidCase;
                }
            }
        }
        else
        {

        }
    }
}

function v4
GetActivatorColor(Activator *activator)
{
    v4 result = { 0 };

    switch(activator->type)
    {
        case ActivatorType_Green: {
            result = V4(0.0f, 1.0f, 0.0f, 1.0f);
        } break;

        case ActivatorType_Blue: {
            result = V4(0.0f, 0.0f, 1.0f, 1.0f);
        } break;

        case ActivatorType_Yellow: {
            result = V4(1.0f, 1.0f, 0.0f, 1.0f);
        } break;

        case ActivatorType_Red: {
            result = V4(1.0f, 0.0f, 0.0f, 1.0f);
        } break;
    }

    return result;
}

function void
SimActivator(RenderBuffer *buffer, World *world, Camera *camera, f32 dt, Activator *activator)
{
    BEGIN_TIMED_FUNCTION();
    TileMap *tile_map = world->tile_map;

    activator->ended = true;
    b32 activator_active = (activator->num_lines != 0);
    b32 moveable_wall_on_any_active = false;

    v4 active_color = GetActivatorColor(activator);

    v4 inactive_color = V4MulF32(active_color, 0.25f);

    ActivatorLine *line = activator->lines;
    for(u32 activator_line_index = 0; activator_line_index < activator->num_lines; activator_line_index++, ++line)
    {

        b32 first_line = activator_line_index == 0;
        b32 last_line = (activator_line_index == activator->num_lines - 1);

        ActivatorLine *prev_line = line - 1;
        ActivatorLine *next_line = line + 1;

        EntityGroup entities = GetEntities(tile_map, line->pos);
        Entity *entity = entities.entities[0];

        if(!first_line)
        {
            switch(prev_line->end_dir)
            {
                case Direction_Right: {
                    line->start_dir = Direction_Left;
                } break;

                case Direction_Left: {
                    line->start_dir = Direction_Right;
                } break;

                case Direction_Up: {
                    line->start_dir = Direction_Down;
                } break;

                case Direction_Down: {
                    line->start_dir = Direction_Up;
                } break;
            }
        }

        if(activator->started && !first_line)
        {
            if(entity)
            {
                if(entity->type == EntityType_MovableWall)
                {
                    if(prev_line->active)
                    {
                        line->active = true;
                    }
                }
            }
        }

        if(first_line)
        {
            if(entity)
            {
                if(entity->type == EntityType_MovableWall)
                {
                    activator->started = true;
                    line->active = true;
                }
            }
        }

        if(last_line)
        {
            if(line->active)
            {
                activator->ended = true;
            }
            else
            {
                activator->ended = false;
            }
        }

        v4 color = inactive_color;
        if(line->active)
        {
            color = SmoothStepV4(inactive_color, active_color, activator->timer);
            if(entity)
            {
                if(entity->type == EntityType_MovableWall)
                {
                    moveable_wall_on_any_active = true;
                }
            }
        }

        if(first_line)
        {
            line->start_dir = Direction_Still;
        }

        TileMapPosition middle = line->pos;
        TileMapPosition side0 = middle;
        TileMapPosition side1 = middle;

        switch(line->start_dir)
        {
            case Direction_Right: {
                side0.tile_rel_pos.x += 0.5f;
            } break;

            case Direction_Left: {
                side0.tile_rel_pos.x -= 0.5f;
            } break;

            case Direction_Up: {
                side0.tile_rel_pos.y += 0.5f;
            } break;

            case Direction_Down: {
                side0.tile_rel_pos.y -= 0.5f;
            } break;
        }

        DrawLineBetweenTiles(buffer, side0, middle, camera, V4(0.2f, 0.2f, +0.2f, 1.0f), (s32)(4 * SCALE * g_zoom));
        DrawLineBetweenTiles(buffer, side0, middle, camera, color, (s32)(2 * SCALE * g_zoom));

        if(!last_line)
        {
            TileMapPosition end_pos = next_line->pos;
            if(end_pos.tile_pos.x == (side0.tile_pos.x + 1))
            {
                line->end_dir = Direction_Right;
            }
            else if(end_pos.tile_pos.x == (side0.tile_pos.x - 1))
            {
                line->end_dir = Direction_Left;
            }
            else if(end_pos.tile_pos.y == (side0.tile_pos.y + 1))
            {
                line->end_dir = Direction_Up;
            }
            else if(end_pos.tile_pos.y == (side0.tile_pos.y - 1))
            {
                line->end_dir = Direction_Down;
            }
        }
        else
        {
            // @Refactoring: This sould be based on where the door is located
            line->end_dir = Direction_Up;
        }

        switch(line->end_dir)
        {
            case Direction_Right: {
                side1.tile_rel_pos.x += 0.5f;
            } break;

            case Direction_Left: {
                side1.tile_rel_pos.x -= 0.5f;
            } break;

            case Direction_Up: {
                side1.tile_rel_pos.y += 0.5f;
            } break;

            case Direction_Down: {
                side1.tile_rel_pos.y -= 0.5f;
            } break;
        }

        DrawLineBetweenTiles(buffer, middle, side1, camera, V4(0.2f, 0.2f, 0.2f, 1.0f), (s32)(4 * SCALE * g_zoom));
        DrawLineBetweenTiles(buffer, middle, side1, camera, color, (s32)(2 * SCALE * g_zoom));
    }

    if(activator_active)
    {
        if(activator->ended)
        {
            if(activator->doors[0])
                activator->doors[0]->open = true;
            if(activator->doors[1])
                activator->doors[1]->open = true;
            activator->timer = 1.0f;
        }
        else
        {
            if(activator->doors[0])
                activator->doors[0]->open = false;
            if(activator->doors[1])
                activator->doors[1]->open = false;
        }

        if(activator->timer >= 1.0f)
        {
            activator->decrement_timer = true;
            activator->increment_timer = false;
        }
        else if(activator->timer <= 0.5f)
        {
            activator->increment_timer = true;
            activator->decrement_timer = false;
        }

        if(activator->increment_timer)
        {
            activator->timer += dt / 2;
        }
        else
        {
            activator->timer -= dt / 2;
        }

        activator->timer = Clamp(0.5f, activator->timer, 1.0f);
        if(!moveable_wall_on_any_active)
        {
            activator->started = false;
            for(u32 activator_line_index = 0; activator_line_index < activator->num_lines; activator_line_index++)
            {
                activator->lines[activator_line_index].active = false;;
            }
        }
    }
    END_TIMED_FUNCTION();
}