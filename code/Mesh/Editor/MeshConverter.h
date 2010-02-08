#ifndef traktor_mesh_MeshConverter_H
#define traktor_mesh_MeshConverter_H

#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Render/VertexElement.h"

namespace traktor
{

class IStream;

	namespace model
	{

class Model;

	}

	namespace mesh
	{

class MeshResource;

class MeshConverter : public Object
{
public:
	struct MaterialInfo
	{
		Guid guid;
		bool opaque;
	};

	virtual Ref< MeshResource > createResource() const = 0;

	virtual bool convert(
		const RefArray< model::Model >& models,
		const std::map< std::wstring, MaterialInfo >& materialInfo,
		const std::vector< render::VertexElement >& vertexElements,
		MeshResource* meshResource,
		IStream* meshResourceStream
	) const = 0;
};

	}
}

#endif	// traktor_mesh_MeshConverter_H
