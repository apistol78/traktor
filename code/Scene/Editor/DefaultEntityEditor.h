/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4ub.h"
#include "Core/Math/Transform.h"
#include "Scene/Editor/IEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class SceneEditorContext;

class T_DLLCLASS DefaultEntityEditor : public IEntityEditor
{
	T_RTTI_CLASS;

public:
	explicit DefaultEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter);

	virtual bool isPickable() const override;

	virtual bool isGroup() const override;

	virtual bool isChildrenPrivate() const override;

	virtual bool addChildEntity(EntityAdapter* insertAfterEntityAdapter, EntityAdapter* childEntityAdapter) const override;

	virtual bool removeChildEntity(EntityAdapter* childEntityAdapter) const override;

	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const override;

	virtual bool queryFrustum(const Frustum& worldFrustum) const override;

	virtual void entitySelected(bool selected) override;

	virtual bool handleCommand(const ui::Command& command) override;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer, const ui::Size& clientSize) const override;

	virtual bool getStatusText(std::wstring& outStatusText) const override;

protected:
	SceneEditorContext* getContext() { return m_context; }

	EntityAdapter* getEntityAdapter() { return m_entityAdapter; }

	const SceneEditorContext* getContext() const { return m_context; }

	const EntityAdapter* getEntityAdapter() const { return m_entityAdapter; }

	bool inModify() const { return m_inModify; }

private:
	SceneEditorContext* m_context;
	EntityAdapter* m_entityAdapter;
	bool m_inModify;
	Transform m_modifyTransform;
	Color4ub m_colorBoundingBox;
	Color4ub m_colorBoundingBoxSel;
	Color4ub m_colorBoundingBoxFaceSel;
	Color4ub m_colorSnap;

	void updateSettings();
};

}
