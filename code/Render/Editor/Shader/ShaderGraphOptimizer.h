#ifndef traktor_render_ShaderGraphOptimizer_H
#define traktor_render_ShaderGraphOptimizer_H

#include <set>
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
class ShaderGraphAdjacency;
class Node;

/*! \brief Shader graph optimizer.
 *
 * \note The shader graph optimizer only works on
 *       fully resolved shader graphs, ie. no fragments.
 */
class T_DLLCLASS ShaderGraphOptimizer : public Object
{
	T_RTTI_CLASS(ShaderGraphOptimizer)

public:
	ShaderGraphOptimizer(const ShaderGraph* shaderGraph);

	/*! \brief Remove unused branches.
	 *
	 * \return Shader graph with removed unused branches.
	 */
	Ref< ShaderGraph > removeUnusedBranches();

	/*! \brief Merge duplicated branches.
	 *
	 * \return Shader graph with removed duplicated branches.
	 */
	Ref< ShaderGraph > mergeBranches();

	/*! \brief Insert interpolator nodes where appropriate.
	 *
	 * Using a order analysis to determine where to
	 * best insert interpolators into the graph
	 * in order to split calculations into vertex and pixel
	 * shaders.
	 *
	 * \param shaderGraph Source shader graph.
	 * \return Shader graph with inserted interpolators.
	 */
	Ref< ShaderGraph > insertInterpolators();

private:
	Ref< ShaderGraph > m_shaderGraph;
	Ref< ShaderGraphAdjacency > m_shaderGraphAdj;
	std::set< const Node* > m_visited;
	std::map< const Node*, int > m_orderComplexity;
	int32_t m_insertedCount;

	void insertInterpolators(Node* node);

	void updateOrderComplexity();
};

	}
}

#endif	// traktor_render_ShaderGraphOptimizer_H
