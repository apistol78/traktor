/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterEditor.h"

#include "Core/Class/Any.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "I18N/Text.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/Events/PostFrameEvent.h"
#include "Scene/Editor/Events/SceneSelectionChangeEvent.h"
#include "Theater/ActData.h"
#include "Theater/Performance.h"
#include "Theater/PropertyPath.h"
#include "Theater/RuntimeProperties.h"
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
#include "Ui/Sequencer/KeySelectEvent.h"
#include "Ui/Sequencer/Marker.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequenceGroup.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/Tick.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarItem.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityComponent.h"

#include <algorithm>
#include <limits>

namespace traktor::theater
{
	namespace
	{

const float c_clampKeyDistance = 1.0f / 30.0f;

/*! Key of a path of a track; a key hold a value of everything the path animate.
 *
 * Wrappers must have runtime type information of their own as they all are
 * attached to the keys of the sequencer under the same name.
 */
class PropertyPathKeyWrapper : public Object
{
	T_RTTI_CLASS;

public:
	TrackData* m_trackData;
	PropertyPath* m_propertyPath;
	float m_T;

	PropertyPathKeyWrapper(TrackData* trackData, PropertyPath* propertyPath, float T)
	:	m_trackData(trackData)
	,	m_propertyPath(propertyPath)
	,	m_T(T)
	{
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.PropertyPathKeyWrapper", PropertyPathKeyWrapper, Object)

class EventKeyWrapper : public Object
{
	T_RTTI_CLASS;

public:
	TrackData::EventKey& m_key;

	EventKeyWrapper(TrackData::EventKey& key)
	:	m_key(key)
	{
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.EventKeyWrapper", EventKeyWrapper, Object)

/*! Get track animating an entity; a track is added to the act if the entity isn't animated yet. */
TrackData* getOrAddTrack(ActData* act, const Guid& entityId)
{
	RefArray< TrackData >& tracks = act->getTracks();

	auto it = std::find_if(tracks.begin(), tracks.end(), [&](const TrackData* trackData) {
		return trackData->getEntityId() == entityId;
	});
	if (it != tracks.end())
		return *it;

	Ref< TrackData > trackData = new TrackData();
	trackData->setEntityId(entityId);
	tracks.push_back(trackData);
	return trackData;
}

/*! Find the object owning an animated property; null if it cannot be resolved. */
ITypedObject* findPropertyObject(const scene::EntityAdapter* entityAdapter, const PropertyPath::Property& property, const TypeInfo*& outObjectType)
{
	world::Entity* entity = (entityAdapter != nullptr) ? entityAdapter->getEntity() : nullptr;
	if (entity == nullptr)
		return nullptr;

	if (property.isEntityProperty())
	{
		outObjectType = &type_of< world::Entity >();
		return entity;
	}

	const TypeInfo* componentType = TypeInfo::find(property.componentType.c_str());
	if (componentType == nullptr)
		return nullptr;

	outObjectType = componentType;
	return entity->getComponent(*componentType);
}

/*! Get the key of a path at a given time, ready to be captured into.
 *
 * \param outIndex Index of the key if the path already has one there, -1 otherwise.
 */
PropertyPath::Key beginCapture(const PropertyPath* path, float time, int32_t& outIndex)
{
	outIndex = path->getClosestKey(time);
	if (outIndex >= 0 && abs(path->get(outIndex).T - time) < c_clampKeyDistance)
		return path->get(outIndex);

	outIndex = -1;

	PropertyPath::Key key;
	key.T = time;
	return key;
}

/*! Put a captured key back into its path. */
void endCapture(PropertyPath* path, int32_t index, const PropertyPath::Key& key)
{
	if (index >= 0)
		path->set(index, key);
	else
		path->insert(key);
}

/*! Capture interpolateable properties of an object into a key of a path.
 *
 * \param componentType Type name of the component; empty if the object is the entity itself.
 * \param only Capture only the property of this name; every one of them if empty.
 * \param space Space in which transforms are stored.
 */
void captureObjectProperties(
	ITypedObject* object,
	const std::wstring& componentType,
	const std::wstring& only,
	const Transform& space,
	PropertyPath* path,
	PropertyPath::Key& key
)
{
	AlignedVector< RuntimeProperties::Property > properties;
	RuntimeProperties::getInstance().getProperties(type_of(object), properties);

	for (const auto& property : properties)
	{
		if (!only.empty() && property.name != only)
			continue;

		// Only values which can be interpolated are animated; runtime classes do not expose their type.
		Any value = property.getter->invoke(object, 0, nullptr);
		if (PropertyPath::typeOfValue(value) == PropertyPath::ValueType::Transform)
			value = CastAny< Transform >::set(space * CastAny< Transform >::get(value));
		else if (PropertyPath::typeOfValue(value) == PropertyPath::ValueType::Invalid)
			continue;

		const bool added = (path->findProperty(componentType, property.name) < 0);
		const int32_t index = path->addProperty(componentType, property.name);

		key.values.resize(path->getProperties().size());
		PropertyPath::pack(value, key.values[index]);

		// A property captured for the first time is constant up until here.
		if (added)
		{
			for (auto& k : path->editKeys())
				k.values[index] = key.values[index];
		}
	}
}

/*! Capture the transform of an entity into the transform path of a track. */
void captureEntityProperties(const scene::EntityAdapter* entityAdapter, TrackData* trackData, const Transform& baseInv, float time)
{
	// Values are captured from the instantiated entity.
	world::Entity* entity = entityAdapter->getEntity();
	if (entity == nullptr)
		return;

	Ref< PropertyPath > path = trackData->getTransform();
	if (!path)
	{
		path = new PropertyPath();
		trackData->setTransform(path);
	}

	int32_t index;
	PropertyPath::Key key = beginCapture(path, time, index);

	// The transform of the entity is stored relative the performance.
	captureObjectProperties(entity, L"", PropertyPath::c_transformProperty, baseInv, path, key);

	endCapture(path, index, key);
}

/*! Capture interpolateable properties of the components of an entity. */
void captureComponentProperties(scene::EntityAdapter* entityAdapter, TrackData* trackData, float time)
{
	// Values are captured from the instantiated components.
	world::EntityData* entityData = entityAdapter->getEntityData();
	if (entityAdapter->getEntity() == nullptr || entityData == nullptr)
		return;

	Ref< PropertyPath > path = trackData->getProperties();
	if (!path)
		path = new PropertyPath();

	int32_t index;
	PropertyPath::Key key = beginCapture(path, time, index);
	key.componentData.resize(0);

	// Components are walked through their data as the data is remembered along with the key.
	for (auto componentData : entityData->getComponents())
	{
		world::IEntityComponent* component = entityAdapter->findComponentProduct(componentData);
		if (component == nullptr)
			continue;

		const std::wstring componentType = type_of(component).getName();

		// Transforms of components are already in the space of the entity.
		captureObjectProperties(component, componentType, L"", Transform::identity(), path, key);

		// Only the components which are animated are remembered, as a copy.
		const bool animated = std::any_of(path->getProperties().begin(), path->getProperties().end(), [&](const PropertyPath::Property& p) {
			return p.componentType == componentType;
		});
		if (animated)
			key.componentData.push_back(DeepClone(componentData).create< world::IEntityComponentData >());
	}

	// Nothing of the entity's components can be animated.
	if (path->getProperties().empty())
		return;

	endCapture(path, index, key);
	trackData->setProperties(path);
}

/*! Get index of the key of a path at a given time; -1 if the path has none there. */
int32_t findKeyAt(const PropertyPath* propertyPath, float T)
{
	const int32_t index = propertyPath->getClosestKey(T);
	if (index >= 0 && abs(propertyPath->get(index).T - T) < c_clampKeyDistance)
		return index;
	else
		return -1;
}

/*! Keep the keys of a path ordered in time, as the spline through them assume they are. */
void sortKeys(PropertyPath* propertyPath)
{
	AlignedVector< PropertyPath::Key >& keys = propertyPath->editKeys();
	std::sort(keys.begin(), keys.end(), [](const PropertyPath::Key& lh, const PropertyPath::Key& rh) {
		return lh.T < rh.T;
	});
}

/*! Get the time of a key of the sequencer; -1 if the key has no time of its own. */
int32_t getKeyTime(const ui::Key* key)
{
	if (const ui::Tick* tick = dynamic_type_cast< const ui::Tick* >(key))
		return tick->getTime();
	else if (const ui::Marker* marker = dynamic_type_cast< const ui::Marker* >(key))
		return marker->getTime();
	else
		return -1;
}

/*! Fit the range shown on the row of a track to the keys of everything it animate. */
void updateTrackRange(ui::SequenceGroup* trackGroup)
{
	int32_t rangeStart = std::numeric_limits< int32_t >::max();
	int32_t rangeEnd = std::numeric_limits< int32_t >::min();

	for (auto childItem : trackGroup->getChildItems())
	{
		ui::Sequence* sequence = dynamic_type_cast< ui::Sequence* >(childItem);
		if (!sequence)
			continue;

		for (auto key : sequence->getKeys())
		{
			const int32_t time = getKeyTime(key);
			if (time >= 0)
			{
				rangeStart = std::min(rangeStart, time);
				rangeEnd = std::max(rangeEnd, time);
			}
		}
	}

	if (rangeStart <= rangeEnd)
		trackGroup->setRange(rangeStart, rangeEnd);
	else
		trackGroup->setRange(0, 0);
}

/*! Get tracks of all selected rows of the sequencer. */
RefArray< TrackData > getSelectedTracks(ui::SequencerControl* sequencer)
{
	RefArray< TrackData > tracks;
	for (auto sequenceItem : sequencer->getSequenceItems(ui::SequencerControl::GfSelectedOnly | ui::SequencerControl::GfDescendants))
	{
		TrackData* trackData = sequenceItem->getData< TrackData >(L"TRACK");
		if (trackData != nullptr && std::find(tracks.begin(), tracks.end(), trackData) == tracks.end())
			tracks.push_back(trackData);
	}
	return tracks;
}

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

	m_toolToggleMode = new ui::ToolBarButton(
		i18n::Text(L"THEATER_EDITOR_MODE"),
		ui::Command(L"Theater.ToggleMode"),
		ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle
	);
	m_dropDownTargets = new ui::ToolBarDropDown(ui::Command(L"Theater.SelectTarget"), 200_ut, i18n::Text(L"THEATER_EDITOR_TARGETS"));
	m_dropDownActs = new ui::ToolBarDropDown(ui::Command(L"Theater.SelectAct"), 150_ut, i18n::Text(L"THEATER_EDITOR_ACTS"));

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(m_container);
	for (int32_t i = 0; i < 8; ++i)
		m_toolBar->addImage(new ui::StyleBitmap(L"Theater.Theater", i));
	// Acts are only edited while the theater mode is entered.
	const auto inMode = [&](ui::ToolBarItem* item) -> ui::ToolBarItem* {
		m_toolItemsInMode.push_back(item);
		return item;
	};

	m_toolBar->addItem(m_toolToggleMode);
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(m_dropDownTargets);
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_ADD_ACT"), 6, ui::Command(L"Theater.AddAct"))));
	m_toolBar->addItem(inMode(m_dropDownActs));
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_RENAME_ACT"), 6, ui::Command(L"Theater.RenameAct"))));
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_REMOVE_ACT"), 7, ui::Command(L"Theater.RemoveAct"))));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_CAPTURE_ENTITIES"), 0, ui::Command(L"Theater.CaptureEntities"))));
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_DELETE_SELECTED_KEY"), 1, ui::Command(L"Theater.DeleteSelectedKey"))));
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_SET_LOOKAT_ENTITY"), 4, ui::Command(L"Theater.SetLookAtEntity"))));
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_EASE_VELOCITY"), 5, ui::Command(L"Theater.EaseVelocity"))));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_GOTO_PREVIOUS_KEY"), 2, ui::Command(L"Theater.GotoPreviousKey"))));
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_GOTO_NEXT_KEY"), 3, ui::Command(L"Theater.GotoNextKey"))));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_SPLIT_ACT"), 2, ui::Command(L"Theater.SplitAct"))));
	m_toolBar->addItem(inMode(new ui::ToolBarButton(i18n::Text(L"THEATER_EDITOR_TIME_SCALE_ACT"), 2, ui::Command(L"Theater.TimeScaleAct"))));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &TheaterEditor::eventToolBarClick);

	for (auto item : m_toolItemsInMode)
		item->setEnable(false);

	m_trackSequencer = new ui::SequencerControl();
	if (!m_trackSequencer->create(m_container, ui::WsAccelerated | ui::SequencerControl::WsSingleSelection))
		return false;

	m_trackSequencer->addEventHandler< ui::CursorMoveEvent >(this, &TheaterEditor::eventSequencerCursorMove);
	m_trackSequencer->addEventHandler< ui::KeyMoveEvent >(this, &TheaterEditor::eventSequencerKeyMove);
	m_trackSequencer->addEventHandler< ui::KeySelectEvent >(this, &TheaterEditor::eventSequencerKeySelect);
	m_trackSequencer->setEnable(false);

	m_context = context;
	m_eventHandlerPostFrame = m_context->addEventHandler< scene::PostFrameEvent >(this, &TheaterEditor::eventContextPostFrame);
	m_eventHandlerSelect = m_context->addEventHandler< scene::SceneSelectionChangeEvent >(this, &TheaterEditor::eventContextSelect);
	m_buildCount = m_context->getBuildCount();

	updateTargets();
	updateView();
	return true;
}

