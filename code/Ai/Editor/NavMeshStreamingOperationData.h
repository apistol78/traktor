/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/ISceneOperationData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ai
{

/*! Stream chunks of chunked navigation mesh as entities.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshStreamingOperationData : public scene::ISceneOperationData
{
	T_RTTI_CLASS;

public:
	//! Distance from chunk within which it's loaded.
	float getLoadDistance() const { return m_loadDistance; }

	virtual void serialize(ISerializer& s) override final;

private:
	float m_loadDistance = 480.0f;
};

}
