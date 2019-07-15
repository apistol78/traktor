#pragma once

#include "Render/Types.h"
#include "Render/OpenGL/Std/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \ingroup OGL */
//@{

class T_DLLCLASS VertexAttribute
{;
public:
	static std::wstring getName(DataUsage usage, int32_t index);

	static int32_t getLocation(DataUsage usage, int32_t index);
};

//@}

	}
}

