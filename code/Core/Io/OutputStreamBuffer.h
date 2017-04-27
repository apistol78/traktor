/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_OutputStreamBuffer_H
#define traktor_OutputStreamBuffer_H

#include "Core/Io/IOutputStreamBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Buffer used by output streams.
 * \ingroup Core
 */
class T_DLLCLASS OutputStreamBuffer : public IOutputStreamBuffer
{
	T_RTTI_CLASS;

public:
	OutputStreamBuffer();

	virtual int32_t getIndent() const T_OVERRIDE T_FINAL;

	virtual void setIndent(int32_t indent) T_OVERRIDE T_FINAL;

	virtual int32_t getDecimals() const T_OVERRIDE T_FINAL;

	virtual void setDecimals(int32_t decimals) T_OVERRIDE T_FINAL;

	virtual bool getPushIndent() const T_OVERRIDE T_FINAL;

	virtual void setPushIndent(bool pushIndent) T_OVERRIDE T_FINAL;

private:
	int32_t m_indent;
	int32_t m_decimals;
	bool m_pushIndent;
};

}

#endif	// traktor_OutputStreamBuffer_H
