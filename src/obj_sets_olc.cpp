// Copyright (c) 2014 Krodo
// Part of Bylins http://www.mud.ru

#include "conf.h"
#include <string>
#include <vector>
#include <map>
#include <array>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

#include "obj_sets.hpp"
#include "obj_sets_stuff.hpp"
#include "structs.h"
#include "obj.hpp"
#include "db.h"
#include "constants.h"
#include "handler.h"
#include "char_player.hpp"
#include "skills.h"
#include "screen.h"
#include "modify.h"
#include "spells.h"

void show_weapon_affects_olc(DESCRIPTOR_DATA *d, const FLAG_DATA &flags);
void show_apply_olc(DESCRIPTOR_DATA *d);
int planebit(const char *str, int *plane, int *bit);

using namespace obj_sets;

namespace obj_sets_olc
{

/// ����� ��� �������������� ���� � ���, �������� ��������� ��������������
/// (������ ������ ������ CON_SEDIT) � ��� ��������� ���� ��� ��� ����
class sedit
{
public:
	sedit() : state(-1), new_entry(false),
		obj_edit(-1), activ_edit(-1), apply_edit(-1) {};

	// ������ �������������� (� ����� �������)
	int state;
	// ������ ���������� ���������
	obj_sets::msg_node msg_edit;
	// ������������� ���
	obj_sets::set_node olc_set;
	// ���� �������� ������ ����
	bool new_entry;

	void parse_global_msg(CHAR_DATA *ch, const char *arg);
	void parse_activ_mage_dmg(CHAR_DATA *ch, const char *arg);
	void parse_activ_phys_dmg(CHAR_DATA *ch, const char *arg);
	void parse_activ_weight_val(CHAR_DATA *ch, const char *arg);
	void parse_activ_ench_sdice(CHAR_DATA *ch, const char *arg);
	void parse_activ_ench_ndice(CHAR_DATA *ch, const char *arg);
	void parse_activ_ench_vnum(CHAR_DATA *ch, const char *arg);
	void parse_activ_skill(CHAR_DATA *ch, const char *arg);
	void parse_activ_apply_mod(CHAR_DATA *ch, const char *arg);
	void parse_activ_apply_loc(CHAR_DATA *ch, const char *arg);
	void parse_activ_prof(CHAR_DATA *ch, const char *arg);
	void parse_activ_change(CHAR_DATA *ch, const char *arg);
	void parse_activ_remove(CHAR_DATA *ch, const char *arg);
	void parse_activ_affects(CHAR_DATA *ch, const char *arg);
	void parse_activ_edit(CHAR_DATA *ch, const char *arg);
	void parse_activ_add(CHAR_DATA *ch, const char *arg);
	void parse_obj_remove(CHAR_DATA *ch, const char *arg);
	void parse_obj_change(CHAR_DATA *ch, const char *arg);
	void parse_obj_edit(CHAR_DATA *ch, const char *arg);
	void parse_obj_add(CHAR_DATA *ch, const char *arg);
	void parse_setmsg(CHAR_DATA *ch, const char *arg);
	void parse_setcomment(CHAR_DATA *ch, const char *arg);
	void parse_setalias(CHAR_DATA *ch, const char *arg);
	void parse_setname(CHAR_DATA *ch, const char *arg);
	void parse_main(CHAR_DATA *ch, const char *arg);

	void save_olc(CHAR_DATA *ch);
	void show_main(CHAR_DATA *ch);
	void show_global_msg(CHAR_DATA *ch);

private:
	// ���� �������� ������� - ��� ����
	int obj_edit;
	// ���� �������� ��������� - ��� ������
	int activ_edit;
	// ���� �������� ������ - ��� ������ � ������
	size_t apply_edit;

