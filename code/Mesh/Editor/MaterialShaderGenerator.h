#pragma once

#include <functional>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IPipelineDepends;

	}

	namespace render
	{

class ShaderGraph;

	}

	namespace model
	{

class Material;
class Model;

	}

	namespace mesh
	{

class T_DLLCLASS MaterialShaderGenerator : public Object
{
	T_RTTI_CLASS;

public:
	Ref< render::ShaderGraph > generate(
		const std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) >& resolve,
		const model::Model& model,
		const model::Material& material,
		const Guid& materialTemplate,
		bool vertexColor
	) const;

	void addDependencies(editor::IPipelineDepends* pipelineDepends);
};

	}
}

