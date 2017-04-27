/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_InstanceClipboardData_H
#define traktor_editor_InstanceClipboardData_H

#include <list>
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
	struct Instance
	{
		std::wstring name;
		Ref< ISerializable > object;
		Guid originalId;
		Guid pasteId;

		void serialize(ISerializer& s);
	};

	InstanceClipboardData();

	void addInstance(const std::wstring& name, ISerializable* object, const Guid& id = Guid());

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const std::list< Instance >& getInstances() const { return m_instances; }

private:
	std::list< Instance > m_instances;
};

	}
}

#endif	// traktor_editor_InstanceClipboardData_H
