/*************************************************************************
*   File: liquid.cpp                                   Part of Bylins    *
*   ��� �� ���������                                                     *
*                                                                        *
*  $Author$                                                      *
*  $Date$                                          *
*  $Revision$                                                     *
************************************************************************ */

#include "liquid.hpp"
#include "char.hpp"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "spells.h"
#include "skills.h"
#include "features.hpp"
#include "room.hpp"
#include "screen.h"

extern void weight_change_object(OBJ_DATA * obj, int weight);

// ���� ����������, ���������� � ����������
const int LIQ_WATER = 0;
const int LIQ_BEER = 1;
const int LIQ_WINE = 2;
const int LIQ_ALE = 3;
const int LIQ_QUAS = 4;
const int LIQ_BRANDY = 5;
const int LIQ_MORSE = 6;
const int LIQ_VODKA = 7;
const int LIQ_BRAGA = 8;
const int LIQ_MED = 9;
const int LIQ_MILK = 10;
const int LIQ_TEA = 11;
const int LIQ_COFFE = 12;
const int LIQ_BLOOD = 13;
const int LIQ_SALTWATER = 14;
const int LIQ_CLEARWATER = 15;
const int LIQ_POTION = 16;
const int LIQ_POTION_RED = 17;
const int LIQ_POTION_BLUE = 18;
const int LIQ_POTION_WHITE = 19;
const int LIQ_POTION_GOLD = 20;
const int LIQ_POTION_BLACK = 21;
const int LIQ_POTION_GREY = 22;
const int LIQ_POTION_FUCHSIA = 23;
const int LIQ_POTION_PINK = 24;
const int LIQ_POISON_ACONITUM = 25;
const int LIQ_POISON_SCOPOLIA = 26;
const int LIQ_POISON_BELENA = 27;
const int LIQ_POISON_DATURA = 28;
// ����������
const int NUM_LIQ_TYPES = 29;

// LIQ_x
const char *drinks[] = { "����",
						 "����",
						 "����",
						 "��������",
						 "�����",
						 "��������",
						 "�����",
						 "�����",
						 "�����",
						 "����",
						 "������",
						 "���",
						 "�����",
						 "�����",
						 "������� ����",
						 "���������� ����",
						 "����������� �����",
						 "�������� ����������� �����",
						 "������ ����������� �����",
						 "������ ����������� �����",
						 "����������� ����������� �����",
						 "������� ����������� �����",
						 "������ ����������� �����",
						 "����������� ����������� �����",
						 "�������� ����������� �����",
						 "�������� �������",
						 "�������� ��������",
						 "�������� ������",
						 "�������� �������",
						 "\n"
					   };

// one-word alias for each drink
const char *drinknames[] = { "�����",
							 "�����",
							 "�����",
							 "���������",
							 "������",
							 "���������",
							 "������",
							 "������",
							 "������",
							 "�����",
							 "�������",
							 "����",
							 "����",
							 "������",
							 "������� �����",
							 "���������� �����",
							 "���������� ������",
							 "������� ���������� ������",
							 "����� ���������� ������",
							 "����� ���������� ������",
							 "���������� ���������� ������",
							 "������ ���������� ������",
							 "����� ���������� ������",
							 "���������� ���������� ������",
							 "������� ���������� ������",
							 "��������� �������",
							 "��������� ��������",
							 "��������� ������",
							 "��������� �������",
							 "\n"
						   };

// effect of drinks on DRUNK, FULL, THIRST -- see values.doc
const int drink_aff[][3] = { {0, 1, 10},	// ����
	{2, 2, 3},			// ����
	{5, 2, 2},			// ����
	{3, 2, 3},			// ��������
	{1, 2, 5},			// ����
	{8, 1, 1},			// �������
	{0, 1, 8},			// ����
	{10, 0, 1},			// �����
	{3, 3, 3},			// �����
	{0, 4, -8},			// ���
	{0, 3, 6},			// ������
	{0, 1, 6},			// ���
	{0, 1, 6},			// ����
	{0, 2, -1},			// �����
	{0, 1, -2},			// ������� ����
	{0, 0, 13},			// ���������� ����
	{0, 1, -1},			// ���������� �����
	{0, 1, -1},			// ������� ���������� �����
	{0, 1, -1},			// ����� ���������� �����
	{0, 1, -1},			// ����� ���������� �����
	{0, 1, -1},			// ���������� ���������� �����
	{0, 1, -1},			// ������ ���������� �����
	{0, 1, -1},			// ����� ���������� �����
	{0, 1, -1},			// ���������� ���������� �����
	{0, 1, -1},			// ������� ���������� �����
	{0, 0, 0},			// �������� �������
	{0, 0, 0},			// �������� ��������
	{0, 0, 0},			// �������� ������
	{0, 0, 0}			// �������� �������
};

// color of the various drinks
const char *color_liquid[] = { "����������",
							   "����������",
							   "��������",
							   "����������",
							   "�����",
							   "����������",
							   "���������",
							   "�������",
							   "������",
							   "������ �������",
							   "�����",
							   "����������",
							   "�����-����������",
							   "�������",
							   "����������",
							   "���������� ������",
							   "����������",
							   "��������",
							   "��������",
							   "�����������",
							   "����������",
							   "������ ������",
							   "����������",
							   "����������",
							   "�������",
							   "��������",
							   "��������",
							   "��������",
							   "��������",
							   "\n"
							 };

/**
* �����, ��������� � ����������, ����� ���� �� ����� ���.
* �� ������, ����� �������� ��������� ��� �������, �������
* ��� ����� ���� �� ������ �� �������.
*/
bool is_potion(const OBJ_DATA *obj)
{
	switch(GET_OBJ_VAL(obj, 2))
	{
	case LIQ_POTION:
	case LIQ_POTION_RED:
	case LIQ_POTION_BLUE:
	case LIQ_POTION_WHITE:
	case LIQ_POTION_GOLD:
	case LIQ_POTION_BLACK:
	case LIQ_POTION_GREY:
	case LIQ_POTION_FUCHSIA:
	case LIQ_POTION_PINK:
		return true;
		break;
	}
	return false;
}

