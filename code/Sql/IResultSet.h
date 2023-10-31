/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Misc/String.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SQL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sql
{

/*! SQL column types.
 * \ingroup SQL
 */
enum class Column
{
	Void,
	Int32,
	Int64,
	Float,
	Double,
	String
};

/*! SQL query/update result set.
 * \ingroup SQL
 */
class T_DLLCLASS IResultSet : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool next() = 0;

	virtual int32_t getColumnCount() const = 0;

	virtual std::wstring getColumnName(int32_t columnIndex) const = 0;

	virtual Column getColumnType(int32_t columnIndex) const = 0;

	virtual int32_t getInt32(int32_t columnIndex) const = 0;

	virtual int64_t getInt64(int32_t columnIndex) const = 0;

	virtual float getFloat(int32_t columnIndex) const = 0;

	virtual double getDouble(int32_t columnIndex) const = 0;

	virtual std::wstring getString(int32_t columnIndex) const = 0;

	/*! \group Access value by column name. */
	//@{

	int32_t findColumn(const std::wstring& columnName) const
	{
		const int32_t columnCount = getColumnCount();
		for (int32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex)
		{
			if (compareIgnoreCase(columnName, getColumnName(columnIndex)) == 0)
				return columnIndex;
		}
		return -1;
	}

	int32_t getInt32(const std::wstring& columnName) const
	{
		const int32_t columnIndex = findColumn(columnName);
		return columnIndex >= 0 ? getInt32(columnIndex) : 0;
	}

	int64_t getInt64(const std::wstring& columnName) const
	{
		const int32_t columnIndex = findColumn(columnName);
		return columnIndex >= 0 ? getInt64(columnIndex) : 0;
	}

	float getFloat(const std::wstring& columnName) const
	{
		const int32_t columnIndex = findColumn(columnName);
		return columnIndex >= 0 ? getFloat(columnIndex) : 0.0f;
	}

	double getDouble(const std::wstring& columnName) const
	{
		const int32_t columnIndex = findColumn(columnName);
		return columnIndex >= 0 ? getDouble(columnIndex) : 0.0;
	}

	std::wstring getString(const std::wstring& columnName) const
	{
		const int32_t columnIndex = findColumn(columnName);
		return columnIndex >= 0 ? getString(columnIndex) : L"";
	}

	//@}
};

}
