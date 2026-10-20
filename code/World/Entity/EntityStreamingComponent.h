/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Thread/Job.h"
#include "World/IWorldComponent.h"
#include "World/Entity/EntityStreamingComponentData.h"

#include <atomic>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class Entity;
class IEntityBuilder;

/*! Streams cells of entities in and out of world by distance to a focus point.
 * \ingroup World
 */
class T_DLLCLASS EntityStreamingComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	explicit EntityStreamingComponent(const IEntityBuilder* entityBuilder, resource::IResourceManager* resourceManager, const EntityStreamingComponentData* data);

	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	/*! Set point around which cells are streamed; nothing is streamed until set. */
	void setFocus(const Vector4& focus);

private:
	struct Cell
	{
		const EntityStreamingComponentData::Cell* data = nullptr;
		RefArray< Entity > entities;
		bool loaded = false;
	};

	Ref< const IEntityBuilder > m_entityBuilder;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< const EntityStreamingComponentData > m_data;
	AlignedVector< Cell > m_cells;
	Vector4 m_focus = Vector4::origo();
	bool m_haveFocus = false;
	bool m_primed = false;
	Ref< Job > m_job;
	int32_t m_loadingCell = -1;
	std::atomic< bool > m_cancel = false;

	void createEntities(Cell& cell) const;
};

}
