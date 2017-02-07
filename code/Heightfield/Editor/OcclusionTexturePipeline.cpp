#include "Core/Functor/Functor.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/GaussianBlurFilter.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/OcclusionLayerAttribute.h"
#include "Heightfield/Editor/OcclusionTextureAsset.h"
#include "Heightfield/Editor/OcclusionTexturePipeline.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

const int32_t c_margin = 12;

struct MeshAndTransform
{
	resource::Id< mesh::IMesh > mesh;
	Transform transform;
};

Ref< ISerializable > resolveAllExternal(editor::IPipelineCommon* pipeline, const ISerializable* object)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const world::ExternalEntityData* externalEntityDataRef = dynamic_type_cast< const world::ExternalEntityData* >(objectMember->get()))
		{
			Ref< const ISerializable > externalEntityData = pipeline->getObjectReadOnly(externalEntityDataRef->getEntityData());
			if (!externalEntityData)
				return 0;

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData));
			if (!resolvedEntityData)
				return 0;

			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());

			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(pipeline, objectMember->get()));
		}
	}

	return reflection->clone();
}

void collectMeshes(const ISerializable* object, AlignedVector< MeshAndTransform >& outMeshes)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (mesh::MeshEntityData* meshEntityData = dynamic_type_cast< mesh::MeshEntityData* >(objectMember->get()))
		{
			MeshAndTransform mat;
			mat.mesh = meshEntityData->getMesh();
			mat.transform = meshEntityData->getTransform();
			outMeshes.push_back(mat);
		}
		else if (world::ComponentEntityData* componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(objectMember->get()))
		{
			mesh::MeshComponentData* meshComponentData = componentEntityData->getComponent< mesh::MeshComponentData >();
			if (meshComponentData)
			{
				MeshAndTransform mat;
				mat.mesh = meshComponentData->getMesh();
				mat.transform = componentEntityData->getTransform();
				outMeshes.push_back(mat);
			}
		}
		else if (world::LayerEntityData* layerEntityData = dynamic_type_cast< world::LayerEntityData* >(objectMember->get()))
		{
			if (!layerEntityData->isInclude())
				continue;

			const OcclusionLayerAttribute* attr = layerEntityData->getAttribute< OcclusionLayerAttribute >();
			if (attr && !attr->trace())
				continue;

			collectMeshes(
				objectMember->get(),
				outMeshes
			);
		}
		else if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(objectMember->get()))
		{
			collectMeshes(
				objectMember->get(),
				outMeshes
			);
		}
	}
}

Vector4 normalAt(const Heightfield* heightfield, int32_t u, int32_t v)
{
	const float c_distance = 0.5f;
	const float directions[][2] =
	{
		{ -c_distance, -c_distance },
		{        0.0f, -c_distance },
		{  c_distance, -c_distance },
		{  c_distance,        0.0f },
		{  c_distance,  c_distance },
		{        0.0f,        0.0f },
		{ -c_distance,  c_distance },
		{ -c_distance,        0.0f }
	};

	float h0 = heightfield->getGridHeightNearest(u, v);

	float h[sizeof_array(directions)];
	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
		h[i] = heightfield->getGridHeightBilinear(u + directions[i][0], v + directions[i][1]);

	const Vector4& worldExtent = heightfield->getWorldExtent();
	float sx = worldExtent.x() / heightfield->getSize();
	float sy = worldExtent.y();
	float sz = worldExtent.z() / heightfield->getSize();

	Vector4 N = Vector4::zero();

	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
	{
		uint32_t j = (i + 1) % sizeof_array(directions);

		float dx1 = directions[i][0] * sx;
		float dy1 = (h[i] - h0) * sy;
		float dz1 = directions[i][1] * sz;

		float dx2 = directions[j][0] * sx;
		float dy2 = (h[j] - h0) * sy;
		float dz2 = directions[j][1] * sz;

		Vector4 n = cross(
			Vector4(dx2, dy2, dz2),
			Vector4(dx1, dy1, dz1)
		);

		N += n;
	}

	return N.normalized();
}

struct TraceTask : public Object
{
	int32_t resolution;
	float maxTraceDistance;
	Vector4 worldExtent;
	Transform transform;
	Ref< const Heightfield > heightfield;
	Ref< const SahTree > tree;
	Ref< drawing::Image > occlusion;
	int32_t x;
	int32_t y;
	int32_t rayCount;

