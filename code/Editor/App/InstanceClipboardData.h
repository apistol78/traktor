#ifndef traktor_editor_InstanceClipboardData_H
#define traktor_editor_InstanceClipboardData_H

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace editor
	{

class InstanceClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	InstanceClipboardData();

	InstanceClipboardData(const std::wstring& name, ISerializable* object);

	const std::wstring& getName() const;

	ISerializable* getObject() const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_name;
	Ref< ISerializable > m_object;
};

	}
}

#endif	// traktor_editor_InstanceClipboardData_H
