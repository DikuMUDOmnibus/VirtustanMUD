/* ************************************************************************
*   File: act.offensive.cpp                               Part of Bylins  *
*  Usage: player-level commands of an offensive nature                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  $Author$                                                               *
*  $Date$                                                                 *
*  $Revision$                                                             *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "screen.h"
#include "spells.h"
#include "skills.h"
#include "pk.h"
#include "features.hpp"
#include "privilege.hpp"
#include "random.hpp"
#include "char.hpp"
#include "room.hpp"
#include "fight.h"

// extern variables
extern DESCRIPTOR_DATA *descriptor_list;

// extern functions
int compute_armor_class(CHAR_DATA * ch);
int awake_others(CHAR_DATA * ch);
void appear(CHAR_DATA * ch);
int legal_dir(CHAR_DATA * ch, int dir, int need_specials_check, int show_msg);
void alt_equip(CHAR_DATA * ch, int pos, int dam, int chance);
void go_protect(CHAR_DATA * ch, CHAR_DATA * vict);

// local functions
ACMD(do_assist);
ACMD(do_hit);
ACMD(do_kill);
ACMD(do_backstab);
ACMD(do_order);
ACMD(do_flee);
ACMD(do_bash);
ACMD(do_rescue);
ACMD(do_kick);
ACMD(do_manadrain);
ACMD(do_coddle_out);
ACMD(do_strangle);
CHAR_DATA *try_protect(CHAR_DATA * victim, CHAR_DATA * ch);


int have_mind(CHAR_DATA * ch)
{
	if (!AFF_FLAGGED(ch, AFF_CHARM) && !IS_HORSE(ch))
		return (TRUE);
	return (FALSE);
}

void set_wait(CHAR_DATA * ch, int waittime, int victim_in_room)
{
	if (!WAITLESS(ch) && (!victim_in_room || (ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))))
		WAIT_STATE(ch, waittime * PULSE_VIOLENCE);
};

int set_hit(CHAR_DATA * ch, CHAR_DATA * victim)
{
	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return (FALSE);
	}

	if (ch->get_fighting() || GET_MOB_HOLD(ch))
	{
		return (FALSE);
	}
	victim = try_protect(victim, ch);

	bool message = false;
	// ���� ������ ����� �� ����� - ���������� ��� ������ � ������ ��� ��� ����
	if (victim->desc && (STATE(victim->desc) == CON_WRITEBOARD || STATE(victim->desc) == CON_WRITE_MOD))
	{
		victim->desc->message.reset();
		victim->desc->board.reset();
		if (*(victim->desc->str))
			free(*victim->desc->str);
		if (victim->desc->str)
			free(victim->desc->str);
		STATE(victim->desc) = CON_PLAYING;
		if (!IS_NPC(victim))
			REMOVE_BIT(PLR_FLAGS(victim, PLR_WRITING), PLR_WRITING);
		if (victim->desc->backstr)
			free(victim->desc->backstr);
		victim->desc->backstr = NULL;
		victim->desc->str = NULL;
		message = true;
	}
	else if (victim->desc && (STATE(victim->desc) == CON_CLANEDIT))
	{
		// ����������, ���� ����� ������ ���� ������� � ���
		victim->desc->clan_olc.reset();
		STATE(victim->desc) = CON_PLAYING;
		message = true;
	}
	else if (victim->desc && (STATE(victim->desc) == CON_SPEND_GLORY))
	{
		// ��� �������-���������� �����
		victim->desc->glory.reset();
		STATE(victim->desc) = CON_PLAYING;
		message = true;
	}
	else if (victim->desc && (STATE(victim->desc) == CON_GLORY_CONST))
	{
		// ��� �������-���������� �����
		victim->desc->glory_const.reset();
		STATE(victim->desc) = CON_PLAYING;
		message = true;
	}
	else if (victim->desc && (STATE(victim->desc) == CON_MAP_MENU))
	{
		// ��� �������� ����� �����
		victim->desc->map_options.reset();
		STATE(victim->desc) = CON_PLAYING;
		message = true;
	}
	else if (victim->desc && (STATE(victim->desc) == CON_TORC_EXCH))
	{
		// ��� ������ ������ (������� ����� � ������)
		STATE(victim->desc) = CON_PLAYING;
		message = true;
	}

	if (message)
	{
		send_to_char(victim, "�� ��� ���� ��������� ���������, �������������� ��������!\r\n");
	}

	// �������. ������ ����. ���� ��� � ����, �� �� ������ ����, �� ������ ���������.
	if (MOB_FLAGGED(ch, MOB_MEMORY) && GET_WAIT(ch) > 0)
	{
		if (!IS_NPC(victim))
		{
			remember(ch, victim);
		}
		else if (AFF_FLAGGED(victim, AFF_CHARM) && victim->master && !IS_NPC(victim->master))
		{
			if (MOB_FLAGGED(victim, MOB_CLONE))
				remember(ch, victim->master);
			else if (IN_ROOM(victim->master) == IN_ROOM(ch) && CAN_SEE(ch, victim->master))
				remember(ch, victim->master);
		}
		return (FALSE);
	}

	hit(ch, victim, TYPE_UNDEFINED, AFF_FLAGGED(ch, AFF_STOPRIGHT) ? 2 : 1);
	set_wait(ch, 2, TRUE);
	return (TRUE);
};

int onhorse(CHAR_DATA * ch)
{
	if (on_horse(ch))
	{
		act("��� ������ $N.", FALSE, ch, 0, get_horse(ch), TO_CHAR);
		return (TRUE);
	}
	return (FALSE);
};

// Add by Voropay 8/05/2004
CHAR_DATA *try_protect(CHAR_DATA * victim, CHAR_DATA * ch)
{

	CHAR_DATA *vict;
	int percent = 0;
	int prob = 0;

	//Polud ���������� ������ �� ���������
	if (ch->get_fighting()==victim)
		return victim;

	for (vict = world[IN_ROOM(victim)]->people; vict; vict = vict->next_in_room)
	{
		if (vict->get_protecting() == victim &&
				!AFF_FLAGGED(vict, AFF_STOPFIGHT) &&
				!AFF_FLAGGED(vict, AFF_MAGICSTOPFIGHT) &&
				!AFF_FLAGGED(vict, AFF_BLIND) && !GET_MOB_HOLD(vict) && GET_POS(vict) >= POS_FIGHTING)
		{
			if (vict == ch)
			{
				act("�� ���������� ������� �� ����, ���� ����������, � ������� � �������� ������������.", FALSE, vict, 0, victim, TO_CHAR);
				act("$N �������� ������� �� ���! ����� �� ��� ������.", FALSE, victim, 0, vict, TO_CHAR);
				vict->set_protecting(0);
				CLR_AF_BATTLE(vict, EAF_PROTECT);
				WAIT_STATE(vict, PULSE_VIOLENCE);
				AFFECT_DATA af;
				af.type = SPELL_BATTLE;
				af.bitvector = AFF_STOPFIGHT;
				af.location = 0;
				af.modifier = 0;
				af.duration = pc_duration(vict, 1, 0, 0, 0, 0);
				af.battleflag = AF_BATTLEDEC | AF_PULSEDEC;
				affect_join(vict, &af, TRUE, FALSE, TRUE, FALSE);
				return victim;
			}
			percent = number(1, skill_info[SKILL_PROTECT].max_percent);
			prob = calculate_skill(vict, SKILL_PROTECT, skill_info[SKILL_PROTECT].max_percent, victim);
			prob = prob * 8 / 10;
			improove_skill(vict, SKILL_PROTECT, prob >= percent, ch);

			if (GET_GOD_FLAG(vict, GF_GODSCURSE))
				percent = 0;

			if ((vict->get_fighting() != ch) && (ch != victim))
			{
				// ����� ������ ����� ���� ����� ����� ��������� �������� �� ��� ����� ��������(������� �������)
				pk_agro_action(ch, victim);
				if (!may_kill_here(vict, ch))
					continue;
				// ����������� � ���������� ������������� ...
				stop_fighting(vict, FALSE);
				set_fighting(vict, ch);
			}

			if (prob < percent)
			{
				act("�� �� ������ �������� $N3.", FALSE, vict, 0, victim, TO_CHAR);
				act("$N �� ����$Q �������� ���.", FALSE, victim, 0, vict, TO_CHAR);
				act("$n �� ����$q �������� $N3.", TRUE, vict, 0, victim, TO_NOTVICT | TO_ARENA_LISTEN);
				set_wait(vict, 3, TRUE);
			}
			else
			{
				pk_agro_action(vict, ch); // �� �������� � ������ ��������� ����-�� ����� �����������
				act("�� ���������� �������� $N3, ������ ���� �� ����.", FALSE,
					vict, 0, victim, TO_CHAR);
				act("$N ���������� �������$G ���, ������ ���� �� ����.", FALSE,
					victim, 0, vict, TO_CHAR);
				act("$n ���������� �������$g $N3, ������ ���� �� ����.", TRUE,
					vict, 0, victim, TO_NOTVICT | TO_ARENA_LISTEN);
				set_wait(vict, 1, TRUE);
				return vict;
			}
		}
	}
	return victim;
}

void parry_override(CHAR_DATA * ch)
{
	const char *message = NULL;
	if (GET_AF_BATTLE(ch, EAF_BLOCK))
	{
		message = "�� ���������� ��������� �� ��� � ��������� � ���.";
		CLR_AF_BATTLE(ch, EAF_BLOCK);
	}
	if (GET_AF_BATTLE(ch, EAF_PARRY))
	{
		message = "�� ���������� ���������� ����� � ��������� � ���.";
		CLR_AF_BATTLE(ch, EAF_PARRY);
	}
	if (GET_AF_BATTLE(ch, EAF_MULTYPARRY))
	{
		message = "�� ������ � ������ � ��������� � ���.";
		CLR_AF_BATTLE(ch, EAF_MULTYPARRY);
	}
	if (message)
		act(message, FALSE, ch, 0, 0, TO_CHAR);
}

int used_attack(CHAR_DATA * ch)
{
	const char *message = NULL;

	parry_override(ch);

	if (!ch->get_extra_victim())
		return (FALSE);
	else
		switch (ch->get_extra_skill())
		{
		case SKILL_BASH:
			message = "����������. �� ��������� ����� $N3.";
			break;
		case SKILL_KICK:
			message = "����������. �� ��������� ����� $N3.";
			break;
		case SKILL_CHOPOFF:
			message = "����������. �� ��������� ������� $N3.";
			break;
		case SKILL_DISARM:
			message = "����������. �� ��������� ����������� $N3.";
			break;
		case SKILL_THROW:
			message = "����������. �� ��������� ������� ������ � $N3.";
			break;
		default:
			return (FALSE);
		}
	if (message)
		act(message, FALSE, ch, 0, ch->get_extra_victim(), TO_CHAR);
	return (TRUE);
}

ACMD(do_assist)
{
	CHAR_DATA *helpee, *opponent;

	if (ch->get_fighting())
	{
		send_to_char("����������. �� ���������� ����.\r\n", ch);
		return;
	}
	one_argument(argument, arg);

	if (!*arg)
	{
		for (helpee = world[ch->in_room]->people; helpee; helpee = helpee->next_in_room)
			if (helpee->get_fighting() && helpee->get_fighting() != ch && ((ch->master && ch->master == helpee->master)
					|| ch->master == helpee
					|| helpee->master == ch))
				break;
		if (!helpee)
		{
			send_to_char("���� �� ������ ������?\r\n", ch);
			return;
		}
	}
	else if (!(helpee = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}
	else if (helpee == ch)
	{
		send_to_char("��� ����� ������ ������ ����!\r\n", ch);
		return;
	}

	// * Hit the same enemy the person you're helping is.
	if (helpee->get_fighting())
		opponent = helpee->get_fighting();
	else
		for (opponent = world[ch->in_room]->people;
				opponent && (opponent->get_fighting() != helpee); opponent = opponent->next_in_room);

	if (!opponent)
		act("�� ����� �� ��������� � $N4!", FALSE, ch, 0, helpee, TO_CHAR);
	else if (!CAN_SEE(ch, opponent))
		act("�� �� ������ ���������� $N1!", FALSE, ch, 0, helpee, TO_CHAR);
	else if (opponent == ch)
		act("��� $E ��������� � ����!", FALSE, ch, 0, helpee, TO_CHAR);
	else if (!may_kill_here(ch, opponent))
		return;
	else if (need_full_alias(ch, opponent))
		act("����������� ������� '���������' ��� ��������� �� $N1.", FALSE, ch, 0, opponent, TO_CHAR);
	else if (set_hit(ch, opponent))
	{
		act("�� �������������� � �����, ������� $N2!", FALSE, ch, 0, helpee, TO_CHAR);
		act("$N �����$G ������ ��� � �����!", 0, helpee, 0, ch, TO_CHAR);
		act("$n �������$g � ��� �� ������� $N1.", FALSE, ch, 0, helpee, TO_NOTVICT | TO_ARENA_LISTEN);
	}
}


ACMD(do_hit)
{
	CHAR_DATA *vict;

	one_argument(argument, arg);

	if (!*arg)
		send_to_char("���� ����-�� �����?\r\n", ch);
	else if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
		send_to_char("�� �� ������ ����.\r\n", ch);
	else if (vict == ch)
	{
		send_to_char("�� ������� ����... ���� ������ ��!\r\n", ch);
		act("$n ������$g ����, � ������ �������$g '�������, ������ ����...'",
			FALSE, ch, 0, vict, TO_ROOM | CHECK_DEAF | TO_ARENA_LISTEN);
		act("$n ������$g ����", FALSE, ch, 0, vict, TO_ROOM | CHECK_NODEAF | TO_ARENA_LISTEN);
	}
	else if (!may_kill_here(ch, vict))
		return;
	else if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == vict))
		act("$N ������� ����� ��� ���, ����� ���� $S.", FALSE, ch, 0, vict, TO_CHAR);
	else
	{
		if (subcmd != SCMD_MURDER && !check_pkill(ch, vict, arg))
			return;
		if (ch->get_fighting())
		{
			if (vict == ch->get_fighting())
			{
				act("�� ��� ���������� � $N4.", FALSE, ch, 0, vict, TO_CHAR);
				return;
			}
			if (ch != vict->get_fighting())
			{
				act("$N �� ��������� � ����, �� �������� $S.", FALSE, ch, 0, vict, TO_CHAR);
				return;
			}
			vict = try_protect(vict, ch);
			stop_fighting(ch, 2); //������ �������������
			set_fighting(ch, vict);
			set_wait(ch, 2, TRUE);
		}
		else if ((GET_POS(ch) == POS_STANDING) && (vict != ch->get_fighting()))
		{
			set_hit(ch, vict);
		}
		else
			send_to_char("��� ���� �� �� ���!\r\n", ch);
	}
}



ACMD(do_kill)
{
	CHAR_DATA *vict;

	if (!IS_IMPL(ch))
	{
		do_hit(ch, argument, cmd, subcmd);
		return;
	}
	one_argument(argument, arg);

	if (!*arg)
	{
		send_to_char("���� �� ����� ������ ������-��?\r\n", ch);
	}
	else
	{
		if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
			send_to_char("� ��� ��� ����� :P.\r\n", ch);
		else if (ch == vict)
			send_to_char("�� ��������... :(\r\n", ch);
		else if (IS_IMPL(vict) || PRF_FLAGGED(vict, PRF_CODERINFO))
			send_to_char("� ���� �� ��� �������� ��������? �����, �������, �����!\r\n", ch);
		else
		{
			act("�� �������� $N3 � ����! ��������! �����!", FALSE, ch, 0, vict, TO_CHAR);
			act("$N �������$g ��� � ���� ����� ����������� ������!", FALSE, vict, 0, ch, TO_CHAR);
			act("$n ������ ���������$g �������� $N3!", FALSE, ch, 0, vict, TO_NOTVICT | TO_ARENA_LISTEN);
			raw_kill(vict, ch);
		}
	}
}

// *********************** BACKSTAB VICTIM
void go_backstab(CHAR_DATA * ch, CHAR_DATA * vict)
{
	int percent, prob;


	if (onhorse(ch))
		return;

	vict = try_protect(vict, ch);

	pk_agro_action(ch, vict);

	if (((MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) || (vict->get_fighting() && !can_use_feat(ch, THIEVES_STRIKE_FEAT)))
			&& !IS_GOD(ch))
	{
		act("�� ��������, ��� $N �������$u ��� ��������!", FALSE, vict, 0, ch, TO_CHAR);
		act("$n �������$g ���� ������� �������� $s!", FALSE, vict, 0, ch, TO_VICT);
		act("$n �������$g ������� $N1 �������� $s!", FALSE, vict, 0, ch, TO_NOTVICT | TO_ARENA_LISTEN);
		set_hit(vict, ch);
		return;
	}

	percent = number(1, skill_info[SKILL_BACKSTAB].max_percent);
	prob = train_skill(ch, SKILL_BACKSTAB, skill_info[SKILL_BACKSTAB].max_percent, vict);

	if (vict->get_fighting())
		prob = prob * (GET_REAL_DEX(ch) + 50) / 100;

	if (AFF_FLAGGED(ch, AFF_HIDE))
		prob += 5;	// Add by Alez - Improove in hide stab probability

	if (GET_MOB_HOLD(vict))
		prob = prob * 5 / 4;
	if (GET_GOD_FLAG(vict, GF_GODSCURSE))
		prob = percent;
	if (GET_GOD_FLAG(vict, GF_GODSLIKE) || GET_GOD_FLAG(ch, GF_GODSCURSE))
		prob = 0;

	if (percent > prob)
	{
		Damage dmg(SkillDmg(SKILL_BACKSTAB), 0, FightSystem::PHYS_DMG);
		dmg.process(ch, vict);
	}
	else
	{
		hit(ch, vict, SKILL_BACKSTAB, 1);
	}
	set_wait(ch, 2, TRUE);
}


ACMD(do_backstab)
{
	CHAR_DATA *vict;

	if (IS_NPC(ch) || !ch->get_skill(SKILL_BACKSTAB))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (onhorse(ch))
	{
		send_to_char("������ ��� ������� ��������������.\r\n", ch);
		return;
	}

	if (GET_POS(ch) < POS_FIGHTING)
	{
		send_to_char("��� ����� ������ �� ����.\r\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		send_to_char("���� �� ��� ������ ����������, ��� ������ ��������?\r\n", ch);
		return;
	}

	if (vict == ch)
	{
		send_to_char("��, �����������, ������������!\r\n", ch);
		return;
	}

	if (!GET_EQ(ch, WEAR_WIELD))
	{
		send_to_char("��������� ������� ������ � ������ ����.\r\n", ch);
		return;
	}

	if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != FightSystem::type_pierce)
	{
		send_to_char("�������� ����� ������ ������ �������!\r\n", ch);
		return;
	}

	if (AFF_FLAGGED(ch, AFF_STOPRIGHT) || AFF_FLAGGED(ch, AFF_STOPFIGHT)
			|| AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	if (vict->get_fighting() && !can_use_feat(ch, THIEVES_STRIKE_FEAT))
	{
		send_to_char("���� ���� ������� ������ �������� - �� ������ ����������!\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;
	go_backstab(ch, vict);
}


// ****************** CHARM ORDERS PROCEDURES
ACMD(do_order)
{
	char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
	bool found = FALSE;
	room_rnum org_room;
	CHAR_DATA *vict;
	struct follow_type *k, *k_next;

	half_chop(argument, name, message);
	if (GET_GOD_FLAG(ch, GF_GODSCURSE))
	{
		send_to_char("�� �������� ������ � ����� �� ��������� ���!\r\n", ch);
		return;
	}
	if (AFF_FLAGGED(ch, AFF_SIELENCE) || AFF_FLAGGED(ch, AFF_STRANGLED))
	{
		send_to_char("�� �� � ��������� ����������� ������.\r\n", ch);
		return;
	}
	if (!*name || !*message)
		send_to_char("��������� ��� � ����?\r\n", ch);
	else if (!(vict = get_char_vis(ch, name, FIND_CHAR_ROOM)) &&
			 !is_abbrev(name, "followers") && !is_abbrev(name, "���") && !is_abbrev(name, "����"))
		send_to_char("�� �� ������ ������ ���������.\r\n", ch);
	else if (ch == vict && !is_abbrev(name, "���") && !is_abbrev(name, "����"))
		send_to_char("�� ������ ������� ������������ ������ - ������ � ���������!\r\n", ch);
	else
	{
		if (vict && !IS_NPC(vict) && !IS_GOD(ch))
		{
			send_to_char(ch, "������� ����������� ����� ������ ����!\r\n");
			return;
		}
		if (AFF_FLAGGED(ch, AFF_CHARM))
		{
			send_to_char("� ����� ��������� �� �� ������ ���� �������� �������.\r\n", ch);
			return;
		}
		if (vict && !is_abbrev(name, "���") && !is_abbrev(name, "����")
				&& !is_abbrev(name, "followers"))
		{
			sprintf(buf, "$N ��������$g ��� '%s'", message);
			act(buf, FALSE, vict, 0, ch, TO_CHAR | CHECK_DEAF);
			act("$n �����$g ������ $N2.", FALSE, ch, 0, vict, TO_ROOM | CHECK_DEAF);

			if ((vict->master != ch) || !AFF_FLAGGED(vict, AFF_CHARM) || AFF_FLAGGED(vict, AFF_DEAFNESS))
			{
				if (!IS_POLY(vict))
					act("$n ����������� ������� �� ��������.", FALSE, vict, 0, 0, TO_ROOM);
				else
					act("$n ����������� ������� �� ��������.", FALSE, vict, 0, 0, TO_ROOM);
			}
			else
			{
				send_to_char(OK, ch);
				if (GET_WAIT_STATE(vict) <= 0)
					command_interpreter(vict, message);
				else if (vict->get_fighting())
				{
					if (vict->last_comm != NULL)
						free(vict->last_comm);
					vict->last_comm = str_dup(message);
				}
			}
		}
		else  	// This is order "followers"
		{
			org_room = ch->in_room;
			act("$n �����$g ������.", FALSE, ch, 0, 0, TO_ROOM | CHECK_DEAF);

			for (k = ch->followers; k; k = k_next)
			{
				k_next = k->next;
				if (org_room == k->follower->in_room)
					if (AFF_FLAGGED(k->follower, AFF_CHARM)
							&& !AFF_FLAGGED(k->follower, AFF_DEAFNESS))
					{
						found = TRUE;
						if (GET_WAIT_STATE(k->follower) <= 0)
							command_interpreter(k->follower, message);
						else if (k->follower->get_fighting())
						{
							if (k->follower->last_comm != NULL)
								free(k->follower->last_comm);
							k->follower->last_comm = str_dup(message);
						}
					}
			}
			if (found)
				send_to_char(OK, ch);
			else
				send_to_char("�� ��������� ������ �������!\r\n", ch);
		}
	}
}

// ********************* FLEE PROCEDURE
void go_flee(CHAR_DATA * ch)
{
	int i, attempt, loss, scandirs = 0, was_in = IN_ROOM(ch);
	CHAR_DATA *was_fighting;

	if (on_horse(ch) && GET_POS(get_horse(ch)) >= POS_FIGHTING && !GET_MOB_HOLD(get_horse(ch)))
	{
		if (!WAITLESS(ch))
			WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
		while (scandirs != (1 << NUM_OF_DIRS) - 1)
		{
			attempt = number(0, NUM_OF_DIRS - 1);
			if (IS_SET(scandirs, (1 << attempt)))
				continue;
			SET_BIT(scandirs, (1 << attempt));
			if (!legal_dir(ch, attempt, TRUE, FALSE) ||
					ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH))
				continue;
			//����� ��������, ����� �� ������� �� ������ � ������� � � ������� � ������ !������
			if (ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_TUNNEL) ||
					(ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_NOHORSE)))
				continue;
			was_fighting = ch->get_fighting();
			if (do_simple_move(ch, attempt | 0x80, TRUE, 0))
			{
				act("����$W $N �����$Q ��� �� ���.", FALSE, ch, 0, get_horse(ch), TO_CHAR);
				if (was_fighting && !IS_NPC(ch))
				{
					loss = MAX(1, GET_REAL_MAX_HIT(was_fighting) - GET_HIT(was_fighting));
					loss *= GET_LEVEL(was_fighting);
					if (!can_use_feat(ch, RETREAT_FEAT)  && !ROOM_FLAGGED(was_in, ROOM_ARENA))
						gain_exp(ch, -loss);
				}
				return;
			}
		}
		send_to_char("������ �������� ����. �� �� ������ �������!\r\n", ch);
		return;
	}

	if (GET_MOB_HOLD(ch))
		return;
	if (AFF_FLAGGED(ch, AFF_NOFLEE) ||AFF_FLAGGED(ch, AFF_LACKY) || IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND))
	{
		send_to_char("��������� ����� ������ ��� �������.\r\n", ch);
		return;
	}
	if (GET_WAIT(ch) > 0)
		return;
	if (GET_POS(ch) < POS_FIGHTING)
	{
		send_to_char("�� �� ������ ������� �� ����� ���������.\r\n", ch);
		return;
	}
	if (!WAITLESS(ch))
		WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
	for (i = 0; i < 6; i++)
	{
		attempt = number(0, NUM_OF_DIRS - 1);	// Select a random direction
		if (legal_dir(ch, attempt, TRUE, FALSE) && !ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH))
		{
			act("$n �����������$g � �����$u �������!", TRUE, ch, 0, 0, TO_ROOM | TO_ARENA_LISTEN);
			was_fighting = ch->get_fighting();
			if ((do_simple_move(ch, attempt | 0x80, TRUE, 0)))
			{
				send_to_char("�� ������ ������� � ���� �����.\r\n", ch);
				if (was_fighting && !IS_NPC(ch))
				{
					loss = MAX(1, GET_REAL_MAX_HIT(was_fighting) - GET_HIT(was_fighting));
					loss *= GET_LEVEL(was_fighting);
					if (!can_use_feat(ch, RETREAT_FEAT) && !ROOM_FLAGGED(was_in, ROOM_ARENA))
						gain_exp(ch, -loss);
				}
			}
			else
			{
				act("$n �����������$g � �������$u �������, �� �� ����$q!", FALSE, ch, 0, 0, TO_ROOM | TO_ARENA_LISTEN);
				send_to_char("������ �������� ����. �� �� ������ �������!\r\n", ch);
			}
			return;
		}
	}
	send_to_char("������ �������� ����. �� �� ������ �������!\r\n", ch);
}


void go_dir_flee(CHAR_DATA * ch, int direction)
{
	int attempt, loss, scandirs = 0, was_in = IN_ROOM(ch);
	CHAR_DATA *was_fighting;

	if (GET_MOB_HOLD(ch))
		return;
	if (AFF_FLAGGED(ch, AFF_NOFLEE) ||AFF_FLAGGED(ch, AFF_LACKY)|| IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND))
	{
		send_to_char("��������� ����� ������ ��� �������.\r\n", ch);
		return;
	}
	if (GET_WAIT(ch) > 0)
		return;
	if (GET_POS(ch) < POS_FIGHTING)
	{
		send_to_char("�� �� ������� ������� �� ����� ���������.\r\n", ch);
		return;
	}

	if (!(IS_IMMORTAL(ch) || GET_GOD_FLAG(ch, GF_GODSLIKE)))
		WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

	while (scandirs != (1 << NUM_OF_DIRS) - 1)
	{
		attempt = direction >= 0 ? direction : number(0, NUM_OF_DIRS - 1);
		direction = -1;
		if (IS_SET(scandirs, (1 << attempt)))
			continue;
		SET_BIT(scandirs, (1 << attempt));
		if (!legal_dir(ch, attempt, TRUE, FALSE) || ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH))
			continue;
		// ����� ��������, ����� �� ������� �� ������ � ������� � � ������� � ������ !������
		if (ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_TUNNEL) ||
				(ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_NOHORSE)))
			if (on_horse(ch))
				continue;
		act("$n �����������$g � �������$u �������.", FALSE, ch, 0, 0, TO_ROOM | TO_ARENA_LISTEN);
		was_fighting = ch->get_fighting();
		if (do_simple_move(ch, attempt | 0x80, TRUE, 0))
		{
			send_to_char("�� ������ ������� � ���� �����.\r\n", ch);
			if (was_fighting && !IS_NPC(ch))
			{
				loss = GET_REAL_MAX_HIT(was_fighting) - GET_HIT(was_fighting);
				loss *= GET_LEVEL(was_fighting);
				if (!can_use_feat(ch, RETREAT_FEAT) && !ROOM_FLAGGED(was_in, ROOM_ARENA))
					gain_exp(ch, -loss);
			}
			return;
		}
		else
			send_to_char("������ �������� ����! �� �� ������ c������.\r\n", ch);
	}
	send_to_char("������ �������� ����! �� �� ������ c������.\r\n", ch);
}


const char *FleeDirs[] = { "�����",
						   "������",
						   "��",
						   "�����",
						   "�����",
						   "����",
						   "\n"
						 };

ACMD(do_flee)
{
	int direction = -1;
	if (!ch->get_fighting())
	{
		send_to_char("�� �� ���� �� � ��� �� ����������!\r\n", ch);
		return;
	}
	if (can_use_feat(ch, CALMNESS_FEAT) || IS_IMMORTAL(ch)
			|| GET_GOD_FLAG(ch, GF_GODSLIKE))
	{
		one_argument(argument, arg);
		if ((direction = search_block(arg, dirs, FALSE)) >= 0 ||
				(direction = search_block(arg, FleeDirs, FALSE)) >= 0)
		{
			go_dir_flee(ch, direction);
			return;
		}
	}
	go_flee(ch);
}

void drop_from_horse(CHAR_DATA *victim)
{
	if (on_horse(victim))
	{
		act("�� ����� � $N1.", FALSE, victim, 0, get_horse(victim), TO_CHAR);
		REMOVE_BIT(AFF_FLAGS(victim, AFF_HORSE), AFF_HORSE);
	}
	if (IS_HORSE(victim) && on_horse(victim->master))
		horse_drop(victim);
}

// ************************* BASH PROCEDURES
void go_bash(CHAR_DATA * ch, CHAR_DATA * vict)
{
	int percent = 0, prob;

	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_STOPLEFT)
			|| AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	if (IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND))
	{
		send_to_char("�� �� ������ ��������� ���� ����� � ����� ���������!\r\n", ch);
		return;
	}

	if (onhorse(ch))
		return;

	if (ch == vict)
	{
		return;
	}

	if (!(IS_NPC(ch) ||	// ���
			GET_EQ(ch, WEAR_SHIELD) ||	// ���� ���
			IS_IMMORTAL(ch) ||	// �����������
			GET_MOB_HOLD(vict) ||	// ���� ���������
			GET_GOD_FLAG(vict, GF_GODSCURSE)	// ���� ��������
		 ))
	{
		send_to_char("�� �� ������ ������� ����� ��� ����.\r\n", ch);
		return;
	};

	if (GET_POS(ch) < POS_FIGHTING)
	{
		send_to_char("��� ����� ������ �� ����.\r\n", ch);
		return;
	}

	vict = try_protect(vict, ch);

	percent = number(1, skill_info[SKILL_BASH].max_percent);
	prob = train_skill(ch, SKILL_BASH, skill_info[SKILL_BASH].max_percent, vict);

	if (PRF_FLAGGED(ch, PRF_AWAKE))
		prob /= 2;
	if (GET_MOB_HOLD(vict))
		prob = percent;
	if (GET_GOD_FLAG(vict, GF_GODSCURSE))
		prob = percent;
	if (GET_GOD_FLAG(ch, GF_GODSCURSE))
		prob = 0;
	if (MOB_FLAGGED(vict, MOB_NOBASH))
		prob = 0;

	if (percent > prob)
	{
		Damage dmg(SkillDmg(SKILL_BASH), 0, FightSystem::PHYS_DMG);
		dmg.process(ch, vict);
		GET_POS(ch) = POS_SITTING;
		prob = 3;
	}
	else
	{
		/*
		 * If we bash a player and they wimp out, they will move to the previous
		 * room before we set them sitting.  If we try to set the victim sitting
		 * first to make sure they don't flee, then we can't bash them!  So now
		 * we only set them sitting if they didn't flee. -gg 9/21/98
		 */

		//�� ����� ������ ����� � ���� ������� ����, �������� � ������
		if (GET_POS(vict) <= POS_STUNNED && GET_WAIT(vict) > 0)
		{
			send_to_char("���� ������ � ��� ������� �����, ���� ���� �����������.\r\n", ch);
			set_wait(ch, 1, FALSE);
			return;
		}

		int dam = str_bonus(GET_REAL_STR(ch), STR_TO_DAM) + GET_REAL_DR(ch) +
				  MAX(0, ch->get_skill(SKILL_BASH) / 10 - 5) + GET_LEVEL(ch) / 5;