	void execute()
	{
		SahTree::QueryCache cache;
		RandomGeometry rnd;

		const Aabb3& aabb = tree->getBoundingBox();

		Vector4 center = transform * aabb.getCenter().xyz1();
		float extent = aabb.getExtent().length();

		Transform Tinv = transform.inverse();

		float fmnx = (center.x() - extent) / worldExtent.x() + 0.5f;
		float fmnz = (center.z() - extent) / worldExtent.z() + 0.5f;

		float fmxx = (center.x() + extent) / worldExtent.x() + 0.5f;
		float fmxz = (center.z() + extent) / worldExtent.z() + 0.5f;

		int32_t mnx = int32_t(fmnx * resolution);
		int32_t mnz = int32_t(fmnz * resolution);

		int32_t mxx = int32_t(fmxx * resolution);
		int32_t mxz = int32_t(fmxz * resolution);

		int32_t width = mxx - mnx + c_margin * 2 + 1;
		int32_t height = mxz - mnz + c_margin * 2 + 1;

		occlusion = new drawing::Image(drawing::PixelFormat::getR8(), width, height);

		x = mnx - c_margin;
		y = mnz - c_margin;

		for (int32_t ix = mnx - c_margin; ix <= mxx + c_margin; ++ix)
		{
			float fx = (ix + 0.5f) / float(resolution);
			float gx = fx * heightfield->getSize();

			for (int32_t iz = mnz - c_margin; iz <= mxz + c_margin; ++iz)
			{
				float fz = (iz + 0.5f) / float(resolution);
				float gz = fz * heightfield->getSize();

				float gh = heightfield->getGridHeightBilinear(gx, gz);
				float wy = heightfield->unitToWorld(gh);

				float wx, wz;
				heightfield->gridToWorld(gx, gz, wx, wz);

				Vector4 origin = Tinv * Vector4(wx, wy, wz, 1.0f);
				Vector4 direction = Tinv * normalAt(heightfield, gx, gz);
				Vector4 directionHalf = direction * Scalar(0.5f);

				int32_t occluded = 0;
				for (int32_t ii = 0; ii < rayCount; ++ii)
				{
					if (tree->queryAnyIntersection(origin, (rnd.nextHemi(direction) + directionHalf).normalized(), maxTraceDistance, cache))
						++occluded;
				}

				float o = 1.0f - occluded / float(rayCount);

				Color4f c(o, o, o, 1.0f);
				occlusion->setPixel(ix - (mnx - c_margin), iz - (mnz - c_margin), c);
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.OcclusionTexturePipeline", 3, OcclusionTexturePipeline, editor::DefaultPipeline)

bool OcclusionTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_editor = settings->getProperty< PropertyBoolean >(L"Pipeline.TargetEditor", false);
	m_build = settings->getProperty< PropertyBoolean >(L"OcclusionPipeline.Build", true);
	return true;
}

TypeInfoSet OcclusionTexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< OcclusionTextureAsset >());
	return typeSet;
}

bool OcclusionTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const OcclusionTextureAsset* asset = checked_type_cast< const OcclusionTextureAsset* >(sourceAsset);
	pipelineDepends->addDependency(asset->m_occluderData, editor::PdfUse);
	pipelineDepends->addDependency< render::TextureOutput >();
	return true;
}

