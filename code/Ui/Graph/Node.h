/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

class Guid;

}

namespace traktor::ui
{

class GraphCanvas;
class GraphControl;
class IBitmap;
class INodeShape;
class Pin;

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

	void setPosition(const UnitPoint& position);

	UnitPoint getPosition() const;

	void setSelected(bool selected);

	bool isSelected() const;

	void setShape(const INodeShape* shape);

	const INodeShape* getShape() const;

	Pin* createInputPin(const std::wstring& name, const Guid& id, bool mandatory, bool bold);

	bool removeInputPin(Pin* pin);

	const RefArray< Pin >& getInputPins() const;

	Pin* getInputPin(uint32_t index) const;

	Pin* findInputPin(const std::wstring& name) const;

	Pin* findInputPin(const Guid& id) const;

	Pin* createOutputPin(const std::wstring& name, const Guid& id);

	bool removeOutputPin(Pin* pin);

	const RefArray< Pin >& getOutputPins() const;

	Pin* getOutputPin(uint32_t index) const;

	Pin* findOutputPin(const std::wstring& name) const;

	Pin* findOutputPin(const Guid& id) const;

	bool hit(const UnitPoint& p) const;

	UnitPoint getPinPosition(const Pin* pin) const;

	Pin* getPinAt(const UnitPoint& p) const;

	void paint(GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const;

	UnitRect calculateRect() const;

private:
	friend class GraphControl;

	GraphControl* m_owner = nullptr;
	std::wstring m_title;
	std::wstring m_info;
	std::wstring m_comment;
	Ref< IBitmap > m_image;
	int32_t m_state;
	UnitPoint m_position;
	UnitSize m_size;
	bool m_selected;
	RefArray< Pin > m_inputPins;
	RefArray< Pin > m_outputPins;
	Ref< const INodeShape > m_shape;

	explicit Node(GraphControl* owner, const std::wstring& title, const std::wstring& info, const UnitPoint& position, const INodeShape* shape);

	void updateSize();
};

}
