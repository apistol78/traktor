/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Net/TcpSocket.h"
#include "Ui/Form.h"
#include "Ui/Tab.h"

namespace traktor
{
	namespace spark
	{

class MainForm : public ui::Form
{
public:
	bool create();

private:
	Ref< ui::Tab > m_tab;
	Ref< net::TcpSocket > m_serverSocket;

	void eventTabClose(ui::TabCloseEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void eventClose(ui::CloseEvent* event);
};

	}
}

