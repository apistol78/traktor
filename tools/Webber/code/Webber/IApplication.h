/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class IStream;

}

namespace traktor::wbr
{

class Widget;

/*! Web application; one instance is created per client session.
 * \ingroup Webber
 */
class IApplication : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create() = 0;

	virtual void destroy() = 0;

	virtual Widget* getRootWidget() const = 0;

	/*! Application stylesheet URI, served through resolve; empty if none. */
	virtual std::wstring getStyleSheet() const = 0;

	virtual Ref< IStream > resolve(const std::wstring& uri) const = 0;

	/*! Location of current view, kept in the browser's history; empty for the start view. */
	virtual std::wstring getLocation() const;

	/*! Show view at location; from browser back/forward or a loaded URL, so location is untrusted. */
	virtual void navigate(const std::wstring& location);
};

}
