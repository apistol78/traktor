/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityReferenceResolver.h"

#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/BrowseEntityDialog.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/SceneEditorPage.h"
#include "World/Entity.h"
#include "World/EntityData.h"

#include <algorithm>

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityReferenceResolver", EntityReferenceResolver, editor::IReferenceResolver)

EntityReferenceResolver::EntityReferenceResolver(SceneEditorContext* context)
	:	m_context(context)
{
}

bool EntityReferenceResolver::initialize(editor::IEditor* editor)
{
	m_paths.clear();

	for (auto entityAdapter : m_context->getEntities(SceneEditorContext::GfDescendants))
	{
		const world::EntityData* entityData = entityAdapter->getEntityData();
		if (entityData && entityData->getId().isNotNull())
			m_paths.insert(entityData->getId(), entityAdapter->getPath());
	}

	return true;
}

TypeInfoSet EntityReferenceResolver::getReferenceTypes() const
{
	return makeTypeInfoSet< world::Entity >();
}

bool EntityReferenceResolver::resolve(const Guid& id, std::wstring& outName) const
{
	const auto it = m_paths.find(id);
	if (it == m_paths.end())
		return false;

	outName = it->second;
	return true;
}

bool EntityReferenceResolver::browse(ui::Widget* parent, const TypeInfo& referenceType, const Guid& current, Guid& outId) const
{
	AlignedVector< std::pair< std::wstring, Guid > > entities;
	for (const auto& it : m_paths)
		entities.push_back({ it.second, it.first });
	std::sort(entities.begin(), entities.end(), [](const auto& lh, const auto& rh) {
		return compareIgnoreCase(lh.first, rh.first) < 0;
	});

	BrowseEntityDialog dialog;
	if (!dialog.create(parent, entities, current))
		return false;

	bool picked = false;
	if (dialog.showModal() == ui::DialogResult::Ok)
	{
		outId = dialog.getSelected();
		picked = outId.isNotNull();
	}

	dialog.destroy();
	return picked;
}

Ref< db::Instance > EntityReferenceResolver::getOwnerInstance(const Guid& id) const
{
	if (m_paths.find(id) == m_paths.end())
		return nullptr;

	editor::IDocument* document = m_context->getDocument();
	return (document && document->getInstanceCount() > 0) ? document->getInstance(0) : nullptr;
}

}
