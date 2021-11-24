#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace render
	{

class Mesh;

/*! Render mesh writer.
 * \ingroup Render
 */
class T_DLLCLASS MeshWriter : public Object
{
	T_RTTI_CLASS;

public:
	bool write(IStream* stream, const Mesh* mesh) const;
};

	}
}

