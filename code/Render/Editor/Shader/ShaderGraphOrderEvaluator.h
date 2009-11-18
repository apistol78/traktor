#ifndef traktor_render_ShaderGraphOrderEvaluator_H
#define traktor_render_ShaderGraphOrderEvaluator_H

#include <map>
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

/*! \brief Evaluate algorithmic order of a shader graph node. */
class T_DLLCLASS ShaderGraphOrderEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	enum Order
	{
		OrConstant = 0,		/*!< Constant order. */
		OrLinear = 1,		/*!< Linear order. */
		OrNonLinear = 2		/*!< Non-linear order, i.e. cubic or higher. */
	};

	ShaderGraphOrderEvaluator(const ShaderGraph* shaderGraph, const ShaderGraphAdjacency* shaderGraphAdj);

	int evaluate(const Node* node, const std::wstring& inputPinName);

	int evaluate(const Node* node);

private:
	Ref< const ShaderGraph > m_shaderGraph;
	Ref< const ShaderGraphAdjacency > m_shaderGraphAdj;
	std::map< const Node*, int > m_evaluated;

	int nodeDefault(const Node* node, int initialOrder);

	int nodeConstantOrNonLinear(const Node* node);

	int nodeMulOrDiv(const Node* node);

	int nodeMulAdd(const Node* node);

	int nodeTrig(const Node* node);

	int nodeIndexedUniform(const Node* node);

	int nodeMatrix(const Node* node);

	int nodeLerp(const Node* node);
};

	}
}

#endif	// traktor_render_ShaderGraphOrderEvaluator_H