namespace drinkcon
{

int init_spell_num(int num)
{
	return (num == 1 ? ObjVal::POTION_SPELL1_NUM
		: num == 2 ? ObjVal::POTION_SPELL2_NUM
		: ObjVal::POTION_SPELL3_NUM);
}

int init_spell_lvl(int num)
{
	return (num == 1 ? ObjVal::POTION_SPELL1_LVL
		: num == 2 ? ObjVal::POTION_SPELL2_LVL
		: ObjVal::POTION_SPELL3_LVL);
}

void reset_potion_values(OBJ_DATA *obj)
{
	obj->values.set(ObjVal::POTION_SPELL1_NUM, -1);
	obj->values.set(ObjVal::POTION_SPELL1_LVL, -1);
	obj->values.set(ObjVal::POTION_SPELL2_NUM, -1);
	obj->values.set(ObjVal::POTION_SPELL2_LVL, -1);
	obj->values.set(ObjVal::POTION_SPELL3_NUM, -1);
	obj->values.set(ObjVal::POTION_SPELL3_LVL, -1);
	obj->values.set(ObjVal::POTION_PROTO_VNUM, -1);
}

/// ������� � ������ (GET_OBJ_VAL(from_obj, 0)) ���� ���� �� ��� �����
bool copy_value(OBJ_DATA *from_obj, OBJ_DATA *to_obj, int num)
{
	if (GET_OBJ_VAL(from_obj, num) > 0)
	{
		to_obj->values.set(init_spell_num(num), GET_OBJ_VAL(from_obj, num));
		to_obj->values.set(init_spell_lvl(num), GET_OBJ_VAL(from_obj, 0));
		return true;
	}
	return false;
}

/// ���������� values ������� (to_obj) �� ����� (from_obj)
void copy_potion_values(OBJ_DATA *from_obj, OBJ_DATA *to_obj)
{
	reset_potion_values(to_obj);
	bool copied = false;

	for (int i = 1; i <= 3; ++i)
	{
		if (copy_value(from_obj, to_obj, i))
		{
			copied = true;
		}
	}

	if (copied)
	{
		to_obj->values.set(ObjVal::POTION_PROTO_VNUM, GET_OBJ_VNUM(from_obj));
	}
}

} // namespace drinkcon

using namespace drinkcon;

int cast_potion_spell(CHAR_DATA *ch, OBJ_DATA *obj, int num)
{
	const int spell = obj->values.get(init_spell_num(num));
	const int level = obj->values.get(init_spell_lvl(num));

	if (spell >= 0 && level >= 0)
	{
		return call_magic(ch, ch, NULL, world[IN_ROOM(ch)],
			spell, level, CAST_POTION);
	}
	return 1;
}

void cast_potion(CHAR_DATA *ch, OBJ_DATA *obj)
{
	for (int i = 1; i <= 3; ++i)
	{
		int result = cast_potion_spell(ch, obj, i);
		if (result <= 0)
		{
			break;
		}
	}
}

ACMD(do_drink)
{
	OBJ_DATA *temp;
	AFFECT_DATA af;
	int amount, weight, duration;
	int on_ground = 0;

	one_argument(argument, arg);

	if (IS_NPC(ch))		// Cannot use GET_COND() on mobs.
		return;

	if (!*arg)
	{
		send_to_char("���� �� ����?\r\n", ch);
		return;
	}

	if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying)))
	{
		if (!(temp = get_obj_in_list_vis(ch, arg, world[ch->in_room]->contents)))
		{
			send_to_char("�� �� ������ ��� �����!\r\n", ch);
			return;
		}
		else
			on_ground = 1;
	}
	if (IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND))
	{
		send_to_char("�� ����� ����������� � ���!\r\n", ch);
		return;
	}
	//��������� �� ������ ������� ���������� �����
	if (GET_OBJ_TYPE(temp) == ITEM_MING)
	{
		send_to_char("�� ������ ����������� - ������� �������!\r\n", ch);
		return;
	}
	if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) && (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN))
	{
		send_to_char("�� �����. ������ � ��� �����!\r\n", ch);
		return;
	}
	if (on_ground && (GET_OBJ_TYPE(temp) == ITEM_DRINKCON))
	{
		send_to_char("������ ��� ����� �������.\r\n", ch);
		return;
	}

	if (!GET_OBJ_VAL(temp, 1))
	{
		send_to_char("�����.\r\n", ch);
		return;
	}
	// Added by Adept - ������ ���� � ������� ��� ������� �����
	if (is_potion(temp) && temp->values.get(ObjVal::POTION_PROTO_VNUM) >= 0)
	{
		act("$n �����$g ����� �� $o1.", TRUE, ch, temp, 0, TO_ROOM);
		send_to_char(ch, "�� ������ ����� �� %s.\r\n", OBJN(temp, ch, 1));
		cast_potion(ch, temp);
		WAIT_STATE(ch, PULSE_VIOLENCE);
		GET_OBJ_WEIGHT(temp)--;
		// ��� ������
		if (--GET_OBJ_VAL(temp, 1) <= 0
			&& GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)
		{
			name_from_drinkcon(temp);
			GET_OBJ_SKILL(temp) = 0;
			reset_potion_values(temp);
		}
		return;
	}
	else if (ch->get_fighting())
	{
		send_to_char("�� ����� ����������� � ���.\r\n", ch);
		return;
	}
	//����� ��������� Adept'��

