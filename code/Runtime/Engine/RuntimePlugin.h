/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IRuntimePlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class T_DLLCLASS RuntimePlugin : public IRuntimePlugin
{
	T_RTTI_CLASS;

public:
	virtual bool create(IEnvironment* environment) override final;

	virtual void destroy(IEnvironment* environment) override final;

	virtual Ref< IState > createInitialState(IEnvironment* environment) override final;
};

	}
}

