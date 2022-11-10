#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/Editor/Glsl/GlslType.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Node;

/*!
 * \ingroup Render
 */
class T_DLLCLASS GlslVariable : public Object
{
public:
	GlslVariable() = default;

	explicit GlslVariable(const Node* node, const std::wstring& name, GlslType type);

	const Node* getNode() const { return m_node; }

	const std::wstring& getName() const { return m_name; }

	void setType(GlslType type) { m_type = type; }

	GlslType getType() const { return m_type; }

	std::wstring cast(GlslType to) const;

	GlslVariable& operator = (const GlslVariable& other);

private:
	const Node* m_node = nullptr;
	std::wstring m_name;
	GlslType m_type = GlslType::Void;
};

}
