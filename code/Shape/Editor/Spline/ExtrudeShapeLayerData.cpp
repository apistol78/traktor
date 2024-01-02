/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#include "Shape/Editor/Spline/ExtrudeShapeLayer.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"

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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.ExtrudeShapeLayerData", 0, ExtrudeShapeLayerData, SplineLayerComponentData)

ExtrudeShapeLayerData::ExtrudeShapeLayerData()
:	m_automaticOrientation(false)
,	m_detail(10.0f)
{
}

Ref< SplineLayerComponent > ExtrudeShapeLayerData::createComponent(db::Database* database, const Path& modelCachePath, const std::wstring& assetPath) const
{
	Ref< mesh::MeshAsset > meshStart;
	Ref< mesh::MeshAsset > meshRepeat;
	Ref< mesh::MeshAsset > meshEnd;

	Ref< model::Model > modelStart;
	Ref< model::Model > modelRepeat;
	Ref< model::Model > modelEnd;

	meshRepeat = database->getObjectReadOnly< mesh::MeshAsset >(m_meshRepeat);
	if (!meshRepeat)
		return nullptr;
	modelRepeat = readModel(meshRepeat, modelCachePath, assetPath);
	if (!modelRepeat)
		return nullptr;

	if (m_meshStart.isNotNull())
	{
		meshStart = database->getObjectReadOnly< mesh::MeshAsset >(m_meshStart);
		if (!meshStart)
			return nullptr;
		modelStart = readModel(meshStart, modelCachePath, assetPath);
		if (!modelStart)
			return nullptr;
	}
	if (m_meshEnd.isNotNull())
	{
		meshEnd = database->getObjectReadOnly< mesh::MeshAsset >(m_meshEnd);
		if (!meshEnd)
			return nullptr;
		modelEnd = readModel(meshEnd, modelCachePath, assetPath);
		if (!modelEnd)
			return nullptr;
	}

	if (!modelStart)
	{
		meshStart = meshRepeat;
		modelStart = modelRepeat;
	}
	if (!modelEnd)
	{
		meshEnd = meshRepeat;
		modelEnd = modelRepeat;
	}
		
	return new ExtrudeShapeLayer(
		this,
		meshStart, modelStart,
		meshRepeat, modelRepeat,
		meshEnd, modelEnd
	);
}

void ExtrudeShapeLayerData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"meshStart", m_meshStart, AttributeType(type_of< mesh::MeshAsset >()));
	s >> Member< Guid >(L"meshRepeat", m_meshRepeat, AttributeType(type_of< mesh::MeshAsset >()));
	s >> Member< Guid >(L"meshEnd", m_meshEnd, AttributeType(type_of< mesh::MeshAsset >()));
	s >> Member< bool >(L"automaticOrientation", m_automaticOrientation);
	s >> Member< float >(L"detail", m_detail);
}

}
