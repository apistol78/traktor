/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

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

enum FeedbackType
{
	FbtNone			= 0,
	FbtCamera		= 1,
	FbtImageProcess	= 2,
	FbtUI			= 3
};

class T_DLLCLASS IFeedbackListener
{
public:
	virtual ~IFeedbackListener() {}

	virtual void feedbackValues(FeedbackType type, const float* values, int32_t count) = 0;
};

	}
}

