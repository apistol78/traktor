/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Utf8Encoding.h"
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
#include "Render/Editor/Shader/ShaderGraphDotTool.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ShaderGraphDotTool", 0, ShaderGraphDotTool, editor::IEditorTool)

std::wstring ShaderGraphDotTool::getDescription() const
{
	return i18n::Text(L"SHADERGRAPH_DOT");
}

Ref< ui::IBitmap > ShaderGraphDotTool::getIcon() const
{
	return nullptr;
}

bool ShaderGraphDotTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool ShaderGraphDotTool::launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param)
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

	AlignedVector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, new Utf8Encoding());
	
	os << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << Endl;
	os << L"<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">" << Endl;
	os << L"\t<key id=\"label\" for=\"node\" attr.name=\"label\" attr.type=\"string\"/>" << Endl;
	os << L"\t<key id=\"color\" for=\"node\" attr.name=\"color\" attr.type=\"string\"><default>yellow</default></key>" << Endl;
	os << L"\t<graph id=\"G\" edgedefault=\"directed\">" << Endl;

	SmallMap< Guid, int32_t > ids;
	for (int32_t i = 0; i < (int32_t)instances.size(); ++i)
	{
		const int32_t id = i + 1;
		ids.insert(instances[i]->getGuid(), id);

		Ref< const ShaderGraph > shaderGraph = instances[i]->getObject< ShaderGraph >();
		if (!shaderGraph)
		{
			log::error << L"Unable to get shader graph from " << instances[i]->getPath() << L"." << Endl;
			continue;
		}

		const bool program = (bool)(ShaderGraphValidator(shaderGraph).estimateType() == ShaderGraphValidator::SgtProgram);
		
		os << L"\t\t<node id=\"n" << id << L"\">" << Endl;
		os << L"\t\t\t<data key=\"label\">" << instances[i]->getName() << L"</data>" << Endl;
		if (program)
			os << L"\t\t\t<data key=\"color\">green</data>" << Endl;
		os << L"\t\t</node>" << Endl;
	}

	int32_t edgeId = 1;
	for (auto instance : instances)
	{
		Ref< const ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
		if (!shaderGraph)
		{
			log::error << L"Unable to get shader graph from " << instance->getPath() << L"." << Endl;
			continue;
		}

		SmallSet< Guid > added;
		for (auto externalRef : shaderGraph->findNodesOf< External >())
		{
			if (added.find(externalRef->getFragmentGuid()) != added.end())
				continue;

			Ref< db::Instance > externalInstance = database->getInstance(externalRef->getFragmentGuid());
			if (!externalInstance)
				continue;

			os << L"\t\t<edge id=\"e" << edgeId << L"\" source=\"n" << ids[instance->getGuid()] << L"\" target=\"n" << ids[externalInstance->getGuid()] << L"\"/>" << Endl;
			++edgeId;

			added.insert(externalRef->getFragmentGuid());
		}
	}

	os << L"\t</graph>" << Endl;
	os << L"</graphml>" << Endl;
	os.close();

	if (!buffer.empty())
	{
		Ref< IStream > file = FileSystem::getInstance().open(
			L"Shaders.graphml",
			File::FmWrite
		);
		if (!file)
			return false;
		file->write(&buffer[0], int(buffer.size()));
		file->close();
	}

	log::info << L"Shaders.graphml file generated." << Endl;
	return true;
}

}
