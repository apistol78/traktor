/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Scene.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/Events/PostFrameEvent.h"
#include "Theater/ActData.h"
#include "Theater/TheaterControllerData.h"
#include "Theater/TrackData.h"
#include "Theater/Editor/TheaterControllerEditor.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/MessageBox.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/EditList.h"
#include "Ui/Custom/EditListEditEvent.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/Sequencer/CursorMoveEvent.h"
#include "Ui/Custom/Sequencer/KeyMoveEvent.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/Tick.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace theater
	{
		namespace
		{

const float c_clampKeyDistance = 1.0f / 30.0f;
const float c_velocityScale = 0.2f;

struct FindTrackData
{
	world::EntityData* m_entityData;

	FindTrackData(world::EntityData* entityData)
	:	m_entityData(entityData)
	{
	}

	bool operator () (const TrackData* trackData) const
	{
		return trackData->getEntityData() == m_entityData;
	}
};

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
		float a = orientation[i];
		float r = reference[i];

		float d1 = abs(a - r);
		float d2 = abs(a + TWO_PI - r);
		float d3 = abs(a - TWO_PI - r);

		if (d2 < d1 && d2 < d3)
			a += TWO_PI;
		else if (d3 < d1 && d3 < d2)
			a -= TWO_PI;

		out.set(i, Scalar(a));
	}
	return out;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterControllerEditor", TheaterControllerEditor, scene::ISceneControllerEditor)

TheaterControllerEditor::TheaterControllerEditor()
:	m_timeOffset(0.0f)
{
}

bool TheaterControllerEditor::create(scene::SceneEditorContext* context, ui::Container* parent)
{
	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(parent, true, ui::scaleBySystemDPI(100));

	Ref< ui::Container > containerActs = new ui::Container();
	if (!containerActs->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBarActs = new ui::custom::ToolBar();
	m_toolBarActs->create(containerActs);
	m_toolBarActs->addImage(new ui::StyleBitmap(L"Theater.Theater"), 8);
	m_toolBarActs->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_ADD_ACT"), 6, ui::Command(L"Theater.AddAct")));
	m_toolBarActs->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_REMOVE_ACT"), 7, ui::Command(L"Theater.RemoveAct")));
	m_toolBarActs->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &TheaterControllerEditor::eventToolBarClick);

	m_listActs = new ui::custom::EditList();
	m_listActs->create(containerActs, ui::custom::ListBox::WsSingle);
	m_listActs->addEventHandler< ui::SelectionChangeEvent >(this, &TheaterControllerEditor::eventActSelected);
	m_listActs->addEventHandler< ui::custom::EditListEditEvent >(this, &TheaterControllerEditor::eventActEdit);

	Ref< ui::Container > containerSequencer = new ui::Container();
	if (!containerSequencer->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(containerSequencer);
	m_toolBar->addImage(new ui::StyleBitmap(L"Theater.Theater"), 8);
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_CAPTURE_ENTITIES"), 0, ui::Command(L"Theater.CaptureEntities")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_DELETE_SELECTED_KEY"), 1, ui::Command(L"Theater.DeleteSelectedKey")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_SET_LOOKAT_ENTITY"), 4, ui::Command(L"Theater.SetLookAtEntity")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_EASE_VELOCITY"), 5, ui::Command(L"Theater.EaseVelocity")));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_GOTO_PREVIOUS_KEY"), 2, ui::Command(L"Theater.GotoPreviousKey")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_GOTO_NEXT_KEY"), 3, ui::Command(L"Theater.GotoNextKey")));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_SPLIT_ACT"), 2, ui::Command(L"Theater.SplitAct")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_TIME_SCALE_ACT"), 2, ui::Command(L"Theater.TimeScaleAct")));
	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &TheaterControllerEditor::eventToolBarClick);

	m_trackSequencer = new ui::custom::SequencerControl();
	if (!m_trackSequencer->create(containerSequencer, ui::WsAccelerated))
		return false;

	m_trackSequencer->addEventHandler< ui::custom::CursorMoveEvent >(this, &TheaterControllerEditor::eventSequencerCursorMove);
	m_trackSequencer->addEventHandler< ui::custom::KeyMoveEvent >(this, &TheaterControllerEditor::eventSequencerKeyMove);

	m_context = context;
	m_context->addEventHandler< scene::PostFrameEvent >(this, &TheaterControllerEditor::eventContextPostFrame);

	updateView();
	return true;
}

