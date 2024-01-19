/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*! Audio filter instance data.
* \ingroup Sound
*/
struct T_DLLCLASS IAudioFilterInstance : public IRefCount
{
};

/*! Audio filer base class.
 * \ingroup Sound
 */
class T_DLLCLASS IAudioFilter : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IAudioFilterInstance > createInstance() const = 0;

	virtual void apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const = 0;
};

}
