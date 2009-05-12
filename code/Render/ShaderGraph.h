#ifndef traktor_render_ShaderGraph_H
#define traktor_render_ShaderGraph_H

#include <list>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Serialization/Serializable.h"

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
class T_DLLCLASS ShaderGraph : public Serializable
{
	T_RTTI_CLASS(ShaderGraph)

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

	/*! \brief Remove all nodes from graph. */
	void removeAllNodes();

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

	/*! \brief Remove all edges from graph. */
	void removeAllEdges();

	/*! \brief Find edge connected to input pin.
	 *
	 * \param inputPin Input pin.
	 * \return Connected edge.
	 */
	Edge* findEdge(const InputPin* inputPin) const;

	/*! \brief Find edges connected from output pin.
	 *
	 * \param outputPin Output pin.
	 * \param outEdges Connected edges.
	 * \return Number of connected edges.
	 */
	size_t findEdges(const OutputPin* outputPin, RefArray< Edge >& outEdges) const;

	/*! \brief Find output pin connected to input pin.
	 *
	 * \param inputPin Input pin.
	 * \return Connected output pin.
	 */
	const OutputPin* findSourcePin(const InputPin* inputPin) const;

	/*! \brief Find all input pins which are connected to output pin.
	 *
	 * \param outputPin Output pin.
	 * \param outDestinations Connected input pins.
	 * \return Number of connected input pins.
	 */
	size_t findDestinationPins(const OutputPin* outputPin, RefArray< const InputPin >& outDestinations) const;
	
	/*! \brief Get all nodes of a specific type.
	 *
	 * \param nodeType Type of node to find.
	 * \param outNodes Nodes of given type.
	 * \return Number of nodes.
	 */
	size_t findNodesOf(const Type& nodeType, RefArray< Node >& outNodes) const;

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
		for (RefArray< Node >::const_iterator i = untypedNodes.begin(); i != untypedNodes.end(); ++i)
			outNodes.push_back(checked_type_cast< NodeType* >(*i));
		return outNodes.size();
	}

	virtual bool serialize(Serializer& s);

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