//      log("[BASH params] = actor = %s, actorlevel = %d, actordex = %d
//           target=  %s, targetlevel = %d, targetdex = %d ,skill = %d,
//           dice = %d, dam = %d", GET_NAME(ch), GET_LEVEL(ch), GET_REAL_DEX(ch),
//         GET_NAME(vict), GET_LEVEL(vict), GET_REAL_DEX(vict),
//         percent, prob, dam);
//������ ������������ ����
		if ((GET_AF_BATTLE(vict, EAF_BLOCK) || (can_use_feat(vict, DEFENDER_FEAT) && GET_EQ(vict, WEAR_SHIELD) && PRF_FLAGGED(vict, PRF_AWAKE) && vict->get_skill(SKILL_AWAKE) && vict->get_skill(SKILL_BLOCK) && GET_POS(vict) > POS_SITTING))
			&& !AFF_FLAGGED(vict, AFF_STOPFIGHT)
			&& !AFF_FLAGGED(vict, AFF_MAGICSTOPFIGHT)
			&& !AFF_FLAGGED(vict, AFF_STOPLEFT)
			&& GET_WAIT(vict) <= 0
			&& GET_MOB_HOLD(vict) == 0)
		{
			if (!(GET_EQ(vict, WEAR_SHIELD) ||
					IS_NPC(vict) || IS_IMMORTAL(vict) || GET_GOD_FLAG(vict, GF_GODSLIKE)))
				send_to_char("� ��� ����� �������� ����� ����������.\r\n", vict);
			else
			{
				int range, prob2;
				range = number(1, skill_info[SKILL_BLOCK].max_percent);
				prob2 = train_skill(vict, SKILL_BLOCK, skill_info[SKILL_BLOCK].max_percent, ch);
				if (prob2 < range)
				{
					act("�� �� ������ ����������� ������� $N1 ����� ���.",
						FALSE, vict, 0, ch, TO_CHAR);
					act("$N �� ����$Q ����������� ���� ������� ����� $S.",
						FALSE, ch, 0, vict, TO_CHAR);
					act("$n �� ����$q ����������� ������� $N1 ����� $s.",
						TRUE, vict, 0, ch, TO_NOTVICT | TO_ARENA_LISTEN);
				}
				else
				{
					act("�� ����������� ������� $N1 ����� ��� � ���.",
						FALSE, vict, 0, ch, TO_CHAR);
					act("�� ������ ����� $N1, �� ��$G ����������$G ���� �������.",
						FALSE, ch, 0, vict, TO_CHAR);
					act("$n ����������$g ������� $N1 ����� $s.",
						TRUE, vict, 0, ch, TO_NOTVICT | TO_ARENA_LISTEN);
					alt_equip(vict, WEAR_SHIELD, 30, 10);
					//���� ������� � ����, �� ��� ����������
					if (!ch->get_fighting())
					{
						set_fighting(ch, vict);
						set_wait(ch, 1, TRUE);
					}
					return;
				}
			}
		}
