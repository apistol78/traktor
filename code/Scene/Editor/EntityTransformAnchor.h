/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/IModifierAnchor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class EntityAdapter;

/*! Entity transform anchor.
 * \ingroup Scene
 *
 * Default anchor implementation that wraps an EntityAdapter's
 * transform. Get/set operations delegate directly to the
 * adapter, preserving the existing component editor notification chain.
 */
class T_DLLCLASS EntityTransformAnchor : public IModifierAnchor
{
	T_RTTI_CLASS;

public:
	explicit EntityTransformAnchor(EntityAdapter* entityAdapter);

	virtual Transform getTransform() const override final;

	virtual void setTransform(const Transform& transform) override final;

	EntityAdapter* getEntityAdapter() const;

private:
	EntityAdapter* m_entityAdapter;
};

}
