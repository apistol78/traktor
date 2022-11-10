/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Mesh/Partition/IPartitionData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class OctreeNodeData;

class T_DLLCLASS OctreePartitionData : public IPartitionData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IPartition > createPartition() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class PartitionMeshConverter;

	AlignedVector< std::wstring > m_worldTechniques;
	Ref< const OctreeNodeData > m_nodeData;
};

}