//������ ������������ ����

		prob = 0;

		Damage dmg(SkillDmg(SKILL_BASH), dam, FightSystem::PHYS_DMG);
		dmg.flags.set(FightSystem::NO_FLEE);
		dam = dmg.process(ch, vict);

		if (dam > 0 || (dam == 0 && AFF_FLAGGED(vict, AFF_SHIELD)))  	// -1 = dead, 0 = miss
		{
			prob = 3;
			if (IN_ROOM(ch) == IN_ROOM(vict))
			{
				GET_POS(vict) = POS_SITTING;
				drop_from_horse(vict);
			}
			set_wait(vict, prob, FALSE);
			prob = 2;
		}
	}
	set_wait(ch, prob, TRUE);
}

ACMD(do_bash)
{
	CHAR_DATA *vict = NULL;

	one_argument(argument, arg);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_BASH))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (onhorse(ch))
		return;

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (!*arg && ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
			vict = ch->get_fighting();
		else
		{
			send_to_char("���� �� �� ��� ������ ������� �����?\r\n", ch);
			return;
		}
	}

	if (vict == ch)
	{
		send_to_char("��� ����������� ���� ������ ��� ������... �� ������������� ���� �����.\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;

	if (IS_IMPL(ch) || !ch->get_fighting())
		go_bash(ch, vict);
	else if (!used_attack(ch))
	{
		act("������. �� ����������� ����� $N3.", FALSE, ch, 0, vict, TO_CHAR);
		ch->set_extra_attack(SKILL_BASH, vict);
	}
}

// ******************* RESCUE PROCEDURES
void go_rescue(CHAR_DATA * ch, CHAR_DATA * vict, CHAR_DATA * tmp_ch)
{
	int percent, prob;

	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	percent = number(1, skill_info[SKILL_RESCUE].max_percent);
	prob = calculate_skill(ch, SKILL_RESCUE, skill_info[SKILL_RESCUE].max_percent, tmp_ch);
	improove_skill(ch, SKILL_RESCUE, prob >= percent, tmp_ch);

	if (GET_GOD_FLAG(ch, GF_GODSLIKE))
		prob = percent;
	if (GET_GOD_FLAG(ch, GF_GODSCURSE))
		prob = 0;

	if (percent != skill_info[SKILL_RESCUE].max_percent && percent > prob)
	{
		act("�� ���������� �������� ������ $N3.", FALSE, ch, 0, vict, TO_CHAR);
		set_wait(ch, 1, FALSE);
		return;
	}

	act("����� �����, �� ���������� ������ $N3!", FALSE, ch, 0, vict, TO_CHAR);
	act("�� ���� ������� $N4. �� ���������� ���� �����!", FALSE, vict, 0, ch, TO_CHAR);
	act("$n ���������� ����$q $N3!", TRUE, ch, 0, vict, TO_NOTVICT | TO_ARENA_LISTEN);

	if (vict->get_fighting() == tmp_ch)
		stop_fighting(vict, FALSE);

	pk_agro_action(ch, tmp_ch);

	if (ch->get_fighting())
		ch->set_fighting(tmp_ch);
	else
		set_fighting(ch, tmp_ch);
	if (tmp_ch->get_fighting())
		tmp_ch->set_fighting(ch);
	else
		set_fighting(tmp_ch, ch);
	set_wait(ch, 1, FALSE);
	set_wait(vict, 2, FALSE);
}

ACMD(do_rescue)
{
	CHAR_DATA *vict, *tmp_ch;

	if (!ch->get_skill(SKILL_RESCUE))
	{
		send_to_char("�� �� �� ������ ���.\r\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		send_to_char("���� �� ������ ������?\r\n", ch);
		return;
	}

	if (vict == ch)
	{
		send_to_char("���� �������� �� ������ �������� ������ �����.\r\n", ch);
		return;
	}
	if (ch->get_fighting() == vict)
	{
		send_to_char("�� ��������� ������ ���������� ���?\r\n" "��� �� � ��� �� ������ ���� � ����?\r\n", ch);
		return;
	}

	for (tmp_ch = world[ch->in_room]->people; tmp_ch && (tmp_ch->get_fighting() != vict); tmp_ch = tmp_ch->next_in_room);

	if (!tmp_ch)
	{
		act("�� ����� �� ��������� � $N4!", FALSE, ch, 0, vict, TO_CHAR);
		return;
	}

	if (IS_NPC(vict) && tmp_ch && (!IS_NPC(tmp_ch) || (AFF_FLAGGED(tmp_ch, AFF_CHARM)
								   && tmp_ch->master && !IS_NPC(tmp_ch->master))) &&
			(!IS_NPC(ch)
			 || (AFF_FLAGGED(ch, AFF_CHARM) && ch->master && !IS_NPC(ch->master))))
	{
		send_to_char("�� ��������� ������ ������ ����������.\r\n", ch);
		return;
	}

	// �������� � ������ ������ �� � ������ � ���, ���� ���������� �������:
	// ���� ���, ��� ���������� ������� - "������" � � ���� ���������� ������
	if (AFF_FLAGGED(ch, AFF_CHARM) && ch->master != NULL)
	{
		// ���� ������� "�������", �� ��������� ���� �� ���������� � �����
		// ������ �������� ��������� � ����������.
		if (AFF_FLAGGED(vict, AFF_CHARM) && (vict->master != NULL) && !same_group(vict->master, ch->master))
		{
			act("������� �� �� ����� ������ �����.", FALSE, ch, 0, vict, TO_CHAR);
			act("�� �� ������ ������ ���� ���.", FALSE, ch->master, 0, vict, TO_CHAR);
			return;
		}
	}

	if (!may_kill_here(ch, tmp_ch))
		return;

	go_rescue(ch, vict, tmp_ch);
}

// ******************  KICK PROCEDURES
void go_kick(CHAR_DATA * ch, CHAR_DATA * vict)
{
	int percent, prob;

	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	if (onhorse(ch))
		return;

	vict = try_protect(vict, ch);

	// 101% is a complete failure
	percent = ((10 - (compute_armor_class(vict) / 10)) * 2) + number(1, skill_info[SKILL_KICK].max_percent);
	prob = train_skill(ch, SKILL_KICK, skill_info[SKILL_KICK].max_percent, vict);
	if (GET_GOD_FLAG(vict, GF_GODSCURSE) || GET_MOB_HOLD(vict) > 0)
		prob = percent;
	if (GET_GOD_FLAG(ch, GF_GODSCURSE) || on_horse(vict))
		prob = 0;

	if (percent > prob)
	{
		Damage dmg(SkillDmg(SKILL_KICK), 0, FightSystem::PHYS_DMG);
		dmg.process(ch, vict);
		prob = 2;
	}
	else
	{
		int dam = str_bonus(GET_REAL_STR(ch), STR_TO_DAM) + GET_REAL_DR(ch) + GET_LEVEL(ch) / 6;
//      int dam = str_bonus(GET_REAL_STR(ch), STR_TO_DAM) + (IS_NPC(ch) ? 0 : GET_REAL_DR(ch)) + GET_LEVEL(ch)/6;
		// ����������� �� ���� ����� �� ������ ��� PC (�������� �����������):
		//  0 -  50%
		//  5 -  75%
		// 10 - 100%
		// 20 - 150%
		// 30 - 200%
//      if ( !IS_NPC(ch) ){
		if (!IS_NPC(ch) || (IS_NPC(ch) && GET_EQ(ch, WEAR_FEET)))
		{
			int modi = MAX(0, (ch->get_skill(SKILL_KICK) + 4) / 5);
			dam += number(0, modi * 2);
			modi = 5 * (10 + (GET_EQ(ch, WEAR_FEET) ? GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_FEET)) : 0));
			dam = modi * dam / 100;
		}
//      log("[KICK damage] Name==%s dam==%d",GET_NAME(ch),dam);
		if (AFF_FLAGGED(ch, EAF_AWAKE))
			dam >>= 2;	// � 4 ���� ������

		Damage dmg(SkillDmg(SKILL_KICK), dam, FightSystem::PHYS_DMG);
		dmg.process(ch, vict);
		prob = 2;
	}
	set_wait(ch, prob, TRUE);
}

