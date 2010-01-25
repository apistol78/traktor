#include "I18N/Text.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Theater/TheaterController.h"
#include "Theater/TheaterControllerData.h"
#include "Theater/TrackData.h"
#include "Theater/Editor/TheaterControllerEditor.h"
#include "Ui/Bitmap.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/Tick.h"
#include "World/Entity/EntityInstance.h"

// Resources
#include "Resources/Theater.h"

namespace traktor
{
	namespace theater
	{
		namespace
		{

const float c_clampKeyDistance = 1.0f / 30.0f;

struct FindInstanceTrackData
{
	world::EntityInstance* m_instance;

	FindInstanceTrackData(world::EntityInstance* instance)
	:	m_instance(instance)
	{
	}

	bool operator () (const TrackData* trackData) const
	{
		return trackData->getInstance() == m_instance;
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterControllerEditor", TheaterControllerEditor, scene::ISceneControllerEditor)

bool TheaterControllerEditor::create(scene::SceneEditorContext* context, ui::Container* parent)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceTheater, sizeof(c_ResourceTheater), L"png"), 4);
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_CAPTURE_ENTITIES"), ui::Command(L"Theater.CaptureEntities"), 0));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_DELETE_SELECTED_KEY"), ui::Command(L"Theater.DeleteSelectedKey"), 1));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_GOTO_PREVIOUS_KEY"), ui::Command(L"Theater.GotoPreviousKey"), 2));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"THEATER_EDITOR_GOTO_NEXT_KEY"), ui::Command(L"Theater.GotoNextKey"), 3));
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &TheaterControllerEditor::eventToolBarClick));

	m_trackSequencer = new ui::custom::SequencerControl();
	if (!m_trackSequencer->create(container))
		return false;

	m_trackSequencer->addCursorMoveEventHandler(ui::createMethodHandler(this, &TheaterControllerEditor::eventSequencerCursorMove));
	m_trackSequencer->addKeyMoveEventHandler(ui::createMethodHandler(this, &TheaterControllerEditor::eventSequencerKeyMove));

	m_context = context;
	m_context->addPostFrameEventHandler(ui::createMethodHandler(this, &TheaterControllerEditor::eventContextPostFrame));

	updateSequencer();
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
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	RefArray< TrackData >& trackData = controllerData->getTrackData();
	for (RefArray< TrackData >::iterator i = trackData.begin(); i != trackData.end(); )
	{
		if ((*i)->getInstance() == entityAdapter->getInstance())
			i = trackData.erase(i);
		else
			++i;
	}

	updateSequencer();
}

void TheaterControllerEditor::propertiesChanged()
{
	updateSequencer();
}

bool TheaterControllerEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Theater.CaptureEntities")
	{
		captureEntities();
		updateSequencer();
	}
	else if (command == L"Theater.DeleteSelectedKey")
	{
		deleteSelectedKey();
		updateSequencer();
	}
	else if (command == L"Theater.GotoPreviousKey")
	{
		gotoPreviousKey();
	}
	else if (command == L"Theater.GotoNextKey")
	{
		gotoNextKey();
	}
	else
		return false;

	return true;
}

void TheaterControllerEditor::draw(render::PrimitiveRenderer* primitiveRenderer)
{
	if (!m_context->getGuideEnable())
		return;

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData*, false >(sceneAsset->getControllerData());

	RefArray< ui::custom::SequenceItem > items;
	m_trackSequencer->getSequenceItems(items, ui::custom::SequencerControl::GfSelectedOnly);

	float duration = controllerData->getDuration();
	bool loop = controllerData->getLoop();

	const RefArray< TrackData >& trackData = controllerData->getTrackData();
	for (RefArray< TrackData >::const_iterator i = trackData.begin(); i != trackData.end(); ++i)
	{
		Color pathColor(180, 180, 80, 120);
		for (RefArray< ui::custom::SequenceItem >::const_iterator j = items.begin(); j != items.end(); ++j)
		{
			if ((*j)->getData(L"TRACK") == *i)
			{
				pathColor = Color(255, 255, 0, 200);
				break;
			}
		}

		const TransformPath& path = (*i)->getPath();

		int32_t steps = int32_t(duration) * 10;
		for (int32_t i = 0; i < steps; ++i)
		{
			float T0 = (float(i) / steps) * duration;
			float T1 = (float(i + 1) / steps) * duration;

			TransformPath::Frame F0 = path.evaluate(T0, duration, loop);
			TransformPath::Frame F1 = path.evaluate(T1, duration, loop);

			primitiveRenderer->drawLine(
				F0.position,
				F1.position,
				pathColor
			);
		}

		const AlignedVector< TransformPath::Key >& keys = path.getKeys();
		for (AlignedVector< TransformPath::Key >::const_iterator i = keys.begin(); i != keys.end(); ++i)
		{
			primitiveRenderer->drawSolidPoint(
				i->value.position,
				8.0f,
				pathColor
			);
		}
	}
}

