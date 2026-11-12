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
#include "Ai/NavMeshChunk.h"
#include "Ai/NavMeshComponent.h"
#include "Ai/NavMeshComponentData.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity.h"
#include "World/World.h"

#include <DetourNavMesh.h>
#include <Recast.h>

namespace traktor::ai
{
namespace
{

const float c_drawDistance = 256.0f;

}

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
	removeChunks();
	m_navMeshComponent = nullptr;
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

	const bool buildNavMeshEditor = m_context->getEditor()->getSettings()->getProperty< bool >(L"NavMeshPipeline.Build", true);
	if (!buildNavMeshEditor)
		return;

	if (m_context->shouldDrawGuide(L"Ai.NavMesh"))
	{
		// World of scene, reached through a layer entity.
		const RefArray< scene::EntityAdapter > layers = m_context->getLayerEntityAdapters();
		const world::Entity* layerEntity = !layers.empty() ? layers.front()->getEntity() : nullptr;
		const world::World* world = layerEntity ? layerEntity->getWorld() : nullptr;
		NavMeshComponent* navMeshComponent = world ? world->getComponent< NavMeshComponent >() : nullptr;
		if (!navMeshComponent || !navMeshComponent->m_navMesh)
			return;

		const Vector4 eye = primitiveRenderer->getView().inverse().translation().xyz1();
		updateChunks(navMeshComponent, eye);

		T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(navMeshComponent->m_lock);

		const dtNavMesh* nm = navMeshComponent->m_detourNavMesh;
		if (!nm)
			return;

		primitiveRenderer->pushWorld(Matrix44::identity());
		primitiveRenderer->pushDepthState(true, false, false);

		// First pass draw depth tested polygons, second pass draw outlines on top.
		for (int32_t pass = 0; pass < 2; ++pass)
		{
			for (int32_t i = 0; i < nm->getMaxTiles(); ++i)
			{
				const dtMeshTile* tile = nm->getTile(i);
				if (!tile->header)
					continue;

				const float dx = std::max(std::max(tile->header->bmin[0] - eye.x(), eye.x() - tile->header->bmax[0]), 0.0f);
				const float dz = std::max(std::max(tile->header->bmin[2] - eye.z(), eye.z() - tile->header->bmax[2]), 0.0f);
				if (dx * dx + dz * dz > c_drawDistance * c_drawDistance)
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
						const float c_offsetY = 0.25f;
						v[k] = Vector4(p[0], p[1] + c_offsetY, p[2], 1.0f);
					}

					// Modifier areas in orange, disabled areas in red.
					Color4ub color(0, 255, 255, 255);
					if (poly.flags == 0)
						color = Color4ub(255, 0, 0, 255);
					else if (poly.getArea() != RC_WALKABLE_AREA)
						color = Color4ub(255, 160, 0, 255);

					if (pass == 0)
						for (int32_t k = 1; k < poly.vertCount - 1; ++k)
							primitiveRenderer->drawSolidTriangle(v[0], v[k], v[k + 1], Color4ub(color.r, color.g, color.b, 64));
					else
						for (int32_t k = 0; k < poly.vertCount; ++k)
							primitiveRenderer->drawLine(v[k], v[(k + 1) % poly.vertCount], color);
				}
			}
		}

		primitiveRenderer->popDepthState();
		primitiveRenderer->popWorld();
	}
}

void NavMeshComponentEditor::updateChunks(NavMeshComponent* navMeshComponent, const Vector4& eye)
{
	if (navMeshComponent != m_navMeshComponent)
	{
		removeChunks();
		m_navMeshComponent = navMeshComponent;
	}

	// Chunks of a chunked navigation mesh are added within draw distance of camera.
	const NavMesh* navMesh = navMeshComponent->m_navMesh;
	for (int32_t i = 0; i < (int32_t)navMesh->m_chunks.size(); ++i)
	{
		const float x0 = navMesh->m_origin[0] + navMesh->m_chunks[i].x * navMesh->m_chunkWidth;
		const float z0 = navMesh->m_origin[2] + navMesh->m_chunks[i].z * navMesh->m_chunkWidth;
		const float dx = std::max(std::max(x0 - eye.x(), eye.x() - (x0 + navMesh->m_chunkWidth)), 0.0f);
		const float dz = std::max(std::max(z0 - eye.z(), eye.z() - (z0 + navMesh->m_chunkWidth)), 0.0f);
		const bool inside = (dx * dx + dz * dz <= c_drawDistance * c_drawDistance);

		const auto it = m_chunks.find(i);
		if (!inside && it != m_chunks.end())
		{
			navMeshComponent->removeChunk(it->second);
			m_chunks.erase(it);
		}
		else if (inside && it == m_chunks.end())
		{
			resource::Proxy< NavMeshChunk > chunk;
			if (m_context->getResourceManager()->bind(resource::Id< NavMeshChunk >(navMesh->m_chunks[i].id), chunk))
			{
				navMeshComponent->addChunk(chunk);
				m_chunks[i] = chunk.getResource();
			}
		}
	}
}

void NavMeshComponentEditor::removeChunks()
{
	for (const auto& it : m_chunks)
		m_navMeshComponent->removeChunk(it.second);
	m_chunks.clear();
}

}
