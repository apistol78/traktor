#ifndef traktor_render_ShaderGraphTechniques_H
#define traktor_render_ShaderGraphTechniques_H

#include <set>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class ShaderGraphAdjacency;
class Node;

/*! \brief Shader graph technique generator. */
class T_DLLCLASS ShaderGraphTechniques : public Object
{
	T_RTTI_CLASS(ShaderGraphTechniques)

public:
	ShaderGraphTechniques(const ShaderGraph* shaderGraph);

	std::set< std::wstring > getNames() const;

	ShaderGraph* generate(const std::wstring& name) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	Ref< ShaderGraphAdjacency > m_shaderGraphAdj;
};

	}
}

#endif	// traktor_render_ShaderGraphTechniques_H
