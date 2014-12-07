#ifndef traktor_db_Types_H
#define traktor_db_Types_H

namespace traktor
{
	namespace db
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

//@}

	}
}

#endif	// traktor_db_Types_H
