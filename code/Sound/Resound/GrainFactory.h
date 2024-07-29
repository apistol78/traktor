/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/Resound/IGrainFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*!
 * \ingroup Sound
 */
class T_DLLCLASS GrainFactory : public IGrainFactory
{
	T_RTTI_CLASS;

public:
	explicit GrainFactory(resource::IResourceManager* resourceManager);

	virtual resource::IResourceManager* getResourceManager() override final;

	virtual Ref< IGrain > createInstance(const IGrainData* grainData) override;

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

}
