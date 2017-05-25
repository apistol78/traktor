/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_MainForm_H
#define traktor_flash_MainForm_H

#include "Net/TcpSocket.h"
#include "Ui/Form.h"
#include "Ui/Tab.h"

namespace traktor
{
	namespace flash
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

#endif	// traktor_flash_MainForm_H
