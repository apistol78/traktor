/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include <fbxsdk.h>
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Model/Model.h"
#include "Model/Formats/Fbx/Conversion.h"
#include "Model/Formats/Fbx/IStreamWrapper.h"
#include "Model/Formats/Fbx/MeshConverter.h"
#include "Model/Formats/Fbx/ModelFormatFbx.h"
#include "Model/Formats/Fbx/SkeletonConverter.h"

namespace traktor::model
{
	namespace
	{

Semaphore s_fbxLock;
FbxManager* s_fbxManager = nullptr;
FbxIOSettings* s_ioSettings = nullptr;
FbxScene* s_scene = nullptr;

bool include(FbxNode* node, const std::wstring& filter)
{
	if (!node)
		return false;

	// Always accept root nodes.
	if (!node->GetParent())
		return true;

	// Only filter on first level of child nodes.
	if (node->GetParent()->GetParent())
		return true;

	// Empty filter means everything is included.
	if (filter.empty())
		return true;

	// Check if node's name match filter tag.
	const std::wstring name = mbstows(node->GetName());
	for (auto s : StringSplit< std::wstring >(filter, L",;"))
	{
		if (s.front() != L'!')
		{
			if (s == name)
				return true;
		}
		else
		{
			if (s.substr(1) != name)
				return true;
		}
	}

	return false;
}

FbxNode* search(FbxNode* node, const std::wstring& filter, const std::function< bool (FbxNode* node) >& predicate)
{
	if (!include(node, filter))
		return nullptr;

	if (predicate(node))
		return node;

	const int32_t childCount = node->GetChildCount();
	for (int32_t i = 0; i < childCount; ++i)
	{
		FbxNode* childNode = node->GetChild(i);
		if (childNode)
		{
			FbxNode* foundNode = search(childNode, filter, predicate);
			if (foundNode)
				return foundNode;
		}
	}

	return nullptr;
}

bool traverse(FbxNode* node, const std::wstring& filter, const std::function< bool (FbxNode* node) >& visitor)
{
	if (!include(node, filter))
		return true;

	if (!visitor(node))
		return false;

	const int32_t childCount = node->GetChildCount();
	for (int32_t i = 0; i < childCount; ++i)
	{
		FbxNode* childNode = node->GetChild(i);
		if (childNode)
		{
			if (!traverse(childNode, filter, visitor))
				return false;
		}
	}

	return true;
}

void dump(FbxNode* node)
{
	log::info << L"Node \"" << mbstows(node->GetName()) << L"\" " << uint64_t(node) << Endl;

	if (node->GetNodeAttribute())
	{
		FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();
		log::info << L"Attribute type " << int32_t(attributeType) << Endl;

		if (attributeType == FbxNodeAttribute::eMesh)
		{
			log::info << L"Mesh" << Endl;

			FbxMesh* mesh = static_cast< FbxMesh* >(node->GetNodeAttribute());

			const int32_t deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
			log::info << L"Deformer count " << deformerCount << Endl;

			for (int32_t i = 0; i < deformerCount; ++i)
			{
				FbxSkin* skinDeformer = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
				if (!skinDeformer)
					continue;

				const int32_t clusterCount = skinDeformer->GetClusterCount();
				log::info << L"deformer[" << i << L"].cluserCount " << clusterCount << Endl;

				for (int32_t j = 0; j < clusterCount; ++j)
				{
					FbxCluster* cluster = skinDeformer->GetCluster(j);
					if (!cluster)
						continue;

					const FbxNode* jointNode = cluster->GetLink();
					T_ASSERT(jointNode);

					log::info << L"deformer[" << i << L"].cluser[" << j << L"].joint = " << uint64_t(jointNode) << Endl;
				}
			}
		}
	}

	if (node->GetVisibility())
		log::info << L"Visible" << Endl;
	else
		log::info << L"Hidden" << Endl;

	const int32_t childCount = node->GetChildCount();
	for (int32_t i = 0; i < childCount; ++i)
	{
		FbxNode* childNode = node->GetChild(i);
		if (!childNode)
			continue;

		log::info << L"Child " << i << L":" << Endl;
		log::info << IncreaseIndent;

		dump(childNode);

		log::info << DecreaseIndent;
	}
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatFbx", 0, ModelFormatFbx, ModelFormat)

void ModelFormatFbx::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Autodesk FBX";
	outExtensions.push_back(L"fbx");
}

bool ModelFormatFbx::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"fbx") == 0;
}

