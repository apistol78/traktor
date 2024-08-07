/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Spark/Editor/Debug/ClientPage.h"
#include "Spark/Editor/Debug/MainForm.h"
#include "Net/SocketAddressIPv4.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/TabPage.h"

namespace traktor
{
	namespace spark
	{

bool MainForm::create()
{
	if (!ui::Form::create(
		L"Traktor - Remote Flash debugger",
		1000_ut,
		500_ut,
		ui::Form::WsDefault,
		new ui::FloodLayout()
	))
		return false;

	m_tab = new ui::Tab();
	m_tab->create(this);
	m_tab->addEventHandler< ui::TabCloseEvent >(this, &MainForm::eventTabClose);

	m_serverSocket = new net::TcpSocket();
	m_serverSocket->bind(net::SocketAddressIPv4(12345));
	m_serverSocket->listen();

	addEventHandler< ui::TimerEvent >(this, &MainForm::eventTimer);
	addEventHandler< ui::CloseEvent >(this, &MainForm::eventClose);

	startTimer(100);

	update();
	show();

	return true;
}

void MainForm::eventTabClose(ui::TabCloseEvent* event)
{
	Ref< ui::TabPage > tabPage = event->getTabPage();

	m_tab->removePage(tabPage);
	m_tab->update();

	safeDestroy(tabPage);
}

void MainForm::eventTimer(ui::TimerEvent* event)
{
	if (m_serverSocket->select(true, false, false, 0) > 0)
	{
		Ref< net::TcpSocket > clientSocket = m_serverSocket->accept();
		if (clientSocket)
		{
			Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(clientSocket);

			Ref< ui::TabPage > clientTabPage = new ui::TabPage();
			clientTabPage->create(m_tab, L"Client", new ui::FloodLayout());

			Ref< ClientPage > clientPage = new ClientPage();
			clientPage->create(clientTabPage, transport);

			m_tab->addPage(clientTabPage);

			update();
		}
	}
}

void MainForm::eventClose(ui::CloseEvent* event)
{
	ui::Application::getInstance()->exit(0);
}

	}
}
