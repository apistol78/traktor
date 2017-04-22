#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IThumbnailGenerator.h"
#include "I18N/Text.h"
#include "Render/ITexture.h"
#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/TextureNodeFacade.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/Node.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureNodeFacade", TextureNodeFacade, INodeFacade)

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
	ShaderGraph* shaderGraph,
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

	updateThumb(editor, editorNode, texture);

	return editorNode;
}

void TextureNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< db::Instance > instance = editor->browseInstance(type_of< ITexture >());
	if (instance)
		checked_type_cast< Texture*, false >(shaderNode)->setExternal(instance->getGuid());
}

void TextureNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(shaderNode->getInformation());
	updateThumb(editor, editorNode, checked_type_cast< Texture*, false >(shaderNode));
}

void TextureNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

void TextureNodeFacade::updateThumb(editor::IEditor* editor, ui::custom::Node* editorNode, Texture* texture) const
{
	Guid textureGuid = texture ? texture->getExternal() : Guid();
	if (textureGuid.isValid() && !textureGuid.isNull())
	{
		Ref< TextureAsset > textureAsset = editor->getSourceDatabase()->getObjectReadOnly< TextureAsset >(textureGuid);
		if (textureAsset)
		{
			std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
			Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());

			Ref< editor::IThumbnailGenerator > thumbnailGenerator = editor->getStoreObject< editor::IThumbnailGenerator >(L"ThumbnailGenerator");
			if (thumbnailGenerator)
			{
				bool visibleAlpha = (textureAsset->m_output.m_hasAlpha == true && textureAsset->m_output.m_ignoreAlpha == false);
				Ref< drawing::Image > thumbnail = thumbnailGenerator->get(fileName, 64, 64, visibleAlpha ? editor::IThumbnailGenerator::AmWithAlpha : editor::IThumbnailGenerator::AmNoAlpha);
				if (thumbnail)
				{
					Ref< ui::Bitmap > nodeImage = new ui::Bitmap();
					if (nodeImage->create(thumbnail))
					{
						editorNode->setImage(nodeImage);
						return;
					}
				}
			}
		}
	}

	// Failed to update thumb; or no texture bound yet.
	editorNode->setImage(0);
}

	}
}
