/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Aabb3.h"
#include "Core/Containers/AlignedVector.h"
#include "Weather/Clouds/CloudParticle.h"

namespace traktor
{
	namespace weather
	{

class CloudParticleData;

class CloudParticleCluster
{
public:
	bool create(const CloudParticleData& particleData);

	void update(const CloudParticleData& particleData, float deltaTime);

	const Aabb3& getBoundingBox() const;

	const AlignedVector< CloudParticle >& getParticles() const;

private:
	Aabb3 m_boundingBox;
	AlignedVector< CloudParticle > m_particles;
};

	}
}

