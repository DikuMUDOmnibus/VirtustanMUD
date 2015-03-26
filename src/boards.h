// Copyright (c) 2005 Krodo
// Part of Bylins http://www.mud.ru

#ifndef _BOARDS_H_
#define _BOARDS_H_

#include <string>
#include <vector>
#include <bitset>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "interpreter.h"

namespace Boards
{

// ��������� ���� ������� � ������
enum Access
{
	ACCESS_CAN_SEE,    // ����� � ������ �� '�����' � �������� ���������� � �����
	ACCESS_CAN_READ,   // ����� ������� � ������ ���������
	ACCESS_CAN_WRITE,  // ����� ������ ���������
	ACCESS_FULL,       // ����� ������� ����� ���������
	ACCESS_NUM         // ���-�� ������
};

// ���� �����
enum BoardTypes: int
{
	GENERAL_BOARD,    // �����
	NEWS_BOARD,       // �������
	IDEA_BOARD,       // ����
	ERROR_BOARD,      // ���� (������)
	GODNEWS_BOARD,    // ������� (������ ��� �����)
	GODGENERAL_BOARD, // ����� (������ ��� �����)
	GODBUILD_BOARD,   // ������� (������ ��� �����)
	GODCODE_BOARD,    // ������ (������ ��� �����)
	GODPUNISH_BOARD,  // ��������� (������ ��� �����)
	PERS_BOARD,       // ������������ (������ ��� �����)
	CLAN_BOARD,       // ��������
	CLANNEWS_BOARD,   // �������� �������
	NOTICE_BOARD,     // ������
	MISPRINT_BOARD,   // �������� (��������)
	SUGGEST_BOARD,    // �������� (�����)
	CODER_BOARD,      // ������� �� ����-����, ������������� �������
	TYPES_NUM         // ���-�� �����
};

extern const int GODGENERAL_BOARD_OBJ;
extern const int GENERAL_BOARD_OBJ;
extern const int GODCODE_BOARD_OBJ;
extern const int GODPUNISH_BOARD_OBJ;
extern const int GODBUILD_BOARD_OBJ;
extern const unsigned MAX_BOARD_MESSAGES;
extern const unsigned MAX_REPORT_MESSAGES;
extern const char *OVERFLOW_MESSAGE;

extern std::string dg_script_text;

bool can_see(CHAR_DATA *ch, const Board &board);
bool can_read(CHAR_DATA *ch, const Board &board);
bool can_write(CHAR_DATA *ch, const Board &board);
bool full_access(CHAR_DATA *ch, const Board &board);
void clan_delete_message(const std::string &name, int vnum);
void new_message_notify(const Board &board);

std::string& format_news_message(std::string &text);

} // namespace BoardSystem

ACMD(report_on_board);

typedef boost::shared_ptr<struct Message> MessagePtr;
typedef std::vector<MessagePtr> MessageListType;

// ��������� ���������
struct Message
{
	Message() : num(0), unique(0), level(0), rank(0), date(0) {};

	int num;             // ����� �� �����
	std::string author;  // ��� ������
	long unique;         // ��� ������
	int level;           // ������� ������ (��� ����, ����� �������� � ������������ �����)
	int rank;            // ��� ��� � ������
	time_t date;         // ���� �����
	std::string subject; // ����
	std::string text;    // ����� ���������
};

class Board
{
public:
	Board(Boards::BoardTypes in_type)
		: type_(in_type), last_write_(0), clan_rent_(0), pers_unique_(0),
		blind_(false) {};

	MessageListType messages; // ������ ���������

	Boards::BoardTypes get_type() const;
	const std::string & get_name() const;

	long get_lastwrite() const;
	void set_lastwrite(long unique);

	void Save();
	void add_message(MessagePtr msg);
	void new_message_notify() const;

	std::bitset<Boards::ACCESS_NUM> get_access(CHAR_DATA *ch) const;
	void do_list(CHAR_DATA *ch) const;
	bool is_special() const;
	time_t last_message_date() const;

	const std::string & get_alias() const;
	bool blind() const;

	static void BoardInit();
	static void ClanInit();
	static void clear_god_boards();
	static void init_god_board(long uid, std::string name);
	static void reload_all();
	static SPECIAL(Special);
	static void LoginInfo(CHAR_DATA * ch);

	friend ACMD(DoBoard);
	friend ACMD(DoBoardList);
	friend ACMD(report_on_board);

private:
	Boards::BoardTypes type_;  // ��� �����
	std::string name_;         // ��� �����
	std::string desc_;         // �������� �����
	long last_write_;          // ��� ���������� ��������� (�� ������)
	int clan_rent_;            // ����� ����� ����� (��� �������� �����)
	int pers_unique_;          // ��� (��� ������������ �����)
	std::string pers_name_;    // ��� (��� ������������ �����)
	std::string file_;         // ��� ����� ��� �����/�����
	std::string alias_;        // ������������ ������ ��� ����.�����
	bool blind_;               // ������ �� ����� �������� ���������

	void Load();
	std::string print_stats(CHAR_DATA *ch, int num);

	static void ShowMessage(CHAR_DATA * ch, MessagePtr message);
	static void create_board(Boards::BoardTypes type, const std::string &name,
		const std::string &desc, const std::string &file);
};

#endif
