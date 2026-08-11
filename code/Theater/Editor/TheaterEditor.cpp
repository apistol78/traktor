/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterEditor.h"

#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/Events/PostFrameEvent.h"
#include "Theater/ActData.h"
#include "Theater/Performance.h"
#include "Theater/TheaterEntityComponentData.h"
#include "Theater/TheaterWorldComponentData.h"
#include "Theater/TrackData.h"
#include "Theater/Editor/TheaterEntityTarget.h"
#include "Theater/Editor/TheaterWorldTarget.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/InputDialog.h"
#include "Ui/MessageBox.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Sequencer/CursorMoveEvent.h"
#include "Ui/Sequencer/KeyMoveEvent.h"
#include "Ui/Sequencer/Marker.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/Tick.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "World/EntityData.h"

#include <algorithm>

namespace traktor::theater
{
	namespace
	{

const float c_clampKeyDistance = 1.0f / 30.0f;

class TransformPathKeyWrapper : public Object
{
public:
	TransformPath::Key& m_key;

	TransformPathKeyWrapper(TransformPath::Key& key)
	:	m_key(key)
	{
	}
};

class EventKeyWrapper : public Object
{
public:
	TrackData::EventKey& m_key;

	EventKeyWrapper(TrackData::EventKey& key)
	:	m_key(key)
	{
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterEditor", TheaterEditor, scene::IComponentPanelEditor)

std::wstring TheaterEditor::getTitle() const
{
	return i18n::Text(L"THEATER_EDITOR_TITLE");
}

bool TheaterEditor::create(scene::SceneEditorContext* context, ui::Container* parent)
{
	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	m_dropDownTargets = new ui::ToolBarDropDown(ui::Command(L"Theater.SelectTarget"), 200_ut, i18n::Text(L"THEATER_EDITOR_TARGETS"));
	m_dropDownActs = new ui::ToolBarDropDown(ui::Command(L"Theater.SelectAct"), 150_ut, i18n::Text(L"THEATER_EDITOR_ACTS"));

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(m_container);
	for (int32_t i = 0; i < 8; ++i)
		m_toolBar->addImage(new ui::StyleBitmap(L"Theater.Theater", i));
	m_toolBar->addItem(m_dropDownTargets);
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_ADD_ACT"), 6, ui::Command(L"Theater.AddAct")));
	m_toolBar->addItem(m_dropDownActs);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_RENAME_ACT"), 6, ui::Command(L"Theater.RenameAct")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_REMOVE_ACT"), 7, ui::Command(L"Theater.RemoveAct")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_CAPTURE_ENTITIES"), 0, ui::Command(L"Theater.CaptureEntities")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_DELETE_SELECTED_KEY"), 1, ui::Command(L"Theater.DeleteSelectedKey")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_SET_LOOKAT_ENTITY"), 4, ui::Command(L"Theater.SetLookAtEntity")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_EASE_VELOCITY"), 5, ui::Command(L"Theater.EaseVelocity")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_GOTO_PREVIOUS_KEY"), 2, ui::Command(L"Theater.GotoPreviousKey")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_GOTO_NEXT_KEY"), 3, ui::Command(L"Theater.GotoNextKey")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_SPLIT_ACT"), 2, ui::Command(L"Theater.SplitAct")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_TIME_SCALE_ACT"), 2, ui::Command(L"Theater.TimeScaleAct")));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &TheaterEditor::eventToolBarClick);

	m_trackSequencer = new ui::SequencerControl();
	if (!m_trackSequencer->create(m_container, ui::WsAccelerated))
		return false;

	m_trackSequencer->addEventHandler< ui::CursorMoveEvent >(this, &TheaterEditor::eventSequencerCursorMove);
	m_trackSequencer->addEventHandler< ui::KeyMoveEvent >(this, &TheaterEditor::eventSequencerKeyMove);

	m_context = context;
	m_eventHandlerPostFrame = m_context->addEventHandler< scene::PostFrameEvent >(this, &TheaterEditor::eventContextPostFrame);
	m_buildCount = m_context->getBuildCount();

	updateTargets();
	updateView();
	return true;
}

