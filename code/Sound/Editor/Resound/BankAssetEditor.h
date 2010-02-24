#ifndef traktor_sound_BankAssetEditor_H
#define traktor_sound_BankAssetEditor_H

#include "Core/RefArray.h"
#include "Editor/IObjectEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace ui
	{

class Command;
class Container;
class Event;
class PopupMenu;
class Slider;

	}

	namespace sound
	{

class BankAsset;
class IGrain;
class IGrainEditor;
class GrainView;
class GrainViewItem;
class SoundSystem;

class T_DLLCLASS BankAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	BankAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< BankAsset > m_asset;
	Ref< GrainView > m_grainView;
	Ref< ui::Container > m_containerGrainProperties;
	Ref< ui::PopupMenu > m_menuGrains;
	Ref< IGrainEditor > m_grainEditorDefault;
	Ref< SoundSystem > m_soundSystem;
	Ref< resource::IResourceManager > m_resourceManager;

	void updateGrainView(GrainViewItem* parent, const RefArray< IGrain >& grains);

	void updateGrainView();

	void handleCommand(const ui::Command& command);

	void eventToolBarClick(ui::Event* event);

	void eventGrainSelect(ui::Event* event);

	void eventGrainButtonUp(ui::Event* event);
};

	}
}

#endif	// traktor_sound_BankAssetEditor_H