//	if ((GET_COND(ch, THIRST) > 0) && AFF_FLAGGED(ch, AFF_DRUNKED))  	// The pig is drunk
//	{
//		send_to_char("�� �� ������ ������� � ������.\r\n", ch);
//		act("$n �������$u ������ ���, �� �� ����$q ������� � ������.", TRUE, ch, 0, 0, TO_ROOM);
//		return;
//	}

	if (subcmd == SCMD_DRINK)
	{
		if (drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] > 0)
			if (GET_DRUNK_STATE(ch)<MAX_COND_VALUE && !AFF_FLAGGED(ch, AFF_ABSTINENT))
				amount = 24 / drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK];
			else
				amount = 0;
		else
			amount = number(3, 10);
	}
	else
	{
		amount = 1;
	}

	amount = MIN(amount, GET_OBJ_VAL(temp, 1));
	amount = MIN(amount, 24 - GET_COND(ch, THIRST));

	if (amount <= 0)
	{
		send_to_char("� ��� ������ �� �����.\r\n", ch);
		return;
	}
	else if (subcmd == SCMD_DRINK)
	{
		sprintf(buf, "$n �����$g %s �� $o1.", drinks[GET_OBJ_VAL(temp, 2)]);
		act(buf, TRUE, ch, temp, 0, TO_ROOM);
		sprintf(buf, "�� ������ %s �� %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)], OBJN(temp, ch, 1));
		send_to_char(buf, ch);
	}
	else
	{
		act("$n ���������$g �� $o1.", TRUE, ch, temp, 0, TO_ROOM);
		sprintf(buf, "�� ������ ���� %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
		send_to_char(buf, ch);
	}

	// You can't subtract more than the object weighs
	weight = MIN(amount, GET_OBJ_WEIGHT(temp));

	if (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)
		weight_change_object(temp, -weight);	// Subtract amount


	if ((GET_DRUNK_STATE(ch) < MAX_COND_VALUE && GET_DRUNK_STATE(ch) == GET_COND(ch, DRUNK))
		|| (GET_COND(ch, DRUNK) < CHAR_DRUNKED && !AFF_FLAGGED(ch, AFF_ABSTINENT)))
	{
		gain_condition(ch, DRUNK, (int)((int) drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] * amount) / 4);
		GET_DRUNK_STATE(ch) = MAX(GET_DRUNK_STATE(ch), GET_COND(ch, DRUNK));
	}

	gain_condition(ch, FULL, (int)((int) drink_aff[GET_OBJ_VAL(temp, 2)][FULL] * amount) / 4);

	gain_condition(ch, THIRST, (int)((int) drink_aff[GET_OBJ_VAL(temp, 2)][THIRST] * amount) / 4);

	if (GET_COND(ch, THIRST) > 20)
		send_to_char("�� �� ���������� �����.\r\n", ch);

	if (GET_COND(ch, FULL) > 20)
		send_to_char("�� ���������� �������� ������� � �������.\r\n", ch);

	if (GET_COND(ch, DRUNK) >= CHAR_DRUNKED)
	{
		if (GET_DRUNK_STATE(ch) == MAX_COND_VALUE || GET_COND(ch, DRUNK) < GET_DRUNK_STATE(ch))
		{
			send_to_char("�� ������� ��� ����������, ������ ��� �������...\r\n", ch);
		}else
			if (GET_COND(ch, DRUNK) >= CHAR_MORTALLY_DRUNKED)
			{
				send_to_char("�������� �� �����, �� ����� ��� �� ����....\r\n", ch);
			}
			else
			{
				send_to_char("�������� ����� ��������� �� ������ ����.\r\n", ch);
			}
		duration = 2 + MAX(0, GET_COND(ch, DRUNK) - CHAR_DRUNKED);
		if (can_use_feat(ch, DRUNKARD_FEAT))
			duration += duration/2;
		if (!AFF_FLAGGED(ch, AFF_ABSTINENT)
				&& GET_DRUNK_STATE(ch) < MAX_COND_VALUE
					&& GET_DRUNK_STATE(ch) == GET_COND(ch, DRUNK))
		{
			send_to_char("������ ���� ������� ��� � ������.\r\n", ch);
			// **** Decrease AC ***** //
			af.type = SPELL_DRUNKED;
			af.duration = pc_duration(ch, duration, 0, 0, 0, 0);
			af.modifier = -20;
			af.location = APPLY_AC;
			af.bitvector = AFF_DRUNKED;
			af.battleflag = 0;
			affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
			// **** Decrease HR ***** //
			af.type = SPELL_DRUNKED;
			af.duration = pc_duration(ch, duration, 0, 0, 0, 0);
			af.modifier = -2;
			af.location = APPLY_HITROLL;
			af.bitvector = AFF_DRUNKED;
			af.battleflag = 0;
			affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
			// **** Increase DR ***** //
			af.type = SPELL_DRUNKED;
			af.duration = pc_duration(ch, duration, 0, 0, 0, 0);
			af.modifier = (GET_LEVEL(ch) + 4) / 5;
			af.location = APPLY_DAMROLL;
			af.bitvector = AFF_DRUNKED;
			af.battleflag = 0;
			affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
		}
	}

	if (GET_OBJ_VAL(temp, 3) && !IS_GOD(ch))  	// The shit was poisoned ! //
	{
		send_to_char("���-�� ���� �����-�� ��������!\r\n", ch);
		act("$n ���������$u � ��������$g.", TRUE, ch, 0, 0, TO_ROOM);

		af.type = SPELL_POISON;
		af.duration = pc_duration(ch, amount == 1 ? amount : amount * 3, 0, 0, 0, 0);
		af.modifier = -2;
		af.location = APPLY_STR;
		af.bitvector = AFF_POISON;
		af.battleflag = AF_SAME_TIME;
		affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
		af.type = SPELL_POISON;
		af.modifier = amount * 3;
		af.location = APPLY_POISON;
		af.bitvector = AFF_POISON;
		af.battleflag = AF_SAME_TIME;
		affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
		ch->Poisoner = 0;
	}

	// empty the container, and no longer poison. 999 - whole fountain //
	if (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN || GET_OBJ_VAL(temp, 1) != 999)
		GET_OBJ_VAL(temp, 1) -= amount;
	if (!GET_OBJ_VAL(temp, 1))  	// The last bit //
	{
		GET_OBJ_VAL(temp, 2) = 0;
		GET_OBJ_VAL(temp, 3) = 0;
		name_from_drinkcon(temp);
	}

	return;
}

