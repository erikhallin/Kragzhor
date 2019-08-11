#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <math.h>
#include <stdlib.h>

//50x200 tiles

#define _world_width 3200
#define _world_height 12800
#define _col_grid_size 128
#define _tile_size 64

const float _version=1.0;
const float _time_step=0.01;
const float _pi=3.14159265359;
const float _Rad2Deg=57.2957795;
const float _Deg2Rad=0.0174532925;
const int   _max_teams=8;
const int   _numof_tile_types=18;

//sound
const int   _sound_chan_music_normal=10;
const int   _sound_chan_music_fast=11;
const float _sound_music_fade_speed=0.2;
const float _sound_music_fast_time=10;

//player values
const float _player_move_speed=1.5;//1.5
const float _player_size=20.0;
const float _player_cam_radius=10.0;
const float _player_cam_radius_limit=50.0;
const float _player_attack_range=70;
const float _player_attack_angle=120;
const float _player_attack_damage=100;
const float _player_attack_speed=0.3;
const float _player_footprint_time=100;
const float _player_footprint_delay=50;
const float _player_footprint_size=19;
const float _player_walk_anim_speed=1.0;
const float _player_hp=500;//500

//unit values
const float _unit_rotation_speed=3.0;
const float _unit_move_speed=1.0;
const float _unit_size=5.0;
const float _unit_push_speed=0.01;
const float _unit_rotation_check_delay=1.0;
const float _unit_defence_stay_delay=1.0;
const float _unit_spot_delay=5.0;
const float _unit_shield_damage_reduction_factor=0.5;
const float _unit_projectile_damage=100;
const float _unit_projectile_speed=4;
const float _unit_eat_hp_regain=10;
const float _unit_walk_anim_speed=2.0;
const int   _unit_shout_prop=3000;
const float _unit_flee_distance=100;
const float _unit_dead_size=26;

//building values
const float _building_size=40.0;
const float _building_hp=300;

//object values
const float _object_size=15;
const float _object_hp=1000;
const float _object_projectile_damage=200;
const float _object_projectile_speed=8;
const float _object_rotation_speed=1000;
const float _object_throw_damping=0.005;
const float _object_throw_speed_limit=0.0050;
const float _flower_size=5;

const float _projectile_speed_default=300.0;

enum unit_types
{
    ut_villager=0,//no attack
    ut_player,
    ut_bowman,//range attack
    ut_knight//melee
};

struct st_pos
{
    st_pos()
    {
        x=y=0.0;
    }

    st_pos(float randval)
    {
        x=(rand()%1000-500)/500.0*randval;
        y=(rand()%1000-500)/500.0*randval;
    }

    st_pos(const st_pos& _input)
    {
        x=_input.x;
        y=_input.y;
    }

    st_pos(const float _x,const float _y)
    {
        x=_x;
        y=_y;
    }

    float x,y;

    st_pos operator=(const st_pos pos2)
    {
        x=pos2.x;
        y=pos2.y;

        return *this;
    }

    bool operator!=(const st_pos pos2)
    {
        return (x!=pos2.x || x!=pos2.x);
    }

    bool operator==(const st_pos pos2)
    {
        return (x==pos2.x && x==pos2.x);
    }

    st_pos operator+(const st_pos pos2)
    {
        st_pos pos_sum;
        pos_sum.x=x+pos2.x;
        pos_sum.y=y+pos2.y;

        return pos_sum;
    }

    st_pos operator/(const float scale)
    {
        st_pos pos_quotient;
        pos_quotient.x=x/scale;
        pos_quotient.y=y/scale;

        return pos_quotient;
    }

    st_pos operator*(const float scale)
    {
        st_pos pos_prod;
        pos_prod.x=x*scale;
        pos_prod.y=y*scale;

        return pos_prod;
    }

    st_pos operator+=(const st_pos pos2)
    {
        x+=pos2.x;
        y+=pos2.y;

        return *this;
    }

    st_pos operator-=(const st_pos pos2)
    {
        x-=pos2.x;
        y-=pos2.y;

        return *this;
    }

    st_pos operator/=(const float scale)
    {
        x/=scale;
        y/=scale;

        return *this;
    }

    st_pos operator*=(const float scale)
    {
        x*=scale;
        y*=scale;

        return *this;
    }

    float length()
    {
        return sqrt( x*x+y*y );
    }

    float length2()
    {
        return x*x+y*y;
    }

    float distance(const st_pos pos2)
    {
        return sqrt( (x-pos2.x)*(x-pos2.x)+(y-pos2.y)*(y-pos2.y) );
    }

    float distance2(const st_pos pos2)
    {
        return (x-pos2.x)*(x-pos2.x)+(y-pos2.y)*(y-pos2.y);
    }

    st_pos normalize()
    {
        float length=sqrt( x*x+y*y );
        if(length==0) return st_pos(1,0);
        x/=length;
        y/=length;
        return *this;
    }

    st_pos get_rel_pos_to(st_pos _pos)
    {
        return st_pos( x-_pos.x,y-_pos.y );
    }

    float get_angle_to(st_pos _pos)
    {
        st_pos rel_pos=this->get_rel_pos_to(_pos);
        //rel_pos.normalize();

        return atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
    }

    /*float get_angle_to_m90(st_pos _pos)
    {
        st_pos rel_pos=this->get_rel_pos_to(_pos);
        rel_pos.normalize();

        return atan2f(-rel_pos.x,rel_pos.y)*_Rad2Deg;
    }*/
};

