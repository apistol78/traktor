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
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStream;

}

namespace traktor::xml
{

/*! XML Node.
 * \ingroup XML
 */
class T_DLLCLASS Node : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~Node();

	virtual std::wstring getName() const;

	virtual void setName(const std::wstring& name);

	virtual std::wstring getValue() const;

	virtual void setValue(const std::wstring& value);

	virtual void write(OutputStream& os) const;

	void unlink();

	void addChild(Node* child);

	void removeChild(Node* child);

	void removeAllChildren();

	void insertBefore(Node* child, Node* beforeNode);

	void insertAfter(Node* child, Node* afterNode);

	Node* getParent() const;

	Node* getPreviousSibling() const;

	Node* getNextSibling() const;

	Node* getFirstChild() const;

	Node* getLastChild() const;

protected:
	virtual Ref< Node > cloneUntyped() const = 0;

	void cloneChildren(Node* clone) const;

private:
	Node* m_parent = nullptr;
	Node* m_previousSibling = nullptr;
	Ref< Node > m_nextSibling;
	Ref< Node > m_firstChild;
	Ref< Node > m_lastChild;
};

}
