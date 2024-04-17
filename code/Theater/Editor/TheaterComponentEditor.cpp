/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Scene.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/Events/PostFrameEvent.h"
#include "Theater/ActData.h"
#include "Theater/TheaterComponent.h"
#include "Theater/TheaterComponentData.h"
#include "Theater/TrackData.h"
#include "Theater/Editor/TheaterComponentEditor.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/MessageBox.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/InputDialog.h"
#include "Ui/Splitter.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/Sequencer/CursorMoveEvent.h"
#include "Ui/Sequencer/KeyMoveEvent.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/Tick.h"
#include "World/EntityData.h"
#include "World/World.h"

namespace traktor::theater
{
	namespace
	{

const float c_clampKeyDistance = 1.0f / 30.0f;
const float c_velocityScale = 0.2f;

class TransformPathKeyWrapper : public Object
{
public:
	TransformPath::Key& m_key;

	TransformPathKeyWrapper(TransformPath::Key& key)
	:	m_key(key)
	{
	}
};

Vector4 fixupOrientation(const Vector4& orientation, const Vector4& reference)
{
	Vector4 out(0.0f, 0.0f, 0.0f, 0.0f);
	for (int32_t i = 0; i < 3; ++i)
	{
		const float r = reference[i];
		float a = orientation[i];

		const float d1 = abs(a - r);
		const float d2 = abs(a + TWO_PI - r);
		const float d3 = abs(a - TWO_PI - r);

		if (d2 < d1 && d2 < d3)
			a += TWO_PI;
		else if (d3 < d1 && d3 < d2)
			a -= TWO_PI;

		out.set(i, Scalar(a));
	}
	return out;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterComponentEditor", TheaterComponentEditor, scene::IWorldComponentEditor)

bool TheaterComponentEditor::create(scene::SceneEditorContext* context, ui::Container* parent)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	m_dropDownActs = new ui::ToolBarDropDown(ui::Command(L"Theater.SelectAct"), 150_ut, i18n::Text(L"THEATER_EDITOR_ACTS"));

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(container);
	for (int32_t i = 0; i < 8; ++i)
		m_toolBar->addImage(new ui::StyleBitmap(L"Theater.Theater", i));
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
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &TheaterComponentEditor::eventToolBarClick);

	m_trackSequencer = new ui::SequencerControl();
	if (!m_trackSequencer->create(container, ui::WsAccelerated))
		return false;

	m_trackSequencer->addEventHandler< ui::CursorMoveEvent >(this, &TheaterComponentEditor::eventSequencerCursorMove);
	m_trackSequencer->addEventHandler< ui::KeyMoveEvent >(this, &TheaterComponentEditor::eventSequencerKeyMove);

	m_context = context;
	m_context->addEventHandler< scene::PostFrameEvent >(this, &TheaterComponentEditor::eventContextPostFrame);

	updateView();
	return true;
}

void TheaterComponentEditor::destroy()
{
	safeDestroy(m_trackSequencer);
	safeDestroy(m_toolBar);
}

void TheaterComponentEditor::entityRemoved(scene::EntityAdapter* entityAdapter)
{
	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
	for (auto act : componentData->getActs())
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
	updateView();
}

void TheaterComponentEditor::propertiesChanged()
{
	m_context->buildController();
	updateView();
}

bool TheaterComponentEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Theater.SelectAct")
	{
		m_trackSequencer->setCursor(0);
		m_context->setTime(0.0f);
		m_context->setPhysicsEnable(false);
		m_context->setPlaying(false);
		updateView();
	}
	else if (command == L"Theater.AddAct")
	{
		TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
		componentData->getActs().push_back(new ActData());
		m_context->buildController();
		updateView();
	}
	else if (command == L"Theater.RenameAct")
	{
		const int32_t selected = m_dropDownActs->getSelected();
		if (selected >= 0)
		{
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
				TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
				ActData* act = componentData->getActs().at(selected);
				act->setName(fields[0].value);
				updateView();
			}
			inputDialog.destroy();
		}
	}
	else if (command == L"Theater.RemoveAct")
	{
		const int32_t selected = m_dropDownActs->getSelected();
		if (selected >= 0)
		{
			const ui::DialogResult result = ui::MessageBox::show(
				m_toolBar,
				i18n::Text(L"THEATER_EDITOR_MESSAGE_REMOVE_ACT"),
				i18n::Text(L"THEATER_EDITOR_TITLE_REMOVE_ACT"),
				ui::MbYesNo | ui::MbIconExclamation
			);
			if (result == ui::DialogResult::Yes)
			{
				TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
				RefArray< ActData >& acts = componentData->getActs();
				acts.erase(acts.begin() + selected);
				m_context->buildController();
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

void TheaterComponentEditor::update()
{
	const int32_t selected = m_dropDownActs->getSelected();
	if (selected < 0)
		return;

	// Ensure controller is up to date, we need to force it to keep in sync with editor.
	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
	TheaterComponent* component = m_context->getScene()->getWorld()->getComponent< TheaterComponent >();
	if (!componentData || !component)
		return;

	component->play(componentData->getActs().at(selected)->getName());
	component->m_timeStart = 0.0f;
}

void TheaterComponentEditor::draw(render::PrimitiveRenderer* primitiveRenderer)
{
	const int32_t selected = m_dropDownActs->getSelected();
	if (selected < 0)
		return;

	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
	ActData* act = componentData->getActs().at(selected);

	RefArray< ui::SequenceItem > items = m_trackSequencer->getSequenceItems(ui::SequencerControl::GfSelectedOnly);

	int32_t cursorTick = m_trackSequencer->getCursor();
	float cursorTime = float(cursorTick / 1000.0f);
	float duration = act->getDuration();

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
				F0.position,
				F1.position,
				pathColor
			);

			F0 = F1;
		}

		for (int32_t i = 0; i <= steps; ++i)
		{
			const float T = (float(i) / steps) * duration;
			const TransformPath::Key F0 = path.evaluate(T, false);

			primitiveRenderer->drawSolidPoint(
				F0.position,
				4.0f,
				Color4ub(255, 255, 255, 200)
			);
		}

		for (const auto& key : path.keys())
		{
			primitiveRenderer->drawSolidPoint(
				key.position,
				8.0f,
				pathColor
			);
		}

		const TransformPath::Key F = path.evaluate(cursorTime, false);
		primitiveRenderer->drawWireFrame(
			F.transform().toMatrix44(),
			1.0f
		);
	}
}

