// $RCSfile$     $Date$     $Revision$
// Copyright (c) 2009 Krodo
// Part of Bylins http://www.mud.ru

#ifndef NAME_LIST_HPP_INCLUDED
#define NAME_LIST_HPP_INCLUDED

#include "conf.h"
#include "sysdep.h"
#include "structs.h"

// ��� ������ �������� � ������� �������� ������ ��������� �� ����� ����������� ����������

namespace CharacterAlias
{

void add(CHAR_DATA *ch);
void remove(CHAR_DATA *ch);
CHAR_DATA * get_by_name(const char *str);

} // namespace CharacterAlias

namespace ObjectAlias
{

void add(OBJ_DATA *obj);
void remove(OBJ_DATA *obj);
OBJ_DATA * get_by_name(const char *str);
OBJ_DATA * locate_object(const char *str);

} // namespace ObjectAlias

#endif // NAME_LIST_HPP_INCLUDED