ACMD(do_kick)
{
	CHAR_DATA *vict = NULL;

	if (IS_NPC(ch) || !ch->get_skill(SKILL_KICK))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (onhorse(ch))
		return;

	one_argument(argument, arg);
	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (!*arg && ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
			vict = ch->get_fighting();
		else
		{
			send_to_char("��� ��� ��� ������ �������� ��� ������ ������?\r\n", ch);
			return;
		}
	}
	if (vict == ch)
	{
		send_to_char("�� ������ ����� ����! ����������, �� ����� � ���...\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;

	if (IS_IMPL(ch) || !ch->get_fighting())
		go_kick(ch, vict);
	else if (!used_attack(ch))
	{
		act("������. �� ����������� ����� $N3.", FALSE, ch, 0, vict, TO_CHAR);
		ch->set_extra_attack(SKILL_KICK, vict);
	}
}

// ******************* BLOCK PROCEDURES
void go_block(CHAR_DATA * ch)
{
	if (AFF_FLAGGED(ch, AFF_STOPLEFT))
	{
		send_to_char("���� ���� ������������.\r\n", ch);
		return;
	}
	SET_AF_BATTLE(ch, EAF_BLOCK);
	send_to_char("������, �� ���������� �������� ����� ��������� �����.\r\n", ch);
}

ACMD(do_block)
{
	if (IS_NPC(ch) || !ch->get_skill(SKILL_BLOCK))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}
	if (!ch->get_fighting())
	{
		send_to_char("�� �� �� � ��� �� ����������!\r\n", ch);
		return;
	};
	if (!(IS_NPC(ch) ||	// ���
			GET_EQ(ch, WEAR_SHIELD) ||	// ���� ���
			IS_IMMORTAL(ch) ||	// �����������
			GET_GOD_FLAG(ch, GF_GODSLIKE)	// ��������
		 ))
	{
		send_to_char("�� �� ������ ������� ��� ��� ����.\r\n", ch);
		return;
	}
	if (GET_AF_BATTLE(ch, EAF_BLOCK))
	{
		send_to_char("�� ��� ������������� �����!\r\n", ch);
		return;
	}
	go_block(ch);
}

// **************** MULTYPARRY PROCEDURES
void go_multyparry(CHAR_DATA * ch)
{
	if (AFF_FLAGGED(ch, AFF_STOPRIGHT) ||
			AFF_FLAGGED(ch, AFF_STOPLEFT) || AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	SET_AF_BATTLE(ch, EAF_MULTYPARRY);
	send_to_char("�� ���������� ������������ ������� ������.\r\n", ch);
}

ACMD(do_multyparry)
{
	OBJ_DATA *primary = GET_EQ(ch, WEAR_WIELD), *offhand = GET_EQ(ch, WEAR_HOLD);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_MULTYPARRY))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}
	if (!ch->get_fighting())
	{
		send_to_char("�� �� �� � ��� �� ����������?\r\n", ch);
		return;
	}
	if (!(IS_NPC(ch) ||	// ���
			(primary && GET_OBJ_TYPE(primary) == ITEM_WEAPON && offhand && GET_OBJ_TYPE(offhand) == ITEM_WEAPON) ||	// ��� ������
			IS_IMMORTAL(ch) ||	// �����������
			GET_GOD_FLAG(ch, GF_GODSLIKE)	// ��������
		 ))
	{
		send_to_char("�� �� ������ �������� ����� ����������.\r\n", ch);
		return;
	}
	if (GET_AF_BATTLE(ch, EAF_STUPOR))
	{
		send_to_char("����������! �� ���������� �������� ����������.\r\n", ch);
		return;
	}
	go_multyparry(ch);
}




