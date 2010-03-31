#include <limits>
#include <stack>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/DeepClone.h"
#include "Resource/IResourceManager.h"
#include "Scene/ISceneController.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/IEntityEditorFactory.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/SceneAsset.h"
#include "Ui/Event.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/EntityManager.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

// Find best matching entity editor.
Ref< IEntityEditor > createEntityEditor(
	SceneEditorContext* context,
	const RefArray< IEntityEditorFactory >& entityEditorFactories,
	const TypeInfo& entityDataType
)
{
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< IEntityEditorFactory > entityEditorFactory;

	for (RefArray< IEntityEditorFactory >::const_iterator i = entityEditorFactories.begin(); i != entityEditorFactories.end(); ++i)
	{
		TypeInfoSet entityDataTypes = (*i)->getEntityDataTypes();
		for (TypeInfoSet::const_iterator j = entityDataTypes.begin(); j != entityDataTypes.end(); ++j)
		{
			if (is_type_of(**j, entityDataType))
			{
				uint32_t classDifference = type_difference(**j, entityDataType);
				if (classDifference < minClassDifference)
				{
					entityEditorFactory = *i;
					minClassDifference = classDifference;
				}
			}
		}
	}

	if (entityEditorFactory)
	{
		Ref< IEntityEditor > entityEditor = entityEditorFactory->createEntityEditor(context, entityDataType);
		T_ASSERT_M (entityEditor, L"Entity editor factory returned null");
		return entityEditor;
	}
	else
		return 0;
}

// Create entity editor for each adapter.
void createEntityEditors(
	SceneEditorContext* context,
	const RefArray< IEntityEditorFactory >& entityEditorFactories,
	EntityAdapter* entityAdapter
)
{
	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();

	if (!entityAdapter->getEntityEditor() && entityAdapter->getEntityData())
	{
		Ref< IEntityEditor > entityEditor = createEntityEditor(
			context,
			entityEditorFactories,
			type_of(entityAdapter->getEntityData())
		);
		if (entityEditor)
			entityAdapter->setEntityEditor(entityEditor);
	}

	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
		createEntityEditors(context, entityEditorFactories, *i);
}

// Find adapter by traverse entity adapter hierarchy.
template < typename Predicate >
EntityAdapter* findAdapter(EntityAdapter* entityAdapter, Predicate predicate)
{
	if (predicate(entityAdapter))
		return entityAdapter;

	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		EntityAdapter* foundEntityAdapter = findAdapter(*i, predicate);
		if (foundEntityAdapter)
			return foundEntityAdapter;
	}

	return 0;
}

struct FindFromEntity
{
	const world::Entity* m_entity;

