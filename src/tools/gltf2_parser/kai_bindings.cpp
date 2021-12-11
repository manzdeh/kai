#include "../../core/includes/utils.h"
#include <stdio.h>

// This needs to be exported so it can be used from the python script
extern "C" KAI_API Uint64 kai_fnv1a64_str_hash(const char *str) {
    return kai::fnv1a64_str_hash(str);
}
