/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Scene/Editor/ISceneOperationData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class PropertyEntityReference;

}

namespace traktor::scene
{

/*! Move static entities of scene into a streaming world component.
 * \ingroup Scene
 */
class T_DLLCLASS EntityStreamingOperationData : public ISceneOperationData
{
	T_RTTI_CLASS;

public:
	/*! Size of grid cell which entities are grouped into. */
	float getCellSize() const { return m_cellSize; }

	/*! Distance from focus within which a cell is loaded. */
	float getLoadDistance() const { return m_loadDistance; }

	/*! Distance from focus beyond which a cell is unloaded. */
	float getUnloadDistance() const { return m_unloadDistance; }

	virtual void serialize(ISerializer& s) override final;

private:
	float m_cellSize = 64.0f;
	float m_loadDistance = 200.0f;
	float m_unloadDistance = 250.0f;
	RefArray< world::PropertyEntityReference > m_residentEntities; //!< Entities which are never streamed.
};

}
