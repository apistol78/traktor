#ifndef traktor_mesh_IMeshConverter_H
#define traktor_mesh_IMeshConverter_H

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

class IMeshResource;

class IMeshConverter : public Object
{
public:
	struct MaterialInfo
	{
		Guid guid;
		bool opaque;
	};

	virtual Ref< IMeshResource > createResource() const = 0;

	virtual bool convert(
		const RefArray< model::Model >& models,
		const std::map< std::wstring, MaterialInfo >& materialInfo,
		const std::vector< render::VertexElement >& vertexElements,
		IMeshResource* meshResource,
		IStream* meshResourceStream
	) const = 0;
};

	}
}

#endif	// traktor_mesh_IMeshConverter_H
