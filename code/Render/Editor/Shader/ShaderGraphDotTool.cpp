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

	os << L"digraph G {" << Endl;
	os << IncreaseIndent;
	os << L"node [shape=box];" << Endl;

	SmallMap< Guid, int32_t > ids;
	for (int32_t i = 0; i < (int32_t)instances.size(); ++i)
	{
		ids.insert(instances[i]->getGuid(), i + 1);
		os << L"N" << ids[instances[i]->getGuid()] << L" [label=\"" << instances[i]->getName() << L"\"];" << Endl;
	}

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

			os << L"N" << ids[instance->getGuid()] << L" -> N" << ids[externalInstance->getGuid()] << L";" << Endl;

			added.insert(externalRef->getFragmentGuid());
		}
	}

	os << DecreaseIndent;
	os << L"}" << Endl;

	os.close();

	if (!buffer.empty())
	{
		Ref< IStream > file = FileSystem::getInstance().open(
			L"ShaderGraphs.dot",
			File::FmWrite
		);
		if (!file)
			return false;
		file->write(&buffer[0], int(buffer.size()));
		file->close();
	}

	log::info << L"ShaderGraph dot file generated." << Endl;
	return true;
}

}
