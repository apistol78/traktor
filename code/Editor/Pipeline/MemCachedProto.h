/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_MemCachedProto_H
#define traktor_editor_MemCachedProto_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace net
	{

class Socket;

	}

	namespace editor
	{

class MemCachedProto : public Object
{
	T_RTTI_CLASS;

public:
	MemCachedProto(net::Socket* socket);

	Semaphore& getLock();

	bool sendCommand(const std::string& command);

	bool readReply(std::string& outReply);

	bool readData(uint8_t* data, uint32_t dataLength);

	/*! \brief Write data to memcached.
	 * \note Data block must contain space for two more
	 *       bytes as we need to append \r\n.
	 */
	bool writeData(uint8_t* data, uint32_t dataLength);

private:
	Semaphore m_lock;
	Ref< net::Socket > m_socket;
};

	}
}

#endif	// traktor_editor_MemCachedProto_H
