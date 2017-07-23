/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Filter_H
#define Filter_H

#include "ProjectItem.h"

class Filter : public ProjectItem
{
	T_RTTI_CLASS;

public:
	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_name;
};

#endif	// Filter_H
