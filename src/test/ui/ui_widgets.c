function UI_Comm
UI_Button(char *string) {
    UI_Widget *widget = UI_WidgetMake(string,
                                      UI_WidgetFlag_Clickable |
                                      UI_WidgetFlag_DrawBorder |
                                      UI_WidgetFlag_DrawText |
                                      UI_WidgetFlag_DrawBackground |
                                      UI_WidgetFlag_HotAnimation |
                                      UI_WidgetFlag_ActiveAnimation);
    UI_Comm comm = UI_CommFromWidget(widget);

    return comm;
}

function UI_Comm
UI_Text(char *string) {
    UI_Widget *widget = UI_WidgetMake(string,
                                      UI_WidgetFlag_DrawText);

    UI_Comm comm = UI_CommFromWidget(widget);

    return comm;
}