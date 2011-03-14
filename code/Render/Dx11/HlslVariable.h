#ifndef traktor_render_HlslVariable_H
#define traktor_render_HlslVariable_H

#include <string>
#include "Core/Object.h"
#include "Render/Dx11/HlslType.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX11
 */
class HlslVariable
{
public:
	HlslVariable();

	HlslVariable(const std::wstring& name, HlslType type);

	inline const std::wstring& getName() const { return m_name; }

	inline void setType(HlslType type) { m_type = type; }

	inline HlslType getType() const { return m_type; }

	std::wstring cast(HlslType to) const;

private:
	std::wstring m_name;
	HlslType m_type;
};

	}
}

#endif	// traktor_render_HlslVariable_H
