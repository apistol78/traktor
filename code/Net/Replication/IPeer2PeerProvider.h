/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_IPeer2PeerProvider_H
#define traktor_net_IPeer2PeerProvider_H

#include <string>
#include "Core/Object.h"
#include "Net/Replication/NetworkTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS IPeer2PeerProvider : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool update() = 0;

	virtual net_handle_t getLocalHandle() const = 0;

	virtual int32_t getPeerCount() const = 0;

	virtual net_handle_t getPeerHandle(int32_t index) const = 0;

	virtual std::wstring getPeerName(int32_t index) const = 0;

	virtual Object* getPeerUser(int32_t index) const = 0;

	virtual bool setPrimaryPeerHandle(net_handle_t node) = 0;

	virtual net_handle_t getPrimaryPeerHandle() const = 0;

	virtual bool send(net_handle_t node, const void* data, int32_t size) = 0;

	virtual int32_t recv(void* data, int32_t size, net_handle_t& outNode) = 0;
};

	}
}

#endif	// traktor_net_IPeer2PeerProvider_H
