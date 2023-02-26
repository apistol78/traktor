/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Class/Any.h"
#include "Json/JsonNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JSON_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace json
	{

class JsonMember;

/*! JSON object
 * \ingroup JSON
 */
class T_DLLCLASS JsonObject : public JsonNode
{
	T_RTTI_CLASS;

public:
	/*! Push member onto object.
	 *
	 * \param member Member to push.
	 */
	void push(JsonMember* member);

	/*! Check if object doesn't contain any member.
	 *
	 * \return True if object has no members.
	 */
	bool empty() const { return m_members.empty(); }

	/*! Get first member.
	 *
	 * \return First member.
	 */
	JsonMember* front() { return m_members.front(); }

	/*! Get last member.
	 *
	 * \return Last member.
	 */
	JsonMember* back() { return m_members.back(); }

	/*! Get number of members.
	 *
	 * \return Number of members.
	 */
	uint32_t size() const { return uint32_t(m_members.size()); }

	/*! Get named member.
	 *
	 * \param name Member name.
	 * \return Member node.
	 */
	JsonMember* getMember(const std::wstring& name) const;

	/*! Set named member value.
	 * \note If no such member exist a new member is created.
	 *
	 * \param name Member name.
	 * \param value Value of member.
	 */
	void setMemberValue(const std::wstring& name, const Any& value);

	/*! Get value of named member.
	 * \note If no such member exist undefined is returned.
	 *
	 * \param name Member name.
	 * \return Value of member.
	 */
	Any getMemberValue(const std::wstring& name) const;

	/*! Get value of member path.
	 *
	 * \param path Member path.
	 * \return Value of member.
	 */
	Any getValue(const std::wstring& path) const;

	/*! Get member by index.
	 *
	 * \param index Member index.
	 * \return Member node.
	 */
	JsonMember* get(uint32_t index) { return m_members[index]; }

	/*! Get array of all members.
	 *
	 * \return All member nodes.
	 */
	const RefArray< JsonMember >& get() const { return m_members; }

	/*!
	 */
	virtual bool write(OutputStream& os) const override;

	/*!
	 */
	int32_t getMemberInt32(const std::wstring& name, int32_t defaultValue = 0) const
	{
		const Any value = getMemberValue(name);
		return value.isNumeric() ? value.getInt32() : defaultValue;
	}

	/*!
	 */
	float getMemberFloat(const std::wstring& name, float defaultValue = 0.0f) const
	{
		const Any value = getMemberValue(name);
		return value.isNumeric() ? value.getFloat() : defaultValue;
	}

	/*!
	 */
	std::wstring getMemberString(const std::wstring& name, const std::wstring& defaultValue = L"") const
	{
		const Any value = getMemberValue(name);
		return value.isString() ? value.getWideString() : defaultValue;
	}

private:
	RefArray< JsonMember > m_members;
};

	}
}

