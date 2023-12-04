/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
		
Ref< model::Model > readModel(db::Database* database, const Path& modelCachePath, const std::wstring& assetPath, const Guid& meshAssetId)
{
	// Read extrude mesh asset from database.
	Ref< const mesh::MeshAsset > meshAsset = database->getObjectReadOnly< mesh::MeshAsset >(meshAssetId);
	if (!meshAsset)
		return nullptr;

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
	Ref< model::Model > modelStart;
	Ref< model::Model > modelRepeat;
	Ref< model::Model > modelEnd;

	modelRepeat = readModel(database, modelCachePath, assetPath, m_meshRepeat);
	if (!modelRepeat)
		return nullptr;

	if (m_meshStart.isNotNull())
		modelStart = readModel(database, modelCachePath, assetPath, m_meshStart);
	if (m_meshEnd.isNotNull())
		modelEnd = readModel(database, modelCachePath, assetPath, m_meshEnd);

	if (!modelStart)
		modelStart = modelRepeat;
	if (!modelEnd)
		modelEnd = modelRepeat;
		
	return new ExtrudeShapeLayer(
		modelStart,
		modelRepeat,
		modelEnd,
		m_automaticOrientation
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
