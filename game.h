#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <SOIL/SOIL.h>
#include <gl/gl.h>
#include <string>
#include <vector>
#include <ctime>

#include "sound.h"
#include "gamepad.h"
#include "key_reroute.h"
#include "unit.h"
#include "building.h"
#include "projectile.h"
#include "object.h"
#include "particle_engine.h"
#include "hud.h"

enum tile_types
{
    tt_grass=0,
    tt_path
};

enum game_states
{
    gs_init=0,
    gs_menu,
    gs_in_game,
    gs_gameover
};

enum hand_objects
{
    ho_none=0,
    ho_solid,
    ho_unit
};

enum music_states
{
    ms_on_normal=0,
    ms_on_fast,
    ms_to_normal,
    ms_to_fast
};

struct st_tile
{
    st_tile()
    {
        type=tt_grass;
        subtype=0;
    }

    st_tile(int _type)
    {
        type=_type;
        subtype=0;
    }

    st_tile(int _type,int _subtype)
    {
        type=_type;
        subtype=_subtype;
    }

    int type,subtype;
};

struct col_square
{
    vector<unit*> vec_pUnits;
};

struct st_footprint
{
    st_footprint()
    {
        rotation=0;
        time_left=_player_footprint_time;
    }

    st_footprint(st_pos _pos,float _rotation)
    {
        pos=_pos;
        rotation=_rotation;
        time_left=_player_footprint_time;
    }

    st_pos pos;
    float  rotation;
    float  time_left;
};

struct st_pre_unit
{
    st_pre_unit(unit_spec _specs,st_pos _pos,int _texture)
    {
        specs=_specs;
        pos=_pos;
        texture=_texture;
    }

    unit_spec specs;
    st_pos pos;
    int texture;
};

struct st_decal
{
    st_decal()
    {
        rotation=0;
        type=0;
    }

    st_decal(st_pos _pos)
    {
        pos=_pos;
        rotation=0;
        type=0;
    }

    st_decal(st_pos _pos,float _rot,int _type)
    {
        pos=_pos;
        rotation=_rot;
        type=_type;
    }

    st_pos pos;
    float rotation;
    int type;
};

class game
{
    public:

        game();

        bool init(int* pWindow_size,bool* pKeys_real,bool* pKeys_translated,
                  int* pMouse_pos,bool* pMouse_but,bool reinit=false);
        bool update(bool& quit_flag);
        bool draw(void);



    private:

        int   m_game_state;
        int   m_window_size[2];
        bool* m_pKeys_real;
        bool* m_pKeys_translated;
        int*  m_pMouse_pos;
        bool* m_pMouse_but;
        bool  m_gamepad_connected[4];
        float m_cam_pos[2];
        float m_eye_open_prog,m_eye_open_time;
        bool  m_game_paused;
        float m_tail_prog,m_eye_pulse;

        //texture
        float m_walk_prog;
        int   m_tex_main,m_tex_info,m_tex_player_move,m_tex_player_footprint;
        int   m_tex_unit_move,m_tex_hud,m_tex_tile,m_tex_rock,m_tex_building,m_tex_gameover,m_tex_knight;
        int   m_tex_mask,m_tex_bowman,m_tex_arrow,m_tex_eye,m_tex_unit_dead,m_tex_hud_health;
        int   m_tex_monster_attack,m_tex_monster_walk,m_tex_monster_tail,m_tex_flower;
        int   m_tex_font[3];
        st_pos m_tex_tile_pos[_numof_tile_types];
        float m_tex_tile_size[2];

        //object
        hud         m_hud;
        key_reroute m_key_rerouter;
        sound*      m_pSound;
        gamepad     m_gamepad[4];
        unit*       m_pPlayer_unit;
        particle_engine* m_pPartEng;

        col_square         m_arr_col_squares[_world_width/_col_grid_size][_world_height/_col_grid_size];
        string             m_arr_team_squares[_world_width/_col_grid_size][_world_height/_col_grid_size];
        st_tile            m_arr_tiles[_world_width/_tile_size][_world_height/_tile_size];
        vector<st_col_id>  m_vec_col_id;
        vector<unit*>      m_vec_pUnits;
        vector<building*>  m_vec_pBuildings;
        vector<object*>    m_vec_pObjects;
        vector<projectile> m_vec_projectiles;
        vector<st_footprint> m_vec_footprints;
        vector<st_pre_unit> m_vec_preunits;
        vector<st_decal>    m_vec_dead_units;
        vector<st_decal>    m_vec_old_buildings;
        vector<st_decal>    m_vec_flowers;

        //triggers
        bool m_key_trig_attack,m_key_trig_grabthrow,m_key_trig_eat,m_key_trig_esc,m_key_trig_enter;

        //misc
        bool    m_draw_player_attack;
        float   m_draw_attack_area_timer;
        int     m_object_in_hands;
        unit*   m_pUnit_in_hands;
        object* m_pObject_in_hands;
        float   m_footprint_timer;
        bool    m_footstep_left;
        float   m_music_vol_normal,m_music_vol_fast;
        int     m_music_state;
        float   m_music_fast_timer;
        bool    m_won;

        bool    m_have_reset;


        bool  load_textures(void);
        bool  load_sounds(void);
        bool  start_music(void);
        bool  clean_up(void);
        bool  load_level(void);
        bool  convert_level(void);
};

#endif // GAME_H
