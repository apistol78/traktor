#include <limits>
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/Reader.h>
#include <Core/Log/Log.h>
#include <Core/Misc/String.h>
#include <Core/Misc/Split.h>
#include <Ui/Application.h>
#include <Ui/MethodHandler.h>
#include <Ui/TableLayout.h>
#include <Ui/Form.h>
#include <Ui/Button.h>
#include <Ui/CheckBox.h>
#include <Ui/DropDown.h>
#include <Ui/FileDialog.h>
#include <Ui/ListBox.h>
#include <Ui/ListView.h>
#include <Ui/MenuBar.h>
#include <Ui/MenuItem.h>
#include <Ui/MessageBox.h>
#include <Ui/ScrollBar.h>
#include <Ui/Static.h>
#include <Ui/ToolForm.h>
#include <Ui/TreeView.h>
#include <Ui/TreeViewItem.h>
#include <Ui/Container.h>
#include <Ui/Events/CommandEvent.h>
#include <Ui/Custom/MiniButton.h>
#include <Ui/Custom/Splitter.h>
#include <Ui/Custom/ToolBar/ToolBar.h>
#include <Ui/Custom/ToolBar/ToolBarButton.h>
#include <Ui/Custom/GridView/GridView.h>
#include <Ui/Custom/GridView/GridColumn.h>
#include <Ui/Custom/GridView/GridRow.h>
#include <Ui/Custom/GridView/GridItem.h>
#include <Ui/Custom/Sequencer/Sequence.h>
#include <Ui/Custom/Sequencer/SequenceGroup.h>
#include <Ui/Custom/Sequencer/SequencerControl.h>
#include <Ui/Custom/Sequencer/Tick.h>
#include <Ui/Events/FileDropEvent.h>

#if defined(_WIN32)
#	include <Ui/Win32/EventLoopWin32.h>
#	include <Ui/Win32/WidgetFactoryWin32.h>
typedef traktor::ui::EventLoopWin32 EventLoopImpl;
typedef traktor::ui::WidgetFactoryWin32 WidgetFactoryImpl;
#elif defined(__APPLE__)
#	include <Ui/Cocoa/EventLoopCocoa.h>
#	include <Ui/Cocoa/WidgetFactoryCocoa.h>
typedef traktor::ui::EventLoopCocoa EventLoopImpl;
typedef traktor::ui::WidgetFactoryCocoa WidgetFactoryImpl;
#elif defined(__GNUC__)
#	include <Ui/Gtk/EventLoopGtk.h>
#	include <Ui/Gtk/WidgetFactoryGtk.h>
typedef traktor::ui::EventLoopGtk EventLoopImpl;
typedef traktor::ui::WidgetFactoryGtk WidgetFactoryImpl;
#else
#	include <Ui/Wx/EventLoopWx.h>
#	include <Ui/Wx/WidgetFactoryWx.h>
typedef traktor::ui::EventLoopWx EventLoopImpl;
typedef traktor::ui::WidgetFactoryWx WidgetFactoryImpl;
#endif

#if defined(MessageBox)
#	undef MessageBox
#endif

using namespace traktor;

class PeerInfo : public Object
{
	T_RTTI_CLASS;

public:
	int32_t time;
	uint8_t handle;
	std::wstring name;
	uint8_t status;
	bool direct;
	uint64_t connectionState;
};

T_IMPLEMENT_RTTI_CLASS(L"PeerInfo", PeerInfo, Object)


class Entry : public Object
{
	T_RTTI_CLASS;

public:
	int32_t time;
	uint8_t direction;
	uint8_t handle;
	int32_t size;
	uint8_t data[1500];
};

T_IMPLEMENT_RTTI_CLASS(L"Entry", Entry, Object)


class IDecoder : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool decode(const uint8_t*& ptr, OutputStream& os) const = 0;
};

T_IMPLEMENT_RTTI_CLASS(L"IDecoder", IDecoder, Object)


class ReliableDecoder : public IDecoder
{
	T_RTTI_CLASS;

public:
	virtual bool decode(const uint8_t*& ptr, OutputStream& os) const
	{
		uint8_t type = *ptr++;
		uint8_t sequence = *ptr++;

		if (type == 0x01)
			os << L"EtUnreliable" << Endl;
		if (type == 0x02)
			os << L"EtReliable" << Endl;
		if (type == 0x03)
			os << L"EtAck" << Endl;

		os << L"Sequence " << int32_t(sequence) << Endl;

		return type != 0x03;
	}
};

T_IMPLEMENT_RTTI_CLASS(L"ReliableDecoder", ReliableDecoder, Object)


class RelayDecoder : public IDecoder
{
	T_RTTI_CLASS;

public:
	virtual bool decode(const uint8_t*& ptr, OutputStream& os) const
	{
		uint8_t flags = *ptr++;
		uint8_t from = *ptr++;
		uint8_t to = *ptr++;

		os << ((flags & 0x80) ? L"Reliable" : L"Unreliable") << Endl;
		os << L"Count " << int32_t(flags & 0x7f) << Endl;
		os << L"From " << int32_t(from) << Endl;
		os << L"To " << int32_t(to) << Endl;

		return true;
	}
};

