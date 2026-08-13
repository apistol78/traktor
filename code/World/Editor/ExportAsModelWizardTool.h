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
#include "Core/Ref.h"
#include "Editor/IWizardTool.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityReplicator;

class T_DLLCLASS ExportAsModelWizardTool : public editor::IWizardTool
{
	T_RTTI_CLASS;

public:
	virtual bool create(const PropertyGroup* settings) override final;

	virtual std::wstring getDescription() const override final;

	virtual const TypeInfoSet getSupportedTypes() const override final;

	virtual uint32_t getFlags() const override final;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance) override final;

private:
	SmallMap< const TypeInfo*, Ref< const IEntityReplicator > > m_entityReplicators;
};

}
