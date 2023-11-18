#ifndef CONFIG_H
#define CONFIG_H

global u32 current_res_index = 5;
const v2s game_res[] ={
    {256, 144}, // 0
    {320, 180}, // 1
    {426, 240}, // 2
    {640, 360}, // 3
    {854, 480}, // 4
    {960, 540}, // 5
    {1280, 720}, // 6
    {1366, 768}, // 7
    {1600, 900}, // 8
    {1920, 1080}, // 9
    {2048, 1152}, // 10
    {2560, 1440}, // 11
    // {3200, 1800}, // 12
    // {3840, 2160}, // 13
    // {5120, 2880}, // 14
    // {7680, 4320}, // 15
};

typedef struct Config {
    s32 game_res_width;
    s32 game_res_height;
} Config;

global Config config ={
    .game_res_width = 960*2,
    .game_res_height = 540*2,
};

#define SCALE (config.game_res_width/960.0f)

#define PIXELS_PER_METER (64.0f*SCALE)

#endif