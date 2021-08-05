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
#include "Render/Editor/Shader/ListUniformsTool.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ListUniformsTool", 0, ListUniformsTool, IEditorTool)

std::wstring ListUniformsTool::getDescription() const
{
	return i18n::Text(L"SHADERGRAPH_LIST_UNIFORMS");
}

Ref< ui::IBitmap > ListUniformsTool::getIcon() const
{
	return nullptr;
}

bool ListUniformsTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool ListUniformsTool::launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param)
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

    struct UniformInfo
    {
        SmallSet< ParameterType > types;
        SmallSet< UpdateFrequency > frequencies;
        SmallSet< uint32_t > lengths;
    };
	SmallMap< std::wstring, UniformInfo > uniformInfos;

	for (auto instance : instances)
	{
		Ref< ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
		if (!shaderGraph)
		{
			log::error << L"Unable to get shader graph from " << instance->getPath() << L"." << Endl;
			instance->revert();
			continue;
		}

		RefArray< Uniform > uniformNodes;
		shaderGraph->findNodesOf< Uniform >(uniformNodes);
		for (auto uniformNode : uniformNodes)
		{
            auto& ui = uniformInfos[uniformNode->getParameterName()];
            ui.types.insert(uniformNode->getParameterType());
            ui.frequencies.insert(uniformNode->getFrequency());
            ui.lengths.insert(1);
        }

		RefArray< IndexedUniform > indexedUniformNodes;
		shaderGraph->findNodesOf< IndexedUniform >(indexedUniformNodes);
		for (auto indexedUniformNode : indexedUniformNodes)
		{
            auto& ui = uniformInfos[indexedUniformNode->getParameterName()];
            ui.types.insert(indexedUniformNode->getParameterType());
            ui.frequencies.insert(indexedUniformNode->getFrequency());
            ui.lengths.insert(indexedUniformNode->getLength());
        }
	}

    const wchar_t* parameterTypeNames[] =
    {
        L"scalar",
        L"vector",
        L"matrix",
        L"texture2d",
        L"texture3d",
        L"textureCube",
        L"structBuffer"
    };

    const wchar_t* updateFrequencyNames[] =
    {
        L"once",
        L"frame",
        L"draw"
    };

    for (auto it : uniformInfos)
    {
        // log::info << it.first << L" " << parameterTypeNames[*it.second.types.begin()] << L"[" << *it.second.lengths.begin() << L"], (" << updateFrequencyNames[*it.second.frequencies.begin()] << L")." << Endl;
        if (it.second.types.size() != 1 || it.second.frequencies.size() != 1 || it.second.lengths.size() != 1)
        {
            log::warning << L"PARAMETER \"" << it.first << L"\" MISMATCH!" << Endl;
        }
    }

	return true;
}

	}
}
