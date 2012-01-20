#ifndef traktor_render_ShaderGraphTypePropagation_H
#define traktor_render_ShaderGraphTypePropagation_H

#include <map>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/Editor/Shader/PinType.h"

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

class InputPin;
class Node;
class OutputPin;
class ShaderGraph;

/*! \brief Shader graph pin type propagation.
 * \ingroup Render
 *
 * Pin types are determined through propagating
 * types "backward" through the shader graph thus
 * being able to determine least required type
 * in order satisfy result.
 */
class T_DLLCLASS ShaderGraphTypePropagation : public Object
{
	T_RTTI_CLASS;

public:
	ShaderGraphTypePropagation(const ShaderGraph* shaderGraph);

	PinType evaluate(const InputPin* inputPin) const;

	PinType evaluate(const OutputPin* outputPin) const;

	void set(const InputPin* inputPin, PinType inputPinType);

	void set(const OutputPin* outputPin, PinType outputPinType);

private:
	Ref< const ShaderGraph > m_shaderGraph;
	std::map< const InputPin*, PinType > m_inputPinTypes;
	std::map< const OutputPin*, PinType > m_outputPinTypes;
};

	}
}

#endif	// traktor_render_ShaderGraphTypePropagation_H
