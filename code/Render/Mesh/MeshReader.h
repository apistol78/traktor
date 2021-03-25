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
class MeshFactory;

/*! Render mesh reader.
 * \ingroup Render
 */
class T_DLLCLASS MeshReader : public Object
{
	T_RTTI_CLASS;

public:
	explicit MeshReader(const MeshFactory* meshFactory);

	Ref< Mesh > read(IStream* stream) const;

private:
	Ref< const MeshFactory > m_meshFactory;
};

	}
}

