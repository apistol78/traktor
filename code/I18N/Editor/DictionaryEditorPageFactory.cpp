/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "I18N/Dictionary.h"
#include "I18N/Editor/DictionaryEditorPage.h"
#include "I18N/Editor/DictionaryEditorPageFactory.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.i18n.DictionaryEditorPageFactory", 0, DictionaryEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet DictionaryEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< Dictionary >();
}

bool DictionaryEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > DictionaryEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new DictionaryEditorPage(editor, site, document);
}

void DictionaryEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > DictionaryEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