	void show_activ_ench_vnum(CHAR_DATA *ch);
	void show_activ_prof(CHAR_DATA *ch);
	void show_activ_skill(CHAR_DATA *ch);
	void show_activ_apply(CHAR_DATA *ch);
	void show_activ_affects(CHAR_DATA *ch);
	void show_activ_edit(CHAR_DATA *ch);
	void show_obj_edit(CHAR_DATA *ch);
	// �������� ���������� �� ���-�� � ����,
	// ����� �� ������� ��� ���������� ��� ������
	bool changed();
	// ��� ��������� � changed()
	bool operator!=(const sedit &r) const
	{
		return olc_set != r.olc_set;
	}
	bool operator==(const sedit &r) const
	{
		return !(*this != r);
	}
};

enum
{
	STATE_MAIN,
	STATE_MAIN_EXIT,
	STATE_SET_REMOVE,
	STATE_NAME,
	STATE_ALIAS,
	STATE_COMMENT,
	STATE_SETMSG_CHAR_ON,
	STATE_SETMSG_CHAR_OFF,
	STATE_SETMSG_ROOM_ON,
	STATE_SETMSG_ROOM_OFF,
	STATE_OBJ_ADD,
	STATE_OBJ_EDIT,
	STATE_OBJ_CHANGE,
	STATE_OBJMSG_CHAR_ON,
	STATE_OBJMSG_CHAR_OFF,
	STATE_OBJMSG_ROOM_ON,
	STATE_OBJMSG_ROOM_OFF,
	STATE_OBJ_REMOVE,
	STATE_TOTAL_ACTIV,
	STATE_ACTIV_ADD,
	STATE_ACTIV_EDIT,
	STATE_ACTIV_AFFECTS,
	STATE_ACTIV_REMOVE,
	STATE_ACTIV_PROF,
	STATE_ACTIV_CHANGE,
	STATE_ACTIV_APPLY_LOC,
	STATE_ACTIV_APPLY_MOD,
	STATE_ACTIV_SKILL,
	STATE_ACTIV_ENCH_VNUM,
	STATE_ACTIV_ENCH_NDICE,
	STATE_ACTIV_ENCH_SDICE,
	STATE_ACTIV_ENCH_WEIGHT,
	STATE_ACTIV_PHYS_DMG,
	STATE_ACTIV_MAGE_DMG,
	STATE_GLOBAL_MSG,
	STATE_GLOBAL_MSG_EXIT,
	STATE_GLBMSG_CHAR_ON,
	STATE_GLBMSG_CHAR_OFF,
	STATE_GLBMSG_ROOM_ON,
	STATE_GLBMSG_ROOM_OFF
};

// ���������� �������� ���� + �������� ��� ��������� ������� ����� MAIN_TOTAL
enum
{
	MAIN_SET_REMOVE = 1,
	MAIN_ENABLED,
	MAIN_NAME,
	MAIN_ALIAS,
	MAIN_COMMENT,
	MAIN_MSG_CHAR_ON,
	MAIN_MSG_CHAR_OFF,
	MAIN_MSG_ROOM_ON,
	MAIN_MSG_ROOM_OFF,
	MAIN_TOTAL
};

/// ���������� ���������������� ������ ������ ����, �������������� ���� ���
/// ��� �� ��������, ��� � �� ����� ����� � ����� ������, ������ ���� �������
std::string main_menu_objlist(CHAR_DATA *ch, const set_node &set, int menu)
{
	std::string out;
	std::vector<int> rnum_list;
	char buf_[128];
	char format[128];
	char buf_vnum[128];
	size_t r_max_name = 0, r_max_vnum = 0;
	size_t l_max_name = 0, l_max_vnum = 0;
	bool left = true;

	for (auto i = set.obj_list.begin(); i != set.obj_list.end(); ++i)
	{
		const int rnum = real_object(i->first);
		if (rnum < 0 || !obj_proto[rnum]->short_description) continue;

		const size_t curr_name =
			strlen_no_colors(obj_proto[rnum]->short_description);
		snprintf(buf_vnum, sizeof(buf_vnum), "%d", obj_index[rnum].vnum);
		const size_t curr_vnum = strlen(buf_vnum);

		if (left)
		{
			l_max_name = std::max(l_max_name, curr_name);
			l_max_vnum = std::max(l_max_vnum, curr_vnum);
		}
		else
		{
			r_max_name = std::max(r_max_name, curr_name);
			r_max_vnum = std::max(r_max_vnum, curr_vnum);
		}
		rnum_list.push_back(rnum);
		left = !left;
	}

	left = true;
	for (auto i = rnum_list.begin(); i != rnum_list.end(); ++i)
	{
		snprintf(buf_vnum, sizeof(buf_vnum), "%d", obj_index[*i].vnum);
		snprintf(format, sizeof(format), "%s%2d%s) %s : %s%%-%zus%s   ",
			CCGRN(ch, C_NRM), menu++, CCNRM(ch, C_NRM),
			colored_name(obj_proto[*i]->short_description,
				left ? l_max_name : r_max_name, true),
			CCCYN(ch, C_NRM), (left ? l_max_vnum : r_max_vnum), CCNRM(ch, C_NRM));
		snprintf(buf_, sizeof(buf_), format, buf_vnum);
		out += buf_;
		if (!left)
		{
			out += "\r\n";
		}
		left = !left;
	}

	if (!out.empty())
	{
		boost::trim_right(out);
		return out + ("\r\n");
	}
	return out;
}

const char * main_menu_str(CHAR_DATA *ch, set_node &olc_set, int num)
{
	static char buf_[1024];
	switch (num)
	{
	case MAIN_SET_REMOVE:
		return "������� �����";
	case MAIN_ENABLED:
		snprintf(buf_, sizeof(buf_), "������ : %s%s%s", CCCYN(ch, C_NRM),
			olc_set.enabled ? "�������" : "���������", CCNRM(ch, C_NRM));
		break;
	case MAIN_NAME:
		snprintf(buf_, sizeof(buf_), "���    : %s",
			olc_set.name.c_str());
		break;
	case MAIN_ALIAS:
		snprintf(buf_, sizeof(buf_), "������ : %s",
			olc_set.alias.c_str());
		break;
	case MAIN_COMMENT:
		snprintf(buf_, sizeof(buf_), "����������� : %s",
			olc_set.comment.c_str());
		break;
	case MAIN_MSG_CHAR_ON:
		snprintf(buf_, sizeof(buf_), "��������� ���������   : %s",
			olc_set.messages.char_on_msg.c_str());
		break;
	case MAIN_MSG_CHAR_OFF:
		snprintf(buf_, sizeof(buf_), "����������� ��������� : %s",
			olc_set.messages.char_off_msg.c_str());
		break;
	case MAIN_MSG_ROOM_ON:
		snprintf(buf_, sizeof(buf_), "��������� � �������   : %s",
			olc_set.messages.room_on_msg.c_str());
		break;
	case MAIN_MSG_ROOM_OFF:
		snprintf(buf_, sizeof(buf_), "����������� � ������� : %s",
			olc_set.messages.room_off_msg.c_str());
		break;
	default:
		return "<error>";
	}
	return buf_;
}

void sedit::show_main(CHAR_DATA *ch)
{
	state = STATE_MAIN;

	char buf_[1024];
	std::string out("\r\n");

	if (new_entry)
	{
		out += "�������� ������ ������ ���������\r\n";
	}
	else
	{
		size_t idx = setidx_by_uid(olc_set.uid);
		if (idx >= sets_list.size())
		{
			send_to_char("�������������� ��������: ����� ��� ������.\r\n", ch);
			ch->desc->sedit.reset();
			STATE(ch->desc) = CON_PLAYING;
			return;
		}
		snprintf(buf_, sizeof(buf_),
			"�������������� ������ ��������� #%zu\r\n", idx + 1);
		out += buf_;
	}
	int i = 1;
	for (/**/; i < MAIN_TOTAL; ++i)
	{
		snprintf(buf_, sizeof(buf_), "%s%2d%s) %s\r\n",
			CCGRN(ch, C_NRM), i, CCNRM(ch, C_NRM),
			main_menu_str(ch, olc_set, i));
		out += buf_;
	}
	// ��������
	snprintf(buf_, sizeof(buf_), "\r\n%s%2d%s) �������� �������(�)\r\n",
		CCGRN(ch, C_NRM), i++, CCNRM(ch, C_NRM));
	out += buf_;
	out += main_menu_objlist(ch, olc_set, i);
	i += olc_set.obj_list.size();
	// ����������
	snprintf(buf_, sizeof(buf_),
		"\r\n%s%2d%s) ����������� ����� �����������\r\n",
		CCGRN(ch, C_NRM), i++, CCNRM(ch, C_NRM));
	out += buf_;
	snprintf(buf_, sizeof(buf_), "%s%2d%s) �������� ���������\r\n",
		CCGRN(ch, C_NRM), i++, CCNRM(ch, C_NRM));
	out += buf_;
	for (auto k = olc_set.activ_list.begin(); k != olc_set.activ_list.end(); ++k)
	{
		if (k->second.prof.count() != k->second.prof.size())
		{
			std::string prof;
			print_bitset(k->second.prof, pc_class_name, ",", prof);
			snprintf(buf_, sizeof(buf_),
				"%s%2d%s) ������������� ���������: %s%d (%s)%s\r\n",
				CCGRN(ch, C_NRM), i++, CCNRM(ch, C_NRM),
				CCCYN(ch, C_NRM), k->first, prof.c_str(),
				CCNRM(ch, C_NRM));
		}
		else
		{
			snprintf(buf_, sizeof(buf_),
				"%s%2d%s) ������������� ���������: %s%d%s\r\n",
				CCGRN(ch, C_NRM), i++, CCNRM(ch, C_NRM),
				CCCYN(ch, C_NRM), k->first,  CCNRM(ch, C_NRM));
		}
		out += buf_;
	}

	snprintf(buf_, sizeof(buf_),
		"\r\n%s%2d%s) ����� Q(�)\r\n"
		"��� ����� : ",
		CCGRN(ch, C_NRM), i++, CCNRM(ch, C_NRM));
	out += buf_;

	send_to_char(out, ch);
}

void sedit::show_obj_edit(CHAR_DATA *ch)
{
	state = STATE_OBJ_EDIT;

	int rnum = real_object(obj_edit);
	if (rnum < 0)
	{
		send_to_char(ch,
			"������: �� ������ �������� �������� %s:%d (%s).\r\n",
			__FILE__, __LINE__, __func__);
		obj_edit = -1;
		show_main(ch);
		return;
	}
	auto obj = olc_set.obj_list.find(obj_edit);
	if (obj == olc_set.obj_list.end())
	{
		send_to_char(ch,
			"������: ������� �� � ������ %s:%d (%s).\r\n",
			__FILE__, __LINE__, __func__);
		show_main(ch);
		return;
	}
	const msg_node &msg = obj->second;

	char buf_[2048];
	snprintf(buf_, sizeof(buf_),
		"\r\n�������������� �������� '%s'\r\n"
		"%s 1%s) ������� �� ������\r\n"
		"%s 2%s) ����������� ����� (vnum) : %s%d%s\r\n"
		"%s 3%s) ��������� ���������   : %s\r\n"
		"%s 4%s) ����������� ��������� : %s\r\n"
		"%s 5%s) ��������� � �������   : %s\r\n"
		"%s 6%s) ����������� � ������� : %s\r\n"
		"%s 7%s) � ������� ���� Q(�)\r\n"
		"��� ����� : ",
		obj_proto[rnum]->short_description,
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM),
		CCCYN(ch, C_NRM), obj_edit, CCNRM(ch, C_NRM),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), msg.char_on_msg.c_str(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), msg.char_off_msg.c_str(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), msg.room_on_msg.c_str(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), msg.room_off_msg.c_str(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM));
	send_to_char(buf_, ch);
}

