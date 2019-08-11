#ifndef UNIT_H
#define UNIT_H

#include <iostream>
#include <gl/gl.h>
#include "definitions.h"

using namespace std;

enum attack_modes
{
    am_none=0,
    am_stand,   //shoots enemies but does not move
    am_defence, //will follow an enemy within view a certain distance
    am_pursuit  //will follow enemies within view
};

class unit
{
    public:
        unit();
        unit(unit_spec specs);
        unit(unit_spec specs,st_pos pos,int texture);

        bool   m_player_controlled,m_player_move_order,m_projectile_mode;
        st_pos m_projectile_direction;
        bool   m_selected,m_awake,m_have_moved,m_forced_move,m_attack_moving,m_is_idle;
        int    m_attack_mode,m_col_id,m_col_state;
        string m_unit_seen_by_team;
        unit*  m_pUnit_enemy_target;
        float  m_rotation_curr,m_size;
        float  m_attack_cooldown,m_attack_mode_cooldown;

        //unit spec
        unit_spec m_spec;


        bool init(st_pos pos,int texture);
        bool update(void);
        bool draw(void);
        bool is_pos_inside(st_pos pos,float size_factor=1.0,bool test_new_pos=false);
        bool is_pos_inside_rect(st_pos pos_top_left,st_pos pos_low_right);
        bool player_move(st_pos move_vec);
        bool move_to(st_pos end_pos,bool is_forced=false,float final_rotation=9999,unit* pEnemy_target=0);
        bool move_to_rel(st_pos rel_pos_shift,float move_direction,bool forced_move,float rot_val[4]);
        bool push_unit(st_pos push_dir,bool pushing_cancels_movement);
        st_pos get_curr_pos(void);
        st_pos get_new_pos(void);
        st_pos get_curr_col_square_pos(void);
        float get_curr_rotation(void);
        bool set_curr_col_square_pos(st_pos new_pos);
        bool accept_new_pos(void);
        bool is_moving(void);
        unit_spec get_spec(void);
        bool is_ready_to_attack(void);
        bool take_damage(float damage,float time_delay=0.0,bool kill=false);
        bool attack_action_done(void);
        bool aim_to_target(float rotation_target,bool special_attack_order_overide=false);
        bool cancel_movement(void);
        bool force_pos(st_pos pos,float rotation);
        bool throw_unit(st_pos direction);

    private:

        st_pos m_pos,m_pos_new,m_col_square_pos,m_pos_attack_move_target,m_pos_defence;
        float  m_rotation_target,m_rotation_target_final;

        float  m_rotation_check_timer,m_defence_stay_timer;
        float  m_random_move_timer,m_walk_prog;



        st_pos m_target_pos,m_target_direction;
        int m_texture;

};

#endif // UNIT_H
