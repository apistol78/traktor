#ifndef traktor_mesh_IMeshConverter_H
#define traktor_mesh_IMeshConverter_H

#include <list>
#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Mesh/Editor/MeshPipelineTypes.h"
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
class MeshAsset;

class IMeshConverter : public Object
{
public:
	virtual Ref< IMeshResource > createResource() const = 0;

	virtual bool convert(
		const MeshAsset* meshAsset,
		const RefArray< model::Model >& models,
		const Guid& materialGuid,
		const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
		const std::vector< render::VertexElement >& vertexElements,
		IMeshResource* meshResource,
		IStream* meshResourceStream
	) const = 0;
};

	}
}

#endif	// traktor_mesh_IMeshConverter_H
