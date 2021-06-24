#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace editor
	{

class IEditor;

	}

	namespace mesh
	{

class MeshAsset;

class MeshAssetRasterizer : public Object
{
	T_RTTI_CLASS;

public:
	bool generate(const editor::IEditor* editor, const MeshAsset* asset, drawing::Image* outImage) const;
};

	}
}