	bool operator () (const EntityAdapter* entityAdapter) const
	{
		return entityAdapter->getEntity() == m_entity;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorContext", SceneEditorContext, ui::EventSubject)

SceneEditorContext::SceneEditorContext(
	editor::IEditor* editor,
	db::Database* resourceDb,
	db::Database* sourceDb,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	physics::PhysicsManager* physicsManager
)
:	m_editor(editor)
,	m_resourceDb(resourceDb)
,	m_sourceDb(sourceDb)
,	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_physicsManager(physicsManager)
,	m_pickEnable(true)
,	m_axisEnable(AeXYZ)
,	m_snapEnable(true)
,	m_physicsEnable(false)
,	m_playing(false)
,	m_timeScale(1.0f)
,	m_time(0.0f)
{
	for (int i = 0; i < sizeof_array(m_cameras); ++i)
		m_cameras[i] = new Camera(
			lookAt(
				Vector4(-4.0f, 4.0f, -4.0f, 1.0f),
				Vector4(0.0f, 0.0f, 0.0f, 1.0f)
			)
		);
}

void SceneEditorContext::addEditorProfile(ISceneEditorProfile* editorProfile)
{
	m_editorProfiles.push_back(editorProfile);
}

void SceneEditorContext::addEditorPlugin(ISceneEditorPlugin* editorPlugin)
{
	m_editorPlugins.push_back(editorPlugin);
}

void SceneEditorContext::setControllerEditor(ISceneControllerEditor* controllerEditor)
{
	m_controllerEditor = controllerEditor;
}

void SceneEditorContext::setInstance(db::Instance* instance)
{
	m_instance = instance;
}

void SceneEditorContext::setModifier(IModifier* modifier)
{
	m_modifier = modifier;
}

Ref< IModifier > SceneEditorContext::getModifier() const
{
	return m_modifier;
}

void SceneEditorContext::setPickEnable(bool pickEnable)
{
	m_pickEnable = pickEnable;
}

bool SceneEditorContext::getPickEnable() const
{
	return m_pickEnable;
}

void SceneEditorContext::setAxisEnable(uint32_t axisEnable)
{
	m_axisEnable = axisEnable;
}

uint32_t SceneEditorContext::getAxisEnable() const
{
	return m_axisEnable;
}

void SceneEditorContext::setSnapEnable(bool snapEnable)
{
	m_snapEnable = snapEnable;
}

bool SceneEditorContext::getSnapEnable() const
{
	return m_snapEnable;
}

void SceneEditorContext::setPhysicsEnable(bool physicsEnable)
{
	m_physicsEnable = physicsEnable;
}

bool SceneEditorContext::getPhysicsEnable() const
{
	return m_physicsEnable;
}

Ref< Camera > SceneEditorContext::getCamera(int index) const
{
	return m_cameras[index];
}

void SceneEditorContext::setFollowEntityAdapter(EntityAdapter* followEntityAdapter)
{
	m_followEntityAdapter = followEntityAdapter;
}

void SceneEditorContext::setPlaying(bool playing)
{
	m_playing = playing;
}

bool SceneEditorContext::isPlaying() const
{
	return m_playing;
}

void SceneEditorContext::setTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float SceneEditorContext::getTimeScale() const
{
	return m_timeScale;
}

void SceneEditorContext::setTime(float time)
{
	m_time = time;
}

float SceneEditorContext::getTime() const
{
	return m_time;
}

void SceneEditorContext::drawGuide(render::PrimitiveRenderer* primitiveRenderer, EntityAdapter* entityAdapter)
{
	Ref< IEntityEditor > entityEditor = entityAdapter->getEntityEditor();
	if (entityEditor)
		entityEditor->drawGuide(this, primitiveRenderer, entityAdapter);
}

void SceneEditorContext::setSceneAsset(SceneAsset* sceneAsset)
{
	m_sceneAsset = sceneAsset;
}

void SceneEditorContext::buildEntities()
{
	if (m_sceneAsset)
	{
		Ref< EntityAdapterBuilder > entityBuilder = new EntityAdapterBuilder(this);

		// Create entity factories.
		for (RefArray< ISceneEditorProfile >::iterator i = m_editorProfiles.begin(); i != m_editorProfiles.end(); ++i)
		{
			RefArray< world::IEntityFactory > entityFactories;
			(*i)->createEntityFactories(this, entityFactories);

			for (RefArray< world::IEntityFactory >::iterator j = entityFactories.begin(); j != entityFactories.end(); ++j)
				entityBuilder->addFactory(*j);
		}

		// Create entity manager and build root instance.
		Ref< world::IEntityManager > entityManager = new world::EntityManager();

		entityBuilder->begin(entityManager);
		Ref< world::Entity > rootEntity = entityBuilder->create(m_sceneAsset->getEntityData());

		// Update scene controller also.
		Ref< ISceneController > controller;
		if (m_sceneAsset->getControllerData())
			controller = m_sceneAsset->getControllerData()->createController(entityBuilder, entityManager);

		entityBuilder->end();

		// Save new root entity adapter.
		m_rootEntityAdapter = entityBuilder->getRootAdapter();

		// Create entity editors.
		if (m_rootEntityAdapter)
		{
			RefArray< IEntityEditorFactory > entityEditorFactories;
			for (RefArray< ISceneEditorProfile >::iterator i = m_editorProfiles.begin(); i != m_editorProfiles.end(); ++i)
				(*i)->createEntityEditorFactories(this, entityEditorFactories);
			createEntityEditors(this, entityEditorFactories, m_rootEntityAdapter);
		}

		// Bind post process settings.
		resource::Proxy< world::PostProcessSettings > postProcessSettings = m_sceneAsset->getPostProcessSettings();
		m_resourceManager->bind(postProcessSettings);

		// Create our scene.
		m_scene = new Scene(
			controller,
			entityManager,
			rootEntity,
			m_sceneAsset->getWorldRenderSettings(),
			postProcessSettings
		);
	}
	else
	{
		m_scene = 0;
		m_rootEntityAdapter = 0;
	}

	raisePostBuild();
}

void SceneEditorContext::selectEntity(EntityAdapter* entityAdapter, bool select)
{
	if (entityAdapter && entityAdapter->m_selected != select)
	{
		entityAdapter->m_selected = select;
		if (entityAdapter->m_entityEditor)
			entityAdapter->m_entityEditor->entitySelected(this, entityAdapter, select);
	}
}

void SceneEditorContext::selectAllEntities(bool select)
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
		selectEntity(*i, select);
}

uint32_t SceneEditorContext::getEntities(RefArray< EntityAdapter >& outEntityAdapters, uint32_t flags) const
{
	typedef std::pair< RefArray< EntityAdapter >::const_iterator, RefArray< EntityAdapter >::const_iterator > range_t;

	outEntityAdapters.resize(0);

	if (!m_rootEntityAdapter)
		return 0;

	std::stack< range_t > stack;

	RefArray< EntityAdapter > rootEntityAdapters(1);
	rootEntityAdapters[0] = m_rootEntityAdapter;

	stack.push(std::make_pair(rootEntityAdapters.begin(), rootEntityAdapters.end()));
	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			Ref< EntityAdapter > entityAdapter = *r.first++;

			if (flags & GfSelectedOnly)
			{
				if (entityAdapter->isSelected())
					outEntityAdapters.push_back(entityAdapter);
			}
			else
				outEntityAdapters.push_back(entityAdapter);

			if (flags & GfDescendants)
			{
				const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
				if (!children.empty())
					stack.push(std::make_pair(children.begin(), children.end()));
			}
		}
		else
			stack.pop();
	}

	return uint32_t(outEntityAdapters.size());
}

