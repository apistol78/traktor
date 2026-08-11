/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterWorldTarget.h"

#include "I18N/Text.h"
#include "Scene/Scene.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Theater/TheaterWorldComponent.h"
#include "Theater/TheaterWorldComponentData.h"
#include "World/World.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterWorldTarget", TheaterWorldTarget, TheaterEditTarget)

TheaterWorldTarget::TheaterWorldTarget(scene::SceneEditorContext* context, TheaterWorldComponentData* componentData)
:	m_context(context)
,	m_componentData(componentData)
{
}

std::wstring TheaterWorldTarget::getName() const
{
	return i18n::Text(L"THEATER_EDITOR_TARGET_WORLD");
}

Guid TheaterWorldTarget::getId() const
{
	return Guid();
}

RefArray< ActData >& TheaterWorldTarget::getActs()
{
	return m_componentData->getActs();
}

Transform TheaterWorldTarget::getBaseTransform() const
{
	return Transform::identity();
}

scene::EntityAdapter* TheaterWorldTarget::findEntityAdapter(const Guid& id) const
{
	for (auto entityAdapter : m_context->getEntities())
	{
		if (entityAdapter->getId() == id)
			return entityAdapter;
	}
	return nullptr;
}

bool TheaterWorldTarget::canCapture(const scene::EntityAdapter* entityAdapter) const
{
	return entityAdapter != nullptr;
}

Performance* TheaterWorldTarget::getPerformance() const
{
	scene::Scene* scene = m_context->getScene();
	if (scene == nullptr)
		return nullptr;

	TheaterWorldComponent* component = scene->getWorld()->getComponent< TheaterWorldComponent >();
	return component != nullptr ? component->getPerformance() : nullptr;
}

void TheaterWorldTarget::rebuild()
{
	// World components are only instantiated when the controller is built.
	m_context->buildController();
}

}
