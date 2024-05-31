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
#include "Spark/ISoundRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class ISoundPlayer;

}

namespace traktor::spark
{

/*! Default sound renderer using Traktor sound package.
 * \ingroup Spark
 */
class T_DLLCLASS SoundRenderer : public ISoundRenderer
{
	T_RTTI_CLASS;

public:
	bool create(sound::ISoundPlayer* soundPlayer);

	virtual void destroy() override final;

	virtual void play(const Sound* sound) override final;

private:
	Ref< sound::ISoundPlayer > m_soundPlayer;
};

}