void TheaterEditor::destroy()
{
	if (m_context)
	{
		m_context->removeEventHandler(m_eventHandlerPostFrame);
		m_eventHandlerPostFrame = nullptr;
		m_context = nullptr;
	}

	m_targets.resize(0);

	safeDestroy(m_trackSequencer);
	safeDestroy(m_toolBar);
	safeDestroy(m_container);
}

void TheaterEditor::entityRemoved(scene::EntityAdapter* entityAdapter)
{
	// Remove tracks of the removed entity from all theater components.
	for (auto target : m_targets)
	{
		for (auto act : target->getActs())
		{
			RefArray< TrackData >& tracks = act->getTracks();
			for (auto it = tracks.begin(); it != tracks.end(); )
			{
				if ((*it)->getEntityId() == entityAdapter->getId())
					it = tracks.erase(it);
				else
					++it;
			}
		}
	}
	updateView();
}

void TheaterEditor::propertiesChanged()
{
	updateTargets();

	if (TheaterEditTarget* target = getTarget())
		target->rebuild();

	updateView();
}

bool TheaterEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Theater.SelectTarget" || command == L"Theater.SelectAct")
	{
		m_trackSequencer->setCursor(0);
		m_context->setTime(0.0f);
		m_context->setPhysicsEnable(false);
		m_context->setPlaying(false);
		updateView();

		if (getAct() == nullptr)
		{
			// No act selected; stop performance and rebuild entities so they are
			// placed as authored instead of as last evaluated.
			if (TheaterEditTarget* target = getTarget())
			{
				if (Performance* performance = target->getPerformance())
					performance->stop();
			}
			m_context->buildEntities(false);
		}

		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Theater.AddAct")
	{
		TheaterEditTarget* target = getTarget();
		if (!target)
			return true;

		target->getActs().push_back(new ActData());
		rebuildTarget();
		updateView();
	}
	else if (command == L"Theater.RenameAct")
	{
		ActData* act = getAct();
		if (!act)
			return true;

		ui::InputDialog::Field fields[] =
		{
			ui::InputDialog::Field(i18n::Text(L"THEATER_EDITOR_RENAME_ACT_NAME"))
		};

		ui::InputDialog inputDialog;
		inputDialog.create(
			m_toolBar,
			i18n::Text(L"THEATER_EDITOR_RENAME_ACT_TITLE"),
			i18n::Text(L"THEATER_EDITOR_RENAME_ACT_MESSAGE"),
			fields,
			sizeof_array(fields)
		);
		if (inputDialog.showModal() == ui::DialogResult::Ok && !fields[0].value.empty())
		{
			act->setName(fields[0].value);
			updateView();
		}
		inputDialog.destroy();
	}
	else if (command == L"Theater.RemoveAct")
	{
		TheaterEditTarget* target = getTarget();
		ActData* act = getAct();
		if (!target || !act)
			return true;

		const ui::DialogResult result = ui::MessageBox::show(
			m_toolBar,
			i18n::Text(L"THEATER_EDITOR_MESSAGE_REMOVE_ACT"),
			i18n::Text(L"THEATER_EDITOR_TITLE_REMOVE_ACT"),
			ui::MbYesNo | ui::MbIconExclamation
		);
		if (result == ui::DialogResult::Yes)
		{
			RefArray< ActData >& acts = target->getActs();
			const auto it = std::find_if(acts.begin(), acts.end(), [&](const ActData* actData) {
				return actData == act;
			});
			if (it != acts.end())
			{
				acts.erase(it);
				rebuildTarget();
				updateView();
			}
		}
	}
	else if (command == L"Theater.CaptureEntities")
	{
		captureEntities();
		updateView();
	}
	else if (command == L"Theater.DeleteSelectedKey")
	{
		deleteSelectedKey();
		updateView();
	}
	else if (command == L"Theater.SetLookAtEntity")
		setLookAtEntity();
	else if (command == L"Theater.EaseVelocity")
		easeVelocity();
	else if (command == L"Theater.GotoPreviousKey")
		gotoPreviousKey();
	else if (command == L"Theater.GotoNextKey")
		gotoNextKey();
	else if (command == L"Theater.SplitAct")
		splitAct();
	else if (command == L"Theater.TimeScaleAct")
		timeScaleAct();
	else
		return false;

	return true;
}

