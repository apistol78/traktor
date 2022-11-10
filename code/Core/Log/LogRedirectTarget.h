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
#include "Core/Log/Log.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_DLLCLASS LogRedirectTarget : public ILogTarget
{
public:
	explicit LogRedirectTarget(ILogTarget* target);

	explicit LogRedirectTarget(ILogTarget* target1, ILogTarget* target2);

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final;

private:
	RefArray< ILogTarget > m_targets;
};

}
