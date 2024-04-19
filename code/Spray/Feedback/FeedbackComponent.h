/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Spray/Feedback/IFeedbackListener.h"
#include "World/IWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spray
{

class T_DLLCLASS FeedbackComponent : public world::IWorldComponent
{
	T_RTTI_CLASS;

public:
	virtual void destroy() override final;

	virtual void update(world::World* world, const world::UpdateParams& update) override final;

	void addListener(FeedbackType type, IFeedbackListener* listener);

	void removeListener(FeedbackType type, IFeedbackListener* listener);

	void apply(FeedbackType type, const float* values, int32_t count);

private:
	SmallMap< FeedbackType, AlignedVector< IFeedbackListener* > > m_listeners;
};

}
