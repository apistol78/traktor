/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Resound/GrainFactory.h"
#include "Sound/Resound/IGrainData.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GrainFactory", GrainFactory, IGrainFactory)

GrainFactory::GrainFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

resource::IResourceManager* GrainFactory::getResourceManager()
{
	return m_resourceManager;
}

Ref< IGrain > GrainFactory::createInstance(const IGrainData* grainData)
{
	return grainData ? grainData->createInstance(this) : nullptr;
}

}
