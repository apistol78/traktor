#ifndef GeneratorContext_H
#define GeneratorContext_H

#include <map>
#include <string>

/*! \brief Generator context. */
class GeneratorContext
{
public:
	void set(const std::wstring& key, const std::wstring& value);

	std::wstring get(const std::wstring& key) const;

	std::wstring format(const std::wstring& option) const;

	std::wstring getProjectRelativePath(const std::wstring& path, bool resolve) const;

	std::wstring generateGUID(const std::wstring& key) const;

private:
	std::map< std::wstring, std::wstring > m_values;
};

#endif	// GeneratorContext_H
