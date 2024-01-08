/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <numeric>
#include "Core/Io/FileSystem.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Database.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/Triangulate.h"
#include "Shape/Editor/Spline/CloneShapeLayer.h"
#include "Shape/Editor/Spline/CloneShapeLayerData.h"

namespace traktor::shape
{
	namespace
	{
		
Ref< model::Model > readModel(const mesh::MeshAsset* meshAsset, const Path& modelCachePath, const std::wstring& assetPath)
{
	// Read model specified by mesh asset.
	const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + meshAsset->getFileName());
	Ref< model::Model > model = model::ModelCache::getInstance().getMutable(modelCachePath, filePath, meshAsset->getImportFilter());
	if (!model)
		return nullptr;

	// Only triangles; this method must only return triangle meshes.
	model::Triangulate().apply(*model);
	return model;
}
		
	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.CloneShapeLayerData", 0, CloneShapeLayerData, SplineLayerComponentData)

CloneShapeLayerData::CloneShapeLayerData()
:	m_automaticOrientation(false)
,	m_distance(1.0f)
{
}

Ref< SplineLayerComponent > CloneShapeLayerData::createComponent(db::Database* database, const Path& modelCachePath, const std::wstring& assetPath) const
{
	Ref< mesh::MeshAsset > mesh;
	Ref< model::Model > model;

	mesh = database->getObjectReadOnly< mesh::MeshAsset >(m_mesh);
	if (!mesh)
		return nullptr;
	model = readModel(mesh, modelCachePath, assetPath);
	if (!model)
		return nullptr;
		
	return new CloneShapeLayer(
		this,
		mesh,
		model
	);
}

void CloneShapeLayerData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"mesh", m_mesh, AttributeType(type_of< mesh::MeshAsset >()));
	s >> Member< bool >(L"automaticOrientation", m_automaticOrientation);
	s >> Member< float >(L"distance", m_distance);
}

}
