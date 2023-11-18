u32 GetFileSize(FILE *file)
{
    s32 result = 0;
    fseek(file, 0L, SEEK_END);
    result = ftell(file);
    fseek(file, 0L, SEEK_SET);

    return result;
}

PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file)
{
    Read_File_Result result = { 0 };

    FILE *file = fopen(file_name, "r");

    if(!file)
    {
        return result;
    }

    u32 file_size = GetFileSize(file);

    assert(file_size < 0xffffffff);

    result.contents = malloc(file_size);
    result.content_size = file_size;

    size_t bytes_read = fread(result.contents, file_size, 1, file);
    fclose(file);

    return result;
}

PLATFORM_ALLOC_MEMORY(platform_alloc_memory)
{
    void *result = 0;
    result = malloc(size);
    return result;
}

PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory)
{
    if(memory)
    {
        free(memory);
    }
}

PLATFORM_TOGGLE_CURSOR(PlatformToggleCursor)
{
}

PLATFORM_FIND_FILES_IN_DIRECTORY(platform_find_files_in_directory)
{

}