void TheaterEditor::update()
{
	// Entity adapters are recreated for each build of the scene, thus all
	// targets need to be collected again.
	if (m_buildCount != m_context->getBuildCount())
	{
		m_buildCount = m_context->getBuildCount();
		updateTargets();
		updateView();
	}

	TheaterEditTarget* target = getTarget();
	if (!target)
		return;

	Performance* performance = target->getPerformance();
	if (performance == nullptr)
		return;

	if (const ActData* act = getAct())
	{
		// Ensure component is up to date, we need to force it to keep in sync with editor.
		performance->preview(act->getName());
	}
	else
	{
		// No act selected; leave entities alone so they can be edited.
		performance->stop();
	}
}

void TheaterEditor::draw(render::PrimitiveRenderer* primitiveRenderer)
{
	TheaterEditTarget* target = getTarget();
	const ActData* act = getAct();
	if (!target || !act)
		return;

	// All tracks of the target are relative this transform.
	const Transform base = target->getBaseTransform();

	RefArray< ui::SequenceItem > items = m_trackSequencer->getSequenceItems(ui::SequencerControl::GfSelectedOnly);

	const int32_t cursorTick = m_trackSequencer->getCursor();
	const float cursorTime = float(cursorTick / 1000.0f);
	const float duration = act->getDuration();

	for (auto track : act->getTracks())
	{
		Color4ub pathColor(180, 180, 80, 120);
		for (auto item : items)
		{
			if (item->getData(L"TRACK") == track)
			{
				pathColor = Color4ub(255, 255, 0, 200);
				break;
			}
		}

		const TransformPath& path = track->getPath();
		const int32_t steps = int32_t(duration) * 10;

		TransformPath::Key F0 = path.evaluate(0.0f, false);
		for (int32_t i = 1; i <= steps; ++i)
		{
			const float T = (float(i) / steps) * duration;
			const TransformPath::Key F1 = path.evaluate(T, false);

			primitiveRenderer->drawLine(
				base * F0.position,
				base * F1.position,
				pathColor
			);

			F0 = F1;
		}

		for (int32_t i = 0; i <= steps; ++i)
		{
			const float T = (float(i) / steps) * duration;
			const TransformPath::Key F = path.evaluate(T, false);

			primitiveRenderer->drawSolidPoint(
				base * F.position,
				4.0f,
				Color4ub(255, 255, 255, 200)
			);
		}

		for (const auto& key : path.keys())
		{
			primitiveRenderer->drawSolidPoint(
				base * key.position,
				8.0f,
				pathColor
			);
		}

		const TransformPath::Key F = path.evaluate(cursorTime, false);
		primitiveRenderer->drawWireFrame(
			(base * F.transform()).toMatrix44(),
			1.0f
		);
	}
}

void TheaterEditor::updateTargets()
{
	// Keep selected target across rebuilds of the scene.
	Guid selectedId;
	if (TheaterEditTarget* target = getTarget())
		selectedId = target->getId();

	m_targets.resize(0);

	// Theater component of the world; animate any entity of the scene.
	if (scene::SceneAsset* sceneAsset = m_context->getSceneAsset())
	{
		if (auto componentData = sceneAsset->getWorldComponent< TheaterWorldComponentData >())
			m_targets.push_back(new TheaterWorldTarget(m_context, componentData));
	}

	// Theater components of entities; animate children of respective entity.
	for (auto entityAdapter : m_context->getEntities())
	{
		if (auto componentData = entityAdapter->getComponentData< TheaterEntityComponentData >())
			m_targets.push_back(new TheaterEntityTarget(entityAdapter, componentData));
	}

	int32_t selected = -1;
	m_dropDownTargets->removeAll();
	for (auto target : m_targets)
	{
		const int32_t index = m_dropDownTargets->add(target->getName(), target);
		if (target->getId() == selectedId)
			selected = index;
	}

	if (selected < 0 && !m_targets.empty())
		selected = 0;

	m_dropDownTargets->select(selected);
	m_toolBar->update();
}

