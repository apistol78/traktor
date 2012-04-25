#include <cmath>
#include "Animation/Editor/SkeletonEditorPage.h"
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Database/Database.h"
#include "Resource/ResourceManager.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/TreeView.h"
#include "Ui/HierarchicalState.h"
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
#include "Render/PrimitiveRenderer.h"
#include "Render/Resource/TextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
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

SkeletonEditorPage::SkeletonEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_selectedBone(-1)
,	m_cameraHead(0.0f)
,	m_cameraY(0.0f)
,	m_cameraZ(0.0f)
,	m_cameraMoveScaleY(1.0f)
,	m_cameraMoveScaleZ(1.0f)
,	m_cameraBoneScale(1.0f)
{
}

bool SkeletonEditorPage::create(ui::Container* parent)
{
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	m_skeleton = m_document->getObject< Skeleton >(0);
	if (!m_skeleton)
		return false;

	m_renderWidget = new ui::Widget();
	m_renderWidget->create(parent, ui::WsNone);
	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventMouseDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventMouseUp));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventMouseMove));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventPaint));

	m_boneMenu = new ui::PopupMenu();
	m_boneMenu->create();
	m_boneMenu->add(new ui::MenuItem(ui::Command(L"Skeleton.Editor.AddBone"), i18n::Text(L"SKELETON_EDITOR_ADD_BONE")));
	m_boneMenu->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SKELETON_EDITOR_DELETE_BONE")));

	// Create skeleton panel.
	m_skeletonPanel = new ui::Container();
	m_skeletonPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));
	m_skeletonPanel->setText(i18n::Text(L"SKELETON_EDITOR_SKELETON"));

	m_treeSkeleton = new ui::TreeView();
	m_treeSkeleton->create(m_skeletonPanel, ui::TreeView::WsDefault & ~ui::WsClientBorder);
	m_treeSkeleton->addImage(ui::Bitmap::load(c_ResourceBones, sizeof(c_ResourceBones), L"png"), 2);
	m_treeSkeleton->addButtonDownEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventTreeButtonDown));
	m_treeSkeleton->addSelectEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventTreeSelect));
	m_treeSkeleton->addEditedEventHandler(ui::createMethodHandler(this, &SkeletonEditorPage::eventTreeEdited));

	m_site->createAdditionalPanel(m_skeletonPanel, 250, false);

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = m_editor->getSettings()->getProperty< PropertyInteger >(L"Editor.MultiSample", 4);
	desc.waitVBlank = false;
	desc.nativeWindowHandle = m_renderWidget->getIWidget()->getSystemHandle();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager();
	m_resourceManager->addFactory(
		new render::TextureFactory(database, renderSystem, 0)
	);
	m_resourceManager->addFactory(
		new render::ShaderFactory(database, renderSystem)
	);

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, renderSystem))
		return false;

	m_site->setPropertyObject(m_skeleton);

	Aabb3 boundingBox = calculateBoundingBox(m_skeleton);

	int majorAxis = majorAxis3(boundingBox.getExtent());
	float majorExtent = boundingBox.getExtent()[majorAxis];

	m_cameraHead = 0.0f;
	m_cameraY = 0.0f;
	m_cameraZ = majorExtent * 2.0f;

	m_cameraMoveScaleY = majorExtent / 200.0f;
	m_cameraMoveScaleZ = majorExtent / 100.0f;
	m_cameraBoneScale = majorExtent / 100.0f;

	createSkeletonTreeNodes();

	m_renderWidget->startTimer(30);
	return true;
}

void SkeletonEditorPage::destroy()
{
	m_site->destroyAdditionalPanel(m_skeletonPanel);
	
	m_renderView->close();

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

bool SkeletonEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool SkeletonEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.PropertiesChanged")
	{
		createSkeletonTreeNodes();
		m_renderWidget->update();
	}
	else if (command == L"Editor.Undo")
	{
		if (!m_document->undo())
			return false;

		m_skeleton = m_document->getObject< Skeleton >(0);
		T_ASSERT (m_skeleton);

		createSkeletonTreeNodes();

		m_site->setPropertyObject(m_skeleton);
		m_renderWidget->update();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_document->redo())
			return false;

		m_skeleton = m_document->getObject< Skeleton >(0);
		T_ASSERT (m_skeleton);

		createSkeletonTreeNodes();

		m_site->setPropertyObject(m_skeleton);
		m_renderWidget->update();
	}
	else if (command == L"Editor.Delete")
	{
		Ref< ui::TreeViewItem > selectedTreeItem = m_treeSkeleton->getSelectedItem();
		if (!selectedTreeItem)
			return true;

		m_document->push();

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

		m_document->push();

		Ref< Bone > parentBone = selectedTreeItem->getData< Bone >(L"BONE");

		Ref< Bone > bone = new Bone();
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
		m_resourceManager->reload(eventId);
}

void SkeletonEditorPage::createSkeletonTreeNodes()
{
	Ref< ui::HierarchicalState > treeSkeletonState = m_treeSkeleton->captureState();

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

	m_document->push();

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

				if ((mouseEvent->getKeyState() & ui::KsMenu) == 0)
				{
					orientation *= Quaternion(Vector4(0.0f, 1.0f, 0.0f, 0.0f), mouseDelta.x);
					orientation *= Quaternion(Vector4(1.0f, 0.0f, 0.0f, 0.0f), mouseDelta.y);
				}
				else
					orientation *= Quaternion(Vector4(0.0f, 0.0f, 1.0f, 0.0f), mouseDelta.x);

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

	m_renderView->reset(sz.cx, sz.cy);
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

	if (!m_renderView->begin(render::EtCyclop))
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
				Color4ub(100, 100, 100)
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				Color4ub(100, 100, 100)
			);
		}

		m_primitiveRenderer->drawArrowHead(
			Vector4(11.0f, 0.0f, 0.0f, 1.0f),
			Vector4(13.0f, 0.0f, 0.0f, 1.0f),
			0.8f,
			Color4ub(255, 64, 64)
		);
		m_primitiveRenderer->drawArrowHead(
			Vector4(0.0f, 0.0f, 11.0f, 1.0f),
			Vector4(0.0f, 0.0f, 13.0f, 1.0f),
			0.8f,
			Color4ub(64, 64, 255)
		);

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

			Color4ub color = (m_selectedBone == i) ? Color4ub(255, 128, 255, 128) : Color4ub(255, 255, 0, 128);

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

			m_primitiveRenderer->drawLine(start, end, Color4ub(255, 255, 128, 128));
			m_primitiveRenderer->drawLine(start, start + a * Scalar(2.0f), Color4ub(255, 0, 0, 128));
			m_primitiveRenderer->drawLine(start, start + b * Scalar(2.0f), Color4ub(0, 255, 0, 128));

			if (bone->getEnableLimits())
			{
				m_primitiveRenderer->drawCone(
					boneTransforms[i].toMatrix44(),
					bone->getConeLimit().x,
					bone->getConeLimit().y,
					radius,
					Color4ub(255, 255, 255, 64),
					Color4ub(0, 0, 0, 32)
				);

				m_primitiveRenderer->drawProtractor(
					start,
					boneTransforms[i].axisX(),
					boneTransforms[i].axisY(),
					-bone->getTwistLimit(),
					bone->getTwistLimit(),
					deg2rad(8.0f),
					radius,
					Color4ub(255, 255, 255, 64),
					Color4ub(0, 0, 0, 32)
				);
			}
		}

		m_primitiveRenderer->end();
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
