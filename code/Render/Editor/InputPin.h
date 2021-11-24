#pragma once

#include <string>
#include "Core/Guid.h"

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

class Node;

/*! Graph node input pin.
 * \ingroup Render
 */
class T_DLLCLASS InputPin
{
public:
	InputPin() = default;

	explicit InputPin(Node* node, const Guid& id, const std::wstring& name, bool optional);

	Node* getNode() const;

	const Guid& getId() const;

	const std::wstring& getName() const;

	bool isOptional() const;

private:
	Node* m_node = nullptr;
	Guid m_id;
	std::wstring m_name;
	bool m_optional = false;
};

	}
}