TheaterEditTarget* TheaterEditor::getTarget() const
{
	const int32_t selected = m_dropDownTargets->getSelected();
	if (selected >= 0 && selected < (int32_t)m_targets.size())
		return m_targets[selected];
	else
		return nullptr;
}

void TheaterEditor::rebuildTarget()
{
	if (TheaterEditTarget* target = getTarget())
		target->rebuild();

	// Acts have been modified outside of the property view; it needs to be
	// refreshed or else it will write back stale values when edited.
	m_refreshProperties = true;
	m_context->enqueueRedraw(nullptr);
}

ActData* TheaterEditor::getAct() const
{
	const int32_t selected = m_dropDownActs->getSelected();
	if (selected < 0)
		return nullptr;

	// First item of the drop down is "<None>" and has no act associated.
	return dynamic_type_cast< ActData* >(m_dropDownActs->getData(selected));
}

void TheaterEditor::updateView()
{
	TheaterEditTarget* target = getTarget();

	// Keep selection across updates of the view; removeAll clear the selection
	// so it must be read before the drop down is repopulated.
	const Ref< ActData > selectedAct = getAct();
	const bool noActSelected = (m_dropDownActs->getSelected() == 0);

	m_dropDownActs->removeAll();
	m_trackSequencer->removeAllSequenceItems();

	if (!target)
	{
		m_trackSequencer->update();
		return;
	}

	// Let user select no act at all in order to edit entities without the
	// performance placing them.
	m_dropDownActs->add(i18n::Text(L"THEATER_EDITOR_NO_ACT"), nullptr);

	RefArray< ActData >& acts = target->getActs();

	int32_t selected = -1;
	for (auto act : acts)
	{
		std::wstring actName = act->getName();
		if (actName.empty())
			actName = i18n::Text(L"THEATER_EDITOR_UNNAMED_ACT");

		const int32_t index = m_dropDownActs->add(actName, act);
		if (act == selectedAct)
			selected = index;
	}

	// Default to the first act, unless user has explicitly selected no act.
	if (selected < 0)
		selected = (noActSelected || acts.empty()) ? 0 : 1;

	m_dropDownActs->select(selected);

	if (ActData* act = getAct())
	{
		for (auto track : act->getTracks())
		{
			const scene::EntityAdapter* entityAdapter = target->findEntityAdapter(track->getEntityId());
			const std::wstring name = (entityAdapter != nullptr) ? entityAdapter->getName() : L"[N/A]";

			Ref< ui::Sequence > trackSequence = new ui::Sequence(name);
			trackSequence->setData(L"TRACK", track);

			for (auto& key : track->getPath().editKeys())
			{
				const int32_t tickTime = (int32_t)(key.T * 1000.0f);
				Ref< ui::Tick > tick = new ui::Tick(tickTime, true);
				tick->setData(L"KEY", new TransformPathKeyWrapper(key));
				trackSequence->addKey(tick);
			}

			// Events are shown as markers in order to tell them apart from
			// the transform keys of the track.
			for (auto& eventKey : track->getEvents())
			{
				const int32_t markerTime = (int32_t)(eventKey.T * 1000.0f);
				Ref< ui::Marker > marker = new ui::Marker(markerTime, true);
				marker->setData(L"EVENT", new EventKeyWrapper(eventKey));
				trackSequence->addKey(marker);
			}

			m_trackSequencer->addSequenceItem(trackSequence);
		}

		m_trackSequencer->setLength((int32_t)(act->getDuration() * 1000.0f));
		m_trackSequencer->setCursor((int32_t)(m_context->getTime() * 1000.0f));
	}

	m_trackSequencer->update();
}

