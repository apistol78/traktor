/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_MeshPipelineTypes_H
#define traktor_mesh_MeshPipelineTypes_H

#include "Core/Config.h"

namespace traktor
{
	namespace mesh
	{

struct MeshMaterialTechnique
{
	std::wstring worldTechnique;	//< World render technique, e.g. "Default", "Depth", "Velocity" etc.
	std::wstring shaderTechnique;	//< Shader, render, technique. e.g. "Mnnnnnnnn".
	uint32_t hash;					//< Shader technique hash.
};

	}
}

#endif	// traktor_mesh_MeshPipelineTypes_H
