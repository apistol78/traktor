/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Editor/IReferenceResolver.h"

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

/*! Resolves references to entities of the scene being edited.
 * \ingroup Scene
 */
class T_DLLCLASS EntityReferenceResolver : public editor::IReferenceResolver
{
	T_RTTI_CLASS;

public:
	explicit EntityReferenceResolver(SceneEditorContext* context);

	virtual bool initialize(editor::IEditor* editor) override final;

	virtual TypeInfoSet getReferenceTypes() const override final;

	virtual bool resolve(const Guid& id, std::wstring& outName) const override final;

	virtual bool browse(ui::Widget* parent, const TypeInfo& referenceType, const Guid& current, Guid& outId) const override final;

	virtual Ref< db::Instance > getOwnerInstance(const Guid& id) const override final;

private:
	SceneEditorContext* m_context;
	SmallMap< Guid, std::wstring > m_paths;
};

}
