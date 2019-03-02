#pragma once

#include "Core/RefArray.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace script
	{

class IScriptDebuggerSessions;

	}

	namespace amalgam
	{

class TargetInstance;

/*! \brief
 * \ingroup Amalgam
 */
class TargetManager : public Object
{
	T_RTTI_CLASS;

public:
	TargetManager(editor::IEditor *editor);

	bool create();

	void destroy();

	void addInstance(TargetInstance* targetInstance);

	void removeInstance(TargetInstance* targetInstance);

	void removeAllInstances();

	bool update();

	uint16_t getPort() const { return m_port; }

private:
	editor::IEditor* m_editor;
	Ref< net::TcpSocket > m_listenSocket;
	RefArray< TargetInstance > m_instances;
	uint16_t m_port;
};

	}
}

