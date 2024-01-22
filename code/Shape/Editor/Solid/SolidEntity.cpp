/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Triangulator.h"
#include "Core/Misc/SafeDestroy.h"
#include "Model/Model.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/Entity/GroupComponent.h"

// https://github.com/evanw/csg.js/blob/master/csg.js

namespace traktor
{
    namespace shape
    {
        namespace
        {

#pragma pack(1)
struct Vertex
{
	float position[3];
	float normal[3];
	float texCoord[2];
};
#pragma pack()

        }

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntity", SolidEntity, world::Entity)

SolidEntity::SolidEntity(
    render::IRenderSystem* renderSystem,
    const resource::Proxy< render::Shader >& shader,
    const Transform& transform
)
:	m_renderSystem(renderSystem)
,	m_shader(shader)
,	m_dirty(true)
{
}

void SolidEntity::update(const world::UpdateParams& update)
{
    world::Entity::update(update);

	auto group = getComponent< world::GroupComponent >();
	if (!group)
		return;

    RefArray< PrimitiveEntity > primitiveEntities;
	for (auto entity : group->getEntities())
	{
		if (is_a< PrimitiveEntity >(entity))
			primitiveEntities.push_back(checked_type_cast< PrimitiveEntity* >(entity));
	}

    // Check if any child entity is dirty and if so update our preview geometry.
    bool dirty = m_dirty;
    for (auto primitiveEntity : primitiveEntities)
    {
        dirty |= primitiveEntity->isDirty();
        primitiveEntity->resetDirty();
    }
    if (dirty)
    {
		model::Model current;

        auto it = primitiveEntities.begin();
        if (it != primitiveEntities.end())
        {
			auto model = (*it)->getModel();
			if (!model)
				return;

			current = *model;
			model::Transform((*it)->getTransform().toMatrix44()).apply(current);

            for (++it; it != primitiveEntities.end(); ++it)
            {
				auto other = (*it)->getModel();
				if (!other)
					continue;

				model::Model result;

                switch ((*it)->getData()->getOperation())
                {
                case BooleanOperation::Union:
                    {
						model::Boolean(
							current,
							Transform::identity(),
							*other,
							(*it)->getTransform(),
							model::Boolean::BoUnion
						).apply(result);
                    }
                    break;

                case BooleanOperation::Intersection:
                    {
						model::Boolean(
							current,
							Transform::identity(),
							*other,
							(*it)->getTransform(),
							model::Boolean::BoIntersection
						).apply(result);
					}
                    break;

                case BooleanOperation::Difference:
                    {
						model::Boolean(
							current,
							Transform::identity(),
							*other,
							(*it)->getTransform(),
							model::Boolean::BoDifference
						).apply(result);
					}
                    break;
                }

				current = std::move(result);
				model::CleanDegenerate().apply(current);
				model::MergeCoplanarAdjacents().apply(current);
            }
        }

		model::Triangulate().apply(current);

        const uint32_t nvertices = current.getVertexCount();
		const uint32_t nindices = current.getPolygonCount() * 3;

        if (nvertices > 0 && nindices > 0)
        {
			if (m_vertexBuffer == nullptr || m_vertexBuffer->getBufferSize() < nvertices * sizeof(Vertex))
			{
				safeDestroy(m_vertexBuffer);

				AlignedVector< render::VertexElement > vertexElements;
				vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat3, offsetof(Vertex, position)));
				vertexElements.push_back(render::VertexElement(render::DataUsage::Normal, render::DtFloat3, offsetof(Vertex, normal)));
				vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(Vertex, texCoord)));
				m_vertexLayout = m_renderSystem->createVertexLayout(vertexElements);

				m_vertexBuffer = m_renderSystem->createBuffer(
					render::BuVertex,
					(nvertices + 4 * 128) * sizeof(Vertex),
					false
				);
			}

            Vertex* vertex = (Vertex*)m_vertexBuffer->lock();
			for (const auto& v : current.getVertices())
            {
				Vector4 p = current.getPosition(v.getPosition());
				Vector4 n = (v.getNormal() != model::c_InvalidIndex) ? current.getNormal(v.getNormal()) : Vector4::zero();
				Vector2 uv = (v.getTexCoord(0) != model::c_InvalidIndex) ? current.getTexCoord(v.getTexCoord(0)) : Vector2::zero();

				p.storeUnaligned(vertex->position);
				n.storeUnaligned(vertex->normal);

				vertex->texCoord[0] = uv.x;
				vertex->texCoord[1] = uv.y;
                    
				++vertex;
            }
            m_vertexBuffer->unlock();

            // Create indices and material batches.
			if (m_indexBuffer == nullptr || m_indexBuffer->getBufferSize() < nindices * sizeof(uint16_t))
			{
				safeDestroy(m_indexBuffer);
            	m_indexBuffer = m_renderSystem->createBuffer(render::BuIndex, (nindices + 3 * 128) * sizeof(uint16_t), false);
			}

            uint16_t* index = (uint16_t*)m_indexBuffer->lock();
			uint32_t offset = 0;
			for (uint32_t i = 0; i < current.getMaterialCount(); ++i)
			{
				uint32_t count = 0;
				for (const auto& p : current.getPolygons())
				{
					if (p.getMaterial() == i)
					{
						*index++ = (uint16_t)p.getVertex(0);
						*index++ = (uint16_t)p.getVertex(1);
						*index++ = (uint16_t)p.getVertex(2);
						++count;
					}
				}

				if (!count)
					continue;

				auto& batch = m_batches.push_back();
				batch.primitives.setIndexed(
					render::PrimitiveType::Triangles,
					offset,
					count,
					0,
					nvertices - 1
				);

				offset += count * 3;
			}
            m_indexBuffer->unlock();
        }
		else
		{
			safeDestroy(m_vertexBuffer);
			safeDestroy(m_indexBuffer);			
		}

		m_dirty = false;
    }
}

void SolidEntity::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_indexBuffer || !m_vertexBuffer)
		return;

	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;

	auto renderContext = context.getRenderContext();
	for (const auto& batch : m_batches)
	{
		auto renderBlock = renderContext->allocNamed< render::SimpleRenderBlock >(L"Solid");
		renderBlock->distance = std::numeric_limits< float >::max();
		renderBlock->program = sp.program;
		renderBlock->indexBuffer = m_indexBuffer->getBufferView();
		renderBlock->indexType = render::IndexType::UInt16;
		renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->primitives = batch.primitives;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams
		);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			sp.priority,
			renderBlock
		);
	}
}

    }
}
