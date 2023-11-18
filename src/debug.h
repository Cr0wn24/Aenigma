#ifndef DEBUG_H
#define DEBUG_H

#if AENIGMA_INTERNAL

#define BEGIN_TIMED_FUNCTION() \
u64 start_cycle_count = __rdtsc(); \
local_persist u32 id##__LINE__ = __COUNTER__; \
_BeginTimedBlock(id##__LINE__, __FILE__, __LINE__, __FUNCTION__)

#define END_TIMED_FUNCTION() \
g_counters[id##__LINE__].cycle_count += __rdtsc() - start_cycle_count; \
g_counters[id##__LINE__].call_count++; 

typedef struct DebugCycleCounter {
    u64 cycle_count;
    u64 prev_cycle_count;

    u64 call_count;

    char *file_name;
    char *function_name;
    u32 line;
} DebugCycleCounter;

DebugCycleCounter g_counters[256];

void _BeginTimedBlock(u32 id, char *file_name, u32 line, char *function_name) {
    g_counters[id].function_name = function_name;
    g_counters[id].line = line;
    g_counters[id].file_name = file_name;
}

#if COMPILER_MSVC

#define _BEGIN_TIMED_BLOCK(id) u64 start_cycle_count##id = __rdtsc();
#define _END_TIMED_BLOCK(id) \
g_memory->counter[DebugCycleCounter_##id].cycle_count += __rdtsc() - start_cycle_count##id; \
g_memory->counter[DebugCycleCounter_##id].call_count++;

#else

#define BEGIN_TIMED_BLOCK(id)
#define END_TIMED_BLOCK.(id)

#endif

#endif

#endif