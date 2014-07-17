#ifndef traktor_sound_SoundBatchDialog_H
#define traktor_sound_SoundBatchDialog_H

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class ListBox;

		namespace custom
		{

class AutoPropertyList;
class ToolBarButtonClickEvent;

		}
	}

	namespace sound
	{

class SoundAsset;

class SoundBatchDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	SoundBatchDialog(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	bool showModal(RefArray< SoundAsset >& outAssets);

private:
	editor::IEditor* m_editor;
	Ref< ui::ListBox > m_soundList;
	Ref< ui::custom::AutoPropertyList > m_soundPropertyList;

	void addSound();

	void removeSound();

	void eventSoundListToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventSoundListSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_sound_SoundBatchDialog_H
