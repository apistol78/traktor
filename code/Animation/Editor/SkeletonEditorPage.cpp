#include <cmath>
#include "Animation/Editor/SkeletonEditorPage.h"
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
#include "Animation/SkeletonUtils.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/IProject.h"
#include "Editor/UndoStack.h"
#include "Database/Database.h"
#include "Resource/ResourceManager.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewState.h"
#include "Ui/TreeViewItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Itf/IWidget.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/TextureFactory.h"
#include "Render/ShaderFactory.h"
#include "Render/PrimitiveRenderer.h"
#include "Core/Math/Const.h"
#include "Core/Math/Vector2.h"

// Resources
#include "Resources/Bones.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

int findIndexOfBone(const Skeleton* skeleton, const Bone* bone)
{
	for (int i = 0; i < int(skeleton->getBoneCount()); ++i)
	{
		if (skeleton->getBone(i) == bone)
			return i;
	}
	return -1;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonEditorPage", SkeletonEditorPage, editor::IEditorPage)

SkeletonEditorPage::SkeletonEditorPage(editor::IEditor* editor)
:	m_editor(editor)
,	m_selectedBone(-1)
,	m_cameraHead(0.0f)
,	m_cameraY(0.0f)
,	m_cameraZ(0.0f)
,	m_cameraMoveScaleY(1.0f)
,	m_cameraMoveScaleZ(1.0f)
,	m_cameraBoneScale(1.0f)
{
}

bool SkeletonEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	m_site = site;
	T_ASSERT (site);

	Ref< render::IRenderSystem > renderSystem = m_editor->getRenderSystem();

	m_renderWidget = gc_new< ui::Widget >();
	m_renderWidget->create(parent, ui::WsNone);
	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventMouseDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventMouseUp));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventMouseMove));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventPaint));

	m_boneMenu = gc_new< ui::PopupMenu >();
	m_boneMenu->create();
	m_boneMenu->add(gc_new< ui::MenuItem >(ui::Command(L"Skeleton.Editor.AddBone"), i18n::Text(L"SKELETON_EDITOR_ADD_BONE")));
	m_boneMenu->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Delete"), i18n::Text(L"SKELETON_EDITOR_DELETE_BONE")));

	// Create skeleton panel.
	m_skeletonPanel = gc_new< ui::Container >();
	m_skeletonPanel->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"100%", 0, 0));
	m_skeletonPanel->setText(i18n::Text(L"SKELETON_EDITOR_SKELETON"));

	m_treeSkeleton = gc_new< ui::TreeView >();
	m_treeSkeleton->create(m_skeletonPanel, ui::TreeView::WsDefault & ~ui::WsClientBorder);
	m_treeSkeleton->addImage(ui::Bitmap::load(c_ResourceBones, sizeof(c_ResourceBones), L"png"), 2);
	m_treeSkeleton->addButtonDownEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventTreeButtonDown));
	m_treeSkeleton->addSelectEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventTreeSelect));
	m_treeSkeleton->addEditedEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventTreeEdited));

	m_site->createAdditionalPanel(m_skeletonPanel, 250, false);

	render::RenderViewCreateDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.mipBias = 0.0f;

	m_renderView = renderSystem->createRenderView(m_renderWidget->getIWidget()->getSystemHandle(), desc);
	if (!m_renderView)
		return false;

	Ref< editor::IProject > project = m_editor->getProject();
	Ref< db::Database > database = project->getOutputDatabase();

	m_resourceManager = gc_new< resource::ResourceManager >();
	m_resourceManager->addFactory(
		gc_new< render::TextureFactory >(database, renderSystem)
	);
	m_resourceManager->addFactory(
		gc_new< render::ShaderFactory >(database, renderSystem)
	);

	m_primitiveRenderer = gc_new< render::PrimitiveRenderer >();
	if (!m_primitiveRenderer->create(m_resourceManager, renderSystem))
		return false;

	m_renderWidget->startTimer(30);
	m_undoStack = gc_new< editor::UndoStack >();

	return true;
}

void SkeletonEditorPage::destroy()
{
	m_site->destroyAdditionalPanel(m_skeletonPanel);
	
	m_renderView->close();

	// Destroy widgets.h
	m_boneMenu->destroy();
	m_skeletonPanel->destroy();
	m_renderWidget->destroy();
}

