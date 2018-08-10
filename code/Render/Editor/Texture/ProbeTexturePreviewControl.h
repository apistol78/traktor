/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include "Resource/Proxy.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace resource
	{
	
class IResourceManager;

	}

	namespace render
	{

class CubeMap;
class ICubeTexture;
class IndexBuffer;
class IRenderSystem;
class IRenderView;
class Shader;
class VertexBuffer;

/*! \brief
 * \ingroup Render
 */
class ProbeTexturePreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	ProbeTexturePreviewControl(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferedSize() const T_OVERRIDE T_FINAL;

	void setCubeMap(const CubeMap* cubeMap, float filterAngle);

private:
	editor::IEditor* m_editor;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ICubeTexture > m_texture;

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

	}
}