struct unit_spec
{
    unit_spec()
    {
        type=ut_villager;
        team=0;
        damage=10;
        damage_range=5;
        projectile_speed=_projectile_speed_default;
        speed_move=_unit_move_speed;
        speed_rotate=_unit_rotation_speed;
        hp_curr=hp_max=100;
        armour=1;
        attack_range=90;
        attack_speed=0.5;
        aim_angle_tolerance=5;
        view_range=150;
        defence_range=40;
        shield_angle=90;
    }

    unit_spec(int _type)
    {
        type=_type;
        team=0;

        switch(type)
        {
            case ut_villager:
            {
                damage=1;
                damage_range=5;
                projectile_speed=_projectile_speed_default;
                speed_move=_unit_move_speed;
                speed_rotate=_unit_rotation_speed;
                hp_curr=hp_max=100;
                armour=1;
                attack_range=90;
                attack_speed=0.5;
                aim_angle_tolerance=5;
                view_range=500;
                defence_range=40;
                shield_angle=90;
            }break;

            case ut_bowman:
            {
                damage=10;
                damage_range=5;
                projectile_speed=_projectile_speed_default;
                speed_move=_unit_move_speed;
                speed_rotate=_unit_rotation_speed;
                hp_curr=hp_max=100;
                armour=1;
                attack_range=150;
                attack_speed=0.5;
                aim_angle_tolerance=5;
                view_range=600;
                defence_range=40;
                shield_angle=10;
            }break;

            case ut_knight:
            {
                damage=10;
                damage_range=10;
                projectile_speed=_projectile_speed_default;
                speed_move=_unit_move_speed;
                speed_rotate=_unit_rotation_speed;
                hp_curr=hp_max=200;
                armour=1;
                attack_range=35;
                attack_speed=0.50;
                aim_angle_tolerance=5;
                view_range=600;
                defence_range=40;
                shield_angle=90;
            }break;

            case ut_player:
            {
                damage=_player_attack_damage;
                damage_range=1;
                projectile_speed=_projectile_speed_default;
                speed_move=_player_move_speed;
                speed_rotate=1;
                hp_curr=hp_max=_player_hp;
                armour=1;
                attack_range=_player_attack_range;
                attack_speed=0.5;
                aim_angle_tolerance=5;
                view_range=200;
                defence_range=40;
                shield_angle=90;
            }break;

            default:
            {
                damage=10;
                damage_range=5;
                projectile_speed=_projectile_speed_default;
                speed_move=_player_move_speed;
                speed_rotate=1;
                hp_curr=hp_max=_player_hp;
                armour=1;
                attack_range=90;
                attack_speed=0.5;
                aim_angle_tolerance=5;
                view_range=200;
                defence_range=40;
                shield_angle=90;
            }
        }



    }

    unit_spec operator=(const unit_spec _input)
    {
        type=_input.type;
        team=_input.team;
        damage=_input.damage;
        damage_range=_input.damage_range;
        projectile_speed=_input.projectile_speed;
        speed_move=_input.speed_move;
        speed_rotate=_input.speed_rotate;
        hp_curr=_input.hp_curr;
        hp_max=_input.hp_max;
        armour=_input.armour;
        attack_range=_input.attack_range;
        attack_speed=_input.attack_speed;
        aim_angle_tolerance=_input.aim_angle_tolerance;
        view_range=_input.view_range;
        defence_range=_input.defence_range;
        shield_angle=_input.shield_angle;

        return *this;
    }

    int   type;//villager, bowman, knight
    int   team;
    float damage;
    float damage_range;
    float projectile_speed;
    float attack_range;
    float attack_speed;
    float speed_move;
    float speed_rotate;
    float hp_max;
    float hp_curr;
    float armour;
    float aim_angle_tolerance;
    float view_range;
    float defence_range;
    float shield_angle;
};

enum en_states
{
    none=0,
    passive,
    active
};

struct st_col_id
{
    st_col_id()
    {

    }

    st_col_id(int _id,st_pos _p1,st_pos _p2,st_pos _p3,st_pos _p4)
    {
        id=_id;
        p1=_p1;
        p2=_p2;
        p3=_p3;
        p4=_p4;
    }

    st_col_id operator=(st_col_id _input)
    {
        id=_input.id;
        p1=_input.p1;
        p2=_input.p2;
        p3=_input.p3;
        p4=_input.p4;

        return *this;
    }

    bool is_pos_inside_square(st_pos test_pos)
    {
        //triangle 1 test
        bool b1, b2, b3;

        b1=(test_pos.x-p2.x)*(p1.y-p2.y)-(p1.x-p2.x)*(test_pos.y-p2.y) < 0.0f;
        b2=(test_pos.x-p3.x)*(p2.y-p3.y)-(p2.x-p3.x)*(test_pos.y-p3.y) < 0.0f;
        b3=(test_pos.x-p1.x)*(p3.y-p1.y)-(p3.x-p1.x)*(test_pos.y-p1.y) < 0.0f;

        bool t1=((b1 == b2) && (b2 == b3));

        //triangle 2 test
        b1=(test_pos.x-p4.x)*(p1.y-p4.y)-(p1.x-p4.x)*(test_pos.y-p4.y) < 0.0f;
        b2=(test_pos.x-p3.x)*(p4.y-p3.y)-(p4.x-p3.x)*(test_pos.y-p3.y) < 0.0f;
        b3=(test_pos.x-p1.x)*(p3.y-p1.y)-(p3.x-p1.x)*(test_pos.y-p1.y) < 0.0f;

        bool t2=((b1 == b2) && (b2 == b3));

        return (t1 || t2);
    }

    int id;
    st_pos p1,p2,p3,p4;
};

#endif
