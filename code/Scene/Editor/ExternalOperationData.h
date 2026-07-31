/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Scene/Editor/ISceneOperationData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class T_DLLCLASS ExternalOperationData : public ISceneOperationData
{
	T_RTTI_CLASS;

public:
	const Guid& getExternalDataId() const { return m_externalDataId; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_externalDataId;
};

}
