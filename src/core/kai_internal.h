/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ENGINE_INTERNAL_H
#define KAI_ENGINE_INTERNAL_H

#include "includes/kai.h"

void init_engine(void);
bool tick_engine(void);
void destroy_engine(void);

void set_log_callback(KaiLogProc func);

kai::StackAllocator * get_engine_memory(void);

#endif /* KAI_ENGINE_INTERNAL_H */
