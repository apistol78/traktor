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
#include "Theater/Editor/TheaterEditTarget.h"

namespace traktor::scene
{

class SceneEditorContext;

}

namespace traktor::theater
{

class TheaterWorldComponentData;

/*! Theater world component being edited; animate any entity of the scene. */
class TheaterWorldTarget : public TheaterEditTarget
{
	T_RTTI_CLASS;

public:
	explicit TheaterWorldTarget(scene::SceneEditorContext* context, TheaterWorldComponentData* componentData);

	virtual std::wstring getName() const override final;

	virtual Guid getId() const override final;

	virtual RefArray< ActData >& getActs() override final;

	virtual Transform getBaseTransform() const override final;

	virtual scene::EntityAdapter* findEntityAdapter(const Guid& id) const override final;

	virtual bool canCapture(const scene::EntityAdapter* entityAdapter) const override final;

	virtual Performance* getPerformance() const override final;

	virtual void rebuild() override final;

private:
	Ref< scene::SceneEditorContext > m_context;
	Ref< TheaterWorldComponentData > m_componentData;
};

}
