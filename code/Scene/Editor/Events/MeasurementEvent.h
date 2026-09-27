/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class T_DLLCLASS MeasurementEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	explicit MeasurementEvent(ui::EventSubject* sender, int32_t viewport, int32_t pass, int32_t level, bool asynchronous, const std::wstring& name, double start, double duration);

	/*! Viewport, camera index, of render control which was measured. */
	int32_t getViewport() const { return m_viewport; }

    int32_t getPass() const { return m_pass; }

    int32_t getLevel() const { return m_level; }

	/*! Pass was executed on the asynchronous compute queue, i.e. overlaps graphics passes. */
	bool isAsynchronous() const { return m_asynchronous; }

    const std::wstring& getName() const { return m_name; }

    double getStart() const { return m_start; }

    double getDuration() const { return m_duration; }

private:
	int32_t m_viewport;
    int32_t m_pass;
    int32_t m_level;
	bool m_asynchronous;
    std::wstring m_name;
    double m_start;
    double m_duration;
};

}
