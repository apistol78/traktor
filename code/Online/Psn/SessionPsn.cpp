#include <np.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Psn/SaveGamePsn.h"
#include "Online/Psn/SessionPsn.h"
#include "Online/Psn/UserPsn.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const uint32_t c_maxDirCount = 32;
const uint32_t c_maxFileCount = 32;
const char c_signature[] = "NPWR01625";

const SceNpCommunicationPassphrase c_passphrase =
{
        {
                0x0a,0x61,0x7e,0xdc,0x62,0x28,0x2e,0xde,
                0x6d,0x1f,0x82,0x78,0xc1,0x97,0xbc,0x57,
                0xab,0xc5,0xe5,0x18,0xf4,0xfa,0x5d,0x0b,
                0xc4,0x01,0xe4,0x95,0x33,0xca,0x9d,0xf5,
                0x1e,0x17,0xc6,0x77,0xc6,0xe8,0x51,0xd1,
                0x27,0xfe,0x4d,0xc7,0x79,0x5f,0x5a,0x33,
                0x89,0x65,0x16,0xdc,0x92,0x2c,0x95,0xfa,
                0xb9,0x56,0xb1,0x50,0xf3,0x73,0x74,0xd5,
                0x36,0xd3,0x8a,0x35,0xf4,0xbd,0xab,0xba,
                0x3e,0x88,0x2d,0x8a,0x4a,0xfe,0x25,0x4f,
                0xaa,0x4c,0x67,0x5f,0x30,0x06,0x0f,0x42,
                0xd6,0x88,0xec,0xc8,0x39,0x60,0x81,0xb2,
                0x87,0x8d,0x1f,0x99,0xd4,0xb5,0x3e,0x40,
                0xf2,0xa6,0x4f,0x46,0xc1,0x28,0xc5,0xf5,
                0x30,0x08,0xc7,0x60,0xba,0xb0,0x30,0x70,
                0xe5,0xdf,0x24,0xe7,0xde,0x9f,0xd0,0x7a
        }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionPsn", SessionPsn, ISession)

SessionPsn::SessionPsn(UserPsn* user)
:	m_user(user)
,	m_connected(false)
,	m_titleContextId(0)
{
}

bool SessionPsn::create()
{
	return true;
}

void SessionPsn::destroy()
{
	if (m_titleContextId > 0)
	{
		sceNpTusDestroyTitleCtx(m_titleContextId);
		m_titleContextId = 0;
	}
}

bool SessionPsn::isConnected() const
{
	return m_connected;
}

Ref< IUser > SessionPsn::getUser()
{
	return m_user;
}

bool SessionPsn::rewardAchievement(const std::wstring& achievementId)
{
	return true;
}

bool SessionPsn::withdrawAchievement(const std::wstring& achievementId)
{
	return false;
}

bool SessionPsn::haveAchievement(const std::wstring& achievementId)
{
	return false;
}

Ref< ILeaderboard > SessionPsn::getLeaderboard(const std::wstring& id)
{
	return 0;
}

bool SessionPsn::setStatValue(const std::wstring& statId, float value)
{
	return false;
}

bool SessionPsn::getStatValue(const std::wstring& statId, float& outValue)
{
	return false;
}

Ref< ISaveGame > SessionPsn::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	uint32_t tmpSize = std::max< uint32_t >(c_maxDirCount * sizeof(CellSaveDataDirList), c_maxFileCount * sizeof(CellSaveDataFileStat));
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	m_saveBuffer.resize(0);

	// Create a serialized image of the attachment.
	DynamicMemoryStream dms(m_saveBuffer, false, true);
	BinarySerializer(&dms).writeObject(attachment);

	// Create save game.
	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	int32_t err = cellSaveDataAutoSave2(
		CELL_SAVEDATA_VERSION_CURRENT,
		"NPEB00401-PUZZLED-0001",
		CELL_SAVEDATA_ERRDIALOG_ALWAYS,
		&buf,
		&saveStatCallback,
		&saveFileCallback,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*)this
	);
	if (err != CELL_SAVEDATA_RET_OK)
		return 0;

	return new SaveGamePsn(name, attachment);
}

bool SessionPsn::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const
{
	uint32_t tmpSize = std::max< uint32_t >(c_maxDirCount * sizeof(CellSaveDataDirList), c_maxFileCount * sizeof(CellSaveDataFileStat));
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	CellSaveDataSetList list;
	std::memset(&list, 0, sizeof(list));
	list.sortType = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
	list.sortOrder = CELL_SAVEDATA_SORTORDER_DESCENT;
	list.dirNamePrefix = "NPEB00401-PUZZLED-";

	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	int32_t err = cellSaveDataListAutoLoad(
		CELL_SAVEDATA_VERSION_CURRENT,
		CELL_SAVEDATA_ERRDIALOG_NOREPEAT,
		&list,
		&buf,
		&loadFixedCallback,
		&loadStatCallback,
		&loadFileCallback,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*)this
	);
	if (err != CELL_SAVEDATA_RET_OK)
		return false;

	outSaveGames = m_saveGames;
	m_saveGames.resize(0);

	return true;
}

void SessionPsn::loadFixedCallback(CellSaveDataCBResult* cbResult, CellSaveDataListGet* get, CellSaveDataFixedSet* set)
{
	log::debug << L"Save data; load fixed callback" << Endl;

	set->dirName = "NPEB00401-PUZZLED-0001";
	set->newIcon = 0;
	set->option = CELL_SAVEDATA_OPTION_NONE;

	int32_t i = 0;
	for (; i < get->dirListNum; ++i)
	{
		if (strcmp(get->dirList[i].dirName, set->dirName) == 0)
			break;
	}

	if (i < get->dirListNum)
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
	else
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
}

