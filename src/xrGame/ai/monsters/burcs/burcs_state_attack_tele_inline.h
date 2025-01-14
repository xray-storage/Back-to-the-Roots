#pragma once

#include "../../../level.h"

#define CStateBurcsAttackTeleAbstract CStateBurcsAttackTele<_Object>


#define GOOD_DISTANCE_FOR_TELE	15.f
#define TELE_DELAY				4000

#define MAX_TIME_CHECK_FAILURE	6000

template <typename _Object>
CStateBurcsAttackTeleAbstract::CStateBurcsAttackTele(_Object *obj) : inherited(obj)
{

}

template <typename _Object>
void CStateBurcsAttackTeleAbstract::initialize()
{
	inherited::initialize		();

	m_action					= ACTION_TELE_STARTED;
	selected_object				= 0;

	SelectObjects				();

	time_started				= 0;

	// ��������� ������ ��������
	object->set_script_capture	(false);

}

template <typename _Object>
void CStateBurcsAttackTeleAbstract::execute()
{
	switch (m_action) {
		/************************/
		case ACTION_TELE_STARTED:
			/************************/

			ExecuteTeleStart();
			m_action = ACTION_TELE_CONTINUE;

			break;
			/************************/
		case ACTION_TELE_CONTINUE:
			/************************/

			ExecuteTeleContinue();

			break;

			/************************/
		case ACTION_TELE_FIRE:
			/************************/

			ExecuteTeleFire();
			m_action = ACTION_WAIT_TRIPLE_END;

			break;
			/***************************/
		case ACTION_WAIT_TRIPLE_END:
			/***************************/
			
			if (!object->com_man().ta_is_active()) {
				if (IsActiveObjects())
					m_action = ACTION_TELE_STARTED;
				else 
					m_action = ACTION_COMPLETED; 
			}

			/*********************/
		case ACTION_COMPLETED:
			/*********************/

			break;
	}

	object->anim().m_tAction		= ACT_STAND_IDLE;
	object->dir().face_target		(object->EnemyMan.get_enemy(), 700);

}

template <typename _Object>
void CStateBurcsAttackTeleAbstract::finalize()
{
	inherited::finalize				();

	tele_objects.clear				();
	object->DeactivateShield		();

	// clear particles on active objects
	if (object->CTelekinesis::is_active()) {
		for (u32 i=0; i<object->CTelekinesis::get_objects_count(); i++) {
			object->StopTeleObjectParticle(object->CTelekinesis::get_object_by_index(i).get_object());
		}
	}

	// �������� ������ �� ������ ��������
	object->set_script_capture			(true);
}

template <typename _Object>
void CStateBurcsAttackTeleAbstract::critical_finalize()
{
	inherited::critical_finalize		();

	object->com_man().ta_pointbreak		();
	object->CTelekinesis::Deactivate	();
	object->DeactivateShield			();

	tele_objects.clear					();

	// clear particles on active objects
	if (object->CTelekinesis::is_active()) {
		for (u32 i=0; i<object->CTelekinesis::get_objects_count(); i++) {
			object->StopTeleObjectParticle(object->CTelekinesis::get_object_by_index(i).get_object());
		}
	}
	
	// �������� ������ �� ������ ��������
	object->set_script_capture			(true);
}

template <typename _Object>
bool CStateBurcsAttackTeleAbstract::check_start_conditions()
{
	return CheckTeleStart();
}

template <typename _Object>
bool CStateBurcsAttackTeleAbstract::check_completion()
{
	return (m_action == ACTION_COMPLETED);
}

//////////////////////////////////////////////////////////////////////////

template <typename _Object>
void CStateBurcsAttackTeleAbstract::FindFreeObjects(xr_vector<CObject*> &tpObjects, const Fvector &pos)
{
	Level().ObjectSpace.GetNearest	(tpObjects, pos, object->m_tele_find_radius, NULL);

	for (u32 i=0;i<tpObjects.size();i++) 
	{
		CPhysicsShellHolder *obj			= smart_cast<CPhysicsShellHolder *>(tpObjects[i]);
		CCustomMonster		*custom_monster	= smart_cast<CCustomMonster *>(tpObjects[i]);

		if (!obj || !obj->PPhysicsShell() || !obj->PPhysicsShell()->isActive() || 
			custom_monster ||
			(obj->spawn_ini() && obj->spawn_ini()->section_exist("ph_heavy"))  ||
			(pSettings->line_exist(obj->cNameSect().c_str(), "quest_item") && pSettings->r_bool(obj->cNameSect().c_str(), "quest_item")) ||
			(obj->m_pPhysicsShell->getMass() < this->object->m_tele_object_min_mass) ||
			(obj->m_pPhysicsShell->getMass() > this->object->m_tele_object_max_mass) || (obj == this->object) ||
			this->object->CTelekinesis::is_active_object(obj) || !obj->m_pPhysicsShell->get_ApplyByGravity())
			continue;

		tele_objects.push_back(obj);
	}
}

template <typename _Object>
void CStateBurcsAttackTeleAbstract::FindObjects	()
{
	u32	res_size					= tele_objects.size		();
	tele_objects.clear	();

	// �������� ������ �������� ������ �����
	m_nearest.clear		();
	m_nearest.reserve				(res_size);
	FindFreeObjects					(m_nearest, object->EnemyMan.get_enemy()->Position());

	// �������� ������ �������� ������ �������
	FindFreeObjects					(m_nearest, object->Position());

	// �������� ������ �������� ����� �������� � ������
	float dist = object->EnemyMan.get_enemy()->Position().distance_to(object->Position());
	Fvector dir;
	dir.sub(object->EnemyMan.get_enemy()->Position(), object->Position());
	dir.normalize();

	Fvector pos;
	pos.mad							(object->Position(), dir, dist / 2.f);
	FindFreeObjects					(m_nearest, pos);	
	

	// �������� ���������� �������
	tele_objects.erase				(
		std::unique(
			tele_objects.begin(),
			tele_objects.end()
		),
		tele_objects.end()
	);
}

