#include <limits>
#include <stack>
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/SceneEditorProfile.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/Modifier.h"
#include "Scene/Editor/EntityEditor.h"
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/SceneAsset.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/Entity.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

// Find best matching entity editor.
EntityEditor* findEntityEditor(const RefArray< EntityEditor >& entityEditors, const Type& entityType)
{
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< EntityEditor > entityEditor;
	
	for (RefArray< EntityEditor >::const_iterator i = entityEditors.begin(); i != entityEditors.end(); ++i)
	{
		TypeSet entityTypes = (*i)->getEntityTypes();
		for (TypeSet::const_iterator j = entityTypes.begin(); j != entityTypes.end(); ++j)
		{
			if (is_type_of(**j, entityType))
			{
				uint32_t classDifference = type_difference(**j, entityType);
				if (classDifference < minClassDifference)
				{
					entityEditor = *i;
					minClassDifference = classDifference;
				}
			}
		}
	}

	return entityEditor;
}

// Attach entity editor in each adapter.
void recursiveAttachEditors(EntityAdapter* entityAdapter, const RefArray< EntityEditor >& entityEditors)
{
	Ref< EntityEditor > entityEditor = findEntityEditor(entityEditors, entityAdapter->getEntityData()->getType());
	if (entityEditor)
		entityAdapter->setEntityEditor(entityEditor);

	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
		recursiveAttachEditors(*i, entityEditors);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorContext", SceneEditorContext, ui::EventSubject)

SceneEditorContext::SceneEditorContext(
	editor::IEditor* editor,
	db::Database* resourceDb,
	db::Database* sourceDb,
	resource::IResourceManager* resourceManager,
	render::RenderSystem* renderSystem,
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
,	m_editSpace(EsWorld)
,	m_guideEnable(true)
,	m_snapEnable(true)
,	m_deltaScale(0.05f)
,	m_physicsEnable(false)
,	m_timeScale(1.0f)
,	m_referenceMode(false)
{
	const Matrix44 identity = Matrix44::identity();
	m_camera = gc_new< Camera >(cref(identity));
}

void SceneEditorContext::addEditorProfile(SceneEditorProfile* editorProfile)
{
	m_editorProfiles.push_back(editorProfile);
}

void SceneEditorContext::setCamera(Camera* camera)
{
	m_camera = camera;
}

Camera* SceneEditorContext::getCamera() const
{
	return m_camera;
}

void SceneEditorContext::setModifier(Modifier* modifier)
{
	m_modifier = modifier;
}

Modifier* SceneEditorContext::getModifier() const
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

void SceneEditorContext::setEditSpace(EditSpace editSpace)
{
	m_editSpace = editSpace;
}

SceneEditorContext::EditSpace SceneEditorContext::getEditSpace() const
{
	return m_editSpace;
}

void SceneEditorContext::setGuideEnable(bool guideEnable)
{
	m_guideEnable = guideEnable;
}

bool SceneEditorContext::getGuideEnable() const
{
	return m_guideEnable;
}

void SceneEditorContext::setSnapEnable(bool snapEnable)
{
	m_snapEnable = snapEnable;
}

bool SceneEditorContext::getSnapEnable() const
{
	return m_snapEnable;
}

void SceneEditorContext::setDeltaScale(float deltaScale)
{
	m_deltaScale = deltaScale;
}

float SceneEditorContext::getDeltaScale() const
{
	return m_deltaScale;
}

void SceneEditorContext::setPhysicsEnable(bool physicsEnable)
{
	m_physicsEnable = physicsEnable;
}

bool SceneEditorContext::getPhysicsEnable() const
{
	return m_physicsEnable;
}

void SceneEditorContext::setTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float SceneEditorContext::getTimeScale() const
{
	return m_timeScale;
}

void SceneEditorContext::setReferenceMode(bool referenceMode)
{
	m_referenceMode = referenceMode;
}

bool SceneEditorContext::inReferenceMode() const
{
	return m_referenceMode;
}

void SceneEditorContext::addEntityEditor(EntityEditor* entityEditor)
{
	m_entityEditors.push_back(entityEditor);
}

void SceneEditorContext::drawGuide(render::PrimitiveRenderer* primitiveRenderer, EntityAdapter* entityAdapter)
{
	Ref< EntityEditor > entityEditor = entityAdapter->getEntityEditor();
	if (entityEditor)
		entityEditor->drawGuide(this, primitiveRenderer, entityAdapter);
}

void SceneEditorContext::setSceneAsset(SceneAsset* sceneAsset)
{
	m_sceneAsset = sceneAsset;
}

void SceneEditorContext::resetEntities()
{
	//RefArray< EntityAdapter > entityAdapters;
	//getEntities(entityAdapters);

	//for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	//{
	//	if ((*i)->isSpatial())
	//		(*i)->forceModified();
	//}

	//buildEntities();

	T_BREAKPOINT;
}

void SceneEditorContext::buildEntities()
{
	if (m_sceneAsset)
	{
		Ref< EntityAdapterBuilder > entityBuilder = gc_new< EntityAdapterBuilder >(this);

		for (RefArray< SceneEditorProfile >::iterator i = m_editorProfiles.begin(); i != m_editorProfiles.end(); ++i)
		{
			RefArray< world::IEntityFactory > entityFactories;
			(*i)->createEntityFactories(this, entityFactories);

			for (RefArray< world::IEntityFactory >::iterator j = entityFactories.begin(); j != entityFactories.end(); ++j)
				entityBuilder->addFactory(*j);
		}

		// (Re-)build entities.
		{
			entityBuilder->begin(0);
			entityBuilder->build(m_sceneAsset->getInstance());
			entityBuilder->end();

			// Save new root entity adapter.
			m_rootEntityAdapter = entityBuilder->getRootAdapter();

			// Ensure no references exist to our entity builder; not allowed to cache it.
			Heap::getInstance().invalidateRefs(entityBuilder);
		}

		// Attach entity editors.
		if (m_rootEntityAdapter)
			recursiveAttachEditors(m_rootEntityAdapter, m_entityEditors);
	}
	else
		m_rootEntityAdapter = 0;

	// Force a collect in order to as quickly as possible remove old, unused, entities.
	Heap::getInstance().collect();

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

EntityAdapter* SceneEditorContext::findAdapterFromInstance(const world::EntityInstance* instance) const
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		if ((*i)->getInstance() == instance)
			return *i;
	}

	return 0;
}

EntityAdapter* SceneEditorContext::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection) const
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	Ref< EntityAdapter > minEntityAdapter;
	Scalar minDistance(1e8f);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		if (!(*i)->isSpatial())
			continue;

		// Transform ray into object space.
		Matrix44 worldInv = (*i)->getTransform().inverseOrtho();
		Vector4 objectRayOrigin = worldInv * worldRayOrigin;
		Vector4 objectRayDirection = worldInv * worldRayDirection;

		// Get entity bounding box; do not pick if origin of ray is within box.
		Aabb boundingBox = (*i)->getBoundingBox();
		if (boundingBox.empty() || boundingBox.inside(objectRayOrigin))
			continue;

		// Trace bounding box to see if ray intersect.
		Scalar distance;
		if (boundingBox.intersect(objectRayOrigin, objectRayOrigin + objectRayDirection * minDistance, distance))
		{
			T_ASSERT (distance < minDistance);
			minEntityAdapter = *i;
			minDistance = distance;
		}
	}

	return minEntityAdapter;
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

void SceneEditorContext::raiseSelect()
{
	raiseEvent(EiSelect, 0);
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