void TheaterEditor::destroy()
{
	// Never leave the scene as the theater mode left it.
	restoreSceneState();

	if (m_context)
	{
		m_context->removeEventHandler(m_eventHandlerPostFrame);
		m_eventHandlerPostFrame = nullptr;

		m_context->removeEventHandler(m_eventHandlerSelect);
		m_eventHandlerSelect = nullptr;
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
			// No act selected; stop performance and rebuild entities as authored.
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
	else if (command == L"Theater.ToggleMode")
		setMode(m_toolToggleMode->isToggled());
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
	// Entity adapters are recreated for each build; collect all targets again.
	if (m_buildCount != m_context->getBuildCount())
	{
		m_buildCount = m_context->getBuildCount();

		// Components are cached anew as the entities are built.
		m_invalidatedTime = -1.0;

		updateTargets();
		updateView();
	}

	TheaterEditTarget* target = getTarget();
	if (!target)
		return;

	Performance* performance = target->getPerformance();
	if (performance == nullptr)
		return;

	const ActData* act = m_mode ? getAct() : nullptr;
	if (act != nullptr)
	{
		// Ensure component is up to date, we need to force it to keep in sync with editor.
		performance->preview(act->getName());

		// The performance write to the components only as time change.
		if (m_invalidatedTime != m_context->getTime())
		{
			m_invalidatedTime = m_context->getTime();
			invalidateAnimatedComponents();
		}
	}
	else
	{
		// No act selected, or the mode is left; leave entities alone.
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

	RefArray< ui::SequenceItem > items = m_trackSequencer->getSequenceItems(ui::SequencerControl::GfSelectedOnly | ui::SequencerControl::GfDescendants);

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

		// Only the motion of the entity is visualized.
		const PropertyPath* path = track->getTransform();
		if (path == nullptr || path->empty())
			continue;

		const int32_t transform = path->findProperty(L"", PropertyPath::c_transformProperty);
		if (transform < 0)
			continue;

		const int32_t steps = int32_t(duration) * 10;

		PropertyPath::Value F0 = path->evaluate(transform, PropertyPath::ValueType::Transform, 0.0f);
		for (int32_t i = 1; i <= steps; ++i)
		{
			const float T = (float(i) / steps) * duration;
			const PropertyPath::Value F1 = path->evaluate(transform, PropertyPath::ValueType::Transform, T);

			primitiveRenderer->drawLine(
				base * F0.value,
				base * F1.value,
				pathColor
			);

			F0 = F1;
		}

		for (int32_t i = 0; i <= steps; ++i)
		{
			const float T = (float(i) / steps) * duration;
			const PropertyPath::Value F = path->evaluate(transform, PropertyPath::ValueType::Transform, T);

			primitiveRenderer->drawSolidPoint(
				base * F.value,
				4.0f,
				Color4ub(255, 255, 255, 200)
			);
		}

		for (const auto& key : path->keys())
		{
			primitiveRenderer->drawSolidPoint(
				base * key.values[transform].value,
				8.0f,
				pathColor
			);
		}

		const PropertyPath::Value F = path->evaluate(transform, PropertyPath::ValueType::Transform, cursorTime);
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

void TheaterEditor::rebuildTarget(bool refreshProperties)
{
	if (TheaterEditTarget* target = getTarget())
		target->rebuild();

	// Acts have been modified outside of the property view; it needs refreshing.
	if (refreshProperties)
		m_refreshProperties = true;

	m_context->enqueueRedraw(nullptr);
}

void TheaterEditor::setMode(bool mode)
{
	if (mode == m_mode)
		return;

	m_mode = mode;

	if (m_toolToggleMode->isToggled() != mode)
		m_toolToggleMode->setToggled(mode);

	for (auto item : m_toolItemsInMode)
		item->setEnable(mode);
	m_toolBar->update();

	m_trackSequencer->setEnable(mode);
	m_trackSequencer->update();

	if (mode)
	{
		rememberSceneState();

		// Enter at a defined point of the act; the time is shared with the scene editor.
		m_trackSequencer->setCursor(0);
		m_context->setTime(0.0);
		m_context->setPhysicsEnable(false);
		m_context->setPlaying(false);

		// A performance is evaluated only as time change; re-instantiate it to pose the scene.
		rebuildTarget(false);

		// The scene must be drawn again for the performance to take effect.
		m_context->enqueueRedraw(nullptr);
		return;
	}

	// Leaving the mode; put the scene back as it was before it was entered.
	m_context->raisePreModify();

	restoreSceneState();

	// Components which the performance has been driving must be created anew.
	invalidateAnimatedComponents();

	m_context->raisePostModify(true);
	updateView();

	// Build the entities last; the panel editors are recreated from within the build.
	Ref< scene::SceneEditorContext > context = m_context;
	context->buildEntities(false);
	context->enqueueRedraw(nullptr);
}

void TheaterEditor::rememberSceneState()
{
	m_originalState.resize(0);
	m_sceneState.resize(0);

	// Collect the entities animated by any act of any theater component.
	SmallSet< Guid > entityIds;
	for (auto target : m_targets)
	{
		for (auto act : target->getActs())
		{
			for (auto track : act->getTracks())
				entityIds.insert(track->getEntityId());
		}
	}

	for (auto entityAdapter : m_context->getEntities())
	{
		if (entityIds.find(entityAdapter->getId()) != entityIds.end())
			rememberEntityState(entityAdapter);
	}

	// Whatever is selected as the mode is entered may be posed right away.
	for (auto entityAdapter : m_context->getEntities(scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly))
		rememberOriginalState(entityAdapter);
}

void TheaterEditor::rememberOriginalState(const scene::EntityAdapter* entityAdapter)
{
	const world::EntityData* entityData = entityAdapter->getEntityData();
	if (entityData == nullptr)
		return;

	for (const auto& state : m_originalState)
	{
		if (state.entityData == entityData)
			return;
	}

	EntityState state;
	if (readEntityState(entityAdapter, state))
		m_originalState.push_back(state);
}

void TheaterEditor::rememberEntityState(const scene::EntityAdapter* entityAdapter)
{
	const world::EntityData* entityData = entityAdapter->getEntityData();
	if (entityData == nullptr)
		return;

	for (const auto& state : m_sceneState)
	{
		if (state.entityData == entityData)
			return;
	}

	// Restore the entity to the state it had as the mode was entered.
	for (const auto& state : m_originalState)
	{
		if (state.entityData == entityData)
		{
			m_sceneState.push_back(state);
			return;
		}
	}

	// The entity was never selected; the state it has now is the earliest which can be restored.
	EntityState state;
	if (readEntityState(entityAdapter, state))
		m_sceneState.push_back(state);
}

bool TheaterEditor::readEntityState(const scene::EntityAdapter* entityAdapter, EntityState& outState) const
{
	world::EntityData* entityData = entityAdapter->getEntityData();
	if (entityData == nullptr)
		return false;

	outState.entityData = entityData;
	outState.transform = entityData->getTransform();
	outState.components.resize(0);

	for (auto componentData : entityData->getComponents())
	{
		// Acts are authored into the theater components; they are kept as they are.
		if (is_a< TheaterEntityComponentData >(componentData))
			outState.components.push_back(componentData);
		else
			outState.components.push_back(DeepClone(componentData).create< world::IEntityComponentData >());
	}

	return true;
}

void TheaterEditor::restoreSceneState()
{
	for (const auto& state : m_sceneState)
	{
		RefArray< world::IEntityComponentData > components = state.components;

		// A theater component added while the mode was entered is kept.
		for (auto componentData : state.entityData->getComponents())
		{
			if (
				is_a< TheaterEntityComponentData >(componentData) &&
				std::find(components.begin(), components.end(), componentData) == components.end()
			)
				components.push_back(componentData);
		}

		state.entityData->setTransform(state.transform);
		state.entityData->setComponents(components);
	}

	m_sceneState.resize(0);
	m_originalState.resize(0);
}

void TheaterEditor::invalidateAnimatedComponents()
{
	TheaterEditTarget* target = getTarget();
	const ActData* act = getAct();
	if (!target || !act)
		return;

	for (auto track : act->getTracks())
	{
		scene::EntityAdapter* entityAdapter = target->findEntityAdapter(track->getEntityId());
		if (entityAdapter == nullptr || entityAdapter->getEntity() == nullptr)
			continue;

		const PropertyPath* path = track->getProperties();
		if (path == nullptr)
			continue;

		for (const auto& property : path->getProperties())
		{
			// Only components are cached as products of their data.
			if (property.isEntityProperty())
				continue;

			const TypeInfo* componentType = TypeInfo::find(property.componentType.c_str());
			if (componentType == nullptr)
				continue;

			if (world::IEntityComponent* component = entityAdapter->getEntity()->getComponent(*componentType))
				entityAdapter->invalidateComponentProduct(component);
		}
	}
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

	// Keep selection across updates of the view; removeAll clear it.
	const Ref< ActData > selectedAct = getAct();
	const bool noActSelected = (m_dropDownActs->getSelected() == 0);

	// Rows, and their keys, are recreated thus a selected key is matched by what it stand for.
	const PropertyPath* selectedPath = nullptr;
	float selectedTime = 0.0f;
	if (const ui::Key* selectedKey = findSelectedKey())
	{
		if (const PropertyPathKeyWrapper* keyWrapper = selectedKey->getData< PropertyPathKeyWrapper >(L"KEY"))
		{
			selectedPath = keyWrapper->m_propertyPath;
			selectedTime = keyWrapper->m_T;
		}
	}

	m_dropDownActs->removeAll();
	m_trackSequencer->removeAllSequenceItems();

	if (!target)
	{
		m_trackSequencer->update();
		return;
	}

	// Let user select no act at all in order to edit entities freely.
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

			// A track is a foldable group of at most a transform, a properties and an events row.
			Ref< ui::SequenceGroup > trackGroup = new ui::SequenceGroup(name);
			trackGroup->setData(L"TRACK", track);

			if (PropertyPath* transform = track->getTransform())
			{
				Ref< ui::Sequence > transformSequence = new ui::Sequence(i18n::Text(L"THEATER_EDITOR_TRANSFORM"));
				transformSequence->setData(L"TRACK", track);

				for (const auto& key : transform->keys())
				{
					const int32_t tickTime = (int32_t)(key.T * 1000.0f);
					Ref< ui::Tick > tick = new ui::Tick(tickTime, true);
					tick->setData(L"KEY", new PropertyPathKeyWrapper(track, transform, key.T));
					transformSequence->addKey(tick);
				}

				trackGroup->addChildItem(transformSequence);
			}

			// Properties of the components share a single row as they are keyed together.
			{
				PropertyPath* properties = track->getProperties();
				if (properties != nullptr && !properties->empty())
				{
					Ref< ui::Sequence > propertySequence = new ui::Sequence(i18n::Text(L"THEATER_EDITOR_PROPERTIES"));
					propertySequence->setData(L"TRACK", track);

					for (const auto& key : properties->keys())
					{
						const int32_t tickTime = (int32_t)(key.T * 1000.0f);
						Ref< ui::Tick > tick = new ui::Tick(tickTime, true);
						tick->setData(L"KEY", new PropertyPathKeyWrapper(track, properties, key.T));
						propertySequence->addKey(tick);
					}

					trackGroup->addChildItem(propertySequence);
				}
			}

			// Events are issued by the entity of the track thus they are shown on a row of their own.
			if (!track->getEvents().empty())
			{
				Ref< ui::Sequence > eventSequence = new ui::Sequence(i18n::Text(L"THEATER_EDITOR_EVENTS"));
				eventSequence->setData(L"TRACK", track);

				for (auto& eventKey : track->getEvents())
				{
					const int32_t markerTime = (int32_t)(eventKey.T * 1000.0f);
					Ref< ui::Marker > marker = new ui::Marker(markerTime, true);
					marker->setData(L"EVENT", new EventKeyWrapper(eventKey));
					eventSequence->addKey(marker);
				}

				trackGroup->addChildItem(eventSequence);
			}

			updateTrackRange(trackGroup);

			m_trackSequencer->addSequenceItem(trackGroup);
		}

		m_trackSequencer->setLength((int32_t)(act->getDuration() * 1000.0f));
		m_trackSequencer->setCursor((int32_t)(m_context->getTime() * 1000.0f));
	}

	if (selectedPath != nullptr)
		selectKey(selectedPath, selectedTime);

	m_trackSequencer->update();
}

ui::Key* TheaterEditor::findSelectedKey() const
{
	for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfDescendants | ui::SequencerControl::GfSelectedOnly))
	{
		ui::Sequence* sequence = dynamic_type_cast< ui::Sequence* >(sequenceItem);
		if (sequence == nullptr)
			continue;

		if (ui::Key* key = sequence->getSelectedKey())
			return key;
	}
	return nullptr;
}