template <typename _Object>
void CStateBurcsAttackTeleAbstract::ExecuteTeleStart()
{
	object->com_man().ta_activate(object->anim_triple_tele);
	time_started = Device.dwTimeGlobal;
	object->ActivateShield();

}

template <typename _Object>
void CStateBurcsAttackTeleAbstract::ExecuteTeleContinue()
{
	if (time_started + object->m_tele_time_to_hold > Device.dwTimeGlobal) return;

	// ����� ������ ��� �����
	bool object_found = false;
	CTelekineticObject tele_object;

	u32 i=0;
	while (i < object->CTelekinesis::get_objects_count()) {
		tele_object = object->CTelekinesis::get_object_by_index(i);

		if ((tele_object.get_state() == TS_Keep) && (tele_object.time_keep_started + 1500 < Device.dwTimeGlobal)) {

			object_found = true;
			break;

		} else i++;

	}

	if (object_found) {
		m_action		= ACTION_TELE_FIRE;
		selected_object = tele_object.get_object();
	} else {
		if (!IsActiveObjects() || (time_started + MAX_TIME_CHECK_FAILURE < Device.dwTimeGlobal)) {
			object->com_man().ta_deactivate	();
			m_action						= ACTION_COMPLETED;
		} 
	}
}

#define HEAD_OFFSET_INDOOR	1.f
#define HEAD_OFFSET_OUTDOOR 5.f

template <typename _Object>
void CStateBurcsAttackTeleAbstract::ExecuteTeleFire()
{
	object->com_man().ta_pointbreak();

	Fvector enemy_pos;
	enemy_pos	= get_head_position(const_cast<CEntityAlive*>(object->EnemyMan.get_enemy()));
	object->CTelekinesis::fire_t(selected_object,enemy_pos, 0.55f);

	object->StopTeleObjectParticle	(selected_object);
	object->sound().play			(CBurcs::eMonsterSoundTeleAttack);
	object->DeactivateShield		();
}

template <typename _Object>
bool CStateBurcsAttackTeleAbstract::IsActiveObjects()
{
	return (object->CTelekinesis::get_objects_count() > 0);
}

template <typename _Object>
bool CStateBurcsAttackTeleAbstract::CheckTeleStart()
{
	if (object->com_man().ta_is_active()) return false;
	
	// �������� �� ������� ���������� 
	if (IsActiveObjects()) return false;

	CEntityAlive const* enemy = object->EnemyMan.get_enemy();
	if (!enemy || enemy != Actor())
		return false;

	// ��������� ��������� �� �����
	float dist = object->Position().distance_to(object->EnemyMan.get_enemy()->Position());
	if (dist < GOOD_DISTANCE_FOR_TELE) return false;

	// ����� ���������������� �������
	FindObjects();

	// ���� ��� ��������
	if (tele_objects.empty()) return false;

	// �� �� ����� �������� ���������
	return true;

}

//////////////////////////////////////////////////////////////////////////
// ����� ���������� �������� ��� ����������
//////////////////////////////////////////////////////////////////////////
class best_object_predicate {
	Fvector enemy_pos;
	Fvector monster_pos;
public:
	best_object_predicate(const Fvector &m_pos, const Fvector &pos) {
		monster_pos = m_pos;
		enemy_pos	= pos;
	}

	bool operator()	 (const CGameObject *tpObject1, const CGameObject *tpObject2) const
	{

		float dist1 = monster_pos.distance_to(tpObject1->Position());
		float dist2 = enemy_pos.distance_to(tpObject2->Position());
		float dist3 = enemy_pos.distance_to(monster_pos);

		return ((dist1 < dist3) && (dist2 > dist3));
	};
};

class best_object_predicate2 {
	Fvector enemy_pos;
	Fvector monster_pos;
public:
	best_object_predicate2(const Fvector &m_pos, const Fvector &pos) {
		monster_pos = m_pos;
		enemy_pos	= pos;
	}

	bool operator()	 (const CGameObject *tpObject1, const CGameObject *tpObject2) const
	{
		float dist1 = enemy_pos.distance_to(tpObject1->Position());
		float dist2 = enemy_pos.distance_to(tpObject2->Position());

		return (dist1 < dist2);		
	};
};


template <typename _Object>
void CStateBurcsAttackTeleAbstract::SelectObjects()
{
	std::sort(tele_objects.begin(),tele_objects.end(),best_object_predicate2(object->Position(), object->EnemyMan.get_enemy()->Position()));

	// ������� ������
	for (u32 i=0; i<tele_objects.size(); i++) {
		CPhysicsShellHolder *obj = tele_objects[i];

		// ��������� ��������� �� ������
		
		float	height = (object->m_monster_type == CBaseMonster::eMonsterTypeIndoor) ? 1.3f : 2.f;
		bool	rotate = (object->m_monster_type == CBaseMonster::eMonsterTypeIndoor) ? false : true;
		
		CTelekineticObject *tele_obj = object->CTelekinesis::activate		(obj, 3.f, height, 10000, rotate);
		tele_obj->set_sound		(object->sound_tele_hold,object->sound_tele_throw);

		object->StartTeleObjectParticle		(obj);

		// ������� �� ������
		tele_objects[i] = tele_objects[tele_objects.size()-1];
		tele_objects.pop_back();

		if (object->CTelekinesis::get_objects_count() >= object->m_tele_max_handled_objects) break;
	}
}



#undef CStateBurcsAttackTeleAbstract
