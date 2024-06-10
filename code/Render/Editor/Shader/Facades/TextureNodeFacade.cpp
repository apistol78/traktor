/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IThumbnailGenerator.h"
#include "I18N/Text.h"
#include "Render/ITexture.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/TextureNodeFacade.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureNodeFacade", TextureNodeFacade, INodeFacade)

TextureNodeFacade::TextureNodeFacade()
{
	m_nodeShape = new ui::DefaultNodeShape(ui::DefaultNodeShape::StDefault);
}

Ref< Node > TextureNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Texture();
}

Ref< ui::Node > TextureNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Texture* texture = checked_type_cast< Texture* >(shaderNode);

	Ref< ui::Node > editorNode = graphControl->createNode(
		i18n::Text(L"SHADERGRAPH_NODE_TEXTURE"),
		shaderNode->getInformation(),
		ui::UnitPoint(
			ui::Unit(shaderNode->getPosition().first),
			ui::Unit(shaderNode->getPosition().second)
		),
		m_nodeShape
	);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			inputPin->getId(),
			!inputPin->isOptional(),
			false
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName(),
			outputPin->getId()
		);
	}

	editorNode->setComment(shaderNode->getComment());

	updateThumb(editor, graphControl, editorNode, texture);

	return editorNode;
}

void TextureNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
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
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(shaderNode->getInformation());
	updateThumb(editor, graphControl, editorNode, checked_type_cast< Texture*, false >(shaderNode));
}

void TextureNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

void TextureNodeFacade::updateThumb(editor::IEditor* editor, ui::GraphControl* graphControl, ui::Node* editorNode, Texture* texture) const
{
	Guid textureGuid = texture ? texture->getExternal() : Guid();
	if (textureGuid.isValid() && !textureGuid.isNull())
	{
		Ref< TextureAsset > textureAsset = editor->getSourceDatabase()->getObjectReadOnly< TextureAsset >(textureGuid);
		if (textureAsset)
		{
			std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
			Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());

			Ref< editor::IThumbnailGenerator > thumbnailGenerator = editor->getObjectStore()->get< editor::IThumbnailGenerator >();
			if (thumbnailGenerator)
			{
				const bool visibleAlpha = (textureAsset->m_output.m_hasAlpha == true && textureAsset->m_output.m_ignoreAlpha == false);
				const bool linearGamma = textureAsset->m_output.m_assumeLinearGamma;
				Ref< drawing::Image > thumbnail = thumbnailGenerator->get(
					fileName,
					graphControl->pixel(64_ut),
					graphControl->pixel(64_ut),
					visibleAlpha ? editor::IThumbnailGenerator::Alpha::WithAlpha : editor::IThumbnailGenerator::Alpha::NoAlpha,
					linearGamma ? editor::IThumbnailGenerator::Gamma::Linear : editor::IThumbnailGenerator::Gamma::SRGB
				);
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
	editorNode->setImage(nullptr);
}

}
