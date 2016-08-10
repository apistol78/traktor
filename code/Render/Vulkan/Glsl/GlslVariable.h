#ifndef traktor_render_GlslVariable_H
#define traktor_render_GlslVariable_H

#include <string>
#include "Core/Object.h"
#include "Render/Vulkan/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
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

#endif	// traktor_render_GlslVariable_H
