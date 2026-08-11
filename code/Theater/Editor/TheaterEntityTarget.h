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

namespace traktor::theater
{

class TheaterEntityComponentData;

/*! Theater entity component being edited; animate child entities of the owner entity. */
class TheaterEntityTarget : public TheaterEditTarget
{
	T_RTTI_CLASS;

public:
	explicit TheaterEntityTarget(scene::EntityAdapter* ownerAdapter, TheaterEntityComponentData* componentData);

	virtual std::wstring getName() const override final;

	virtual Guid getId() const override final;

	virtual RefArray< ActData >& getActs() override final;

	virtual Transform getBaseTransform() const override final;

	virtual scene::EntityAdapter* findEntityAdapter(const Guid& id) const override final;

	virtual bool canCapture(const scene::EntityAdapter* entityAdapter) const override final;

	virtual Performance* getPerformance() const override final;

	virtual void rebuild() override final;

private:
	Ref< scene::EntityAdapter > m_ownerAdapter;
	Ref< TheaterEntityComponentData > m_componentData;
};

}
