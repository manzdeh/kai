/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "includes/kai.h"
#include "kai_internal.h"

#include "alloc.cpp"

void init_engine(void) {
    MemoryManager::init(kai::gibibytes(4));
}

void destroy_engine(void) {
    MemoryManager::destroy();
}