void TheaterEditor::selectKey(const PropertyPath* propertyPath, float time)
{
	for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfDescendants))
	{
		ui::Sequence* sequence = dynamic_type_cast< ui::Sequence* >(sequenceItem);
		if (sequence == nullptr)
			continue;

		for (auto key : sequence->getKeys())
		{
			const PropertyPathKeyWrapper* keyWrapper = key->getData< PropertyPathKeyWrapper >(L"KEY");
			if (keyWrapper == nullptr || keyWrapper->m_propertyPath != propertyPath)
				continue;

			if (abs(keyWrapper->m_T - time) < c_clampKeyDistance)
			{
				// The key belong to the selection of its row thus the row is selected first.
				sequence->setSelected(true);
				sequence->setSelectedKey(key);
				return;
			}
		}
	}
}

void TheaterEditor::captureEntities()
{
	if (!m_mode)
	{
		log::warning << L"Unable to capture entities; theater mode is not entered." << Endl;
		return;
	}

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

	const double time = m_context->getTime();

	// Capture the transform before the entities are built or it is reset to its data.
	const Transform baseInv = target->getBaseTransform().inverse();

	for (auto selectedEntity : selectedEntities)
	{
		if (!target->canCapture(selectedEntity))
		{
			log::warning << L"Unable to capture \"" << selectedEntity->getName() << L"\"; not animated by selected theater component." << Endl;
			continue;
		}

		// The entity becomes animated by the theater from now on.
		rememberEntityState(selectedEntity);

		captureEntityProperties(
			selectedEntity,
			getOrAddTrack(act, selectedEntity->getId()),
			baseInv,
			(float)time
		);
	}

	// Build the entities so their components reflect their data before reading properties.
	invalidateAnimatedComponents();
	m_context->buildEntities(false);

	// Entity adapters are reused across builds but the entities they hold are not.
	selectedEntities = m_context->getEntities(scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);

	for (auto selectedEntity : selectedEntities)
	{
		if (!target->canCapture(selectedEntity))
			continue;

		captureComponentProperties(
			selectedEntity,
			getOrAddTrack(act, selectedEntity->getId()),
			(float)time
		);
	}

	rebuildTarget();
}