void SessionPsn::loadStatCallback(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	SessionPsn* this_ = static_cast< SessionPsn* >(cbResult->userdata);
	T_ASSERT (this_);

	log::debug << L"Save data; load stat callback" << Endl;

	if (get->isNewData)
	{
		log::error << L"Save data; no attachment in save game" << Endl;
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
		return;
	}

	if (get->fileNum > get->fileListNum)
	{
		log::error << L"Save data; file number overflow" << Endl;
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		return;
	}

	int32_t i = 0;
	for (; i < get->fileListNum; ++i)
	{
		if (strcmp(get->fileList[i].fileName, "ATT.BIN") == 0)
		{
			this_->m_saveBuffer.resize(get->fileList[i].st_size);
			break;
		}
	}
	if (i >= get->fileListNum)
	{
		log::error << L"Save data; no attachment in save game (2)" << Endl;
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		return;
	}

	this_->m_saveBufferPending = true;

	set->reCreateMode = CELL_SAVEDATA_RECREATE_NO_NOBROKEN;
	set->setParam = 0;
	set->indicator = 0;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void SessionPsn::loadFileCallback(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	SessionPsn* this_ = static_cast< SessionPsn* >(cbResult->userdata);
	T_ASSERT (this_);

	log::debug << L"Save data; load file callback" << Endl;

	cbResult->progressBarInc = 50;

	if (this_->m_saveBufferPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_READ;
		set->fileType = CELL_SAVEDATA_FILETYPE_NORMALFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = this_->m_saveBuffer.size();
		set->fileBufSize = this_->m_saveBuffer.size();
		set->fileBuf = &this_->m_saveBuffer[0];
		set->reserved = 0;

		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		this_->m_saveBufferPending = false;
	}
	else
	{
		// De-serialize attachment.
		DynamicMemoryStream dms(this_->m_saveBuffer, true, false);
		Ref< ISerializable > attachment = BinarySerializer(&dms).readObject();
		if (attachment)
		{
			this_->m_saveGames.push_back(new SaveGamePsn(L"", attachment));
			cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		}
		else
		{
			log::error << L"Save data; corrupt attachment" << Endl;
			cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		}
	}
}

void SessionPsn::saveStatCallback(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	SessionPsn* this_ = static_cast< SessionPsn* >(cbResult->userdata);
	T_ASSERT (this_);

	log::debug << L"Save data; save stat callback" << Endl;

	set->setParam = &get->getParam;
	set->reCreateMode = CELL_SAVEDATA_RECREATE_YES;
	set->indicator = 0;

	if (get->isNewData)
	{
		strncpy(set->setParam->title, "Puzzle Dimension", CELL_SAVEDATA_SYSP_TITLE_SIZE);
		strncpy(set->setParam->subTitle, "", CELL_SAVEDATA_SYSP_SUBTITLE_SIZE);
		strncpy(set->setParam->detail, "", CELL_SAVEDATA_SYSP_DETAIL_SIZE);
		strncpy(set->setParam->listParam, "", CELL_SAVEDATA_SYSP_LPARAM_SIZE);

		set->setParam->attribute = CELL_SAVEDATA_ATTR_NORMAL;

		std::memset(set->setParam->reserved, 0, sizeof(set->setParam->reserved));
		std::memset(set->setParam->reserved2, 0, sizeof(set->setParam->reserved2));	
	}

	this_->m_saveBufferPending = true;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void SessionPsn::saveFileCallback(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	SessionPsn* this_ = static_cast< SessionPsn* >(cbResult->userdata);
	T_ASSERT (this_);

	log::debug << L"Save data; save file callback" << Endl;

	if (this_->m_saveBufferPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
		set->fileType = CELL_SAVEDATA_FILETYPE_NORMALFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = this_->m_saveBuffer.size();
		set->fileBufSize = this_->m_saveBuffer.size();
		set->fileBuf = &this_->m_saveBuffer[0];
		set->reserved = 0;

		cbResult->progressBarInc = 50;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		this_->m_saveBufferPending = false;
	}
	else
	{
		cbResult->progressBarInc = 50;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	}
}

void SessionPsn::update()
{
	int32_t state;
	if (sceNpManagerGetStatus(&state) == 0)
	{
		if (state == SCE_NP_MANAGER_STATUS_ONLINE)
		{
			if (!m_connected)
			{
				log::debug << L"NP CONNECTED" << Endl;

				SceNpId selfNpId;
				int32_t err;

				err = sceNpManagerGetNpId(&selfNpId);
				if (err < 0)
				{
					log::error << L"Unable to create session manager; sceNpManagerGetNpId failed" << Endl;
					//return false;
				}

				SceNpCommunicationId communicationId;
				std::memset(&communicationId, 0, sizeof(communicationId));
				strcpy(communicationId.data, c_signature);

				m_titleContextId = sceNpTusCreateTitleCtx(&communicationId, &c_passphrase, &selfNpId);
				if (m_titleContextId < 0)
				{
					log::error << L"Unable to create session; sceNpTusCreateTitleCtx failed" << Endl;
					//return false;
				}
			}

			m_connected = true;
		}
		else
		{
			if (m_connected)
			{
				log::debug << L"NP DISCONNECTED (1)" << Endl;
			}

			m_connected = false;
		}
	}
	else
	{
		if (m_connected)
		{
			log::debug << L"NP DISCONNECTED (2)" << Endl;
		}

		m_connected = false;
	}
}

	}
}
