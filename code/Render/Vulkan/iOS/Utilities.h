#pragma once

#include "Core/Config.h"

namespace traktor
{
    namespace render
    {

int32_t getScreenWidth();

int32_t getScreenHeight();

int32_t getViewWidth(void* view);

int32_t getViewHeight(void* view);

    }
}