void sedit::show_activ_edit(CHAR_DATA *ch)
{
	state = STATE_ACTIV_EDIT;

	auto i = olc_set.activ_list.find(activ_edit);
	if (i == olc_set.activ_list.end())
	{
		send_to_char(ch,
			"������: ��������� �� ������ %s:%d (%s).\r\n",
			__FILE__, __LINE__, __func__);
		show_main(ch);
		return;
	}
	const activ_node &activ = i->second;

	char buf_aff[2048];
	sprintbits(activ.affects, weapon_affects, buf_aff, ",");
	std::string aff_str = line_split_str(buf_aff, ",", 80, 14);
	std::string prof_str;
	if (activ.prof.count() != activ.prof.size())
	{
		print_bitset(activ.prof, pc_class_name, ",", prof_str);
	}
	else
	{
		prof_str = "���";
	}

	std::string out;
	char buf_[2048];
	snprintf(buf_, sizeof(buf_),
		"\r\n�������������� ����������\r\n"
		"%s 1%s) ������� �� ������\r\n"
		"%s 2%s) ���������� ��������� : %s%d%s\r\n"
		"%s 3%s) ��������� : %s%s%s\r\n"
		"%s 4%s) ������� : %s%s%s\r\n",
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM),
		CCCYN(ch, C_NRM), activ_edit, CCNRM(ch, C_NRM),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM),
		CCCYN(ch, C_NRM), prof_str.c_str(), CCNRM(ch, C_NRM),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM),
		CCCYN(ch, C_NRM), aff_str.c_str(), CCNRM(ch, C_NRM));
	out += buf_;

	int cnt = 5;
	for (auto i = activ.apply.begin(); i != activ.apply.end(); ++i)
	{
		if (i->location > 0)
		{
			snprintf(buf_, sizeof(buf_), "%s%2d%s) ��������� ������ : %s",
				CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM),
				print_obj_affects(*i).c_str());
		}
		else
		{
			snprintf(buf_, sizeof(buf_),
				"%s%2d%s) ��������� ������ : %s%s%s\r\n",
				CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM),
				CCCYN(ch, C_NRM), "������", CCNRM(ch, C_NRM));
		}
		out += buf_;
	}

	if (activ.skill.first > 0 && activ.skill.first <= MAX_SKILL_NUM)
	{
		snprintf(buf_, sizeof(buf_),
			"%s%2d%s) ���������� ������ : %s%+d to %s%s\r\n",
			CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM), CCCYN(ch, C_NRM),
			activ.skill.second, skill_info[activ.skill.first].name,
			CCNRM(ch, C_NRM));
	}
	else
	{
		snprintf(buf_, sizeof(buf_), "%s%2d%s) ���������� ������ : ���\r\n",
			CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM));
	}
	out += buf_;

	if (activ.enchant.first > 0)
	{
		const int rnum = real_object(activ.enchant.first);
		const char *name =
			(rnum >= 0 ? obj_proto[rnum]->short_description : "<null>");
		if (GET_OBJ_TYPE(obj_proto[rnum]) == ITEM_WEAPON)
		{
			snprintf(buf_, sizeof(buf_),
				"%s%2d%s) ����������� �������� : %s[%d] %s ��� %+d, ������ %+dD%+d%s\r\n",
				CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM), CCCYN(ch, C_NRM),
				activ.enchant.first, name, activ.enchant.second.weight,
				activ.enchant.second.ndice, activ.enchant.second.sdice,
				CCNRM(ch, C_NRM));
		}
		else
		{
			snprintf(buf_, sizeof(buf_),
				"%s%2d%s) ����������� �������� : %s[%d] %s ��� %+d%s\r\n",
				CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM), CCCYN(ch, C_NRM),
				activ.enchant.first, name, activ.enchant.second.weight,
				CCNRM(ch, C_NRM));
		}
	}
	else
	{
		snprintf(buf_, sizeof(buf_), "%s%2d%s) ����������� ��������: ���\r\n",
			CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM));
	}
	out += buf_;

	snprintf(buf_, sizeof(buf_),
		"%s%2d%s) ���������� ���. ����� : %s%+d%%%s\r\n",
		CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM),
		CCCYN(ch, C_NRM), activ.bonus.phys_dmg, CCNRM(ch, C_NRM));
	out += buf_;

	snprintf(buf_, sizeof(buf_),
		"%s%2d%s) ���������� ���. ����� : %s%+d%%%s\r\n",
		CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM),
		CCCYN(ch, C_NRM), activ.bonus.mage_dmg, CCNRM(ch, C_NRM));
	out += buf_;

	snprintf(buf_, sizeof(buf_),
		"%s%2d%s) � ������� ���� Q(�)\r\n"
		"��� ����� : ",
		CCGRN(ch, C_NRM), cnt++, CCNRM(ch, C_NRM));
	out += buf_;

	send_to_char(out, ch);
}

bool sedit::changed()
{
	const size_t idx = setidx_by_uid(olc_set.uid);
	if (idx < sets_list.size() && *(sets_list.at(idx)) != olc_set)
	{
		return true;
	}
	return false;
}

void sedit::save_olc(CHAR_DATA *ch)
{
	if (new_entry)
	{
		boost::shared_ptr<set_node> set_ptr =
			boost::make_shared<set_node>(olc_set);
		sets_list.push_back(set_ptr);
		verify_set(*set_ptr);
		save();
		init_obj_index();
		return;
	}

	const size_t idx = setidx_by_uid(olc_set.uid);
	if (idx < sets_list.size())
	{
		*(sets_list.at(idx)) = olc_set;
		verify_set(*(sets_list.at(idx)));
		save();
		init_obj_index();
	}
	else
	{
		send_to_char("������ ����������: ����� ��� ������.", ch);
	}
}

void parse_main_exit(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	switch (*arg)
	{
	case 'y':
	case 'Y':
	case '�':
	case '�':
		STATE(ch->desc) = CON_PLAYING;
		ch->desc->sedit->save_olc(ch);
		ch->desc->sedit.reset();
		send_to_char("��������� ���������.\r\n", ch);
		break;
	case 'n':
	case 'N':
	case '�':
	case '�':
		ch->desc->sedit.reset();
		STATE(ch->desc) = CON_PLAYING;
		send_to_char("�������������� ��������.\r\n", ch);
		break;
	default:
		send_to_char(
			"�������� �����!\r\n"
			"�� ������ ��������� ���������? Y(�)/N(�) : ", ch);
		break;
	}
}

void parse_set_remove(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	switch (*arg)
	{
	case 'y':
	case 'Y':
	case '�':
	case '�':
	{
		for (auto i = sets_list.begin(); i != sets_list.end(); ++i)
		{
			if ((*i)->uid == ch->desc->sedit->olc_set.uid)
			{
				sets_list.erase(i);
				save();
				init_obj_index();
				send_to_char("����� ������.\r\n", ch);
				break;
			}
		}
		STATE(ch->desc) = CON_PLAYING;
		ch->desc->sedit.reset();
		break;
	}
	case 'n':
	case 'N':
	case '�':
	case '�':
		send_to_char("�������� ��������.\r\n", ch);
		ch->desc->sedit->show_main(ch);
		break;
	default:
		send_to_char(
			"�������� �����!\r\n"
			"����������� �������� ������ Y(�)/N(�) : ", ch);
		break;
	}
}

void sedit::parse_obj_remove(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	switch (*arg)
	{
	case 'y':
	case 'Y':
	case '�':
	case '�':
	{
		auto i = olc_set.obj_list.find(obj_edit);
		if (i != olc_set.obj_list.end())
		{
			olc_set.obj_list.erase(i);
			send_to_char("������� ������ �� ������.\r\n", ch);
		}
		obj_edit = -1;
		show_main(ch);
		break;
	}
	case 'n':
	case 'N':
	case '�':
	case '�':
		send_to_char("�������� ��������.\r\n", ch);
		show_obj_edit(ch);
		break;
	default:
		send_to_char("�������� �����!\r\n", ch);
		send_to_char("����������� �������� �������� Y(�)/N(�) :", ch);
		break;
	}
}

