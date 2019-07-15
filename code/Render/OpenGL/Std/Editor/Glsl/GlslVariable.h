#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{

class Node;

/*!
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

	std::wstring castToInteger(GlslType to) const;

	GlslVariable& operator = (const GlslVariable& other);

private:
	const Node* m_node;
	std::wstring m_name;
	GlslType m_type;
};

	}
}

