/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/ShaderDependencyPane.h"
#include "Ui/TableLayout.h"
#include "Ui/ListBox/ListBox.h"

namespace traktor::render
{
	namespace
	{

class RefereeListData : public Object
{
	T_RTTI_CLASS;

public:
	RefereeListData() = default;

	explicit RefereeListData(const db::Instance* _instance)
	{
		id = _instance->getGuid();
	}

public:
	Guid id;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderDependencyPane.RefereeListData", RefereeListData, Object)

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderDependencyPane", ShaderDependencyPane, ui::Container)

ShaderDependencyPane::ShaderDependencyPane(editor::IEditor* editor, const Guid& shaderId)
:	m_editor(editor)
,	m_shaderId(shaderId)
{
	m_dependencyTracker = m_editor->getObjectStore()->get< ShaderDependencyTracker >();
}

void ShaderDependencyPane::destroy()
{
	if (m_dependencyTracker)
		m_dependencyTracker->removeListener(this);

	ui::Container::destroy();
}

bool ShaderDependencyPane::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0_ut, 0_ut)))
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
		m_refereeList->add(instance->getPath(), new RefereeListData(instance));
}

void ShaderDependencyPane::dependencyRemoved(const Guid& fromShader, const Guid& toShader)
{
	if (toShader != m_shaderId)
		return;

	const int32_t count = m_refereeList->count();
	for (int32_t i = 0; i < count; ++i)
	{
		Ref< RefereeListData > rd = m_refereeList->getData< RefereeListData >(i);
		if (rd->id == fromShader)
		{
			m_refereeList->remove(i);
			break;
		}
	}
}

void ShaderDependencyPane::eventRefereeListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< RefereeListData > rd = m_refereeList->getSelectedData< RefereeListData >();
	if (rd)
	{
		Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(rd->id);
		if (instance)
			m_editor->openEditor(instance);
	}
}

}