void SkeletonEditorPage::activate()
{
}

void SkeletonEditorPage::deactivate()
{
}

bool SkeletonEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	m_skeletonInstance = instance;
	m_skeleton = checked_type_cast< Skeleton* >(data);
	m_site->setPropertyObject(m_skeleton);

	Aabb boundingBox = calculateBoundingBox(m_skeleton);
	
	int majorAxis = majorAxis3(boundingBox.getExtent());
	float majorExtent = boundingBox.getExtent()[majorAxis];

	m_cameraHead = 0.0f;
	m_cameraY = 0.0f;
	m_cameraZ = majorExtent * 2.0f;

	m_cameraMoveScaleY = majorExtent / 200.0f;
	m_cameraMoveScaleZ = majorExtent / 100.0f;
	m_cameraBoneScale = majorExtent / 100.0f;

	createSkeletonTreeNodes();
	return true;
}

Ref< db::Instance > SkeletonEditorPage::getDataInstance()
{
	return m_skeletonInstance;
}

Ref< Object > SkeletonEditorPage::getDataObject()
{
	return m_skeleton;
}

void SkeletonEditorPage::propertiesChanged()
{
	createSkeletonTreeNodes();
	m_renderWidget->update();
}

bool SkeletonEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool SkeletonEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Undo")
	{
		if (!m_undoStack->canUndo())
			return false;

		m_skeleton = checked_type_cast< Skeleton* >(m_undoStack->undo(m_skeleton));

		createSkeletonTreeNodes();

		m_site->setPropertyObject(m_skeleton);
		m_renderWidget->update();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_undoStack->canRedo())
			return false;

		m_skeleton = checked_type_cast< Skeleton* >(m_undoStack->redo(m_skeleton));

		createSkeletonTreeNodes();

		m_site->setPropertyObject(m_skeleton);
		m_renderWidget->update();
	}
	else if (command == L"Editor.Delete")
	{
		Ref< ui::TreeViewItem > selectedTreeItem = m_treeSkeleton->getSelectedItem();
		if (!selectedTreeItem)
			return true;

		m_undoStack->push(m_skeleton);

		Ref< Bone > bone = selectedTreeItem->getData< Bone >(L"BONE");

		int boneIndex = findIndexOfBone(m_skeleton, bone);
		T_ASSERT (boneIndex >= 0);

		for (int i = 0; i < int(m_skeleton->getBoneCount()); ++i)
		{
			if (m_skeleton->getBone(i)->getParent() == boneIndex)
				m_skeleton->getBone(i)->setParent(-1);
		}

		m_skeleton->removeBone(bone);

		createSkeletonTreeNodes();

		m_site->setPropertyObject(m_skeleton);
		m_renderWidget->update();
	}
	else if (command == L"Skeleton.Editor.AddBone")
	{
		Ref< ui::TreeViewItem > selectedTreeItem = m_treeSkeleton->getSelectedItem();
		if (!selectedTreeItem)
			return true;

		m_undoStack->push(m_skeleton);

		Ref< Bone > parentBone = selectedTreeItem->getData< Bone >(L"BONE");

		Ref< Bone > bone = gc_new< Bone >();
		bone->setName(L"Bone");
		bone->setParent(parentBone ? findIndexOfBone(m_skeleton, parentBone) : -1);
		m_skeleton->addBone(bone);

		createSkeletonTreeNodes();

		m_site->setPropertyObject(bone);
		m_renderWidget->update();
	}
	else
		return false;

	return true;
}

void SkeletonEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->update(eventId, true);
}

void SkeletonEditorPage::createSkeletonTreeNodes()
{
	Ref< ui::TreeViewState > treeSkeletonState = m_treeSkeleton->captureState();

	m_treeSkeleton->removeAllItems();

	Ref< ui::TreeViewItem > itemRoot = m_treeSkeleton->createItem(0, i18n::Text(L"SKELETON_EDITOR_ROOT"), 0);
	createSkeletonTreeNodes(itemRoot, -1);

	m_treeSkeleton->applyState(treeSkeletonState);
}

