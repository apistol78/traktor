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
	CifDefault = 0,			//< Default flag.
	CifReplaceExisting = 1,	//< Replace existing instance.
	CifKeepExistingGuid = 2	//< Keep existing instance's guid.	
};

enum CheckoutInstanceFlags
{
	CfReadOnly = 0,
	CfExclusive = 1
};

enum CommitInstanceFlags
{
	CfDefault = 0,
	CfKeepCheckedOut = 1
};

enum ProviderEvent
{
	PeInvalid = 0,
	PeCommited = 2,
	PeReverted = 3,
	PeRenamed = 4,
	PeRemoved = 5
};

//@}

	}
}

#endif	// traktor_db_Types_H
