#ifndef traktor_render_ShaderGraphOrderEvaluator_H
#define traktor_render_ShaderGraphOrderEvaluator_H

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

/*! \brief Evaluate algorithmic order of a shader graph node.
 * \ingroup Render
 */
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

	ShaderGraphOrderEvaluator(
		const ShaderGraph* shaderGraph,
		bool frequentUniformsAsLinear
	);

	int evaluate(const Node* node, const std::wstring& inputPinName) const;

	int evaluate(const Node* node) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	bool m_frequentUniformsAsLinear;
	mutable std::map< const Node*, int > m_evaluated;

	int nodeDefault(const Node* node, int initialOrder) const;

	int nodeConstantOrNonLinear(const Node* node) const;

	int nodeMulOrDiv(const Node* node) const;

	int nodeMulAdd(const Node* node) const;

	int nodeTrig(const Node* node) const;

	int nodeArcusTan(const Node* node) const;
	
	int nodeUniform(const Node* node) const;

	int nodeIndexedUniform(const Node* node) const;

	int nodeMatrix(const Node* node) const;

	int nodeLerp(const Node* node) const;
};

	}
}

#endif	// traktor_render_ShaderGraphOrderEvaluator_H
