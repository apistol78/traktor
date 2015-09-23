#ifndef traktor_render_ShaderGraphTechniques_H
#define traktor_render_ShaderGraphTechniques_H

#include <map>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class Node;

/*! \brief Shader graph technique generator.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphTechniques : public Object
{
	T_RTTI_CLASS;

public:
	ShaderGraphTechniques(const ShaderGraph* shaderGraph);

	std::set< std::wstring > getNames() const;

	Ref< ShaderGraph > generate(const std::wstring& name) const;

private:
	std::map< std::wstring, Ref< const ShaderGraph > > m_techniques;
};

	}
}

#endif	// traktor_render_ShaderGraphTechniques_H
