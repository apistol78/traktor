/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateControl.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Layer.h"
#include "Runtime/Engine/Stage.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.Layer", Layer, Object)

Layer::Layer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition
)
:	m_stage(stage)
,	m_name(name)
,	m_permitTransition(permitTransition)
{
}

Layer::~Layer()
{
	destroy();
}

void Layer::destroy()
{
	m_stage = nullptr;
}

}
