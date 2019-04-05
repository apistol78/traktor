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
	GlslVariable();

	GlslVariable(const Node* node, const std::wstring& name, GlslType type);

	const Node* getNode() const { return m_node; }

	const std::wstring& getName() const { return m_name; }

	GlslType getType() const { return m_type; }

	std::wstring cast(GlslType to) const;

	GlslVariable& operator = (const GlslVariable& other);

private:
	const Node* m_node;
	std::wstring m_name;
	GlslType m_type;
};

	}
}

