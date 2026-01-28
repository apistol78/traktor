/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/TouchShaderGraphsTool.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ParameterDeclaration.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/UniformDeclaration.h"

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
		instances);

	int32_t errorCount = 0;
	for (auto instance : instances)
	{
		// Ref< const UniformDeclaration > ud = instance->getObject< UniformDeclaration >();
		// if (!ud)
		//{
		//	errorCount++;
		//	continue;
		// }

		// Ref< ParameterDeclaration > pd = new ParameterDeclaration();
		// pd->m_type = ud->getParameterType();
		// pd->m_length = ud->getLength();
		// pd->m_frequency = ud->getFrequency();
		//
		// Ref< db::Instance > outputInstance = database->createInstance(L"New/" + instance->getPath(), db::CifReplaceExisting);
		// if (!outputInstance)
		//{
		//	errorCount++;
		//	continue;
		// }

		// outputInstance->setObject(pd);
		// outputInstance->commit();

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

		int32_t nmods = 0;

		for (auto readStructNode : shaderGraph->findNodesOf< ReadStruct2 >())
		{
			const Edge* edgeStruct = shaderGraph->findEdge(readStructNode->findInputPin(L"Struct"));
			const Edge* edgeIndex = shaderGraph->findEdge(readStructNode->findInputPin(L"Index"));

			Ref< ArrayElement > arrayElement = new ArrayElement();
			arrayElement->setPosition(readStructNode->getPosition());
			shaderGraph->addNode(arrayElement);

			shaderGraph->addEdge(new Edge(
				edgeStruct->getSource(),
				arrayElement->findInputPin(L"Input")));
			shaderGraph->addEdge(new Edge(
				edgeIndex->getSource(),
				arrayElement->findInputPin(L"Index")));

			for (int32_t i = 0; i < readStructNode->getOutputPinCount(); ++i)
			{
				const OutputPin* outputPin = readStructNode->getOutputPin(i);

				Ref< MemberValue > memberValue = new MemberValue();
				memberValue->setPosition(readStructNode->getPosition());
				memberValue->setMemberName(outputPin->getName());
				shaderGraph->addNode(memberValue);

				shaderGraph->addEdge(new Edge(
					arrayElement->findOutputPin(L"Output"),
					memberValue->findInputPin(L"Input")));

				const RefArray< Edge > edgeOutputs = shaderGraph->findEdges(outputPin);
				for (const auto edgeOutput : edgeOutputs)
				{
					shaderGraph->removeEdge(edgeOutput);

					shaderGraph->addEdge(new Edge(
						memberValue->findOutputPin(L"Output"),
						edgeOutput->getDestination()));
				}
			}

			shaderGraph->removeNode(readStructNode);

			++nmods;
		}

		/*
		for (auto readStructNode : shaderGraph->findNodesOf< ReadStruct >())
		{
			Ref< ArrayElement > arrayElement = new ArrayElement();
			arrayElement->setPosition(readStructNode->getPosition());
			shaderGraph->addNode(arrayElement);

			Ref< MemberValue > memberValue = new MemberValue();
			memberValue->setPosition(readStructNode->getPosition());
			memberValue->setMemberName(readStructNode->getName());
			shaderGraph->addNode(memberValue);

			const Edge* edgeStruct = shaderGraph->findEdge(readStructNode->findInputPin(L"Struct"));
			const Edge* edgeIndex = shaderGraph->findEdge(readStructNode->findInputPin(L"Index"));
			const RefArray< Edge > edgeOutputs = shaderGraph->findEdges(readStructNode->findOutputPin(L"Output"));

			shaderGraph->addEdge(new Edge(
				edgeStruct->getSource(),
				arrayElement->findInputPin(L"Input")));
			shaderGraph->addEdge(new Edge(
				edgeIndex->getSource(),
				arrayElement->findInputPin(L"Index")));

			shaderGraph->addEdge(new Edge(
				arrayElement->findOutputPin(L"Output"),
				memberValue->findInputPin(L"Input")));

			for (const auto edgeOutput : edgeOutputs)
			{
				shaderGraph->removeEdge(edgeOutput);

				shaderGraph->addEdge(new Edge(
					memberValue->findOutputPin(L"Output"),
					edgeOutput->getDestination()));
			}

			shaderGraph->removeNode(readStructNode);

			++nmods;
		}
		*/

		/*
		for (auto structNode : shaderGraph->findNodesOf< Struct >())
		{
			const std::wstring parameterDeclPath = L"New/" + structNode->getParameterName();

			Ref< db::Instance > parameterDeclInst = database->getInstance(parameterDeclPath);
			if (parameterDeclInst == nullptr)
			{
				parameterDeclInst = database->createInstance(parameterDeclPath);

				Ref< ParameterDeclaration > parameterDecl = new ParameterDeclaration();
				parameterDecl->m_type = ParameterType::StructBuffer;
				parameterDecl->m_structDeclaration = structNode->m_structDeclaration;

				parameterDeclInst->setObject(parameterDecl);
				parameterDeclInst->commit();
			}


			Ref< Parameter > parameterNode = new Parameter();
			parameterNode->setPosition(structNode->getPosition());
			parameterNode->setComment(structNode->getComment());
			parameterNode->m_parameterDeclaration = parameterDeclInst->getGuid();

			shaderGraph->replace(structNode, parameterNode);

			++nmods;
		}
		*/

		/*
		for (auto uniformNode : shaderGraph->findNodesOf< Uniform >())
		{
			Ref< db::Instance > uniformDeclInst = database->getInstance(uniformNode->getDeclaration());
			if (!uniformDeclInst)
				continue;

			std::wstring path = uniformDeclInst->getPath();
			std::wstring newpt = replaceAll(path, L"/Declarations/", L"/Parameters/");

			log::info << path << L" => " << newpt << Endl;

			Ref< db::Instance > parameterDeclInst = database->getInstance(newpt);
			if (!parameterDeclInst)
				continue;

			if (!parameterDeclInst->getObject< ParameterDeclaration >())
				continue;

			Ref< Parameter > parameterNode = new Parameter();
			parameterNode->setPosition(uniformNode->getPosition());
			parameterNode->setComment(uniformNode->getComment());
			parameterNode->m_parameterDeclaration = parameterDeclInst->getGuid();

			shaderGraph->replace(uniformNode, parameterNode);

			++nmods;
		}
		*/

		/*
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
					auto it = std::find_if(inputPorts.begin(), inputPorts.end(), [=](InputPort* inputPort) {
						return inputPort->getName() == inputPin->getName();
					});
					if (it != inputPorts.end())
					{
						*inputPin = InputPin(
							inputPin->getNode(),
							it->getId(),
							inputPin->getName(),
							inputPin->isOptional());
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
					auto it = std::find_if(outputPorts.begin(), outputPorts.end(), [=](OutputPort* outputPort) {
						return outputPort->getName() == outputPin->getName();
					});
					if (it != outputPorts.end())
					{
						*outputPin = OutputPin(
							outputPin->getNode(),
							it->getId(),
							outputPin->getName());
					}
					else
					{
						log::error << errorPrefix << L"No such output port \"" << outputPin->getName() << L"\" in fragment \"" << externalNode->getFragmentGuid().format() << L"\"; not updated." << Endl;
						errorCount++;
					}
				}
			}
		}
		*/

		if (nmods > 0)
		{
			instance->setObject(shaderGraph);

			if (!instance->commit())
			{
				instance->revert();
				log::error << L"Unable to commit " << instance->getPath() << L"." << Endl;
				errorCount++;
			}
		}
		else
			instance->revert();
	}

	log::info << L"All instances touched, " << errorCount << L" error(s)." << Endl;
	return true;
}

}
}
