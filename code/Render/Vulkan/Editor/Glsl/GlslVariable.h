#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/Vulkan/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{

class Node;

/*!
 * \ingroup Vulkan
 */
class GlslVariable : public Object
{
public:
	GlslVariable() = default;

	GlslVariable(const Node* node, const std::wstring& name, GlslType type);

	const Node* getNode() const { return m_node; }

	const std::wstring& getName() const { return m_name; }

	void setType(GlslType type) { m_type = type; }

	GlslType getType() const { return m_type; }

	std::wstring cast(GlslType to) const;

	std::wstring castToInteger(GlslType to) const;

	GlslVariable& operator = (const GlslVariable& other);

private:
	const Node* m_node = nullptr;
	std::wstring m_name;
	GlslType m_type = GtVoid;
};

	}
}

