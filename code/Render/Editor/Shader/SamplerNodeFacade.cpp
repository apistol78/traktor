#include "Render/Editor/Shader/SamplerNodeFacade.h"
#include "Render/Editor/TextureAsset.h"
#include "Render/Nodes.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Editor/Settings.h"
#include "Editor/TypeBrowseFilter.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "I18N/Text.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SamplerNodeFacade", SamplerNodeFacade, NodeFacade)

SamplerNodeFacade::SamplerNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = gc_new< ui::custom::DefaultNodeShape >(graphControl);
}

Node* SamplerNodeFacade::createShaderNode(
	const Type* nodeType,
	editor::IEditor* editor
)
{
	return gc_new< Sampler >();
}

ui::custom::Node* SamplerNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	Sampler* sampler = checked_type_cast< Sampler* >(shaderNode);

	Ref< ui::custom::Node > editorNode = gc_new< ui::custom::Node >(
		i18n::Text(L"SHADERGRAPH_NODE_SAMPLER"),
		shaderNode->getInformation(),
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);

	editorNode->setColor(traktor::Color(255, 255, 200));

	Guid textureGuid = sampler->getExternal();
	if (!textureGuid.isNull())
	{
		Ref< editor::IProject > project = editor->getProject();
		Ref< TextureAsset > textureAsset = project->getSourceDatabase()->getObjectReadOnly< TextureAsset >(textureGuid);
		if (textureAsset)
		{
			std::wstring assetPath = editor->getSettings()->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
			Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());

			Ref< drawing::Image > textureImage = drawing::Image::load(fileName);
			if (textureImage)
			{
				drawing::ScaleFilter scale(
					64,
					64,
					drawing::ScaleFilter::MnAverage,
					drawing::ScaleFilter::MgNearest
				);
				textureImage = textureImage->applyFilter(&scale);

				// Create alpha preview.
				if (textureImage->getPixelFormat()->getAlphaBits() > 0 && textureAsset->m_hasAlpha == true && textureAsset->m_ignoreAlpha == false)
				{
					for (int32_t y = 0; y < textureImage->getHeight(); ++y)
					{
						for (int32_t x = 0; x < textureImage->getWidth(); ++x)
						{
							drawing::Color alpha =
								((x >> 2) & 1) ^ ((y >> 2) & 1) ?
								drawing::Color(0.4f, 0.4f, 0.4f) :
								drawing::Color(0.6f, 0.6f, 0.6f);

							drawing::Color pixel;
							textureImage->getPixel(x, y, pixel);

							pixel = pixel * pixel.getAlpha() + alpha * (1.0f - pixel.getAlpha());
							pixel.setAlpha(1.0f);

							textureImage->setPixel(x, y, pixel);
						}
					}
				}
				else	// Create solid alpha channel.
				{
					textureImage->convert(drawing::PixelFormat::getR8G8B8A8());

					for (int32_t y = 0; y < textureImage->getHeight(); ++y)
					{
						for (int32_t x = 0; x < textureImage->getWidth(); ++x)
						{
							drawing::Color pixel;
							textureImage->getPixel(x, y, pixel);

							pixel.setAlpha(1.0f);
							textureImage->setPixel(x, y, pixel);
						}
					}
				}

				Ref< ui::Bitmap > nodeImage = gc_new< ui::Bitmap >();
				if (nodeImage->create(textureImage))
					editorNode->setImage(nodeImage);
			}
		}
	}

	return editorNode;
}

void SamplerNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	editor::TypeBrowseFilter filter(type_of< TextureAsset >());
	Ref< db::Instance > instance = editor->browseInstance(&filter);
	if (instance)
		checked_type_cast< Sampler* >(shaderNode)->setExternal(instance->getGuid());
}

void SamplerNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setColor(validationSucceeded ? traktor::Color(200, 255, 255) : traktor::Color(255, 255, 200));
}

	}
}
