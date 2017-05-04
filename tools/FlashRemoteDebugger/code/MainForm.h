/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef MainForm_H
#define MainForm_H

#include <Net/TcpSocket.h>
#include <Ui/Form.h>
#include <Ui/Tab.h>

class MainForm : public traktor::ui::Form
{
public:
	bool create();

private:
	traktor::Ref< traktor::ui::Tab > m_tab;
	traktor::Ref< traktor::net::TcpSocket > m_serverSocket;

	void eventTimer(traktor::ui::TimerEvent* event);

	void eventClose(traktor::ui::CloseEvent* event);
};

#endif	// MainForm_H
