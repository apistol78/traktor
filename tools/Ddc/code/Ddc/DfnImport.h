/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_DfnImport_H
#define ddc_DfnImport_H

#include <Core/Ref.h>
#include <Core/Io/Path.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnImport : public DfnNode
{
	T_RTTI_CLASS;

public:
	DfnImport();

	DfnImport(const std::wstring& module);

	virtual bool serialize(traktor::ISerializer& s);

	const std::wstring& getModule() const { return m_module; }

	void setPath(const traktor::Path& path) { m_path = path; }

	const traktor::Path& getPath() const { return m_path; }

	void setDefinition(const DfnNode* definition) { m_definition = definition; }

	const DfnNode* getDefinition() const { return m_definition; }

private:
	std::wstring m_module;
	traktor::Path m_path;
	traktor::Ref< const DfnNode > m_definition;
};

}

#endif	// ddc_DfnImport_H
