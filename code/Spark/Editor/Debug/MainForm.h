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

