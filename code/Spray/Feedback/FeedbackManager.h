/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Spray/Feedback/IFeedbackManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS FeedbackManager : public IFeedbackManager
{
	T_RTTI_CLASS;

public:
	virtual void addListener(FeedbackType type, IFeedbackListener* listener) override final;

	virtual void removeListener(FeedbackType type, IFeedbackListener* listener) override final;

	virtual void apply(FeedbackType type, const float* values, int32_t count) override final;

private:
	SmallMap< FeedbackType, AlignedVector< IFeedbackListener* > > m_listeners;
};

	}
}

