/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Spray/Feedback/FeedbackComponent.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.FeedbackComponent", FeedbackComponent, world::IWorldComponent)

void FeedbackComponent::destroy()
{
}

void FeedbackComponent::update(world::World* world, const world::UpdateParams& update)
{
}

void FeedbackComponent::addListener(FeedbackType type, IFeedbackListener* listener)
{
	m_listeners[type].push_back(listener);
}

void FeedbackComponent::removeListener(FeedbackType type, IFeedbackListener* listener)
{
	auto& listeners = m_listeners[type];
	auto it = std::find(listeners.begin(), listeners.end(), listener);
	if (it != listeners.end())
		listeners.erase(it);
}

void FeedbackComponent::apply(FeedbackType type, const float* values, int32_t count)
{
	auto& listeners = m_listeners[type];
	for (auto listener : listeners)
		listener->feedbackValues(type, values, count);
}

}