T_IMPLEMENT_RTTI_CLASS(L"RelayDecoder", RelayDecoder, Object)


class ReplicatorDecoder : public IDecoder
{
	T_RTTI_CLASS;

public:
	virtual bool decode(const uint8_t*& ptr, OutputStream& os) const
	{
		uint8_t type = *ptr++;
		uint32_t time = *(uint32_t*)ptr; ptr += 4;

		if (type == 0xf1)
			os << L"MtIAm" << Endl;
		if (type == 0xf2)
			os << L"MtBye" << Endl;
		if (type == 0xf3)
			os << L"MtPing" << Endl;
		if (type == 0xf4)
			os << L"MtPong" << Endl;
		if (type == 0x11)
			os << L"MtFullState" << Endl;
		if (type == 0x12)
			os << L"MtDeltaState" << Endl;
		if (type == 0x20)
			os << L"MtEvent0" << Endl;
		if (type == 0x21)
			os << L"MtEvent1" << Endl;
		if (type == 0x22)
			os << L"MtEvent2" << Endl;
		if (type == 0x23)
			os << L"MtEvent3" << Endl;
		if (type == 0x24)
			os << L"MtEvent4" << Endl;

		os << L"Time " << time << Endl;

		os << IncreaseIndent;

		switch (type)
		{
		case 0xf1:
			{
				uint8_t sequence = *ptr++;
				uint32_t id = *(uint32_t*)ptr; ptr += 4;

				os << L"Sequence " << int32_t(sequence) << Endl;
				os << L"Id " << id << Endl;
			}
			break;

		case 0xf4:
			{
				uint32_t time0 = *(uint32_t*)ptr; ptr += 4;
				uint32_t latency = *(uint32_t*)ptr; ptr += 4;

				os << L"Time0 " << time0 << Endl;
				os << L"Latency " << latency << Endl;
			}
			break;
		}

		os << DecreaseIndent;
		return false;
	}
};

T_IMPLEMENT_RTTI_CLASS(L"ReplicatorDecoder", ReplicatorDecoder, Object)


class MainForm : public ui::Form
{
public:
	bool create();

private:
	Ref< ui::custom::SequencerControl > m_sequencer;
	Ref< ui::ListBox > m_listBox;

	void eventClose(ui::Event* event);

	void eventSelect(ui::Event* event);

	void eventFileDrop(ui::Event* event);
};

bool MainForm::create()
{
	if (!ui::Form::create(
		L"Network Debugger",
		800,
		700,
		ui::Form::WsDefault,
		new ui::TableLayout(L"100%", L"100%", 0, 0)
	))
		return false;

	addCloseEventHandler(ui::createMethodHandler(this, &MainForm::eventClose));

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, false, -50, true);

	m_sequencer = new ui::custom::SequencerControl();
	m_sequencer->create(splitter, ui::WsClientBorder | ui::WsDoubleBuffer | ui::WsAcceptFileDrop);
	m_sequencer->setLength(0);
	m_sequencer->addSelectEventHandler(ui::createMethodHandler(this, &MainForm::eventSelect));
	m_sequencer->addFileDropEventHandler(ui::createMethodHandler(this, &MainForm::eventFileDrop));

	m_listBox = new ui::ListBox();
	m_listBox->create(splitter);

	update();
	show();

	return true;
}

void MainForm::eventClose(ui::Event* event)
{
	ui::Application::getInstance()->exit(0);
}

void MainForm::eventSelect(ui::Event* event)
{
	m_listBox->removeAll();

	RefArray< ui::custom::SequenceItem > sequences;
	m_sequencer->getSequenceItems(sequences, ui::custom::SequencerControl::GfDescendants | ui::custom::SequencerControl::GfSelectedOnly);

	if (sequences.empty())
		return;

	Ref< ui::custom::Sequence > sequence = dynamic_type_cast< ui::custom::Sequence* >(sequences[0]);
	if (sequence)
	{
		Ref< ui::custom::Key > key = sequence->getSelectedKey();
		if (key)
		{
			PeerInfo* info = dynamic_type_cast< PeerInfo* >(key->getData(L"INFO"));
			if (info)
			{
				m_listBox->add(L"Time " + toString< int32_t >(info->time));
				m_listBox->add(L"Status " + toString< int32_t >(info->status));
				m_listBox->add(info->direct ? L"Direct" : L"Relayed");

				m_listBox->add(L"Connection state " + toString< uint64_t >(info->connectionState));
				for (uint32_t i = 0; i < 8; ++i)
				{
					StringOutputStream ss;
					ss << L"   " << i << L". ";
					
					if (i != info->handle)
					{
						if ((info->connectionState & (1ULL << i)) != 0)
							ss << L"Direct";
						else
							ss << L"Relay";
					}

					m_listBox->add(ss.str());
				}
			}

			Entry* entry = dynamic_type_cast< Entry* >(key->getData(L"ENTRY"));
			if (entry)
			{
				m_listBox->add(L"Time " + toString< int32_t >(entry->time));
				m_listBox->add(entry->direction == 0 ? L"Receive" : L"Sent");
				m_listBox->add(toString< int32_t >(entry->size) + L" byte(s)");
				m_listBox->add(L"");

				RefArray< IDecoder > decoders;
				decoders.push_back(new ReliableDecoder());
				decoders.push_back(new RelayDecoder());
				decoders.push_back(new ReplicatorDecoder());

				const uint8_t* ptr = entry->data;
				for (RefArray< IDecoder >::const_iterator i = decoders.begin(); i != decoders.end(); ++i)
				{
					StringOutputStream ss;

					bool continueNext = (*i)->decode(ptr, ss);

					m_listBox->add(type_name(*i));

					std::vector< std::wstring > strs;
					Split< std::wstring >::any(ss.str(), L"\n\r", strs, false);

					for (std::vector< std::wstring >::const_iterator j = strs.begin(); j != strs.end(); ++j)
						m_listBox->add(L"   " + *j);

					if (!continueNext)
						break;
				}

				m_listBox->add(L"");
				for (uint32_t i = 0; i < entry->size; i += 16)
				{
					StringOutputStream ss;

					for (uint32_t j = 0; j < 16; ++j)
					{
						if (j + i < entry->size)
							FormatHex(ss, entry->data[j + i], 2) << L" ";
					}

					m_listBox->add(ss.str());
				}
			}
		}
	}
}

