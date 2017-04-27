/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_FloatTemplate_H
#define traktor_net_FloatTemplate_H

#include <limits>
#include "Net/Replication/State/IValueTemplate.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

enum FloatTemplatePrecision
{
	Ftp32,
	Ftp16,
	Ftp8,
	Ftp4
};

class T_DLLCLASS FloatTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	FloatTemplate(const wchar_t* const tag, float threshold = std::numeric_limits< float >::max());

	explicit FloatTemplate(const wchar_t* const tag, float threshold, float min, float max, FloatTemplatePrecision precision, bool cyclic);

	virtual const TypeInfo& getValueType() const T_OVERRIDE T_FINAL;

	virtual uint32_t getMaxPackedDataSize() const T_OVERRIDE T_FINAL;

	virtual void pack(BitWriter& writer, const IValue* V) const T_OVERRIDE T_FINAL;

	virtual Ref< const IValue > unpack(BitReader& reader) const T_OVERRIDE T_FINAL;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const T_OVERRIDE T_FINAL;

	virtual bool threshold(const IValue* Vn1, const IValue* V) const T_OVERRIDE T_FINAL;

private:
	const wchar_t* const m_tag;
	float m_threshold;
	float m_min;
	float m_max;
	FloatTemplatePrecision m_precision;
	bool m_cyclic;
};

	}
}

#endif	// traktor_net_FloatTemplate_H