ACMD(do_drunkoff)
{
	OBJ_DATA *obj;
	AFFECT_DATA af[3];
	struct timed_type timed;
	int amount, weight, prob, percent, duration;
	int on_ground = 0;

	if (IS_NPC(ch))		// Cannot use GET_COND() on mobs. //
		return;

	if (ch->get_fighting())
	{
		send_to_char("�� ����� ����������� � ���.\r\n", ch);
		return;
	}

	if (AFF_FLAGGED(ch, AFF_DRUNKED))
	{
		send_to_char("�� ������ ��������� ���� ���� �����?\r\n" "��� �� ���� �� ������!\r\n", ch);
		return;
	}

	if (!AFF_FLAGGED(ch, AFF_ABSTINENT) && GET_COND(ch, DRUNK) < CHAR_DRUNKED)
	{
		send_to_char("�� ����� ������ ����� �� ������� ������.\r\n", ch);
		return;
	}

	if (timed_by_skill(ch, SKILL_DRUNKOFF))
	{
		send_to_char("�� �� � ��������� ��� ����� �����������.\r\n"
					 "��������� ����� ������������ ���.\r\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (!*arg)
	{
		for (obj = ch->carrying; obj; obj = obj->next_content)
			if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON)
				break;
		if (!obj)
		{
			send_to_char("� ��� ��� ����������� ������� ��� ��������.\r\n", ch);
			return;
		}
	}
	else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying)))
	{
		if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room]->contents)))
		{
			send_to_char("�� �� ������ ��� �����!\r\n", ch);
			return;
		}
		else
			on_ground = 1;
	}

	if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) && (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
	{
		send_to_char("���� �� ����-�� ������� �����������.\r\n", ch);
		return;
	}

	if (on_ground && (GET_OBJ_TYPE(obj) == ITEM_DRINKCON))
	{
		send_to_char("������ ��� ����� �������.\r\n", ch);
		return;
	}

	if (!GET_OBJ_VAL(obj, 1))
	{
		send_to_char("�����.\r\n", ch);
		return;
	}

	switch (GET_OBJ_VAL(obj, 2))
	{
	case LIQ_BEER:
	case LIQ_WINE:
	case LIQ_ALE:
	case LIQ_QUAS:
	case LIQ_BRANDY:
	case LIQ_VODKA:
	case LIQ_BRAGA:
		break;
	default:
		send_to_char("��������� �������� �������� :\r\n" "\"���� �������� ������...\"\r\n", ch);
		return;
	}

	amount = MAX(1, GET_WEIGHT(ch) / 50);
	if (amount > GET_OBJ_VAL(obj, 1))
	{
		send_to_char("��� ����� �� ������ ����� ���������� ��� ��������...\r\n", ch);
		return;
	}

	timed.skill = SKILL_DRUNKOFF;
	timed.time = can_use_feat(ch, DRUNKARD_FEAT) ? feature_mod(DRUNKARD_FEAT, FEAT_TIMER) : 12;
	timed_to_char(ch, &timed);

	percent = number(1, skill_info[SKILL_DRUNKOFF].max_percent);
	prob = train_skill(ch, SKILL_DRUNKOFF, skill_info[SKILL_DRUNKOFF].max_percent, 0);
	amount = MIN(amount, GET_OBJ_VAL(obj, 1));
	weight = MIN(amount, GET_OBJ_WEIGHT(obj));
	weight_change_object(obj, -weight);	// Subtract amount //
	GET_OBJ_VAL(obj, 1) -= amount;
	if (!GET_OBJ_VAL(obj, 1))  	// The last bit //
	{
		GET_OBJ_VAL(obj, 2) = 0;
		GET_OBJ_VAL(obj, 3) = 0;
		name_from_drinkcon(obj);
	}

	if (percent > prob)
	{
		sprintf(buf,
				"�� ���������� %s �� $o1, �� ���� ������ ����� ��� �������...", drinks[GET_OBJ_VAL(obj, 2)]);
		act(buf, FALSE, ch, obj, 0, TO_CHAR);
		act("$n ����������$g �����������, �� ��� �� ����� $m �� ������.", FALSE, ch, 0, 0, TO_ROOM);
		duration = MAX(1, amount / 3);
		af[0].type = SPELL_ABSTINENT;
		af[0].duration = pc_duration(ch, duration, 0, 0, 0, 0);
		af[0].modifier = 0;
		af[0].location = APPLY_DAMROLL;
		af[0].bitvector = AFF_ABSTINENT;
		af[0].battleflag = 0;
		af[1].type = SPELL_ABSTINENT;
		af[1].duration = pc_duration(ch, duration, 0, 0, 0, 0);
		af[1].modifier = 0;
		af[1].location = APPLY_HITROLL;
		af[1].bitvector = AFF_ABSTINENT;
		af[1].battleflag = 0;
		af[2].type = SPELL_ABSTINENT;
		af[2].duration = pc_duration(ch, duration, 0, 0, 0, 0);
		af[2].modifier = 0;
		af[2].location = APPLY_AC;
		af[2].bitvector = AFF_ABSTINENT;
		af[2].battleflag = 0;
		switch (number(0, ch->get_skill(SKILL_DRUNKOFF) / 20))
		{
		case 0:
		case 1:
			af[0].modifier = -2;
		case 2:
		case 3:
			af[1].modifier = -2;
		default:
			af[2].modifier = 10;
		}
		for (prob = 0; prob < 3; prob++)
			affect_join(ch, &af[prob], TRUE, FALSE, TRUE, FALSE);
		gain_condition(ch, DRUNK, amount);
	}
	else
	{
		sprintf(buf,
				"�� ���������� %s �� $o1 � ������������� �������� �������� �� ���� ����...",
				drinks[GET_OBJ_VAL(obj, 2)]);
		act(buf, FALSE, ch, obj, 0, TO_CHAR);
		act("$n ��������$u � �������$g ���� �� ������.", FALSE, ch, 0, 0, TO_ROOM);
		affect_from_char(ch, SPELL_ABSTINENT);
	}

	return;
}