void MainForm::eventFileDrop(ui::Event* event)
{
	ui::FileDropEvent* fileDropEvent = checked_type_cast< ui::FileDropEvent* >(event);
	
	const std::vector< Path >& files = fileDropEvent->getFiles();
	if (!files.empty())
	{
		m_sequencer->removeAllSequenceItems();

		Ref< traktor::IStream > file = FileSystem::getInstance().open(files[0], File::FmRead);
		if (file)
		{
			std::map< uint8_t, Ref< ui::custom::SequenceGroup > > sequences;
			int32_t duration = 0;

			while (file->available() > 0)
			{
				Reader r(file);

				uint8_t cmd;
				uint32_t time;

				r >> cmd;
				r >> time;

				if (cmd == 0x00)
				{
					uint32_t size;

					r >> size;
					for (uint32_t i = 0; i < size; ++i)
					{
						Ref< PeerInfo > info = new PeerInfo();

						info->time = time;

						r >> info->handle;
						r >> info->name;
						r >> info->status;
						r >> info->direct;
						r >> info->connectionState;

						Ref< ui::custom::SequenceGroup >& group = sequences[info->handle];
						if (!group)
						{
							group = new ui::custom::SequenceGroup(info->name + L" (" + toString< int32_t >(info->handle) + L")");

							group->addChildItem(new ui::custom::Sequence(L"Info"));
							group->addChildItem(new ui::custom::Sequence(L"Sent"));
							group->addChildItem(new ui::custom::Sequence(L"Receive"));

							m_sequencer->addSequenceItem(group);
						}

						Ref< ui::custom::Tick > tick = new ui::custom::Tick(info->time);
						tick->setData(L"INFO", info);

						checked_type_cast< ui::custom::Sequence* >(group->getChildItems().at(0))->addKey(tick);
					}
				}
				else if (cmd == 0x01 || cmd == 0x02)
				{
					Ref< Entry > e = new Entry();

					std::memset(e->data, 0, sizeof(e->data));

					e->time = time;
					e->direction = (cmd == 0x01 ? 0 : 1);

					r >> e->handle;
					r >> e->size;
					r.read(e->data, 1, e->size);

					Ref< ui::custom::SequenceGroup >& group = sequences[e->handle];
					if (!group)
					{
						group = new ui::custom::SequenceGroup(L"n/a (" + toString< int32_t >(e->handle) + L")");

						group->addChildItem(new ui::custom::Sequence(L"Info"));
						group->addChildItem(new ui::custom::Sequence(L"Sent"));
						group->addChildItem(new ui::custom::Sequence(L"Receive"));

						m_sequencer->addSequenceItem(group);
					}

					Ref< ui::custom::Tick > tick = new ui::custom::Tick(e->time);
					tick->setData(L"ENTRY", e);
					
					if (cmd == 0x01)
						checked_type_cast< ui::custom::Sequence* >(group->getChildItems().at(2))->addKey(tick);
					else
						checked_type_cast< ui::custom::Sequence* >(group->getChildItems().at(1))->addKey(tick);
				}

				duration = std::max< int32_t >(duration, time);
			}

			m_sequencer->setLength(duration);

			file->close();
			file = 0;
		}

		m_sequencer->update();
	}
}

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main()
#endif
{
	ui::Application::getInstance()->initialize(
		new EventLoopImpl(),
		new WidgetFactoryImpl()
	);

	MainForm form;

	bool result = true;

	if ((result &= form.create()) == false)
		traktor::log::error << L"Form create failed" << Endl;

	if (result)
	{
		ui::Application::getInstance()->execute();
		form.destroy();
	}

	ui::Application::getInstance()->finalize();
	return 0;
}
