#include "Core/Log/Log.h"
#include "Core/Math/Triangulator.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/Utilities.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntity", SolidEntity, world::GroupEntity)

SolidEntity::SolidEntity(
    render::IRenderSystem* renderSystem,
    const resource::Proxy< render::Shader >& shader,
    const Transform& transform
)
:   world::GroupEntity(transform)
,   m_renderSystem(renderSystem)
,   m_shader(shader)
{
}

void SolidEntity::update(const world::UpdateParams& update)
{
    world::GroupEntity::update(update);

    RefArray< PrimitiveEntity > primitiveEntities;
    getEntitiesOf< PrimitiveEntity >(primitiveEntities);

    // Check if any child entity is dirty and if so update our preview geometry.
    bool dirty = false;
    for (auto primitiveEntity : primitiveEntities)
    {
        dirty |= primitiveEntity->isDirty();
        primitiveEntity->resetDirty();
    }
    if (dirty)
    {
        m_windings.resize(0);

        auto it = primitiveEntities.begin();
        if (it != primitiveEntities.end())
        {
            m_windings = transform((*it)->getWindings(), (*it)->getTransform());
            for (++it; it != primitiveEntities.end(); ++it)
            {
                auto windings = transform((*it)->getWindings(), (*it)->getTransform());
				if (windings.empty())
					continue;

                switch ((*it)->getOperation())
                {
                case BooleanOperation::BoUnion:
                    {
                        auto result = unioon(m_windings, windings);
                        m_windings.swap(result);
                    }
                    break;

                case BooleanOperation::BoIntersection:
                    {
                        auto result = intersection(m_windings, windings);
                        m_windings.swap(result);
                    }
                    break;

                case BooleanOperation::BoDifference:
                    {
                        auto result = difference(m_windings, windings);
                        m_windings.swap(result);
                    }
                    break;
                }
            }
        }

        // Triangulate all windings.
        AlignedVector< Winding3 > triangulated;
		AlignedVector< Triangulator::Triangle > triangles;
        for (const auto& w : m_windings)
        {
            Plane pl;
            if (!w.getPlane(pl))
                continue;

			triangles.resize(0);
            Triangulator().freeze(
                w.get(),
                pl.normal(),
                triangles
            );
            for (const auto& t : triangles)
            {
                auto& tw = triangulated.push_back();
                tw.resize(3);
                tw[0] = w[t.indices[0]];
                tw[1] = w[t.indices[1]];
                tw[2] = w[t.indices[2]];
            }
        }

        safeDestroy(m_vertexBuffer);
        safeDestroy(m_indexBuffer);

        const uint32_t ntriangles = triangulated.size();
        const uint32_t nvertices = ntriangles * 3;

        if (ntriangles > 0)
        {
            // Create vertices.
            AlignedVector< render::VertexElement > vertexElements;
            vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(Vertex, position)));
            vertexElements.push_back(render::VertexElement(render::DuNormal, render::DtFloat3, offsetof(Vertex, normal)));
            vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord)));

            m_vertexBuffer = m_renderSystem->createVertexBuffer(
                vertexElements,
                nvertices * sizeof(Vertex),
                false
            );

            Vertex* vertex = (Vertex*)m_vertexBuffer->lock();
            for (const auto& w : triangulated)
            {
                Plane pl;
                w.getPlane(pl);

                Vector4 normal = pl.normal();

                Vector4 fu, fv;
				switch (majorAxis3(normal))
				{
				case 0:
					fu = Vector4(0.0f, 0.0f, 1.0f);
					fv = Vector4(0.0f, 1.0f, 0.0f);
					break;
				case 1:
					fu = Vector4(1.0f, 0.0f, 0.0f);
					fv = Vector4(0.0f, 0.0f, 1.0f);
					break;
				case 2:
					fu = Vector4(1.0f, 0.0f, 0.0f);
					fv = Vector4(0.0f, 1.0f, 0.0f);
					break;
				}

                for (int32_t i = 0; i < 3; ++i)
                {
                    w[i].storeUnaligned(vertex->position);
                    normal.storeUnaligned(vertex->normal);

                    vertex->texCoord[0] = dot3(fu, w[i]);
                    vertex->texCoord[1] = dot3(fv, w[i]);
                    
                    ++vertex;
                }
            }
            m_vertexBuffer->unlock();

            // Create indices.
            m_indexBuffer = m_renderSystem->createIndexBuffer(render::ItUInt16, ntriangles * 3 * sizeof(uint16_t), false);

            uint16_t* index = (uint16_t*)m_indexBuffer->lock();
            for (uint32_t i = 0; i < ntriangles * 3; ++i)
                *index++ = i;
            m_indexBuffer->unlock();

            // Create primitives.
            m_primitives.setIndexed(
                render::PtTriangles,
                0,
                ntriangles,
                0,
                nvertices - 1
            );
        }
    }
}

void SolidEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_indexBuffer || !m_vertexBuffer)
		return;

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	auto renderContext = worldContext.getRenderContext();

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Solid");

	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = program;
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(
		renderBlock->programParams
	);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(m_shader->getCurrentPriority(), renderBlock);
}

    }
}