void generate_drinkcon_name(OBJ_DATA *to_obj, int spell)
{
	switch (spell)
	{
		// �������������� (�������) //
	case SPELL_REFRESH:
	case SPELL_GROUP_REFRESH:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION_RED;
		name_to_drinkcon(to_obj, LIQ_POTION_RED);
		break;
		// ��������� (�����) //
	case SPELL_FULL:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION_BLUE;
		name_to_drinkcon(to_obj, LIQ_POTION_BLUE);
		break;
		// ������� (�����) //
	case SPELL_DETECT_INVIS:
	case SPELL_DETECT_MAGIC:
	case SPELL_DETECT_POISON:
	case SPELL_DETECT_ALIGN:
	case SPELL_SENSE_LIFE:
	case SPELL_INFRAVISION:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION_WHITE;
		name_to_drinkcon(to_obj, LIQ_POTION_WHITE);
		break;
		// �������� (����������) //
	case SPELL_ARMOR:
	case SPELL_GROUP_ARMOR:
	case SPELL_CLOUDLY:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION_GOLD;
		name_to_drinkcon(to_obj, LIQ_POTION_GOLD);
		break;
		// ����������������� �������� (������) //
	case SPELL_CURE_CRITIC:
	case SPELL_CURE_LIGHT:
	case SPELL_HEAL:
	case SPELL_GROUP_HEAL:
	case SPELL_CURE_SERIOUS:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION_BLACK;
		name_to_drinkcon(to_obj, LIQ_POTION_BLACK);
		break;
		// ��������� ������� ������� (�����) //
	case SPELL_CURE_BLIND:
	case SPELL_REMOVE_CURSE:
	case SPELL_REMOVE_HOLD:
	case SPELL_REMOVE_SIELENCE:
	case SPELL_CURE_PLAQUE:
	case SPELL_REMOVE_DEAFNESS:
	case SPELL_REMOVE_POISON:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION_GREY;
		name_to_drinkcon(to_obj, LIQ_POTION_GREY);
		break;
		// ������ ���������� (����������) //
	case SPELL_INVISIBLE:
	case SPELL_GROUP_INVISIBLE:
	case SPELL_STRENGTH:
	case SPELL_GROUP_STRENGTH:
	case SPELL_FLY:
	case SPELL_GROUP_FLY:
	case SPELL_BLESS:
	case SPELL_GROUP_BLESS:
	case SPELL_HASTE:
	case SPELL_GROUP_HASTE:
	case SPELL_STONESKIN:
	case SPELL_BLINK:
	case SPELL_EXTRA_HITS:
	case SPELL_WATERBREATH:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION_FUCHSIA;
		name_to_drinkcon(to_obj, LIQ_POTION_FUCHSIA);
		break;
	case SPELL_PRISMATICAURA:
	case SPELL_GROUP_PRISMATICAURA:
	case SPELL_AIR_AURA:
	case SPELL_FIRE_AURA:
	case SPELL_ICE_AURA:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION_PINK;
		name_to_drinkcon(to_obj, LIQ_POTION_PINK);
		break;
	default:
		GET_OBJ_VAL(to_obj, 2) = LIQ_POTION;
		name_to_drinkcon(to_obj, LIQ_POTION);	// ��������� ����� ������� //
	}
}

int check_potion_spell(OBJ_DATA *from_obj, OBJ_DATA *to_obj, int num)
{
	const int spell = init_spell_num(num);
	const int level = init_spell_lvl(num);

	if (GET_OBJ_VAL(from_obj, num) != to_obj->values.get(spell))
	{
		// �� ������� �����
		return 0;
	}
	if (GET_OBJ_VAL(from_obj, 0) < to_obj->values.get(level))
	{
		// ������������ ����� ���� ������ ����� � �������
		return -1;
	}
	return 1;
}

/// \return 1 - ����� ����������
///         0 - ������ ��������� ������ �����
///        -1 - ������� �������� ����� � ������� ������� �����
int check_equal_potions(OBJ_DATA *from_obj, OBJ_DATA *to_obj)
{
	// ������� � ��� ��������� ����� ������
	if (to_obj->values.get(ObjVal::POTION_PROTO_VNUM) > 0
		&& GET_OBJ_VNUM(from_obj) != to_obj->values.get(ObjVal::POTION_PROTO_VNUM))
	{
		return 0;
	}
	// ���������� ������ � �� �������� ������
	for (int i = 1; i <= 3; ++i)
	{
		if (GET_OBJ_VAL(from_obj, i) > 0)
		{
			int result = check_potion_spell(from_obj, to_obj, i);
			if (result <= 0)
			{
				return result;
			}
		}
	}
	return 1;
}

