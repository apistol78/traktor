#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IThumbnailGenerator.h"
#include "Editor/TypeBrowseFilter.h"
#include "I18N/Text.h"
#include "Render/Shader/Nodes.h"
#include "Render/Editor/TextureAsset.h"
#include "Render/Editor/Shader/Facades/TextureNodeFacade.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/Node.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureNodeFacade", TextureNodeFacade, NodeFacade)

TextureNodeFacade::TextureNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = new ui::custom::DefaultNodeShape(graphControl);
}

Ref< Node > TextureNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Texture();
}

Ref< ui::custom::Node > TextureNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	Texture* texture = checked_type_cast< Texture* >(shaderNode);

	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
		i18n::Text(L"SHADERGRAPH_NODE_TEXTURE"),
		shaderNode->getInformation(),
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName()
		);
	}

	editorNode->setComment(shaderNode->getComment());
	editorNode->setColor(traktor::Color(255, 255, 200));

	Guid textureGuid = texture->getExternal();
	if (!textureGuid.isNull())
	{
		Ref< TextureAsset > textureAsset = editor->getSourceDatabase()->getObjectReadOnly< TextureAsset >(textureGuid);
		if (textureAsset)
		{
			std::wstring assetPath = editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
			Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());

			Ref< editor::IThumbnailGenerator > thumbnailGenerator = editor->getStoreObject< editor::IThumbnailGenerator >(L"ThumbnailGenerator");
			if (thumbnailGenerator)
			{
				bool visibleAlpha = (textureAsset->m_hasAlpha == true && textureAsset->m_ignoreAlpha == false);
				Ref< drawing::Image > thumbnail = thumbnailGenerator->get(fileName, 64, 64, visibleAlpha);
				if (thumbnail)
				{
					Ref< ui::Bitmap > nodeImage = new ui::Bitmap();
					if (nodeImage->create(thumbnail))
						editorNode->setImage(nodeImage);
				}
			}
		}
	}

	return editorNode;
}

void TextureNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	editor::TypeBrowseFilter filter(type_of< TextureAsset >());
	Ref< db::Instance > instance = editor->browseInstance(&filter);
	if (instance)
		checked_type_cast< Texture*, false >(shaderNode)->setExternal(instance->getGuid());
}

void TextureNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setColor(validationSucceeded ? traktor::Color(200, 255, 255) : traktor::Color(255, 255, 200));
}

	}
}
