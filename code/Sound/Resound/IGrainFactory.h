/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class IGrain;
class IGrainData;

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS IGrainFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual resource::IResourceManager* getResourceManager() = 0;

	virtual Ref< IGrain > createInstance(const IGrainData* grainData) = 0;
};

	}
}

