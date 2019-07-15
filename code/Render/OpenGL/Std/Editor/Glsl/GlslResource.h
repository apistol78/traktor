#pragma once

#include <string>
#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class GlslResource : public Object
{
	T_RTTI_CLASS;

public:
	const std::wstring& getName() const { return m_name; }

protected:
	GlslResource(const std::wstring& name);

private:
	friend class GlslLayout;

	std::wstring m_name;
};

	}
}
