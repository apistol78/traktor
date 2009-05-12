#ifndef traktor_render_MeshReader_H
#define traktor_render_MeshReader_H

#include "Core/Heap/Ref.h"
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

class MeshFactory;
class Mesh;

/*! \brief Render mesh reader.
 * \ingroup Render
 */
class T_DLLCLASS MeshReader : public Object
{
	T_RTTI_CLASS(MeshReader)

public:
	MeshReader(MeshFactory* meshFactory);

	Mesh* read(Stream* stream) const;

private:
	Ref< MeshFactory > m_meshFactory;
};

	}
}

#endif	// traktor_render_MeshReader_H