// **************** PARRY PROCEDURES
void go_parry(CHAR_DATA * ch)
{
	if (AFF_FLAGGED(ch, AFF_STOPRIGHT) ||
			AFF_FLAGGED(ch, AFF_STOPLEFT) || AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	SET_AF_BATTLE(ch, EAF_PARRY);
	send_to_char("�� ���������� ��������� ��������� �����.\r\n", ch);
}

ACMD(do_parry)
{
	if (IS_NPC(ch) || !ch->get_skill(SKILL_PARRY))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}
	if (!ch->get_fighting())
	{
		send_to_char("�� �� �� � ��� �� ����������?\r\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch) && !GET_GOD_FLAG(ch, GF_GODSLIKE))
	{
		if (GET_EQ(ch, WEAR_BOTHS))
		{
			send_to_char("�� �� ������ ��������� ����� ��������� �������.\r\n", ch);
			return;
		}

		bool prim = 0, offh = 0;
		if (GET_EQ(ch, WEAR_WIELD) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_WEAPON)
			prim = 1;
		if (GET_EQ(ch, WEAR_HOLD) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_WEAPON)
			offh = 1;

		if (!prim && !offh)
		{
			send_to_char("�� �� ������ ��������� ����� ����������.\r\n", ch);
			return;
		}
		else if (!prim || !offh)
		{
			send_to_char("�� ������ ��������� ����� ������ � ����� �������� � �����.\r\n", ch);
			return;
		}
	}

	if (GET_AF_BATTLE(ch, EAF_STUPOR))
	{
		send_to_char("����������! �� ���������� �������� ����������.\r\n", ch);
		return;
	}
	go_parry(ch);
}

// ************** PROTECT PROCEDURES
void go_protect(CHAR_DATA * ch, CHAR_DATA * vict)
{
	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	ch->set_protecting(vict);
	act("�� ����������� �������� $N3 �� ���������.", FALSE, ch, 0, vict, TO_CHAR);
	SET_AF_BATTLE(ch, EAF_PROTECT);
}

ACMD(do_protect)
{
	CHAR_DATA *vict, *tch;

	one_argument(argument, arg);
	if (!*arg)
	{
		if (ch->get_protecting())
		{
			CLR_AF_BATTLE(ch, EAF_PROTECT);
			ch->set_protecting(0);
			send_to_char("�� ��������� ���������� ������ ��������.\r\n", ch);
		}else
		{
			send_to_char("�� ������ �� �����������.\r\n", ch);
		}
		return;
	}

	if (IS_NPC(ch) || !ch->get_skill(SKILL_PROTECT))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		send_to_char("� ��� ��� ������ ��� ������ ������?\r\n", ch);
		return;
	};

	if (vict == ch)
	{
		send_to_char("���������� ���������� ����� ��� ���������� �����.\r\n", ch);
		return;
	}

	if (ch->get_fighting() == vict)
	{
		send_to_char("�� ���� ��������, ��� ��������.\r\n", ch);
		return;
	}

	for (tch = world[IN_ROOM(ch)]->people; tch; tch = tch->next_in_room)
		if (tch->get_fighting() == vict)
			break;

	if (IS_NPC(vict) && tch && (!IS_NPC(tch) || (AFF_FLAGGED(tch, AFF_CHARM)
								&& tch->master && !IS_NPC(tch->master))) && (!IS_NPC(ch)
										||
										(AFF_FLAGGED
										 (ch, AFF_CHARM)
										 && ch->master
										 && !IS_NPC(ch->
													master))))
	{
		send_to_char("�� ��������� �������� ������ ����������.\r\n", ch);
		return;
	}

	for (tch = world[IN_ROOM(ch)]->people; tch; tch = tch->next_in_room)
	{
		if (tch->get_fighting() == vict && !may_kill_here(ch, tch))
			return;
	}
	go_protect(ch, vict);
}

// ************* TOUCH PROCEDURES
void go_touch(CHAR_DATA * ch, CHAR_DATA * vict)
{
	if (AFF_FLAGGED(ch, AFF_STOPRIGHT) || AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}
	act("�� ����������� ����������� ��������� ����� $N1.", FALSE, ch, 0, vict, TO_CHAR);
	SET_AF_BATTLE(ch, EAF_TOUCH);
	ch->set_touching(vict);
}

ACMD(do_touch)
{
	OBJ_DATA *primary = GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch, WEAR_WIELD) : GET_EQ(ch,
						WEAR_BOTHS);
	CHAR_DATA *vict = NULL;

	one_argument(argument, arg);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_TOUCH))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}
	if (!(IS_IMMORTAL(ch) ||	// �����������
			IS_NPC(ch) ||	// ���
			GET_GOD_FLAG(ch, GF_GODSLIKE) ||	// ��������
			!primary		// ��� ������
		 ))
	{
		send_to_char("� ��� ������ ����.\r\n", ch);
		return;
	}
	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		for (vict = world[IN_ROOM(ch)]->people; vict; vict = vict->next_in_room)
			if (vict->get_fighting() == ch)
				break;
		if (!vict)
		{
			if (!ch->get_fighting())
			{
				send_to_char("�� �� �� � ��� �� ����������.\r\n", ch);
				return;
			}
			else
				vict = ch->get_fighting();
		}
	}

	if (ch == vict)
	{
		send_to_char(GET_NAME(ch), ch);
		send_to_char(", �� ������ �� �������, �������� ����������� �����.\r\n", ch);
		return;
	}
	if (vict->get_fighting() != ch && ch->get_fighting() != vict)
	{
		act("�� �� �� ���������� � $N4.", FALSE, ch, 0, vict, TO_CHAR);
		return;
	}
	if (GET_AF_BATTLE(ch, EAF_MIGHTHIT))
	{
		send_to_char("����������. �� ������������� � ������������ �����.\r\n", ch);
		return;
	}

	if (!check_pkill(ch, vict, arg))
		return;

	parry_override(ch);

	go_touch(ch, vict);
}

// ************* DEVIATE PROCEDURES
void go_deviate(CHAR_DATA * ch)
{
	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}
	if (onhorse(ch))
		return;
	SET_AF_BATTLE(ch, EAF_DEVIATE);
	send_to_char("������, �� ����������� ���������� �� ��������� �����!\r\n", ch);
}

ACMD(do_deviate)
{
	if (IS_NPC(ch) || !ch->get_skill(SKILL_DEVIATE))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (!(ch->get_fighting()))
	{
		send_to_char("�� �� ���� �� � ��� �� ����������!\r\n", ch);
		return;
	}

	if (onhorse(ch))
		return;

	if (GET_AF_BATTLE(ch, EAF_DEVIATE))
	{
		send_to_char("�� � ��� ���������, ��� ������.\r\n", ch);
		return;
	};
	go_deviate(ch);
}

// ************* DISARM PROCEDURES
void go_disarm(CHAR_DATA * ch, CHAR_DATA * vict)
{
	int percent, prob, pos = 0;
	OBJ_DATA *wielded = GET_EQ(vict, WEAR_WIELD) ? GET_EQ(vict, WEAR_WIELD) :
						GET_EQ(vict, WEAR_BOTHS), *helded = GET_EQ(vict, WEAR_HOLD);

	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}
// shapirus: ������ ���������� ����� ���, ����� �����
	if (!((wielded && GET_OBJ_TYPE(wielded) != ITEM_LIGHT) || (helded && GET_OBJ_TYPE(helded) != ITEM_LIGHT)))
		return;
	if (number(1, 100) > 30)
		pos = wielded ? (GET_EQ(vict, WEAR_BOTHS) ? WEAR_BOTHS : WEAR_WIELD) : WEAR_HOLD;
	else
		pos = helded ? WEAR_HOLD : (GET_EQ(vict, WEAR_BOTHS) ? WEAR_BOTHS : WEAR_WIELD);

	if (!pos || !GET_EQ(vict, pos))
		return;

	percent = number(1, skill_info[SKILL_DISARM].max_percent);
	prob = train_skill(ch, SKILL_DISARM, skill_info[SKILL_DISARM].max_percent, vict);
	if (IS_IMMORTAL(ch) || GET_GOD_FLAG(vict, GF_GODSCURSE)
			|| GET_GOD_FLAG(ch, GF_GODSLIKE))
		prob = percent;
	if (IS_IMMORTAL(vict) || GET_GOD_FLAG(ch, GF_GODSCURSE) || GET_GOD_FLAG(vict, GF_GODSLIKE))
		prob = 0;


	if (percent > prob || OBJ_FLAGGED(GET_EQ(vict, pos), ITEM_NODISARM))
	{
		sprintf(buf, "%s�� �� ������ ����������� %s...%s\r\n",
				CCWHT(ch, C_NRM), GET_PAD(vict, 3), CCNRM(ch, C_NRM));
		send_to_char(buf, ch);
		// act("�� �� ������ ����������� $N1!",FALSE,ch,0,vict,TO_CHAR);
		prob = 3;
	}
	else
	{
		wielded = GET_EQ(vict, pos);
		sprintf(buf, "%s�� ����� ������ %s �� ��� %s...%s\r\n",
				CCIBLU(ch, C_NRM), wielded->PNames[3], GET_PAD(vict, 1), CCNRM(ch, C_NRM));
		send_to_char(buf, ch);
		// act("�� ����� ������ $o3 �� ��� $N1.",FALSE,ch,wielded,vict,TO_CHAR);
		// act("$n ����� �����$g $o3 �� ����� ���.", FALSE, ch, wielded, vict, TO_VICT);
		send_to_char(vict, "%s ����� �����%s %s%s �� ����� ���.\r\n",
			GET_PAD(ch, 0), GET_CH_VIS_SUF_1(ch, vict),
			wielded->PNames[3], char_get_custom_label(wielded, vict).c_str());
		act("$n ����� �����$g $o3 �� ��� $N1.", TRUE, ch, wielded, vict, TO_NOTVICT | TO_ARENA_LISTEN);
		unequip_char(vict, pos);
		if (GET_WAIT(vict) <= 0)
		{
			set_wait(vict, IS_NPC(vict) ? 1 : 2, FALSE);
		}
		prob = 2;
		if (ROOM_FLAGGED(IN_ROOM(vict), ROOM_ARENA))
			obj_to_char(wielded, vict);
		else
		{
			obj_to_room(wielded, IN_ROOM(vict));
			obj_decay(wielded);
		}
	}

	pk_agro_action(ch, vict);

	appear(ch);
	if (IS_NPC(vict) && CAN_SEE(vict, ch) && have_mind(vict) && GET_WAIT(ch) <= 0)
	{
		set_hit(vict, ch);
	}
	set_wait(ch, prob, FALSE);
}

ACMD(do_disarm)
{
	CHAR_DATA *vict = NULL;

	one_argument(argument, arg);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_DISARM))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (!*arg && ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
			vict = ch->get_fighting();
		else
		{
			send_to_char("���� �������������?\r\n", ch);
			return;
		}
	};

	if (ch == vict)
	{
		send_to_char(GET_NAME(ch), ch);
		send_to_char(", ���������� ������� \"����� <��������.������>\".\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;

// shapirus: ������ ���������� ����� ���, ����� �����
	if (!((GET_EQ(vict, WEAR_WIELD)
			&& GET_OBJ_TYPE(GET_EQ(vict, WEAR_WIELD)) != ITEM_LIGHT)
			|| (GET_EQ(vict, WEAR_HOLD)
				&& GET_OBJ_TYPE(GET_EQ(vict, WEAR_HOLD)) != ITEM_LIGHT)
			|| (GET_EQ(vict, WEAR_BOTHS)
				&& GET_OBJ_TYPE(GET_EQ(vict, WEAR_BOTHS)) != ITEM_LIGHT)))
	{
		send_to_char("�� �� ������ ����������� ���������� ��������.\r\n", ch);
		return;
	}

	if (IS_IMPL(ch) || !ch->get_fighting())
		go_disarm(ch, vict);
	else if (!used_attack(ch))
	{
		act("������. �� ����������� ���������� $N3.", FALSE, ch, 0, vict, TO_CHAR);
		ch->set_extra_attack(SKILL_DISARM, vict);
	}
}

// ************************* CHOPOFF PROCEDURES
void go_chopoff(CHAR_DATA * ch, CHAR_DATA * vict)
{
	int percent, prob;

	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	if (IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND))
	{
		send_to_char("�� �� ������ ��������� ���� ����� � ����� ���������!\r\n", ch);
		return;
	}

	if (onhorse(ch))
		return;

	// ������� �����: coded by �����
	if ((GET_POS(vict) < POS_FIGHTING))
	{
		if (number(1, 100) < ch->get_skill(SKILL_CHOPOFF))
		{
			send_to_char("�� ������������� �������� ��������, �� ������� ������������.\r\n", ch);
			set_wait(ch, 1, FALSE);
			return;
		}
	}

	percent = number(1, skill_info[SKILL_CHOPOFF].max_percent);
	prob = train_skill(ch, SKILL_CHOPOFF, skill_info[SKILL_CHOPOFF].max_percent, vict);

	if (GET_GOD_FLAG(ch, GF_GODSLIKE) || GET_MOB_HOLD(vict) > 0 || GET_GOD_FLAG(vict, GF_GODSCURSE))
		prob = percent;

	if (GET_GOD_FLAG(ch, GF_GODSCURSE) ||
			GET_GOD_FLAG(vict, GF_GODSLIKE) ||
			on_horse(vict) || GET_POS(vict) < POS_FIGHTING || MOB_FLAGGED(vict, MOB_NOTRIP) || IS_IMMORTAL(vict))
		prob = 0;

	if (percent > prob)
	{
		sprintf(buf, "%s�� ���������� ������� $N3, �� ����� ����...%s", CCWHT(ch, C_NRM), CCNRM(ch, C_NRM));
		act(buf,FALSE,ch,0,vict,TO_CHAR);
		act("$n �������$u ������� ���, �� ����$g ���$g.", FALSE, ch, 0, vict, TO_VICT);
		act("$n �������$u ������� $N3, �� ����$g ���$g.", TRUE, ch, 0, vict, TO_NOTVICT | TO_ARENA_LISTEN);
		GET_POS(ch) = POS_SITTING;
		prob = 3;
	}
	else
	{
		sprintf(buf, "%s�� ������� ��������, ����� ������ $N3 �� �����.%s", CCIBLU(ch, C_NRM), CCNRM(ch, C_NRM));
		act(buf,FALSE,ch,0,vict,TO_CHAR);
		act("$n ����� ������$q ���, ������ �� ����.", FALSE, ch, 0, vict, TO_VICT);
		act("$n ����� ������$q $N3, ������ $S �� �����.", TRUE, ch, 0, vict, TO_NOTVICT | TO_ARENA_LISTEN);
		set_wait(vict, 3, FALSE);
		if (IN_ROOM(ch) == IN_ROOM(vict))
			GET_POS(vict) = POS_SITTING;
		if (IS_HORSE(vict) && on_horse(vict->master))
			horse_drop(vict);
		prob = 1;
	}

	pk_agro_action(ch, vict);

	appear(ch);
	if (IS_NPC(vict) && CAN_SEE(vict, ch) && have_mind(vict) && GET_WAIT_STATE(vict) <= 0)
		set_hit(vict, ch);

	set_wait(ch, prob, FALSE);
}


