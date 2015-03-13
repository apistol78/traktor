#ifndef traktor_render_ShaderGraphEvaluator_H
#define traktor_render_ShaderGraphEvaluator_H

#include "Core/Object.h"
#include "Render/Editor/Shader/Constant.h"

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
class OutputPin;

/*! \brief Evaluate constant value of shader graph pins.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	ShaderGraphEvaluator(const ShaderGraph* shaderGraph);

	Constant evaluate(const OutputPin* outputPin) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
};

	}
}

#endif	// traktor_render_ShaderGraphEvaluator_H