void TheaterControllerEditor::updateSequencer()
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData*, false >(sceneAsset->getControllerData());

	m_trackSequencer->removeAllSequenceItems();

	RefArray< TrackData >& trackData = controllerData->getTrackData();
	for (RefArray< TrackData >::iterator i = trackData.begin(); i != trackData.end(); ++i)
	{
		Ref< ui::custom::Sequence > trackSequence = new ui::custom::Sequence((*i)->getInstance()->getName());
		trackSequence->setData(L"TRACK", *i);

		TransformPath& path = (*i)->getPath();
		AlignedVector< TransformPath::Key >& keys = path.getKeys();

		for (AlignedVector< TransformPath::Key >::iterator j = keys.begin(); j != keys.end(); ++j)
		{
			int32_t tickTime = int32_t(j->T * 1000.0f);

			Ref< ui::custom::Tick > tick = new ui::custom::Tick(tickTime);
			tick->setData(L"KEY", new TransformPathKeyWrapper(*j));

			trackSequence->addKey(tick);
		}

		m_trackSequencer->addSequenceItem(trackSequence);
	}

	m_trackSequencer->setLength(int32_t(controllerData->getDuration() * 1000.0f));
	m_trackSequencer->setCursor(int32_t(m_context->getTime() * 1000.0f));

	m_trackSequencer->update();
}

void TheaterControllerEditor::captureEntities()
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	RefArray< scene::EntityAdapter > selectedEntities;
	m_context->getEntities(selectedEntities, scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);
	if (selectedEntities.empty())
		return;

	float time = m_context->getTime();

	RefArray< TrackData >& trackData = controllerData->getTrackData();
	for (RefArray< scene::EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
	{
		if (!(*i)->isSpatial())
			continue;

		Transform transform = (*i)->getTransform();

		Ref< world::EntityInstance > instance = (*i)->getInstance();
		T_ASSERT (instance);

		Ref< TrackData > instanceTrackData;

		RefArray< TrackData >::iterator j = std::find_if(trackData.begin(), trackData.end(), FindInstanceTrackData(instance));
		if (j != trackData.end())
			instanceTrackData = *j;
		else
		{
			instanceTrackData = new TrackData();
			instanceTrackData->setInstance(instance);
			trackData.push_back(instanceTrackData);
		}

		T_ASSERT (instanceTrackData);
		TransformPath& pathData = instanceTrackData->getPath();

		TransformPath::Key* closestKey = pathData.getClosestKey(time);
		if (closestKey && abs(closestKey->T - time) < c_clampKeyDistance)
		{
			closestKey->value.position = transform.translation();
			closestKey->value.orientation = transform.rotation();
		}
		else
		{
			TransformPath::Frame frame;
			frame.position = transform.translation();
			frame.orientation = transform.rotation();
			pathData.insert(time, frame);
		}
	}

	m_context->buildEntities();
}

void TheaterControllerEditor::deleteSelectedKey()
{
	// @fixme
}

void TheaterControllerEditor::gotoPreviousKey()
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	float time = m_context->getTime();
	float previousTime = 0.0f;

	const RefArray< TrackData >& trackData = controllerData->getTrackData();
	for (RefArray< TrackData >::const_iterator i = trackData.begin(); i != trackData.end(); ++i)
	{
		TransformPath& path = (*i)->getPath();
		TransformPath::Key* key = path.getClosestPreviousKey(time);
		if (key && key->T > previousTime)
			previousTime = key->T;
	}

	int32_t cursorTick = int32_t(previousTime * 1000.0f);

	m_trackSequencer->setCursor(cursorTick);
	m_trackSequencer->update();

	m_context->setTime(previousTime);
	m_context->setPlaying(false);
}

void TheaterControllerEditor::gotoNextKey()
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	float time = m_context->getTime();
	float nextTime = controllerData->getDuration();

	const RefArray< TrackData >& trackData = controllerData->getTrackData();
	for (RefArray< TrackData >::const_iterator i = trackData.begin(); i != trackData.end(); ++i)
	{
		TransformPath& path = (*i)->getPath();
		TransformPath::Key* key = path.getClosestNextKey(time);
		if (key && key->T < nextTime)
			nextTime = key->T;
	}

	int32_t cursorTick = int32_t(nextTime * 1000.0f);

	m_trackSequencer->setCursor(cursorTick);
	m_trackSequencer->update();

	m_context->setTime(nextTime);
	m_context->setPlaying(false);
}

void TheaterControllerEditor::eventToolBarClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

void TheaterControllerEditor::eventSequencerCursorMove(ui::Event* event)
{
	int32_t cursorTick = m_trackSequencer->getCursor();
	float cursorTime = float(cursorTick / 1000.0f);

	m_context->setTime(cursorTime);
	m_context->setPlaying(false);
}

void TheaterControllerEditor::eventSequencerKeyMove(ui::Event* event)
{
	const ui::CommandEvent* commandEvent = checked_type_cast< const ui::CommandEvent* >(event);
	const ui::Command& command = commandEvent->getCommand();

	ui::custom::Tick* tick = checked_type_cast< ui::custom::Tick*, false >(commandEvent->getItem());

	TransformPathKeyWrapper* keyWrapper = static_cast< TransformPathKeyWrapper* >(tick->getData(L"KEY").ptr());
	T_ASSERT (keyWrapper);

	keyWrapper->m_key.T = tick->getTime() / 1000.0f;

	m_context->buildEntities();
}

void TheaterControllerEditor::eventContextPostFrame(ui::Event* event)
{
	if (m_context->isPlaying())
	{
		float cursorTime = m_context->getTime();

		int32_t cursorTickMax = m_trackSequencer->getLength();
		int32_t cursorTick = int32_t(cursorTime * 1000.0f) % cursorTickMax;

		m_trackSequencer->setCursor(cursorTick);
		m_trackSequencer->update();
	}
}

	}
}
