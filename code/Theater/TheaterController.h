/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/RefArray.h"
#include "Scene/ISceneController.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace theater
	{

class Act;

/*! Theater scene controller.
 * \ingroup Theater
 */
class T_DLLCLASS TheaterController : public scene::ISceneController
{
	T_RTTI_CLASS;

public:
	explicit TheaterController(const RefArray< const Act >& acts, float totalDuration);

	bool play(const std::wstring& actName);

	virtual void update(scene::Scene* scene, float time, float deltaTime) override final;

private:
	friend class TheaterControllerEditor;

	RefArray< const Act > m_acts;
	float m_totalDuration = 0.0f;
	const Act* m_act = nullptr;
	float m_timeStart = -1.0f;
	float m_timeLast = -1.0f;
};

	}
}

