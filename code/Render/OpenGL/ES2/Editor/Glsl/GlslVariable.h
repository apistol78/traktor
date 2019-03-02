#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/OpenGL/ES2/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class GlslVariable : public Object
{
public:
	GlslVariable();

	GlslVariable(const std::wstring& name, GlslType type);

	const std::wstring& getName() const { return m_name; }

	void setType(GlslType type) { m_type = type; }

	GlslType getType() const { return m_type; }

	std::wstring cast(GlslType to) const;

private:
	std::wstring m_name;
	GlslType m_type;
};

	}
}

