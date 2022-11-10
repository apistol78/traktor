#pragma once

#include <map>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

/*! Connection string parser.
 * \ingroup Database
 */
class T_DLLCLASS ConnectionString : public Object
{
	T_RTTI_CLASS;

public:
	ConnectionString() = default;

	ConnectionString(const std::wstring& connectionString);

	bool have(const std::wstring& key) const;

	void set(const std::wstring& key, const std::wstring& value);

	std::wstring get(const std::wstring& key) const;

	std::wstring format() const;

private:
	std::map< std::wstring, std::wstring > m_values;
};

}
