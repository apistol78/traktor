#ifndef Filter_H
#define Filter_H

#include "ProjectItem.h"

class Filter : public ProjectItem
{
	T_RTTI_CLASS(Filter)

public:
	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::wstring m_name;
};

#endif	// Filter_H
