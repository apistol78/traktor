#include "Theater/Editor/TheaterControllerEditor.h"
#include "Theater/TheaterControllerData.h"
#include "Theater/TheaterController.h"
#include "Theater/TrackData.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/SceneAsset.h"
#include "World/Entity/EntityInstance.h"
#include "Ui/MethodHandler.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/Tick.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace theater
	{
		namespace
		{

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterControllerEditor", TheaterControllerEditor, scene::ISceneControllerEditor)

bool TheaterControllerEditor::create(scene::SceneEditorContext* context, ui::Container* parent)
{
	m_trackSequencer = gc_new< ui::custom::SequencerControl >();
	if (!m_trackSequencer->create(parent))
		return false;

	m_trackSequencer->addCursorMoveEventHandler(ui::createMethodHandler(this, &TheaterControllerEditor::eventSequencerCursorMove));

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
	else
		return false;

	return true;
}

void TheaterControllerEditor::updateSequencer()
{
	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	Ref< TheaterControllerData > controllerData = checked_type_cast< TheaterControllerData* >(sceneAsset->getControllerData());

	m_trackSequencer->removeAllSequenceItems();

	const RefArray< TrackData >& trackData = controllerData->getTrackData();
	for (RefArray< TrackData >::const_iterator i = trackData.begin(); i != trackData.end(); ++i)
	{
		Ref< ui::custom::Sequence > trackSequence = gc_new< ui::custom::Sequence >((*i)->getInstance()->getName());

		const TransformPath& path = (*i)->getPath();
		const AlignedVector< TransformPath::Key >& keys = path.getKeys();

		for (AlignedVector< TransformPath::Key >::const_iterator j = keys.begin(); j != keys.end(); ++j)
		{
			int32_t tickTime = int32_t(j->T * 1000.0f);
			trackSequence->addKey(gc_new< ui::custom::Tick >(tickTime));
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

		Matrix44 transform = (*i)->getTransform();

		Ref< world::EntityInstance > instance = (*i)->getInstance();
		T_ASSERT (instance);

		Ref< TrackData > instanceTrackData;

		RefArray< TrackData >::iterator j = std::find_if(trackData.begin(), trackData.end(), FindInstanceTrackData(instance));
		if (j != trackData.end())
			instanceTrackData = *j;
		else
		{
			instanceTrackData = gc_new< TrackData >();
			instanceTrackData->setInstance(instance);
			trackData.push_back(instanceTrackData);
		}

		T_ASSERT (instanceTrackData);

		TransformPath::Frame frame;
		frame.position = transform.translation();
		frame.orientation = Quaternion(transform.inverse()).normalized();

		TransformPath& pathData = instanceTrackData->getPath();
		pathData.insert(time, frame);
	}

	m_context->buildEntities();
}

void TheaterControllerEditor::eventSequencerCursorMove(ui::Event* event)
{
	int32_t cursorTick = m_trackSequencer->getCursor();
	float cursorTime = float(cursorTick / 1000.0f);

	m_context->setTime(cursorTime);
	m_context->setPlaying(false);
}

void TheaterControllerEditor::eventContextPostFrame(ui::Event* event)
{
	if (m_context->isPlaying())
	{
		float cursorTime = m_context->getTime();
		int32_t cursorTick = int32_t(cursorTime * 1000.0f);

		m_trackSequencer->setCursor(cursorTick);
		m_trackSequencer->update();
	}
}

	}
}
