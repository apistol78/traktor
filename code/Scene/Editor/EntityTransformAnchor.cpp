/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityTransformAnchor.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityTransformAnchor", EntityTransformAnchor, IModifierAnchor)

EntityTransformAnchor::EntityTransformAnchor(EntityAdapter* entityAdapter)
:	m_entityAdapter(entityAdapter)
{
}

Transform EntityTransformAnchor::getTransform() const
{
	return m_entityAdapter->getTransform();
}

void EntityTransformAnchor::setTransform(const Transform& transform)
{
	m_entityAdapter->setTransform(transform);
}

EntityAdapter* EntityTransformAnchor::getEntityAdapter() const
{
	return m_entityAdapter;
}

}
