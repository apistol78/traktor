#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Math/Color4ub.h"
#include "Ui/Associative.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class GraphCanvas;
class GraphControl;
class IBitmap;
class INodeShape;
class Pin;
class Size;

/*! Graph node.
 * \ingroup UI
 */
class T_DLLCLASS Node
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	void setTitle(const std::wstring& title);

	const std::wstring& getTitle() const;

	void setInfo(const std::wstring& info);

	const std::wstring& getInfo() const;

	void setComment(const std::wstring& comment);

	const std::wstring& getComment() const;

	void setImage(IBitmap* image);

	IBitmap* getImage() const;

	void setState(int32_t state);

	int32_t getState() const;

	void setPosition(const Point& position);

	Point getPosition() const;

	void setSelected(bool selected);

	bool isSelected() const;

	void setShape(const INodeShape* shape);

	const INodeShape* getShape() const;

	Pin* createInputPin(const std::wstring& name, bool mandatory);

	Pin* createInputPin(const std::wstring& name, const std::wstring& label, bool mandatory);

	const RefArray< Pin >& getInputPins() const;

	Pin* getInputPin(uint32_t index) const;

	Pin* findInputPin(const std::wstring& name) const;

	Pin* createOutputPin(const std::wstring& name);

	Pin* createOutputPin(const std::wstring& name, const std::wstring& label);

	const RefArray< Pin >& getOutputPins() const;

	Pin* getOutputPin(uint32_t index) const;

	Pin* findOutputPin(const std::wstring& name) const;

	bool hit(const Point& p) const;

	Point getPinPosition(const Pin* pin) const;

	Pin* getPinAt(const Point& p) const;

	void paint(GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const;

	Rect calculateRect() const;

private:
	friend class GraphControl;

	GraphControl* m_owner;
	std::wstring m_title;
	std::wstring m_info;
	std::wstring m_comment;
	Ref< IBitmap > m_image;
	int32_t m_state;
	Point m_position;
	Size m_size;
	bool m_selected;
	RefArray< Pin > m_inputPins;
	RefArray< Pin > m_outputPins;
	Ref< const INodeShape > m_shape;

	explicit Node(GraphControl* owner, const std::wstring& title, const std::wstring& info, const Point& position, const INodeShape* shape);

	void updateSize();
};

	}
}

