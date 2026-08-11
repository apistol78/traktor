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
#include "World/IEntityComponent.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::theater
{

class Performance;

/*! Theater entity component.
 *
 * Animate child entities of the owner entity's group; all tracks are
 * relative the owner's transform so an animated group of entities can
 * be instantiated, several times, from an external entity.
 *
 * \ingroup Theater
 */
class T_DLLCLASS TheaterEntityComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit TheaterEntityComponent(Performance* performance);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual bool allowConcurrentUpdate() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	bool play(const std::wstring& actName);

	void stop();

	bool isPlaying() const;

	Performance* getPerformance() const { return m_performance; }

private:
	world::Entity* m_owner = nullptr;
	Ref< Performance > m_performance;
};

}