void sedit::parse_activ_remove(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	switch (*arg)
	{
	case 'y':
	case 'Y':
	case '�':
	case '�':
	{
		auto i = olc_set.activ_list.find(activ_edit);
		if (i != olc_set.activ_list.end())
		{
			olc_set.activ_list.erase(i);
			send_to_char("��������� ������ �� ������.\r\n", ch);
		}
		activ_edit = -1;
		show_main(ch);
		break;
	}
	case 'n':
	case 'N':
	case '�':
	case '�':
		send_to_char("�������� ��������.\r\n", ch);
		show_activ_edit(ch);
		break;
	default:
		send_to_char("�������� �����!\r\n", ch);
		send_to_char("����������� �������� ���������� Y(�)/N(�) :", ch);
		break;
	}
}

void sedit::show_global_msg(CHAR_DATA *ch)
{
	state = STATE_GLOBAL_MSG;

	char buf_[1024];
	snprintf(buf_, sizeof(buf_),
		"\r\n�������������� ���������� ���������\r\n"
		"%s 1%s) ��������� ���������   : %s\r\n"
		"%s 2%s) ����������� ��������� : %s\r\n"
		"%s 3%s) ��������� � �������   : %s\r\n"
		"%s 4%s) ����������� � ������� : %s\r\n"
		"%s 5%s) ����� Q(�)\r\n"
		"��� ����� : ",
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), msg_edit.char_on_msg.c_str(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), msg_edit.char_off_msg.c_str(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), msg_edit.room_on_msg.c_str(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), msg_edit.room_off_msg.c_str(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM));
	send_to_char(buf_, ch);
}

void sedit::parse_global_msg(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg)
	{
		send_to_char("�������� �����!\r\n", ch);
		show_global_msg(ch);
		return;
	}
	if (!isdigit(*arg))
	{
		switch(*arg)
		{
		case 'Q':
		case 'q':
		case '�':
		case '�':
			if (msg_edit != global_msg)
			{
				send_to_char("�� ������ ��������� ���������? Y(�)/N(�) : ", ch);
				state = STATE_GLOBAL_MSG_EXIT;
			}
			else
			{
				send_to_char("�������������� ��������.\r\n", ch);
				ch->desc->sedit.reset();
				STATE(ch->desc) = CON_PLAYING;
			}
			break;
		default:
			send_to_char("�������� �����!\r\n", ch);
			show_global_msg(ch);
			break;
		}
		return;
	}

	unsigned num = atoi(arg);

	switch (num)
	{
	case 1:
		send_to_char("��������� ��������� ��� ��������� : ", ch);
		state = STATE_GLBMSG_CHAR_ON;
		break;
	case 2:
		send_to_char("��������� ��������� ��� ����������� : ", ch);
		state = STATE_GLBMSG_CHAR_OFF;
		break;
	case 3:
		send_to_char("��������� � ������� ��� ��������� : ", ch);
		state = STATE_GLBMSG_ROOM_ON;
		break;
	case 4:
		send_to_char("��������� � ������� ��� ����������� : ", ch);
		state = STATE_GLBMSG_ROOM_OFF;
		break;
	case 5:
		if (msg_edit != global_msg)
		{
			send_to_char("�� ������ ��������� ���������? Y(�)/N(�) : ", ch);
			state = STATE_GLOBAL_MSG_EXIT;
		}
		else
		{
			send_to_char("�������������� ��������.\r\n", ch);
			ch->desc->sedit.reset();
			STATE(ch->desc) = CON_PLAYING;
		}
		break;
	default:
		send_to_char("�������� �����!\r\n", ch);
		show_global_msg(ch);
		break;
	}
}

void parse_global_msg_exit(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	switch (*arg)
	{
	case 'y':
	case 'Y':
	case '�':
	case '�':
		STATE(ch->desc) = CON_PLAYING;
		global_msg = ch->desc->sedit->msg_edit;
		save();
		ch->desc->sedit.reset();
		send_to_char("��������� ���������.\r\n", ch);
		break;
	case 'n':
	case 'N':
	case '�':
	case '�':
		ch->desc->sedit.reset();
		STATE(ch->desc) = CON_PLAYING;
		send_to_char("�������������� ��������.\r\n", ch);
		break;
	default:
		send_to_char(
			"�������� �����!\r\n"
			"�� ������ ��������� ���������? Y(�)/N(�) : ", ch);
		break;
	}
}

void sedit::parse_main(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg)
	{
		send_to_char("�������� �����!\r\n", ch);
		show_main(ch);
		return;
	}
	if (!isdigit(*arg))
	{
		switch(*arg)
		{
		case 'Q':
		case 'q':
		case '�':
		case '�':
			if (new_entry || changed())
			{
				send_to_char("�� ������ ��������� ���������? Y(�)/N(�) : ", ch);
				state = STATE_MAIN_EXIT;
			}
			else
			{
				send_to_char("�������������� ��������.\r\n", ch);
				ch->desc->sedit.reset();
				STATE(ch->desc) = CON_PLAYING;
			}
			break;
		default:
			send_to_char("�������� �����!\r\n", ch);
			show_main(ch);
			break;
		}
		return;
	}

	unsigned num = atoi(arg);

	switch (num)
	{
	case MAIN_SET_REMOVE:
		send_to_char("����������� �������� ������ Y(�)/N(�) : ", ch);
		state = STATE_SET_REMOVE;
		return;
	case MAIN_ENABLED:
		olc_set.enabled = !olc_set.enabled;
		show_main(ch);
		return;
	case MAIN_NAME:
		send_to_char("��� ������ (���������, ������� ����) : ", ch);
		state = STATE_NAME;
		return;
	case MAIN_ALIAS:
		send_to_char(
			"������ ������ ��� ������� (����� ������� �/��� ������) : ", ch);
		state = STATE_ALIAS;
		return;
	case MAIN_COMMENT:
		send_to_char("��� ����������� �� 40 �������� (����� �� slist) : ", ch);
		state = STATE_COMMENT;
		return;
	case MAIN_MSG_CHAR_ON:
		send_to_char("��������� ��������� ��� ��������� : ", ch);
		state = STATE_SETMSG_CHAR_ON;
		return;
	case MAIN_MSG_CHAR_OFF:
		send_to_char("��������� ��������� ��� ����������� : ", ch);
		state = STATE_SETMSG_CHAR_OFF;
		return;
	case MAIN_MSG_ROOM_ON:
		send_to_char("��������� � ������� ��� ��������� : ", ch);
		state = STATE_SETMSG_ROOM_ON;
		return;
	case MAIN_MSG_ROOM_OFF:
		send_to_char("��������� � ������� ��� ����������� : ", ch);
		state = STATE_SETMSG_ROOM_OFF;
		return;
	default:
		break;
	}

	const unsigned NUM_ADD_OBJ = MAIN_TOTAL;
	const unsigned NUM_TOTAL_ACTIV = NUM_ADD_OBJ + olc_set.obj_list.size() + 1;
	const unsigned NUM_ADD_ACTIV = NUM_TOTAL_ACTIV + 1;
	const unsigned NUM_QUIT = NUM_ADD_ACTIV + olc_set.activ_list.size() + 1;

	// ����� ���������� ���� ���� ��������, �� ���� ����������
	if (num == NUM_ADD_OBJ)
	{
		send_to_char(
			"Vnum ������������ �������� (��������� ����� ������) : ", ch);
		state = STATE_OBJ_ADD;
	}
	else if (!olc_set.obj_list.empty()
		&& num > NUM_ADD_OBJ
		&& num <= NUM_ADD_OBJ + olc_set.obj_list.size())
	{
		// �������������� ��������
		const unsigned offset = NUM_ADD_OBJ + 1;
		auto i = olc_set.obj_list.begin();
		if (num > offset)
		{
			std::advance(i, num - offset);
		}
		obj_edit = i->first;
		show_obj_edit(ch);
	}
	else if (num == NUM_TOTAL_ACTIV)
	{
		state = STATE_TOTAL_ACTIV;
		send_to_char(print_total_activ(olc_set), ch);
		send_to_char("������� ����� ������� ��� ����������� : ", ch);
	}
	else if (num == NUM_ADD_ACTIV)
	{
		send_to_char(ch,
			"������� ���-�� ��������� ��� ��������� (%u-%u) : ",
			MIN_ACTIVE_SIZE, MAX_ACTIVE_SIZE);
		state = STATE_ACTIV_ADD;
	}
	else if (!olc_set.activ_list.empty()
		&& num > NUM_ADD_ACTIV
		&& num <= NUM_ADD_ACTIV + olc_set.activ_list.size())
	{
		// �������������� ����������
		const unsigned offset = NUM_ADD_ACTIV + 1;
		auto i = olc_set.activ_list.begin();
		if (num > offset)
		{
			std::advance(i, num - offset);
		}
		activ_edit = i->first;
		show_activ_edit(ch);
	}
	else if (num == NUM_QUIT)
	{
		if (new_entry || changed())
		{
			send_to_char("�� ������ ��������� ���������? Y(�)/N(�) : ", ch);
			state = STATE_MAIN_EXIT;
		}
		else
		{
			send_to_char("�������������� ��������.\r\n", ch);
			ch->desc->sedit.reset();
			STATE(ch->desc) = CON_PLAYING;
		}
	}
	else
	{
		send_to_char("�������� �����!\r\n", ch);
		show_main(ch);
	}
}

