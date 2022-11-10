/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/ShaderDependencyPane.h"
#include "Ui/TableLayout.h"
#include "Ui/ListBox/ListBox.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderDependencyPane", ShaderDependencyPane, ui::Container)

ShaderDependencyPane::ShaderDependencyPane(editor::IEditor* editor, const Guid& shaderId)
:	m_editor(editor)
,	m_shaderId(shaderId)
{
	m_dependencyTracker = m_editor->getStoreObject< ShaderDependencyTracker >(L"ShaderDependencyTracker");
}

void ShaderDependencyPane::destroy()
{
	if (m_dependencyTracker)
		m_dependencyTracker->removeListener(this);

	ui::Container::destroy();
}

bool ShaderDependencyPane::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	setText(i18n::Text(L"SHADERGRAPH_REFEREES"));

	m_refereeList = new ui::ListBox();
	m_refereeList->create(this, ui::ListBox::WsSingle | ui::ListBox::WsSort);

	if (m_dependencyTracker)
	{
		m_refereeList->addEventHandler< ui::MouseDoubleClickEvent >(this, &ShaderDependencyPane::eventRefereeListDoubleClick);
		m_dependencyTracker->addListener(this);
	}
	else
	{
		m_refereeList->add(i18n::Text(L"SHADERGRAPH_NO_DEPENDENCY_TRACKER_ERROR"));
		m_refereeList->setEnable(false);
	}

	return true;
}

void ShaderDependencyPane::dependencyAdded(const Guid& fromShader, const Guid& toShader)
{
	if (toShader != m_shaderId)
		return;

	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(fromShader);
	if (instance)
		m_refereeList->add(instance->getPath(), instance);
}

void ShaderDependencyPane::dependencyRemoved(const Guid& fromShader, const Guid& toShader)
{
	if (toShader != m_shaderId)
		return;

	int32_t count = m_refereeList->count();
	for (int32_t i = 0; i < count; ++i)
	{
		Ref< db::Instance > instance = m_refereeList->getData< db::Instance >(i);
		if (instance != nullptr && instance->getGuid() == fromShader)
		{
			m_refereeList->remove(i);
			break;
		}
	}
}

void ShaderDependencyPane::eventRefereeListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< db::Instance > instance = m_refereeList->getSelectedData< db::Instance >();
	if (instance)
		m_editor->openEditor(instance);
}

	}
}