ACMD(do_chopoff)
{
	CHAR_DATA *vict = NULL;

	one_argument(argument, arg);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_CHOPOFF))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (onhorse(ch))
		return;

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (!*arg && ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
			vict = ch->get_fighting();
		else
		{
			send_to_char("���� �� ����������� �������?\r\n", ch);
			return;
		}
	}

	if (vict == ch)
	{
		send_to_char("�� ������ ��������������� �������� <�����>.\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;

	if (IS_IMPL(ch) || !ch->get_fighting())
		go_chopoff(ch, vict);
	else if (!used_attack(ch))
	{
		act("������. �� ����������� ������� $N3.", FALSE, ch, 0, vict, TO_CHAR);
		ch->set_extra_attack(SKILL_CHOPOFF, vict);
	}
}

// ************************* STUPOR PROCEDURES
void go_stupor(CHAR_DATA * ch, CHAR_DATA * victim)
{
	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	if (IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND))
	{
		send_to_char("�� �� ������ ��������� ���� ����� � ����� ���������!\r\n", ch);
		return;
	}

	victim = try_protect(victim, ch);

	if (!ch->get_fighting())
	{
		SET_AF_BATTLE(ch, EAF_STUPOR);
		hit(ch, victim, SKILL_STUPOR, 1);
		set_wait(ch, 2, TRUE);
	}
	else
	{
		act("�� ����������� �������� $N3.", FALSE, ch, 0, victim, TO_CHAR);
		if (ch->get_fighting() != victim)
		{
			stop_fighting(ch, FALSE);
			set_fighting(ch, victim);
			set_wait(ch, 2, TRUE);
		}
		SET_AF_BATTLE(ch, EAF_STUPOR);
	}
}

ACMD(do_stupor)
{
	CHAR_DATA *vict = NULL;

	one_argument(argument, arg);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_STUPOR))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (!*arg && ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
			vict = ch->get_fighting();
		else
		{
			send_to_char("���� �� ������ ��������?\r\n", ch);
			return;
		}
	}

	if (vict == ch)
	{
		send_to_char("�� ������ �������, �������� ���� ����������� �����.\r\n", ch);
		return;
	}
//  if (GET_AF_BATTLE(ch, EAF_PARRY))
//     {send_to_char("����������. �� ������������� �� ���������� ����.\r\n", ch);
//      return;
//     }
	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;

	parry_override(ch);

	go_stupor(ch, vict);
}

// ************************* MIGHTHIT PROCEDURES
void go_mighthit(CHAR_DATA * ch, CHAR_DATA * victim)
{
	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	if (IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND))
	{
		send_to_char("�� �� ������ ��������� ���� ����� � ����� ���������!\r\n", ch);
		return;
	}

	victim = try_protect(victim, ch);

	if (!ch->get_fighting())
	{
		SET_AF_BATTLE(ch, EAF_MIGHTHIT);
		hit(ch, victim, SKILL_MIGHTHIT, 1);
		set_wait(ch, 2, TRUE);
	}
	else if ((victim->get_fighting() != ch) && (ch->get_fighting() != victim))
		act("$N �� ��������� � ����, �� �������� $S.", FALSE, ch, 0, victim, TO_CHAR);
	else
	{
		act("�� ����������� ������� ����������� ���� �� $N2.", FALSE, ch, 0, victim, TO_CHAR);
		if (ch->get_fighting() != victim)
		{
			stop_fighting(ch, 2); //������ �������������
			set_fighting(ch, victim);
			set_wait(ch, 2, TRUE);
		}
		SET_AF_BATTLE(ch, EAF_MIGHTHIT);
	}
}

ACMD(do_mighthit)
{
	CHAR_DATA *vict = NULL;

	one_argument(argument, arg);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_MIGHTHIT))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (!*arg && ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
			vict = ch->get_fighting();
		else
		{
			send_to_char("���� �� ������ ������ �������?\r\n", ch);
			return;
		}
	}

	if (vict == ch)
	{
		send_to_char("�� ������ ������� ����. �� �� � �� �����.\r\n", ch);
		return;
	}

	if (GET_AF_BATTLE(ch, EAF_TOUCH))
	{
		send_to_char("����������. �� ������������� �� ������� ����������.\r\n", ch);
		return;
	}
	if (!(IS_NPC(ch) || IS_IMMORTAL(ch)) &&
			(GET_EQ(ch, WEAR_BOTHS) || GET_EQ(ch, WEAR_WIELD) ||
			 GET_EQ(ch, WEAR_HOLD) || GET_EQ(ch, WEAR_SHIELD) || GET_EQ(ch, WEAR_LIGHT)))
	{
		send_to_char("���� ���������� ������ ��� ������� ����.\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;

	parry_override(ch);

	go_mighthit(ch, vict);
}

const char *cstyles[] = { "normal",
						  "�������",
						  "punctual",
						  "������",
						  "awake",
						  "����������",
						  "powerattack",
						  "�����������",
						  "grandpowerattack",
						  "���������������������",
						  "aimattack",
						  "���������������",
						  "grandaimattack",
						  "�������������������������",
						  "\n"
						};

ACMD(do_style)
{
	int tp;

	one_argument(argument, arg);



	if (!*arg)
	{
		sprintf(buf, "�� ���������� %s ������.\r\n",
				IS_SET(PRF_FLAGS(ch, PRF_PUNCTUAL),
					   PRF_PUNCTUAL) ? "������" : IS_SET(PRF_FLAGS(ch,
															   PRF_AWAKE),
															   PRF_AWAKE) ? "����������" : "�������");
		send_to_char(buf, ch);
		return;
	}
	if (((tp = search_block(arg, cstyles, FALSE)) == -1))
	{
		send_to_char("������: ����� { �������� ����� }\r\n", ch);
		return;
	}
	tp >>= 1;
	if ((tp == 1 && !ch->get_skill(SKILL_PUNCTUAL)) || (tp == 2 && !ch->get_skill(SKILL_AWAKE)))
	{
		send_to_char("��� ���������� ����� ����� ���.\r\n", ch);
		return;
	}
	if ((tp == 3 && !can_use_feat(ch, POWER_ATTACK_FEAT)) || (tp == 4 && !can_use_feat(ch, GREAT_POWER_ATTACK_FEAT)))
	{
		send_to_char("�� �� ������ ������������ ��� �����.\r\n", ch);
		return;
	}
	if ((tp == 5 && !can_use_feat(ch, AIMING_ATTACK_FEAT)) || (tp == 6 && !can_use_feat(ch, GREAT_AIMING_ATTACK_FEAT)))
	{
		send_to_char("�� �� ������ ������������ ��� �����.\r\n", ch);
		return;
	}
	switch (tp)
	{
	case 0:
	case 1:
	case 2:
		REMOVE_BIT(PRF_FLAGS(ch, PRF_PUNCTUAL), PRF_PUNCTUAL);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_AWAKE), PRF_AWAKE);

		SET_BIT(PRF_FLAGS(ch, PRF_PUNCTUAL), PRF_PUNCTUAL * (tp == 1));
		SET_BIT(PRF_FLAGS(ch, PRF_AWAKE), PRF_AWAKE * (tp == 2));

		if (ch->get_fighting() && !(AFF_FLAGGED(ch, AFF_COURAGE) ||
							  AFF_FLAGGED(ch, AFF_DRUNKED) || AFF_FLAGGED(ch, AFF_ABSTINENT)))
		{
			CLR_AF_BATTLE(ch, EAF_PUNCTUAL);
			CLR_AF_BATTLE(ch, EAF_AWAKE);
			if (tp == 1)
				SET_AF_BATTLE(ch, EAF_PUNCTUAL);
			else if (tp == 2)
				SET_AF_BATTLE(ch, EAF_AWAKE);
		}

		sprintf(buf, "�� ������� %s%s%s ����� ���.\r\n",
				CCRED(ch, C_SPR), tp == 0 ? "�������" : tp == 1 ? "������" : "����������", CCNRM(ch, C_OFF));
		break;
	case 3:
		REMOVE_BIT(PRF_FLAGS(ch, PRF_AIMINGATTACK), PRF_AIMINGATTACK);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_GREATAIMINGATTACK), PRF_GREATAIMINGATTACK);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_GREATPOWERATTACK), PRF_GREATPOWERATTACK);
		if (PRF_FLAGGED(ch, PRF_POWERATTACK))
		{
			REMOVE_BIT(PRF_FLAGS(ch, PRF_POWERATTACK), PRF_POWERATTACK);
			sprintf(buf, "%s�� ���������� ������������ ������ �����.%s\r\n",
					CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
		}
		else
		{
			SET_BIT(PRF_FLAGS(ch, PRF_POWERATTACK), PRF_POWERATTACK);
			sprintf(buf, "%s�� ������ ������������ ������ �����.%s\r\n",
					CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
		}
		break;
	case 4:
		REMOVE_BIT(PRF_FLAGS(ch, PRF_AIMINGATTACK), PRF_AIMINGATTACK);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_GREATAIMINGATTACK), PRF_GREATAIMINGATTACK);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_POWERATTACK), PRF_POWERATTACK);
		if (PRF_FLAGGED(ch, PRF_GREATPOWERATTACK))
		{
			REMOVE_BIT(PRF_FLAGS(ch, PRF_GREATPOWERATTACK), PRF_GREATPOWERATTACK);
			sprintf(buf, "%s�� ���������� ������������ ���������� ������ �����.%s\r\n",
					CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
		}
		else
		{
			SET_BIT(PRF_FLAGS(ch, PRF_GREATPOWERATTACK), PRF_GREATPOWERATTACK);
			sprintf(buf, "%s�� ������ ������������ ���������� ������ �����.%s\r\n",
					CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
		}
		break;
	case 5:
		REMOVE_BIT(PRF_FLAGS(ch, PRF_POWERATTACK), PRF_POWERATTACK);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_GREATPOWERATTACK), PRF_GREATPOWERATTACK);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_GREATAIMINGATTACK), PRF_GREATAIMINGATTACK);
		if (PRF_FLAGGED(ch, PRF_AIMINGATTACK))
		{
			REMOVE_BIT(PRF_FLAGS(ch, PRF_AIMINGATTACK), PRF_AIMINGATTACK);
			sprintf(buf, "%s�� ���������� ������������ ���������� �����.%s\r\n",
					CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
		}
		else
		{
			SET_BIT(PRF_FLAGS(ch, PRF_AIMINGATTACK), PRF_AIMINGATTACK);
			sprintf(buf, "%s�� ������ ������������ ���������� �����.%s\r\n",
					CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
		}
		break;
	case 6:
		REMOVE_BIT(PRF_FLAGS(ch, PRF_POWERATTACK), PRF_POWERATTACK);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_GREATPOWERATTACK), PRF_GREATPOWERATTACK);
		REMOVE_BIT(PRF_FLAGS(ch, PRF_AIMINGATTACK), PRF_AIMINGATTACK);
		if (PRF_FLAGGED(ch, PRF_GREATAIMINGATTACK))
		{
			REMOVE_BIT(PRF_FLAGS(ch, PRF_GREATAIMINGATTACK), PRF_GREATAIMINGATTACK);
			sprintf(buf, "%s�� ���������� ������������ ���������� ���������� �����.%s\r\n",
					CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
		}
		else
		{
			SET_BIT(PRF_FLAGS(ch, PRF_GREATAIMINGATTACK), PRF_GREATAIMINGATTACK);
			sprintf(buf, "%s�� ������ ������������ ���������� ���������� �����.%s\r\n",
					CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
		}
		break;
	}
	send_to_char(buf, ch);
	if (!WAITLESS(ch))
		WAIT_STATE(ch, PULSE_VIOLENCE);
}