void sedit::parse_setname(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg)
	{
		send_to_char("��� ������ �������.\r\n", ch);
		olc_set.name.clear();
	}
	else
	{
		olc_set.name = arg;
	}
	show_main(ch);
}

void sedit::parse_setalias(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg)
	{
		send_to_char("����� ������ ������.\r\n", ch);
		olc_set.alias.clear();
	}
	else
	{
		olc_set.alias = arg;
	}
	show_main(ch);
}

void sedit::parse_setcomment(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg)
	{
		send_to_char("����������� ������ ������.\r\n", ch);
		olc_set.comment.clear();
	}
	else
	{
		olc_set.comment = arg;
		olc_set.comment = olc_set.comment.substr(0, 40);
	}
	show_main(ch);
}

/// ����� � ����� ����� ���� ���������� ��� ��� ���� ���������
enum { PARSE_GLB_MSG, PARSE_SET_MSG, PARSE_OBJ_MSG };

void sedit::parse_setmsg(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	int parse_type = PARSE_GLB_MSG;

	switch(state)
	{
	case STATE_SETMSG_CHAR_ON:
	case STATE_SETMSG_CHAR_OFF:
	case STATE_SETMSG_ROOM_ON:
	case STATE_SETMSG_ROOM_OFF:
		parse_type = PARSE_SET_MSG;
		break;
	case STATE_OBJMSG_CHAR_ON:
	case STATE_OBJMSG_CHAR_OFF:
	case STATE_OBJMSG_ROOM_ON:
	case STATE_OBJMSG_ROOM_OFF:
		parse_type = PARSE_OBJ_MSG;
		break;
	default:
		break;
	}

	msg_node &msg = parse_type == PARSE_SET_MSG
		? olc_set.messages : parse_type == PARSE_OBJ_MSG
			? olc_set.obj_list.at(obj_edit) : msg_edit;

	switch(state)
	{
	case STATE_SETMSG_CHAR_ON:
	case STATE_OBJMSG_CHAR_ON:
	case STATE_GLBMSG_CHAR_ON:
		if ((msg.char_on_msg = arg).empty())
			send_to_char("��������� ���������� ��������� �������.\r\n", ch);
		break;
	case STATE_SETMSG_CHAR_OFF:
	case STATE_OBJMSG_CHAR_OFF:
	case STATE_GLBMSG_CHAR_OFF:
		if ((msg.char_off_msg = arg).empty())
			send_to_char("��������� ������������ ��������� �������.\r\n", ch);
		break;
	case STATE_SETMSG_ROOM_ON:
	case STATE_OBJMSG_ROOM_ON:
	case STATE_GLBMSG_ROOM_ON:
		if ((msg.room_on_msg = arg).empty())
			send_to_char("��������� ���������� � ������� �������.\r\n", ch);
		break;
	case STATE_SETMSG_ROOM_OFF:
	case STATE_OBJMSG_ROOM_OFF:
	case STATE_GLBMSG_ROOM_OFF:
		if ((msg.room_off_msg = arg).empty())
			send_to_char("��������� ������������ � ������� �������.\r\n", ch);
		break;
	default:
		break;
	}

	if (parse_type == PARSE_OBJ_MSG)
	{
		show_obj_edit(ch);
	}
	else if (parse_type == PARSE_SET_MSG)
	{
		show_main(ch);
	}
	else
	{
		show_global_msg(ch);
	}
}

void sedit::parse_activ_add(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	unsigned num = 0;
	if (!*arg || !isdigit(*arg)
		|| (num = atoi(arg)) < MIN_ACTIVE_SIZE
		|| num > MAX_ACTIVE_SIZE)
	{
		send_to_char("������������ ���-�� ��������� ��� ���������.\r\n", ch);
		show_main(ch);
		return;
	}
	auto i = olc_set.activ_list.find(num);
	if (i != olc_set.activ_list.end())
	{
		send_to_char(ch, "� ������ ��� ���� ��������� �� %d %s.\r\n",
			num, desc_count(num, WHAT_OBJECT));
	}
	else
	{
		send_to_char(ch, "��������� �� %d %s �������� � �����.\r\n",
			num, desc_count(num, WHAT_OBJECT));
		activ_node node;
		// GCC 4.4
		//olc_set.activ_list.emplace(num, node);
		olc_set.activ_list.insert(std::make_pair(num, node));
	}
	show_main(ch);
}

void sedit::show_activ_ench_vnum(CHAR_DATA *ch)
{
	state = STATE_ACTIV_ENCH_VNUM;
	std::string out = main_menu_objlist(ch, olc_set, 1);
	out += "������� vnum �������� (0 - ������� � �����, ������ ���� - �����) :";
	send_to_char(out, ch);
}

void sedit::parse_activ_ench_vnum(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);

	if (!*arg || !isdigit(*arg))
	{
		show_activ_edit(ch);
		return;
	}

	int rnum = -1;
	unsigned vnum = atoi(arg);
	if (vnum == 0)
	{
		olc_set.activ_list.at(activ_edit).enchant.first = 0;
		olc_set.activ_list.at(activ_edit).enchant.second.weight = 0;
		olc_set.activ_list.at(activ_edit).enchant.second.ndice = 0;
		olc_set.activ_list.at(activ_edit).enchant.second.sdice = 0;
		show_activ_edit(ch);
		return;
	}

	if (!olc_set.obj_list.empty()
		&& vnum <= olc_set.obj_list.size()
		&& vnum >= 1)
	{
		auto i = olc_set.obj_list.begin();
		int add = vnum - 1;
		if (add > 0)
		{
			std::advance(i, add);
		}
		vnum = i->first;
		rnum = real_object(vnum);
	}
	else
	{
		rnum = real_object(vnum);
		if (rnum < 0)
		{
			send_to_char(ch, "��������� � vnum %d �� ����������.\r\n", vnum);
			show_activ_ench_vnum(ch);
			return;
		}
		else if (olc_set.obj_list.find(vnum) == olc_set.obj_list.end())
		{
			send_to_char(ch,
				"� ������ ������ ��� �������� � vnum %d.\r\n", vnum);
			show_activ_ench_vnum(ch);
			return;
		}
	}

	olc_set.activ_list.at(activ_edit).enchant.first = vnum;

	if (rnum >= 0 && GET_OBJ_TYPE(obj_proto[rnum]) == ITEM_WEAPON)
	{
		state = STATE_ACTIV_ENCH_NDICE;
		send_to_char("������� ��������� ������� ������ (0 - ��� ���������) :", ch);
	}
	else
	{
		olc_set.activ_list.at(activ_edit).enchant.second.ndice = 0;
		olc_set.activ_list.at(activ_edit).enchant.second.sdice = 0;
		state = STATE_ACTIV_ENCH_WEIGHT;
		send_to_char("������� ������������ ��� (0 - ��� ���������) :", ch);
	}
}

