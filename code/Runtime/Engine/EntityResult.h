/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Result.h"

#include <atomic>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;

}

namespace traktor::runtime
{

/*! Deferred result of asynchronously created entity.
 * \ingroup Runtime
 */
class T_DLLCLASS EntityResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(world::Entity* entity);

	world::Entity* get() const;

	void cancel();

	bool cancelled() const;

private:
	Ref< world::Entity > m_entity;
	std::atomic< bool > m_cancelled = false;
};

}
