#ifndef traktor_hf_HeightfieldAssetEditor_H
#define traktor_hf_HeightfieldAssetEditor_H

#include "Core/Ref.h"
#include "Editor/IObjectEditor.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{
	
class Edit;
class Image;

		namespace custom
		{
		
class ToolBarButtonClickEvent;

		}
	}

	namespace hf
	{
	
class Heightfield;
class HeightfieldAsset;

class HeightfieldAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	HeightfieldAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void apply() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferredSize() const T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< HeightfieldAsset > m_asset;
	Ref< Heightfield > m_heightfield;
	Ref< ui::Edit > m_editExtent[3];
	Ref< ui::Edit > m_editVista;
	Ref< ui::Edit > m_editSize;
	Ref< ui::Image > m_imagePreview;

	void updatePreviewImage();

	void eventToolBar(ui::custom::ToolBarButtonClickEvent* event);
};

	}
}

#endif	// traktor_hf_HeightfieldAssetEditor_H