void sedit::parse_activ_weight_val(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);

	const int weight = atoi(arg);
	olc_set.activ_list.at(activ_edit).enchant.second.weight =
		std::max(std::min(weight, 100), -100);

	show_activ_edit(ch);
}

void sedit::parse_activ_ench_ndice(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);

	const int ndice = atoi(arg);
	olc_set.activ_list.at(activ_edit).enchant.second.ndice =
		std::max(std::min(ndice, 100), -100);

	state = STATE_ACTIV_ENCH_SDICE;
	send_to_char("������� ��������� ������ ������� (0 - ��� ���������) :", ch);
}

void sedit::parse_activ_ench_sdice(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);

	const int sdice = atoi(arg);
	olc_set.activ_list.at(activ_edit).enchant.second.sdice =
		std::max(std::min(sdice, 100), -100);

	state = STATE_ACTIV_ENCH_WEIGHT;
	send_to_char("������� ������������ ��� (0 - ��� ���������) :", ch);
}

void sedit::parse_obj_add(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);

	if (!*arg || !isdigit(*arg))
	{
		send_to_char("������������ ����������� ����� ��������.\r\n", ch);
		show_main(ch);
		return;
	}

	std::vector<std::string> vnum_list;
	boost::split(vnum_list, arg, boost::is_any_of(" "),
		boost::token_compress_on);
	for (auto i = vnum_list.begin(); i != vnum_list.end(); ++i)
	{
		const int vnum = atoi(i->c_str());
		const int rnum = real_object(vnum);
		if (olc_set.obj_list.size() >= MAX_OBJ_LIST)
		{
			send_to_char(
				"����� ��� �������� ������������ ���-�� ���������.\r\n", ch);
		}
		else if (rnum < 0)
		{
			send_to_char(ch, "��������� � vnum %d �� ����������.\r\n", vnum);
		}
		else if (is_duplicate(olc_set.uid, vnum))
		{
			send_to_char(ch,
				"������� '%s' ��� �������� ������ ������� ������.\r\n",
				obj_proto[rnum]->short_description);
		}
		else if (!verify_wear_flag(obj_proto[rnum]))
		{
			send_to_char(ch,
				"������� '%s' ����� ����������� ���� ��� ���������.\r\n",
				obj_proto[rnum]->short_description);
		}
		else
		{
			msg_node empty_msg;
			// GCC 4.4
			//olc_set.obj_list.emplace(vnum, empty_msg);
			olc_set.obj_list.insert(std::make_pair(vnum, empty_msg));
			send_to_char(ch,
				"������� '%s' �������� � �����.\r\n",
				obj_proto[rnum]->short_description);
		}
	}

	show_main(ch);
}

void sedit::parse_obj_edit(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg)
	{
		send_to_char("�������� �����!\r\n", ch);
		show_obj_edit(ch);
		return;
	}
	if (!isdigit(*arg))
	{
		switch(*arg)
		{
		case 'Q':
		case 'q':
		case '�':
		case '�':
			show_main(ch);
			break;
		default:
			send_to_char("�������� �����!\r\n", ch);
			show_obj_edit(ch);
			break;
		}
		return;
	}

	unsigned num = atoi(arg);
	switch (num)
	{
	case 1:
		send_to_char("����������� �������� �������� Y(�)/N(�) :", ch);
		state = STATE_OBJ_REMOVE;
		break;
	case 2:
		send_to_char("����������� ����� ������ �������� : ", ch);
		state = STATE_OBJ_CHANGE;
		break;
	case 3:
		send_to_char("��������� ��������� ��� ��������� : ", ch);
		state = STATE_OBJMSG_CHAR_ON;
		break;
	case 4:
		send_to_char("��������� ��������� ��� ����������� : ", ch);
		state = STATE_OBJMSG_CHAR_OFF;
		break;
	case 5:
		send_to_char("��������� � ������� ��� ��������� : ", ch);
		state = STATE_OBJMSG_ROOM_ON;
		break;
	case 6:
		send_to_char("��������� � ������� ��� ����������� : ", ch);
		state = STATE_OBJMSG_ROOM_OFF;
		break;
	case 7:
		show_main(ch);
		break;
	default:
		send_to_char("�������� �����!\r\n", ch);
		show_obj_edit(ch);
		break;
	}
}

void sedit::show_activ_affects(CHAR_DATA *ch)
{
	state = STATE_ACTIV_AFFECTS;
	show_weapon_affects_olc(ch->desc,
		olc_set.activ_list.at(activ_edit).affects);
}

void sedit::show_activ_apply(CHAR_DATA *ch)
{
	state = STATE_ACTIV_APPLY_LOC;
	show_apply_olc(ch->desc);
}

void sedit::show_activ_skill(CHAR_DATA *ch)
{
	state = STATE_ACTIV_SKILL;

	int col = 0;
	std::string out;
	char buf_[128];

	for (int i = 1; i <= MAX_SKILL_NUM; ++i)
	{
		if (!skill_info[i].name || *skill_info[i].name == '!')
		{
			continue;
		}
		snprintf(buf_, sizeof(buf_), "%s%3d%s) %25s     %s",
			CCGRN(ch, C_NRM), i, CCNRM(ch, C_NRM),
			skill_info[i].name, !(++col % 2) ? "\r\n" : "");
		out += buf_;
	}
	send_to_char(out, ch);
	send_to_char(
		"\r\n������� ����� � ������� �������� ������� (0 - �����) : ", ch);
}

void sedit::show_activ_prof(CHAR_DATA *ch)
{
	state = STATE_ACTIV_PROF;
	char buf_[128];
	std::string out;
	std::bitset<NUM_CLASSES> &bits = olc_set.activ_list.at(activ_edit).prof;

	for (size_t i = 0; i < bits.size(); ++i)
	{
		snprintf(buf_, sizeof(buf_), "%s%2zu%s) %s\r\n",
			CCGRN(ch, C_NRM), i + 1, CCNRM(ch, C_NRM),
			i < pc_class_name.size() ? pc_class_name.at(i) : "UNDEF");
		out += buf_;
	}

	snprintf(buf_, sizeof(buf_),
		"%s%2zu%s) �������� ���\r\n"
		"%s%2zu%s) ���������� ���\r\n",
		CCGRN(ch, C_NRM), bits.size() + 1, CCNRM(ch, C_NRM),
		CCGRN(ch, C_NRM), bits.size() + 2, CCNRM(ch, C_NRM));
	out += buf_;

	snprintf(buf_, sizeof(buf_), "������� ��������� : %s", CCCYN(ch, C_NRM));
	out += buf_;
	print_bitset(bits, pc_class_name, ",", out, true);
	out += CCNRM(ch, C_NRM);
	out += "\r\n�������� ��������� ��� ������� ���������� (0 - �����) : ";

	send_to_char(out, ch);
}

void sedit::parse_activ_prof(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg || !isdigit(*arg))
	{
		send_to_char("������������ ����.\r\n", ch);
		show_activ_prof(ch);
		return;
	}

	const unsigned num = atoi(arg);
	if (num == 0)
	{
		show_activ_edit(ch);
		return;
	}

	std::bitset<NUM_CLASSES> &bits = olc_set.activ_list.at(activ_edit).prof;
	if (num > 0 && num <= bits.size())
	{
		bits.flip(num - 1);
	}
	else if (num == bits.size() + 1)
	{
		bits.reset();
	}
	else if (num == bits.size() + 2)
	{
		bits.set();
	}
	else
	{
		send_to_char("������������ ����.\r\n", ch);
	}
	show_activ_prof(ch);
}

