#include "Core/Math/BspTree.h"
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

AlignedVector< Winding3 > transform(const AlignedVector< Winding3 >& windings, const Transform& transform)
{
    AlignedVector< Winding3 > result(windings.size());
    for (uint32_t i = 0; i < windings.size(); ++i)
    {
        const auto& sw = windings[i];
        auto& dw = result[i];

        dw.resize(sw.size());
        for (uint32_t j = 0; j < sw.size(); ++j)
            dw[j] = transform * sw[j].xyz1();
    }
    return result;
}

AlignedVector< Winding3 > invert(const AlignedVector< Winding3 >& windings)
{
    AlignedVector< Winding3 > result(windings.size());
    for (uint32_t i = 0; i < windings.size(); ++i)
    {
        result[i] = windings[i];
        result[i].flip();
    }
    return result;
}

// A | B
AlignedVector< Winding3 > unioon(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
    AlignedVector< Winding3 > result;

	// Clip all A to tree B.
    {
        BspTree treeB(windingsB);
        for (const auto& wa : windingsA)
        {
            treeB.clip(wa, [&](uint32_t index, const Winding3& w, uint32_t cl, bool splitted)
            {
                if (w.size() >= 3 && cl == Winding3::CfFront)
                    result.push_back(w);
            });
        }
    }

	// Clip all B to tree A.
    {
        BspTree treeA(windingsA);
        for (const auto& wb : windingsB)
        {
            treeA.clip(wb, [&](uint32_t index, const Winding3& w, uint32_t cl, bool splitted)
            {
                if (w.size() >= 3 && cl == Winding3::CfFront)
                    result.push_back(w);
            });
        }
    }

    return result;
}

// A & B == ~(~A | ~B)
AlignedVector< Winding3 > intersection(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
    auto invWindingsA = invert(windingsA);
    auto invWindingsB = invert(windingsB);
    auto invResult = unioon(invWindingsA, invWindingsB);
    return invert(invResult);
}

// A - B == ~(~A | B)
AlignedVector< Winding3 > difference(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
    auto invWindingsA = invert(windingsA);
    auto invResult = unioon(invWindingsA, windingsB);
    return invert(invResult);
}

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
        for (const auto& w : m_windings)
        {
            Plane pl;
            if (!w.getPlane(pl))
                continue;

            AlignedVector< Triangulator::Triangle > triangles;
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
                orthogonalFrame(normal, fu, fv);

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