EntityAdapter* SceneEditorContext::findAdapterFromEntity(const world::Entity* entity) const
{
	FindFromEntity predicate;
	predicate.m_entity = entity;
	return findAdapter(m_rootEntityAdapter, predicate);
}

EntityAdapter* SceneEditorContext::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection) const
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	EntityAdapter* minEntityAdapter = 0;
	Scalar minDistance(1e8f);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		// Must be visible, spatial and exclusively pick-able.
		if (
			!(*i)->isVisible() ||
			!(*i)->isSpatial() ||
			!(*i)->getEntityEditor() ||
			!(*i)->getEntityEditor()->isPickable(*i)
		)
			continue;

		// Transform ray into object space.
		Transform worldInv = (*i)->getTransform().inverse();
		Vector4 objectRayOrigin = worldInv * worldRayOrigin;
		Vector4 objectRayDirection = worldInv * worldRayDirection;

		// Get entity bounding box; do not pick if origin of ray is within box.
		Aabb boundingBox = (*i)->getBoundingBox();
		if (boundingBox.empty() || boundingBox.inside(objectRayOrigin))
			continue;

		// Trace bounding box to see if ray intersect.
		Scalar distance;
		if (boundingBox.intersectSegment(objectRayOrigin, objectRayOrigin + objectRayDirection * (minDistance - Scalar(FUZZY_EPSILON)), distance))
		{
			T_ASSERT (distance <= minDistance);
			minEntityAdapter = *i;
			minDistance = distance;
		}
	}

	return minEntityAdapter;
}

void SceneEditorContext::cloneSelected()
{
	RefArray< EntityAdapter > selectedEntityAdapters;
	getEntities(selectedEntityAdapters, GfDescendants | GfSelectedOnly);

	for (RefArray< EntityAdapter >::iterator i = selectedEntityAdapters.begin(); i != selectedEntityAdapters.end(); ++i)
	{
		Ref< EntityAdapter > parentContainerGroup = (*i)->getParentContainerGroup();
		T_ASSERT (parentContainerGroup);

		Ref< world::EntityData > clonedEntityData = DeepClone((*i)->getEntityData()).create< world::EntityData >();
		T_ASSERT (clonedEntityData);

		Ref< EntityAdapter > clonedEntityAdapter = new EntityAdapter(clonedEntityData);
		parentContainerGroup->addChild(clonedEntityAdapter);

		(*i)->m_selected = false;
		clonedEntityAdapter->m_selected = true;
	}

	buildEntities();
	raiseSelect(this);
}

void SceneEditorContext::setDebugTexture(render::ITexture* debugTexture)
{
	m_debugTexture = debugTexture;
}

Ref< render::ITexture > SceneEditorContext::getDebugTexture()
{
	return m_debugTexture;
}

void SceneEditorContext::raisePreModify()
{
	raiseEvent(EiPreModify, 0);
}

void SceneEditorContext::raisePostModify()
{
	raiseEvent(EiPostModify, 0);
}

void SceneEditorContext::raisePostFrame(ui::Event* event)
{
	raiseEvent(EiPostFrame, event);
}

void SceneEditorContext::raisePostBuild()
{
	raiseEvent(EiPostBuild, 0);
}

void SceneEditorContext::raiseSelect(Object* item)
{
	ui::Event event(this, item);
	raiseEvent(EiSelect, &event);
}

void SceneEditorContext::addPreModifyEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPreModify, eventHandler);
}

void SceneEditorContext::addPostModifyEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostModify, eventHandler);
}

void SceneEditorContext::addPostFrameEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostFrame, eventHandler);
}

void SceneEditorContext::addPostBuildEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostBuild, eventHandler);
}

void SceneEditorContext::addSelectEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiSelect, eventHandler);
}

	}
}