void sedit::parse_activ_skill(CHAR_DATA *ch, const char *arg)
{
	auto &skill = olc_set.activ_list.at(activ_edit).skill;
	skip_spaces(&arg);
	int num = atoi(arg), ssnum = 0, ssval = 0;

	if (num == 0)
	{
		skill.first = 0;
		skill.second = 0;
		show_activ_edit(ch);
		return;
	}

	if (sscanf(arg, "%d %d", &ssnum, &ssval) < 2)
	{
		send_to_char("�� ������ ������� �������� �������.\r\n", ch);
		show_activ_skill(ch);
	}
	else if (ssnum > MAX_SKILL_NUM || ssnum < 0
		|| !skill_info[ssnum].name
		|| *skill_info[ssnum].name == '!')
	{
		send_to_char("����������� ������.\r\n", ch);
		show_activ_skill(ch);
	}
	else if (ssval == 0)
	{
		skill.first = 0;
		skill.second = 0;
		show_activ_edit(ch);
	}
	else
	{
		skill.first = ssnum;
		skill.second = std::max(-200, std::min(ssval, 200));
		show_activ_edit(ch);
	}
}

void sedit::parse_activ_phys_dmg(CHAR_DATA *ch, const char *arg)
{
	bonus_type &bonus = olc_set.activ_list.at(activ_edit).bonus;
	skip_spaces(&arg);
	int num = atoi(arg);

	if (num <= 0)
	{
		bonus.phys_dmg = 0;
	}
	else
	{
		bonus.phys_dmg = std::min(num, 1000);
	}
	show_activ_edit(ch);
}

void sedit::parse_activ_mage_dmg(CHAR_DATA *ch, const char *arg)
{
	bonus_type &bonus = olc_set.activ_list.at(activ_edit).bonus;
	skip_spaces(&arg);
	int num = atoi(arg);

	if (num <= 0)
	{
		bonus.mage_dmg = 0;
	}
	else
	{
		bonus.mage_dmg = std::min(num, 1000);
	}
	show_activ_edit(ch);
}

void sedit::parse_activ_edit(CHAR_DATA *ch, const char *arg)
{
	auto i = olc_set.activ_list.find(activ_edit);
	if (i == olc_set.activ_list.end())
	{
		send_to_char(ch,
			"������: ��������� �� ������ %s:%d (%s).\r\n",
			__FILE__, __LINE__, __func__);
		show_main(ch);
		return;
	}
	const activ_node &activ = i->second;

	skip_spaces(&arg);
	if (!*arg)
	{
		send_to_char("�������� �����!\r\n", ch);
		show_activ_edit(ch);
		return;
	}
	if (!isdigit(*arg))
	{
		switch(*arg)
		{
		case 'Q':
		case 'q':
		case '�':
		case '�':
			show_main(ch);
			break;
		default:
			send_to_char("�������� �����!\r\n", ch);
			show_activ_edit(ch);
			break;
		}
		return;
	}

	unsigned num = atoi(arg);
	switch (num)
	{
	case 1:
		state = STATE_ACTIV_REMOVE;
		send_to_char("����������� �������� ���������� Y(�)/N(�) :", ch);
		return;
	case 2:
		state = STATE_ACTIV_CHANGE;
		send_to_char("������� ����� ���-�� ��������� ��� ��������� :", ch);
		return;
	case 3:
		show_activ_prof(ch);
		return;
	case 4:
		show_activ_affects(ch);
		return;
	default:
		break;
	}
	const unsigned offset = 4;

	if (num > offset && num <= offset + activ.apply.size())
	{
		show_activ_apply(ch);
		apply_edit = num - offset - 1;
	}
	else if (num == offset + activ.apply.size() + 1)
	{
		show_activ_skill(ch);
	}
	else if (num == offset + activ.apply.size() + 2)
	{
		show_activ_ench_vnum(ch);
	}
	else if (num == offset + activ.apply.size() + 3)
	{
		state = STATE_ACTIV_PHYS_DMG;
		send_to_char("������� ������� ������������� ���. ����� :", ch);
	}
	else if (num == offset + activ.apply.size() + 4)
	{
		state = STATE_ACTIV_MAGE_DMG;
		send_to_char("������� ������� ������������� ���. ����� :", ch);
	}
	else if (num == offset + activ.apply.size() + 5)
	{
		show_main(ch);
	}
	else
	{
		send_to_char("������������ ����.\r\n", ch);
		show_activ_edit(ch);
	}
}

void sedit::parse_activ_affects(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	int bit = 0, plane = 0;
	int num = planebit(arg, &plane, &bit);

	if (num < 0)
	{
		show_activ_affects(ch);
	}
	else if (num == 0)
	{
		show_activ_edit(ch);
	}
	else
	{
		TOGGLE_BIT(olc_set.activ_list.at(activ_edit).affects.flags[plane],
			1 << (bit));
		show_activ_affects(ch);
		return;
	}
}

void sedit::parse_activ_apply_loc(CHAR_DATA *ch, const char *arg)
{
	obj_affected_type &apply =
		olc_set.activ_list.at(activ_edit).apply.at(apply_edit);

	skip_spaces(&arg);
	int num = atoi(arg);

	if (num == 0)
	{
		apply.location = 0;
		apply.modifier = 0;
		show_activ_edit(ch);
	}
	else if (num < 0 || num >= NUM_APPLIES)
	{
		send_to_char("������������ ����.\r\n", ch);
		show_apply_olc(ch->desc);
	}
	else
	{
		apply.location = num;
		state = STATE_ACTIV_APPLY_MOD;
		send_to_char("������� ����������� : ", ch);
	}
}

void sedit::parse_activ_apply_mod(CHAR_DATA *ch, const char *arg)
{
	obj_affected_type &apply =
		olc_set.activ_list.at(activ_edit).apply.at(apply_edit);

	skip_spaces(&arg);
	int num = atoi(arg);

	if (num == 0)
	{
		apply.location = 0;
		apply.modifier = 0;
		show_activ_edit(ch);
	}
	else
	{
		apply.modifier = num;
		show_activ_edit(ch);
	}
}

void sedit::parse_obj_change(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg || !isdigit(*arg))
	{
		send_to_char("������������ ����������� ����� ��������.\r\n", ch);
		show_obj_edit(ch);
		return;
	}

	const int vnum = atoi(arg);
	if (vnum == obj_edit)
	{
		send_to_char("�� � ��� ������������ ������ �������.\r\n", ch);
		show_obj_edit(ch);
		return;
	}

	const int rnum = real_object(vnum);
	if (rnum < 0)
	{
		send_to_char(ch, "��������� � vnum %d �� ����������.\r\n", vnum);
	}
	else if (is_duplicate(olc_set.uid, vnum))
	{
		send_to_char(ch,
			"������� '%s' ��� �������� ������ ������� ������.\r\n",
			obj_proto[rnum]->short_description);
	}
	else if (olc_set.obj_list.find(vnum) != olc_set.obj_list.end())
	{
		send_to_char(ch,
			"������� '%s' ��� �������� ������ ������� ������.\r\n",
			obj_proto[rnum]->short_description);
	}
	else
	{
		msg_node msg;
		auto i = olc_set.obj_list.find(obj_edit);
		if (i != olc_set.obj_list.end())
		{
			msg = i->second;
			olc_set.obj_list.erase(i);
		}
		obj_edit = vnum;
		// GCC 4.4
		//olc_set.obj_list.emplace(vnum, msg);
		olc_set.obj_list.insert(std::make_pair(vnum, msg));
		send_to_char(ch,
			"������� '%s' �������� � �����.\r\n",
			obj_proto[rnum]->short_description);
	}
	show_obj_edit(ch);
}

