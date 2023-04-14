/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/TouchShaderGraphsTool.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.TouchShaderGraphsTool", 0, TouchShaderGraphsTool, editor::IEditorTool)

std::wstring TouchShaderGraphsTool::getDescription() const
{
	return i18n::Text(L"SHADERGRAPH_TOUCH_ALL");
}

Ref< ui::IBitmap > TouchShaderGraphsTool::getIcon() const
{
	return nullptr;
}

bool TouchShaderGraphsTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool TouchShaderGraphsTool::launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param)
{
	Ref< db::Database > database = editor->getSourceDatabase();
	if (!database)
		return true;

	RefArray< db::Instance > instances;
	db::recursiveFindChildInstances(
		database->getRootGroup(),
		db::FindInstanceByType(type_of< ShaderGraph >()),
		instances
	);

	int32_t errorCount = 0;
	for (auto instance : instances)
	{
		if (!instance->checkout())
		{
			log::error << L"Unable to checkout " << instance->getPath() << L"." << Endl;
			errorCount++;
			continue;
		}

		Ref< ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
		if (!shaderGraph)
		{
			log::error << L"Unable to get shader graph from " << instance->getPath() << L"." << Endl;
			instance->revert();
			errorCount++;
			continue;
		}

		const std::wstring errorPrefix = L"Error when updating shader graph \"" + instance->getGuid().format() + L"\"; ";

		for (auto externalNode : shaderGraph->findNodesOf< External >())
		{
			Ref< const ShaderGraph > fragmentGraph = database->getObjectReadOnly< ShaderGraph >(externalNode->getFragmentGuid());
			if (!fragmentGraph)
			{
				log::error << errorPrefix << L"Unable to read fragment \"" << externalNode->getFragmentGuid().format() << L"\"; not updated." << Endl;
				errorCount++;
				continue;
			}

			RefArray< InputPort > inputPorts = fragmentGraph->findNodesOf< InputPort >();
			RefArray< OutputPort > outputPorts = fragmentGraph->findNodesOf< OutputPort >();

			for (auto& inputPin : externalNode->getInputPins())
			{
				if (inputPin->getId().isNull())
				{
					auto it = std::find_if(inputPorts.begin(), inputPorts.end(), [=](InputPort* inputPort){
						return inputPort->getName() == inputPin->getName();
					});
					if (it != inputPorts.end())
					{
						*inputPin = InputPin(
							inputPin->getNode(),
							it->getId(),
							inputPin->getName(),
							inputPin->isOptional()
						);
					}
					else
					{
						log::error << errorPrefix << L"No such input port \"" << inputPin->getName() << L"\" in fragment \"" << externalNode->getFragmentGuid().format() << L"\"; not updated." << Endl;
						errorCount++;
					}
				}
			}

			for (auto outputPin : externalNode->getOutputPins())
			{
				if (outputPin->getId().isNull())
				{
					auto it = std::find_if(outputPorts.begin(), outputPorts.end(), [=](OutputPort* outputPort){
						return outputPort->getName() == outputPin->getName();
					});
					if (it != outputPorts.end())
					{
						*outputPin = OutputPin(
							outputPin->getNode(),
							it->getId(),
							outputPin->getName()
						);
					}
					else
					{
						log::error << errorPrefix << L"No such output port \"" << outputPin->getName() << L"\" in fragment \"" << externalNode->getFragmentGuid().format() << L"\"; not updated." << Endl;
						errorCount++;
					}
				}
			}
		}

		instance->setObject(shaderGraph);

		if (!instance->commit())
		{
			instance->revert();
			log::error << L"Unable to commit " << instance->getPath() << L"." << Endl;
			errorCount++;
		}
	}

	log::info << L"All instances touched, " << errorCount << L" error(s)." << Endl;
	return true;
}

	}
}