void TheaterComponentEditor::updateView()
{
	scene::SceneAsset* sceneAsset = m_context->getSceneAsset();
	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();

	RefArray< ActData >& acts = componentData->getActs();

	int32_t selected = m_dropDownActs->getSelected();
	if (selected >= (int32_t)acts.size())
		selected = (int32_t)acts.size() - 1;
	else if (selected < 0 && !acts.empty())
		selected = 0;

	m_dropDownActs->removeAll();
	for (auto act : acts)
	{
		std::wstring actName = act->getName();
		if (actName.empty())
			actName = i18n::Text(L"THEATER_EDITOR_UNNAMED_ACT");
		m_dropDownActs->add(actName, act);
	}

	m_dropDownActs->select(selected);

	m_trackSequencer->removeAllSequenceItems();

	if (selected >= 0)
	{
		RefArray< scene::EntityAdapter > entities = m_context->getEntities();
		for (auto track : acts[selected]->getTracks())
		{
			auto it = std::find_if(entities.begin(), entities.end(), [&](scene::EntityAdapter* entity) {
				return entity->getId() == track->getEntityId();
				});

			std::wstring name;
			if (it != entities.end())
				name = it->getName();
			else
				name = L"[N/A]";

			Ref< ui::Sequence > trackSequence = new ui::Sequence(name);
			trackSequence->setData(L"TRACK", track);

			for (auto& key : track->getPath().editKeys())
			{
				const int32_t tickTime = (int32_t)(key.T * 1000.0f);
				Ref< ui::Tick > tick = new ui::Tick(tickTime, true);
				tick->setData(L"KEY", new TransformPathKeyWrapper(key));
				trackSequence->addKey(tick);
			}

			m_trackSequencer->addSequenceItem(trackSequence);
		}

		m_trackSequencer->setLength((int32_t)(acts[selected]->getDuration() * 1000.0f));
		m_trackSequencer->setCursor((int32_t)(m_context->getTime() * 1000.0f));
	}

	m_trackSequencer->update();
}

void TheaterComponentEditor::captureEntities()
{
	const int32_t selected = m_dropDownActs->getSelected();
	if (selected < 0)
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

	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
	ActData* act = componentData->getActs().at(selected);

	const double time = m_context->getTime();

	RefArray< TrackData >& tracks = act->getTracks();
	for (auto selectedEntity : selectedEntities)
	{
		const Guid& selectedId = selectedEntity->getId();
		Transform transform = selectedEntity->getTransform();

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

	 		// Ensure orientation are "logically" fixed up to previous key.
	 		// if (cki > 0)
	 		// {
	 		// 	pathData[cki].orientation = fixupOrientation(
	 		// 		pathData[cki].orientation,
	 		// 		pathData[cki - 1].orientation
	 		// 	);
	 		// }

	 		pathData.set(cki, closestKey);
		}
		else
		{
	 		TransformPath::Key key;
	 		key.T = time;
	 		key.position = transform.translation();
	 		key.orientation = transform.rotation().toEulerAngles();
	 		const size_t at = pathData.insert(key);

	 		// Ensure orientation are "logically" fixed up to previous key.
	 		// if (at > 0)
	 		// {
	 		// 	pathData[at].orientation = fixupOrientation(
	 		// 		pathData[at].orientation,
	 		// 		pathData[at - 1].orientation
	 		// 	);
	 		// }
		}
	}

	m_context->buildController();
}

