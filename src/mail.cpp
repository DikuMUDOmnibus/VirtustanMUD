/* ************************************************************************
*   File: mail.cpp                                      Part of Bylins    *
*  Usage: Internal funcs and player spec-procs of mud-mail system         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include <string>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "pugixml.hpp"

#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "handler.h"
#include "mail.h"
#include "char.hpp"
#include "parcel.hpp"
#include "char_player.hpp"
#include "named_stuff.hpp"
#include "parse.hpp"
#include "screen.h"

extern room_rnum r_helled_start_room;
extern room_rnum r_named_start_room;
extern room_rnum r_unreg_start_room;

void postmaster_send_mail(CHAR_DATA * ch, CHAR_DATA * mailman, int cmd, char *arg);
void postmaster_check_mail(CHAR_DATA * ch, CHAR_DATA * mailman, int cmd, char *arg);
void postmaster_receive_mail(CHAR_DATA * ch, CHAR_DATA * mailman, int cmd, char *arg);
SPECIAL(postmaster);

namespace mail
{

bool check_poster_cnt(CHAR_DATA* ch);

/// ��� undelivered_list
struct undelivered_node
{
	undelivered_node(const char *name, int to_uid) : total_num(1), names(name)
	{
		uids.insert(to_uid);
	};
	/// ���-�� �������������� �����
	size_t total_num;
	/// ������ � ������� ���������
	std::string names;
	/// ���� ��������� ��� ������������ ����
	std::unordered_set<int> uids;
};
/// ��� �����������, ���� � �������������� ������� �� ������ ������
std::unordered_map<int, undelivered_node> undelivered_list;

void add_undelivered(int from_uid, const char *name, int to_uid)
{
	if (from_uid < 0 || !name) return;

	auto i = undelivered_list.find(from_uid);
	if (i != undelivered_list.end())
	{
		i->second.total_num += 1;
		// �������� �� ������������ � ���������� ����� � ������
		if (i->second.uids.find(to_uid) == i->second.uids.end())
		{
			i->second.names += " ";
			i->second.names += name;
			if (!(i->second.total_num % 8))
			{
				i->second.names += "\r\n";
			}
			i->second.uids.insert(to_uid);
		}
	}
	else
	{
		undelivered_node tmp(name, to_uid);
		undelivered_list.insert(std::make_pair(from_uid, tmp));
	}
}

void print_undelivered(CHAR_DATA *ch)
{
	auto i = undelivered_list.find(ch->get_uid());
	if (i != undelivered_list.end())
	{
		std::string out(
			"���������� �� �������������� (�� ������ ������������) �������.\r\n"
			"���������� �����: ");
		out += boost::lexical_cast<std::string>(i->second.total_num);
		out += ", ��������:\r\n " + i->second.names + "\r\n";
		send_to_char(out, ch);
	}
}

} // namespace mail

using namespace mail;

// ���.������� ��� �������� �����
const int MIN_MAIL_LEVEL = 2;
// ��������� �������� ������
const int STAMP_PRICE = 50;
// ����. ������ ���������
const int MAX_MAIL_SIZE = 4096;

//****************************************************************
//* Below is the spec_proc for a postmaster using the above      *
//* routines.  Written by Jeremy Elson (jelson@circlemud.org)    *
//****************************************************************
SPECIAL(postmaster)
{
	if (!ch->desc || IS_NPC(ch))
		return (0);	// so mobs don't get caught here

	if (!(CMD_IS("mail") || CMD_IS("check") || CMD_IS("receive")
			|| CMD_IS("�����") || CMD_IS("��������") || CMD_IS("���������")
			|| CMD_IS("return") || CMD_IS("�������")))
		return (0);

	if (CMD_IS("mail") || CMD_IS("���������"))
	{
		postmaster_send_mail(ch, (CHAR_DATA *) me, cmd, argument);
		return (1);
	}
	else if (CMD_IS("check") || CMD_IS("�����"))
	{
		postmaster_check_mail(ch, (CHAR_DATA *) me, cmd, argument);
		return (1);
	}
	else if (CMD_IS("receive") || CMD_IS("��������"))
	{
		one_argument(argument, arg);
		if(is_abbrev(arg, "����")) {
			NamedStuff::receive_items(ch, (CHAR_DATA *) me);
		} else {
			postmaster_receive_mail(ch, (CHAR_DATA *) me, cmd, argument);
		}
		return (1);
	}
	else if (CMD_IS("return") || CMD_IS("�������"))
	{
		Parcel::bring_back(ch, (CHAR_DATA *) me);
		return (1);
	}
	else
		return (0);
}

void postmaster_check_mail(CHAR_DATA * ch, CHAR_DATA * mailman, int cmd, char *arg)
{
	bool empty = true;
	if (mail::has_mail(ch->get_uid()))
	{
		empty = false;
		act("$n ������$g ��� : '��� ������� �����.'", FALSE, mailman, 0, ch, TO_VICT);
	}
	if (Parcel::has_parcel(ch))
	{
		empty = false;
		act("$n ������$g ��� : '��� ������� �������.'", FALSE, mailman, 0, ch, TO_VICT);
	}

	if (empty)
	{
		act("$n ������$g ��� : '������, ������� ��� ������ ���.'",
			FALSE, mailman, 0, ch, TO_VICT);
	}
	Parcel::print_sending_stuff(ch);
	print_undelivered(ch);
}


void postmaster_receive_mail(CHAR_DATA* ch, CHAR_DATA* mailman, int cmd, char* arg)
{
	if (!Parcel::has_parcel(ch) && !mail::has_mail(ch->get_uid()))
	{
		act("$n ��������� ������$g ��� : '�� ��� ��� ��� �����!?'",
			FALSE, mailman, 0, ch, TO_VICT);
		return;
	}
	Parcel::receive(ch, mailman);
	mail::receive(ch, mailman);
}

void postmaster_send_mail(CHAR_DATA * ch, CHAR_DATA * mailman, int cmd, char *arg)
{
	int recipient;
	int cost;
	char buf[256], **write;

	IS_IMMORTAL(ch) || PRF_FLAGGED(ch, PRF_CODERINFO) ? cost = 0 : cost = STAMP_PRICE;

	if (GET_LEVEL(ch) < MIN_MAIL_LEVEL)
	{
		sprintf(buf,
			"$n ������$g ���, '��������, �� ������ ���������� %d ������, ����� ��������� ������!'",
			MIN_MAIL_LEVEL);
		act(buf, FALSE, mailman, 0, ch, TO_VICT);
		return;
	}
	if (!mail::check_poster_cnt(ch))
	{
		act("$n ������$g ���, '��������, �� ��� ��������� ������������ ���-�� ���������!'",
			FALSE, mailman, 0, ch, TO_VICT);
		return;
	}

	arg = one_argument(arg, buf);

	if (!*buf)  		// you'll get no argument from me!
	{
		act("$n ������$g ���, '�� �� ������� ��������!'", FALSE, mailman, 0, ch, TO_VICT);
		return;
	}
	if ((recipient = GetUniqueByName(buf)) <= 0)
	{
		act("$n ������$g ���, '��������, �� ������ ������ ��� � ����!'", FALSE, mailman, 0, ch, TO_VICT);
		return;
	}

	skip_spaces(&arg);
	if (*arg)
	{
		if ((IN_ROOM(ch) == r_helled_start_room) ||
			(IN_ROOM(ch) == r_named_start_room) ||
			(IN_ROOM(ch) == r_unreg_start_room))
		{
			act("$n ������$g ��� : '�������? �� ��������!'", FALSE, mailman, 0, ch, TO_VICT);
			return;
		}
		long vict_uid = GetUniqueByName(buf);
		if (vict_uid > 0)
		{
			Parcel::send(ch, mailman, vict_uid, arg);
		}
		else
		{
			act("$n ������$g ��� : '�������� �����, �������� �����!'", FALSE, mailman, 0, ch, TO_VICT);
		}
		return;
	}

	if (ch->get_gold() < cost)
	{
		sprintf(buf, "$n ������$g ���, '������ ����� %d %s.'\r\n"
				"$n ������$g ���, '...������� � ��� ������-�������� ���.'",
				STAMP_PRICE, desc_count(STAMP_PRICE, WHAT_MONEYu));
		act(buf, FALSE, mailman, 0, ch, TO_VICT);
		return;
	}

	act("$n �����$g ������ ������.", TRUE, ch, 0, 0, TO_ROOM);
	if (cost == 0)
		sprintf(buf, "$n ������$g ���, '�� ����� - �������� ���� �� �����.'\r\n"
				"$n ������$g ���, '������ ������, (/s saves /h for help)'");
	else
		sprintf(buf,
				"$n ������$g ���, '�������, � ��� %d %s ��������� �����.'\r\n"
				"$n ������$g ���, '������ ������, (/s saves /h for help)'",
				STAMP_PRICE, desc_count(STAMP_PRICE, WHAT_MONEYa));

	act(buf, FALSE, mailman, 0, ch, TO_VICT);
	ch->remove_gold(cost);
	SET_BIT(PLR_FLAGS(ch, PLR_MAILING), PLR_MAILING);	// string_write() sets writing.

	// Start writing!
	CREATE(write, char *, 1);
	string_write(ch->desc, write, MAX_MAIL_SIZE, recipient, NULL);
}

namespace coder
{

static const std::string base64_chars =
	"abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"0123456789+/";

static inline bool is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(const char* in_str, size_t in_len)
{
	const unsigned char* bytes_to_encode =
		reinterpret_cast<const unsigned char*>(in_str);
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while((i++ < 3))
			ret += '=';

	}

	return ret;

}

std::string base64_decode(std::string const& encoded_string)
{
	size_t in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
	{
		char_array_4[i++] = encoded_string[in_];
		in_++;
		if (i ==4)
		{
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

std::string to_iso8601(time_t time)
{
	char buf_[MAX_INPUT_LENGTH];
	strftime(buf_, sizeof(buf_), "%FT%T", localtime(&time));
	return buf_;
}

time_t from_iso8601(const std::string& str)
{
	std::tm tm_time;
	int parsed = sscanf(str.c_str(), "%d-%d-%dT%d:%d:%d",
		&tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
		&tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec);

	time_t time = 0;
	if (parsed == 6)
	{
		tm_time.tm_mon -= 1;
		tm_time.tm_year -= 1900;
		tm_time.tm_isdst = -1;
		time = std::mktime(&tm_time);
	}
	return time;
}

} // namespace coder

namespace mail
{

const char* MAIL_XML_FILE = LIB_ETC"plrmail.xml";
// ���� ��������� � �������� �� ����������� ����� �� NAME_LEVEL ������
const int LOW_LVL_MAX_POST = 10;
// ���� ����� ��� ���� ���������
const int HIGH_LVL_MAX_POST = 100;

struct mail_node
{
	mail_node() : from(-1), date(0) {};

	// ��� ������ (����� ���� ����������)
	int from;
	// ���� ��������� ������
	time_t date;
	// ����� ������ � ������������ base64
	std::string text;
	// (���� � iso8601 + ����� + ����������) � ������������ base64
	std::string header;
};

// ���� ��� ��������� ������ ��������������
bool need_save = false;
// ��� ���������� -> ������
std::unordered_multimap<int, mail_node> mail_list;
// ��� -> ���-�� ������������ ��������� � �� (����-��������)
std::unordered_map<int, int> poster_list;
// ��� - �������� ������ ����� (� ���������� ���������), ������� �����
// ����������� ������� ����������� ��� ����� ����� (����-��������)
std::unordered_set<int> notice_list;

void add_notice(int uid)
{
	notice_list.insert(uid);
}

void print_notices()
{
	for(auto i = notice_list.begin(); i != notice_list.end(); ++i)
	{
		if (!has_mail(*i))
		{
			continue;
		}
		DESCRIPTOR_DATA* d = DescByUID(*i);
		if (d)
		{
			send_to_char(d->character,
				"%s��� ������ ������, ������� �� ����� � �����������!%s\r\n",
				CCWHT(d->character, C_NRM), CCNRM(d->character, C_NRM));
		}
	}
	notice_list.clear();
}

void add_poster(int uid)
{
	if (uid < 0) return;

	auto i = poster_list.find(uid);
	if (i != poster_list.end())
	{
		i->second += 1;
	}
	else
	{
		poster_list[uid] = 1;
	}
}

void sub_poster(int uid)
{
	if (uid < 0) return;

	auto i = poster_list.find(uid);
	if (i != poster_list.end())
	{
		i->second -= 1;
		if (i->second <= 0)
		{
			poster_list.erase(i);
		}
	}
}

bool check_poster_cnt(CHAR_DATA* ch)
{
	auto i = poster_list.find(ch->get_uid());
	if (i != poster_list.end())
	{
		if (GET_REMORT(ch) <= 0
			&& GET_LEVEL(ch) <= NAME_LEVEL
			&& i->second >= LOW_LVL_MAX_POST)
		{
				return false;
		}
		if (i->second >= HIGH_LVL_MAX_POST)
		{
			return false;
		}
	}
	return true;
}

void add(int to_uid, int from_uid, char* message)
{
	if (to_uid < 0 || !message || !*message)
	{
		return;
	}

	mail_node node;
	node.from = from_uid;
	node.date = time(0);
	node.text = coder::base64_encode(message, strlen(message));

	char buf_[MAX_STRING_LENGTH];
	snprintf(buf_, sizeof(buf_), "%s %d %d",
		coder::to_iso8601(node.date).c_str(), from_uid, to_uid);
	node.header = coder::base64_encode(buf_, strlen(buf_));

	mail_list.insert(std::make_pair(to_uid, node));
	add_poster(from_uid);
	need_save = true;
}

void add_by_id(int to_id, int from_id, char* message)
{
	const int to_uid = get_uid_by_id(to_id);
	const int from_uid = from_id >= 0 ? get_uid_by_id(from_id) : from_id;

	add(to_uid, from_uid, message);
}

bool has_mail(int uid)
{
	return (mail_list.find(uid) != mail_list.end());
}

std::string get_author_name(int uid)
{
	std::string out;
	if (uid == -1)
	{
		out = "�������� ������";
	}
	else if (uid == -2)
	{
		out = "<�������� ��� ������>";
	}
	else if (uid < 0)
	{
		out = "����������";
	}
	else
	{
		const char* name = get_name_by_unique(uid);
		if (name)
		{
			out = name;
			name_convert(out);
		}
		else
		{
			out = "<�������� ��� ������>";
		}
	}
	return out;
}

void receive(CHAR_DATA* ch, CHAR_DATA* mailman)
{
	auto rng = mail_list.equal_range(ch->get_uid());
	for(auto i = rng.first; i != rng.second; ++i)
	{
		OBJ_DATA *obj = create_obj();
		obj->aliases = str_dup("mail paper letter ������ ����� ������");
		obj->short_description = str_dup("������");
		obj->description = str_dup("���-�� ����� ����� ���� ������.");
		obj->PNames[0] = str_dup("������");
		obj->PNames[1] = str_dup("������");
		obj->PNames[2] = str_dup("������");
		obj->PNames[3] = str_dup("������");
		obj->PNames[4] = str_dup("�������");
		obj->PNames[5] = str_dup("������");

		GET_OBJ_TYPE(obj) = ITEM_NOTE;
		GET_OBJ_WEAR(obj) = ITEM_WEAR_TAKE | ITEM_WEAR_HOLD;
		GET_OBJ_WEIGHT(obj) = 1;
		GET_OBJ_MATER(obj) = MAT_PAPER;
		obj->set_cost(0);
		obj->set_rent(10);
		obj->set_rent_eq(10);
		obj->set_timer(24 * 60);

		char buf_date[MAX_INPUT_LENGTH];
		strftime(buf_date, sizeof(buf_date),
			"%H:%M %d-%m-%Y", localtime(&i->second.date));

		char buf_[MAX_INPUT_LENGTH];
		snprintf(buf_, sizeof(buf_),
			" * * * * ��������� ����� * * * *\r\n"
			"����: %s\r\n"
			"����: %s\r\n"
			"  ��: %s\r\n\r\n",
			buf_date, ch->get_name(), get_author_name(i->second.from).c_str());

		std::string text = coder::base64_decode(i->second.text);
		boost::trim_if(text, ::isspace);
		obj->action_description = str_dup((buf_ + text + "\r\n\r\n").c_str());

		obj_to_char(obj, ch);
		act("$n ���$g ��� ������.", FALSE, mailman, 0, ch, TO_VICT);
		act("$N ���$G $n2 ������.", FALSE, ch, 0, mailman, TO_ROOM);

		sub_poster(i->second.from);
	}
	mail_list.erase(rng.first, rng.second);
	need_save = true;
}

void save()
{
	print_notices();

	if (!need_save) return;

	pugi::xml_document doc;
	doc.append_child().set_name("mail");
	pugi::xml_node mail_n = doc.child("mail");

	for (auto i = mail_list.cbegin(), iend = mail_list.cend(); i != iend; ++i)
	{
		pugi::xml_node msg_n = mail_n.append_child();
		msg_n.set_name("m");
		msg_n.append_attribute("h") = i->second.header.c_str();
		msg_n.append_attribute("t") = i->second.text.c_str();
	}

	doc.save_file(MAIL_XML_FILE);
	need_save = false;
}

void load()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(MAIL_XML_FILE);
	if (!result)
	{
		snprintf(buf, MAX_STRING_LENGTH, "...%s", result.description());
		mudlog(buf, CMP, LVL_IMMORT, SYSLOG, TRUE);
		return;
	}
    pugi::xml_node mail_n = doc.child("mail");
    if (!mail_n)
    {
		snprintf(buf, MAX_STRING_LENGTH, "...<mail> read fail");
		mudlog(buf, CMP, LVL_IMMORT, SYSLOG, TRUE);
		return;
    }

	for (pugi::xml_node msg_n = mail_n.child("m");
		msg_n; msg_n = msg_n.next_sibling("m"))
	{
		mail_node message;
		message.header = Parse::attr_str(msg_n, "h");
		message.text = Parse::attr_str(msg_n, "t");
		// ����������� ������ ������
		std::string header = coder::base64_decode(message.header);
		int to_uid = -1;
		// ����������� ����, ����������� � ����������
		std::vector<std::string> param_list;
		boost::split(param_list, header, boost::is_any_of(" "));
		if (param_list.size() == 3)
		{
			message.date = coder::from_iso8601(param_list[0]);
			try
			{
				message.from = boost::lexical_cast<int>(param_list[1]);
				to_uid = boost::lexical_cast<int>(param_list[2]);
			}
			catch (boost::bad_lexical_cast&)
			{
				log("SYSERROR: ������ ������ mail header #1 (%s)",
					header.c_str());
				continue;
			}
		}
		else
		{
			log("SYSERROR: ������ ������ mail header #2 (%s)",
				header.c_str());
			continue;
		}
		const char *to_name = get_name_by_unique(to_uid);
		// ���������, ���� ���������� � ������
		if (!to_name)
		{
			// �������� ������ ���
			continue;
		}
		if (message.from == -1 && time(0) - message.date > 60 * 60 * 24 * 365)
		{
			// ����������� ��������� ������ ����
			continue;
		}
		if (message.from > 0 && !get_name_by_unique(message.from))
		{
			// ������� ����� ����, ����� ����� � ���-���� ������ �� �������
			message.from = -2;
		}
		mail_list.insert(std::make_pair(to_uid, message));
		add_poster(message.from);
		add_undelivered(message.from, to_name, to_uid);
	}
	need_save = true;
}

size_t get_msg_count()
{
	return mail_list.size();
}

} // namespace mail