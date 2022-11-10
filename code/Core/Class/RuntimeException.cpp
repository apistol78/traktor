/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/RuntimeException.h"

namespace traktor
{

RuntimeException::RuntimeException(const std::wstring& what_)
:	m_what(what_)
{
}

const std::wstring& RuntimeException::what() const throw()
{
	return m_what;
}

}
