#include "Core/Functor/Functor.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldFormat.h"
#include "Heightfield/Editor/OcclusionTextureAsset.h"
#include "Heightfield/Editor/OcclusionTexturePipeline.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/Utilities.h"
#include "Model/Formats/ModelFormat.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

const int32_t c_margin = 16;

template < typename PipelineType >
Ref< ISerializable > resolveAllExternal(PipelineType* pipeline, const ISerializable* object)
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
			//meshEntityData->setTransform(transform);
			outMeshEntityData.push_back(meshEntityData);
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

	void execute()
	{
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
			for (int32_t iz = mnz - c_margin; iz <= mxz + c_margin; ++iz)
			{
				float fx = (ix + 0.5f) / float(resolution);
				float fz = (iz + 0.5f) / float(resolution);

				float gx = fx * heightfield->getSize();
				float gz = fz * heightfield->getSize();

				float gh = heightfield->getGridHeightBilinear(gx, gz);
				float wy = heightfield->unitToWorld(gh);

				float wx, wz;
				heightfield->gridToWorld(gx, gz, wx, wz);

				Vector4 origin = Tinv * Vector4(wx, wy, wz, 1.0f);
				Vector4 direction = Tinv * Vector4(0.0f, 1.0f, 0.0f, 0.0f);

				int32_t occluded = 0;
				for (int32_t ii = 0; ii < 64; ++ii)
				{
					if (tree->queryAnyIntersection(origin, (rnd.nextHemi(direction) + direction * Scalar(0.5f)).normalized(), maxTraceDistance))
						++occluded;
				}

				float o = 1.0f - occluded / 64.0f;

				Color4f c(o, 0.0f, 0.0f, 1.0f);
				occlusion->setPixel(ix - (mnx - c_margin), iz - (mnz - c_margin), c);
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.OcclusionTexturePipeline", 0, OcclusionTexturePipeline, editor::DefaultPipeline)

bool OcclusionTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
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
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	const OcclusionTextureAsset* asset = checked_type_cast< const OcclusionTextureAsset* >(sourceAsset);
	pipelineDepends->addDependency(asset->m_occluderData, editor::PdfUse);
	pipelineDepends->addDependency< render::TextureOutput >();
	return true;
}

bool OcclusionTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const OcclusionTextureAsset* asset = checked_type_cast< const OcclusionTextureAsset* >(sourceAsset);

	// Get heightfield asset.
	Ref< const HeightfieldAsset > heightfieldAsset = pipelineBuilder->getObjectReadOnly< HeightfieldAsset >(asset->m_heightfield);
	if (!heightfieldAsset)
	{
		log::error << L"Heightfield texture pipeline failed; unable to read heightfield asset" << Endl;
		return false;
	}

	// Load heightfield from source file.
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, heightfieldAsset->getFileName());
	Ref< Heightfield > heightfield = HeightfieldFormat().read(
		fileName,
		heightfieldAsset->getWorldExtent(),
		heightfieldAsset->getInvertX(),
		heightfieldAsset->getInvertZ(),
		heightfieldAsset->getDetailSkip()
	);
	if (!heightfield)
	{
		log::error << L"Unable to read heightfield source \"" << fileName.getPathName() << L"\"" << Endl;
		return 0;
	}

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

		Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, meshAsset->getFileName());
		std::map< std::wstring, Ref< SahTree > >::const_iterator j = treeCache.find(fileName.getPathName());
		if (j != treeCache.end())
			tree = j->second;
		else
		{
			log::info << L"Loading \"" << fileName.getFileName() << L"\"..." << Endl;

			Ref< model::Model > model = model::ModelFormat::readAny(fileName, model::ModelFormat::IfMeshPositions | model::ModelFormat::IfMeshVertices | model::ModelFormat::IfMeshPolygons);
			if (!model)
			{
				log::warning << L"Unable to read model \"" << fileName.getPathName() << L"\"" << Endl;
				continue;
			}

			log::info << L"Generating trace tree..." << Endl;

			model::triangulateModel(*model);

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
					w.points.push_back(polyVertexPosition);
				}
			}

			tree = new SahTree();
			tree->build(windings);

			treeCache[fileName.getPathName()] = tree;
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

		Ref< Job > job = JobManager::getInstance().add(makeFunctor(task.ptr(), &TraceTask::execute));
		T_ASSERT (job);

		tasks.push_back(task);
		jobs.push_back(job);


		//const Aabb3& aabb = tree->getBoundingBox();

		//Vector4 center = (*i)->getTransform() * aabb.getCenter().xyz1();
		//float extent = aabb.getExtent().length();

		//Transform Tinv = (*i)->getTransform().inverse();

		//float fmnx = (center.x() - extent) / heightfieldAsset->getWorldExtent().x() + 0.5f;
		//float fmnz = (center.z() - extent) / heightfieldAsset->getWorldExtent().z() + 0.5f;

		//float fmxx = (center.x() + extent) / heightfieldAsset->getWorldExtent().x() + 0.5f;
		//float fmxz = (center.z() + extent) / heightfieldAsset->getWorldExtent().z() + 0.5f;

		//int32_t mnx = int32_t(fmnx * image->getWidth());
		//int32_t mnz = int32_t(fmnz * image->getHeight());

		//int32_t mxx = int32_t(fmxx * image->getWidth());
		//int32_t mxz = int32_t(fmxz * image->getHeight());

		//for (int32_t ix = mnx - c_margin; ix <= mxx + c_margin; ++ix)
		//{
		//	for (int32_t iz = mnz - c_margin; iz <= mxz + c_margin; ++iz)
		//	{
		//		float fx = (ix + 0.5f) / float(image->getWidth());
		//		float fz = (iz + 0.5f) / float(image->getHeight());

		//		float gx = fx * heightfield->getSize();
		//		float gz = fz * heightfield->getSize();

		//		float gh = heightfield->getGridHeightBilinear(gx, gz);
		//		float wy = heightfield->unitToWorld(gh);

		//		float wx, wz;
		//		heightfield->gridToWorld(gx, gz, wx, wz);

		//		Vector4 origin = Tinv * Vector4(wx, wy, wz, 1.0f);
		//		Vector4 direction = Tinv * Vector4(0.0f, 1.0f, 0.0f, 0.0f);

		//		int32_t occluded = 0;
		//		for (int32_t ii = 0; ii < 64; ++ii)
		//		{
		//			if (tree->queryAnyIntersection(origin, (rnd.nextHemi(direction) + direction * Scalar(0.5f)).normalized(), maxTraceDistance))
		//				++occluded;
		//		}

		//		float o = 1.0f - occluded / 64.0f;

		//		Color4f clr(1.0f, 1.0f, 1.0f, 1.0f);
		//		image->getPixel(ix, image->getHeight() - iz - 1, clr);

		//		clr = clr * Color4f(o, o, o, 1.0f);
		//		image->setPixel(ix, image->getHeight() - iz - 1, clr);
		//	}
		//}
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

	Ref< drawing::ConvolutionFilter > blurFilter = drawing::ConvolutionFilter::createGaussianBlur();
	image = image->applyFilter(blurFilter);

	drawing::MirrorFilter mirrorFilter(false, true);
	image = image->applyFilter(&mirrorFilter);

	image->save(L"data/Temp/Occlusion.png");

	render::TextureOutput output;
	output.m_hasAlpha = false;
	output.m_ignoreAlpha = true;
	output.m_enableCompression = true;
	output.m_linearGamma = true;

	return pipelineBuilder->buildOutput(
		&output,
		image,
		outputPath,
		outputGuid
	);
}

	}
}
