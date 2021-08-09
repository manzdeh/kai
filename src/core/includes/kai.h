/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ENGINE_H
#define KAI_ENGINE_H

#include "alloc.h"
#include "input.h"
#include "types.h"
#include "utils.h"

#ifdef KAI_PLATFORM_WIN32
#define KAI_API __declspec(dllexport)
#endif

// All of the required callbacks have the same signature
#define KAI_GAME_CALLBACK_FUNC(name) void (name)(void)

#define KAI_GAME_INIT_PROCNAME      kai_game_init
#define KAI_GAME_UPDATE_PROCNAME    kai_game_update
#define KAI_GAME_DESTROY_PROCNAME   kai_game_destroy

#define KAI_GAME_INIT       extern "C" KAI_API KAI_GAME_CALLBACK_FUNC(KAI_GAME_INIT_PROCNAME)
#define KAI_GAME_UPDATE     extern "C" KAI_API KAI_GAME_CALLBACK_FUNC(KAI_GAME_UPDATE_PROCNAME)
#define KAI_GAME_DESTROY    extern "C" KAI_API KAI_GAME_CALLBACK_FUNC(KAI_GAME_DESTROY_PROCNAME)

typedef KAI_GAME_CALLBACK_FUNC(*KaiGameInitProc);
typedef KAI_GAME_CALLBACK_FUNC(*KaiGameUpdateProc);
typedef KAI_GAME_CALLBACK_FUNC(*KaiGameDestroyProc);

namespace kai {
    struct GameCallbacks {
        KaiGameInitProc init;
        KaiGameUpdateProc update;
        KaiGameDestroyProc destroy;
    };
}

#endif /* KAI_ENGINE_H */
