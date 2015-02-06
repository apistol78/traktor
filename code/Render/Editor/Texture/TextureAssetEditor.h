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

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual ui::Size getPreferredSize() const;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< TextureAsset > m_asset;
	Ref< ui::Image > m_imageTexture;
	Ref< ui::custom::AutoPropertyList > m_propertyList;

	void updatePreview();

	void eventPropertyCommand(ui::custom::PropertyCommandEvent* event);

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;
};

	}
}

#endif	// traktor_render_TextureAssetEditor_H
