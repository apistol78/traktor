#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Mesh/Editor/OcclusionTextureAsset.h"
#include "Model/Model.h"
#include "Model/Utilities.h"
#include "Model/Formats/ModelFormat.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.OcclusionTextureAsset", 5, OcclusionTextureAsset, render::TextureAssetBase)

Ref< drawing::Image > OcclusionTextureAsset::load(const std::wstring& assetPath) const
{
	if (!m_scaleImage || m_scaleWidth <= 0 || m_scaleHeight <= 0)
	{
		log::error << L"Invalid parameters; must define a scale size of occlusion texture asset" << Endl;
		return 0;
	}

	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, getFileName());

	// Read source model.
	Ref< model::Model > model = model::ModelFormat::readAny(fileName);
	if (!model)
	{
		log::error << L"Unable to read source model (" << fileName.getPathName() << L")" << Endl;
		return 0;
	}

	// Ensure model is triangulated.
	model::triangulateModel(*model);

	// Bake occlusion map.
	Ref< drawing::Image > image = model::bakePixelOcclusion(*model, m_scaleWidth, m_scaleHeight);
	if (!image)
	{
		log::error << L"Unable to bake occlusion map" << Endl;
		return 0;
	}

	return image;
}

	}
}
