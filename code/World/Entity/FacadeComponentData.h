/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "World/Entity/GroupComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityData;

/*! Facade component persistent data.
 * \ingroup World
 */
class T_DLLCLASS FacadeComponentData : public GroupComponentData
{
	T_RTTI_CLASS;

public:
	const std::wstring& getShow() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_show;
};

	}
}
