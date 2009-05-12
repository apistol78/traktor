#ifndef traktor_render_OutputPin_H
#define traktor_render_OutputPin_H

#include <string>
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

class Node;

/*! \brief Shader graph node output pin.
 * \ingroup Render
 */
class T_DLLCLASS OutputPin : public Object
{
	T_RTTI_CLASS(OutputPin)

public:
	OutputPin();

	OutputPin(Node* node, const std::wstring& name);
	
	Node* getNode() const;
	
	const std::wstring& getName() const;

private:
	Ref< Node > m_node;
	std::wstring m_name;
};

	}
}

#endif	// traktor_render_OutputPin_H
