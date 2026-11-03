/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>
#include "Core/Object.h"

namespace traktor::html
{

class Element;

}

namespace traktor::wbr
{

/*! Widget base; builds HTML elements and consumes client events.
 * \ingroup Webber
 */
class Widget : public Object
{
	T_RTTI_CLASS;

public:
	Widget();

	int32_t getId() const { return m_id; }

	/*! Additional CSS class of widget's element. */
	void setStyleClass(const std::wstring& styleClass);

	virtual void build(html::Element* parent) const = 0;

	/*! Receive client input values, keyed by widget id; called before consume. */
	virtual void consumeValues(const std::map< int32_t, std::wstring >& values);

	virtual void consume(int32_t senderId, int32_t action);

protected:
	/*! Create widget's element, with id and CSS classes, as child of parent. */
	html::Element* createElement(const std::wstring& name, const std::wstring& className, html::Element* parent) const;

private:
	int32_t m_id;
	std::wstring m_styleClass;
};

}
