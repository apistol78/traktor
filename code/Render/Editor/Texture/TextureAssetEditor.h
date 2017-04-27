/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TextureAssetEditor_H
#define traktor_render_TextureAssetEditor_H

#include "Editor/IObjectEditor.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Image;

		namespace custom
		{

class PropertyCommandEvent;

		}
	}

	namespace render
	{

class TextureAsset;

class TextureAssetEditor
:	public editor::IObjectEditor
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	TextureAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void apply() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferredSize() const T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< TextureAsset > m_asset;
	Ref< ui::Image > m_imageTextureWithAlpha;
	Ref< ui::Image > m_imageTextureNoAlpha;
	Ref< ui::Image > m_imageTextureAlphaOnly;
	Ref< ui::custom::AutoPropertyList > m_propertyList;

	void updatePreview();

	void eventPropertyCommand(ui::custom::PropertyCommandEvent* event);

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_TextureAssetEditor_H