// ***************** STOPFIGHT
ACMD(do_stopfight)
{
	CHAR_DATA *tmp_ch;

	if (!ch->get_fighting() || IS_NPC(ch))
	{
		send_to_char("�� �� �� �� � ��� �� ����������.\r\n", ch);
		return;
	}

	if (GET_POS(ch) < POS_FIGHTING)
	{
		send_to_char("�� ����� ��������� ��������� ����������.\r\n", ch);
		return;
	}

	if (IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND) || AFF_FLAGGED(ch, AFF_LACKY))
	{
		send_to_char("�� �� ������� ���������, �� ������������� �� ����� �������!\r\n", ch);
		return;
	}

	for (tmp_ch = world[IN_ROOM(ch)]->people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
		if (tmp_ch->get_fighting() == ch)
			break;

	if (tmp_ch)
	{
		send_to_char("����������, �� ���������� �� ���� �����.\r\n", ch);
		return;
	}
	else
	{
		stop_fighting(ch, TRUE);
		if (!(IS_IMMORTAL(ch) || GET_GOD_FLAG(ch, GF_GODSLIKE)))
			WAIT_STATE(ch, PULSE_VIOLENCE);
		send_to_char("�� ��������� �� �����.\r\n", ch);
		act("$n �����$g �� �����.", FALSE, ch, 0, 0, TO_ROOM | TO_ARENA_LISTEN);
	}
}

// ************* THROW PROCEDURES
void go_throw(CHAR_DATA * ch, CHAR_DATA * vict)
{
	int percent, prob;
	OBJ_DATA *wielded = GET_EQ(ch, WEAR_WIELD);

	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}

	if (!(wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON))
	{
		send_to_char("��� �� ������ �������?\r\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch) && !OBJ_FLAGGED(wielded, ITEM_THROWING))
	{
		act("$o �� ������������$A ��� �������.", FALSE, ch, wielded, 0, TO_CHAR);
		return;
	}

	vict = try_protect(vict, ch);

	percent = number(1, skill_info[SKILL_THROW].max_percent);
	prob = train_skill(ch, SKILL_THROW, skill_info[SKILL_THROW].max_percent, vict);
	if (IS_IMMORTAL(ch) || GET_GOD_FLAG(vict, GF_GODSCURSE)
			|| GET_GOD_FLAG(ch, GF_GODSLIKE))
		prob = percent;
	if (IS_IMMORTAL(vict) || GET_GOD_FLAG(ch, GF_GODSCURSE) || GET_GOD_FLAG(vict, GF_GODSLIKE))
		prob = 0;

	// log("Start throw");
	if (percent > prob)
	{
		Damage dmg(SkillDmg(SKILL_THROW), 0, FightSystem::PHYS_DMG);
		dmg.process(ch, vict);
	}
	else
		hit(ch, vict, SKILL_THROW, 1);
	// log("[THROW] Start extract weapon...");
	if (GET_EQ(ch, WEAR_WIELD))
	{
		wielded = unequip_char(ch, WEAR_WIELD);
		if (IN_ROOM(vict) != NOWHERE)
			obj_to_char(wielded, vict);
		else
			obj_to_room(wielded, IN_ROOM(ch));
		obj_decay(wielded);
	}
	// log("[THROW] Miss stop extract weapon...");
	set_wait(ch, 3, TRUE);
	// log("Stop throw");
}

ACMD(do_throw)
{
	CHAR_DATA *vict = NULL;

	one_argument(argument, arg);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_THROW))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
		{
			vict = ch->get_fighting();
		}
		else
		{
			send_to_char("� ���� �����?\r\n", ch);
			return;
		}
	};

	if (ch == vict)
	{
		send_to_char("�� ������, � �� ������ ������!\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;

	if (IS_IMPL(ch) || !ch->get_fighting())
		go_throw(ch, vict);
	else if (!used_attack(ch))
	{
		act("������. �� ����������� ������� ������ � $N3.", FALSE, ch, 0, vict, TO_CHAR);
		ch->set_extra_attack(SKILL_THROW, vict);
	}
}

ACMD(do_manadrain)
{
	CHAR_DATA *vict;
	struct timed_type timed;
	int drained_mana, prob, percent, skill;

	one_argument(argument, arg);

	if (IS_NPC(ch) || !ch->get_skill(SKILL_MANADRAIN))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch) && timed_by_skill(ch, SKILL_MANADRAIN))
	{
		send_to_char("��� ����� �� ���������.\r\n", ch);
		return;
	}

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
		{
			vict = ch->get_fighting();
		}
		else
		{
			send_to_char("� � ��� �� ������� ��������� �����?\r\n", ch);
			return;
		}
	};

	if (ch == vict)
	{
		send_to_char("�� ������� ���� �� ����� ���.\r\n", ch);
		return;
	}

	if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOBATTLE))
	{
		send_to_char("������� ������ ����� ��� ��������� ����� ����������� ������������.\r\n", ch);
		return;
	}

	if (!IS_NPC(vict))
	{
		send_to_char("�� ����� ��������? ������ �� ��� ���!\r\n", ch);
		return;
	}

	if (affected_by_spell(vict, SPELL_SHIELD) || MOB_FLAGGED(vict, MOB_PROTECT))
	{
		send_to_char("���� ������ ���� ������.\r\n", ch);
		return;
	}

	skill = ch->get_skill(SKILL_MANADRAIN);

	percent = number(1, skill_info[SKILL_MANADRAIN].max_percent);
	// ����������� ������ - 90% - 5% �� ������ ������� ������ ������ ������ ���� 20% ���.
	prob = MAX(20, 90 - 5 * MAX(0, GET_LEVEL(vict) - GET_LEVEL(ch)));
	improove_skill(ch, SKILL_MANADRAIN, percent > prob, vict);

	if (percent > prob)
	{
		Damage dmg(SkillDmg(SKILL_MANADRAIN), 0, FightSystem::MAGE_DMG);
		dmg.process(ch, vict);
	}
	else
	{
		// % ��������������� ���� - %������ - 10% �� ������ ������� ������ ������ ������ ���� - 10% ���.
		skill = MAX(10, skill - 10 * MAX(0, GET_LEVEL(ch) - GET_LEVEL(vict)));
		drained_mana = (GET_MAX_MANA(ch) - GET_MANA_STORED(ch)) * skill / 100;
		GET_MANA_STORED(ch) = MIN(GET_MAX_MANA(ch), GET_MANA_STORED(ch) + drained_mana);

		Damage dmg(SkillDmg(SKILL_MANADRAIN), 10, FightSystem::MAGE_DMG);
		dmg.process(ch, vict);
	}

	if (!IS_IMMORTAL(ch))
	{
		timed.skill = SKILL_MANADRAIN;
		timed.time = 6;
		timed_to_char(ch, &timed);
	}

}

extern char cast_argument[MAX_INPUT_LENGTH];
ACMD(do_townportal)
{

	struct char_portal_type *tmp, *dlt = NULL;
	char arg2[MAX_INPUT_LENGTH];
	int vnum = 0;

	if (IS_NPC(ch) || !ch->get_skill(SKILL_TOWNPORTAL))
	{
		send_to_char("������ ������� ������ ���������� ����.\r\n", ch);
		return;
	}

	two_arguments(argument, arg, arg2);
	if (!str_cmp(arg, "������"))
	{
		vnum = find_portal_by_word(arg2);
		for (tmp = GET_PORTALS(ch); tmp; tmp = tmp->next)
		{
			if (tmp->vnum == vnum)
			{
				if (dlt)
				{
					dlt->next = tmp->next;
				}
				else
				{
					GET_PORTALS(ch) = tmp->next;
				}
				free(tmp);
				sprintf(buf, "�� ��������� ������, ��� �������� ������ '&R%s&n'.\r\n", arg2);
				send_to_char(buf, ch);
				break;
			}
			dlt = tmp;
		}
		return;
	}

	*cast_argument = '\0';
	if (argument)
		strcat(cast_argument, arg);
	spell_townportal(GET_LEVEL(ch), ch, NULL, NULL);

}

// Added by Gorrah
ACMD(do_turn_undead)
{
	int percent, dam = 0;
	int sum, max_level;
	struct timed_type timed;
	vector<CHAR_DATA*> ch_list;
	CHAR_DATA *ch_vict;

	if (IS_NPC(ch))		// Cannot use on mobs.
		return;

	if (!ch->get_skill(SKILL_TURN_UNDEAD))
	{
		send_to_char("��� ��� �� �� �����.\r\n", ch);
		return;
	}

	if (timed_by_skill(ch, SKILL_TURN_UNDEAD))
	{
		send_to_char("��� ������ �� �� ����� �������� ������, ����� ���������.\r\n", ch);
		return;
	}

	timed.skill = SKILL_TURN_UNDEAD;
	timed.time = IS_PALADINE(ch) ? 6 : 8;
	if (can_use_feat(ch, EXORCIST_FEAT))
		timed.time -= 2;
	timed_to_char(ch, &timed);

	send_to_char(ch, "�� ����� ���� � ���������� ����� � �������� ������� ����� ���� �����.\r\n");
	act("$n ����$g ���� � ���������� ����� � �������� ������� ����� ���� �����.\r\n", FALSE, ch, 0, 0, TO_ROOM | TO_ARENA_LISTEN);

//���������� ������ ���� ������ � ������� � ���������� ������������� � ��-������
	for (ch_vict = world[ch->in_room]->people; ch_vict; ch_vict = ch_vict->next_in_room)
	{
		if (IS_IMMORTAL(ch_vict))
			continue;
		if (!HERE(ch_vict))
			continue;
		if (same_group(ch, ch_vict))
			continue;
		if (!(IS_UNDEAD(ch_vict) || GET_RACE(ch_vict) == NPC_RACE_GHOST))
			continue;
		if (!may_kill_here(ch, ch_vict))
			return;
		ch_list.push_back(ch_vict);
	}

	if (ch_list.size() > 0)
		percent = ch->get_skill(SKILL_TURN_UNDEAD);
	else
	{
		ch_list.clear();
		return;
	}

//���������� ������������ ������� ���������� ������
	if (number(1, skill_info[SKILL_TURN_UNDEAD].max_percent) <= percent)
		max_level = GET_LEVEL(ch) + number(1, percent) / 10 + 5;
	else
		max_level = GET_LEVEL(ch) - number(1, 5);
	sum = dice(3, 8) + GET_LEVEL(ch) + percent / 2;

//���������.
//���� ������� ������ �������������, ��� ����������� - ���� �� ����� �����
//���� ��������� - �� �����+�����, ���� �� ������ ������ ���� - ������ �����.
	for (vector<CHAR_DATA *>::iterator it=ch_list.begin();it!=ch_list.end();++it)
	{
		if (sum <= 0)
			break;
		ch_vict = *it;
		if (IN_ROOM(ch) == NOWHERE || IN_ROOM(ch_vict) == NOWHERE)
			continue;
		if ((GET_LEVEL(ch_vict) > max_level) ||
				(dice(1, GET_SAVE(ch_vict, SAVING_STABILITY) + GET_REAL_CON(ch_vict)) >
				 dice(1, GET_REAL_WIS(ch))))
		{
			train_skill(ch, SKILL_TURN_UNDEAD, skill_info[SKILL_TURN_UNDEAD].max_percent, ch_vict);
			pk_agro_action(ch, ch_vict);

			Damage dmg(SkillDmg(SKILL_TURN_UNDEAD), 0, FightSystem::MAGE_DMG);
			dmg.flags.set(FightSystem::IGNORE_FSHIELD);
			dmg.process(ch, ch_vict);
			continue;
		}
		sum -= GET_LEVEL(ch_vict);
		if (GET_LEVEL(ch) - 8 >= GET_LEVEL(ch_vict))
		{
			dam = MAX(1, GET_HIT(ch_vict) + 11);
		}
		else
		{
			if (IS_CLERIC(ch))
				dam = dice(8, 3 * GET_REAL_WIS(ch)) + GET_LEVEL(ch);
			else
				dam = dice(8, 4 * GET_REAL_WIS(ch) + GET_REAL_INT(ch)) + GET_LEVEL(ch);
		}
		train_skill(ch, SKILL_TURN_UNDEAD, skill_info[SKILL_TURN_UNDEAD].max_percent, ch_vict);

		Damage dmg(SkillDmg(SKILL_TURN_UNDEAD), dam, FightSystem::MAGE_DMG);
		dmg.flags.set(FightSystem::IGNORE_FSHIELD);
		dmg.process(ch, ch_vict);

		if (!MOB_FLAGGED(ch_vict, MOB_NOFEAR) &&
				!general_savingthrow(ch, ch_vict, SAVING_WILL, GET_REAL_WIS(ch) + GET_REAL_INT(ch)))
			go_flee(ch_vict);
	}

}

