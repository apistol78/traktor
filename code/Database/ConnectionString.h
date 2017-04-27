/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_ConnectionString_H
#define traktor_db_ConnectionString_H

#include <map>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Connection string parser.
 * \ingroup Database
 */
class T_DLLCLASS ConnectionString : public Object
{
	T_RTTI_CLASS;

public:
	ConnectionString();

	ConnectionString(const std::wstring& connectionString);

	bool have(const std::wstring& key) const;

	void set(const std::wstring& key, const std::wstring& value);

	std::wstring get(const std::wstring& key) const;

	std::wstring format() const;

private:
	std::map< std::wstring, std::wstring > m_values;
};

	}
}

#endif	// traktor_db_ConnectionString_H
