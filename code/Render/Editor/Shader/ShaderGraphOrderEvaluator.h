#ifndef traktor_render_ShaderGraphOrderEvaluator_H
#define traktor_render_ShaderGraphOrderEvaluator_H

#include <map>
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
class Node;

/*! \brief Evaluate algorithmic order of a shader graph node. */
class T_DLLCLASS ShaderGraphOrderEvaluator : public Object
{
	T_RTTI_CLASS(ShaderGraphOrderEvaluator)

public:
	enum Order
	{
		OrConstant = 0,		/*!< Constant order. */
		OrLinear = 1,		/*!< Linear order. */
		OrNonLinear = 2		/*!< Non-linear order, i.e. cubic or higher. */
	};

	ShaderGraphOrderEvaluator(const ShaderGraph* shaderGraph);

	int evaluate(const Node* node, const std::wstring& inputPinName);

	int evaluate(const Node* node);

private:
	Ref< const ShaderGraph > m_shaderGraph;
	std::map< const Node*, int > m_evaluated;

	int nodeDefault(const Node* node, int initialOrder);

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
