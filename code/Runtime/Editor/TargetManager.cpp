/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Editor/IEditor.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketSet.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Runtime/Editor/TargetInstance.h"
#include "Runtime/Editor/TargetManager.h"
#include "Runtime/Target/TargetID.h"
#include "Script/Editor/IScriptDebuggerSessions.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetManager", TargetManager, Object)

TargetManager::TargetManager(editor::IEditor* editor, const fn_new_log_target_t& newLogTargetFn)
:	m_editor(editor)
,	m_newLogTargetFn(newLogTargetFn)
,	m_port(0)
{
}

bool TargetManager::create()
{
	// Create our server socket.
	m_listenSocket = new net::TcpSocket();
	if (!m_listenSocket->bind(net::SocketAddressIPv4(0)))
	{
		log::error << L"Failed to create target manager; Unable to bind socket." << Endl;
		return false;
	}

	if (!m_listenSocket->listen())
	{
		log::error << L"Failed to create target manager; Unable to listen on socket." << Endl;
		return false;
	}

	m_port = dynamic_type_cast< net::SocketAddressIPv4* >(m_listenSocket->getLocalAddress())->getPort();

	log::info << L"Target manager @" << m_port << L" created." << Endl;
	return true;
}

void TargetManager::destroy()
{
	m_instances.clear();
	safeClose(m_listenSocket);
	m_editor = nullptr;
}

void TargetManager::addInstance(TargetInstance* targetInstance)
{
	m_instances.push_back(targetInstance);
}

void TargetManager::removeInstance(TargetInstance* targetInstance)
{
	m_instances.remove(targetInstance);
}

void TargetManager::removeAllInstances()
{
	m_instances.clear();
}

bool TargetManager::update()
{
	net::SocketSet socketSet, socketSetResult;
	bool needUpdate = false;

	// Update all targets; if any has been disconnected then we return true in order
	// to update user interface.
	for (auto instance : m_instances)
	{
		if (instance->update())
			needUpdate |= true;
	}

	// Gather all sockets so we can wait on all simultaneously.
	socketSet.add(m_listenSocket);
	for (auto instance : m_instances)
	{
		RefArray< TargetConnection> connections = instance->getConnections();
		for (auto connection : connections)
		{
			net::BidirectionalObjectTransport* transport = connection->getTransport();
			T_ASSERT(transport);

			if (transport->getSocket())
				socketSet.add(transport->getSocket());
		}
	}

	// Wait on all sockets.
	if (socketSet.select(true, false, false, 0, socketSetResult) <= 0)
		return needUpdate;

	// Check if any pending connection available.
	if (m_listenSocket->select(true, false, false, 0))
	{
		// Accept new connection.
		Ref< net::TcpSocket > socket = m_listenSocket->accept();
		if (socket)
		{
			Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(socket);
			socket->setNoDelay(true);
			socket->setQuickAck(true);

			// Target must send it's identifier upon connection.
			Ref< TargetID > targetId;
			if (transport->recv< TargetID >(1000, targetId) == net::BidirectionalObjectTransport::Result::Success)
			{
				// Find instance with matching identifier.
				auto it = std::find_if(m_instances.begin(), m_instances.end(), [&](TargetInstance* ti) {
					return ti->getId() == targetId->getId();
				});
				if (it != m_instances.end())
				{
					TargetInstance* instance = *it;

					// Determine log name; if multiple running instances with same name then add number.
					int32_t running = 0;
					for (auto connection : instance->getConnections())
					{
						if (connection->getName() == targetId->getName())
							++running;
					}

					std::wstring logName = targetId->getName();
					if (running > 0)
						logName += L" (" + toString(running) + L")";

					Ref< ILogTarget > targetLog = m_newLogTargetFn(logName);

					// Create connection object and add to instance.
					Ref< script::IScriptDebuggerSessions > debuggerSessions = m_editor->getObjectStore()->get< script::IScriptDebuggerSessions >();
					instance->addConnection(new TargetConnection(targetId->getName(), transport, targetLog, debuggerSessions));
					needUpdate |= true;
					log::info << L"New target connection accepted; ID " << targetId->getId().format() << Endl;
				}
				else
				{
					// Unknown target; refusing connection.
					log::error << L"Unknown target ID " << targetId->getId().format() << L" from " << targetId->getName() << L"; connection refused" << Endl;
				}
			}
			else
			{
				// Invalid target ID message; refusing connection.
				log::error << L"Invalid target ID message; connection refused" << Endl;
			}
		}
	}

	return needUpdate;
}

}
