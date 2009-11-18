#ifndef traktor_render_ShaderGraphValidator_H
#define traktor_render_ShaderGraphValidator_H

#include <vector>
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
class Node;

class T_DLLCLASS ShaderGraphValidator : public Object
{
	T_RTTI_CLASS;

public:
	enum ShaderGraphType
	{
		SgtShader,
		SgtProgram,
		SgtFragment
	};

	ShaderGraphValidator(const ShaderGraph* shaderGraph);

	bool validate(ShaderGraphType type, std::vector< const Node* >* outErrorNodes = 0) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
};

	}
}

#endif	// traktor_render_ShaderGraphValidator_H
