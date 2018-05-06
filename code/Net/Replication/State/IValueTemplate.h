/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_IValueTemplate_H
#define traktor_net_IValueTemplate_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BitReader;
class BitWriter;

	namespace net
	{

class IValue;

class T_DLLCLASS IValueTemplate : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getValueType() const = 0;

	virtual uint32_t getMaxPackedDataSize() const = 0;

	virtual void pack(BitWriter& writer, const IValue* V) const = 0;

	virtual Ref< const IValue > unpack(BitReader& reader) const = 0;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const = 0;

	virtual bool threshold(const IValue* Vn1, const IValue* V) const = 0;
};

	}
}

#endif	// traktor_net_IValueTemplate_H
