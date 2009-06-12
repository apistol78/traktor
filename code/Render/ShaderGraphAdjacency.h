#ifndef traktor_render_ShaderGraphAdjacency_H
#define traktor_render_ShaderGraphAdjacency_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

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

class ShaderGraph;
class Node;
class Edge;
class OutputPin;
class InputPin;

/*! \brief Shader graph adjacency.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphAdjacency : public Object
{
	T_RTTI_CLASS(ShaderGraphAdjacency)

public:
	ShaderGraphAdjacency(const ShaderGraph* shaderGraph);

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
	size_t findDestinationPins(const OutputPin* outputPin, std::vector< const InputPin* >& outDestinations) const;
	
private:
	Ref< const ShaderGraph > m_shaderGraph;
	std::map< const InputPin*, Ref< Edge > > m_inputPinEdge;
	std::map< const OutputPin*, RefArray< Edge > > m_outputPinEdges;
};

	}
}

#endif	// traktor_render_ShaderGraphAdjacency_H
