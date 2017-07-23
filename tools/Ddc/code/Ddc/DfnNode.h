/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_DfnNode_H
#define ddc_DfnNode_H

#include <Core/Serialization/ISerializable.h>

namespace ddc
{

class DfnNode : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(traktor::ISerializer& s);

	void setNext(DfnNode* next) { m_next = next; }

	DfnNode* getNext() const { return m_next; }

private:
	traktor::Ref< DfnNode > m_next;
};

}

#endif	// ddc_DfnNode_H