void TheaterComponentEditor::deleteSelectedKey()
{
	for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfSelectedOnly | ui::SequencerControl::GfDescendants))
	{
		ui::Sequence* selectedSequence = checked_type_cast< ui::Sequence*, false >(sequenceItem);
		ui::Tick* selectedTick = checked_type_cast< ui::Tick*, true >(selectedSequence->getSelectedKey());
		if (!selectedTick)
			continue;

		Ref< TrackData > trackData = selectedSequence->getData< TrackData >(L"TRACK");
		T_ASSERT(trackData);

		TransformPathKeyWrapper* keyWrapper = selectedTick->getData< TransformPathKeyWrapper >(L"KEY");
		T_ASSERT(keyWrapper);

		TransformPath& path = trackData->getPath();
		AlignedVector< TransformPath::Key >& keys = path.editKeys();
		for (auto it = keys.begin(); it != keys.end(); ++it)
		{
			if (&(*it) == &keyWrapper->m_key)
			{
				selectedSequence->removeKey(selectedTick);
				keys.erase(it);
				break;
			}
		}
	}
	m_context->buildController();
}

void TheaterComponentEditor::setLookAtEntity()
{
	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();

	RefArray< scene::EntityAdapter > selectedEntities = m_context->getEntities(scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);
	if (selectedEntities.size() > 1)
		return;

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

	m_context->buildController();
}

void TheaterComponentEditor::easeVelocity()
{
	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();

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
		std::vector< float > distances(keys.size(), 0.0f);
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
	m_context->buildController();
}

void TheaterComponentEditor::gotoPreviousKey()
{
	const int32_t selected = m_dropDownActs->getSelected();
	if (selected < 0)
	{
		log::warning << L"Unable to goto key entities; no act selected" << Endl;
		return;
	}

	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
	Ref< ActData > act = componentData->getActs().at(selected);

	double time = m_context->getTime();
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

void TheaterComponentEditor::gotoNextKey()
{
	const int32_t selected = m_dropDownActs->getSelected();
	if (selected < 0)
	{
		log::warning << L"Unable to goto key entities; no act selected" << Endl;
		return;
	}

	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
	Ref< ActData > act = componentData->getActs().at(selected);

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

void TheaterComponentEditor::splitAct()
{
	const int32_t selected = m_dropDownActs->getSelected();
	if (selected < 0)
		return;

	const int32_t cursorTick = m_trackSequencer->getCursor();
	const float cursorTime = float(cursorTick / 1000.0f);
	if (cursorTime < FUZZY_EPSILON)
		return;

	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
	Ref< ActData > actLeft = componentData->getActs().at(selected);

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
	RefArray< ActData >& acts = componentData->getActs();
	acts.insert(acts.begin() + selected + 1, actRight);

	// Update UI and scene editor.
	m_context->setTime(cursorTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);

	updateView();

	m_context->buildController();
}

void TheaterComponentEditor::timeScaleAct()
{
	const int32_t selected = m_dropDownActs->getSelected();
	if (selected < 0)
		return;

	TheaterComponentData* componentData = m_context->getSceneAsset()->getWorldComponent< TheaterComponentData >();
	Ref< ActData > act = componentData->getActs().at(selected);

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
		m_context->buildController();
	}

	enterTimeDialog.destroy();
}

void TheaterComponentEditor::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void TheaterComponentEditor::eventSequencerCursorMove(ui::CursorMoveEvent* event)
{
	const int32_t cursorTick = m_trackSequencer->getCursor();
	const float cursorTime = cursorTick / 1000.0f;

	m_context->setTime(cursorTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);
	m_context->raiseRedraw(nullptr);
}

void TheaterComponentEditor::eventSequencerKeyMove(ui::KeyMoveEvent* event)
{
	const ui::Tick* tick = dynamic_type_cast< ui::Tick* >(event->getKey());
	if (tick)
	{
		TransformPathKeyWrapper* keyWrapper = static_cast< TransformPathKeyWrapper* >(tick->getData(L"KEY"));
		T_ASSERT(keyWrapper);
		keyWrapper->m_key.T = tick->getTime() / 1000.0f;
		m_context->buildController();
	}
}

void TheaterComponentEditor::eventContextPostFrame(scene::PostFrameEvent* event)
{
	const double cursorTime = m_context->getTime();
	const int32_t cursorTick = (int32_t)(cursorTime * 1000.0);
	if (m_trackSequencer->getCursor() != cursorTick)
	{
		m_trackSequencer->setCursor(cursorTick);
		m_trackSequencer->update();
	}
}

}
