#ifndef traktor_amalgam_TargetManager_H
#define traktor_amalgam_TargetManager_H

#include "Core/RefArray.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace amalgam
	{

class TargetInstance;
class TargetScriptDebuggerSessions;

class TargetManager : public Object
{
	T_RTTI_CLASS;

public:
	TargetManager(editor::IEditor* editor, TargetScriptDebuggerSessions* targetDebuggerSessions);

	bool create(uint16_t port);

	void destroy();

	void addInstance(TargetInstance* targetInstance);

	void removeInstance(TargetInstance* targetInstance);

	void removeAllInstances();

	bool update();

private:
	editor::IEditor* m_editor;
	Ref< TargetScriptDebuggerSessions > m_targetDebuggerSessions;
	Ref< net::TcpSocket > m_listenSocket;
	RefArray< TargetInstance > m_instances;
};

	}
}

#endif	// traktor_amalgam_TargetManager_H
