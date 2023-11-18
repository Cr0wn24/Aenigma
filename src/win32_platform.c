PLATFORM_FIND_FILES_IN_DIRECTORY(platform_FindFilesInDirectory) {
    ZeroMemory(dest, 32 * dest_size);
    u32 file_count = 0;
    WIN32_FIND_DATA data;
    char path[256] = { 0 };
    StringFormat(path, 256, "worlds\\%s*", file_name);
    HANDLE hFind = FindFirstFile(path, &data);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            StringFormat(dest[file_count++], dest_size, "%s", data.cFileName);
            if (file_count >= 32) {
                break;
            }
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }
}

PLATFORM_DELETE_FILE(platform_DeleteFile) {
    DeleteFileA(path);
}

b32 cursor_visible = true;
PLATFORM_TOGGLE_CURSOR(Win32PlatformToggleCursor) {
    ShowCursor(cursor_visible = !cursor_visible);
}

PLATFORM_READ_ENTIRE_FILE(platform_ReadEntireFile) {
    ReadFileResult result = { 0 };

    HANDLE file = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (file == INVALID_HANDLE_VALUE) {
        // @Incomplete:  Logging
        return result;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file, &file_size)) {
        // @Incomplete:  Logging
        return result;
    }

    Assert(file_size.QuadPart <= 0xffffffff);

    u32 file_size32 = (u32)file_size.QuadPart;

    result.contents = VirtualAlloc(0, file_size32, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    result.content_size = file_size32;
    if (!result.contents) {
        // @Incomplete:  Logging
        return result;
    }

    DWORD bytes_read;
    if (!ReadFile(file, result.contents, file_size32, &bytes_read, 0)) {
        // @Incomplete:  Logging
        VirtualFree(result.contents, 0, MEM_RELEASE);
        return result;
    }

    Assert(bytes_read == file_size32);

    CloseHandle(file);

    return result;
}

PLATFORM_FREE_FILE_MEMORY(platform_FreeMemory) {
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

PLATFORM_WRITE_ENTIRE_FILE(platform_WriteEntireFile) {

    HANDLE file = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (file == INVALID_HANDLE_VALUE) {
        // @Incomplete:  Logging
        return false;
    }

    DWORD bytes_written;
    if (!WriteFile(file, memory, memory_size, &bytes_written, 0)) {
        // @Incomplete:  Logging
        return false;
    }

    if (bytes_written != memory_size) {
        return false;
    }

    CloseHandle(file);

    return true;
}

PLATFORM_RESIZE_RENDER_BUFFER(platform_ResizeRenderBuffer) {
    if (g_backbuffer.memory) {
        VirtualFree(g_backbuffer.memory, 0, MEM_RELEASE);
    }

    g_backbuffer.width = new_width;
    g_backbuffer.height = new_height;
    g_backbuffer.bytes_per_pixel = 4;
    g_backbuffer.pitch = g_backbuffer.bytes_per_pixel * g_backbuffer.width;

    g_backbuffer.info.bmiHeader.biSize = sizeof(g_backbuffer.info.bmiHeader);
    g_backbuffer.info.bmiHeader.biWidth = g_backbuffer.width;
    g_backbuffer.info.bmiHeader.biHeight = -g_backbuffer.height;
    g_backbuffer.info.bmiHeader.biPlanes = 1;
    g_backbuffer.info.bmiHeader.biBitCount = 32;
    g_backbuffer.info.bmiHeader.biCompression = BI_RGB;

    s32 bitmap_memory_size = g_backbuffer.width * g_backbuffer.height * g_backbuffer.bytes_per_pixel;
    g_backbuffer.memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

}
PLATFORM_ALLOC_MEMORY(platform_AllocateMemory) {
    void *result;
    result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return result;
}

global HCURSOR g_hand_cursor;
global HCURSOR g_arrow_cursor;
global HCURSOR g_slider_cursor;

PLATFORM_SET_CURSOR_TYPE(platform_SetCursorType) {
    switch (type) {
        case CursorType_Arrow:
        {
            SetCursor(g_arrow_cursor);
        } break;

        case CursorType_Hand:
        {

            SetCursor(g_hand_cursor);
        } break;


        case CursorType_Slider:
        {
            SetCursor(g_slider_cursor);
        } break;
    }
}