void TheaterControllerEditor::destroy()
{
	if (m_trackSequencer)
	{
		m_trackSequencer->destroy();
		m_trackSequencer = 0;
	}
	if (m_toolBar)
	{
		m_toolBar->destroy();
		m_toolBar = 0;
	}
}

void TheaterControllerEditor::entityRemoved(scene::EntityAdapter* entityAdapter)
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	RefArray< ActData >& acts = controllerData->getActs();
	for (RefArray< ActData >::iterator i = acts.begin(); i != acts.end(); ++i)
	{
		RefArray< TrackData >& tracks = (*i)->getTracks();
		for (RefArray< TrackData >::iterator i = tracks.begin(); i != tracks.end(); )
		{
			if ((*i)->getEntityData() == entityAdapter->getEntityData())
				i = tracks.erase(i);
			else
				++i;
		}
	}

	updateView();
}

void TheaterControllerEditor::propertiesChanged()
{
	m_context->buildController();
	updateView();
}

bool TheaterControllerEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Theater.AddAct")
	{
		Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
		Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
		controllerData->getActs().push_back(new ActData());
		m_context->buildController();
		updateView();
	}
	else if (command == L"Theater.RemoveAct")
	{
		int32_t selected = m_listActs->getSelected();
		if (selected >= 0)
		{
			int result = ui::MessageBox::show(
				m_toolBarActs,
				i18n::Text(L"THEATER_EDITOR_MESSAGE_REMOVE_ACT"),
				i18n::Text(L"THEATER_EDITOR_TITLE_REMOVE_ACT"),
				ui::MbYesNo | ui::MbIconExclamation
			);
			if (result == ui::DrYes)
			{
				Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
				Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
				RefArray< ActData >& acts = controllerData->getActs();
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

void TheaterControllerEditor::update()
{
}

void TheaterControllerEditor::draw(render::PrimitiveRenderer* primitiveRenderer)
{
	int32_t selected = m_listActs->getSelected();
	if (selected < 0)
		return;

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
	Ref< ActData > act = controllerData->getActs().at(selected);

	RefArray< ui::custom::SequenceItem > items;
	m_trackSequencer->getSequenceItems(items, ui::custom::SequencerControl::GfSelectedOnly);

	int32_t cursorTick = m_trackSequencer->getCursor();
	float cursorTime = float(cursorTick / 1000.0f);
	float duration = act->getDuration();

	const RefArray< TrackData >& tracks = act->getTracks();
	for (RefArray< TrackData >::const_iterator i = tracks.begin(); i != tracks.end(); ++i)
	{
		Color4ub pathColor(180, 180, 80, 120);
		for (RefArray< ui::custom::SequenceItem >::const_iterator j = items.begin(); j != items.end(); ++j)
		{
			if ((*j)->getData(L"TRACK") == *i)
			{
				pathColor = Color4ub(255, 255, 0, 200);
				break;
			}
		}

		const TransformPath& path = (*i)->getPath();
		int32_t steps = int32_t(duration) * 10;
		
		TransformPath::Key F0 = path.evaluate(0.0f, duration);
		for (int32_t i = 1; i <= steps; ++i)
		{
			float T = (float(i) / steps) * duration;
			TransformPath::Key F1 = path.evaluate(T, duration);

			primitiveRenderer->drawLine(
				F0.position,
				F1.position,
				pathColor
			);

			F0 = F1;
		}

		for (int32_t i = 0; i <= steps; ++i)
		{
			float T = (float(i) / steps) * duration;
			TransformPath::Key F0 = path.evaluate(T, duration);

			primitiveRenderer->drawSolidPoint(
				F0.position,
				4.0f,
				Color4ub(255, 255, 255, 200)
			);
		}

		const AlignedVector< TransformPath::Key >& keys = path.getKeys();
		for (AlignedVector< TransformPath::Key >::const_iterator i = keys.begin(); i != keys.end(); ++i)
		{
			primitiveRenderer->drawSolidPoint(
				i->position,
				8.0f,
				pathColor
			);
		}

		TransformPath::Key F = path.evaluate(cursorTime, duration);
		primitiveRenderer->drawWireFrame(
			F.transform().toMatrix44(),
			1.0f
		);
	}
}

void TheaterControllerEditor::updateView()
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	RefArray< ActData >& acts = controllerData->getActs();

	int32_t selected = m_listActs->getSelected();
	if (selected >= int32_t(acts.size()))
		selected = int32_t(acts.size()) - 1;

	m_listActs->removeAll();
	for (RefArray< ActData >::iterator i = acts.begin(); i != acts.end(); ++i)
	{
		std::wstring actName = (*i)->getName();
		if (actName.empty())
			actName = i18n::Text(L"THEATER_EDITOR_UNNAMED_ACT");
		m_listActs->add(actName, *i);
	}

	m_listActs->select(selected);

	m_trackSequencer->removeAllSequenceItems();

	if (selected >= 0)
	{
		RefArray< TrackData >& tracks = acts[selected]->getTracks();
		for (RefArray< TrackData >::iterator i = tracks.begin(); i != tracks.end(); ++i)
		{
			Ref< ui::custom::Sequence > trackSequence = new ui::custom::Sequence((*i)->getEntityData()->getName());
			trackSequence->setData(L"TRACK", *i);

			TransformPath& path = (*i)->getPath();
			AlignedVector< TransformPath::Key >& keys = path.getKeys();

			for (AlignedVector< TransformPath::Key >::iterator j = keys.begin(); j != keys.end(); ++j)
			{
				int32_t tickTime = int32_t(j->T * 1000.0f);

				Ref< ui::custom::Tick > tick = new ui::custom::Tick(tickTime, true);
				tick->setData(L"KEY", new TransformPathKeyWrapper(*j));

				trackSequence->addKey(tick);
			}

			m_trackSequencer->addSequenceItem(trackSequence);
		}

		m_trackSequencer->setLength(int32_t(acts[selected]->getDuration() * 1000.0f));
		m_trackSequencer->setCursor(int32_t((m_context->getTime() - m_timeOffset) * 1000.0f));
	}

	m_trackSequencer->update();
}

void TheaterControllerEditor::captureEntities()
{
	int32_t selected = m_listActs->getSelected();
	if (selected < 0)
	{
		log::warning << L"Unable to capture entities; no act selected" << Endl;
		return;
	}

	RefArray< scene::EntityAdapter > selectedEntities;
	m_context->getEntities(selectedEntities, scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);
	if (selectedEntities.empty())
	{
		log::warning << L"Unable to capture entities; no entities selected" << Endl;
		return;
	}

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
	Ref< ActData > act = controllerData->getActs().at(selected);

	float time = m_context->getTime() - m_timeOffset;

	RefArray< TrackData >& tracks = act->getTracks();
	for (RefArray< scene::EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
	{
		Transform transform = (*i)->getTransform();

		Ref< world::EntityData > entityData = (*i)->getEntityData();
		T_ASSERT (entityData);

		Ref< TrackData > instanceTrackData;

		RefArray< TrackData >::iterator j = std::find_if(tracks.begin(), tracks.end(), FindTrackData(entityData));
		if (j != tracks.end())
			instanceTrackData = *j;
		else
		{
			instanceTrackData = new TrackData();
			instanceTrackData->setEntityData(entityData);
			tracks.push_back(instanceTrackData);
		}

		T_ASSERT (instanceTrackData);
		TransformPath& pathData = instanceTrackData->getPath();

		int32_t cki = pathData.getClosestKey(time);
		TransformPath::Key* closestKey = (cki >= 0) ? &pathData[cki] : 0;

		if (closestKey && abs(closestKey->T - time) < c_clampKeyDistance)
		{
			closestKey->position = transform.translation();
			closestKey->orientation = transform.rotation().toEulerAngles();

			// Ensure orientation are "logially" fixed up to previous key.
			if (cki > 0)
			{
				pathData[cki].orientation = fixupOrientation(
					pathData[cki].orientation,
					pathData[cki - 1].orientation
				);
			}
		}
		else
		{
			TransformPath::Key key;
			key.T = time;
			key.position = transform.translation();
			key.orientation = transform.rotation().toEulerAngles();
			size_t at = pathData.insert(key);

			// Ensure orientation are "logially" fixed up to previous key.
			if (at > 0)
			{
				pathData[at].orientation = fixupOrientation(
					pathData[at].orientation,
					pathData[at - 1].orientation
				);
			}
		}
	}

	m_context->buildController();
}

void TheaterControllerEditor::deleteSelectedKey()
{
	RefArray< ui::custom::SequenceItem > sequenceItems;
	m_trackSequencer->getSequenceItems(sequenceItems, ui::custom::SequencerControl::GfSelectedOnly | ui::custom::SequencerControl::GfDescendants);

	for (RefArray< ui::custom::SequenceItem >::iterator i = sequenceItems.begin(); i != sequenceItems.end(); ++i)
	{
		ui::custom::Sequence* selectedSequence = checked_type_cast< ui::custom::Sequence*, false >(*i);
		ui::custom::Tick* selectedTick = checked_type_cast< ui::custom::Tick*, true >(selectedSequence->getSelectedKey());
		if (!selectedTick)
			continue;

		Ref< TrackData > trackData = selectedSequence->getData< TrackData >(L"TRACK");
		T_ASSERT (trackData);

		TransformPathKeyWrapper* keyWrapper = selectedTick->getData< TransformPathKeyWrapper >(L"KEY");
		T_ASSERT (keyWrapper);

		TransformPath& path = trackData->getPath();
		AlignedVector< TransformPath::Key >& keys = path.getKeys();
		for (AlignedVector< TransformPath::Key >::iterator j = keys.begin(); j != keys.end(); ++j)
		{
			if (&(*j) == &keyWrapper->m_key)
			{
				selectedSequence->removeKey(selectedTick);
				keys.erase(j);
				break;
			}
		}
	}

	m_context->buildController();
}

void TheaterControllerEditor::setLookAtEntity()
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	RefArray< ui::custom::SequenceItem > sequenceItems;
	m_trackSequencer->getSequenceItems(sequenceItems, ui::custom::SequencerControl::GfSelectedOnly | ui::custom::SequencerControl::GfDescendants);

	RefArray< scene::EntityAdapter > selectedEntities;
	m_context->getEntities(selectedEntities, scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);
	if (selectedEntities.size() > 1)
		return;

	for (RefArray< ui::custom::SequenceItem >::iterator i = sequenceItems.begin(); i != sequenceItems.end(); ++i)
	{
		ui::custom::Sequence* selectedSequence = checked_type_cast< ui::custom::Sequence*, false >(*i);
		Ref< TrackData > trackData = selectedSequence->getData< TrackData >(L"TRACK");
		T_ASSERT (trackData);

		if (!selectedEntities.empty())
			trackData->setLookAtEntityData(selectedEntities[0]->getEntityData());
		else
			trackData->setLookAtEntityData(0);
	}

	m_context->buildController();
}

void TheaterControllerEditor::easeVelocity()
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	RefArray< ui::custom::SequenceItem > sequenceItems;
	m_trackSequencer->getSequenceItems(sequenceItems, ui::custom::SequencerControl::GfSelectedOnly | ui::custom::SequencerControl::GfDescendants);

	for (RefArray< ui::custom::SequenceItem >::iterator i = sequenceItems.begin(); i != sequenceItems.end(); ++i)
	{
		ui::custom::Sequence* selectedSequence = checked_type_cast< ui::custom::Sequence*, false >(*i);
		Ref< TrackData > trackData = selectedSequence->getData< TrackData >(L"TRACK");
		T_ASSERT (trackData);

		TransformPath& path = trackData->getPath();
		AlignedVector< TransformPath::Key >& keys = path.getKeys();
		if (keys.size() < 3)
			continue;

		float Ts = keys.front().T;
		float Te = keys.back().T;

		// Measure euclidean distance of keys.
		std::vector< float > distances(keys.size(), 0.0f);
		float totalDistance = 0.0f;

		for (uint32_t i = 1; i < keys.size(); ++i)
		{
			float T0 = keys[i - 1].T;
			float T1 = keys[i].T;

			const float c_measureStep = 1.0f / 1000.0f;
			for (float T = T0; T <= T1 - c_measureStep; T += c_measureStep)
			{
				TransformPath::Key Fc = path.evaluate(T);
				TransformPath::Key Fn = path.evaluate(T + c_measureStep);
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

void TheaterControllerEditor::gotoPreviousKey()
{
	int32_t selected = m_listActs->getSelected();
	if (selected < 0)
	{
		log::warning << L"Unable to goto key entities; no act selected" << Endl;
		return;
	}

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
	Ref< ActData > act = controllerData->getActs().at(selected);

	float time = m_context->getTime() - m_timeOffset;
	float previousTime = 0.0f;

	const RefArray< TrackData >& tracks = act->getTracks();
	for (RefArray< TrackData >::const_iterator i = tracks.begin(); i != tracks.end(); ++i)
	{
		TransformPath& path = (*i)->getPath();
		int32_t pki = path.getClosestPreviousKey(time);
		if (pki >= 0)
		{
			if (path[pki].T > previousTime)
				previousTime = path[pki].T;
		}
	}

	int32_t cursorTick = int32_t(previousTime * 1000.0f);

	m_trackSequencer->setCursor(cursorTick);
	m_trackSequencer->update();

	m_context->setTime(m_timeOffset + previousTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);
}

void TheaterControllerEditor::gotoNextKey()
{
	int32_t selected = m_listActs->getSelected();
	if (selected < 0)
	{
		log::warning << L"Unable to goto key entities; no act selected" << Endl;
		return;
	}

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
	Ref< ActData > act = controllerData->getActs().at(selected);

	float time = m_context->getTime() - m_timeOffset;
	float nextTime = act->getDuration();

	const RefArray< TrackData >& tracks = act->getTracks();
	for (RefArray< TrackData >::const_iterator i = tracks.begin(); i != tracks.end(); ++i)
	{
		TransformPath& path = (*i)->getPath();
		int32_t nki = path.getClosestNextKey(time);
		if (nki >= 0)
		{
			if (path[nki].T < nextTime)
				nextTime = path[nki].T;
		}
	}

	int32_t cursorTick = int32_t(nextTime * 1000.0f);

	m_trackSequencer->setCursor(cursorTick);
	m_trackSequencer->update();

	m_context->setTime(m_timeOffset + nextTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);
}

void TheaterControllerEditor::splitAct()
{
	int32_t selected = m_listActs->getSelected();
	if (selected < 0)
		return;

	int32_t cursorTick = m_trackSequencer->getCursor();
	float cursorTime = float(cursorTick / 1000.0f);
	if (cursorTime < FUZZY_EPSILON)
		return;

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	Ref< ActData > actLeft = controllerData->getActs().at(selected);

	float actTotalDuration = actLeft->getDuration();
	std::wstring actName = actLeft->getName();

	actLeft->setName(actName + L" (Left)");
	actLeft->setDuration(cursorTime);

	Ref< ActData > actRight = new ActData();
	actRight->setName(actName + L" (Right)");
	actRight->setDuration(actTotalDuration - cursorTime);

	// Remove looping and split paths.
	for (size_t i = 0; i < actLeft->getTracks().size(); ++i)
	{
		TrackData* trackLeft = actLeft->getTracks().at(i);
		trackLeft->setLoopStart(0.0f);
		trackLeft->setLoopEnd(0.0f);

		Ref< TrackData > trackRight = new TrackData();
		trackRight->setEntityData(trackLeft->getEntityData());
		trackRight->setLookAtEntityData(trackLeft->getLookAtEntityData());
		trackRight->setWobbleMagnitude(trackLeft->getWobbleMagnitude());
		trackRight->setWobbleRate(trackLeft->getWobbleRate());

		TransformPath pathLeft, pathRight;
		trackLeft->getPath().split(cursorTime, pathLeft, pathRight);
		trackLeft->setPath(pathLeft);
		trackRight->setPath(pathRight);

		actRight->getTracks().push_back(trackRight);
	}

	// Add new act after splitted act.
	RefArray< ActData >& acts = controllerData->getActs();
	acts.insert(acts.begin() + selected + 1, actRight);

	// Update UI and scene editor.
	m_context->setTime(m_timeOffset + cursorTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);

	updateView();

	m_context->buildController();
}

void TheaterControllerEditor::timeScaleAct()
{
	int32_t selected = m_listActs->getSelected();
	if (selected < 0)
		return;

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
	Ref< ActData > act = controllerData->getActs().at(selected);

	float fromDuration = act->getDuration();

	ui::custom::InputDialog::Field fields[] = 
	{
		ui::custom::InputDialog::Field(i18n::Text(L"THEATER_EDITOR_TIME_SCALE_NEW_TIME"), toString(fromDuration), new ui::NumericEditValidator(true, 0.0f))
	};

	ui::custom::InputDialog enterTimeDialog;
	enterTimeDialog.create(m_listActs, i18n::Text(L"THEATER_EDITOR_TIME_SCALE_TITLE"), i18n::Text(L"THEATER_EDITOR_TIME_SCALE_MESSAGE"), fields, sizeof_array(fields));

	if (enterTimeDialog.showModal() == ui::DrOk)
	{
		float toDuration = parseString< float >(fields[0].value);
		float f = toDuration / fromDuration;

		RefArray< TrackData >& t = act->getTracks();
		for (RefArray< TrackData >::iterator i = t.begin(); i != t.end(); ++i)
		{
			(*i)->setLoopStart((*i)->getLoopStart() * f);
			(*i)->setLoopEnd((*i)->getLoopEnd() * f);
			(*i)->setTimeOffset((*i)->getTimeOffset() * f);

			TransformPath& tp = (*i)->getPath();
			
			AlignedVector< TransformPath::Key >& k = tp.getKeys();
			for (AlignedVector< TransformPath::Key >::iterator j = k.begin(); j != k.end(); ++j)
				j->T *= f;
		}

		act->setDuration(toDuration);

		updateView();

		m_context->buildController();
	}

	enterTimeDialog.destroy();
}

void TheaterControllerEditor::eventActSelected(ui::SelectionChangeEvent* event)
{
	int32_t selected = m_listActs->getSelected();
	if (selected >= 0)
	{
		Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
		Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
		m_timeOffset = controllerData->getActStartTime(selected);
	}
	else
		m_timeOffset = 0.0f;

	m_trackSequencer->setCursor(0);

	m_context->setTime(m_timeOffset);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);

	updateView();
}

void TheaterControllerEditor::eventActEdit(ui::custom::EditListEditEvent* event)
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = mandatory_non_null_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());
	Ref< ActData > act = controllerData->getActs().at(event->getIndex());
	act->setName(event->getText());
	updateView();
}

void TheaterControllerEditor::eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void TheaterControllerEditor::eventSequencerCursorMove(ui::custom::CursorMoveEvent* event)
{
	int32_t cursorTick = m_trackSequencer->getCursor();
	float cursorTime = float(cursorTick / 1000.0f);

	m_context->setTime(m_timeOffset + cursorTime);
	m_context->setPhysicsEnable(false);
	m_context->setPlaying(false);
	m_context->raiseRedraw();
}

void TheaterControllerEditor::eventSequencerKeyMove(ui::custom::KeyMoveEvent* event)
{
	ui::custom::Tick* tick = dynamic_type_cast< ui::custom::Tick* >(event->getKey());
	if (tick)
	{
		TransformPathKeyWrapper* keyWrapper = static_cast< TransformPathKeyWrapper* >(tick->getData(L"KEY").ptr());
		T_ASSERT (keyWrapper);

		keyWrapper->m_key.T = tick->getTime() / 1000.0f;

		m_context->buildController();
	}
}

void TheaterControllerEditor::eventContextPostFrame(scene::PostFrameEvent* event)
{
	if (m_context->isPlaying())
	{
		float cursorTime = m_context->getTime() - m_timeOffset;

		int32_t cursorTickMax = m_trackSequencer->getLength();
		int32_t cursorTick = int32_t(cursorTime * 1000.0f) % cursorTickMax;

		m_trackSequencer->setCursor(cursorTick);
		m_trackSequencer->update();
	}
}

	}
}