/// �� check_equal_drinkcon()
int check_drincon_spell(OBJ_DATA *from_obj, OBJ_DATA *to_obj, int num)
{
	const int spell = init_spell_num(num);
	const int level = init_spell_lvl(num);

	if (from_obj->values.get(spell) != to_obj->values.get(spell))
	{
		// �� ������� �����
		return 0;
	}
	if (from_obj->values.get(level) < to_obj->values.get(level))
	{
		// ������������ ����� ���� ������ ����� � �������
		return -1;
	}
	return 1;
}

/// ��������� ������ check_equal_potions ��� ���� ��������, ���� � �������
/// ��� ��� values � �������/��������.
/// \return 1 - ����� ����������
///         0 - ������ ��������� ������ �����
///        -1 - ������� �������� ����� � ������� ������� �����
int check_equal_drinkcon(OBJ_DATA *from_obj, OBJ_DATA *to_obj)
{
	// ���������� ������ � �� �������� ������ (� � ��� �� �������, ��� ����)
	for (int i = 1; i <= 3; ++i)
	{
		if (GET_OBJ_VAL(from_obj, i) > 0)
		{
			int result = check_drincon_spell(from_obj, to_obj, i);
			if (result <= 0)
			{
				return result;
			}
		}
	}
	return 1;
}

/// ����������� ����� ���������� ��� ����������� �� ������� � ������ � ������ �������
void spells_to_drinkcon(OBJ_DATA *from_obj, OBJ_DATA *to_obj)
{
	// ���� ������
	for (int i = 1; i <= 3; ++i)
	{
		const int spell = init_spell_num(i);
		const int level = init_spell_lvl(i);
		to_obj->values.set(spell, from_obj->values.get(spell));
		to_obj->values.set(level, from_obj->values.get(level));
	}
	// ���������� ���� � �������������� ��������� �����
	const int proto_vnum = from_obj->values.get(ObjVal::POTION_PROTO_VNUM) > 0
		? from_obj->values.get(ObjVal::POTION_PROTO_VNUM)
		: GET_OBJ_VNUM(from_obj);
	to_obj->values.set(ObjVal::POTION_PROTO_VNUM, proto_vnum);
}

ACMD(do_pour)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *from_obj = NULL, *to_obj = NULL;
	int amount;

	two_arguments(argument, arg1, arg2);

	if (subcmd == SCMD_POUR)
	{
		if (!*arg1)  	// No arguments //
		{
			send_to_char("������ ����������?\r\n", ch);
			return;
		}
		if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying)))
		{
			send_to_char("� ��� ��� �����!\r\n", ch);
			return;
		}
		if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON && GET_OBJ_TYPE(from_obj) != ITEM_POTION)
		{
			send_to_char("�� �� ������ �� ����� ����������!\r\n", ch);
			return;
		}
	}
	if (subcmd == SCMD_FILL)
	{
		if (!*arg1)  	// no arguments //
		{
			send_to_char("��� � �� ���� �� ������ �� ���������?\r\n", ch);
			return;
		}
		if (!(to_obj = get_obj_in_list_vis(ch, arg1, ch->carrying)))
		{
			send_to_char("� ��� ����� ���!\r\n", ch);
			return;
		}
		if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON)
		{
			act("�� �� ������ ��������� $o3!", FALSE, ch, to_obj, 0, TO_CHAR);
			return;
		}
		if (!*arg2)  	// no 2nd argument //
		{
			act("�� ���� �� ���������� ��������� $o3?", FALSE, ch, to_obj, 0, TO_CHAR);
			return;
		}
		if (!(from_obj = get_obj_in_list_vis(ch, arg2, world[ch->in_room]->contents)))
		{
			sprintf(buf, "�� �� ������ ����� '%s'.\r\n", arg2);
			send_to_char(buf, ch);
			return;
		}
		if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN)
		{
			act("�� �� ������� ������ ��������� �� $o1.", FALSE, ch, from_obj, 0, TO_CHAR);
			return;
		}
	}
	if (GET_OBJ_VAL(from_obj, 1) == 0)
	{
		act("�����.", FALSE, ch, from_obj, 0, TO_CHAR);
		return;
	}
	if (subcmd == SCMD_POUR)  	// pour //
	{
		if (!*arg2)
		{
			send_to_char("���� �� ������ ����? �� ����� ��� �� ���-��?\r\n", ch);
			return;
		}
		if (!str_cmp(arg2, "out") || !str_cmp(arg2, "�����"))
		{
			act("$n ���������$g $o3.", TRUE, ch, from_obj, 0, TO_ROOM);
			act("�� ���������� $o3.", FALSE, ch, from_obj, 0, TO_CHAR);

			weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, 1));	// Empty //

			GET_OBJ_VAL(from_obj, 1) = 0;
			GET_OBJ_VAL(from_obj, 2) = 0;
			GET_OBJ_VAL(from_obj, 3) = 0;
			GET_OBJ_SKILL(from_obj) = 0;
			name_from_drinkcon(from_obj);
			reset_potion_values(from_obj);

			return;
		}
		if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying)))
		{
			send_to_char("�� �� ������ ����� �����!\r\n", ch);
			return;
		}
		if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) && (GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN))
		{
			send_to_char("�� �� ������� � ��� ������.\r\n", ch);
			return;
		}
	}
	if (to_obj == from_obj)
	{
		send_to_char("����� ������ ������� �� ���������, �������, �� �����.\r\n", ch);
		return;
	}

	if (GET_OBJ_VAL(to_obj, 1) != 0 &&
			GET_OBJ_TYPE(from_obj) != ITEM_POTION && GET_OBJ_VAL(to_obj, 2) != GET_OBJ_VAL(from_obj, 2))
	{
		send_to_char("�� ������� �������� �������, �� �� � ����� ����.\r\n", ch);
		return;
	}
	if (GET_OBJ_VAL(to_obj, 1) >= GET_OBJ_VAL(to_obj, 0))
	{
		send_to_char("��� ��� �����.\r\n", ch);
		return;
	}
