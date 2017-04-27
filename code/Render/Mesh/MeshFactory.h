/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_MeshFactory_H
#define traktor_render_MeshFactory_H

#include <vector>
#include "Core/Object.h"
#include "Render/VertexElement.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Mesh;

/*! \brief Render mesh factory.
 * \ingroup Render
 */
class T_DLLCLASS MeshFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Mesh > createMesh(
		const std::vector< VertexElement >& vertexElements,
		unsigned int vertexBufferSize,
		IndexType indexType,
		unsigned int indexBufferSize
	) = 0;
};

	}
}

#endif	// traktor_render_MeshFactory_H
