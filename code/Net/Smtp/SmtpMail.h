/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

/*! SMTP mail sender.
 * \ingroup Net
 */
class T_DLLCLASS SmtpMail : public Object
{
	T_RTTI_CLASS;

public:
	explicit SmtpMail(const std::wstring& server, int port);

	bool send(const std::wstring& to, const std::wstring& from, const std::wstring& message);

private:
	std::wstring m_server;
	int m_port;
};

}
