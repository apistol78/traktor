#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/Ps3/Editor/Cg/CgType.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup PS3
 */
class CgVariable : public Object
{
	T_RTTI_CLASS;

public:
	CgVariable();

	CgVariable(const std::wstring& name, CgType type);

	inline const std::wstring& getName() const { return m_name; }

	inline void setType(CgType type) { m_type = type; }

	inline CgType getType() const { return m_type; }

	std::wstring cast(CgType to) const;

private:
	std::wstring m_name;
	CgType m_type;
};

	}
}

