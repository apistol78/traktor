/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

namespace traktor::script
{

/*! Script outline parser interface.
 * \ingroup Script
 */
class IScriptOutline : public Object
{
	T_RTTI_CLASS;

public:
	class Node : public Object
	{
		T_RTTI_CLASS;

	public:
		Node(int32_t line);

		void setNext(Node* node);

		void setNextTail(Node* node);

		Node* getNext() const;

		int32_t getLine() const;

	private:
		Ref< Node > m_next;
		int32_t m_line;
	};

	class FunctionReferenceNode : public Node
	{
		T_RTTI_CLASS;

	public:
		FunctionReferenceNode(int32_t line, const std::wstring& name);

		const std::wstring& getName() const;

	private:
		std::wstring m_name;
	};

	class FunctionNode : public Node
	{
		T_RTTI_CLASS;

	public:
		FunctionNode(int32_t line, const std::wstring& name, bool local, Node* body);

		const std::wstring& getName() const;

		bool isLocal() const;

		Node* getBody() const;

	private:
		std::wstring m_name;
		bool m_local;
		Ref< Node > m_body;
	};

	virtual Ref< Node > parse(const std::wstring& text) const = 0;
};

}
