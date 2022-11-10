#pragma once

namespace traktor::db
{

/*! \ingroup Database */
//@{

enum CreateInstanceFlags
{
	CifDefault = 0,			//!< Default flag.
	CifReplaceExisting = 1,	//!< Replace existing instance.
	CifKeepExistingGuid = 2	//!< Keep existing instance's guid.
};

enum CommitInstanceFlags
{
	CfDefault = 0,			//!< Default commit flags; transaction will be closed after commit.
	CfKeepCheckedOut = 1	//!< Keep transaction opened after commit.
};

enum InstanceFlags
{
	IfNormal = 0,
	IfReadOnly = 1 << 0,
	IfModified = 1 << 1
};

//@}

}
