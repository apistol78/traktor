/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

#include <string>
#include <vector>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MCP_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;
class OutputStream;

}

namespace traktor::mcp
{

/*! JSON value.
 * \ingroup MCP
 *
 * Lightweight JSON document model used to build and serialize MCP
 * (JSON-RPC 2.0) messages.
 *
 * \note
 * Traktor's json module (rapidjson backed) is used for \em parsing. This type
 * is retained over the json module's own DOM purely for ergonomics: a typed
 * builder API (set/push/typed getters) and a compact writer tailored for wire
 * messages. The json module's writer is itself RFC 8259 compliant; it just
 * pretty-prints, which is unnecessary for protocol responses.
 */
class T_DLLCLASS Json : public Object
{
	T_RTTI_CLASS;

public:
	enum class Type
	{
		Null,
		Boolean,
		Number,
		Real,
		String,
		Array,
		Object
	};

	Json() = default;

	virtual ~Json();

	/*! Parse a single top-level JSON value from a UTF-8 stream.
	 *
	 * \return Parsed value, or null if the stream did not contain a value.
	 */
	static Ref< Json > parse(IStream* stream);

	/*! Parse a single top-level JSON value from a string. */
	static Ref< Json > parse(const std::wstring& text);

	static Ref< Json > createNull();

	static Ref< Json > createBoolean(bool value);

	static Ref< Json > createNumber(int64_t value);

	static Ref< Json > createReal(double value);

	static Ref< Json > createString(const std::wstring& value);

	static Ref< Json > createArray();

	static Ref< Json > createObject();

	Type getType() const { return m_type; }

	bool isNull() const { return m_type == Type::Null; }

	bool isBoolean() const { return m_type == Type::Boolean; }

	bool isNumber() const { return m_type == Type::Number || m_type == Type::Real; }

	bool isString() const { return m_type == Type::String; }

	bool isArray() const { return m_type == Type::Array; }

	bool isObject() const { return m_type == Type::Object; }

	bool getBoolean(bool defaultValue = false) const;

	int64_t getNumber(int64_t defaultValue = 0) const;

	double getReal(double defaultValue = 0.0) const;

	std::wstring getString(const std::wstring& defaultValue = L"") const;

	// \name Object accessors.
	// \{

	Json* set(const std::wstring& key, Json* value);

	Json* setBoolean(const std::wstring& key, bool value);

	Json* setNumber(const std::wstring& key, int64_t value);

	Json* setString(const std::wstring& key, const std::wstring& value);

	/*! Get object member by name; null if no such member. */
	Json* getMember(const std::wstring& key) const;

	/*! Get number of object members. */
	uint32_t getMemberCount() const;

	/*! Get object member name by index. */
	const std::wstring& getMemberName(uint32_t index) const;

	/*! Get object member value by index. */
	Json* getMemberValue(uint32_t index) const;

	// \}

	// \name Array accessors.
	// \{

	Json* push(Json* value);

	uint32_t size() const;

	Json* at(uint32_t index) const;

	// \}

	/*! Serialize value as compact, RFC 8259 compliant JSON. */
	void write(OutputStream& os) const;

	/*! Serialize value to a string. */
	std::wstring toString() const;

private:
	Type m_type = Type::Null;
	bool m_boolean = false;
	int64_t m_number = 0;
	double m_real = 0.0;
	std::wstring m_string;
	RefArray< Json > m_array;
	std::vector< std::wstring > m_memberNames;
	RefArray< Json > m_memberValues;
};

}
