#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

function void
StringCat(char *source0, size_t source0_count, char *source1, size_t source1_count, char *dest, size_t dest_count)
{
    for(s32 index = 0; index < source0_count; ++index)
    {
        *dest++ = *source0++;
    }
    for(s32 index = 0; index < source1_count; ++index)
    {
        *dest++ = *source1++;
    }

    *dest++ = 0;
}

function inline u32
StringLength(char *string)
{
    u32 length = 0;

    if(string)
    {
        while(*string++) ++length;
    }

    return length;
}

function void
StringFormat(char *dest, u32 dest_size, char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(dest, dest_size, format, args);
    va_end(args);
}

function void
StringAppend(char *dest, u32 dest_size, char *src)
{
    u32 used_length = StringLength(dest);
    u32 src_length = StringLength(src);
    for(u32 i = 0; i < src_length; ++i)
    {
        dest[used_length++] = *src++;
    }
    dest[used_length] = 0;
}

function b32
StringEmpty(char *string)
{
    b32 result;

    result = StringLength(string) == 0;

    return result;
}

// returns true when same, else false.
function b32
StringCompare(char *string0, char *string1)
{
    b32 result = true;

    s32 length0 = StringLength(string0);
    s32 length1 = StringLength(string1);

    if(length0 != length1)
    {
        result = false;
        goto end;
    }

    for(s32 i = 0; i < length0; ++i)
    {
        if(string0[i] != string1[i])
        {
            result = false;
            goto end;
        }
    }

end:
    return result;
}

function void
StringCopy(char *dest, u32 dest_size, char *src)
{
    u32 dest_length = StringLength(dest);
    u32 src_length = StringLength(src);
    Assert((dest_length + src_length) < dest_size);
    for(u32 i = 0; i < src_length; ++i)
    {
        *dest++ = *src++;
    }
    *dest = 0;
}