void sedit::parse_activ_change(CHAR_DATA *ch, const char *arg)
{
	skip_spaces(&arg);
	if (!*arg || !isdigit(*arg))
	{
		send_to_char("������������ ���-�� ��������� ��� ���������.\r\n", ch);
		show_activ_edit(ch);
		return;
	}

	const int num = atoi(arg);
	if (num == activ_edit)
	{
		send_to_char("���-�� ��������� ��������� �� ����������.\r\n", ch);
	}
	else if (olc_set.activ_list.find(num) != olc_set.activ_list.end())
	{
		send_to_char(ch,
			"����� ��� �������� ��������� �� %d %s.\r\n",
			num, desc_count(num, WHAT_OBJECT));
	}
	else
	{
		activ_node activ;
		auto i = olc_set.activ_list.find(activ_edit);
		if (i != olc_set.activ_list.end())
		{
			activ = i->second;
			olc_set.activ_list.erase(i);
		}
		activ_edit = num;
		// GCC 4.4
		//olc_set.activ_list.emplace(num, activ);
		olc_set.activ_list.insert(std::make_pair(num, activ));
		send_to_char(ch, "��������� �� %d %s �������� � �����.\r\n",
			num, desc_count(num, WHAT_OBJECT));
	}
	show_activ_edit(ch);
}

/// ������ ��� ��������� ������ � ��� ������ ������ ��� �������
/// ���� ��� �������� � try/catch ��� ����� ��������� out_of_range, �.�. ������
/// ��� ����� .at � ������� ��� �� ��� ������������� ������ ������ ���������
void parse_input(CHAR_DATA *ch, const char *arg)
{
	sedit &olc = *(ch->desc->sedit);

	switch(olc.state)
	{
	case STATE_MAIN:
		olc.parse_main(ch, arg);
		break;
	case STATE_MAIN_EXIT:
		parse_main_exit(ch, arg);
		break;
	case STATE_SET_REMOVE:
		parse_set_remove(ch, arg);
		break;
	case STATE_NAME:
		olc.parse_setname(ch, arg);
		break;
	case STATE_ALIAS:
		olc.parse_setalias(ch, arg);
		break;
	case STATE_COMMENT:
		olc.parse_setcomment(ch, arg);
		break;
	case STATE_SETMSG_CHAR_ON:
	case STATE_SETMSG_CHAR_OFF:
	case STATE_SETMSG_ROOM_ON:
	case STATE_SETMSG_ROOM_OFF:
	case STATE_OBJMSG_CHAR_ON:
	case STATE_OBJMSG_CHAR_OFF:
	case STATE_OBJMSG_ROOM_ON:
	case STATE_OBJMSG_ROOM_OFF:
	case STATE_GLBMSG_CHAR_ON:
	case STATE_GLBMSG_CHAR_OFF:
	case STATE_GLBMSG_ROOM_ON:
	case STATE_GLBMSG_ROOM_OFF:
		olc.parse_setmsg(ch, arg);
		break;
	case STATE_OBJ_ADD:
		olc.parse_obj_add(ch, arg);
		break;
	case STATE_OBJ_EDIT:
		olc.parse_obj_edit(ch, arg);
		break;
	case STATE_OBJ_CHANGE:
		olc.parse_obj_change(ch, arg);
		break;
	case STATE_OBJ_REMOVE:
		olc.parse_obj_remove(ch, arg);
		break;
	case STATE_TOTAL_ACTIV:
		olc.show_main(ch);
		break;
	case STATE_ACTIV_ADD:
		olc.parse_activ_add(ch, arg);
		break;
	case STATE_ACTIV_EDIT:
		olc.parse_activ_edit(ch, arg);
		break;
	case STATE_ACTIV_AFFECTS:
		olc.parse_activ_affects(ch, arg);
		break;
	case STATE_ACTIV_REMOVE:
		olc.parse_activ_remove(ch, arg);
		break;
	case STATE_ACTIV_PROF:
		olc.parse_activ_prof(ch, arg);
		break;
	case STATE_ACTIV_CHANGE:
		olc.parse_activ_change(ch, arg);
		break;
	case STATE_ACTIV_APPLY_LOC:
		olc.parse_activ_apply_loc(ch, arg);
		break;
	case STATE_ACTIV_APPLY_MOD:
		olc.parse_activ_apply_mod(ch, arg);
		break;
	case STATE_ACTIV_SKILL:
		olc.parse_activ_skill(ch, arg);
		break;
	case STATE_ACTIV_ENCH_VNUM:
		olc.parse_activ_ench_vnum(ch, arg);
		break;
	case STATE_ACTIV_ENCH_NDICE:
		olc.parse_activ_ench_ndice(ch, arg);
		break;
	case STATE_ACTIV_ENCH_SDICE:
		olc.parse_activ_ench_sdice(ch, arg);
		break;
	case STATE_ACTIV_ENCH_WEIGHT:
		olc.parse_activ_weight_val(ch, arg);
		break;
	case STATE_ACTIV_PHYS_DMG:
		olc.parse_activ_phys_dmg(ch, arg);
		break;
	case STATE_ACTIV_MAGE_DMG:
		olc.parse_activ_mage_dmg(ch, arg);
		break;
	case STATE_GLOBAL_MSG:
		olc.parse_global_msg(ch, arg);
		break;
	case STATE_GLOBAL_MSG_EXIT:
		parse_global_msg_exit(ch, arg);
		break;
	default:
		ch->desc->sedit.reset();
		STATE(ch->desc) = CON_PLAYING;
		send_to_char(ch,
			"������: �� ������ STATE, �������������� �������� %s:%d (%s).\r\n",
			__FILE__, __LINE__, __func__);
		break;
	} // switch
}

} // namespace obj_sets_olc

using namespace obj_sets_olc;

namespace
{

void start_sedit(CHAR_DATA *ch, size_t idx)
{
	STATE(ch->desc) = CON_SEDIT;
	ch->desc->sedit = boost::make_shared<obj_sets_olc::sedit>();
	if (idx == static_cast<size_t>(-1))
	{
		ch->desc->sedit->new_entry = true;
	}
	else
	{
		ch->desc->sedit->olc_set = *(sets_list.at(idx));
	}
	ch->desc->sedit->show_main(ch);
}

const char *SEDIT_HELP =
	"������ �������:\r\n"
	"   sedit - �������� ������ ����\r\n"
	"   sedit <��������� ����� ���� �� slist> - �������������� ������������� ����\r\n"
	"   sedit <vnum ������ �������� �� ����> - �������������� ������������� ����\r\n"
	"   sedit <msg|messages> - �������������� ���������� ��������� �����\r\n";

} // namespace

/// ������� sedit, ��. SEDIT_HELP
ACMD(do_sedit)
{
	if (IS_NPC(ch))
	{
		return;
	}

	skip_spaces(&argument);
	if (!argument || !*argument)
	{
		// �������� ������ ����
		start_sedit(ch, -1);
	}
	else if (isdigit(*argument))
	{
		// �������������� �������������
		unsigned num = atoi(argument);
		if (num > 0 && num <= sets_list.size())
		{
			// �� ������ ���� �� slist
			--num;
			start_sedit(ch, num);
		}
		else
		{
			// �� ����� ��������
			if (real_object(num) < 0)
			{
				send_to_char(SEDIT_HELP, ch);
				send_to_char(ch, "��������� � vnum %s �� ����������.\r\n",
					argument);
				return;
			}
			size_t idx = setidx_by_objvnum(num);
			if (idx < sets_list.size())
			{
				start_sedit(ch, idx);
			}
			else
			{
				send_to_char(SEDIT_HELP, ch);
				send_to_char(ch,
					"� ����� ��������� � vnum %s �� �������.\r\n", argument);
			}
		}
	}
	else if (!str_cmp(argument, "msg") || !str_cmp(argument, "messages"))
	{
		// �������������� ���������� ���������
		STATE(ch->desc) = CON_SEDIT;
		ch->desc->sedit = boost::make_shared<obj_sets_olc::sedit>();
		ch->desc->sedit->msg_edit = global_msg;
		ch->desc->sedit->show_global_msg(ch);
	}
	else
	{
		send_to_char(SEDIT_HELP, ch);
	}
}
