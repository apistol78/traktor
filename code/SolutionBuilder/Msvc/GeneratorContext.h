#pragma once

#include <string>
#include "Core/Containers/SmallMap.h"

namespace traktor
{
	namespace sb
	{

/*! Generator context. */
class GeneratorContext
{
public:
	explicit GeneratorContext(bool includeExternal);

	void set(const std::wstring& key, const std::wstring& value);

	std::wstring get(const std::wstring& key) const;

	std::wstring format(const std::wstring& option) const;

	std::wstring getProjectRelativePath(const std::wstring& path, bool resolve) const;

	std::wstring generateGUID(const std::wstring& key) const;

	bool getIncludeExternal() const { return m_includeExternal; }

private:
	bool m_includeExternal;
	SmallMap< std::wstring, std::wstring > m_values;
};

	}
}

