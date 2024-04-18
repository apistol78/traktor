/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Model/Editor/ModelTool.h"
#include "Model/Editor/ModelToolDialog.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptFactory.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelTool", 0, ModelTool, editor::IEditorTool)

ModelTool::~ModelTool()
{
	safeDestroy(m_dialog);
}

std::wstring ModelTool::getDescription() const
{
	return i18n::Text(L"MODEL_TOOL_DESCRIPTION");
}

Ref< ui::IBitmap > ModelTool::getIcon() const
{
	return nullptr;
}

bool ModelTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	outDependencies.insert(Guid(L"{123602E4-BC6F-874D-92E8-A20852D140A3}"));	// System/Primitive/Textures/SmallFont
	outDependencies.insert(Guid(L"{0163BEDD-9297-A64F-AAD5-360E27E37C6E}"));
	return true;
}

bool ModelTool::launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param)
{
	safeDestroy(m_dialog);

	Ref< db::Database > database = editor->getOutputDatabase();
	if (!database)
		return false;

	Ref< render::IRenderSystem > renderSystem = editor->getObjectStore()->get< render::IRenderSystem >();
	if (!renderSystem)
		return false;

	Ref< script::IScriptManager > scriptManager = editor->getObjectStore()->get< script::IScriptManager >();
	if (!scriptManager)
		return false;

	Ref< resource::IResourceManager > resourceManager = new resource::ResourceManager(database, editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	resourceManager->addFactory(new render::TextureFactory(renderSystem, 0));
	resourceManager->addFactory(new render::ShaderFactory(renderSystem));
	resourceManager->addFactory(new script::ScriptFactory(scriptManager));

	std::wstring fileName;
	float scale = 1.0f;

	if (param)
	{
		fileName = param->getProperty< std::wstring >(L"fileName", L"");
		scale = param->getProperty< float >(L"scale", 1.0f);
	}

	m_dialog = new ModelToolDialog(editor, resourceManager, renderSystem);
	if (!m_dialog->create(parent, fileName, scale))
	{
		safeDestroy(m_dialog);
		return false;
	}

	return true;
}

}
