#ifndef ProjectItem_H
#define ProjectItem_H

#include <Core/Heap/Ref.h>
#include <Core/Serialization/Serializable.h>

class ProjectItem : public traktor::Serializable
{
	T_RTTI_CLASS(ProjectItem)

public:
	void addItem(ProjectItem* item);

	void removeItem(ProjectItem* item);

	const traktor::RefList< ProjectItem >& getItems() const;

	virtual bool serialize(traktor::Serializer& s);

private:
	traktor::RefList< ProjectItem > m_items;
};

#endif	// ProjectItem_H
