/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Script/Editor/IScriptOutline.h"

namespace traktor::script
{

/*! Variable declaration node.
 * \ingroup Script
 */
class VariableNode : public IScriptOutline::Node
{
	T_RTTI_CLASS;

public:
	VariableNode(int32_t line, const std::wstring& name, bool isLocal);

	const std::wstring& getName() const;

	bool isLocal() const;

private:
	std::wstring m_name;
	bool m_local;
};

/*! Function parameter node.
 * \ingroup Script
 */
class ParameterNode : public IScriptOutline::Node
{
	T_RTTI_CLASS;

public:
	ParameterNode(int32_t line, const std::wstring& name);

	const std::wstring& getName() const;

private:
	std::wstring m_name;
};

/*! Table definition node.
 * \ingroup Script
 */
class TableNode : public IScriptOutline::Node
{
	T_RTTI_CLASS;

public:
	TableNode(int32_t line, const std::wstring& name);

	const std::wstring& getName() const;

	void addField(const std::wstring& fieldName);

	const std::vector< std::wstring >& getFields() const;

private:
	std::wstring m_name;
	std::vector< std::wstring > m_fields;
};

/*! Field access node (e.g., table.field).
 * \ingroup Script
 */
class FieldAccessNode : public IScriptOutline::Node
{
	T_RTTI_CLASS;

public:
	FieldAccessNode(int32_t line, const std::wstring& tableName, const std::wstring& fieldName);

	const std::wstring& getTableName() const;

	const std::wstring& getFieldName() const;

private:
	std::wstring m_tableName;
	std::wstring m_fieldName;
};

/*! LUA script outline parser.
 * \ingroup Script
 */
class ScriptOutlineLua : public IScriptOutline
{
	T_RTTI_CLASS;

public:
	virtual Ref< Node > parse(const std::wstring& text) const override final;

	void parseResult(Node* node) const;

	void syntaxError() const;

private:
	mutable Ref< Node > m_result;
};

}
