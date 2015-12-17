#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundBatchDialog.h"
#include "Ui/Bitmap.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/TableLayout.h"
#include "Ui/ListBox.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

// Resources
#include "Resources/PlusMinus.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundBatchDialog", SoundBatchDialog, ui::ConfigDialog)

SoundBatchDialog::SoundBatchDialog(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool SoundBatchDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"SOUND_BATCH_DIALOG_TITLE"),
		900,
		500,
		ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 200);

	Ref< ui::Container > soundListContainer = new ui::Container();
	soundListContainer->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::custom::ToolBar > soundListTools = new ui::custom::ToolBar();
	if (!soundListTools->create(soundListContainer))
		return false;

	soundListTools->addImage(ui::Bitmap::load(c_ResourcePlusMinus, sizeof(c_ResourcePlusMinus), L"image"), 4);
	soundListTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SOUND_BATCH_ADD"), 0, ui::Command(L"SoundBatch.Add")));
	soundListTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SOUND_BATCH_REMOVE"), 1, ui::Command(L"SoundBatch.Remove")));
	soundListTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &SoundBatchDialog::eventSoundListToolClick);

	m_soundList = new ui::ListBox();
	m_soundList->create(soundListContainer, L"", ui::ListBox::WsExtended);
	m_soundList->addEventHandler< ui::SelectionChangeEvent >(this, &SoundBatchDialog::eventSoundListSelect);

	m_soundPropertyList = new ui::custom::AutoPropertyList();
	m_soundPropertyList->create(splitter, ui::WsDoubleBuffer | ui::custom::AutoPropertyList::WsColumnHeader);
	m_soundPropertyList->setSeparator(200);
	m_soundPropertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_soundPropertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));

	return true;
}

void SoundBatchDialog::destroy()
{
	ui::ConfigDialog::destroy();
}

bool SoundBatchDialog::showModal(RefArray< SoundAsset >& outAssets)
{
	if (ui::ConfigDialog::showModal() != ui::DrOk)
		return false;

	m_soundPropertyList->apply();

	for (int i = 0; i < m_soundList->count(); ++i)
		outAssets.push_back(checked_type_cast< SoundAsset* >(m_soundList->getData(i)));

	return true;
}

void SoundBatchDialog::addSound()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, i18n::Text(L"SOUND_BATCH_FILE_TITLE"), L"All files;*.*"))
		return;

	std::vector< Path > fileNames;
	if (fileDialog.showModal(fileNames) != ui::DrOk)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	std::wstring assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");

	for (std::vector< Path >::iterator i = fileNames.begin(); i != fileNames.end(); ++i)
	{
		Path soundPath;
		if (!FileSystem::getInstance().getRelativePath(
			FileSystem::getInstance().getAbsolutePath(*i),
			FileSystem::getInstance().getAbsolutePath(assetPath),
			soundPath
		))
			soundPath = *i;

		Ref< SoundAsset > asset = new SoundAsset();
		asset->setFileName(soundPath);

		m_soundList->add(
			i->getFileName(),
			asset
		);
	}
}

void SoundBatchDialog::removeSound()
{
	m_soundPropertyList->bind(0);
	m_soundPropertyList->update();

	for (int index = m_soundList->getSelected(); index >= 0; index = m_soundList->getSelected())
		m_soundList->remove(index);
}

void SoundBatchDialog::eventSoundListToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"SoundBatch.Add")
		addSound();
	else if (cmd == L"SoundBatch.Remove")
		removeSound();
}

void SoundBatchDialog::eventSoundListSelect(ui::SelectionChangeEvent* event)
{
	Ref< SoundAsset > asset = checked_type_cast< SoundAsset* >(m_soundList->getSelectedData());
	m_soundPropertyList->apply();
	m_soundPropertyList->bind(asset);
	m_soundPropertyList->update();
}

	}
}
