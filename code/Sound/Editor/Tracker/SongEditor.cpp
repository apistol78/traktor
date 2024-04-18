/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Resource/ResourceManager.h"
#include "Sound/AudioChannel.h"
#include "Sound/AudioResourceFactory.h"
#include "Sound/AudioSystem.h"
#include "Sound/Editor/WaveformControl.h"
#include "Sound/Editor/Tracker/SongAsset.h"
#include "Sound/Editor/Tracker/SongEditor.h"
#include "Sound/Tracker/IEventData.h"
#include "Sound/Tracker/PatternData.h"
#include "Sound/Tracker/PlayData.h"
#include "Sound/Tracker/SongBuffer.h"
#include "Sound/Tracker/TrackData.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SongEditor", SongEditor, editor::IEditorPage)

SongEditor::SongEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_currentPattern(0)
{
}

bool SongEditor::create(ui::Container* parent)
{
	m_songAsset = m_document->getObject< SongAsset >(0);
	if (!m_songAsset)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));
	container->addEventHandler< ui::TimerEvent >(this, &SongEditor::eventTimer);
	container->startTimer(10);

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(container, ui::WsDoubleBuffer);
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &SongEditor::eventToolBarClick);
	m_toolBar->addItem(new ui::ToolBarButton(L"<", ui::Command(L"SongEditor.PreviousPattern")));
	m_toolBar->addItem(new ui::ToolBarButton(L">", ui::Command(L"SongEditor.NextPattern")));
	m_toolBar->addItem(new ui::ToolBarButton(L"Play", ui::Command(L"SongEditor.Play")));
	m_toolBar->addItem(new ui::ToolBarButton(L"Stop", ui::Command(L"SongEditor.Stop")));

	m_patternGrid = new ui::GridView();
	m_patternGrid->create(container, ui::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_patternGrid->addColumn(new ui::GridColumn(L"#", 100_ut));
	for (uint32_t i = 0; i < 16; ++i)
		m_patternGrid->addColumn(new ui::GridColumn(L"Channel " + toString(i), 100_ut));

	// Get audio system for preview.
	m_audioSystem = m_editor->getObjectStore()->get< AudioSystem >();
	if (m_audioSystem)
	{
		m_audioChannel = m_audioSystem->getChannel(0);
		if (!m_audioChannel)
			m_audioSystem = nullptr;
	}
	if (!m_audioSystem)
		log::warning << L"Unable to create preview audio system; preview unavailable." << Endl;

	m_resourceManager = new resource::ResourceManager(m_editor->getOutputDatabase(), true);
	m_resourceManager->addFactory(new AudioResourceFactory());

	updateGrid();
	return true;
}

void SongEditor::destroy()
{
	if (m_audioChannel)
	{
		m_audioChannel->stop();
		m_audioChannel = nullptr;
	}

	m_resourceManager = nullptr;
	m_audioSystem = nullptr;
}

bool SongEditor::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool SongEditor::handleCommand(const ui::Command& command)
{
	if (command == L"SongEditor.PreviousPattern")
	{
		if (m_currentPattern > 0)
		{
			m_currentPattern--;
			updateGrid();
		}
	}
	else if (command == L"SongEditor.NextPattern")
	{
		m_currentPattern++;
		updateGrid();
	}
	else if (command == L"SongEditor.Play")
	{
		play();
	}
	else if (command == L"SongEditor.Stop")
	{
		stop();
	}
	else
		return false;

	return true;
}

void SongEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void SongEditor::play()
{
	RefArray< Pattern > patterns;
	for (auto patternData : m_songAsset->getPatterns())
	{
		Ref< Pattern > pattern = patternData->createInstance(m_resourceManager);
		if (!pattern)
			return;

		patterns.push_back(pattern);
	}

	m_songBuffer = new SongBuffer(patterns, m_songAsset->getBpm());

	m_audioChannel->play(
		m_songBuffer,
		0,
		1.0f,
		false,
		0
	);
}

void SongEditor::stop()
{
	m_audioChannel->stop();
	m_songBuffer = nullptr;
}

void SongEditor::updateGrid()
{
	m_patternGrid->removeAllRows();

	if (m_songAsset)
	{
		const auto& patterns = m_songAsset->getPatterns();
		if (m_currentPattern >= 0 && m_currentPattern < int32_t(patterns.size()))
		{
			const auto pattern = patterns[m_currentPattern];
			for (int32_t row = 0; row < pattern->getDuration(); ++row)
			{
				Ref< ui::GridRow > patternRow = new ui::GridRow();
				patternRow->add(toString(row));

				const auto& tracks = pattern->getTracks();
				for (size_t i = 0; i < tracks.size(); ++i)
				{
					const auto& track = tracks[i];
					const TrackData::Key* key = track->findKey(row);
					if (!key)
						continue;

					StringOutputStream ss;

					if (key->play)
					{
						const wchar_t* c_notes[] = { L"C", L"C#", L"D", L"D#", L"E", L"F", L"F#", L"G", L"G#", L"A", L"A#", L"B" };
						ss << c_notes[key->play->getNote() % sizeof_array(c_notes)] << (key->play->getNote() / sizeof_array(c_notes));
					}

					if (!key->events.empty())
						ss << L" *";

					if (!ss.empty())
						patternRow->set(1 + i, new ui::GridItem(ss.str()));
				}

				m_patternGrid->addRow(patternRow);
			}
		}
	}

	m_patternGrid->update();
}

void SongEditor::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void SongEditor::eventTimer(ui::TimerEvent* event)
{
	if (!m_songBuffer)
		return;

	const auto cursor = m_audioChannel->getCursor();
	if (!cursor)
		return;

	const int32_t pattern = m_songBuffer->getCurrentPattern(cursor);
	const int32_t row = m_songBuffer->getCurrentRow(cursor);

	if (pattern != m_currentPattern)
	{
		m_currentPattern = pattern;
		updateGrid();
	}

	m_patternGrid->deselectAll();

	if (row >= 0)
		m_patternGrid->getRow(row)->setState(ui::GridRow::Selected);

	m_patternGrid->update();
}

	}
}