void TheaterEditor::captureEntities()
{
	TheaterEditTarget* target = getTarget();
	ActData* act = getAct();
	if (!target || !act)
	{
		log::warning << L"Unable to capture entities; no act selected." << Endl;
		return;
	}

	RefArray< scene::EntityAdapter > selectedEntities = m_context->getEntities(scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);
	if (selectedEntities.empty())
	{
		log::warning << L"Unable to capture entities; no entities selected." << Endl;
		return;
	}

	// Tracks are stored relative the target's base transform.
	const Transform baseInv = target->getBaseTransform().inverse();

	const double time = m_context->getTime();

	RefArray< TrackData >& tracks = act->getTracks();
	for (auto selectedEntity : selectedEntities)
	{
		if (!target->canCapture(selectedEntity))
		{
			log::warning << L"Unable to capture \"" << selectedEntity->getName() << L"\"; not animated by selected theater component." << Endl;
			continue;
		}

		const Guid& selectedId = selectedEntity->getId();
		const Transform transform = baseInv * selectedEntity->getTransform();

		Ref< TrackData > instanceTrackData;

		auto it = std::find_if(tracks.begin(), tracks.end(), [&](const TrackData* trackData) {
			return trackData->getEntityId() == selectedId;
		});
		if (it != tracks.end())
			instanceTrackData = *it;
		else
		{
			instanceTrackData = new TrackData();
			instanceTrackData->setEntityId(selectedId);
			tracks.push_back(instanceTrackData);
		}

		T_ASSERT(instanceTrackData);
		TransformPath& pathData = instanceTrackData->getPath();

		const int32_t cki = pathData.getClosestKey(time);
		if (cki >= 0 && abs(pathData.get(cki).T - time) < c_clampKeyDistance)
		{
			TransformPath::Key closestKey = pathData.get(cki);
			closestKey.position = transform.translation();
			closestKey.orientation = transform.rotation().toEulerAngles();
			pathData.set(cki, closestKey);
		}
		else
		{
			TransformPath::Key key;
			key.T = time;
			key.position = transform.translation();
			key.orientation = transform.rotation().toEulerAngles();
			pathData.insert(key);
		}
	}

	rebuildTarget();
}

void TheaterEditor::deleteSelectedKey()
{
	TheaterEditTarget* target = getTarget();
	if (!target)
		return;

	for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfSelectedOnly | ui::SequencerControl::GfDescendants))
	{
		ui::Sequence* selectedSequence = checked_type_cast< ui::Sequence*, false >(sequenceItem);
		ui::Key* selectedKey = selectedSequence->getSelectedKey();
		if (!selectedKey)
			continue;

		Ref< TrackData > trackData = selectedSequence->getData< TrackData >(L"TRACK");
		T_ASSERT(trackData);

		if (TransformPathKeyWrapper* keyWrapper = selectedKey->getData< TransformPathKeyWrapper >(L"KEY"))
		{
			TransformPath& path = trackData->getPath();
			AlignedVector< TransformPath::Key >& keys = path.editKeys();
			for (auto it = keys.begin(); it != keys.end(); ++it)
			{
				if (&(*it) == &keyWrapper->m_key)
				{
					selectedSequence->removeKey(selectedKey);
					keys.erase(it);
					break;
				}
			}
		}
		else if (EventKeyWrapper* eventWrapper = selectedKey->getData< EventKeyWrapper >(L"EVENT"))
		{
			AlignedVector< TrackData::EventKey >& events = trackData->getEvents();
			for (auto it = events.begin(); it != events.end(); ++it)
			{
				if (&(*it) == &eventWrapper->m_key)
				{
					selectedSequence->removeKey(selectedKey);
					events.erase(it);
					break;
				}
			}
		}
	}

	rebuildTarget();
}

void TheaterEditor::setLookAtEntity()
{
	TheaterEditTarget* target = getTarget();
	if (!target)
		return;

	RefArray< scene::EntityAdapter > selectedEntities = m_context->getEntities(scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);
	if (selectedEntities.size() > 1)
		return;

	// Look-at entity must be animated by the same theater component as it is
	// resolved through the same set of entities.
	if (!selectedEntities.empty() && !target->canCapture(selectedEntities[0]))
	{
		log::warning << L"Unable to look at \"" << selectedEntities[0]->getName() << L"\"; not animated by selected theater component." << Endl;
		return;
	}

	for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfSelectedOnly | ui::SequencerControl::GfDescendants))
	{
		ui::Sequence* selectedSequence = checked_type_cast< ui::Sequence*, false >(sequenceItem);
		TrackData* trackData = selectedSequence->getData< TrackData >(L"TRACK");
		T_ASSERT(trackData);

		if (!selectedEntities.empty())
			trackData->setLookAtEntityId(selectedEntities[0]->getId());
		else
			trackData->setLookAtEntityId(Guid::null);
	}

	rebuildTarget();
}