Ref< Model > ModelFormatFbx::read(const Path& filePath, const std::wstring& filter) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_fbxLock);

	if (!s_fbxManager)
	{
		s_fbxManager = FbxManager::Create();
		if (!s_fbxManager)
		{
			log::error << L"Unable to import FBX model; failed to create FBX SDK instance." << Endl;
			return nullptr;
		}

		s_ioSettings = FbxIOSettings::Create(s_fbxManager, IOSROOT);
		s_fbxManager->SetIOSettings(s_ioSettings);

		s_scene = FbxScene::Create(s_fbxManager, "");
	}

	FbxIOPluginRegistry* registry = s_fbxManager->GetIOPluginRegistry();
	int readerID = registry->FindReaderIDByExtension("fbx");
    if (readerID < 0)
	{
		log::error << L"Unable to import FBX model; no reader for \"fbx\" extension registered." << Endl;
		return nullptr;
	}

	FbxImporter* importer = FbxImporter::Create(s_fbxManager, "");
	if (!importer)
	{
		log::error << L"Unable to import FBX model; failed to create FBX importer instance." << Endl;
		return nullptr;
	}

	// Ensure embedded resources are extracted at a known location.
	const Path embeddedPath(OS::getInstance().getWritableFolderPath() + L"/Traktor/Fbx");
	FileSystem::getInstance().makeAllDirectories(embeddedPath);
	importer->SetEmbeddingExtractionFolder(wstombs(embeddedPath.getPathNameOS()).c_str());

	Ref< IStream > stream = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!stream)
		return nullptr;

	// Wrap source stream into a buffered stream if necessary as
	// FBX keep reading very small chunks.
	if (!is_a< BufferedStream >(stream))
		stream = new BufferedStream(stream);

	AutoPtr< FbxStream > fbxStream(new IStreamWrapper());
	bool status = importer->Initialize(fbxStream.ptr(), stream, readerID, s_fbxManager->GetIOSettings());
	if (!status)
	{
		log::error << L"Unable to import FBX model; failed to initialize FBX importer (" << mbstows(importer->GetStatus().GetErrorString()) << L")." << Endl;
		return nullptr;
	}

	s_scene->Clear();

	status = importer->Import(s_scene);
	if (!status)
	{
		log::error << L"Unable to import FBX model; FBX importer failed (" << mbstows(importer->GetStatus().GetErrorString()) << L")." << Endl;
		return nullptr;
	}

	FbxNode* rootNode = s_scene->GetRootNode();
	if (!rootNode)
		return nullptr;

#if defined(_DEBUG)
	// Dump fbx hierarchy.
	dump(rootNode);
#endif

	Matrix44 axisTransform = calculateAxisTransform(
		s_scene->GetGlobalSettings().GetAxisSystem()
	);

	Ref< Model > model = new Model();
	AlignedVector< std::string > channels;
	bool result = true;

	// Convert skeleton and animations.
	FbxNode* skeletonNode = search(rootNode, filter, [&](FbxNode* node) {
		return (node->GetNodeAttribute() != nullptr && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton);
	});
	if (skeletonNode)
	{
		if (!convertSkeleton(*model, s_scene, skeletonNode, axisTransform))
			return nullptr;

		for(int32_t i = 0; i < importer->GetAnimStackCount(); i++)
		{
			FbxAnimStack* animStack = s_scene->GetSrcObject< FbxAnimStack >(i);
			if (!animStack)
				continue;

			s_scene->SetCurrentAnimationStack(animStack);

			std::wstring takeName = mbstows(animStack->GetName());
			size_t p = takeName.find(L'|');
			if (p != std::wstring::npos)
				takeName = takeName.substr(p + 1);

			FbxTime start = animStack->LocalStart;
			FbxTime end = animStack->LocalStop;

			int32_t startFrame = start.GetFrameCount(FbxTime::eFrames30);
			int32_t endFrame = end.GetFrameCount(FbxTime::eFrames30);

			Ref< Animation > anim = new Animation();
			anim->setName(takeName);
			for (int32_t frame = startFrame; frame <= endFrame; ++frame)
			{
				FbxTime time;
				time.SetFrame(frame, FbxTime::eFrames30);

				Ref< Pose > pose = convertPose(*model, s_scene, skeletonNode, time, axisTransform);
				if (!pose)
					return nullptr;

				anim->insertKeyFrame(frame / 30.0f, pose);
			}
			model->addAnimation(anim);
		}
	}

	// Convert and merge all meshes.
	result &= traverse(rootNode, filter, [&](FbxNode* node) {
		if (node->GetNodeAttribute() && node->GetVisibility())
		{
			FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();
			if (attributeType == FbxNodeAttribute::eMesh)
			{
				if (!convertMesh(*model, s_scene, node, axisTransform))
					return false;
			}
		}
		return true;
	});

	// Create and assign default material if anonymous faces has been created.
	uint32_t defaultMaterialIndex = c_InvalidIndex;
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
	{
		const Polygon& polygon = model->getPolygon(i);
		if (polygon.getMaterial() == c_InvalidIndex)
		{
			if (defaultMaterialIndex == c_InvalidIndex)
			{
				Material material;
				material.setName(L"FBX_Default");
				material.setColor(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
				material.setDiffuseTerm(1.0f);
				material.setSpecularTerm(1.0f);

				//if (true /* has uv */)
				//{
				//	material.setNormalMap(Material::Map(L"Normals", L"default", false));
				//	material.setDiffuseMap(Material::Map(L"Albedo", L"default", true));
				//	material.setMetalnessMap(Material::Map(L"Metalness", L"default", false));
				//	material.setRoughnessMap(Material::Map(L"Roughness", L"default", false));
				//	material.setSpecularMap(Material::Map(L"Specular", L"default", false));
				//}

				defaultMaterialIndex = model->addMaterial(material);
			}

			Polygon replacement = polygon;
			replacement.setMaterial(defaultMaterialIndex);
			model->setPolygon(i, replacement);
		}
	}

	// \fixme Destroy when failing...
	importer->Destroy();
	importer = nullptr;

	return model;
}

bool ModelFormatFbx::write(const Path& filePath, const Model* model) const
{
	return false;
}

}
