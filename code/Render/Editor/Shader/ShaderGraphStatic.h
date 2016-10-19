#ifndef traktor_render_ShaderGraphStatic_H
#define traktor_render_ShaderGraphStatic_H

#include "Core/Object.h"
#include "Core/Ref.h"

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

/*! \brief Static analysis on shader graphs.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphStatic : public Object
{
	T_RTTI_CLASS;

public:
	ShaderGraphStatic(const ShaderGraph* shaderGraph);

	Ref< ShaderGraph > getPlatformPermutation(const std::wstring& platform) const;

	Ref< ShaderGraph > getConnectedPermutation() const;

	Ref< ShaderGraph > getTypePermutation() const;

	Ref< ShaderGraph > getSwizzledPermutation() const;

	Ref< ShaderGraph > getConstantFolded() const;

	Ref< ShaderGraph > cleanupRedundantSwizzles() const;

	Ref< ShaderGraph > getStateResolved() const;

	Ref< ShaderGraph > getVariableResolved() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	Ref< const ShaderGraph > m_shaderGraphClean;
};

	}
}

#endif	// traktor_render_ShaderGraphStatic_H
