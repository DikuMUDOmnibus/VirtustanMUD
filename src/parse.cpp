// Copyright (c) 2013 Krodo
// Part of Bylins http://www.mud.ru

#include "conf.h"
#include <unordered_map>
#include <array>
#include <boost/lexical_cast.hpp>

#include "parse.hpp"
#include "db.h"
#include "obj.hpp"

namespace TextId
{

///
/// �������� ������ ������������ �����=������/������=����� ��� ��������
/// ������-������� �������� � ��������� �� � �������.
/// ����� ��� ������������� ������ �� ��������� ������� �������� � ����.
///
class TextIdNode
{
public:
	void add(int num, std::string str);

	std::string to_str(int num) const;
	int to_num(const std::string &str) const;

private:
	std::unordered_map<int, std::string> num_to_str;
	std::unordered_map<std::string, int> str_to_num;
};

// ����� ������ �������������� ��������
std::array<TextIdNode, TEXT_ID_COUNT> text_id_list;

///
/// ���� ��������� �� ������� ��� �������.
///
void init_char_class()
{
	TextIdNode &tmp = text_id_list.at(CHAR_CLASS);
	tmp.add(CLASS_CLERIC, "CLASS_CLERIC");
	tmp.add(CLASS_BATTLEMAGE, "CLASS_BATTLEMAGE");
	tmp.add(CLASS_THIEF, "CLASS_THIEF");
	tmp.add(CLASS_WARRIOR, "CLASS_WARRIOR");
	tmp.add(CLASS_ASSASINE, "CLASS_ASSASINE");
	tmp.add(CLASS_GUARD, "CLASS_GUARD");
	tmp.add(CLASS_CHARMMAGE, "CLASS_CHARMMAGE");
	tmp.add(CLASS_DEFENDERMAGE, "CLASS_DEFENDERMAGE");
	tmp.add(CLASS_NECROMANCER, "CLASS_NECROMANCER");
	tmp.add(CLASS_PALADINE, "CLASS_PALADINE");
	tmp.add(CLASS_RANGER, "CLASS_RANGER");
	tmp.add(CLASS_SMITH, "CLASS_SMITH");
	tmp.add(CLASS_MERCHANT, "CLASS_MERCHANT");
	tmp.add(CLASS_DRUID, "CLASS_DRUID");
}

///
/// ���� ��������� �� ���������� ��������� ��� ���������� � ����.
///
void init_obj_vals()
{
	TextIdNode &tmp = text_id_list.at(OBJ_VALS);
	tmp.add(ObjVal::POTION_SPELL1_NUM, "POTION_SPELL1_NUM");
	tmp.add(ObjVal::POTION_SPELL1_LVL, "POTION_SPELL1_LVL");
	tmp.add(ObjVal::POTION_SPELL2_NUM, "POTION_SPELL2_NUM");
	tmp.add(ObjVal::POTION_SPELL2_LVL, "POTION_SPELL2_LVL");
	tmp.add(ObjVal::POTION_SPELL3_NUM, "POTION_SPELL3_NUM");
	tmp.add(ObjVal::POTION_SPELL3_LVL, "POTION_SPELL3_LVL");
	tmp.add(ObjVal::POTION_PROTO_VNUM, "POTION_PROTO_VNUM");
}

///
/// ����� ���� ������� ��������� ��, ��������� ��� ������ ����.
///
void init()
{
	/// CHAR_CLASS
	init_char_class();
	/// OBJ_VALS
	init_obj_vals();
	/// ...
}

///
/// ��������������� ���������� �� ��������� � �� �������� � ����.
/// \return �������� ��������� ��� -1, ���� ������ �� ���� �������
///
int to_num(IdType type, const std::string &str)
{
	if (type < TEXT_ID_COUNT)
	{
		return text_id_list.at(type).to_num(str);
	}
	return -1;
}

///
/// ��������������� �������� ��������� � �� ��������� ��.
/// \return ��������� �� ��������� ��� ������ ������, ���� ������ �� ���� �������
///
std::string to_str(IdType type, int num)
{
	if (type < TEXT_ID_COUNT)
	{
		return text_id_list.at(type).to_str(num);
	}
	return "";
}

///
/// ���������� ������������ ��������=���������/���������=��������
///
void TextIdNode::add(int num, std::string str)
{
	num_to_str.insert(std::make_pair(num, str));
	str_to_num.insert(std::make_pair(str, num));
}

///
/// ��������������� �������� -> ��������� ��
///
std::string TextIdNode::to_str(int num) const
{
	auto i = num_to_str.find(num);
	if (i != num_to_str.end())
	{
		return i->second;
	}
	return "";
}

///
/// ��������������� ��������� �� -> ��������
///
int TextIdNode::to_num(const std::string &str) const
{
	auto i = str_to_num.find(str);
	if (i != str_to_num.end())
	{
		return i->second;
	}
	return -1;
}

} // namespace TextId

