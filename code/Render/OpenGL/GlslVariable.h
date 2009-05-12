#ifndef traktor_render_GlslVariable_H
#define traktor_render_GlslVariable_H

#include <string>
#include "Core/Object.h"
#include "Render/OpenGL/GlslType.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class GlslVariable
{
public:
	GlslVariable();

	GlslVariable(const std::wstring& name, GlslType type);

	inline const std::wstring& getName() const { return m_name; }

	inline void setType(GlslType type) { m_type = type; }

	inline GlslType getType() const { return m_type; }

	std::wstring cast(GlslType to) const;

private:
	std::wstring m_name;
	GlslType m_type;
};

	}
}

#endif	// traktor_render_GlslVariable_H
