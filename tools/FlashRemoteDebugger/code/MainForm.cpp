/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Net/SocketAddressIPv4.h>
#include <Ui/Application.h>
#include <Ui/FloodLayout.h>
#include <Ui/TabPage.h>
#include "MainForm.h"
#include "DebugView.h"

using namespace traktor;

bool MainForm::create()
{
	if (!ui::Form::create(
		L"Traktor - Remote Flash debugger",
		ui::scaleBySystemDPI(1000),
		ui::scaleBySystemDPI(500),
		ui::Form::WsDefault,
		new ui::FloodLayout()
	))
		return false;

	m_tab = new ui::Tab();
	m_tab->create(this);

	m_serverSocket = new net::TcpSocket();
	m_serverSocket->bind(net::SocketAddressIPv4(12345));
	m_serverSocket->listen();

	addEventHandler< ui::TimerEvent >(this, &MainForm::eventTimer);
	startTimer(100);

	update();
	show();

	return true;
}

void MainForm::eventTimer(ui::TimerEvent* event)
{
	if (m_serverSocket->select(true, false, false, 0) > 0)
	{
		Ref< net::TcpSocket > clientSocket = m_serverSocket->accept();
		if (clientSocket)
		{
			Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(clientSocket);

			Ref< ui::TabPage > clientPage = new ui::TabPage();
			clientPage->create(m_tab, L"Client", new ui::FloodLayout());

			Ref< DebugView > clientDebugView = new DebugView();
			clientDebugView->create(clientPage, transport);

			m_tab->addPage(clientPage);

			update();
		}
	}
}
