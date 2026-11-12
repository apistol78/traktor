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
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::terrain
{

/*! Operation data for splitting terrain collision into streamed tiles.
 * \ingroup Terrain
 *
 * Must be placed after entity streaming operation since tiles are added as cells of entity streaming.
 */
class T_DLLCLASS TerrainCollisionOperationData : public scene::ISceneOperationData
{
	T_RTTI_CLASS;

public:
	//! Distance from tile within which it's collision is loaded.
	float getLoadDistance() const { return m_loadDistance; }

	virtual void serialize(ISerializer& s) override final;

private:
	float m_loadDistance = 640.0f;
};

}
