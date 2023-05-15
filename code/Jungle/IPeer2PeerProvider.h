/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Jungle/NetworkTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JUNGLE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::jungle
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
