// Copyright (c) 2012 Krodo
// Part of Bylins http://www.mud.ru

#ifndef SETS_DROP_HPP_INCLUDED
#define SETS_DROP_HPP_INCLUDED

#include "conf.h"
#include "sysdep.h"
#include "structs.h"

namespace SetsDrop
{

// ������ ���������� ������� ����� � ������� ����� (������)
const int SAVE_PERIOD = 27;
// ���� ������� ��� ������ ����
void init();
// ������ ������ ����� � ������������� ������ ������
// ��� ������� ���������� �� ��������� �����
void reload(int zone_vnum = 0);
// ���������� ������� ����� �� �������
void reload_by_timer();
// �������� ����� ������
int check_mob(int mob_rnum);
// ����� ���� ������ ������
void renumber_obj_rnum(const int rnum, const int mob_rnum = -1);
// ���������� ���� � ������� �������
void init_xhelp();
void init_xhelp_full();
// ������ ������� ������ ������� ����� ����� ��������� �������
void print_timer_str(CHAR_DATA *ch);
// ���� ������� ������� ����� � ������
void save_drop_table();

} // namespace SetsDrop

#endif // SETS_DROP_HPP_INCLUDED
