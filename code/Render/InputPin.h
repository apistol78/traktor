#ifndef traktor_render_InputPin_H
#define traktor_render_InputPin_H

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

/*! \brief Shader graph node input pin.
 * \ingroup Render
 */
class T_DLLCLASS InputPin : public Object
{
	T_RTTI_CLASS(InputPin)

public:
	InputPin(Node* node, const std::wstring& name, bool optional);
	
	Ref< Node > getNode() const;
	
	const std::wstring& getName() const;

	bool isOptional() const;

private:
	Node* m_node;
	std::wstring m_name;
	bool m_optional;
};

	}
}

#endif	// traktor_render_InputPin_H
