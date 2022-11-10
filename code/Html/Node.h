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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStream;

	namespace html
	{

/*! HTML DOM Node.
 * \ingroup HTML
 */
class T_DLLCLASS Node : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getName() const;

	virtual std::wstring getValue() const;

	void addChild(Node* child);

	void insertBefore(Node* child, Node* node);

	void insertAfter(Node* child, Node* node);

	Node* getParent() const;

	Node* getPreviousSibling() const;

	Node* getNextSibling() const;

	Node* getFirstChild() const;

	Node* getLastChild() const;

	virtual void toString(OutputStream& os) const;

private:
	Node* m_parent = nullptr;
	Node* m_previousSibling = nullptr;
	Ref< Node > m_nextSibling;
	Ref< Node > m_firstChild;
	Ref< Node > m_lastChild;
};

	}
}

