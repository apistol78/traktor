#pragma once

#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VULKAN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS VertexAttributes
{;
public:
	static std::wstring getName(DataUsage usage, int32_t index);

	static int32_t getLocation(DataUsage usage, int32_t index);
};

}
