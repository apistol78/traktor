/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Constant_H
#define traktor_render_Constant_H

#include "Render/Editor/Shader/PinType.h"

namespace traktor
{
	namespace render
	{

class Constant
{
public:
	Constant();

	Constant(PinType type);

	explicit Constant(float x);

	explicit Constant(float x, float y, float z, float w);

	Constant cast(PinType type) const;

	PinType getType() const;

	int32_t getWidth() const;

	void setVariant(int32_t index);

	void setValue(int32_t index, float value);

	float getValue(int32_t index) const;

	bool isAllConst() const;

	bool isAnyConst() const;

	bool isConst(int32_t index) const;

	bool isAllZero() const;

	bool isZero(int32_t index) const;

	bool isAllOne() const;

	bool isOne(int32_t index) const;

	bool operator == (const Constant& rh) const;

	bool operator != (const Constant& rh) const;

	bool isConstX() const { return isConst(0); }

	bool isConstY() const { return isConst(1); }

	bool isConstZ() const { return isConst(2); }

	bool isConstW() const { return isConst(3); }

	float x() const { return getValue(0); }

	float y() const { return getValue(1); }
	
	float z() const { return getValue(2); }

	float w() const { return getValue(3); }

private:
	PinType m_type;
	bool m_const[4];
	float m_value[4];
};

	}
}

#endif	// traktor_render_Constant_H