void SkeletonEditorPage::createSkeletonTreeNodes(ui::TreeViewItem* parentItem, int parentNodeIndex)
{
	int boneCount = m_skeleton->getBoneCount();
	for (int i = 0; i < boneCount; ++i)
	{
		Bone* bone = m_skeleton->getBone(i);
		if (bone->getParent() == parentNodeIndex)
		{
			Ref< ui::TreeViewItem > itemBone = m_treeSkeleton->createItem(parentItem, bone->getName(), 1);
			itemBone->setData(L"BONE", bone);

			createSkeletonTreeNodes(itemBone, i);
		}
	}
}

void SkeletonEditorPage::eventMouseDown(ui::Event* event)
{
	if (!m_skeleton)
		return;

	m_undoStack->push(m_skeleton);

	m_lastMousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();
	m_renderWidget->setCapture();
	m_renderWidget->setFocus();
}

void SkeletonEditorPage::eventMouseUp(ui::Event* event)
{
	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();
}

void SkeletonEditorPage::eventMouseMove(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);

	if (!m_skeleton || !m_renderWidget->hasCapture())
		return;

	ui::Point mousePosition = mouseEvent->getPosition();

	Vector2 mouseDelta(
		float(m_lastMousePosition.x - mousePosition.x),
		float(m_lastMousePosition.y - mousePosition.y)
	);

	if ((mouseEvent->getKeyState() & ui::KsControl) == 0)
	{
		if (m_selectedBone >= 0)
		{
			Bone* bone = m_skeleton->getBone(m_selectedBone);

			mouseDelta /= 200.0f;

			if (mouseEvent->getButton() == ui::MouseEvent::BtLeft)
			{
				Quaternion orientation = bone->getOrientation();

				orientation *= Quaternion(Vector4(0.0f, mouseDelta.x, 0.0f, 0.0f));
				orientation *= Quaternion(Vector4(mouseDelta.y, 0.0f, 0.0f, 0.0f));

				bone->setOrientation(orientation);
			}
			else if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
			{
				float length = bone->getLength() + mouseDelta.y * m_cameraBoneScale;
				bone->setLength(Scalar(std::max(length, 0.0f)));
			}
		}
	}
	else
	{
		m_cameraHead += mouseDelta.x / 100.0f;
		if (mouseEvent->getButton() == ui::MouseEvent::BtLeft)
			m_cameraZ -= mouseDelta.y * m_cameraMoveScaleZ;
		else
			m_cameraY += mouseDelta.y * m_cameraMoveScaleY;
	}

	m_lastMousePosition = mousePosition;

	m_renderWidget->update();
}

