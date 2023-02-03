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
#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::net
{

class DiscoveryManager;

}

namespace traktor::runtime
{

/*! Enumerator of hosts able to communicate with editor.
 * \ingroup Runtime
 */
class HostEnumerator : public Object
{
	T_RTTI_CLASS;

public:
	explicit HostEnumerator(const PropertyGroup* settings, net::DiscoveryManager* discoveryManager);

	int32_t count() const;

	const std::wstring& getHost(int32_t index) const;

	const std::wstring& getDescription(int32_t index) const;

	bool supportPlatform(int32_t index, const std::wstring& platform) const;

	bool isLocal(int32_t index) const;

	void update();

private:
	struct Host
	{
		std::wstring description;
		std::wstring host;
		AlignedVector< std::wstring > platforms;
		bool local;

		Host();

		bool operator < (const Host& h) const;
	};

	Ref< net::DiscoveryManager > m_discoveryManager;
	mutable Semaphore m_lock;
	std::vector< Host > m_manual;
	mutable std::vector< Host > m_hosts;
	mutable std::vector< Host > m_pending;
};

}