void TheaterEditor::deleteSelectedKey()
{
	TheaterEditTarget* target = getTarget();
	if (!target)
		return;

	// At most one row is selected, and only a selected row hold a selected key.
	for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfDescendants | ui::SequencerControl::GfSelectedOnly))
	{
		ui::Sequence* selectedSequence = dynamic_type_cast< ui::Sequence* >(sequenceItem);
		if (!selectedSequence)
			continue;

		ui::Key* selectedKey = selectedSequence->getSelectedKey();
		if (!selectedKey)
			continue;

		Ref< TrackData > trackData = selectedSequence->getData< TrackData >(L"TRACK");
		T_ASSERT(trackData);

		if (PropertyPathKeyWrapper* keyWrapper = selectedKey->getData< PropertyPathKeyWrapper >(L"KEY"))
		{
			PropertyPath* propertyPath = keyWrapper->m_propertyPath;

			// Everything the path animate is keyed together.
			const int32_t index = findKeyAt(propertyPath, keyWrapper->m_T);
			if (index >= 0)
			{
				AlignedVector< PropertyPath::Key >& keys = propertyPath->editKeys();
				keys.erase(keys.begin() + index);
				selectedSequence->removeKey(selectedKey);
			}

			// A path without keys no longer animate anything of the track.
			if (propertyPath->empty())
			{
				if (trackData->getTransform() == propertyPath)
					trackData->setTransform(nullptr);
				if (trackData->getProperties() == propertyPath)
					trackData->setProperties(nullptr);
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

	// Look-at entity must be animated by the same theater component.
	if (!selectedEntities.empty() && !target->canCapture(selectedEntities[0]))
	{
		log::warning << L"Unable to look at \"" << selectedEntities[0]->getName() << L"\"; not animated by selected theater component." << Endl;
		return;
	}

	for (auto trackData : getSelectedTracks(m_trackSequencer))
	{
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

	for (auto trackData : getSelectedTracks(m_trackSequencer))
	{
		PropertyPath* path = trackData->getTransform();
		if (path == nullptr || path->size() < 3)
			continue;

		const int32_t transform = path->findProperty(L"", PropertyPath::c_transformProperty);
		if (transform < 0)
			continue;

		const AlignedVector< PropertyPath::Key >& keys = path->keys();
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
				const PropertyPath::Value Fc = path->evaluate(transform, PropertyPath::ValueType::Transform, T);
				const PropertyPath::Value Fn = path->evaluate(transform, PropertyPath::ValueType::Transform, T + c_measureStep);
				totalDistance += (Fn.value - Fc.value).length();
			}

			distances[i] = totalDistance;
		}

		// Distribute keys according to distances in time.
		const float c_smoothFactor = 0.1f;
		AlignedVector< PropertyPath::Key >& editKeys = path->editKeys();
		for (uint32_t i = 1; i < editKeys.size(); ++i)
			editKeys[i].T = lerp(editKeys[i].T, Ts + (distances[i] / totalDistance) * (Te - Ts), c_smoothFactor);
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
		const PropertyPath* paths[] = { track->getTransform(), track->getProperties() };
		for (auto path : paths)
		{
			if (path == nullptr)
				continue;

			const int32_t pki = path->getClosestPreviousKey((float)time);
			if (pki >= 0 && path->get(pki).T > previousTime)
				previousTime = path->get(pki).T;
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
		const PropertyPath* paths[] = { track->getTransform(), track->getProperties() };
		for (auto path : paths)
		{
			if (path == nullptr)
				continue;

			const int32_t nki = path->getClosestNextKey((float)time);
			if (nki >= 0 && path->get(nki).T < nextTime)
				nextTime = path->get(nki).T;
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

		if (PropertyPath* transform = trackLeft->getTransform())
		{
			Ref< PropertyPath > left = new PropertyPath();
			Ref< PropertyPath > right = new PropertyPath();
			transform->split(cursorTime, *left, *right);

			trackLeft->setTransform(left);
			trackRight->setTransform(right);
		}

		if (PropertyPath* properties = trackLeft->getProperties())
		{
			Ref< PropertyPath > left = new PropertyPath();
			Ref< PropertyPath > right = new PropertyPath();
			properties->split(cursorTime, *left, *right);

			trackLeft->setProperties(left);
			trackRight->setProperties(right);
		}

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
			PropertyPath* paths[] = { track->getTransform(), track->getProperties() };
			for (auto path : paths)
			{
				if (path == nullptr)
					continue;

				for (auto& k : path->editKeys())
					k.T *= f;
			}

			for (auto& eventKey : track->getEvents())
				eventKey.T *= f;
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
		if (PropertyPathKeyWrapper* keyWrapper = tick->getData< PropertyPathKeyWrapper >(L"KEY"))
		{
			PropertyPath* propertyPath = keyWrapper->m_propertyPath;

			const int32_t index = findKeyAt(propertyPath, keyWrapper->m_T);
			if (index >= 0)
			{
				const float T = tick->getTime() / 1000.0f;

				PropertyPath::Key key = propertyPath->get(index);
				key.T = T;
				propertyPath->set(index, key);
				sortKeys(propertyPath);

				keyWrapper->m_T = T;
				moved = true;
			}
		}
	}
	else if (const ui::Marker* marker = dynamic_type_cast< ui::Marker* >(event->getKey()))
	{
		EventKeyWrapper* eventWrapper = marker->getData< EventKeyWrapper >(L"EVENT");
		T_ASSERT(eventWrapper);
		eventWrapper->m_key.T = marker->getTime() / 1000.0f;
		moved = true;
	}

	if (moved)
	{
		// The view is not rebuilt while dragging thus the range of the row is fitted in place.
		for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfDefault))
		{
			if (ui::SequenceGroup* trackGroup = dynamic_type_cast< ui::SequenceGroup* >(sequenceItem))
				updateTrackRange(trackGroup);
		}

		// Refreshing the property view undo the pose; leave it until the drag is over.
		m_keysMoved = true;
		rebuildTarget(false);
	}
}

void TheaterEditor::eventContextSelect(scene::SceneSelectionChangeEvent* event)
{
	if (!m_mode)
		return;

	// Selecting an entity is the last moment it is known to be unposed.
	for (auto entityAdapter : m_context->getEntities(scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly))
		rememberOriginalState(entityAdapter);
}

void TheaterEditor::applyCapturedComponentData(const ui::Key* key)
{
	if (key == nullptr)
		return;

	// Only the properties row stand for the components.
	PropertyPathKeyWrapper* keyWrapper = key->getData< PropertyPathKeyWrapper >(L"KEY");
	if (keyWrapper == nullptr || keyWrapper->m_propertyPath != keyWrapper->m_trackData->getProperties())
		return;

	TheaterEditTarget* target = getTarget();
	if (!target)
		return;

	scene::EntityAdapter* entityAdapter = target->findEntityAdapter(keyWrapper->m_trackData->getEntityId());
	world::EntityData* entityData = (entityAdapter != nullptr) ? entityAdapter->getEntityData() : nullptr;
	if (entityData == nullptr)
		return;

	const int32_t index = findKeyAt(keyWrapper->m_propertyPath, keyWrapper->m_T);
	if (index < 0)
		return;

	const RefArray< world::IEntityComponentData >& componentData = keyWrapper->m_propertyPath->get(index).componentData;
	if (componentData.empty())
		return;

	// Only data which differ is put back as building the entities is expensive.
	bool modified = false;
	for (auto cd : componentData)
	{
		const world::IEntityComponentData* current = entityData->getComponent(type_of(cd));
		if (current != nullptr && DeepHash(current) == DeepHash(cd))
			continue;

		// A copy is put back as the key must keep the state it was captured with.
		entityData->setComponent(DeepClone(cd).create< world::IEntityComponentData >());
		modified = true;
	}

	if (!modified)
		return;

	m_context->buildEntities(false);
	m_context->enqueueRedraw(nullptr);

	// Nothing of the scene has moved thus the scene operators need not be evaluated.
	m_context->raisePostModify(false);
}

bool TheaterEditor::isTrackingKey() const
{
	for (auto sequenceItem : m_trackSequencer->getSequenceItems(ui::SequencerControl::GfDescendants))
	{
		if (sequenceItem->isTrackingKey())
			return true;
	}
	return false;
}

void TheaterEditor::eventSequencerKeySelect(ui::KeySelectEvent* event)
{
	const ui::Tick* tick = dynamic_type_cast< const ui::Tick* >(event->getKey());
	if (tick == nullptr)
		return;

	// The performance pose the scene at the time of the cursor; move it onto the key.
	if (m_trackSequencer->getCursor() != tick->getTime())
	{
		m_trackSequencer->setCursor(tick->getTime());
		m_trackSequencer->update();

		m_context->setTime(tick->getTime() / 1000.0);
		m_context->setPhysicsEnable(false);
		m_context->setPlaying(false);
	}

	applyCapturedComponentData(tick);
}

void TheaterEditor::eventContextPostFrame(scene::PostFrameEvent* event)
{
	// Keys have been dragged; refresh the property view once the drag is over.
	if (m_keysMoved && !isTrackingKey())
	{
		m_keysMoved = false;
		m_refreshProperties = true;
	}

	if (m_refreshProperties)
	{
		m_refreshProperties = false;

		// Nothing of the scene has moved thus the scene operators need not be evaluated.
		m_context->raisePostModify(false);
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
