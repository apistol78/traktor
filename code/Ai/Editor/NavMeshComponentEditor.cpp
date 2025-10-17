/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/Editor/NavMeshComponentEditor.h"
#include "Ai/NavMesh.h"
#include "Ai/NavMeshComponentData.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshComponentEditor", NavMeshComponentEditor, scene::IWorldComponentEditor)

bool NavMeshComponentEditor::create(scene::SceneEditorContext* context, ui::Container* parent)
{
	m_context = context;
	return true;
}

void NavMeshComponentEditor::destroy()
{
}

void NavMeshComponentEditor::entityRemoved(scene::EntityAdapter* entityAdapter)
{
}

void NavMeshComponentEditor::propertiesChanged()
{
}

bool NavMeshComponentEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void NavMeshComponentEditor::update()
{
}

void NavMeshComponentEditor::draw(render::PrimitiveRenderer* primitiveRenderer)
{
	NavMeshComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< NavMeshComponentData >();
	if (!componentData)
		return;

	if (m_context->shouldDrawGuide(L"Ai.NavMesh"))
	{
		resource::Proxy< NavMesh > navMesh;
		if (!m_context->getResourceManager()->bind(componentData->get(), navMesh))
			return;

		if (navMesh->m_navMeshPolygons.empty())
			return;

		primitiveRenderer->pushWorld(Matrix44::identity());
		primitiveRenderer->pushDepthState(true, false, false);

		const uint16_t* nmp = navMesh->m_navMeshPolygons.c_ptr();
		T_ASSERT(nmp);

		for (uint32_t i = 0; i < navMesh->m_navMeshPolygons.size(); )
		{
			const uint16_t npv = nmp[i++];
			for (uint16_t j = 0; j < npv - 2; ++j)
			{
				const uint16_t i0 = nmp[i];
				const uint16_t i1 = nmp[i + j + 1];
				const uint16_t i2 = nmp[i + j + 2];
				primitiveRenderer->drawSolidTriangle(
					navMesh->m_navMeshVertices[i0],
					navMesh->m_navMeshVertices[i1],
					navMesh->m_navMeshVertices[i2],
					Color4ub(0, 255, 255, 64)
				);
			}
			i += npv;
		}

		primitiveRenderer->popDepthState();
		primitiveRenderer->pushDepthState(false, false, false);

		for (uint32_t i = 0; i < navMesh->m_navMeshPolygons.size(); )
		{
			const uint16_t npv = nmp[i++];
			for (uint16_t j = 0; j < npv; ++j)
			{
				const uint16_t i0 = nmp[i + j];
				const uint16_t i1 = nmp[i + (j + 1) % npv];
				primitiveRenderer->drawLine(
					navMesh->m_navMeshVertices[i0],
					navMesh->m_navMeshVertices[i1],
					Color4ub(0, 255, 255, 255)
				);
			}
			i += npv;
		}

		primitiveRenderer->popDepthState();
		primitiveRenderer->popWorld();
	}
}

}