//Added by Adept - ����������� ����� �� ������� ��� ������� � �������

	//���������� �� ������� � ������ � �������
	if (GET_OBJ_TYPE(from_obj) == ITEM_POTION)
	{
		int result = check_equal_potions(from_obj, to_obj);
		if (GET_OBJ_VAL(to_obj, 1) == 0 || result > 0)
		{
			send_to_char(ch, "�� �������� ������������ ����� � %s.\r\n",
				OBJN(to_obj, ch, 3));
			if (GET_OBJ_VAL(to_obj, 1) == 0)
			{
				copy_potion_values(from_obj, to_obj);
				// ����������� �������� ����� �� ������������� ���������� //
				generate_drinkcon_name(to_obj, GET_OBJ_VAL(from_obj, 1));
			}
			weight_change_object(to_obj, 1);
			GET_OBJ_VAL(to_obj, 1)++;
			extract_obj(from_obj);
			return;
		}
		else if (result < 0)
		{
			send_to_char("�� ��������� ��������� ����� ������ �����!\r\n", ch);
			return;
		}
		else
		{
			send_to_char(
				"��������� ������ �����?! �� ��, ��������, ������!\r\n", ch);
			return;
		}
	}

	//���������� �� ������� ��� ������� � ������ ����-��
	if ((GET_OBJ_TYPE(from_obj) == ITEM_DRINKCON ||
			GET_OBJ_TYPE(from_obj) == ITEM_FOUNTAIN)
		&& is_potion(from_obj))
	{
		if (GET_OBJ_VAL(to_obj, 1) == 0)
		{
			spells_to_drinkcon(from_obj, to_obj);
		}
		else
		{
			const int result = check_equal_drinkcon(from_obj, to_obj);
			if (result < 0)
			{
				send_to_char("�� ��������� ��������� ����� ������ �����!\r\n", ch);
				return;
			}
			else if (!result)
			{
				send_to_char(
					"��������� ������ �����?! �� ��, ��������, ������!\r\n", ch);
				return;
			}
		}
	}
//����� ��������� Adept'��

	if (subcmd == SCMD_POUR)
	{
		send_to_char(ch, "�� �������� ������������ %s � %s.",
			drinks[GET_OBJ_VAL(from_obj, 2)], OBJN(to_obj, ch, 3));
	}
	if (subcmd == SCMD_FILL)
	{
		act("�� ��������� $o3 �� $O1.", FALSE, ch, to_obj, from_obj, TO_CHAR);
		act("$n ��������$g $o3 �� $O1.", TRUE, ch, to_obj, from_obj, TO_ROOM);
	}

	// �������� ��� �������� //
	GET_OBJ_VAL(to_obj, 2) = GET_OBJ_VAL(from_obj, 2);

	// New alias //
	if (GET_OBJ_VAL(to_obj, 1) == 0)
		name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, 2));

	// Then how much to pour //
	if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN || GET_OBJ_VAL(from_obj, 1) != 999)
		GET_OBJ_VAL(from_obj, 1) -= (amount = (GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1)));
	else
		amount = GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1);

	GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);

	// Then the poison boogie //
	GET_OBJ_VAL(to_obj, 3) = (GET_OBJ_VAL(to_obj, 3) || GET_OBJ_VAL(from_obj, 3));

	if (GET_OBJ_VAL(from_obj, 1) <= 0)  	// There was too little //
	{
		GET_OBJ_VAL(to_obj, 1) += GET_OBJ_VAL(from_obj, 1);
		amount += GET_OBJ_VAL(from_obj, 1);
		GET_OBJ_VAL(from_obj, 1) = 0;
		GET_OBJ_VAL(from_obj, 2) = 0;
		GET_OBJ_VAL(from_obj, 3) = 0;
		name_from_drinkcon(from_obj);
		reset_potion_values(from_obj);
	}

	// And the weight boogie //
	if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN)
		weight_change_object(from_obj, -amount);
	weight_change_object(to_obj, amount);	// Add weight //
}

size_t find_liquid_name(char * name)
{
	std::string tmp = std::string(name);
	size_t pos, result = std::string::npos;
	for (int i = 0; strcmp(drinknames[i],"\n"); i++)
	{
		pos = tmp.find(drinknames[i]);
		if (pos != std::string::npos)
			result = pos;
	}
	return result;
}

void name_from_drinkcon(OBJ_DATA * obj)
{
	char new_name[MAX_STRING_LENGTH];
	std::string tmp;

	size_t pos = find_liquid_name(obj->aliases);
	if (pos == std::string::npos) return;
	tmp = std::string(obj->aliases).substr(0, pos - 1);

	sprintf(new_name, "%s", tmp.c_str());
	if (GET_OBJ_RNUM(obj) < 0 || obj->aliases != obj_proto[GET_OBJ_RNUM(obj)]->aliases)
		free(obj->aliases);
	obj->aliases = str_dup(new_name);

	pos = find_liquid_name(obj->short_description);
	if (pos == std::string::npos) return;
	tmp = std::string(obj->short_description).substr(0, pos - 3);

	sprintf(new_name, "%s", tmp.c_str());
	if (GET_OBJ_RNUM(obj) < 0 || obj->short_description != obj_proto[GET_OBJ_RNUM(obj)]->short_description)
		free(obj->short_description);
	obj->short_description = str_dup(new_name);


	for (int c = 0; c < NUM_PADS; c++)
	{
		pos = find_liquid_name(obj->PNames[c]);
		if (pos == std::string::npos) return;
		tmp = std::string(obj->PNames[c]).substr(0, pos - 3);
		sprintf(new_name, "%s", tmp.c_str());
		if (GET_OBJ_RNUM(obj) < 0 || obj->PNames[c] != obj_proto[GET_OBJ_RNUM(obj)]->PNames[c])
			free(obj->PNames[c]);
		obj->PNames[c] = str_dup(new_name);
	}
}

