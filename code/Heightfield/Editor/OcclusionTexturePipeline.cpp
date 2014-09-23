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
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/OcclusionTextureAsset.h"
#include "Heightfield/Editor/OcclusionTexturePipeline.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

const int32_t c_margin = 12;

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

void collectMeshEntities(const ISerializable* object, const Transform& transform, RefArray< mesh::MeshEntityData >& outMeshEntityData)
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
			outMeshEntityData.push_back(meshEntityData);
		}
		else if (world::LayerEntityData* layerEntityData = dynamic_type_cast< world::LayerEntityData* >(objectMember->get()))
		{
			if (layerEntityData->isDynamic())
				continue;

			collectMeshEntities(
				objectMember->get(),
				layerEntityData->getTransform(),
				outMeshEntityData
			);
		}
		else if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(objectMember->get()))
		{
			collectMeshEntities(
				objectMember->get(),
				entityData->getTransform(),
				outMeshEntityData
			);
		}
	}
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

		occlusion = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), width, height);

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
				Vector4 direction = Tinv * Vector4(0.0f, 1.0f, 0.0f, 0.0f);
				Vector4 directionHalf = direction * Scalar(0.5f);

				int32_t occluded = 0;
				for (int32_t ii = 0; ii < rayCount; ++ii)
				{
					if (tree->queryAnyIntersection(origin, (rnd.nextHemi(direction) + directionHalf).normalized(), maxTraceDistance, cache))
						++occluded;
				}

				float o = 1.0f - occluded / float(rayCount);

				Color4f c(0.4f + o * 0.6f, 0.0f, 0.0f, 1.0f);
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

	RefArray< mesh::MeshEntityData > meshEntityData;
	collectMeshEntities(occluderData, Transform::identity(), meshEntityData);

	log::info << L"Found " << int32_t(meshEntityData.size()) << L" mesh(es)" << Endl;

	// Trace occlusion onto heightfield.
	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), 1024, 1024);
	image->clear(Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	std::map< std::wstring, Ref< SahTree > > treeCache;
	RandomGeometry rnd;

	float maxTraceDistance = c_margin * heightfieldAsset->getWorldExtent().x() / image->getWidth();
	log::info << L"Tracing, max distance = " << maxTraceDistance << L" unit(s)" << Endl;

	RefArray< TraceTask > tasks;
	RefArray< Job > jobs;

	for (RefArray< mesh::MeshEntityData >::const_iterator i = meshEntityData.begin(); i != meshEntityData.end(); ++i)
	{
		const resource::Id< mesh::IMesh >& mesh = (*i)->getMesh();

		Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(mesh);
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
		task->resolution = image->getWidth();
		task->maxTraceDistance = maxTraceDistance;
		task->worldExtent = heightfieldAsset->getWorldExtent();
		task->transform = (*i)->getTransform();
		task->heightfield = heightfield;
		task->tree = tree;
		task->x = 0;
		task->y = 0;
		task->rayCount = m_editor ? 16 : 64;

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
				image->setPixel(x + tasks[i]->x, y + tasks[i]->y, cd * cs);
			}
		}
	}

	jobs.clear();
	tasks.clear();

	log::info << L"Blurring occlusion mask..." << Endl;

	Ref< drawing::ConvolutionFilter > blurFilter = drawing::ConvolutionFilter::createGaussianBlur3();
	image->apply(blurFilter);

	drawing::MirrorFilter mirrorFilter(false, true);
	image->apply(&mirrorFilter);

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
