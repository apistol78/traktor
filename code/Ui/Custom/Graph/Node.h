/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_Node_H
#define traktor_ui_custom_Node_H

#include <map>
#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Color4ub.h"
#include "Ui/Associative.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;
class Canvas;
class Size;

		namespace custom
		{

class PaintSettings;
class NodeShape;
class Pin;

/*! \brief Graph node.
 * \ingroup UIC
 */
class T_DLLCLASS Node
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	Node(const std::wstring& title, const std::wstring& info, const Point& position, NodeShape* shape);

	void setTitle(const std::wstring& title);

	const std::wstring& getTitle() const;

	void setInfo(const std::wstring& info);

	const std::wstring& getInfo() const;

	void setComment(const std::wstring& comment);

	const std::wstring& getComment() const;

	void setImage(Bitmap* image);

	Ref< Bitmap > getImage() const;

	void setState(int32_t state);

	int32_t getState() const;

	void setPosition(const Point& position);

	Point getPosition() const;

	void setSelected(bool selected);

	bool isSelected() const;

	Ref< Pin > createInputPin(const std::wstring& name, bool mandatory);

	const RefArray< Pin >& getInputPins() const;

	Ref< Pin > findInputPin(const std::wstring& name) const;

	Ref< Pin > createOutputPin(const std::wstring& name);

	const RefArray< Pin >& getOutputPins() const;

	Ref< Pin > findOutputPin(const std::wstring& name) const;

	bool hit(const Point& p) const;

	Point getPinPosition(const Pin* pin) const;

	Ref< Pin > getPinAt(const Point& p) const;

	void paint(PaintSettings* settings, Canvas* canvas, const Size& offset) const;

	Rect calculateRect() const;

private:
	std::wstring m_title;
	std::wstring m_info;
	std::wstring m_comment;
	Ref< Bitmap > m_image;
	int32_t m_state;
	Point m_position;
	Size m_size;
	bool m_selected;
	RefArray< Pin > m_inputPins;
	RefArray< Pin > m_outputPins;
	Ref< NodeShape > m_shape;

	void updateSize();
};

		}
	}
}

#endif	// traktor_ui_custom_Node_H