void SkeletonEditorPage::eventSize(ui::Event* event)
{
	if (!m_renderView)
		return;

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	m_renderView->resize(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void SkeletonEditorPage::eventPaint(ui::Event* event)
{
	ui::PaintEvent* paintEvent = checked_type_cast< ui::PaintEvent* >(event);
	ui::Rect rc = m_renderWidget->getInnerRect();

	T_ASSERT (m_renderView);
	T_ASSERT (m_primitiveRenderer);

	if (!m_skeleton)
		return;

	if (!m_renderView->begin())
		return;

	const float clearColor[] = { 0.5f, 0.5f, 0.44f, 0.0f };
	m_renderView->clear(
		render::CfColor | render::CfDepth,
		clearColor,
		1.0f,
		128
	);

	render::Viewport viewport = m_renderView->getViewport();
	float aspect = float(viewport.width) / viewport.height;

	Matrix44 viewTransform = translate(0.0f, m_cameraY, m_cameraZ) * rotateY(m_cameraHead);
	Matrix44 projectionTransform = perspectiveLh(
		80.0f * PI / 180.0f,
		aspect,
		0.1f,
		2000.0f
	);

	if (m_primitiveRenderer->begin(m_renderView))
	{
		m_primitiveRenderer->pushProjection(projectionTransform);
		m_primitiveRenderer->pushView(viewTransform);

		for (int x = -10; x <= 10; ++x)
		{
			m_primitiveRenderer->drawLine(
				Vector4(float(x), 0.0f, -10.0f, 1.0f),
				Vector4(float(x), 0.0f, 10.0f, 1.0f),
				Color(100, 100, 100)
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				Color(100, 100, 100)
			);
		}

		AlignedVector< Transform > boneTransforms;
		calculateBoneTransforms(
			m_skeleton,
			boneTransforms
		);

		for (int i = 0; i < int(m_skeleton->getBoneCount()); ++i)
		{
			const Bone* bone = m_skeleton->getBone(i);

			Vector4 start = boneTransforms[i].translation();
			Vector4 end = boneTransforms[i].translation() + boneTransforms[i] * Vector4(0.0f, 0.0f, bone->getLength(), 0.0f);

			Color color = (m_selectedBone == i) ? Color(255, 128, 255, 128) : Color(255, 255, 0, 128);

			Vector4 d = boneTransforms[i].axisZ();
			Vector4 a = boneTransforms[i].axisX();
			Vector4 b = boneTransforms[i].axisY();

			Scalar radius = bone->getRadius();
			d *= radius;
			a *= radius;
			b *= radius;

			m_primitiveRenderer->drawLine(start, start + d + a + b, color);
			m_primitiveRenderer->drawLine(start, start + d - a + b, color);
			m_primitiveRenderer->drawLine(start, start + d + a - b, color);
			m_primitiveRenderer->drawLine(start, start + d - a - b, color);

			m_primitiveRenderer->drawLine(start + d + a + b, end, color);
			m_primitiveRenderer->drawLine(start + d - a + b, end, color);
			m_primitiveRenderer->drawLine(start + d + a - b, end, color);
			m_primitiveRenderer->drawLine(start + d - a - b, end, color);

			m_primitiveRenderer->drawLine(start + d + a + b, start + d - a + b, color);
			m_primitiveRenderer->drawLine(start + d - a + b, start + d - a - b, color);
			m_primitiveRenderer->drawLine(start + d - a - b, start + d + a - b, color);
			m_primitiveRenderer->drawLine(start + d + a - b, start + d + a + b, color);

			m_primitiveRenderer->drawLine(start, end, Color(255, 255, 128, 128));
			m_primitiveRenderer->drawLine(start, start + a * Scalar(2.0f), Color(255, 0, 0, 128));
			m_primitiveRenderer->drawLine(start, start + b * Scalar(2.0f), Color(0, 255, 0, 128));

			if (bone->getEnableLimits())
			{
				m_primitiveRenderer->drawCone(
					boneTransforms[i].toMatrix44(),
					bone->getConeLimit().x,
					bone->getConeLimit().y,
					radius,
					Color(255, 255, 255, 64),
					Color(0, 0, 0, 32)
				);

				m_primitiveRenderer->drawProtractor(
					start,
					boneTransforms[i].axisX(),
					boneTransforms[i].axisY(),
					-bone->getTwistLimit(),
					bone->getTwistLimit(),
					deg2rad(8.0f),
					radius,
					Color(255, 255, 255, 64),
					Color(0, 0, 0, 32)
				);
			}
		}

		m_primitiveRenderer->end(m_renderView);
	}

	m_renderView->end();
	m_renderView->present();

	paintEvent->consume();
}

void SkeletonEditorPage::eventTreeButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() != ui::MouseEvent::BtRight)
		return;

	Ref< ui::MenuItem > menuItem = m_boneMenu->show(m_treeSkeleton, mouseEvent->getPosition());
	if (menuItem)
	{
		if (handleCommand(menuItem->getCommand()))
			mouseEvent->consume();
	}
}

void SkeletonEditorPage::eventTreeSelect(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent* >(event);
	ui::TreeViewItem* selectedItem = checked_type_cast< ui::TreeViewItem* >(cmdEvent->getItem());

	Ref< Bone > bone = selectedItem->getData< Bone >(L"BONE");
	m_selectedBone = findIndexOfBone(m_skeleton, bone);

	if (bone)
		m_site->setPropertyObject(bone);
	else
		m_site->setPropertyObject(m_skeleton);

	m_renderWidget->update();
}

void SkeletonEditorPage::eventTreeEdited(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent* >(event);
	ui::TreeViewItem* selectedItem = checked_type_cast< ui::TreeViewItem* >(cmdEvent->getItem());

	Ref< Bone > bone = selectedItem->getData< Bone >(L"BONE");
	std::wstring name = selectedItem->getText();

	bone->setName(name);
}

	}
}
