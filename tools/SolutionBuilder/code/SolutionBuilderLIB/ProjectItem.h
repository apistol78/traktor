#ifndef ProjectItem_H
#define ProjectItem_H

#include <Core/RefArray.h>
#include <Core/Serialization/ISerializable.h>

class ProjectItem : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	void addItem(ProjectItem* item);

	void removeItem(ProjectItem* item);

	const traktor::RefArray< ProjectItem >& getItems() const;

	virtual void serialize(traktor::ISerializer& s);

private:
	traktor::RefArray< ProjectItem > m_items;
};

#endif	// ProjectItem_H
