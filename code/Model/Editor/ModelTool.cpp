/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Editor/IEditor.h"
#include "Model/Editor/ModelTool.h"
#include "Model/Editor/ModelToolDialog.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelTool", 0, ModelTool, editor::IEditorTool)

ModelTool::~ModelTool()
{
	safeDestroy(m_dialog);
}

std::wstring ModelTool::getDescription() const
{
	return L"Model Tool";
}

Ref< ui::IBitmap > ModelTool::getIcon() const
{
	return 0;
}

bool ModelTool::launch(ui::Widget* parent, editor::IEditor* editor, const std::wstring& param)
{
	safeDestroy(m_dialog);

	Ref< render::IRenderSystem > renderSystem = editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< db::Database > database = editor->getOutputDatabase();
	if (!database)
		return false;

	Ref< resource::IResourceManager > resourceManager = new resource::ResourceManager(database, true);
	resourceManager->addFactory(new render::TextureFactory(renderSystem, 0));
	resourceManager->addFactory(new render::ShaderFactory(renderSystem));

	m_dialog = new ModelToolDialog(resourceManager, renderSystem);
	if (!m_dialog->create(parent, param))
	{
		safeDestroy(m_dialog);
		return false;
	}

	return true;
}

	}
}