bool OcclusionTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const OcclusionTextureAsset* asset = checked_type_cast< const OcclusionTextureAsset* >(sourceAsset);

	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getR8(), asset->m_size, asset->m_size);
	image->clear(Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	if (m_build)
	{
		// Get heightfield asset and instance.
		Ref< const db::Instance > heightfieldAssetInstance = pipelineBuilder->getSourceDatabase()->getInstance(asset->m_heightfield);
		if (!heightfieldAssetInstance)
		{
			log::error << L"Heightfield texture pipeline failed; unable to get heightfield asset instance" << Endl;
			return false;
		}

		Ref< const HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const HeightfieldAsset >();
		if (!heightfieldAsset)
		{
			log::error << L"Heightfield texture pipeline failed; unable to get heightfield asset" << Endl;
			return false;
		}

		Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
		if (!sourceData)
		{
			log::error << L"Heightfield pipeline failed; unable to open heights" << Endl;
			return false;
		}

		Ref< Heightfield > heightfield = HeightfieldFormat().read(
			sourceData,
			heightfieldAsset->getWorldExtent()
		);
		if (!heightfield)
		{
			log::error << L"Heightfield pipeline failed; unable to read heights" << Endl;
			return 0;
		}

		sourceData->close();
		sourceData = 0;

		uint32_t size = heightfield->getSize();

		// Extract occluder meshes.
		Ref< const ISerializable > occluderData = pipelineBuilder->getObjectReadOnly(asset->m_occluderData);
		if (!occluderData)
			return false;

		occluderData = resolveAllExternal(pipelineBuilder, occluderData);

		AlignedVector< MeshAndTransform > meshes;
		collectMeshes(occluderData, meshes);

		log::info << L"Found " << int32_t(meshes.size()) << L" mesh(es)" << Endl;

		// Trace occlusion onto heightfield.
		std::map< std::wstring, Ref< SahTree > > treeCache;
		RandomGeometry rnd;

		RefArray< TraceTask > tasks;
		RefArray< Job > jobs;

		for (AlignedVector< MeshAndTransform >::const_iterator i = meshes.begin(); i != meshes.end(); ++i)
		{
			if (ThreadManager::getInstance().getCurrentThread()->stopped())
				break;

			Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(i->mesh);
			if (!meshAsset)
				continue;

			Ref< SahTree > tree;

			std::map< std::wstring, Ref< SahTree > >::const_iterator j = treeCache.find(meshAsset->getFileName().getOriginal());
			if (j != treeCache.end())
				tree = j->second;
			else
			{
				log::info << L"Loading \"" << meshAsset->getFileName().getFileName() << L"\"..." << Endl;

				Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), meshAsset->getFileName().getOriginal());
				if (!file)
				{
					log::warning << L"Unable to open model \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
					continue;
				}

				Ref< model::Model > model = model::ModelFormat::readAny(
					file,
					meshAsset->getFileName().getExtension(),
					model::ModelFormat::IfMeshPositions | model::ModelFormat::IfMeshVertices | model::ModelFormat::IfMeshPolygons
				);
				if (!model)
				{
					log::warning << L"Unable to read model \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
					continue;
				}

				log::info << L"Generating trace tree..." << Endl;

				model::Triangulate().apply(*model);

				const std::vector< model::Polygon >& polygons = model->getPolygons();
				const std::vector< model::Vertex >& vertices = model->getVertices();

				AlignedVector< Winding3 > windings(polygons.size());
				for (uint32_t j = 0; j < polygons.size(); ++j)
				{
					Winding3& w = windings[j];

					const std::vector< uint32_t >& vertexIndices = polygons[j].getVertices();
					for (std::vector< uint32_t >::const_iterator k = vertexIndices.begin(); k != vertexIndices.end(); ++k)
					{
						const model::Vertex& polyVertex = vertices[*k];
						Vector4 polyVertexPosition = model->getPosition(polyVertex.getPosition()).xyz1();
						w.push(polyVertexPosition);
					}
				}

				tree = new SahTree();
				tree->build(windings);

				treeCache[meshAsset->getFileName().getOriginal()] = tree;
			}

			Ref< TraceTask > task = new TraceTask();
			task->resolution = asset->m_size;
			task->maxTraceDistance = asset->m_traceDistance;
			task->worldExtent = heightfieldAsset->getWorldExtent();
			task->transform = i->transform;
			task->heightfield = heightfield;
			task->tree = tree;
			task->x = 0;
			task->y = 0;
			task->rayCount = m_editor ? 16 : 128;

			Ref< Job > job = JobManager::getInstance().add(makeFunctor(task.ptr(), &TraceTask::execute));
			T_ASSERT (job);

			tasks.push_back(task);
			jobs.push_back(job);
		}

		log::info << L"Collecting task(s)..." << Endl;

		for (size_t i = 0; i < jobs.size(); ++i)
		{
			Color4f cd, cs;

			jobs[i]->wait();
			jobs[i] = 0;

			if (!tasks[i]->occlusion)
				continue;

			for (int32_t y = 0; y < tasks[i]->occlusion->getHeight(); ++y)
			{
				for (int32_t x = 0; x < tasks[i]->occlusion->getWidth(); ++x)
				{
					tasks[i]->occlusion->getPixel(x, y, cs);
					image->getPixel(x + tasks[i]->x, y + tasks[i]->y, cd);

					float os = cs.getRed();
					float od = cd.getRed();
					float o = std::min(os, od);

					image->setPixel(x + tasks[i]->x, y + tasks[i]->y, Color4f(o, o, o, 1.0f));
				}
			}
		}

		jobs.clear();
		tasks.clear();

		if (ThreadManager::getInstance().getCurrentThread()->stopped())
		{
			log::info << L"Occlusion texture pipeline terminated; pipeline aborted." << Endl;
			return false;
		}

		if (asset->m_blurRadius > 0)
		{
			Ref< drawing::GaussianBlurFilter > blurFilter = new drawing::GaussianBlurFilter(asset->m_blurRadius);
			image->apply(blurFilter);
		}
	}

	Ref< render::TextureOutput > output = new render::TextureOutput();
	output->m_hasAlpha = false;
	output->m_ignoreAlpha = true;
	output->m_enableCompression = true;
	output->m_linearGamma = true;

	return pipelineBuilder->buildOutput(
		output,
		outputPath,
		outputGuid,
		image
	);
}

	}
}