void TheaterEditor::easeVelocity()
{
	TheaterEditTarget* target = getTarget();
	if (!target)
		return;

	for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfSelectedOnly | ui::SequencerControl::GfDescendants))
	{
		ui::Sequence* selectedSequence = checked_type_cast< ui::Sequence*, false >(sequenceItem);
		TrackData* trackData = selectedSequence->getData< TrackData >(L"TRACK");
		T_ASSERT(trackData);

		TransformPath& path = trackData->getPath();
		AlignedVector< TransformPath::Key >& keys = path.editKeys();
		if (keys.size() < 3)
			continue;

		const float Ts = keys.front().T;
		const float Te = keys.back().T;

		// Measure euclidean distance of keys.
		AlignedVector< float > distances(keys.size(), 0.0f);
		float totalDistance = 0.0f;

		for (uint32_t i = 1; i < keys.size(); ++i)
		{
			const float T0 = keys[i - 1].T;
			const float T1 = keys[i].T;

			const float c_measureStep = 1.0f / 1000.0f;
			for (float T = T0; T <= T1 - c_measureStep; T += c_measureStep)
			{
				const TransformPath::Key Fc = path.evaluate(T, false);
				const TransformPath::Key Fn = path.evaluate(T + c_measureStep, false);
				totalDistance += (Fn.position - Fc.position).length();
			}

			distances[i] = totalDistance;
		}

		// Distribute keys according to distances in time.
		const float c_smoothFactor = 0.1f;
		for (uint32_t i = 1; i < keys.size(); ++i)
			keys[i].T = lerp(keys[i].T, Ts + (distances[i] / totalDistance) * (Te - Ts), c_smoothFactor);
	}

	updateView();
	rebuildTarget();
}

void TheaterEditor::gotoPreviousKey()
{
	const ActData* act = getAct();
	if (!act)
	{
		log::warning << L"Unable to goto key entities; no act selected" << Endl;
		return;
	}

	const double time = m_context->getTime();
	double previousTime = 0.0;

	for (auto track : act->getTracks())
	{
		const TransformPath& path = track->getPath();
		const int32_t pki = path.getClosestPreviousKey(time);
		if (pki >= 0)
		{
			if (path.get(pki).T > previousTime)
				previousTime = path.get(pki).T;
		}
	}

	const int32_t cursorTick = (int32_t)(previousTime * 1000.0);

	m_trackSequencer->setCursor(cursorTick);
	m_trackSequencer->update();

	m_context->setTime(previousTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);
}

void TheaterEditor::gotoNextKey()
{
	const ActData* act = getAct();
	if (!act)
	{
		log::warning << L"Unable to goto key entities; no act selected" << Endl;
		return;
	}

	const double time = m_context->getTime();
	double nextTime = act->getDuration();

	for (auto track : act->getTracks())
	{
		const TransformPath& path = track->getPath();
		const int32_t nki = path.getClosestNextKey(time);
		if (nki >= 0)
		{
			if (path.get(nki).T < nextTime)
				nextTime = path.get(nki).T;
		}
	}

	const int32_t cursorTick = int32_t(nextTime * 1000.0);

	m_trackSequencer->setCursor(cursorTick);
	m_trackSequencer->update();

	m_context->setTime(nextTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);
}

