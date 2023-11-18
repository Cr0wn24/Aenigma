function b32
SimulateMenu(RenderBuffer *buffer, ThreadContext *thread, Input *input, GameState *game_state)
{
    if(KeyPressed(input, KeyCode_Escape))
    {
        game_state->program_mode = !game_state->program_mode;
    }

    Rect screen_rect = { 0 };
    screen_rect.max = V2((f32)buffer->width, (f32)buffer->height);
#if 0
    Rect play_rect = { 0 };
    play_rect.color = V4(1.0f, 0.0f, 0.0f, 1.0f);
    play_rect.max = V2(80, 16);
    CenterRect(&play_rect, screen_rect);

    Rect new_game_rect = { 0 };
    new_game_rect.color = V4(1.0f, 0.0f, 0.0f, 1.0f);
    new_game_rect.max = V2(160, 16);
    CenterRect(&new_game_rect, screen_rect);

    new_game_rect.min.y += 60;
    new_game_rect.max.y += 60;

    Rect quit_rect = { 0 };
    quit_rect.color = V4(1.0f, 0.0f, 0.0f, 1.0f);
    quit_rect.max = V2(80, 16);
    CenterRect(&quit_rect, screen_rect);
    quit_rect.min.y += 120;
    quit_rect.max.y += 120;
#endif
    ClearScreen(buffer, V4(0, 0.2f, 0.3f, 1.0f));

    DrawText(buffer, "AENIGMA", 3.0f, V2(0.38f, 0.2f), V4(1.0f, 1.0f, 1.0f, 1.0f));

    /*
    // @Incomplete: This still needs to be here to navigate with the keyboard.
    // Decide somehow if the user is using the
    // keyboard or mouse and navigate accordingly

    if (PointInsideRect(mouse_pos, play_rect)) {
        selected_button = MenuButton_Play;
    } else if (PointInsideRect(mouse_pos, new_game_rect)) {
        selected_button = MenuButton_StartNewGame;
    } else if (PointInsideRect(mouse_pos, quit_rect)) {
        selected_button = MenuButton_Quit;
    }
    if (selected_button == MenuButton_Play) {
        play_button_color = selection_color;
    } else if (selected_button == MenuButton_Quit) {
        quit_button_color = selection_color;
    } else if (selected_button == MenuButton_StartNewGame) {
        start_new_game_button_color = selection_color;
    }
    */
    UI_Layout menu_ui = { 0 };
    menu_ui.num_buttons = 3;

    UI_BeginLayout(&menu_ui, buffer, input);

#if 0
    if(TextButton("PLAY", play_rect, 2.0f))
    {
        game_state->program_mode = ProgramMode_Game;
    }

    if(TextButton("NEW GAME", new_game_rect, 2.0f))
    {
        // Not implemented
    }

    if(TextButton("QUIT", quit_rect, 2.0f))
    {
        return true;
    }
#endif

    UI_EndLayout();

    return false;
}