// ������ "�������� �����"
void go_iron_wind(CHAR_DATA * ch, CHAR_DATA * victim)
{
	OBJ_DATA *weapon;

	if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
		return;
	}
	if (GET_POS(ch) < POS_FIGHTING)
	{
		send_to_char("��� ����� ������ �� ����.\r\n", ch);
		return;
	}
	if (IS_SET(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND))
	{
		send_to_char("�� ��� ����� � �����������.\r\n", ch);
		return;
	}
	if (ch->get_fighting() && (ch->get_fighting() != victim))
	{
		act("$N �� ��������� � ����, �� �������� $S.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	parry_override(ch);

	//(void) train_skill(ch, SKILL_IRON_WIND, skill_info[SKILL_IRON_WIND].max_percent, 0);

	act("��� ������ ������� ���, � �� ��������� �� $N3!\r\n", FALSE, ch, 0, victim, TO_CHAR);
	if ((weapon = GET_EQ(ch, WEAR_WIELD)) || (weapon = GET_EQ(ch, WEAR_BOTHS)))
	{
		strcpy(buf, "$n �������$g � �����$u �� $N3, ������� ���������� $o4!");
		strcpy(buf2, "$N �������$G � �����$U �� ���, ������� ���������� $o4!");
	}
	else
	{
		strcpy(buf, "$n ������� �������$g � �����$u �� $N3!");
		strcpy(buf2, "$N ������� �������$G � �����$U �� ���!");
	};
	act(buf, FALSE, ch, weapon, victim, TO_NOTVICT | TO_ARENA_LISTEN);
	act(buf2, FALSE, victim, weapon, ch, TO_CHAR);

	if (!ch->get_fighting())
	{
		SET_BIT(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND);
		SET_AF_BATTLE(ch, EAF_IRON_WIND);
		hit(ch, victim, TYPE_UNDEFINED, 1);
		set_wait(ch, 2, TRUE);
	}
	else
	{
		SET_BIT(PRF_FLAGS(ch, PRF_IRON_WIND), PRF_IRON_WIND);
		SET_AF_BATTLE(ch, EAF_IRON_WIND);
	}
}

ACMD(do_iron_wind)
{
	CHAR_DATA *vict = NULL;
	int moves;


	if (IS_NPC(ch) || !ch->get_skill(SKILL_IRON_WIND))
	{
		send_to_char("�� �� ������ ���.\r\n", ch);
		return;
	};
	if (GET_AF_BATTLE(ch, EAF_STUPOR) || GET_AF_BATTLE(ch, EAF_MIGHTHIT))
	{
		send_to_char("����������! �� ������ ������ ����!\r\n", ch);
		return;
	};
	moves = GET_MAX_MOVE(ch) / (2 + MAX(15, ch->get_skill(SKILL_IRON_WIND)) / 15);
	if (GET_MAX_MOVE(ch) < moves * 2)
	{
		send_to_char("�� ������� ������...\r\n", ch);
		return;
	}
	if (!AFF_FLAGGED(ch, AFF_DRUNKED) && !IS_IMMORTAL(ch) && !GET_GOD_FLAG(ch, GF_GODSLIKE))
	{
		send_to_char("�� ������� ������������� ��� �����...\r\n", ch);
		return;
	};

	one_argument(argument, arg);
	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		if (!*arg && ch->get_fighting() && IN_ROOM(ch) == IN_ROOM(ch->get_fighting()))
			vict = ch->get_fighting();
		else
		{
			send_to_char("���� ��� ������ �������� � �������?\r\n", ch);
			return;
		}
	}
	if (vict == ch)
	{
		send_to_char("�� � �������� ������������ ����������� ����� ������� �����... ��������.\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;

	go_iron_wind(ch, vict);
}

void go_strangle(CHAR_DATA * ch, CHAR_DATA * vict)
{
	int percent, prob, dam, delay;
	int visibl=0, aware=0, awake=0, react=0;
	AFFECT_DATA af;
	struct timed_type timed;

	if (AFF_FLAGGED(ch, AFF_STOPRIGHT) || AFF_FLAGGED(ch, AFF_STOPFIGHT)
			|| AFF_FLAGGED(ch, AFF_MAGICSTOPFIGHT))
	{
		send_to_char("������ � ��� �� ��������� ��������� ���� �����.\r\n", ch);
		return;
	}

	if (ch->get_fighting())
	{
		send_to_char("�� �� ������ ������ ��� � ���!\r\n", ch);
		return;
	}

	if (GET_POS(ch) < POS_FIGHTING)
	{
		send_to_char("��� ����� ������ �� ����.\r\n", ch);
		return;
	}

	vict = try_protect(vict, ch);
	pk_agro_action(ch, vict);

	act("�� ���������� �������� ������ �� ��� $N2.\r\n", FALSE, ch, 0, vict, TO_CHAR);

	prob = train_skill(ch, SKILL_STRANGLE, skill_info[SKILL_STRANGLE].max_percent, vict);
	delay = 6 - MIN(4, (ch->get_skill(SKILL_STRANGLE) + 30) / 50);
	percent = number(1, skill_info[SKILL_STRANGLE].max_percent);

	if (prob > 100)
		prob = 100+(prob-100)/2; //�� 0 �� 150 � ����������� �� % ������

	prob += MAX(0, GET_REAL_DEX(ch)-25); //����� �� ������ ���� ����� ������ 25

	if (GET_MOB_HOLD(vict))
		prob += prob/2;
	else
	{
		if (!CAN_SEE(ch,vict))
			visibl = prob/5;
		if (vict->get_fighting() ||
			((MOB_FLAGGED(vict, MOB_AWARE) || AFF_FLAGGED(vict, AFF_AWARNESS)) && AWAKE(vict) && !IS_GOD(ch)))
			aware = -prob/2;
		if (AFF_FLAGGED (vict, PRF_AWAKE))
			awake = -(vict->get_skill(SKILL_AWAKE)/5);
		react = -2*GET_SAVE(vict, SAVING_REFLEX);
		prob = MAX(0,prob+visibl+aware+awake+react);
	}
	if (GET_GOD_FLAG(vict, GF_GODSCURSE))
		prob = percent;
	if (GET_GOD_FLAG(vict, GF_GODSLIKE) || GET_GOD_FLAG(ch, GF_GODSCURSE) ||
		affected_by_spell(vict, SPELL_SHIELD) || MOB_FLAGGED(vict, MOB_PROTECT))
		prob = 0;

//������ ����� - ���� ���������, ���� �� ���.
	//double mean = 21-1/(0.25+((4*sqrt(11)-1)/320)*ch->get_skill(SKILL_STRANGLE));
	//mean = (300+5*ch->get_skill(SKILL_STRANGLE))/70;
	//awake = GaussIntNumber((300+5*ch->get_skill(SKILL_STRANGLE))/70, 7.0, 1, 30);
	//dam = (GET_MAX_HIT(vict)*GaussIntNumber((300+5*ch->get_skill(SKILL_STRANGLE))/70, 7.0, 1, 30))/100;
	//sprintf(buf1, "Gauss result mean = %f, sigma 7.0, percent  %d, damage %d", mean, awake, dam);
	//mudlog(buf1, LGH, LVL_IMMORT, SYSLOG, TRUE);

	if (percent > prob)
	{
		//act("Strangle failed.\r\n", FALSE, ch, 0, vict, TO_CHAR);
		Damage dmg(SkillDmg(SKILL_STRANGLE), 0, FightSystem::PHYS_DMG);
		dmg.flags.set(FightSystem::IGNORE_ARMOR);
		dmg.process(ch, vict);
		set_wait(ch, 3, TRUE);
	}
	else
	{
		//act("Strangle success.\r\n", FALSE, ch, 0, vict, TO_CHAR);
		af.type = SPELL_STRANGLE;
		af.duration = IS_NPC(vict) ? 8 : 15;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.battleflag = AF_SAME_TIME;
		af.bitvector = AFF_STRANGLED;
		affect_to_char(vict, &af);

		//���� �������������� ���������. ����������� ������� ��������� � �������� ������. ����� ��������� ����������������.
		//���� ��������� � ��������� �� ������������� ����� ����� ������.
		dam = (GET_MAX_HIT(vict)*GaussIntNumber((300+5*ch->get_skill(SKILL_STRANGLE))/70, 7.0, 1, 30))/100;
		//����������� ����� ������: �� ����� �������� �����*2, �� ����� *6
		dam = (IS_NPC(vict) ? MIN(dam, 6*GET_MAX_HIT(ch)) : MIN(dam, 2*GET_MAX_HIT(ch)));
		Damage dmg(SkillDmg(SKILL_STRANGLE), dam, FightSystem::PHYS_DMG);
		dmg.flags.set(FightSystem::IGNORE_ARMOR);
		dmg.process(ch, vict);
		if (GET_POS(vict) > POS_DEAD)
		{
			set_wait(ch, 2, TRUE);
			set_wait(vict, 2, TRUE); //�� ������ ���� ��� ����� ��� ���� �� �������
			if (on_horse(vict))
			{
				act("������ �� ����, $N ������$G ��� �� �����.", FALSE, vict, 0, ch, TO_CHAR);
				act("������ �� ����, �� ������� $n3 �� �����.", FALSE, vict, 0, ch, TO_VICT);
				act("������ �� ����, $N ������$G $n3 �� �����.", FALSE, vict, 0, ch, TO_NOTVICT | TO_ARENA_LISTEN);
				REMOVE_BIT(AFF_FLAGS(vict, AFF_HORSE), AFF_HORSE);
			}
			if (ch->get_skill(SKILL_CHOPOFF) && (IN_ROOM(ch) == IN_ROOM(vict)))
				go_chopoff(ch, vict);
		}
	}

	if (!IS_IMMORTAL(ch))
	{
		timed.skill = SKILL_STRANGLE;
		timed.time = delay;
		timed_to_char(ch, &timed);
	}
}

ACMD(do_strangle)
{
	CHAR_DATA *vict;

	if (IS_NPC(ch) || !ch->get_skill(SKILL_STRANGLE))
	{
		send_to_char("�� �� ������ �����.\r\n", ch);
		return;
	}

	if (timed_by_skill(ch, SKILL_STRANGLE))
	{
		send_to_char("��� ����� ������ ������ - �������� ��������.\r\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
	{
		send_to_char("���� �� ������� �������?\r\n", ch);
		return;
	}

	if (AFF_FLAGGED(vict, AFF_STRANGLED))
	{
		send_to_char("���� ������ ��������� ������ �� ����� - �� �����������!\r\n", ch);
		return;
	}

	if (IS_UNDEAD(vict)
		|| GET_RACE(vict) == NPC_RACE_FISH
		|| GET_RACE(vict) == NPC_RACE_PLANT
		|| GET_RACE(vict) == NPC_RACE_THING
		|| GET_RACE(vict) == NPC_RACE_GHOST)
	{
		send_to_char("�� �� ��� ��������� ����� ������� �����������...\r\n", ch);
		return;
	}

	if (vict == ch)
	{
		send_to_char("�������������� �������� ���������� ������.\r\n���������� �������� - ������!\r\n", ch);
		return;
	}

	if (!may_kill_here(ch, vict))
		return;
	if (!check_pkill(ch, vict, arg))
		return;
	go_strangle(ch, vict);
}

