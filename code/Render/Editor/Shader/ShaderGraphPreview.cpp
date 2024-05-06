#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IEditor.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphPreview.h"
#include "Render/Editor/Texture/TextureAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphPreview", ShaderGraphPreview, Object)

ShaderGraphPreview::ShaderGraphPreview(const editor::IEditor* editor)
{
	m_assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_database = editor->getSourceDatabase();
}

ShaderGraphPreview::ShaderGraphPreview(const std::wstring& assetPath, db::Database* database)
:	m_assetPath(assetPath)
,	m_database(database)
{
}

Ref< drawing::Image > ShaderGraphPreview::generate(const ShaderGraph* shaderGraph, int32_t width, int32_t height) const
{
	Ref< ShaderGraph > resolvedShaderGraph = FragmentLinker([&](const Guid& fragmentId) -> Ref< const ShaderGraph > {
		return m_database->getObjectReadOnly< ShaderGraph >(fragmentId);
	}).resolve(shaderGraph, true);
	if (!resolvedShaderGraph)
		return nullptr;

	const RefArray< PreviewOutput > previewOutputs = resolvedShaderGraph->findNodesOf< PreviewOutput >();
	if (previewOutputs.empty())
		return nullptr;

	Ref< drawing::Image > image;

	const InputPin* inputPin = previewOutputs.front()->getInputPin(0);
	const OutputPin* outputPin = resolvedShaderGraph->findSourcePin(inputPin);
	if (outputPin == nullptr)
		return nullptr;

	if (auto colorNode = dynamic_type_cast< const Color* >(outputPin->getNode()))
	{
		image = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), width, height);
		image->clear(colorNode->getColor());
	}
	else if (auto textureNode = dynamic_type_cast< const Texture* >(outputPin->getNode()))
	{
		Ref< const render::TextureAsset > textureAsset = m_database->getObjectReadOnly< render::TextureAsset >(textureNode->getExternal());
		if (textureAsset)
		{
			const Path filePath = FileSystem::getInstance().getAbsolutePath(m_assetPath, textureAsset->getFileName());
			Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
			if (file)
			{
				image = drawing::Image::load(file, textureAsset->getFileName().getExtension());
				if (image)
				{
					const drawing::ScaleFilter scaleFilter(width, height, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
					image->apply(&scaleFilter);
				}
			}
		}
	}
	
	return image;
}

}
