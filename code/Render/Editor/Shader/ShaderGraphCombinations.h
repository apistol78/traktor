#ifndef traktor_render_ShaderGraphCombinations_H
#define traktor_render_ShaderGraphCombinations_H

#include <vector>
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

/*! \brief Shader graph combination generator. */
class T_DLLCLASS ShaderGraphCombinations : public Object
{
	T_RTTI_CLASS(ShaderGraphCombinations)

public:
	ShaderGraphCombinations(const ShaderGraph* shaderGraph);

	const std::vector< std::wstring >& getParameterNames() const;

	uint32_t getCombinationCount() const;

	std::vector< std::wstring > getParameterCombination(uint32_t combination) const;

	ShaderGraph* generate(uint32_t combination) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	std::vector< std::wstring > m_parameterNames;
};

	}
}

#endif	// traktor_render_ShaderGraphCombinations_H
