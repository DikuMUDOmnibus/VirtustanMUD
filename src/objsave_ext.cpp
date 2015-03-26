// Part of Bylins http://www.mud.ru

#include <list>
#include <vector>
#include <boost/bind.hpp>
#include "objsave.h"
#include "house.h"
#include "depot.hpp"
#include "parcel.hpp"

namespace ObjSaveSync
{

struct NodeType
{
	// ��� ����/���� ����-����� ����������
	int init_uid;
	// ���� ��� ���� ����������
	int init_type;
	// ��� ����/���� ����-����� �� ����
	int targ_uid;
	// ���� ��� ���� �� ����
	int targ_type;
};

struct ForceNodeType
{
	int uid;
	int type;
};

// ������ ���� ��� �� �������� ����
std::list<NodeType> save_list;
// ��������� �� ������ ������ �� ������� ����-����
std::vector<ForceNodeType> force_list;

void add(int init_uid, int targ_uid, int targ_type)
{
	NodeType node;
	node.init_uid = init_uid;
	node.init_type = CHAR_SAVE;
	node.targ_uid = targ_uid;
	node.targ_type = targ_type;
	save_list.push_front(node);
}

void write_file(int uid, int type)
{
	if (type == CHAR_SAVE)
	{
		for (CHAR_DATA *ch = character_list; ch; ch = ch->next)
		{
			if (ch->get_uid() == uid)
			{
				Crash_crashsave(ch);
				return;
			}
		}
	}
	else if (type == CLAN_SAVE)
	{
		for (ClanListType::const_iterator i = Clan::ClanList.begin(),
			iend = Clan::ClanList.end(); i != iend; ++i)
		{
			if ((*i)->GetRent() == uid)
			{
				(*i)->save_chest();
				return;
			}
		}
	}
	else if (type == PERS_CHEST_SAVE)
	{
		Depot::save_char_by_uid(uid);
	}
	else if (type == PARCEL_SAVE)
	{
		Parcel::save();
	}
}

void add_to_list(int uid, int type)
{
	if (type == PARCEL_SAVE)
	{
		// ����� ���� �� ���� - ���������� ����� �� ���������� type
		std::vector<ForceNodeType>::const_iterator i =
			std::find_if(force_list.begin(), force_list.end(),
				boost::bind(&ForceNodeType::type, _1) == type);
		if (i != force_list.end())
		{
			return;
		}
	}
	else
	{
		// ���������� ��������� ����� �� ���������� � uid, � type
		std::vector<ForceNodeType>::const_iterator i =
			std::find_if(force_list.begin(), force_list.end(),
				boost::bind(&ForceNodeType::uid, _1) == uid &&
				boost::bind(&ForceNodeType::type, _1) == type);
		if (i != force_list.end())
		{
			return;
		}
	}
	ForceNodeType node;
	node.uid = uid;
	node.type = type;
	force_list.push_back(node);
}

void fill_force_list(int uid, int type)
{
	for (std::list<NodeType>::iterator i = save_list.begin();
		i != save_list.end(); /* empty */)
	{
		// ������ � ���������, � ������� ��� �� � ����� �����
		// � save_list ��� ������ � ���� init ��� -> targ �������
		// ������� ������� ����, ���������� �� ������� � targ
		if ((type == PARCEL_SAVE && i->targ_type == PARCEL_SAVE)
			|| (i->targ_uid == uid && i->targ_type == type))
		{
			const int uid = i->init_uid, type = i->init_type;
			save_list.erase(i);
			add_to_list(uid, type);
			fill_force_list(uid, type);
			i = save_list.begin();
		}
		else if (i->init_uid == uid && i->init_type == type)
		{
			const int uid = i->targ_uid, type = i->targ_type;
			save_list.erase(i);
			add_to_list(uid, type);
			fill_force_list(uid, type);
			i = save_list.begin();
		}
		else
		{
			++i;
		}
	}
}

// ���� ������ ������ ��������� �� ���� ����� fill_force_list
// ������� ����� � �������� ����� � ����� �� ���� ������� �����
// �������� � �������� �������� �����
bool checking = false;

void check(int uid, int type)
{
	if (checking) return;

	log("ObjSaveSync::check start");
	checking = true;

	fill_force_list(uid, type);
	for (std::vector<ForceNodeType>::const_iterator i = force_list.begin(),
		iend = force_list.end(); i != iend; ++i)
	{
		if (i->uid == uid && i->type == type)
		{
			// ���������� ������ ���� ����
			continue;
		}
		if (type == PARCEL_SAVE && i->type == PARCEL_SAVE)
		{
			// � ����� ����� ���������� ������ �� ����
			continue;
		}
		write_file(i->uid, i->type);
	}
	force_list.clear();

	checking = false;
	log("ObjSaveSync::check end");
}

} // namespace ObjSaveSync