namespace Parse
{

///
/// ������� ��������������� \param text � <int> � ���������� ����������
/// \return ����� ��� -1, � ������ �������
///
int cast_to_int(const char *text)
{
	int result = -1;

	try
	{
		result = boost::lexical_cast<int>(text);
	}
	catch(...)
	{
		snprintf(buf, MAX_STRING_LENGTH,
			"...lexical_cast<int> fail (value='%s')", text);
		mudlog(buf, CMP, LVL_IMMORT, SYSLOG, TRUE);
	}

	return result;
}

///
/// ������� �� pugixml ��� ������ ��������� ���������
/// � ����������� � ���- � ������
/// � ������� ��� �������� ��� <param value="1234" />
/// \return -1 � ������ �������
int attr_int(const pugi::xml_node &node, const char *text)
{
	pugi::xml_attribute attr = node.attribute(text);
	if (!attr)
	{
		snprintf(buf, MAX_STRING_LENGTH, "...%s read fail", text);
		mudlog(buf, CMP, LVL_IMMORT, SYSLOG, TRUE);
	}
	return cast_to_int(attr.value());
}

///
/// ����, ��� � attr_int, ������ ��� ������ child_value()
/// � ������� ��� �������� ��� <param>1234<param>
///
int child_value_int(const pugi::xml_node &node, const char *text)
{
	pugi::xml_node child_node = node.child(text);
	if (!child_node)
	{
		snprintf(buf, MAX_STRING_LENGTH, "...%s read fail", text);
		mudlog(buf, CMP, LVL_IMMORT, SYSLOG, TRUE);
	}
	return cast_to_int(child_node.child_value());
}

///
/// ������ attr_int, \return ������ �� ��������� ��� ������ ������
///
std::string attr_str(const pugi::xml_node &node, const char *text)
{
	pugi::xml_attribute attr = node.attribute(text);
	if (!attr)
	{
		snprintf(buf, MAX_STRING_LENGTH, "...%s read fail", text);
		mudlog(buf, CMP, LVL_IMMORT, SYSLOG, TRUE);
	}
	else
	{
		return attr.value();
	}
	return "";
}

///
/// ������ child_value_int, \return ������ �� ��������� ��� ������ ������
///
std::string child_value_str(const pugi::xml_node &node, const char *text)
{
	pugi::xml_node child_node = node.child(text);
	if (!child_node)
	{
		snprintf(buf, MAX_STRING_LENGTH, "...%s read fail", text);
		mudlog(buf, CMP, LVL_IMMORT, SYSLOG, TRUE);
	}
	else
	{
		return child_node.child_value();
	}
	return "";
}

///
/// �������� ���������� ����� ������� � ������������ ������ � ��� � ������
/// \return true - ���� ���� �������� ������� (����) � ������ ������
///
bool valid_obj_vnum(int vnum)
{
	if (real_object(vnum) < 0)
	{
		snprintf(buf, sizeof(buf), "...bad obj vnum (%d)", vnum);
		mudlog(buf, CMP, LVL_IMMORT, SYSLOG, TRUE);
		return false;
	}

	return true;
}

} // namespace Parse

const char *mon_name[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "\n"
};

time_t parse_asctime(const std::string &str)
{
	char tmp[10], month[10];
	std::tm tm_time;
	tm_time.tm_mon = -1;

	int parsed = sscanf(str.c_str(), "%3s %3s %d %d:%d:%d %d",
		tmp, month,
		&tm_time.tm_mday,
		&tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec,
		&tm_time.tm_year);

	int i = 0;
	for (/**/; *mon_name[i] != '\n'; ++i)
	{
		if (!strcmp(month, mon_name[i]))
		{
			tm_time.tm_mon = i;
			break;
		}
	}

	time_t time = 0;
	if (tm_time.tm_mon != -1 && parsed == 7)
	{
		tm_time.tm_year -= 1900;
		tm_time.tm_isdst = -1;
		time = std::mktime(&tm_time);
	}
	return time;
}
