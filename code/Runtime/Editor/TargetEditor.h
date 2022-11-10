/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Editor/IObjectEditor.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Bitmap;
class Container;
class DropDown;
class Edit;
class EditList;
class EditListEditEvent;
class Image;
class ListBox;
class MiniButton;

	}

	namespace runtime
	{

class Feature;
class Platform;
class Target;
class TargetConfiguration;

/*! \brief
 * \ingroup Runtime
 */
class TargetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	TargetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	struct EditFeature : public Object
	{
		Ref< Feature > feature;
		Ref< db::Instance > featureInstance;

		bool operator < (const EditFeature& ef) const;
	};

	editor::IEditor* m_editor;
	Ref< db::Instance > m_editInstance;
	Ref< Target > m_editTarget;
	RefArray< db::Instance > m_platformInstances;
	std::list< EditFeature > m_features;
	Ref< ui::Bitmap > m_bitmapNoIcon;
	Ref< ui::Container > m_containerOuter;
	Ref< ui::EditList > m_listBoxTargetConfigurations;
	Ref< ui::Image > m_imageIcon;
	Ref< ui::DropDown > m_dropDownPlatform;
	Ref< ui::MiniButton > m_buttonBuildRootInstance;
	Ref< ui::MiniButton > m_buttonStartupInstance;
	Ref< ui::MiniButton > m_buttonDefaultInputInstance;
	Ref< ui::MiniButton > m_buttonOnlineConfigInstance;
	Ref< ui::Edit > m_editBuildRootInstance;
	Ref< ui::Edit > m_editStartupInstance;
	Ref< ui::Edit > m_editDefaultInputInstance;
	Ref< ui::Edit > m_editOnlineConfigInstance;
	Ref< ui::ListBox > m_listBoxAvailFeatures;
	Ref< ui::ListBox > m_listBoxUsedFeatures;

	void updateTargetConfigurations();

	void updateAvailableFeatures();

	void updateUsedFeatures();

	void updateRoots();

	void updateIcon();

	void selectPlatform(const Guid& platformGuid) const;

	void eventListBoxTargetConfigurationsEdit(ui::EditListEditEvent* event);

	void eventListBoxTargetConfigurationsSelect(ui::SelectionChangeEvent* event);

	void eventButtonNewTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventButtonCloneTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventButtonRemoveTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventDropDownPlatformSelect(ui::SelectionChangeEvent* event);

	void eventBrowseRootButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseStartupButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseDefaultInputButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseOnlineConfigButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseIconClick(ui::MouseButtonDownEvent* event);

	void eventButtonAddFeatureClick(ui::ButtonClickEvent* event);

	void eventButtonRemoveFeatureClick(ui::ButtonClickEvent* event);
};

	}
}

