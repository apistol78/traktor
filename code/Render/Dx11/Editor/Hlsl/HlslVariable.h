#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/Dx11/Editor/Hlsl/HlslType.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX11
 */
class HlslVariable : public Object
{
public:
	HlslVariable();

	HlslVariable(const std::wstring& name, HlslType type);

	const std::wstring& getName() const { return m_name; }

	void setType(HlslType type) { m_type = type; }

	HlslType getType() const { return m_type; }

	std::wstring cast(HlslType to) const;

private:
	std::wstring m_name;
	HlslType m_type;
};

	}
}

