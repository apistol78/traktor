/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/RefArray.h"
#include "Editor/IObjectEditor.h"
#include "Ui/Events/AllEvents.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::ui
{

class Command;
class Container;
class Menu;
class Panel;
class Slider;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;

}

namespace traktor::sound
{

class BankAsset;
class BankBuffer;
class BankControl;
class BankControlGrain;
class IGrain;
class IGrainData;
class IGrainFacade;
class ISoundHandle;
class ISoundPlayer;
class GrainProperties;

class BankAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	explicit BankAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< BankAsset > m_asset;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::ToolBarButton > m_toolBarItemPlay;
	Ref< ui::ToolBarButton > m_toolBarItemRepeat;
	Ref< BankControl > m_bankControl;
	Ref< ui::Panel > m_containerParameters;
	RefArray< ui::Slider > m_sliderParameters;
	Ref< ui::Panel > m_containerGrainProperties;
	Ref< GrainProperties > m_grainProperties;
	Ref< ui::Panel > m_containerGrainView;
	Ref< ui::Menu > m_menuGrains;
	std::map< const TypeInfo*, Ref< IGrainFacade > > m_grainFacades;
	Ref< ui::Widget > m_currentGrainView;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< ISoundPlayer > m_soundPlayer;
	Ref< ISoundHandle > m_soundHandle;
	Ref< BankBuffer > m_bankBuffer;
	std::map< const IGrainData*, const IGrain* > m_grainInstances;

	void updateBankControl(BankControlGrain* parent, const RefArray< IGrainData >& grains);

	void updateBankControl();

	void updateProperties();

	void eventParameterChange(ui::ContentChangeEvent* event);

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventGrainSelect(ui::SelectionChangeEvent* event);

	void eventGrainButtonUp(ui::MouseButtonUpEvent* event);

	void eventGrainPropertiesChange(ui::ContentChangeEvent* event);

	void eventGrainViewChange(ui::ContentChangeEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

}
