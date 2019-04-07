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
	int32_t getBinding() const { return m_binding; }

	const std::wstring& getName() const { return m_name; }

protected:
	GlslResource(const std::wstring& name);

private:
	friend class GlslLayout;

	int32_t m_binding;
	std::wstring m_name;
};

	}
}
