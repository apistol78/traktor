#ifndef traktor_render_CgVariable_H
#define traktor_render_CgVariable_H

#include <string>
#include "Core/Object.h"
#include "Render/Ps3/CgType.h"

namespace traktor
{
	namespace render
	{

class CgVariable
{
public:
	CgVariable(const std::wstring& name, CgType type);

	const std::wstring& getName() const;

	CgType getType() const;

	std::wstring cast(CgType to) const;

private:
	std::wstring m_name;
	CgType m_type;
};

	}
}

#endif	// traktor_render_CgVariable_H
