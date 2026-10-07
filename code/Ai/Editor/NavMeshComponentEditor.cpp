/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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

#include <DetourNavMesh.h>

namespace traktor::ai
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshComponentEditor", NavMeshComponentEditor, scene::IComponentPanelEditor)

std::wstring NavMeshComponentEditor::getTitle() const
{
	return L"Navigation mesh";
}

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

		const dtNavMesh* nm = navMesh->m_navMesh;
		if (!nm)
			return;

		primitiveRenderer->pushWorld(Matrix44::identity());

		// First pass draw depth tested polygons, second pass draw outlines on top.
		for (int32_t pass = 0; pass < 2; ++pass)
		{
			primitiveRenderer->pushDepthState(pass == 0, false, false);

			for (int32_t i = 0; i < nm->getMaxTiles(); ++i)
			{
				const dtMeshTile* tile = nm->getTile(i);
				if (!tile->header)
					continue;

				for (int32_t j = 0; j < tile->header->polyCount; ++j)
				{
					const dtPoly& poly = tile->polys[j];
					if (poly.getType() == DT_POLYTYPE_OFFMESH_CONNECTION)
						continue;

					Vector4 v[DT_VERTS_PER_POLYGON];
					for (int32_t k = 0; k < poly.vertCount; ++k)
					{
						const float* p = &tile->verts[poly.verts[k] * 3];
						v[k] = Vector4(p[0], p[1], p[2], 1.0f);
					}

					if (pass == 0)
					{
						for (int32_t k = 1; k < poly.vertCount - 1; ++k)
							primitiveRenderer->drawSolidTriangle(v[0], v[k], v[k + 1], Color4ub(0, 255, 255, 64));
					}
					else
					{
						for (int32_t k = 0; k < poly.vertCount; ++k)
							primitiveRenderer->drawLine(v[k], v[(k + 1) % poly.vertCount], Color4ub(0, 255, 255, 255));
					}
				}
			}

			primitiveRenderer->popDepthState();
		}

		primitiveRenderer->popWorld();
	}
}

}
