#include "Core/Misc/SafeDestroy.h"
#include "Model/Model.h"
#include "Model/Operations/MergeModel.h"
//#include "Model/Operations/Triangulate.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/SplineEntity.h"
#include "Shape/Editor/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/Entity/GroupComponent.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineEntity", SplineEntity, world::Entity)

SplineEntity::SplineEntity(
	const SplineEntityData* data,
	db::Database* database,
    render::IRenderSystem* renderSystem,
	model::ModelCache* modelCache,
	const std::wstring& assetPath,
    const resource::Proxy< render::Shader >& shader
)
:	m_data(data)
,	m_database(database)
,	m_renderSystem(renderSystem)
,	m_modelCache(modelCache)
,	m_assetPath(assetPath)
,	m_shader(shader)
,	m_dirty(true)
{
}

void SplineEntity::update(const world::UpdateParams& update)
{
	world::Entity::update(update);

	// Fetch group component; contain all control point entities.
	auto group = getComponent< world::GroupComponent >();
	if (!group)
		return;

	// Get control points.
	RefArray< ControlPointComponent > controlPoints;
	for (auto entity : group->getEntities())
	{
		auto controlPoint = entity->getComponent< ControlPointComponent >();
		if (controlPoint)
			controlPoints.push_back(controlPoint);
	}

	// Check if any control point is dirty.
	bool controlPointsDirty = m_dirty;
	for (auto controlPoint : controlPoints)
	{
		if (controlPoint->checkDirty())
		{
			controlPointsDirty |= true;
			break;
		}
	}

	// Update transform path if any control point is dirty.
	if (controlPointsDirty)
	{
		m_path = TransformPath();
		for (uint32_t i = 0; i < controlPoints.size(); ++i)
		{
			Transform T = controlPoints[i]->getTransform();

			TransformPath::Key k;
			k.T = (float)i / (controlPoints.size() - 1);
			k.position = T.translation();
			k.orientation = T.rotation().toEulerAngles();
			k.values[0] = controlPoints[i]->getScale();
			m_path.insert(k);
		}

		// Generate geometry from path.
		Ref< model::Model > outputModel;
		for (auto component : m_data->getComponents())
		{
			if (const auto layerData = dynamic_type_cast< const SplineLayerComponentData* >(component))
			{
				Ref< model::Model > layerModel = layerData->createModel(m_database, m_modelCache, m_assetPath, m_path);
				if (!layerModel)
					continue;

				if (outputModel)
				{
					model::MergeModel merge(*layerModel, Transform::identity(), 0.01f);
					merge.apply(*outputModel);
				}
				else
					outputModel = layerModel;
			}
		}

		// In case no layers has been added yet.
		if (!outputModel)
		{
			safeDestroy(m_vertexBuffer);
			safeDestroy(m_indexBuffer);	
			m_dirty = false;
			return;
		}

		//// Create runtime render mesh from model.
		//model::Triangulate().apply(*outputModel);

		m_batches.resize(0);

        const uint32_t nvertices = outputModel->getVertexCount();
		const uint32_t nindices = outputModel->getPolygonCount() * 3;

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
					nvertices + 4 * 128,
					sizeof(Vertex),
					false
				);
			}

            Vertex* vertex = (Vertex*)m_vertexBuffer->lock();
			for (const auto& v : outputModel->getVertices())
            {
				Vector4 p = outputModel->getPosition(v.getPosition());
				Vector4 n = (v.getNormal() != model::c_InvalidIndex) ? outputModel->getNormal(v.getNormal()) : Vector4::zero();
				Vector2 uv = (v.getTexCoord(0) != model::c_InvalidIndex) ? outputModel->getTexCoord(v.getTexCoord(0)) : Vector2::zero();

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
            	m_indexBuffer = m_renderSystem->createBuffer(render::BuIndex, nindices + 3 * 128, sizeof(uint16_t), false);
			}

            uint16_t* index = (uint16_t*)m_indexBuffer->lock();
			uint32_t offset = 0;
			for (uint32_t i = 0; i < outputModel->getMaterialCount(); ++i)
			{
				uint32_t count = 0;
				for (const auto& p : outputModel->getPolygons())
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

void SplineEntity::build(
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
		auto renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"Solid");
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