void name_to_drinkcon(OBJ_DATA * obj, int type)
{
	int c;
	char new_name[MAX_INPUT_LENGTH];

	sprintf(new_name, "%s %s", obj->aliases, drinknames[type]);
	if (GET_OBJ_RNUM(obj) < 0 || obj->aliases != obj_proto[GET_OBJ_RNUM(obj)]->aliases)
		free(obj->aliases);
	obj->aliases = str_dup(new_name);

	sprintf(new_name, "%s � %s", obj->short_description, drinknames[type]);
	if (GET_OBJ_RNUM(obj) < 0 || obj->short_description != obj_proto[GET_OBJ_RNUM(obj)]->short_description)
		free(obj->short_description);
	obj->short_description = str_dup(new_name);


	for (c = 0; c < NUM_PADS; c++)
	{
		sprintf(new_name, "%s � %s", obj->PNames[c], drinknames[type]);
		if (GET_OBJ_RNUM(obj) < 0 || obj->PNames[c] != obj_proto[GET_OBJ_RNUM(obj)]->PNames[c])
			free(obj->PNames[c]);
		obj->PNames[c] = str_dup(new_name);
	}
}

void set_abstinent(CHAR_DATA *ch)
{
	AFFECT_DATA af;

	int duration = pc_duration(ch, 2, MAX(0, GET_DRUNK_STATE(ch) - CHAR_DRUNKED), 4, 2, 5);

	if (can_use_feat(ch, DRUNKARD_FEAT))
		duration /= 2;

	af.type = SPELL_ABSTINENT;
	af.bitvector = AFF_ABSTINENT;
	af.duration = duration;

	af.location = APPLY_AC;
	af.modifier = 20;
	affect_join(ch, &af, 0,0,0,0);

	af.location = APPLY_HITROLL;
	af.modifier = -2;
	affect_join(ch, &af, 0,0,0,0);

	af.location = APPLY_DAMROLL;
	af.modifier = -2;
	affect_join(ch, &af, 0,0,0,0);

}

std::string print_spell(CHAR_DATA *ch, const OBJ_DATA *obj, int num)
{
	const int spell = init_spell_num(num);
	const int level = init_spell_lvl(num);

	if (obj->values.get(spell) == -1)
	{
		return "";
	}

	char buf_[MAX_INPUT_LENGTH];
	snprintf(buf_, sizeof(buf_), "�������� ����������: %s%s (%d ��.)%s\r\n",
		CCCYN(ch, C_NRM),
		spell_name(obj->values.get(spell)), obj->values.get(level),
		CCNRM(ch, C_NRM));

	return buf_;
}

namespace drinkcon
{

std::string print_spells(CHAR_DATA *ch, const OBJ_DATA *obj)
{
	std::string out;
	char buf_[MAX_INPUT_LENGTH];

	for (int i = 1; i <= 3; ++i)
	{
		out += print_spell(ch, obj, i);
	}

	if (!out.empty() && !is_potion(obj))
	{
		snprintf(buf_, sizeof(buf_),
			"%s��������%s: ��� �������� �� �������� ������\r\n",
			CCIRED(ch, C_NRM), CCNRM(ch, C_NRM));
		out += buf_;
	}
	else if (out.empty() && is_potion(obj))
	{
		snprintf(buf_, sizeof(buf_),
			"%s��������%s: � ������� ����� ����������� ����������\r\n",
			CCIRED(ch, C_NRM), CCNRM(ch, C_NRM));
		out += buf_;
	}

	return out;
}

void identify(CHAR_DATA *ch, const OBJ_DATA *obj)
{
	std::string out;
	char buf_[MAX_INPUT_LENGTH];

	snprintf(buf_, sizeof(buf_), "����� �������� �����: %s%d %s%s\r\n",
		CCCYN(ch, C_NRM),
		GET_OBJ_VAL(obj, 0), desc_count(GET_OBJ_VAL(obj, 0), WHAT_GULP),
		CCNRM(ch, C_NRM));
	out += buf_;

	// ������� �� �����
	if (GET_OBJ_VAL(obj, 1) > 0)
	{
		// ���� �����-�� �����
		if (obj->values.get(ObjVal::POTION_PROTO_VNUM) >= 0)
		{
			if (IS_IMMORTAL(ch))
			{
				snprintf(buf_, sizeof(buf_),
					"�������� %d %s %s (VNUM: %d).\r\n",
					GET_OBJ_VAL(obj, 1),
					desc_count(GET_OBJ_VAL(obj, 1), WHAT_GULP),
					drinks[GET_OBJ_VAL(obj, 2)],
					obj->values.get(ObjVal::POTION_PROTO_VNUM));
			}
			else
			{
				snprintf(buf_, sizeof(buf_), "�������� %d %s %s.\r\n",
					GET_OBJ_VAL(obj, 1),
					desc_count(GET_OBJ_VAL(obj, 1), WHAT_GULP),
					drinks[GET_OBJ_VAL(obj, 2)]);
			}
			out += buf_;
			out += print_spells(ch, obj);
		}
		else
		{
			snprintf(buf_, sizeof(buf_), "�������� %s �� %d%%\r\n",
				drinknames[GET_OBJ_VAL(obj, 2)],
				GET_OBJ_VAL(obj, 1)*100/GET_OBJ_VAL(obj, 0));
			out += buf_;
			// ����� ������ ������� �� ���� ����� ��� ������
			if (is_potion(obj))
			{
				out += print_spells(ch, obj);
			}
		}
	}
	send_to_char(out, ch);
}

} // namespace drinkcon
