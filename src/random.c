#include "random.h"

function inline u32
GetNextRandom() {
    u32 result;

    result = random_table[random_index++];

    if (random_index >= ArrayCount(random_table)) {
        random_index = 0;
    }

    return result;
}

function inline f32
GetRandomBetween(f32 min, f32 max) {
    f32 result;

    result = min + ((f32)GetNextRandom() / (f32)(RANDOM_MAX)) * (max - min);

    return result;
}