#ifndef traktor_render_MeshWriter_H
#define traktor_render_MeshWriter_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Stream;

	namespace render
	{

class Mesh;

/*! \brief Render mesh writer.
 * \ingroup Render
 */
class T_DLLCLASS MeshWriter : public Object
{
	T_RTTI_CLASS(MeshWriter)

public:
	bool write(Stream* stream, const Mesh* mesh) const;
};

	}
}

#endif	// traktor_render_MeshWriter_H
