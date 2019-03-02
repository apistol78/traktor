#pragma once

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