void TheaterEditor::splitAct()
{
	TheaterEditTarget* target = getTarget();
	Ref< ActData > actLeft = getAct();
	if (!target || !actLeft)
		return;

	const int32_t cursorTick = m_trackSequencer->getCursor();
	const float cursorTime = float(cursorTick / 1000.0f);
	if (cursorTime < FUZZY_EPSILON)
		return;

	RefArray< ActData >& acts = target->getActs();
	const auto it = std::find_if(acts.begin(), acts.end(), [&](const ActData* actData) {
		return actData == actLeft;
	});
	if (it == acts.end())
		return;

	const float actTotalDuration = actLeft->getDuration();
	const std::wstring actName = actLeft->getName();

	actLeft->setName(actName + L" (Left)");
	actLeft->setDuration(cursorTime);

	Ref< ActData > actRight = new ActData();
	actRight->setName(actName + L" (Right)");
	actRight->setDuration(actTotalDuration - cursorTime);

	// Remove looping and split paths.
	for (size_t i = 0; i < actLeft->getTracks().size(); ++i)
	{
		TrackData* trackLeft = actLeft->getTracks().at(i);

		Ref< TrackData > trackRight = new TrackData();
		trackRight->setEntityId(trackLeft->getEntityId());
		trackRight->setLookAtEntityId(trackLeft->getLookAtEntityId());

		TransformPath pathLeft, pathRight;
		trackLeft->getPath().split(cursorTime, pathLeft, pathRight);
		trackLeft->setPath(pathLeft);
		trackRight->setPath(pathRight);

		actRight->getTracks().push_back(trackRight);
	}

	// Add new act after splitted act.
	acts.insert(it + 1, actRight);

	// Update UI and scene editor.
	m_context->setTime(cursorTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);

	updateView();

	rebuildTarget();
}

void TheaterEditor::timeScaleAct()
{
	TheaterEditTarget* target = getTarget();
	ActData* act = getAct();
	if (!target || !act)
		return;

	const float fromDuration = act->getDuration();

	ui::InputDialog::Field fields[] =
	{
		ui::InputDialog::Field(i18n::Text(L"THEATER_EDITOR_TIME_SCALE_NEW_TIME"), toString(fromDuration), new ui::NumericEditValidator(true, 0.0f))
	};

	ui::InputDialog enterTimeDialog;
	enterTimeDialog.create(m_toolBar, i18n::Text(L"THEATER_EDITOR_TIME_SCALE_TITLE"), i18n::Text(L"THEATER_EDITOR_TIME_SCALE_MESSAGE"), fields, sizeof_array(fields));

	if (enterTimeDialog.showModal() == ui::DialogResult::Ok)
	{
		const float toDuration = parseString< float >(fields[0].value);
		const float f = toDuration / fromDuration;

		for (auto track : act->getTracks())
		{
			auto& path = track->getPath();
			for (size_t i = 0; i < path.size(); ++i)
			{
				auto k = path.get(i);
				k.T *= f;
				path.set(i, k);
			}
		}

		act->setDuration(toDuration);
		updateView();
		rebuildTarget();
	}

	enterTimeDialog.destroy();
}

void TheaterEditor::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void TheaterEditor::eventSequencerCursorMove(ui::CursorMoveEvent* event)
{
	const int32_t cursorTick = m_trackSequencer->getCursor();
	const float cursorTime = cursorTick / 1000.0f;

	m_context->setTime(cursorTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);
	m_context->raiseRedraw(nullptr);
}

void TheaterEditor::eventSequencerKeyMove(ui::KeyMoveEvent* event)
{
	bool moved = false;

	if (const ui::Tick* tick = dynamic_type_cast< ui::Tick* >(event->getKey()))
	{
		TransformPathKeyWrapper* keyWrapper = tick->getData< TransformPathKeyWrapper >(L"KEY");
		T_ASSERT(keyWrapper);
		keyWrapper->m_key.T = tick->getTime() / 1000.0f;
		moved = true;
	}
	else if (const ui::Marker* marker = dynamic_type_cast< ui::Marker* >(event->getKey()))
	{
		EventKeyWrapper* eventWrapper = marker->getData< EventKeyWrapper >(L"EVENT");
		T_ASSERT(eventWrapper);
		eventWrapper->m_key.T = marker->getTime() / 1000.0f;
		moved = true;
	}

	if (moved)
		rebuildTarget();
}

void TheaterEditor::eventContextPostFrame(scene::PostFrameEvent* event)
{
	if (m_refreshProperties)
	{
		m_refreshProperties = false;
		m_context->raisePostModify();
	}

	const double cursorTime = m_context->getTime();
	const int32_t cursorTick = (int32_t)(cursorTime * 1000.0);
	if (m_trackSequencer->getCursor() != cursorTick)
	{
		m_trackSequencer->setCursor(cursorTick);
		m_trackSequencer->update();
	}
}

}
