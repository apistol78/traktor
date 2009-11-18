#ifndef traktor_render_ShaderGraph_H
#define traktor_render_ShaderGraph_H

#include <list>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Node;
class Edge;
class OutputPin;
class InputPin;

/*! \brief Shader graph.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraph : public ISerializable
{
	T_RTTI_CLASS;

public:
	ShaderGraph();

	ShaderGraph(const RefArray< Node >& nodes, const RefArray< Edge >& edges);

	/*! \brief Add node to graph.
	 *
	 * \param node Node to add.
	 */
	void addNode(Node* node);

	/*! \brief Remove node from graph.
	 *
	 * \param node Node to remove.
	 */
	void removeNode(Node* node);

	/*! \brief Add edge to graph.
	 *
	 * \param edge Edge to add.
	 */
	void addEdge(Edge* edge);

	/*! \brief Remove edge from graph.
	 *
	 * \param edge Edge to remove.
	 */
	void removeEdge(Edge* edge);

	/*! \brief Remove all nodes and edges from graph. */
	void removeAll();

	/*! \brief Get all nodes of a specific type.
	 *
	 * \param nodeType Type of node to find.
	 * \param outNodes Nodes of given type.
	 * \return Number of nodes.
	 */
	size_t findNodesOf(const TypeInfo& nodeType, RefArray< Node >& outNodes) const;

	/*! \brief Get all nodes of a specific type.
	 *
	 * \param nodeType Type of node to find.
	 * \param outNodes Nodes of given type.
	 * \return Number of nodes.
	 */
	template < typename NodeType >
	size_t findNodesOf(RefArray< NodeType >& outNodes) const
	{
		RefArray< Node > untypedNodes;
		findNodesOf(type_of< NodeType >(), untypedNodes);
		
		outNodes.resize(untypedNodes.size());
		for (size_t i = 0; i < untypedNodes.size(); ++i)
			outNodes[i] = checked_type_cast< NodeType* >(untypedNodes[i]);

		return outNodes.size();
	}

	/*! \brief Serialize graph. */
	virtual bool serialize(ISerializer& s);

	/*! \brief Get all nodes.
	 *
	 * \return Array of nodes.
	 */
	inline const RefArray< Node >& getNodes() const { return m_nodes; }

	/*! \brief Get all edges.
	 *
	 * \return Array of edges.
	 */
	inline const RefArray< Edge >& getEdges() const { return m_edges; }

private:
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
};

	}
}

#endif	// traktor_render_ShaderGraph_H
