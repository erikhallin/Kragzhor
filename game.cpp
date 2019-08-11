#include "game.h"

game::game()
{
    m_game_state=gs_init;
}

bool game::init(int* window_size,bool* pKeys_real,bool* pKeys_translated,
                int* pMouse_pos,bool* pMouse_but,bool reinit)
{
    cout<<"Game: Initialization\n";

    //convert level data
    //convert_level();
    //return false;

    int SEED=time(0);
    cout<<"Seed: "<<SEED<<endl;
    srand(SEED);

    m_window_size[0]=window_size[0];
    m_window_size[1]=window_size[1];
    m_pKeys_real=pKeys_real;
    m_pKeys_translated=pKeys_translated;
    m_pMouse_pos=pMouse_pos;
    m_pMouse_but=pMouse_but;
    m_cam_pos[0]=m_cam_pos[1]=0;
    m_draw_player_attack=false;
    m_key_trig_attack=false;
    m_key_trig_grabthrow=false;
    m_key_trig_eat=false;
    m_draw_attack_area_timer=0;
    m_object_in_hands=ho_none;
    m_footprint_timer=_player_footprint_delay;
    m_footstep_left=false;
    m_walk_prog=0;
    m_eye_open_prog=m_eye_open_time=0.5;
    m_game_paused=false;
    m_eye_pulse=0.0;
    m_won=false;


    //load textures and sound
    if(!reinit)
    {
        m_have_reset=false;
        m_key_trig_esc=false;
        m_key_trig_enter=false;

        //texture
        if(!load_textures())
        {
            return false;
        }

        //sound
        if(!load_sounds())
        {
            return false;
        }

        //tile texture calc
        m_tex_tile_size[0]=_tile_size/384.0;
        m_tex_tile_size[1]=_tile_size/192.0;
        float curr_x=0;
        float curr_y=0;
        for(int i=0;i<_numof_tile_types;i++)
        {
            m_tex_tile_pos[i].x=curr_x;
            m_tex_tile_pos[i].y=curr_y;

            curr_x+=_tile_size/384.0;
            if(curr_x>=0.99)
            {
                curr_x=0;
                curr_y+=_tile_size/192.0;
            }
        }
    }
    //start music
    start_music();

    //init gamepads and players
    for(int i=0;i<4;i++)
    {
        m_gamepad[i]=gamepad(i);
        if( m_gamepad[i].IsConnected() )
         m_gamepad_connected[i]=true;
        else
         m_gamepad_connected[i]=false;
    }

    m_key_rerouter.init(m_pKeys_real,m_pKeys_translated);

    //particle engine
    m_pPartEng=new particle_engine();

    //hud
    m_hud.init(m_tex_hud,m_window_size,m_tex_font,m_tex_hud_health);

    m_game_state=gs_menu;
    //m_game_state=gs_gameover;

    //init tiles
    if( !load_level() )
    {
        cout<<"WARNING: Could not use level data\n";
        //use default
        for(int x=0;x<_world_width/_tile_size;x++)
        for(int y=0;y<_world_height/_tile_size;y++)
        {
            m_arr_tiles[x][y].type=tt_grass;
        }
    }


    //TEMP
    //add units
    /*for(int i=0;i<1000;i++)
    {
        int pos_x=rand()%_world_width;
        int pos_y=rand()%_world_height;

        //spawn types

        int team=0;
        int type=ut_villager;
        //if(pos_x<_world_width*0.4) team=0;
        //else if(pos_x>_world_width*0.6) team=1;
        if(pos_x<_world_width*0.3) type=ut_bowman;
        else if(pos_x>_world_width*0.6) type=ut_knight;

        unit_spec specs(type);
        specs.team=team;

        m_vec_pUnits.push_back( new unit(specs) );
        m_vec_pUnits.back()->init( st_pos(pos_x,pos_y),m_tex_unit_move );

        //place in col square
        int col_square_x=pos_x/_col_grid_size;
        int col_square_y=pos_y/_col_grid_size;
        m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits.back() );
        m_vec_pUnits.back()->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );
    }*/

    //add player
    unit_spec player_specs;
    player_specs.attack_speed=_player_attack_speed;
    player_specs.team=1;
    player_specs.type=ut_player;
    player_specs.hp_curr=_player_hp;
    player_specs.hp_max=_player_hp;
    m_vec_pUnits.push_back( new unit(player_specs) );
    m_vec_pUnits.back()->init( st_pos(_world_width*0.5,_world_height-300),m_tex_unit_move );
    int col_x=_world_width*0.5/_col_grid_size;
    int col_y=_world_height*0.5/_col_grid_size;
    m_arr_col_squares[col_x][col_y].vec_pUnits.push_back( m_vec_pUnits.back() );
    m_vec_pUnits.back()->set_curr_col_square_pos( st_pos(col_x,col_y) );
    m_pPlayer_unit=m_vec_pUnits.back();
    m_pPlayer_unit->m_player_controlled=true;
    m_pPlayer_unit->m_size=_player_size;

    //spawn flowers
    int numof_flowers=500;
    for(int i=0;i<numof_flowers;i++)
    {
        st_pos pos;
        pos.x=rand()%(int)_world_width;
        pos.y=rand()%(int)_world_height;
        int type=rand()%4;
        m_vec_flowers.push_back( st_decal(pos,0,type) );
    }

    /*//add buildings
    m_vec_pBuildings.push_back( new building() );
    m_vec_pBuildings.back()->init( st_pos(150,70),m_tex_building );
    m_vec_pBuildings.push_back( new building() );
    m_vec_pBuildings.back()->init( st_pos(250,90),m_tex_building );
    m_vec_pBuildings.push_back( new building() );
    m_vec_pBuildings.back()->init( st_pos(340,80),m_tex_building );*/

    /*//add objects
    m_vec_pObjects.push_back( new object() );
    m_vec_pObjects.back()->init( st_pos(150,170),m_tex_rock );
    m_vec_pObjects.push_back( new object() );
    m_vec_pObjects.back()->init( st_pos(250,290),m_tex_rock );
    m_vec_pObjects.push_back( new object() );
    m_vec_pObjects.back()->init( st_pos(340,380),m_tex_rock );*/

    cout<<"Game: Initialization complete\n";

    return true;
}

bool game::update(bool& quit_flag)
{
    //get gamepad data
    st_gamepad_data gamepad_data[4];
    for(int gamepad_i=0;gamepad_i<4;gamepad_i++)
    {
        if( m_gamepad[gamepad_i].IsConnected() )
        {
            //test if new connection
            if(!m_gamepad_connected[gamepad_i])
            {
                cout<<"Gamepad: New controller conencted: "<<gamepad_i+1<<endl;
            }

            m_gamepad_connected[gamepad_i]=true;

            //get data
            gamepad_data[gamepad_i]=m_gamepad[gamepad_i].GetState();
        }
        else//lost controller
        {
            if( m_gamepad_connected[gamepad_i] )//had connection and lost it
            {
                m_gamepad_connected[gamepad_i]=false;
                cout<<"Gamepad: Lost connection to controller: "<<gamepad_i+1<<endl;
            }
        }
    }

    //update key rerouter
    m_key_rerouter.update();

    //music update
    if(m_music_fast_timer>0)
    {
        m_music_fast_timer-=_time_step;
        if(m_music_fast_timer<=0)
        {
            m_music_fast_timer=0;
            m_music_state=ms_to_normal;
        }
    }
    switch(m_music_state)
    {
        case ms_on_normal:
        {
            //idle
        }break;

        case ms_on_fast:
        {
            //idle
        }break;

        case ms_to_normal:
        {
            //cout<<"music to normal\n";

            //lower fast
            m_music_vol_fast-=_sound_music_fade_speed*_time_step;
            if(m_music_vol_fast<0)
            {
                m_music_vol_fast=0;
            }

            //increase normal
            m_music_vol_normal+=_sound_music_fade_speed*_time_step;
            if(m_music_vol_normal>1.0)
            {
                m_music_vol_normal=1.0;
                m_music_vol_fast=0;

                //done
                m_music_state=ms_on_normal;
            }

            //update volume
            m_pSound->set_volume(_sound_chan_music_normal,m_music_vol_normal);
            m_pSound->set_volume(_sound_chan_music_fast,m_music_vol_fast);

        }break;

        case ms_to_fast:
        {
            //cout<<"music to fast\n";

            //increase fast
            m_music_vol_fast+=_sound_music_fade_speed*_time_step;
            if(m_music_vol_fast>1.0)
            {
                m_music_vol_fast=1.0;
            }

            //lower normal
            m_music_vol_normal-=_sound_music_fade_speed*_time_step;
            if(m_music_vol_normal<0)
            {
                m_music_vol_normal=0;
                m_music_vol_fast=1.0;

                //done
                m_music_state=ms_on_fast;
            }

            //update volume
            m_pSound->set_volume(_sound_chan_music_normal,m_music_vol_normal);
            m_pSound->set_volume(_sound_chan_music_fast,m_music_vol_fast);

        }break;
    }

    //update game
    switch(m_game_state)
    {
        case gs_init:
        {
            //nothing
        }break;

        case gs_menu:
        {
            bool start_game=false;
            if(m_pKeys_translated[key_enter])
            {
                if(!m_key_trig_enter)
                {
                    m_key_trig_enter=true;
                    start_game=true;
                }
            }
            else m_key_trig_enter=false;


            //gamepad, from one pad
            for(int player_i=0;player_i<4;player_i++)
            {
                if(m_gamepad_connected[player_i])
                {
                    if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_start)
                    {
                        m_key_trig_enter=true;
                        start_game=true;
                        break;
                    }
                }
            }

            if(start_game)
            {
                m_game_state=gs_in_game;

                //play sound
                int rand_sound=0;
                switch(rand()%3)
                {
                    case 0: rand_sound=wav_roar1; break;
                    case 1: rand_sound=wav_roar2; break;
                    case 2: rand_sound=wav_roar3; break;
                }
                m_pSound->playSimpleSound(rand_sound,1.0);

                break;
            }

            //exit key
            if(m_pKeys_translated[key_esc])
            {
                if(!m_key_trig_esc)
                {
                    m_key_trig_esc=true;
                    quit_flag=true;
                    return true;
                }
            }
            else m_key_trig_esc=false;

        }break;

        case gs_in_game:
        {
            //win test, no building or enemy left (player is a unit and is ignored
            if(!m_won)
            {
                if( (int)m_vec_pUnits.size()==1 && m_vec_pUnits.back()==m_pPlayer_unit &&
                    m_vec_pBuildings.empty() &&
                    m_vec_preunits.empty() )
                {
                    m_won=true;

                    m_hud.end(true);

                    m_game_state=gs_gameover;

                    //play sound
                    int rand_sound=0;
                    switch(rand()%3)
                    {
                        case 0: rand_sound=wav_roar1; break;
                        case 1: rand_sound=wav_roar2; break;
                        case 2: rand_sound=wav_roar3; break;
                    }
                    m_pSound->playSimpleSound(rand_sound,1.0);

                    return true;
                }
            }


            //eye timer
            if(m_eye_open_prog>0.0)
            {
                m_eye_open_prog-=_time_step;
                if(m_eye_open_prog<0) m_eye_open_prog=0;
            }

            //gamepad data translation
            st_pos thumbstick_dir;
            for(int player_i=0;player_i<4;player_i++)
            {
                if(m_gamepad_connected[player_i])
                {
                    //attack
                    if(gamepad_data[player_i].button_A)
                     m_pKeys_translated[key_b]=true;

                    //grab/throw
                    if(gamepad_data[player_i].button_X)
                     m_pKeys_translated[key_v]=true;

                    //eat
                    if(gamepad_data[player_i].button_Y)
                     m_pKeys_translated[key_c]=true;

                    //pause
                    if(gamepad_data[player_i].button_start)
                     m_pKeys_translated[key_esc]=true;

                    //move direction
                    //dpad
                    if(gamepad_data[player_i].dpad_up)
                     m_pKeys_translated[key_up]=true;
                    if(gamepad_data[player_i].dpad_right)
                     m_pKeys_translated[key_right]=true;
                    if(gamepad_data[player_i].dpad_down)
                     m_pKeys_translated[key_down]=true;
                    if(gamepad_data[player_i].dpad_left)
                     m_pKeys_translated[key_left]=true;
                    //thumbstick
                    int deadzone=5000;
                    st_pos stick_dir;
                    if(gamepad_data[player_i].thumbstick_left_x>deadzone ||
                       gamepad_data[player_i].thumbstick_left_x<-deadzone)
                    {
                        stick_dir.x=gamepad_data[player_i].thumbstick_left_x;
                    }
                    if(gamepad_data[player_i].thumbstick_left_y>deadzone ||
                       gamepad_data[player_i].thumbstick_left_y<-deadzone)
                    {
                        stick_dir.y=gamepad_data[player_i].thumbstick_left_y;
                    }
                    //test if update stick data
                    float prev_length=thumbstick_dir.length2();
                    float new_length=stick_dir.length2();
                    if(new_length>prev_length)
                    {
                        thumbstick_dir=stick_dir;
                    }
                }
            }

            //paused
            if(m_game_paused)
            {
                //test only for pause menu controls
                //unpause
                if(m_pKeys_translated[key_esc])
                {
                    if(!m_key_trig_esc)
                    {
                        m_key_trig_esc=true;

                        m_game_paused=false;
                        return true;
                    }
                }
                else m_key_trig_esc=false;

                //exit
                if(m_pKeys_translated[key_enter])
                {
                    if(!m_key_trig_enter)
                    {
                        m_key_trig_enter=true;

                        //exit game to menu
                        clean_up();
                        return false;
                    }
                }
                else m_key_trig_enter=false;

                break;
            }
            else//possible to pause
            {
                if(m_pKeys_translated[key_esc])
                {
                    if(!m_key_trig_esc)
                    {
                        m_key_trig_esc=true;

                        m_game_paused=true;
                        return true;
                    }
                }
                else m_key_trig_esc=false;
            }

            //attack area timer
            if(m_draw_attack_area_timer>0.0) m_draw_attack_area_timer-=_time_step*10;

            //tail timer
            m_tail_prog+=_time_step;
            if(m_tail_prog>=1.0) m_tail_prog-=1;

            //eye timer
            m_eye_pulse+=_time_step*0.3;
            if(m_eye_pulse>=1.0) m_eye_pulse-=1;

            //square grid size
            int square_x_max=_world_width/_col_grid_size;
            int square_y_max=_world_height/_col_grid_size;


            /*//move cam TEMP
            float cam_sens=3.0;
            if(m_pKeys_real[key_np_4])
            {
                m_cam_pos[0]-=cam_sens;
            }
            if(m_pKeys_real[key_np_8])
            {
                m_cam_pos[1]-=cam_sens;
            }
            if(m_pKeys_real[key_np_6])
            {
                m_cam_pos[0]+=cam_sens;
            }
            if(m_pKeys_real[key_np_2])
            {
                m_cam_pos[1]+=cam_sens;
            }*/

            //move player
            st_pos move_vec;
            float move_factor=0;
            if(m_pKeys_translated[key_left])
            {
                move_vec.x-=_player_move_speed;
                move_factor=1;
            }
            if(m_pKeys_translated[key_up])
            {
                move_vec.y-=_player_move_speed;
                move_factor=1;
            }
            if(m_pKeys_translated[key_right])
            {
                move_vec.x+=_player_move_speed;
                move_factor=1;
            }
            if(m_pKeys_translated[key_down])
            {
                move_vec.y+=_player_move_speed;
                move_factor=1;
            }
            //data from gamepad if no key was pressed
            if(move_factor==0 && (thumbstick_dir.x!=0||thumbstick_dir.y!=0) )
            {
                //cout<<thumbstick_dir.x<<"\t"<<thumbstick_dir.y<<endl;
                thumbstick_dir.x/=32768.0;
                thumbstick_dir.y/=32768.0;
                move_factor=thumbstick_dir.length();
                move_vec.x+=thumbstick_dir.x*_player_move_speed;
                move_vec.y-=thumbstick_dir.y*_player_move_speed;
            }
            m_pPlayer_unit->player_move(move_vec);
            m_footprint_timer-=move_factor;
            m_walk_prog+=_player_walk_anim_speed*_time_step*move_factor;
            while(m_walk_prog>1.0) m_walk_prog-=1.0;
            if(m_footprint_timer<=0)
            {
                //place footstep
                st_pos foot_pos=m_pPlayer_unit->get_curr_pos();
                float player_rotation=m_pPlayer_unit->m_rotation_curr;
                float leg_length=20;
                float leg_angle=0;
                if(m_footstep_left)
                {
                    leg_angle=-90;
                }
                else//right
                {
                    leg_angle=90;
                }
                foot_pos.x+=leg_length*cosf( (player_rotation+leg_angle)*_Deg2Rad );
                foot_pos.y+=leg_length*sinf( (player_rotation+leg_angle)*_Deg2Rad );
                m_vec_footprints.push_back( st_footprint(foot_pos,player_rotation) );

                m_footprint_timer+=_player_footprint_delay;
                m_footstep_left=!m_footstep_left;

                //add particles
                float pos[2]={foot_pos.x,foot_pos.y};
                float part_color[3]={0.6,0.4,0.2};
                m_pPartEng->add_explosion(pos,30,30,1,part_color,player_rotation-90.0);

                //play sound
                int rand_sound=0;
                switch(rand()%3)
                {
                    case 0: rand_sound=wav_step1; break;
                    case 1: rand_sound=wav_step2; break;
                    case 2: rand_sound=wav_step3; break;
                }
                m_pSound->playSimpleSound(rand_sound,1.0);
            }

            //follow cam
            float cam_follow_speed=0.2;
            st_pos player_pos=m_pPlayer_unit->get_curr_pos();
            if(player_pos.x>m_cam_pos[0]+m_window_size[0]*0.5-_player_cam_radius)
            {
                m_cam_pos[0]+=cam_follow_speed;
            }
            if(player_pos.x<m_cam_pos[0]+m_window_size[0]*0.5+_player_cam_radius)
            {
                m_cam_pos[0]-=cam_follow_speed;
            }
            if(player_pos.y>m_cam_pos[1]+m_window_size[1]*0.5-_player_cam_radius)
            {
                m_cam_pos[1]+=cam_follow_speed;
            }
            if(player_pos.y<m_cam_pos[1]+m_window_size[1]*0.5+_player_cam_radius)
            {
                m_cam_pos[1]-=cam_follow_speed;
            }
            //outer window
            if(player_pos.x>m_cam_pos[0]+m_window_size[0]*0.5+_player_cam_radius_limit)
            {
                m_cam_pos[0]=player_pos.x-m_window_size[0]*0.5-_player_cam_radius_limit;
            }
            if(player_pos.x<m_cam_pos[0]+m_window_size[0]*0.5-_player_cam_radius_limit)
            {
                m_cam_pos[0]=player_pos.x-m_window_size[0]*0.5+_player_cam_radius_limit;
            }
            if(player_pos.y>m_cam_pos[1]+m_window_size[1]*0.5+_player_cam_radius_limit)
            {
                m_cam_pos[1]=player_pos.y-m_window_size[1]*0.5-_player_cam_radius_limit;
            }
            if(player_pos.y<m_cam_pos[1]+m_window_size[1]*0.5-_player_cam_radius_limit)
            {
                m_cam_pos[1]=player_pos.y-m_window_size[1]*0.5+_player_cam_radius_limit;
            }
            //cap cam po
            if(m_cam_pos[0]<0) m_cam_pos[0]=0;
            if(m_cam_pos[1]<0) m_cam_pos[1]=0;
            if(m_cam_pos[0]>_world_width-m_window_size[0]) m_cam_pos[0]=_world_width-m_window_size[0];
            if(m_cam_pos[1]>_world_height-m_window_size[1]) m_cam_pos[1]=_world_height-m_window_size[1];

            //player attack
            if(m_pKeys_translated[key_b])
            {
                if(m_pPlayer_unit->is_ready_to_attack() && !m_key_trig_attack)
                {
                    m_pPlayer_unit->attack_action_done();
                    m_draw_player_attack=true;
                    m_key_trig_attack=true;
                    m_draw_attack_area_timer=1.0;

                    //play sound
                    m_pSound->playSimpleSound(wav_hit_miss,1.0);

                    float attack_range2=_player_attack_range*_player_attack_range;

                    //attack units
                    for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                    {
                        if(m_vec_pUnits[unit_i]==m_pPlayer_unit) continue;

                        //test range
                        if( player_pos.distance2( m_vec_pUnits[unit_i]->get_curr_pos() ) < attack_range2 )
                        {
                            //test angle
                            float player_rot=m_pPlayer_unit->m_rotation_curr+90;
                            if(player_rot>180) player_rot-=360;
                            //cout<<player_rot<<endl;
                            float angle_to_unit=player_pos.get_angle_to(m_vec_pUnits[unit_i]->get_curr_pos())-90;
                            if(angle_to_unit> 180) angle_to_unit-=360;
                            if(angle_to_unit<-180) angle_to_unit+=360;

                            if( fabs(angle_to_unit-player_rot) < _player_attack_angle*0.5 ||
                                //special occasion if faceing down
                                ( player_rot==180 && fabs(angle_to_unit+player_rot) < _player_attack_angle*0.5 ) )
                            {

                                //unit within attack range
                                m_vec_pUnits[unit_i]->take_damage( _player_attack_damage );
                                m_vec_pUnits[unit_i]->m_selected=true;

                                //add particles
                                float color[3]={0.6,0.2,0.2};
                                float pos[2]={m_vec_pUnits[unit_i]->get_curr_pos().x,
                                              m_vec_pUnits[unit_i]->get_curr_pos().y};
                                m_pPartEng->add_explosion(pos,50,50,2,color,player_rot-10);
                                m_pPartEng->add_explosion(pos,50,50,2,color,player_rot);
                                m_pPartEng->add_explosion(pos,50,50,2,color,player_rot+10);

                                //play sound
                                m_pSound->playSimpleSound(wav_hit_enemy,1.0);

                                //play sound
                                int rand_sound=0;
                                switch(rand()%3)
                                {
                                    case 0: rand_sound=wav_enemy_hurt1; break;
                                    case 1: rand_sound=wav_enemy_hurt2; break;
                                    case 2: rand_sound=wav_enemy_hurt3; break;
                                }
                                m_pSound->playSimpleSound(rand_sound,1.0);
                            }
                        }
                    }

                    //attack buildings
                    attack_range2=(_player_attack_range+_building_size)*(_player_attack_range+_building_size);

                    for(unsigned int building_i=0;building_i<m_vec_pBuildings.size();building_i++)
                    {
                        //test range
                        if( player_pos.distance2( m_vec_pBuildings[building_i]->get_curr_pos() ) < attack_range2 )
                        {
                            //test angle
                            float player_rot=m_pPlayer_unit->m_rotation_curr+90;
                            if(player_rot>180) player_rot-=360;
                            //cout<<player_rot<<endl;
                            float angle_to_building=player_pos.get_angle_to(m_vec_pBuildings[building_i]->get_curr_pos())-90;
                            if(angle_to_building> 180) angle_to_building-=360;
                            if(angle_to_building<-180) angle_to_building+=360;

                            if( fabs(angle_to_building-player_rot) < _player_attack_angle*0.5 ||
                                //special occasion if faceing down
                                ( player_rot==180 && fabs(angle_to_building+player_rot) < _player_attack_angle*0.5 ) )
                            {

                                //unit within attack range
                                m_vec_pBuildings[building_i]->take_damage( _player_attack_damage );
                                m_vec_pBuildings[building_i]->m_selected=true;

                                //add particles
                                float color[3]={0.5,0.3,0.2};
                                float pos[2]={m_vec_pBuildings[building_i]->get_curr_pos().x,
                                              m_vec_pBuildings[building_i]->get_curr_pos().y};
                                pos[0]+=(rand()%(int)_building_size*2)-_building_size;
                                pos[1]+=(rand()%(int)_building_size*2)-_building_size;
                                m_pPartEng->add_explosion(pos,20,50,2,color,player_rot);

                                //play sound
                                m_pSound->playSimpleSound(wav_hit_building,1.0);
                            }
                        }
                    }
                }
            }
            else m_key_trig_attack=false;

            //player grab/throw unit
            if(m_pKeys_translated[key_v])
            {
                if(!m_key_trig_grabthrow)
                {
                    m_key_trig_grabthrow=true;

                    //trow try
                    bool grab_try=false;
                    switch(m_object_in_hands)
                    {
                        case ho_none:
                        {
                            grab_try=true;//throw disable
                        }break;

                        case ho_solid:
                        {
                            m_object_in_hands=ho_none;
                            //calc throw direction
                            float player_rotation=m_pPlayer_unit->m_rotation_curr;
                            st_pos throw_direction( cosf(player_rotation*_Deg2Rad),sinf(player_rotation*_Deg2Rad) );
                            m_pObject_in_hands->throw_object(throw_direction);

                            //play sound
                            m_pSound->playSimpleSound(wav_throw,1.0);

                        }break;

                        case ho_unit:
                        {
                            m_object_in_hands=ho_none;
                            //calc throw direction
                            float player_rotation=m_pPlayer_unit->m_rotation_curr;
                            st_pos throw_direction( cosf(player_rotation*_Deg2Rad),sinf(player_rotation*_Deg2Rad) );
                            m_pUnit_in_hands->throw_unit(throw_direction);

                            //play sound
                            m_pSound->playSimpleSound(wav_throw,1.0);

                        }break;
                    }

                    //grab
                    if(grab_try)
                    {
                        bool object_found=false;
                        //test if unit nearby
                        float attack_range2=_player_attack_range*_player_attack_range;
                        for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                        {
                            if(m_vec_pUnits[unit_i]==m_pPlayer_unit) continue;

                            //test range
                            if( player_pos.distance2( m_vec_pUnits[unit_i]->get_curr_pos() ) < attack_range2 )
                            {
                                //test angle
                                float player_rot=m_pPlayer_unit->m_rotation_curr+90;
                                if(player_rot>180) player_rot-=360;
                                //cout<<player_rot<<endl;
                                float angle_to_unit=player_pos.get_angle_to(m_vec_pUnits[unit_i]->get_curr_pos())-90;
                                if(angle_to_unit> 180) angle_to_unit-=360;
                                if(angle_to_unit<-180) angle_to_unit+=360;

                                if( fabs(angle_to_unit-player_rot) < _player_attack_angle*0.5 ||
                                    //special occasion if faceing down
                                    ( player_rot==180 && fabs(angle_to_unit+player_rot) < _player_attack_angle*0.5 ) )
                                {
                                    //pick up unit
                                    m_object_in_hands=ho_unit;
                                    m_pUnit_in_hands=m_vec_pUnits[unit_i];
                                    object_found=true;
                                    m_draw_attack_area_timer=1;
                                    break;//only one unit
                                }
                            }
                        }

                        //test if solids nearby
                        if(!object_found)
                        {
                            for(unsigned int object_i=0;object_i<m_vec_pObjects.size();object_i++)
                            {
                                //test range
                                if( player_pos.distance2( m_vec_pObjects[object_i]->get_curr_pos() ) < attack_range2 )
                                {
                                    //test angle
                                    float player_rot=m_pPlayer_unit->m_rotation_curr+90;
                                    if(player_rot>180) player_rot-=360;
                                    //cout<<player_rot<<endl;
                                    float angle_to_unit=player_pos.get_angle_to(m_vec_pObjects[object_i]->get_curr_pos())-90;
                                    if(angle_to_unit> 180) angle_to_unit-=360;
                                    if(angle_to_unit<-180) angle_to_unit+=360;

                                    if( fabs(angle_to_unit-player_rot) < _player_attack_angle*0.5 ||
                                        //special occasion if faceing down
                                        ( player_rot==180 && fabs(angle_to_unit+player_rot) < _player_attack_angle*0.5 ) )
                                    {
                                        //pick up unit
                                        m_object_in_hands=ho_solid;
                                        m_pObject_in_hands=m_vec_pObjects[object_i];
                                        m_pObject_in_hands->m_in_hands=true;
                                        object_found=true;
                                        m_draw_attack_area_timer=1;
                                        break;//only one unit
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else m_key_trig_grabthrow=false;

            //player eat unit
            if(m_pKeys_translated[key_c])
            {
                if(!m_key_trig_eat)
                {
                    m_key_trig_eat=true;

                    //anything to eat
                    if(m_object_in_hands==ho_unit)
                    {
                        m_object_in_hands=ho_none;

                        //regain health
                        m_pPlayer_unit->m_spec.hp_curr+=_unit_eat_hp_regain;
                        if(m_pPlayer_unit->m_spec.hp_curr>m_pPlayer_unit->m_spec.hp_max)
                         m_pPlayer_unit->m_spec.hp_curr=m_pPlayer_unit->m_spec.hp_max;

                        //kill unit
                        m_pUnit_in_hands->take_damage(0,0,true);

                        //play sound
                        m_pSound->playSimpleSound(wav_eat,1.0);
                    }
                }
            }
            else m_key_trig_eat=false;

            //update team squares
            for(int square_x=0;square_x<square_x_max;square_x++)
            for(int square_y=0;square_y<square_y_max;square_y++)
            {
                //reset team flags
                m_arr_team_squares[square_x][square_y]=string(_max_teams,'0');

                //test if square is empty of units
                if( m_arr_col_squares[square_x][square_y].vec_pUnits.empty() )
                {
                    ;//nothing, already reseted to 0
                }
                else//test which teams that are present
                {
                    for(unsigned int unit_i=0;unit_i<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i++)
                    {
                        m_arr_team_squares[square_x][square_y].at(
                         m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i]->get_spec().team )='1';
                    }
                }
            }

            //update units
            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
            {
                m_vec_pUnits[unit_i]->update();

                //m_vec_pUnits[unit_i]->accept_new_pos();//TEMP

                //death test
                if( m_vec_pUnits[unit_i]->get_spec().hp_curr<=0.0 )
                {
                    //special player test
                    if(m_vec_pUnits[unit_i]==m_pPlayer_unit)
                    {
                        //player is dead
                        cout<<"Player is dead\n";
                        m_game_state=gs_gameover;

                        m_hud.end(false);

                        //play sound
                        int rand_sound=0;
                        switch(rand()%3)
                        {
                            case 0: rand_sound=wav_player_death1; break;
                            case 1: rand_sound=wav_player_death2; break;
                            case 2: rand_sound=wav_player_death3; break;
                        }
                        m_pSound->playSimpleSound(rand_sound,1.0);

                        return true;//skip rest
                    }

                    //if unit in hands
                    if(m_object_in_hands==ho_unit)
                    {
                        if(m_vec_pUnits[unit_i]==m_pUnit_in_hands)
                        {
                            m_object_in_hands=ho_none;
                        }
                    }

                    //test if other units had special attack order on this unit
                    for(unsigned int unit_i1=0;unit_i1<m_vec_pUnits.size();unit_i1++)
                    {
                        if(m_vec_pUnits[unit_i1]->m_pUnit_enemy_target==m_vec_pUnits[unit_i])
                        {
                            //reset that target
                            m_vec_pUnits[unit_i1]->m_pUnit_enemy_target=0;
                            m_vec_pUnits[unit_i1]->m_forced_move=false;
                            //m_vec_pUnits[unit_i1]->cancel_movement();
                        }
                    }

                    //remove unit from square arr
                    st_pos square_pos=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                    bool unit_found=false;
                    for(unsigned int square_unit_i=0;
                        square_unit_i<m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.size();
                        square_unit_i++)
                    {
                        if( m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits[square_unit_i]==m_vec_pUnits[unit_i] )
                        {
                            unit_found=true;
                            //found unit in arr
                            m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.erase(
                             m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.begin()+square_unit_i );

                            break;
                        }
                    }

                    if(!unit_found)
                    {
                        cout<<"ERROR: Unit removal from square array: Could not find unit\n";
                    }

                    //add particles
                    float color[3]={0.6,0.2,0.2};
                    float pos[2]={m_vec_pUnits[unit_i]->get_curr_pos().x,
                                  m_vec_pUnits[unit_i]->get_curr_pos().y};
                    m_pPartEng->add_explosion(pos,60,50,1,color,rand()%360);
                    m_pPartEng->add_explosion(pos,60,100,1,color,rand()%360);


                    //play sound
                    int rand_sound=0;
                    switch(rand()%3)
                    {
                        case 0: rand_sound=wav_unit_death1; break;
                        case 1: rand_sound=wav_unit_death2; break;
                        case 2: rand_sound=wav_unit_death3; break;
                    }
                    m_pSound->playSimpleSound(rand_sound,1.0);


                    //add score
                    m_hud.add_score(1);

                    //add to dead units vec
                    int type=0;
                    switch(m_vec_pUnits[unit_i]->m_spec.type)
                    {
                        case ut_villager: type=0; break;
                        case ut_knight: type=1; break;
                        case ut_bowman: type=2; break;
                    }
                    m_vec_dead_units.push_back( st_decal(m_vec_pUnits[unit_i]->get_curr_pos(),
                                                         m_vec_pUnits[unit_i]->m_rotation_curr,type) );


                    //remove unit memory
                    delete m_vec_pUnits[unit_i];

                    //remove unit from unit vec
                    //m_vec_pUnits.erase( m_vec_pUnits.begin()+unit_i );
                    m_vec_pUnits[unit_i]=m_vec_pUnits.back();
                    m_vec_pUnits.pop_back();
                    unit_i--;

                    continue;//skip rest
                }

                //test if should be frozen
                if(m_vec_pUnits[unit_i]!=m_pPlayer_unit)
                {
                    //test if outside screen
                    if(m_cam_pos[1]-500>m_vec_pUnits[unit_i]->get_curr_pos().y ||
                       m_cam_pos[1]+m_window_size[1]+500<m_vec_pUnits[unit_i]->get_curr_pos().y )
                    {
                        //store preunit
                        //cout<<"unit stored\n";
                        int texture=m_tex_unit_move;
                        unit_spec specs=m_vec_pUnits[unit_i]->m_spec;
                        switch(specs.type)
                        {
                            case ut_villager: texture=m_tex_unit_move; break;
                            case ut_knight: texture=m_tex_knight; break;
                            case ut_bowman: texture=m_tex_bowman; break;
                        }
                        m_vec_preunits.push_back( st_pre_unit(specs,m_vec_pUnits[unit_i]->get_curr_pos(),texture) );

                        //if unit in hands
                        if(m_object_in_hands==ho_unit)
                        {
                            if(m_vec_pUnits[unit_i]==m_pUnit_in_hands)
                            {
                                m_object_in_hands=ho_none;
                            }
                        }

                        //test if other units had special attack order on this unit
                        for(unsigned int unit_i1=0;unit_i1<m_vec_pUnits.size();unit_i1++)
                        {
                            if(m_vec_pUnits[unit_i1]->m_pUnit_enemy_target==m_vec_pUnits[unit_i])
                            {
                                //reset that target
                                m_vec_pUnits[unit_i1]->m_pUnit_enemy_target=0;
                                m_vec_pUnits[unit_i1]->m_forced_move=false;
                                //m_vec_pUnits[unit_i1]->cancel_movement();
                            }
                        }

                        //remove unit from square arr
                        st_pos square_pos=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                        bool unit_found=false;
                        for(unsigned int square_unit_i=0;
                            square_unit_i<m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.size();
                            square_unit_i++)
                        {
                            if( m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits[square_unit_i]==m_vec_pUnits[unit_i] )
                            {
                                unit_found=true;
                                //found unit in arr
                                m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.erase(
                                 m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.begin()+square_unit_i );

                                break;
                            }
                        }

                        if(!unit_found)
                        {
                            cout<<"ERROR: Unit removal from square array: Could not find unit\n";
                        }

                        //remove unit
                        delete m_vec_pUnits[unit_i];
                        m_vec_pUnits[unit_i]=m_vec_pUnits.back();
                        m_vec_pUnits.pop_back();
                        unit_i--;

                        continue;//skip rest
                    }
                }

                //if have moved, update col square test
                //if(m_vec_pUnits[unit_i]->m_have_moved)
                {
                    m_vec_pUnits[unit_i]->m_have_moved=false;

                    //test square pos
                    st_pos unit_pos=m_vec_pUnits[unit_i]->get_curr_pos();
                    int col_square_x=unit_pos.x/_col_grid_size;
                    int col_square_y=unit_pos.y/_col_grid_size;
                    //cap grid pos, if unit temp outside world limit
                    if(col_square_x<0) col_square_x=0;
                    else if(col_square_x>_world_width/_col_grid_size-1) col_square_x=_world_width/_col_grid_size-1;
                    if(col_square_y<0) col_square_y=0;
                    else if(col_square_y>_world_height/_col_grid_size-1) col_square_y=_world_height/_col_grid_size-1;

                    st_pos old_col_square_pos=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                    if( old_col_square_pos.x!=col_square_x || old_col_square_pos.y!=col_square_y )
                    {
                        //needs update
                        //cout<<"Unit moved to new col_square\n";

                        //remove from old square pos
                        bool unit_found=false;
                        for(unsigned int square_unit_i=0;
                            square_unit_i<m_arr_col_squares[(int)old_col_square_pos.x][(int)old_col_square_pos.y].vec_pUnits.size();
                            square_unit_i++)
                        {
                            if( m_arr_col_squares[(int)old_col_square_pos.x][(int)old_col_square_pos.y].vec_pUnits[square_unit_i]==m_vec_pUnits[unit_i] )
                            {
                                unit_found=true;

                                //remove from arr vec
                                m_arr_col_squares[(int)old_col_square_pos.x][(int)old_col_square_pos.y].vec_pUnits.erase(
                                 m_arr_col_squares[(int)old_col_square_pos.x][(int)old_col_square_pos.y].vec_pUnits.begin()+square_unit_i );

                                break;
                            }
                        }
                        if(!unit_found)
                        {
                            cout<<"ERROR: Could not find Unit in old col_square pos\n";
                        }
                        //cout<<"3 - "<<col_square_x<<", "<<col_square_y<<" - ";

                        //place unit in new col_square
                        m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits[unit_i] );
                        m_vec_pUnits[unit_i]->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );

                        //cout<<" Old square: "<<old_col_square_pos.x<<", "<<old_col_square_pos.y<<endl;
                        //cout<<" New square: "<<col_square_x<<", "<<col_square_y<<endl;
                    }
                }

                //attack view update
                bool attack_done=false;
                unit_spec specs=m_vec_pUnits[unit_i]->get_spec();
                float range_px_attack=specs.attack_range;
                float range_px2_attack=range_px_attack*range_px_attack;
                float range_px2_attack_to_player=(range_px_attack+_player_size)*(range_px_attack+_player_size);
                int range_squares_attack=range_px_attack/_col_grid_size+1;
                float range_px_view=specs.view_range;
                float range_px2_view=range_px_view*range_px_view;
                int range_squares_view=range_px_view/_col_grid_size+1;
                bool attack_only_player=true;

                //unit type
                if(specs.type==ut_villager)
                {
                    //does not attack, flees
                    attack_done=true;
                }

                //special attack order active
                if(m_vec_pUnits[unit_i]->m_pUnit_enemy_target!=0 && m_vec_pUnits[unit_i]!=m_pPlayer_unit &&!attack_only_player)
                {
                    //test if that unit is within attack range
                    float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2( m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos() );
                    if(range_px2_attack>dist2)
                    {
                        //rotation test, copy to other square tests
                        st_pos rel_pos(m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos().x-m_vec_pUnits[unit_i]->get_curr_pos().x,
                                       m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos().y-m_vec_pUnits[unit_i]->get_curr_pos().y);
                        float rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;

                        if( m_vec_pUnits[unit_i]->aim_to_target(rotation_target,true) && m_vec_pUnits[unit_i]->is_ready_to_attack() )//true of rotation ok
                        {
                            //cout<<"ATTACK\n";
                            //in range, attack
                            m_vec_pUnits[unit_i]->attack_action_done();

                            switch(specs.type)
                            {
                                case ut_knight:
                                {
                                    //give direct damage
                                    m_vec_pUnits[unit_i]->m_pUnit_enemy_target->take_damage(specs.damage,0);
                                }break;

                                case ut_bowman:
                                {
                                    //create projectile, if bowman
                                    m_vec_projectiles.push_back( projectile( m_vec_pUnits[unit_i]->get_curr_pos(),
                                                                 m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos(),
                                                                 m_vec_pUnits[unit_i]->get_spec().projectile_speed,
                                                                 m_vec_pUnits[unit_i]->get_spec().damage,
                                                                 m_vec_pUnits[unit_i]->get_spec().damage_range,
                                                                 m_vec_pUnits[unit_i]->get_spec().team ) );
                                }break;
                            }

                        }

                        attack_done=true;//might not have attacked but will skip attack testing
                        continue;//go to next unit
                    }
                    else//not within attack range
                    {
                        //if this unit can be seen by any allied unit, update the target pos and move there
                        if(m_vec_pUnits[unit_i]->m_pUnit_enemy_target->m_unit_seen_by_team[specs.team])
                        {
                            //cout<<"enemy spotted\n";
                            //update enemy pos
                            m_vec_pUnits[unit_i]->move_to(m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos(),true,9999,
                                                          m_vec_pUnits[unit_i]->m_pUnit_enemy_target);
                        }
                    }
                }

                //enemy to attack test normal
                if( m_vec_pUnits[unit_i]->is_ready_to_attack() &&
                    m_vec_pUnits[unit_i]!=m_pPlayer_unit &&
                    specs.type!=ut_villager )//cancel villager
                {
                    if(attack_only_player)
                    {
                        //test dist
                        float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2( m_pPlayer_unit->get_curr_pos() );
                        if(range_px2_attack_to_player>dist2)
                        {
                            //rotation test, copy to other square tests
                            st_pos rel_pos(m_pPlayer_unit->get_curr_pos().x-m_vec_pUnits[unit_i]->get_curr_pos().x,
                                           m_pPlayer_unit->get_curr_pos().y-m_vec_pUnits[unit_i]->get_curr_pos().y);
                            float rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
                            //cout<<rotation_target<<endl;

                            if( m_vec_pUnits[unit_i]->aim_to_target(rotation_target) )//true of rotation ok
                            {
                                //cout<<"ATTACK in other square "<<specs.team<<endl;;
                                attack_done=true;
                                //in range, attack
                                m_vec_pUnits[unit_i]->attack_action_done();

                                switch(specs.type)
                                {
                                    case ut_knight:
                                    {
                                        //give direct damage
                                        m_pPlayer_unit->take_damage(specs.damage,0);

                                        //play sound
                                        m_pSound->playSimpleSound(wav_swordhit,1.0);

                                    }break;

                                    case ut_bowman:
                                    {
                                        //create projectile, if bowman
                                        m_vec_projectiles.push_back( projectile( m_vec_pUnits[unit_i]->get_curr_pos(),
                                                                     m_pPlayer_unit->get_curr_pos(),
                                                                     m_vec_pUnits[unit_i]->get_spec().projectile_speed,
                                                                     m_vec_pUnits[unit_i]->get_spec().damage,
                                                                     m_vec_pUnits[unit_i]->get_spec().damage_range,
                                                                     m_vec_pUnits[unit_i]->get_spec().team ) );

                                        //play sound
                                        m_pSound->playSimpleSound(wav_bowfire,1.0);

                                    }break;
                                }
                            }
                        }
                    }
                    else
                    {
                        //test if any target close

                        //test center square first
                        st_pos center_square=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                        bool enemy_in_square=false;
                        bool priority_to_center_square=false;//toggle on/off
                        if(priority_to_center_square && !attack_done)
                        {
                            for(int team_i=0;team_i<_max_teams;team_i++)
                            {
                                if(team_i==specs.team) continue;

                                if(m_arr_team_squares[(int)center_square.x][(int)center_square.y].at(team_i)!='0')
                                {
                                    enemy_in_square=true;
                                    break;
                                }
                            }

                            if(enemy_in_square)//if not, skip this and test other squares
                             for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits.size();unit_i1++)
                            {
                                if(m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                                {
                                    //other team found, test dist
                                    float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2(
                                          m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos() );
                                    if(range_px2_attack>dist2)
                                    {
                                        //rotation test, copy to other square tests
                                        st_pos rel_pos(m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos().x-m_vec_pUnits[unit_i]->get_curr_pos().x,
                                                       m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos().y-m_vec_pUnits[unit_i]->get_curr_pos().y);
                                        float rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;

                                        if( m_vec_pUnits[unit_i]->aim_to_target(rotation_target) )//true of rotation ok
                                        {
                                            attack_done=true;
                                            //in range, attack
                                            m_vec_pUnits[unit_i]->attack_action_done();

                                            switch(specs.type)
                                            {
                                                case ut_knight:
                                                {
                                                    //give direct damage
                                                    m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->take_damage(specs.damage,0);
                                                }break;

                                                case ut_bowman:
                                                {
                                                    //create projectile, if bowman
                                                    m_vec_projectiles.push_back( projectile( m_vec_pUnits[unit_i]->get_curr_pos(),
                                                                                 m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos(),
                                                                                 m_vec_pUnits[unit_i]->get_spec().projectile_speed,
                                                                                 m_vec_pUnits[unit_i]->get_spec().damage,
                                                                                 m_vec_pUnits[unit_i]->get_spec().damage_range,
                                                                                 m_vec_pUnits[unit_i]->get_spec().team ) );
                                                }break;
                                            }

                                            //report that unit visible to this team
                                            //m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->spotted_by_team(specs.team);
                                        }

                                        break;//do not find another unit in sight even if aim angle not ok
                                    }
                                }
                            }
                        }

                        //test squares in reach
                        if(!attack_done)
                        {
                            //cout<<"RANGE: x "<<(int)center_square.x-range_squares<<" to "<<(int)center_square.x+range_squares<<endl;

                            for(int square_x=(int)center_square.x-range_squares_attack;
                                square_x<square_x_max && square_x<=(int)center_square.x+range_squares_attack; square_x++)
                            for(int square_y=(int)center_square.y-range_squares_attack;
                                square_y<square_y_max && square_y<=(int)center_square.y+range_squares_attack; square_y++)
                            {
                                //cout<<"ATTACK square: "<<square_x<<", "<<square_y<<endl;

                                if(attack_done) break;
                                if(square_x<0 || square_y<0 ) continue;//outside world
                                if(priority_to_center_square && square_x==(int)center_square.x
                                                             && square_y==(int)center_square.y) continue; //skip center, if already tested

                                enemy_in_square=false;
                                for(int team_i=0;team_i<_max_teams;team_i++)
                                {
                                    if(team_i==specs.team) continue;

                                    if(m_arr_team_squares[square_x][square_y].at(team_i)!='0')
                                    {
                                        //cout<<" enemy found\n";
                                        enemy_in_square=true;
                                        break;
                                    }
                                }

                                if(enemy_in_square)//if not, skip this ant test other squares
                                 for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                                {
                                    if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                                    {
                                        //other team found, test dist
                                        float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2(
                                              m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos() );
                                        bool attack_player=false;
                                        if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]==m_pPlayer_unit)
                                        {
                                            //test other distance based on player size
                                            if(range_px2_attack_to_player>dist2)
                                            {
                                                attack_player=true;
                                            }
                                            //cout<<range_px2_attack_to_player<<"\t"<<dist2<<endl;
                                        }
                                        if(range_px2_attack>dist2 || attack_player)
                                        {
                                            //rotation test, copy to other square tests
                                            st_pos rel_pos(m_arr_col_squares[(int)square_x][(int)square_y].vec_pUnits[unit_i1]->get_curr_pos().x-m_vec_pUnits[unit_i]->get_curr_pos().x,
                                                           m_arr_col_squares[(int)square_x][(int)square_y].vec_pUnits[unit_i1]->get_curr_pos().y-m_vec_pUnits[unit_i]->get_curr_pos().y);
                                            float rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;

                                            if( m_vec_pUnits[unit_i]->aim_to_target(rotation_target) )//true of rotation ok
                                            {
                                                //cout<<"ATTACK in other square "<<specs.team<<endl;;
                                                attack_done=true;
                                                //in range, attack
                                                m_vec_pUnits[unit_i]->attack_action_done();

                                                switch(specs.type)
                                                {
                                                    case ut_knight:
                                                    {
                                                        //give direct damage
                                                        m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->take_damage(specs.damage,0);
                                                    }break;

                                                    case ut_bowman:
                                                    {
                                                        //create projectile, if bowman
                                                        m_vec_projectiles.push_back( projectile( m_vec_pUnits[unit_i]->get_curr_pos(),
                                                                                     m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos(),
                                                                                     m_vec_pUnits[unit_i]->get_spec().projectile_speed,
                                                                                     m_vec_pUnits[unit_i]->get_spec().damage,
                                                                                     m_vec_pUnits[unit_i]->get_spec().damage_range,
                                                                                     m_vec_pUnits[unit_i]->get_spec().team ) );
                                                    }break;
                                                }

                                                //report that unit visible to this team
                                                //m_arr_col_squares[(int)square_x][(int)square_y].vec_pUnits[unit_i1]->spotted_by_team(specs.team);
                                            }

                                            break;//do not find another unit in sight even if aim angle not ok
                                        }
                                        //else cout<<"attack distance fail\n";
                                    }
                                }
                                //else continue;
                            }
                        }
                    }
                }

                //spot enemy test, if not attacking
                bool go_through_all_units_in_spotting=true;
                bool spot_only_player=true;
                if(!attack_done || go_through_all_units_in_spotting)
                {
                    if(spot_only_player)
                    {
                        float dist=m_vec_pUnits[unit_i]->get_curr_pos().distance2( m_pPlayer_unit->get_curr_pos() );
                        if(range_px2_view>dist)
                        {
                            bool is_villager=false;
                            //tell unit to move towards that unit (will recalc angle every frame), if unit not in stand mode
                            if( (m_vec_pUnits[unit_i]->m_attack_mode==am_pursuit || m_vec_pUnits[unit_i]->m_attack_mode==am_defence) &&
                                 m_vec_pUnits[unit_i]->m_pUnit_enemy_target==0 && m_vec_pUnits[unit_i]!=m_pPlayer_unit)
                            {
                                //cout<<"move to";
                                switch(specs.type)
                                {
                                    case ut_villager:
                                    {
                                        is_villager=true;
                                        //flee in opposite direction
                                        st_pos flee_dir=m_vec_pUnits[unit_i]->get_curr_pos().get_rel_pos_to(
                                                         m_pPlayer_unit->get_curr_pos() ) ;
                                        flee_dir.normalize();
                                        st_pos new_pos=(flee_dir*_unit_flee_distance)+m_vec_pUnits[unit_i]->get_curr_pos();
                                        m_vec_pUnits[unit_i]->move_to( new_pos );
                                        //cout<<"Flee dir: "<<flee_dir.x<<"\t"<<flee_dir.y<<endl;

                                    }break;

                                    case ut_bowman:
                                    {
                                        //test if already within distance, toa avoid placement behind player
                                        float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2( m_pPlayer_unit->get_curr_pos() );
                                        if(range_px2_attack_to_player>dist2)
                                         break;

                                        //place at a distance
                                        //calc nearest point to attack
                                        st_pos from_enemy_dir=m_vec_pUnits[unit_i]->get_curr_pos().get_rel_pos_to(
                                                               m_pPlayer_unit->get_curr_pos() ) ;
                                        from_enemy_dir.normalize();
                                        //scale to fire range x0.75
                                        from_enemy_dir*=specs.attack_range*0.75;
                                        st_pos new_pos=(from_enemy_dir)+m_pPlayer_unit->get_curr_pos();
                                        m_vec_pUnits[unit_i]->move_to( new_pos );

                                    }break;

                                    case ut_knight:
                                    {
                                        //go to player pos, melee
                                        m_vec_pUnits[unit_i]->move_to( m_pPlayer_unit->get_curr_pos() );
                                    }break;
                                }
                            }

                            //play sound
                            int tes_val=0;
                            if(is_villager) tes_val=5;
                            if(rand()%_unit_shout_prop<=tes_val)
                            {
                                if(m_vec_pUnits[unit_i]!=m_pPlayer_unit)
                                {
                                    int rand_sound=0;
                                    switch(rand()%3)
                                    {
                                        case 0: rand_sound=wav_enemy_shout1; break;
                                        case 1: rand_sound=wav_enemy_shout2; break;
                                        case 2: rand_sound=wav_enemy_shout3; break;
                                    }
                                    if(is_villager)
                                    {
                                        switch(rand()%3)
                                        {
                                            case 0: rand_sound=wav_villager_shout1; break;
                                            case 1: rand_sound=wav_villager_shout2; break;
                                            case 2: rand_sound=wav_villager_shout3; break;
                                        }
                                    }
                                    if(is_villager)
                                     m_pSound->playSimpleSound(rand_sound,0.2);
                                    else
                                     m_pSound->playSimpleSound(rand_sound,1.0);
                                }
                            }
                        }
                    }
                    else
                    {
                        //is there any enemies within view range
                        bool enemy_spotted=false;

                        //test center square first
                        st_pos center_square=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                        bool enemy_in_square=false;
                        bool priority_to_center_square=false;//toggle on/off
                        if(priority_to_center_square)
                        {
                            for(int team_i=0;team_i<_max_teams;team_i++)
                            {
                                if(team_i==specs.team) continue;

                                if(m_arr_team_squares[(int)center_square.x][(int)center_square.y].at(team_i)!='0')
                                {
                                    enemy_in_square=true;
                                    break;
                                }
                            }

                            if(enemy_in_square)//if not, skip this and test other squares
                             for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits.size();unit_i1++)
                            {
                                if(m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                                {
                                    //other team found, test dist
                                    float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2(
                                          m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos() );
                                    if(range_px2_view>dist2)
                                    {
                                        //cout<<"enemy spotted\n";

                                        enemy_spotted=true;

                                        //report that unit visible to this team
                                        //m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->spotted_by_team(specs.team);

                                        //if that enemy is within fire range, dont move
                                        if(range_px2_attack>dist2 && !go_through_all_units_in_spotting) break;//current unit is just reloading

                                        //tell unit to move towards that unit (will recalc angle every frame), if unit not in stand mode
                                        if( (m_vec_pUnits[unit_i]->m_attack_mode==am_pursuit || m_vec_pUnits[unit_i]->m_attack_mode==am_defence) &&
                                             m_vec_pUnits[unit_i]->m_pUnit_enemy_target==0)
                                        {
                                            m_vec_pUnits[unit_i]->move_to( m_arr_col_squares[(int)center_square.x][(int)center_square.y].
                                                                           vec_pUnits[unit_i1]->get_curr_pos() );
                                        }

                                        if(!go_through_all_units_in_spotting)
                                         break;//do not find another unit in sight
                                    }
                                }
                            }
                        }

                        //test squares in reach
                        if(!enemy_spotted || go_through_all_units_in_spotting)
                        {
                            //cout<<"RANGE: x "<<(int)center_square.x-range_squares<<" to "<<(int)center_square.x+range_squares<<endl;

                            for(int square_x=(int)center_square.x-range_squares_view;
                                square_x<square_x_max && square_x<=(int)center_square.x+range_squares_view; square_x++)
                            for(int square_y=(int)center_square.y-range_squares_view;
                                square_y<square_y_max && square_y<=(int)center_square.y+range_squares_view; square_y++)
                            {
                                //cout<<"ATTACK square: "<<square_x<<", "<<square_y<<endl;

                                if(enemy_spotted) break;
                                if(square_x<0 || square_y<0 ) continue;//outside world
                                if(priority_to_center_square && square_x==(int)center_square.x
                                                             && square_y==(int)center_square.y) continue; //skip center, if already tested

                                enemy_in_square=false;
                                for(int team_i=0;team_i<_max_teams;team_i++)
                                {
                                    if(team_i==specs.team) continue;

                                    if(m_arr_team_squares[square_x][square_y].at(team_i)!='0')
                                    {
                                        //cout<<" enemy found\n";
                                        enemy_in_square=true;
                                        break;
                                    }
                                }

                                if(enemy_in_square)//if not, skip this ant test other squares
                                 for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                                {
                                    if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                                    {
                                        //other team found, test dist
                                        float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2(
                                              m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos() );
                                        if(range_px2_view>dist2)
                                        {
                                            //cout<<"enemy spotted\n";

                                            enemy_spotted=true;

                                            //report that unit visible to this team
                                            //m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->spotted_by_team(specs.team);

                                            //if that enemy is within fire range, dont move
                                            if(range_px2_attack>dist2 && !go_through_all_units_in_spotting) break;//current unit is just reloading

                                            //tell unit to move towards that unit (will recalc angle every frame), if unit not in stand mode
                                            if( (m_vec_pUnits[unit_i]->m_attack_mode==am_pursuit || m_vec_pUnits[unit_i]->m_attack_mode==am_defence) &&
                                                 m_vec_pUnits[unit_i]->m_pUnit_enemy_target==0 && m_vec_pUnits[unit_i]!=m_pPlayer_unit)
                                            {
                                                //cout<<"move to";
                                                switch(specs.type)
                                                {
                                                    case ut_villager:
                                                    {
                                                        //flee in opposite direction
                                                        st_pos flee_dir=m_vec_pUnits[unit_i]->get_curr_pos().get_rel_pos_to(
                                                                         m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->
                                                                         get_curr_pos() ) ;
                                                        flee_dir.normalize();
                                                        st_pos new_pos=(flee_dir*_unit_flee_distance)+m_vec_pUnits[unit_i]->get_curr_pos();
                                                        m_vec_pUnits[unit_i]->move_to( new_pos );
                                                        //cout<<"Flee dir: "<<flee_dir.x<<"\t"<<flee_dir.y<<endl;

                                                    }break;

                                                    case ut_bowman:
                                                    {
                                                        //place at a distance
                                                        //calc nearest point to attack
                                                        st_pos from_enemy_dir=m_vec_pUnits[unit_i]->get_curr_pos().get_rel_pos_to(
                                                                               m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->
                                                                               get_curr_pos() ) ;
                                                        from_enemy_dir.normalize();
                                                        //scale to fire range x0.75
                                                        from_enemy_dir*=specs.attack_range*0.75;
                                                        st_pos new_pos=from_enemy_dir+
                                                                       m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos();
                                                        m_vec_pUnits[unit_i]->move_to( new_pos );

                                                    }break;

                                                    case ut_knight:
                                                    {
                                                        //go to player pos, melee
                                                        m_vec_pUnits[unit_i]->move_to( m_arr_col_squares[square_x][square_y].
                                                                                       vec_pUnits[unit_i1]->get_curr_pos() );
                                                    }break;
                                                }
                                            }

                                            //play sound
                                            if(rand()%_unit_shout_prop==0 && m_vec_pUnits[unit_i]!=m_pPlayer_unit)
                                            {
                                                int rand_sound=0;
                                                switch(rand()%3)
                                                {
                                                    case 0: rand_sound=wav_enemy_shout1; break;
                                                    case 1: rand_sound=wav_enemy_shout2; break;
                                                    case 2: rand_sound=wav_enemy_shout3; break;
                                                }
                                                m_pSound->playSimpleSound(rand_sound,1.0);
                                            }


                                            if(!go_through_all_units_in_spotting)
                                             break;
                                        }
                                        //else cout<<"attack distance fail\n";
                                    }
                                }
                                //else continue;
                            }
                        }
                    }
                }
            }

            //player-building col test
            float player_building_dist2=(_building_size+_player_size)*(_building_size+_player_size);
            //bool player_pos_ok=true;
            for(unsigned int building_i=0;building_i<m_vec_pBuildings.size();building_i++)
            {
                if(m_vec_pBuildings[building_i]->is_pos_inside_circle2(m_pPlayer_unit->get_curr_pos(),player_building_dist2))
                {
                    //strong push of player
                    st_pos player_pos=m_pPlayer_unit->get_curr_pos();
                    st_pos building_pos=m_vec_pBuildings[building_i]->get_curr_pos();
                    st_pos rel_pos(player_pos.x-building_pos.x,player_pos.y-building_pos.y);
                    float push_factor_building=4.0;
                    float push_min_limit=3;
                    if( fabs(rel_pos.x)<push_min_limit && fabs(rel_pos.y)<push_min_limit)
                     rel_pos.x=push_min_limit;//force right push
                    m_pPlayer_unit->push_unit( st_pos( rel_pos.x*push_factor_building,
                                                       rel_pos.y*push_factor_building ),
                                               m_pPlayer_unit->is_moving() );

                    //player_pos_ok=false;
                    //break;
                }
            }
            //player-object col test
            float player_object_dist2=(_object_size+_player_size)*(_object_size+_player_size);
            //bool player_pos_ok=true;
            for(unsigned int object_i=0;object_i<m_vec_pObjects.size();object_i++)
            {
                if(m_vec_pObjects[object_i]->is_pos_inside_circle2(m_pPlayer_unit->get_curr_pos(),player_object_dist2))
                {
                    //strong push of player
                    st_pos player_pos=m_pPlayer_unit->get_curr_pos();
                    st_pos object_pos=m_vec_pObjects[object_i]->get_curr_pos();
                    st_pos rel_pos(player_pos.x-object_pos.x,player_pos.y-object_pos.y);
                    float push_factor_building=4.0;
                    float push_min_limit=3;
                    if( fabs(rel_pos.x)<push_min_limit && fabs(rel_pos.y)<push_min_limit)
                     rel_pos.x=push_min_limit;//force right push
                    m_pPlayer_unit->push_unit( st_pos( rel_pos.x*push_factor_building,
                                                       rel_pos.y*push_factor_building ),
                                               m_pPlayer_unit->is_moving() );

                    //player_pos_ok=false;
                    //break;
                }
            }
            //if(player_pos_ok) m_pPlayer_unit->accept_new_pos();

            //unit square collision test
            int col_test_counter=0;
            for(int square_x=0;square_x<square_x_max;square_x++)
            for(int square_y=0;square_y<square_y_max;square_y++)
            {
                for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                {
                    //skip player unit
                    //if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]==m_pPlayer_unit) continue;

                    bool unit_projectile=false;
                    if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_projectile_mode) unit_projectile=true;

                    st_pos unit_new_pos=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_new_pos();
                    st_pos unit_curr_pos=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos();

                    //unit-unit test
                    st_pos pos_of_col_object;
                    unit* pCollided_unit=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1];//itself, temp
                    bool collision_done=false;
                    for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i2++)
                    {
                        if(unit_i1==unit_i2) continue;

                        col_test_counter++;

                        float col_size_factor=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_size/
                                              m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2]->m_size*1.0;
                        if(col_size_factor<2.0) col_size_factor=2.0;

                        if( m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,col_size_factor,true) )
                        {
                            collision_done=true;
                            pos_of_col_object=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2]->get_curr_pos();
                            pCollided_unit=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2];

                            //test if unit projectile, remove both units, unless player unit
                            if(unit_projectile)
                            {
                                if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2]!=m_pPlayer_unit)
                                {
                                    //cout<<"unit proj - unit\n";

                                    //remove both units
                                    m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2]->take_damage(0,0,true);
                                    m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->take_damage(0,0,true);
                                }
                            }

                            break;
                        }
                    }

                    //unit-building test
                    st_pos pos_of_col_building;
                    bool collision_building_done=false;
                    for(unsigned int building_i=0;building_i<m_vec_pBuildings.size();building_i++)
                    {
                        col_test_counter++;

                        //new pos inside building, only cancel movement
                        if( m_vec_pBuildings[building_i]->is_pos_inside(unit_new_pos) )
                        {
                            collision_building_done=true;
                            pos_of_col_building=m_vec_pBuildings[building_i]->get_curr_pos();
                            //break later
                        }

                        //old pos inside building, push unit
                        if(collision_building_done)
                        {
                            //test if unit projectile
                            if(unit_projectile)
                            {
                                //cout<<"unit proj - building\n";

                                //damage building
                                m_vec_pBuildings[building_i]->take_damage(_unit_projectile_damage);

                                //play sound
                                m_pSound->playSimpleSound(wav_hit_building,1.0);

                                //remove unit
                                m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->take_damage(0,0,true);
                            }

                            if( m_vec_pBuildings[building_i]->is_pos_inside(unit_curr_pos) )
                            {
                                //push unit
                                st_pos rel_pos( unit_curr_pos.x-pos_of_col_building.x,unit_curr_pos.y-pos_of_col_building.y );
                                float push_factor_building=1.0;
                                float push_min_limit=3;
                                if( fabs(rel_pos.x)<push_min_limit && fabs(rel_pos.y)<push_min_limit) rel_pos.x=push_min_limit;//force right push
                                m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->push_unit( st_pos( rel_pos.x*push_factor_building,
                                                                                                              rel_pos.y*push_factor_building ),
                                                                                                      m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() );
                                break;
                            }
                        }
                    }

                    //unit-object test
                    st_pos pos_of_col_solidobject;
                    bool collision_object_done=false;
                    for(unsigned int object_i=0;object_i<m_vec_pObjects.size();object_i++)
                    {
                        col_test_counter++;

                        //new pos inside building, only cancel movement
                        if( m_vec_pObjects[object_i]->is_pos_inside(unit_new_pos) )
                        {
                            collision_object_done=true;
                            pos_of_col_solidobject=m_vec_pObjects[object_i]->get_curr_pos();
                            //break later
                        }

                        //old pos inside building, push unit
                        if(collision_object_done)
                        {
                            /*//test if unit projectile
                            if(unit_projectile)
                            {
                                //cout<<"unit proj - building\n";

                                //remove object
                                m_vec_pObjects[object_i]->take_damage(_unit_projectile_damage);

                                //remove unit
                                m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->take_damage(0,0,true);
                            }*/

                            if( m_vec_pObjects[object_i]->is_pos_inside(unit_curr_pos) )
                            {
                                //push unit
                                st_pos rel_pos( unit_curr_pos.x-pos_of_col_solidobject.x,unit_curr_pos.y-pos_of_col_solidobject.y );
                                float push_factor_object=1.0;
                                float push_min_limit=3;
                                if( fabs(rel_pos.x)<push_min_limit && fabs(rel_pos.y)<push_min_limit) rel_pos.x=push_min_limit;//force right push
                                m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->push_unit( st_pos( rel_pos.x*push_factor_object,
                                                                                                              rel_pos.y*push_factor_object ),
                                                                                                      m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() );
                                break;
                            }
                        }
                    }

                    //near border test
                    if(!collision_done)
                    {
                        //calc rel square pos
                        float rel_x=unit_new_pos.x-int(unit_new_pos.x/_col_grid_size)*_col_grid_size;
                        float rel_y=unit_new_pos.y-int(unit_new_pos.y/_col_grid_size)*_col_grid_size;

                        //left side
                        if(square_x>0 && rel_x<=_unit_size*2.0 && !collision_done)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x-1][square_y].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x-1][square_y].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x-1][square_y].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x-1][square_y].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //right side
                        if(square_x<square_x_max-1 && rel_x>=_col_grid_size-_unit_size*2.0 && !collision_done)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x+1][square_y].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x+1][square_y].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x+1][square_y].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x+1][square_y].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //top side
                        if(square_y>0 && rel_y<=_unit_size*2.0 && !collision_done)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x][square_y-1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x][square_y-1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x][square_y-1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x][square_y-1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //low side
                        if(square_y<square_y_max-1 && rel_y>=_col_grid_size-_unit_size*2.0 && !collision_done)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x][square_y+1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x][square_y+1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x][square_y+1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x][square_y+1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //low left side
                        if(square_y<square_y_max-1 && rel_y>=_col_grid_size-_unit_size*2.0 && !collision_done &&
                           square_x>0 && rel_x<=_unit_size*2.0)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x-1][square_y+1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x-1][square_y+1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x-1][square_y+1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x-1][square_y+1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //low right side
                        if(square_y<square_y_max-1 && rel_y>=_col_grid_size-_unit_size*2.0 && !collision_done &&
                           square_x<square_x_max-1 && rel_x>=_col_grid_size-_unit_size*2.0)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x+1][square_y+1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x+1][square_y+1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x+1][square_y+1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x+1][square_y+1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //top left side
                        if(square_y>0 && rel_y<=_unit_size*2.0 && !collision_done &&
                           square_x>0 && rel_x<=_unit_size*2.0)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x-1][square_y-1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x-1][square_y-1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x-1][square_y-1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x-1][square_y-1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //top right side
                        if(square_y>0 && rel_y<=_unit_size*2.0 && !collision_done &&
                           square_x<square_x_max-1 && rel_x>=_col_grid_size-_unit_size*2.0)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x+1][square_y-1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x+1][square_y-1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x+1][square_y-1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x+1][square_y-1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                    }

                    //test if collision can be ignored (unit-unit)
                    if(collision_done && !collision_building_done && !collision_object_done)
                    {
                        if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_col_id==pCollided_unit->m_col_id)
                        {
                            //same id
                            if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_col_state==active ||
                               pCollided_unit->m_col_state==active )
                            {
                                //one is active (the other should be active or passive)
                                //noone can be off (none)
                                if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_col_state!=none &&
                                   pCollided_unit->m_col_state!=none)
                                {
                                    //test if any of the units are inside the formation square
                                    //find col_id
                                    int col_id_ind=-1;
                                    for(unsigned int col_i=0;col_i<m_vec_col_id.size();col_i++)
                                    {
                                        if(m_vec_col_id[col_i].id==pCollided_unit->m_col_id)
                                        {
                                            col_id_ind=col_i;
                                            break;
                                        }
                                    }
                                    if(col_id_ind!=-1)
                                    {
                                        if( m_vec_col_id[col_id_ind].is_pos_inside_square(unit_new_pos) ||
                                            m_vec_col_id[col_id_ind].is_pos_inside_square(pCollided_unit->get_curr_pos() ) )
                                        {
                                            //at least one unit inside the formation square
                                            collision_done=false;//collision avoided
                                        }
                                    }
                                    else//bad
                                    {
                                        cout<<"ERROR: Could not find col_id in vector\n";
                                    }
                                }
                            }
                        }
                    }

                    if(!collision_done && !collision_building_done && !collision_object_done)
                     m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->accept_new_pos();
                    else if(collision_done)//trigger unit push
                    {
                        //calc push direction
                        st_pos rel_pos( unit_curr_pos.x-pos_of_col_object.x,unit_curr_pos.y-pos_of_col_object.y );
                        //push unit
                        float push_min_limit=3;
                        if( fabs(rel_pos.x)<push_min_limit && fabs(rel_pos.y)<push_min_limit) rel_pos.x=push_min_limit;//force right push
                        //m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->push_unit(rel_pos);
                        //push other unit, stronger if other unit is moving
                        float push_factor=1.0;
                        if(m_pPlayer_unit==m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1])
                         push_factor=4.0;
                        if(m_pPlayer_unit==pCollided_unit)
                         push_factor=0.4;
                        //if( m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() ) push_factor=1.0;
                        if(!m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() &&
                           pCollided_unit->is_moving() ) ;//no push if idle detects moveing inside
                        else if( m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() &&
                                 pCollided_unit->is_moving()) //both are pushing, rand to push otherwise front element will be favourd
                        {
                            if(rand()%2==0)//push collided
                            {
                                pCollided_unit->push_unit( st_pos( -rel_pos.x*push_factor,-rel_pos.y*push_factor ),
                                                           m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() );
                            }
                            else//push current unit
                            {
                                m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->push_unit( st_pos( rel_pos.x*push_factor,rel_pos.y*push_factor ),
                                                                                                      pCollided_unit->is_moving() );
                            }
                        }
                        else//the idle is pushed
                         pCollided_unit->push_unit( st_pos( -rel_pos.x*push_factor,-rel_pos.y*push_factor ),
                                                    m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() );
                        //cout<<push_factor<<endl;

                        //projectile mode unit

                    }
                }
            }
            //cout<<col_test_counter<<endl;

            //thrown object collision
            float unit_object_dist2=(_unit_size+_object_size)*(_unit_size+_object_size);
            float building_object_dist2=(_unit_size+_building_size)*(_unit_size+_building_size);
            for(unsigned int object_i=0;object_i<m_vec_pObjects.size();object_i++)
            {
                if(!m_vec_pObjects[object_i]->m_projectile_mode) continue;

                st_pos object_pos=m_vec_pObjects[object_i]->get_curr_pos();

                //to unit
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                {
                    //ignore player
                    if(m_vec_pUnits[unit_i]==m_pPlayer_unit) continue;

                    if( object_pos.distance2( m_vec_pUnits[unit_i]->get_curr_pos() ) < unit_object_dist2 )
                    {
                        //remove object, or pierce
                        //m_vec_pObjects[object_i]->take_damage(0,true);

                        //remove unit
                        m_vec_pUnits[unit_i]->take_damage(0,0,true);
                    }
                }

                //to buildings
                for(unsigned int building_i=0;building_i<m_vec_pBuildings.size();building_i++)
                {
                    if( object_pos.distance2( m_vec_pBuildings[building_i]->get_curr_pos() ) < building_object_dist2 )
                    {
                        //remove object
                        m_vec_pObjects[object_i]->take_damage(0,true);

                        //damage building
                        m_vec_pBuildings[building_i]->take_damage(_object_projectile_damage);

                        //play sound
                        m_pSound->playSimpleSound(wav_hit_building,1.0);
                    }
                }
            }

            //update col_id
            for(unsigned int col_i=0;col_i<m_vec_col_id.size();col_i++)
            {
                //see if any active units using the current col_id
                bool col_id_active=false;
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                {
                    if( m_vec_pUnits[unit_i]->m_col_id==m_vec_col_id[col_i].id )
                    {
                        //same id, test if active
                        if(m_vec_pUnits[unit_i]->m_col_state==active)
                        {
                            col_id_active=true;
                            break;
                        }
                    }
                }
                if(!col_id_active)
                {
                    //no units are using this col_id

                    //make pasive units to none state
                    for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                    {
                        if( m_vec_pUnits[unit_i]->m_col_id==m_vec_col_id[col_i].id )
                        {
                            m_vec_pUnits[unit_i]->m_col_state=none;
                        }
                    }

                    //remove col_id from vector
                    m_vec_col_id[col_i]=m_vec_col_id.back();
                    m_vec_col_id.pop_back();
                    col_i--;
                }
            }

            //force pos for object in hands
            switch(m_object_in_hands)
            {
                case ho_none:
                {

                }break;

                case ho_solid:
                {
                    st_pos hand_pos=m_pPlayer_unit->get_curr_pos();
                    float player_rotation=m_pPlayer_unit->m_rotation_curr;
                    float arm_length=44-1.0*sinf(m_walk_prog*2.0*_pi);
                    float arm_angle=-62-5.0*sinf(m_walk_prog*2.0*_pi);
                    hand_pos.x+=arm_length*cosf( (player_rotation+arm_angle)*_Deg2Rad );
                    hand_pos.y+=arm_length*sinf( (player_rotation+arm_angle)*_Deg2Rad );
                    m_pObject_in_hands->force_pos(hand_pos,player_rotation+arm_angle);
                }break;

                case ho_unit:
                {
                    st_pos hand_pos=m_pPlayer_unit->get_curr_pos();
                    float player_rotation=m_pPlayer_unit->m_rotation_curr;
                    float arm_length=43-3.0*sinf(m_walk_prog*2.0*_pi);
                    float arm_angle=-50-5.0*sinf(m_walk_prog*2.0*_pi);
                    hand_pos.x+=arm_length*cosf( (player_rotation+arm_angle)*_Deg2Rad );
                    hand_pos.y+=arm_length*sinf( (player_rotation+arm_angle)*_Deg2Rad );
                    m_pUnit_in_hands->force_pos(hand_pos,player_rotation+arm_angle);
                }break;
            }

            //update buildings
            for(unsigned int building_i=0;building_i<m_vec_pBuildings.size();building_i++)
            {
                if(!m_vec_pBuildings[building_i]->update())
                {
                    //add particles
                    float color[3]={0.4,0.3,0.2};
                    float pos[2]={m_vec_pBuildings[building_i]->get_curr_pos().x,
                                  m_vec_pBuildings[building_i]->get_curr_pos().y};
                    float pos1[2]={pos[0]-_building_size*0.5,pos[1]-_building_size*0.5};
                    float pos2[2]={pos[0]-_building_size*0.5,pos[1]+_building_size*0.5};
                    float pos3[2]={pos[0]+_building_size*0.5,pos[1]+_building_size*0.5};
                    float pos4[2]={pos[0]+_building_size*0.5,pos[1]-_building_size*0.5};
                    m_pPartEng->add_explosion(pos ,50,50,2,color,rand()%360);
                    m_pPartEng->add_explosion(pos1,50,50,2,color,45);
                    m_pPartEng->add_explosion(pos2,50,50,2,color,135);
                    m_pPartEng->add_explosion(pos3,50,50,2,color,225);
                    m_pPartEng->add_explosion(pos4,50,50,2,color,315);

                    //play sound
                    m_pSound->playSimpleSound(wav_bulding_destroyed,1.0);

                    //add score
                    m_hud.add_score(10);

                    //add old building
                    m_vec_old_buildings.push_back( st_decal(st_pos(pos[0],pos[1])) );

                    //remove, no hp left
                    delete m_vec_pBuildings[building_i];
                    m_vec_pBuildings[building_i]=m_vec_pBuildings.back();
                    m_vec_pBuildings.pop_back();
                    building_i--;
                }
            }

            //update objects
            for(unsigned int object_i=0;object_i<m_vec_pObjects.size();object_i++)
            {
                if(!m_vec_pObjects[object_i]->update())
                {
                    //add particles
                    float color[3]={0.2,0.2,0.2};
                    float pos[2]={m_vec_pObjects[object_i]->get_curr_pos().x,
                                  m_vec_pObjects[object_i]->get_curr_pos().y};
                    m_pPartEng->add_explosion(pos,50,50,2,color,rand()%360);
                    st_pos rand_pos(10);
                    pos[0]+=rand_pos.x;
                    pos[1]+=rand_pos.y;
                    m_pPartEng->add_explosion(pos,50,50,2,color,rand()%360);
                    rand_pos=st_pos(10);
                    pos[0]+=rand_pos.x;
                    pos[1]+=rand_pos.y;
                    m_pPartEng->add_explosion(pos,50,50,2,color,rand()%360);

                    //play sound
                    m_pSound->playSimpleSound(wav_object_destroyed,1.0);

                    //remove, no hp left
                    delete m_vec_pObjects[object_i];
                    m_vec_pObjects[object_i]=m_vec_pObjects.back();
                    m_vec_pObjects.pop_back();
                    object_i--;
                }
            }

            //update projectiles
            for(unsigned int proj_i=0;proj_i<m_vec_projectiles.size();proj_i++)
            {
                if( m_vec_projectiles[proj_i].update() )
                {
                    //end reached, make damage
                    //calc square pos
                    int col_square_x=m_vec_projectiles[proj_i].m_pos_end.x/_col_grid_size;
                    int col_square_y=m_vec_projectiles[proj_i].m_pos_end.y/_col_grid_size;
                    //cap grid pos, if unit temp outside world limit
                    if(col_square_x<0) col_square_x=0;
                    else if(col_square_x>_world_width/_col_grid_size-1) col_square_x=_world_width/_col_grid_size-1;
                    if(col_square_y<0) col_square_y=0;
                    else if(col_square_y>_world_height/_col_grid_size-1) col_square_y=_world_height/_col_grid_size-1;

                    //calc square range
                    float range_px=m_vec_projectiles[proj_i].m_damage_range;
                    if(range_px<=0) range_px=1;//1 pix min dist
                    float range_px2=range_px*range_px;
                    int range_squares=range_px/_col_grid_size+1;
                    st_pos center_square(col_square_x,col_square_y);

                    //special range for player size
                    float range_player2=(range_px+_player_size)*(range_px+_player_size);

                    for(int square_x=(int)center_square.x-range_squares;
                        square_x<square_x_max && square_x<=(int)center_square.x+range_squares; square_x++)
                    for(int square_y=(int)center_square.y-range_squares;
                        square_y<square_y_max && square_y<=(int)center_square.y+range_squares; square_y++)
                    {
                        if(square_x<0 || square_y<0 ) continue;//outside world

                        //enemy test, turn off for friendly fire
                        /*enemy_in_square=false;
                        for(int team_i=0;team_i<_max_teams;team_i++)
                        {
                            if(team_i==m_vec_projectiles[proj_i].team_fire) continue;

                            if(m_arr_team_squares[square_x][square_y].at(team_i)!='0')
                            {
                                //cout<<" enemy found\n";
                                enemy_in_square=true;
                                break;
                            }
                        }*/

                        //if(enemy_in_square)//if not, skip this ant test other squares
                         for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                        {
                            //team test, OFF
                            //if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                            {
                                //other team found, test dist
                                float dist2=m_vec_projectiles[proj_i].m_pos_end.distance2(
                                            m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos() );
                                //special test if player
                                bool player_hit=false;
                                if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]==m_pPlayer_unit)
                                {
                                    if(range_player2>dist2) player_hit=true;
                                }
                                if(range_px2>dist2 || player_hit)
                                {
                                    //cout<<"Damage caused to unit\n";
                                    float damage=m_vec_projectiles[proj_i].m_damage;

                                    //shield test
                                    float unit_shield_angle=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().shield_angle;
                                    if(unit_shield_angle>0)
                                    {
                                        float projectile_angle=m_vec_projectiles[proj_i].m_angle;
                                        //is the projectile inside the unit, use inverted projectile angle
                                        if(_unit_size*_unit_size>dist2)
                                        {
                                            projectile_angle+=180;
                                            if(projectile_angle>180) projectile_angle-=360;
                                        }
                                        else//calc angle between the unit and the projectile
                                        {
                                            projectile_angle=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos()
                                                              .get_angle_to(m_vec_projectiles[proj_i].m_pos_end );
                                        }

                                        //shield angle test
                                        if( fabs(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_rotation_curr-projectile_angle) < unit_shield_angle )
                                        {
                                            damage*=_unit_shield_damage_reduction_factor;
                                            //cout<<"+shield angle effect\n";
                                        }
                                        //else cout<<"-not shield\n";
                                    }


                                    //within damage range, cause damage to unit
                                    m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->take_damage(damage);
                                    //cout<<"damage to unit: "<<unit_i1<<endl;

                                    //add particles
                                    float color[3]={0.6,0.2,0.2};
                                    float pos[2]={m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos().x,
                                                  m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos().y};
                                    if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]==m_pPlayer_unit)
                                    {
                                        pos[0]+=(rand()%(int)_player_size)-_player_size*0.5;
                                        pos[1]+=(rand()%(int)_player_size)-_player_size*0.5;
                                        color[0]=0.4;
                                        color[1]=0.2;
                                        color[2]=0.1;
                                    }

                                    m_pPartEng->add_explosion(pos,20,50,1,color,m_vec_projectiles[proj_i].m_angle,1);

                                    //play sound
                                    if(player_hit)
                                    {
                                        int rand_sound=0;
                                        switch(rand()%3)
                                        {
                                            case 0: rand_sound=wav_player_hurt1; break;
                                            case 1: rand_sound=wav_player_hurt2; break;
                                            case 2: rand_sound=wav_player_hurt3; break;
                                        }
                                        m_pSound->playSimpleSound(rand_sound,1.0);

                                        //trigger music
                                        m_music_fast_timer=_sound_music_fast_time;
                                        if(m_music_state==ms_on_normal || m_music_state==ms_to_normal)
                                        {
                                            m_music_state=ms_to_fast;
                                        }
                                    }
                                    else//enemy hit
                                    {
                                        int rand_sound=0;
                                        switch(rand()%3)
                                        {
                                            case 0: rand_sound=wav_enemy_hurt1; break;
                                            case 1: rand_sound=wav_enemy_hurt2; break;
                                            case 2: rand_sound=wav_enemy_hurt3; break;
                                        }
                                        m_pSound->playSimpleSound(rand_sound,1.0);
                                    }
                                }
                            }
                        }
                    }

                    /*//remove projectile
                    m_vec_projectiles.erase( m_vec_projectiles.begin()+proj_i );
                    proj_i--;*/

                    //new remove, copy last element to current and pop back
                    m_vec_projectiles[proj_i]=m_vec_projectiles.back();
                    m_vec_projectiles.pop_back();
                    proj_i--;
                }
            }

            //update footprints
            for(unsigned int foot_i=0;foot_i<m_vec_footprints.size();foot_i++)
            {
                m_vec_footprints[foot_i].time_left-=_time_step;

                //remove
                if(m_vec_footprints[foot_i].time_left<=0)
                {
                    m_vec_footprints[foot_i]=m_vec_footprints.back();
                    m_vec_footprints.pop_back();
                    foot_i--;
                }
            }

            //update particle engine
            m_pPartEng->update();

            //update hud
            m_hud.update( m_pPlayer_unit->m_spec.hp_curr/m_pPlayer_unit->m_spec.hp_max );

            //update preunits
            for(unsigned int pre_i=0;pre_i<m_vec_preunits.size();pre_i++)
            {
                //test if in sight range
                if(m_cam_pos[1]-200<m_vec_preunits[pre_i].pos.y &&
                   m_cam_pos[1]+m_window_size[1]+200>m_vec_preunits[pre_i].pos.y )
                {
                    //spawn
                    m_vec_pUnits.push_back( new unit( m_vec_preunits[pre_i].specs,
                                                      m_vec_preunits[pre_i].pos,
                                                      m_vec_preunits[pre_i].texture) );
                    int col_square_x=m_vec_preunits[pre_i].pos.x/_col_grid_size;
                    int col_square_y=m_vec_preunits[pre_i].pos.y/_col_grid_size;
                    m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits.back() );
                    m_vec_pUnits.back()->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );

                    //remove from vec
                    m_vec_preunits[pre_i]=m_vec_preunits.back();
                    m_vec_preunits.pop_back();
                    pre_i--;
                }
            }

        }break;

        case gs_gameover:
        {
            //test for input to skip
            bool restart=false;
            if(m_pKeys_translated[key_enter])
             restart=true;

            //gamepad, from one pad
            for(int player_i=0;player_i<4;player_i++)
            {
                if(m_gamepad_connected[player_i])
                {
                    if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_start)
                    {
                        restart=true;
                        break;
                    }
                }
            }
            if(restart)
            {
                //cleanup before restart
                clean_up();
                return false;
            }
        }break;
    }

    return true;
}

bool game::draw(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    //glLoadIdentity();

    switch(m_game_state)
    {
        case gs_init:
        {
            //nothing
        }break;

        case gs_menu:
        {
            //draw main screen
            glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_main);
            glColor3f(1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,0.0);
            glVertex2f(0.0,0.0);
            glTexCoord2f(0.0,1.0);
            glVertex2f(0.0,m_window_size[1]);
            glTexCoord2f(1.0,1.0);
            glVertex2f(m_window_size[0],m_window_size[1]);
            glTexCoord2f(1.0,0.0);
            glVertex2f(m_window_size[0],0.0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();
        }break;

        case gs_in_game:
        {
            //if paused
            if(m_game_paused)
            {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_info);
                glColor3f(1,1,1);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0,0.0);
                glVertex2f(0.0,0.0);
                glTexCoord2f(0.0,1.0);
                glVertex2f(0.0,m_window_size[1]);
                glTexCoord2f(1.0,1.0);
                glVertex2f(m_window_size[0],m_window_size[1]);
                glTexCoord2f(1.0,0.0);
                glVertex2f(m_window_size[0],0.0);
                glEnd();
                glDisable(GL_TEXTURE_2D);

                break;//draw nothing more
            }

            glPushMatrix();
            glTranslatef(-m_cam_pos[0],-m_cam_pos[1],0);

            //draw tiles
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_tile);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glColor3f(0.7,0.7,0.7);
            //glColor3f(1,1,1);
            glBegin(GL_QUADS);
            for(int x=0;x<_world_width/_tile_size;x++)
            for(int y=0;y<_world_height/_tile_size;y++)
            {
                glTexCoord2f(m_tex_tile_pos[ m_arr_tiles[x][y].subtype ].x,m_tex_tile_pos[ m_arr_tiles[x][y].subtype ].y);
                glVertex2f(x*_tile_size,y*_tile_size);
                glTexCoord2f(m_tex_tile_pos[ m_arr_tiles[x][y].subtype ].x,m_tex_tile_pos[ m_arr_tiles[x][y].subtype ].y+m_tex_tile_size[1]);
                glVertex2f(x*_tile_size,y*_tile_size+_tile_size);
                glTexCoord2f(m_tex_tile_pos[ m_arr_tiles[x][y].subtype ].x+m_tex_tile_size[0],m_tex_tile_pos[ m_arr_tiles[x][y].subtype ].y+m_tex_tile_size[1]);
                glVertex2f(x*_tile_size+_tile_size,y*_tile_size+_tile_size);
                glTexCoord2f(m_tex_tile_pos[ m_arr_tiles[x][y].subtype ].x+m_tex_tile_size[0],m_tex_tile_pos[ m_arr_tiles[x][y].subtype ].y);
                glVertex2f(x*_tile_size+_tile_size,y*_tile_size);
            }
            glEnd();
            glDisable(GL_TEXTURE_2D);

            //draw world edge
            /*glLineWidth(2);
            glColor3f(1,1,1);
            glBegin(GL_LINE_STRIP);
            glVertex2f(0,0);
            glVertex2f(0,_world_height);
            glVertex2f(_world_width,_world_height);
            glVertex2f(_world_width,0);
            glVertex2f(0,0);
            glEnd();*/

            /*//draw col_squares
            glLineWidth(1);
            glColor3f(1,1,1);
            glBegin(GL_LINES);
            for(int x=0;x<_world_width;x+=_col_grid_size)
            {
                glVertex2f(x,0);
                glVertex2f(x,_world_height);
            }
            glEnd();
            glBegin(GL_LINES);
            for(int y=0;y<_world_height;y+=_col_grid_size)
            {
                glVertex2f(0,y);
                glVertex2f(_world_width,y);
            }
            glEnd();*/

            //draw attack area, debug
            /*if(m_draw_attack_area_timer>0)
            {
                st_pos player_pos=m_pPlayer_unit->get_curr_pos();
                float player_rot=m_pPlayer_unit->m_rotation_curr;

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(1,1,1,m_draw_attack_area_timer);
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(player_pos.x,player_pos.y);//center
                for(int ii = 0; ii < 20; ii++)
                for(float angle=player_rot-_player_attack_angle*0.5;
                    angle<=player_rot+_player_attack_angle*0.5;angle+=10.0)
                {
                    float theta=angle*_Deg2Rad;//get the current angle
                    float x=_player_attack_range*cosf(theta);
                    float y=_player_attack_range*sinf(theta);
                    glVertex2f(x+player_pos.x,y+player_pos.y);

                    //test if last angle, force one last point
                    if(angle+10>player_rot+_player_attack_angle)
                    {
                        angle=player_rot+_player_attack_angle;
                        float theta_last=angle*_Deg2Rad;//get the current angle
                        float x_last=_player_attack_range*cosf(theta_last);
                        float y_last=_player_attack_range*sinf(theta_last);
                        glVertex2f(x_last+player_pos.x,y_last+player_pos.y);
                    }

                }
                glEnd();
                glDisable(GL_BLEND);
                m_draw_player_attack=false;
            }*/

            //draw flowers
            for(unsigned int flow_i=0;flow_i<m_vec_flowers.size();flow_i++)
            {
                float tex_x=(float)m_vec_flowers[flow_i].type*_flower_size*2.0/40.0;
                float tex_size=_flower_size*2.0/40.0;

                glPushMatrix();
                glTranslatef(m_vec_flowers[flow_i].pos.x,m_vec_flowers[flow_i].pos.y,0);
                //glRotatef(m_vec_flowers[flow_i].rotation,0,0,1);
                glColor3f(0.7,0.7,0.7);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_flower);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBegin(GL_QUADS);
                glTexCoord2f(tex_x,0.0);
                glVertex2f(-_flower_size,-_flower_size);
                glTexCoord2f(tex_x,1.0);

                glVertex2f(-_flower_size,_flower_size);
                glTexCoord2f(tex_x+tex_size,1.0);

                glVertex2f(_flower_size,_flower_size);
                glTexCoord2f(tex_x+tex_size,0.0);

                glVertex2f(_flower_size,-_flower_size);
                glEnd();
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glPopMatrix();
            }

            //draw footprints
            for(unsigned int foot_i=0;foot_i<m_vec_footprints.size();foot_i++)
            {
                glPushMatrix();
                glTranslatef(m_vec_footprints[foot_i].pos.x,m_vec_footprints[foot_i].pos.y,0);
                glRotatef(m_vec_footprints[foot_i].rotation,0,0,1);
                glColor3f(1,1,1);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_player_footprint);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0,1.0);
                glVertex2f(-_player_footprint_size,-_player_footprint_size);

                glTexCoord2f(1.0,1.0);
                glVertex2f(-_player_footprint_size,_player_footprint_size);

                glTexCoord2f(1.0,0.0);
                glVertex2f(_player_footprint_size,_player_footprint_size);

                glTexCoord2f(0.0,0.0);
                glVertex2f(_player_footprint_size,-_player_footprint_size);
                glEnd();
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glPopMatrix();
            }

            //draw dead units
            for(unsigned int dec_i=0;dec_i<m_vec_dead_units.size();dec_i++)
            {
                float tex_x=(float)m_vec_dead_units[dec_i].type*_unit_dead_size*2.0/156.0;
                float tex_size=_unit_dead_size*2.0/156.0;

                glPushMatrix();
                glTranslatef(m_vec_dead_units[dec_i].pos.x,m_vec_dead_units[dec_i].pos.y,0);
                glRotatef(m_vec_dead_units[dec_i].rotation,0,0,1);
                glColor3f(0.5,0.5,0.5);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_unit_dead);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBegin(GL_QUADS);
                glTexCoord2f(tex_x,1.0);
                glVertex2f(-_unit_dead_size,-_unit_dead_size);

                glTexCoord2f(tex_x+tex_size,1.0);
                glVertex2f(-_unit_dead_size,_unit_dead_size);

                glTexCoord2f(tex_x+tex_size,0.0);
                glVertex2f(_unit_dead_size,_unit_dead_size);

                glTexCoord2f(tex_x,0.0);
                glVertex2f(_unit_dead_size,-_unit_dead_size);
                glEnd();
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glPopMatrix();
            }

            //draw buildings
            for(unsigned int build_i=0;build_i<m_vec_pBuildings.size();build_i++)
            {
                m_vec_pBuildings[build_i]->draw();
            }

            //draw old buildings
            for(unsigned int dec_i=0;dec_i<m_vec_old_buildings.size();dec_i++)
            {
                float tex_x=184.0/276.0;
                float tex_size=92.0/276.0;

                glPushMatrix();
                glTranslatef(m_vec_old_buildings[dec_i].pos.x,m_vec_old_buildings[dec_i].pos.y,0);
                //glRotatef(m_vec_old_buildings[dec_i].rotation,0,0,1);
                glColor3f(0.9,0.9,0.9);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_building);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBegin(GL_QUADS);
                glTexCoord2f(tex_x,0.0);
                glVertex2f(-46,-46);
                glTexCoord2f(tex_x,1.0);
                glVertex2f(-46,46);
                glTexCoord2f(tex_x+tex_size,1.0);
                glVertex2f(46,46);
                glTexCoord2f(tex_x+tex_size,0.0);
                glVertex2f(46,-46);
                glEnd();
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glPopMatrix();
            }

            //draw objects
            for(unsigned int object_i=0;object_i<m_vec_pObjects.size();object_i++)
            {
                m_vec_pObjects[object_i]->draw();
            }

            //draw units
            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
            {
                m_vec_pUnits[unit_i]->draw();
            }

            //draw particles
            m_pPartEng->draw();


            //draw player
            {
                st_pos player_pos=m_pPlayer_unit->get_curr_pos();
                float rotation=m_pPlayer_unit->m_rotation_curr;
                float image_size_x=960;
                float image_size_y=640;
                float frame_size=160;
                int frame=(m_walk_prog*24.0);
                if(frame==24) frame=0;
                int row=int(float(frame)/6.0);
                int column=frame-(row*6);
                //float tex_x=column*frame_size;
                //float tex_y=row*frame_size;

                glPushMatrix();
                glTranslatef(player_pos.x,player_pos.y,0);
                glRotatef(rotation,0,0,1);
                glColor3f(1,1,1);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_monster_walk);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                //feet
                glBindTexture(GL_TEXTURE_2D,m_tex_monster_walk);

                glBegin(GL_QUADS);
                glTexCoord2f(frame_size*(column)/image_size_x,frame_size*(row)/image_size_y);
                glVertex2f(frame_size*0.5,-frame_size*0.5);
                glTexCoord2f(frame_size*(column)/image_size_x,frame_size*(row+1.0)/image_size_y);
                glVertex2f(-frame_size*0.5,-frame_size*0.5);
                glTexCoord2f(frame_size*(column+1.0)/image_size_x,frame_size*(row+1.0)/image_size_y);
                glVertex2f(-frame_size*0.5,frame_size*0.5);
                glTexCoord2f(frame_size*(column+1.0)/image_size_x,frame_size*(row)/image_size_y);
                glVertex2f(frame_size*0.5,frame_size*0.5);
                glEnd();


                //tail
                glBindTexture(GL_TEXTURE_2D,m_tex_monster_tail);
                frame=(m_tail_prog*24.0);
                if(frame==24) frame=0;
                row=int(float(frame)/6.0);
                column=frame-(row*6);
                glBegin(GL_QUADS);
                glTexCoord2f(frame_size*(column)/image_size_x,frame_size*(row)/image_size_y);
                glVertex2f(frame_size*0.5,-frame_size*0.5);
                glTexCoord2f(frame_size*(column)/image_size_x,frame_size*(row+1.0)/image_size_y);
                glVertex2f(-frame_size*0.5,-frame_size*0.5);
                glTexCoord2f(frame_size*(column+1.0)/image_size_x,frame_size*(row+1.0)/image_size_y);
                glVertex2f(-frame_size*0.5,frame_size*0.5);
                glTexCoord2f(frame_size*(column+1.0)/image_size_x,frame_size*(row)/image_size_y);
                glVertex2f(frame_size*0.5,frame_size*0.5);
                glEnd();


                //body
                if(m_pPlayer_unit->m_attack_cooldown<=0)
                {
                    glBindTexture(GL_TEXTURE_2D,m_tex_player_move);
                    frame=(m_walk_prog*24.0);
                    if(frame==24) frame=0;
                    row=int(float(frame)/6.0);
                    column=frame-(row*6);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D,m_tex_monster_attack);
                    frame=((1.0-m_pPlayer_unit->m_attack_cooldown/_player_attack_speed)*24.0);
                    if(frame==24) frame=0;
                    row=int(float(frame)/6.0);
                    column=frame-(row*6);
                }

                glBegin(GL_QUADS);
                glTexCoord2f(frame_size*(column)/image_size_x,frame_size*(row)/image_size_y);
                glVertex2f(frame_size*0.5,-frame_size*0.5);
                glTexCoord2f(frame_size*(column)/image_size_x,frame_size*(row+1.0)/image_size_y);
                glVertex2f(-frame_size*0.5,-frame_size*0.5);
                glTexCoord2f(frame_size*(column+1.0)/image_size_x,frame_size*(row+1.0)/image_size_y);
                glVertex2f(-frame_size*0.5,frame_size*0.5);
                glTexCoord2f(frame_size*(column+1.0)/image_size_x,frame_size*(row)/image_size_y);
                glVertex2f(frame_size*0.5,frame_size*0.5);
                glEnd();

                //eyes
                int eye_size=9;
                int eye_pos_x=8+4.0*sinf(m_walk_prog*2.0*_pi);
                if(m_pPlayer_unit->m_attack_cooldown>0)
                {
                    eye_pos_x=4;
                }
                int eye_pos_y=12;
                float eye_blend=0.7+0.2*sinf(m_eye_pulse*2.0*_pi);
                glColor4f(1,0,0,eye_blend);
                glBindTexture(GL_TEXTURE_2D,m_tex_eye);
                //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glBegin(GL_QUADS);
                glTexCoord2f(0,0);
                glVertex2f(-eye_size+eye_pos_x,eye_size+eye_pos_y);
                glTexCoord2f(0,1);
                glVertex2f(-eye_size+eye_pos_x,-eye_size+eye_pos_y);
                glTexCoord2f(1,1);
                glVertex2f(eye_size+eye_pos_x,-eye_size+eye_pos_y);
                glTexCoord2f(1,0);
                glVertex2f(eye_size+eye_pos_x,eye_size+eye_pos_y);
                glEnd();
                //eye_pos_x=8+4.0*sinf(m_walk_prog*2.0*_pi);
                eye_pos_y=10;
                glBegin(GL_QUADS);
                glTexCoord2f(0,0);
                glVertex2f(-eye_size+eye_pos_x,eye_size-eye_pos_y);
                glTexCoord2f(0,1);
                glVertex2f(-eye_size+eye_pos_x,-eye_size-eye_pos_y);
                glTexCoord2f(1,1);
                glVertex2f(eye_size+eye_pos_x,-eye_size-eye_pos_y);
                glTexCoord2f(1,0);
                glVertex2f(eye_size+eye_pos_x,eye_size-eye_pos_y);
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glPopMatrix();
            }

            //draw projectiles
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_arrow);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //glColor3f(1,1,1);
            //glPointSize(2);
            //glBegin(GL_QUADS);
            //glBegin(GL_POINTS);
            for(unsigned int proj_i=0;proj_i<m_vec_projectiles.size();proj_i++)
            {
                m_vec_projectiles[proj_i].draw();
            }
            //glEnd();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            //draw particles, layer ontop of player
            m_pPartEng->draw(1);

            glPopMatrix();

            //draw HUD
            m_hud.draw();

            //draw eye
            if(m_eye_open_prog>0)
            {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_main);
                glColor3f(0,0,0);
                glBegin(GL_QUADS);
                glTexCoord2f( 0.0 ,0.0 );
                glVertex2f(0,0);
                glTexCoord2f( 0.0 ,0.5 );
                glVertex2f(0,m_window_size[1]*0.5*(m_eye_open_prog/m_eye_open_time));
                glTexCoord2f( 1.0 ,1.0 );
                glVertex2f(m_window_size[0],m_window_size[1]*0.5*(m_eye_open_prog/m_eye_open_time));
                glTexCoord2f( 1.0 ,0.0 );
                glVertex2f(m_window_size[0],0);

                glTexCoord2f( 0.0 ,0.0 );
                glVertex2f(0,m_window_size[1]*0.5+m_window_size[1]*0.5*(1.0-m_eye_open_prog/m_eye_open_time));
                glTexCoord2f( 0.0 ,1.0 );
                glVertex2f(0,m_window_size[1]);
                glTexCoord2f( 1.0 ,1.0 );
                glVertex2f(m_window_size[0],m_window_size[1]);
                glTexCoord2f( 1.0 ,0.0 );
                glVertex2f(m_window_size[0],m_window_size[1]*0.5+m_window_size[1]*0.5*(1.0-m_eye_open_prog/m_eye_open_time));
                glEnd();
                glDisable(GL_TEXTURE_2D);
            }

            //draw mask
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_mask);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor3f(1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f( 0.0 ,0.0 );
            glVertex2f(0,0);
            glTexCoord2f( 0.0 ,1.0 );
            glVertex2f(0,m_window_size[1]);
            glTexCoord2f( 1.0 ,1.0 );
            glVertex2f(m_window_size[0],m_window_size[1]);
            glTexCoord2f( 1.0 ,0.0 );
            glVertex2f(m_window_size[0],0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

        }break;

        case gs_gameover:
        {
            //draw main screen
            glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_gameover);
            glColor3f(1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,0.0);
            glVertex2f(0.0,0.0);
            glTexCoord2f(0.0,1.0);
            glVertex2f(0.0,m_window_size[1]);
            glTexCoord2f(1.0,1.0);
            glVertex2f(m_window_size[0],m_window_size[1]);
            glTexCoord2f(1.0,0.0);
            glVertex2f(m_window_size[0],0.0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            //draw win/lost text and score
            m_hud.draw();
        }break;
    }

    return true;
}


//Private

bool game::load_textures(void)
{
    cout<<"Game: Loading texture\n";

    m_tex_main=SOIL_load_OGL_texture
    (
        "data\\texture\\main.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_info=SOIL_load_OGL_texture
    (
        "data\\texture\\info.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_player_move=SOIL_load_OGL_texture
    (
        "data\\texture\\monsterWalkAnim.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_player_footprint=SOIL_load_OGL_texture
    (
        "data\\texture\\footprintNoOut.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_unit_move=SOIL_load_OGL_texture
    (
        "data\\texture\\villagerWalkAnim.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_hud=SOIL_load_OGL_texture
    (
        "data\\texture\\hud.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    //font
    m_tex_font[1]=SOIL_load_OGL_texture
    (
        "data\\texture\\fonts\\default_dark.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
    );
    m_tex_font[0]=SOIL_load_OGL_texture
    (
        "data\\texture\\fonts\\default_light.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
    );
    m_tex_font[2]=SOIL_load_OGL_texture
    (
        "data\\texture\\fonts\\default_mask.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_tile=SOIL_load_OGL_texture
    (
        "data\\texture\\tiles.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_rock=SOIL_load_OGL_texture
    (
        "data\\texture\\rock.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_building=SOIL_load_OGL_texture
    (
        "data\\texture\\house.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_gameover=SOIL_load_OGL_texture
    (
        "data\\texture\\end.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_knight=SOIL_load_OGL_texture
    (
        "data\\texture\\knightV.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_mask=SOIL_load_OGL_texture
    (
        "data\\texture\\mask.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_bowman=SOIL_load_OGL_texture
    (
        "data\\texture\\bowmanV.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_arrow=SOIL_load_OGL_texture
    (
        "data\\texture\\arrow.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_monster_attack=SOIL_load_OGL_texture
    (
        "data\\texture\\monsterAttackAnim.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_monster_walk=SOIL_load_OGL_texture
    (
        "data\\texture\\monsterFeetAnim.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_monster_tail=SOIL_load_OGL_texture
    (
        "data\\texture\\monsterTailAnim.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_eye=SOIL_load_OGL_texture
    (
        "data\\texture\\eye.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_unit_dead=SOIL_load_OGL_texture
    (
        "data\\texture\\deadDudes.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_hud_health=SOIL_load_OGL_texture
    (
        "data\\texture\\healthBar.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );

    m_tex_flower=SOIL_load_OGL_texture
    (
        "data\\texture\\flower.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT
    );



    if(m_tex_main==0||m_tex_info==0||m_tex_player_move==0||m_tex_player_footprint==0||
       m_tex_unit_move==0||m_tex_hud==0||m_tex_tile==0||m_tex_rock==0||m_tex_gameover==0||m_tex_knight==0||
       m_tex_mask==0||m_tex_bowman==0||m_tex_arrow==0||m_tex_eye==0||m_tex_unit_dead==0||m_tex_flower==0||
       m_tex_monster_attack==0||m_tex_monster_walk==0||m_tex_monster_tail==0||m_tex_hud_health==0||
       m_tex_font[0]==0||m_tex_font[1]==0||m_tex_font[2]==0)
    {
        cout<<"ERROR: Could not load texture\n";
        return false;
    }

    return true;
}

bool game::load_sounds(void)
{
    cout<<"Game: Loading sound\n";

    m_pSound=new sound();

    bool error_flag=false;

    if( !m_pSound->load_WAVE_from_file( wav_beep1,"data\\sound\\beep1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_bulding_destroyed,"data\\sound\\building_destroyed.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_object_destroyed,"data\\sound\\object_destroyed.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_eat,"data\\sound\\eat.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_enemy_hurt1,"data\\sound\\enemy_hurt1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_enemy_hurt2,"data\\sound\\enemy_hurt2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_enemy_hurt3,"data\\sound\\enemy_hurt3.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_enemy_shout1,"data\\sound\\enemy_shout1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_enemy_shout2,"data\\sound\\enemy_shout2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_enemy_shout3,"data\\sound\\enemy_shout3.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_villager_shout1,"data\\sound\\villager_shout1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_villager_shout2,"data\\sound\\villager_shout2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_villager_shout3,"data\\sound\\villager_shout3.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_hit_building,"data\\sound\\hit_building.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_hit_enemy,"data\\sound\\hit_enemy.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_hit_miss,"data\\sound\\hit_miss.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_roar1,"data\\sound\\roar1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_roar2,"data\\sound\\roar2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_roar3,"data\\sound\\roar3.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_step1,"data\\sound\\step1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_step2,"data\\sound\\step2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_step3,"data\\sound\\step3.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_throw,"data\\sound\\throw.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_unit_death1,"data\\sound\\unit_death1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_unit_death2,"data\\sound\\unit_death2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_unit_death3,"data\\sound\\unit_death3.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_hurt1,"data\\sound\\player_hurt1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_hurt2,"data\\sound\\player_hurt2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_hurt3,"data\\sound\\player_hurt3.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_death1,"data\\sound\\player_death1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_death2,"data\\sound\\player_death2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_death3,"data\\sound\\player_death3.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_swordhit,"data\\sound\\swordhit.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_bowfire,"data\\sound\\bowfire.wav" ) ) error_flag=true;

    if( !m_pSound->load_OGG_from_file( ogg_music_normal,"data\\sound\\music_normal.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music_fast,"data\\sound\\music_fast.ogg" ) ) error_flag=true;

    if(error_flag)
    {
        cout<<"ERROR: Could not load sound\n";
        return false;
    }

    return true;
}

bool game::start_music(void)
{
    //start music
    m_music_state=ms_on_normal;
    m_music_vol_normal=1.0;
    m_music_vol_fast=0.0;
    m_pSound->playSimpleSound(ogg_music_normal,1.0,_sound_chan_music_normal,true);
    m_pSound->playSimpleSound(ogg_music_fast,0.0,_sound_chan_music_fast,true);

    return true;
}

bool game::clean_up(void)
{
    cout<<"Game: Clean up\n";
    m_have_reset=true;

    for(unsigned int i=0;i<m_vec_pUnits.size();i++)
    {
        delete m_vec_pUnits[i];
    }
    m_vec_pUnits.clear();

    for(unsigned int i=0;i<m_vec_pBuildings.size();i++)
    {
        delete m_vec_pBuildings[i];
    }
    m_vec_pBuildings.clear();

    for(unsigned int i=0;i<m_vec_pObjects.size();i++)
    {
        delete m_vec_pObjects[i];
    }
    m_vec_pObjects.clear();

    m_vec_projectiles.clear();
    m_vec_footprints.clear();
    m_vec_preunits.clear();
    m_vec_dead_units.clear();
    m_vec_old_buildings.clear();
    m_vec_flowers.clear();

    for(int x=0;x<_world_width/_col_grid_size;x++)
    for(int y=0;y<_world_height/_col_grid_size;y++)
    {
        m_arr_col_squares[x][y].vec_pUnits.clear();
        m_arr_team_squares[x][y]=string(_max_teams,'0');
    }

    delete m_pPartEng;



    //stop music
    m_pSound->stopSound(_sound_chan_music_normal);
    m_pSound->stopSound(_sound_chan_music_fast);

    cout<<"Game: Reset\n";

    return true;
}

bool game::load_level(void)
{
    //load tile data from file
    /*ifstream file("data\\level.txt");
    if(file==0)
    {
        cout<<"ERROR: Could not load level data\n";
        return false;
    }*/

    string line,word;
    int line_counter=0;
    bool too_much_input=false;
    /*while( getline(file,line) )
    {
        if((int)line.length()>_world_height/_tile_size)
        {
            too_much_input=true;
            break;
        }

        for(unsigned int c=0;c<line.length();c++)
        {
            switch(line[c])
            {
                case '0'://grass
                {
                    m_arr_tiles[c][line_counter].type=tt_grass;
                }break;

                case '1'://path
                {
                    m_arr_tiles[c][line_counter].type=tt_path;
                }break;

                case '2':
                {

                }break;

                case '3':
                {

                }break;

                case '4':
                {

                }break;
            }
        }

        line_counter++;
        if( line_counter>_world_height/_tile_size )
        {
            too_much_input=true;
            break;
        }
    }
    if(too_much_input)
    {
        cout<<"WARNING: Too many rows/lines in level file\n";
    }
    file.close();*/

    //read subtype tile
    /*ifstream file2("data\\subtype.txt");
    if(file2==0)
    {
        cout<<"ERROR: Could not load level data\n";
        return false;
    }*/
    string subtype_string=string(
"05666666666666666666666666666666666666666666666700\n"
"05666666666666666666666666666666666666666666666700\n"
"05666666666666666666666666666666666666666666666700\n"
"05666666666666666666666666666666666666666666666700\n"
"05666666666666666666666666666666666666666666666700\n"
"05666666666666666666666666666666666666666666666700\n"
"05666666666666666666666666666666666666666666666700\n"
"05666666666666666666666666666666666666666666666700\n"
"05666666666666666666666666666666666666666666666700\n"
"08999999999999999999999669999999999999999999999a00\n"
"00000000000000000000000233400000000000000000000000\n"
"00000000000000000000000566700000000000000000000000\n"
"00000000000000000000000566700000000000000000000000\n"
"00000000000000000000000899a00000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000010000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000010000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000010000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000233333333333333333333333333340000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"00000000000566666666666666666666666666670000000000\n"
"000000000008999999999999999999999999999a0000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"000000000000000000000000d0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"0000000000000000000000efhfg00000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000c0000000000000000000000000\n"
"00000000000002333333333333333333333400000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000005666666666666666666666700000000000000\n"
"00000000000008999999999999999999999a00000000000000\n"
"000000000000000000000000d0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000c0000000000000000000000000\n"
"00000000000000000023333333334000000000000000000000\n"
"00000000000000000056666666667000000000000000000000\n"
"00000000000000000056666666667000000000000000000000\n"
"00000000000000000056666666667effffg000000000000000\n"
"00000000000000000056666666667000000000000000000000\n"
"00000000000000000056666666667000000000000000000000\n"
"00000000000000effg56666666667000000000000000000000\n"
"00000000000000000056666666667000000000000000000000\n"
"0000000000000000008999999999a000000000000000000000\n"
"000000000000000000000000d0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000c0000000000000000000000000\n"
"00000000000000000000023333400000000000000000000000\n"
"00000000000000000000056666700000000000000000000000\n"
"00000000000000000000056666700000000000000000000000\n"
"00000000000000000000056666700000000000000000000000\n"
"00000000000000000000089999a00000000000000000000000\n"
"000000000000000000000000d0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000b0000000000000000000000000\n"
"000000000000000000000000c0000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000001000000000000000100000000000000000\n"
"00000000000000000000000001000000000000000000000000\n"
"00000000100000000000000000000000000000000000000000\n"
"00000000000000000000000000000000000000100000000000\n"
"00000010000000000100000000000100000000000100000000\n"
"00000000000000000000000000000000000000000000000000\n"
"00000000000000100000001000000000000100000000000000\n"
"33333333333333333333333333333333333333333333333333\n"
"66666666666666666666666666666666666666666666666666\n"
"66666666666666666666666666666666666666666666666666\n"
                                 );
    stringstream ss_file2(subtype_string);
    too_much_input=false;
    line_counter=0;
    while( getline(ss_file2,line) )
    {
        if((int)line.length()>_world_height/_tile_size)
        {
            too_much_input=true;
            break;
        }

        for(unsigned int c=0;c<line.length();c++)
        {
            switch(line[c])
            {
                case '0':m_arr_tiles[c][line_counter].subtype=0;break;
                case '1':m_arr_tiles[c][line_counter].subtype=1;break;
                case '2':m_arr_tiles[c][line_counter].subtype=2;break;
                case '3':m_arr_tiles[c][line_counter].subtype=3;break;
                case '4':m_arr_tiles[c][line_counter].subtype=4;break;
                case '5':m_arr_tiles[c][line_counter].subtype=5;break;
                case '6':m_arr_tiles[c][line_counter].subtype=6;break;
                case '7':m_arr_tiles[c][line_counter].subtype=7;break;
                case '8':m_arr_tiles[c][line_counter].subtype=8;break;
                case '9':m_arr_tiles[c][line_counter].subtype=9;break;
                case 'a':m_arr_tiles[c][line_counter].subtype=10;break;
                case 'b':m_arr_tiles[c][line_counter].subtype=11;break;
                case 'c':m_arr_tiles[c][line_counter].subtype=12;break;
                case 'd':m_arr_tiles[c][line_counter].subtype=13;break;
                case 'e':m_arr_tiles[c][line_counter].subtype=14;break;
                case 'f':m_arr_tiles[c][line_counter].subtype=15;break;
                case 'g':m_arr_tiles[c][line_counter].subtype=16;break;
                case 'h':m_arr_tiles[c][line_counter].subtype=17;break;
            }
        }

        line_counter++;
        if( line_counter>_world_height/_tile_size )
        {
            too_much_input=true;
            break;
        }
    }
    if(too_much_input)
    {
        cout<<"WARNING: Too many rows/lines in level file\n";
    }

    //read list of objects to place
    /*ifstream file3("data\\list.txt");
    if(file3==0)
    {
        cout<<"ERROR: Could not load level data\n";
        return false;
    }*/
    string string_file3=string(
"s 650 12780\n"
"s 1000 12780\n"
"s 810 12770\n"
"s 930 12770\n"
"s 1160 12770\n"
"s 1240 12770\n"
"s 60 12760\n"
"s 490 12760\n"
"s 1090 12760\n"
"s 1300 12760\n"
"s 1380 12760\n"
"s 1420 12760\n"
"s 1550 12760\n"
"s 1620 12760\n"
"s 1680 12760\n"
"s 2350 12760\n"
"s 210 12750\n"
"s 280 12750\n"
"s 1460 12750\n"
"s 1510 12750\n"
"s 1750 12750\n"
"s 1810 12750\n"
"s 1900 12750\n"
"s 1960 12750\n"
"s 3020 12750\n"
"s 2070 12740\n"
"s 2710 12740\n"
"s 2840 12740\n"
"s 1020 12730\n"
"s 1600 12730\n"
"s 2220 12730\n"
"s 1290 12720\n"
"s 1680 12720\n"
"s 1360 12710\n"
"s 160 12700\n"
"s 1170 12700\n"
"s 1480 12700\n"
"s 1540 12700\n"
"s 1860 12700\n"
"s 1990 12700\n"
"s 2570 12700\n"
"s 820 12690\n"
"s 1610 12690\n"
"s 440 12680\n"
"s 620 12680\n"
"s 1440 12680\n"
"s 1740 12680\n"
"s 3100 12680\n"
"s 1210 12670\n"
"s 1300 12670\n"
"s 1380 12670\n"
"s 2100 12670\n"
"s 2920 12670\n"
"s 220 12660\n"
"s 1910 12660\n"
"s 2300 12660\n"
"s 1680 12650\n"
"s 1840 12650\n"
"s 120 12640\n"
"s 2440 12630\n"
"s 310 12610\n"
"s 540 12610\n"
"s 1120 12600\n"
"s 1610 12600\n"
"s 980 12590\n"
"s 1480 12590\n"
"s 1880 12590\n"
"s 2700 12590\n"
"s 1310 12570\n"
"s 780 12560\n"
"s 3050 12560\n"
"s 210 12550\n"
"s 1740 12550\n"
"s 2130 12520\n"
"s 40 12510\n"
"s 140 12440\n"
"s 2930 12440\n"
"s 2590 12370\n"
"s 440 12360\n"
"s 1280 12350\n"
"s 3090 12350\n"
"s 80 12310\n"
"s 1690 12270\n"
"s 40 12170\n"
"s 2070 12160\n"
"s 3150 12160\n"
"v 650 12140\n"
"v 2780 12140\n"
"s 970 12100\n"
"v 400 11990\n"
"v 2810 11900\n"
"s 2280 11830\n"
"v 1320 11800\n"
"v 1660 11800\n"
"s 710 11790\n"
"v 1700 11780\n"
"v 1420 11560\n"
"s 1580 11540\n"
"v 1320 11530\n"
"b 1430 11480\n"
"v 1360 11450\n"
"v 1670 11450\n"
"v 1530 11440\n"
"v 1580 11440\n"
"s 1220 11410\n"
"b 1650 11380\n"
"s 190 11370\n"
"v 1400 11280\n"
"s 1770 11280\n"
"b 2460 11240\n"
"s 1540 11180\n"
"s 1390 11170\n"
"b 710 11040\n"
"s 1190 10810\n"
"s 650 10560\n"
"s 2170 10510\n"
"s 1590 10270\n"
"v 1440 10200\n"
"v 1590 10200\n"
"v 1510 10190\n"
"b 1350 10180\n"
"v 1640 10170\n"
"v 1440 10130\n"
"v 1480 10130\n"
"v 1630 10120\n"
"s 900 10110\n"
"a 1080 10110\n"
"s 2130 10110\n"
"s 1290 10100\n"
"v 1890 10090\n"
"b 1680 10080\n"
"s 2420 10080\n"
"a 560 10060\n"
"b 1510 10050\n"
"a 2590 10050\n"
"a 1380 10040\n"
"a 200 10030\n"
"a 790 10010\n"
"a 1270 10010\n"
"v 1720 10010\n"
"a 1810 10000\n"
"v 1040 9990\n"
"a 1650 9990\n"
"a 2910 9970\n"
"a 2010 9960\n"
"b 1390 9950\n"
"v 1670 9940\n"
"v 1280 9930\n"
"b 1730 9930\n"
"a 2220 9930\n"
"s 850 9900\n"
"v 1310 9880\n"
"b 1220 9870\n"
"a 1660 9870\n"
"a 1750 9860\n"
"a 1430 9850\n"
"s 2570 9830\n"
"b 1690 9770\n"
"s 1350 9750\n"
"b 1500 9750\n"
"s 1560 9670\n"
"a 1410 9630\n"
"a 1620 9610\n"
"s 1060 9420\n"
"s 1900 9250\n"
"s 2650 9190\n"
"s 840 9020\n"
"s 2150 8880\n"
"v 1530 8760\n"
"v 1590 8590\n"
"s 1680 8430\n"
"s 1900 8410\n"
"s 1430 8390\n"
"v 1510 8390\n"
"s 1650 8350\n"
"s 1380 8340\n"
"s 1470 8310\n"
"s 1640 8310\n"
"s 2130 8310\n"
"s 2030 8300\n"
"s 960 8290\n"
"s 1090 8290\n"
"s 1880 8290\n"
"s 1200 8280\n"
"s 1290 8280\n"
"s 1010 8270\n"
"s 1150 8270\n"
"s 1400 8270\n"
"v 1580 8270\n"
"s 1700 8270\n"
"s 1970 8270\n"
"s 2210 8270\n"
"s 480 8260\n"
"s 1830 8260\n"
"s 1330 8250\n"
"s 1450 8250\n"
"s 1650 8240\n"
"s 2260 8240\n"
"s 960 8230\n"
"s 930 8200\n"
"s 2320 8190\n"
"a 1020 8160\n"
"k 1160 8160\n"
"s 2260 8160\n"
"v 1600 8150\n"
"s 940 8140\n"
"k 1330 8140\n"
"k 1950 8140\n"
"k 1370 8130\n"
"k 1410 8130\n"
"s 790 8100\n"
"s 2250 8100\n"
"a 1060 8080\n"
"a 1560 8080\n"
"a 1580 8080\n"
"a 1610 8080\n"
"a 1640 8080\n"
"s 860 8070\n"
"k 1710 8070\n"
"k 1750 8070\n"
"k 1800 8070\n"
"a 2180 8070\n"
"s 940 8050\n"
"b 1230 8050\n"
"b 1340 8050\n"
"b 1460 8050\n"
"a 2040 8050\n"
"a 2120 8050\n"
"s 2390 8040\n"
"s 910 8030\n"
"s 2280 8020\n"
"a 2180 7990\n"
"b 1700 7980\n"
"b 1810 7980\n"
"b 1930 7980\n"
"a 2060 7980\n"
"k 2960 7980\n"
"s 910 7970\n"
"k 1610 7970\n"
"k 1000 7960\n"
"k 190 7950\n"
"k 410 7950\n"
"k 740 7950\n"
"k 2640 7950\n"
"s 2230 7940\n"
"k 2390 7940\n"
"b 1230 7930\n"
"b 1340 7930\n"
"b 1460 7930\n"
"k 2130 7930\n"
"a 1080 7910\n"
"s 840 7900\n"
"s 930 7900\n"
"s 2240 7890\n"
"a 1600 7880\n"
"s 2240 7880\n"
"b 1700 7860\n"
"b 1810 7860\n"
"b 1930 7860\n"
"s 900 7850\n"
"a 1370 7830\n"
"a 1600 7810\n"
"a 1210 7800\n"
"b 1520 7800\n"
"s 890 7790\n"
"a 1660 7790\n"
"a 1760 7770\n"
"a 1870 7770\n"
"s 2250 7770\n"
"k 1300 7760\n"
"a 1810 7760\n"
"s 2300 7750\n"
"s 700 7740\n"
"k 1460 7740\n"
"s 930 7730\n"
"k 1710 7730\n"
"k 1890 7720\n"
"s 2240 7720\n"
"a 1590 7700\n"
"s 920 7680\n"
"k 1650 7670\n"
"s 2130 7670\n"
"s 990 7660\n"
"a 1490 7640\n"
"s 2170 7640\n"
"s 1080 7630\n"
"s 910 7620\n"
"s 1010 7590\n"
"s 1200 7580\n"
"s 2110 7580\n"
"s 2200 7570\n"
"s 1940 7560\n"
"s 2020 7560\n"
"s 950 7550\n"
"s 1150 7550\n"
"s 1250 7550\n"
"s 1360 7550\n"
"s 2660 7550\n"
"s 1270 7540\n"
"s 2180 7540\n"
"s 1420 7530\n"
"s 1520 7530\n"
"s 1080 7520\n"
"s 1650 7520\n"
"s 1690 7520\n"
"s 1770 7520\n"
"s 1850 7520\n"
"s 1910 7520\n"
"s 2060 7520\n"
"s 1460 7510\n"
"s 2000 7510\n"
"s 2110 7510\n"
"s 1720 7500\n"
"s 1670 7490\n"
"s 1440 7470\n"
"s 1490 7470\n"
"s 1630 7460\n"
"s 1770 7460\n"
"s 1690 7450\n"
"s 1900 7450\n"
"s 510 7420\n"
"s 1500 7420\n"
"s 930 7390\n"
"s 1780 7350\n"
"s 2310 7260\n"
"a 270 7240\n"
"a 2880 7190\n"
"s 2620 7130\n"
"s 1250 7120\n"
"a 560 7070\n"
"s 1950 7030\n"
"a 2640 6950\n"
"a 940 6930\n"
"s 2890 6860\n"
"a 2520 6790\n"
"s 2660 6620\n"
"k 1240 6350\n"
"k 1270 6350\n"
"k 1300 6350\n"
"k 1340 6350\n"
"k 1370 6350\n"
"k 1400 6350\n"
"k 1440 6350\n"
"k 1470 6350\n"
"k 1740 6350\n"
"k 1770 6350\n"
"k 1800 6350\n"
"k 1840 6350\n"
"k 1870 6350\n"
"k 1900 6350\n"
"k 1940 6350\n"
"k 1970 6350\n"
"k 1240 6310\n"
"k 1270 6310\n"
"k 1300 6310\n"
"k 1340 6310\n"
"k 1370 6310\n"
"k 1400 6310\n"
"k 1440 6310\n"
"k 1470 6310\n"
"k 1740 6310\n"
"k 1770 6310\n"
"k 1800 6310\n"
"k 1840 6310\n"
"k 1870 6310\n"
"k 1900 6310\n"
"k 1940 6310\n"
"k 1970 6310\n"
"k 1030 6290\n"
"k 1060 6290\n"
"k 1090 6290\n"
"k 1130 6290\n"
"k 2070 6290\n"
"k 2100 6290\n"
"k 2130 6290\n"
"k 2170 6290\n"
"k 1240 6270\n"
"k 1270 6270\n"
"k 1300 6270\n"
"k 1340 6270\n"
"k 1370 6270\n"
"k 1400 6270\n"
"k 1440 6270\n"
"k 1470 6270\n"
"k 1740 6270\n"
"k 1770 6270\n"
"k 1800 6270\n"
"k 1840 6270\n"
"k 1870 6270\n"
"k 1900 6270\n"
"k 1940 6270\n"
"k 1970 6270\n"
"a 3110 6260\n"
"k 1030 6250\n"
"k 1060 6250\n"
"k 1090 6250\n"
"k 1130 6250\n"
"k 2070 6250\n"
"k 2100 6250\n"
"k 2130 6250\n"
"k 2170 6250\n"
"a 2960 6240\n"
"a 700 6230\n"
"a 890 6230\n"
"a 2740 6230\n"
"a 350 6220\n"
"a 1270 6220\n"
"a 1300 6220\n"
"a 1330 6220\n"
"a 1360 6220\n"
"a 1390 6220\n"
"a 1420 6220\n"
"a 1770 6220\n"
"a 1800 6220\n"
"a 1830 6220\n"
"a 1860 6220\n"
"a 1890 6220\n"
"a 1920 6220\n"
"a 2330 6220\n"
"a 210 6210\n"
"k 1030 6210\n"
"k 1060 6210\n"
"k 1090 6210\n"
"k 1130 6210\n"
"k 1550 6210\n"
"k 1580 6210\n"
"k 1620 6210\n"
"k 1650 6210\n"
"k 2070 6210\n"
"k 2100 6210\n"
"k 2130 6210\n"
"k 2170 6210\n"
"a 2540 6210\n"
"a 510 6200\n"
"a 1270 6180\n"
"a 1300 6180\n"
"a 1330 6180\n"
"a 1360 6180\n"
"a 1390 6180\n"
"a 1420 6180\n"
"a 1770 6180\n"
"a 1800 6180\n"
"a 1830 6180\n"
"a 1860 6180\n"
"a 1890 6180\n"
"a 1920 6180\n"
"k 1550 6170\n"
"k 1580 6170\n"
"k 1620 6170\n"
"k 1650 6170\n"
"k 1550 6130\n"
"k 1580 6130\n"
"k 1620 6130\n"
"k 1650 6130\n"
"v 1510 5700\n"
"s 2640 5700\n"
"s 720 5540\n"
"s 1110 5500\n"
"s 2240 5490\n"
"v 1720 5440\n"
"v 1650 5210\n"
"s 1790 5180\n"
"v 1530 5170\n"
"s 2510 5140\n"
"s 1470 5130\n"
"v 1640 5080\n"
"s 600 5040\n"
"v 1680 5030\n"
"v 1590 4990\n"
"s 1430 4980\n"
"v 1520 4970\n"
"s 1830 4960\n"
"v 1700 4900\n"
"v 1450 4880\n"
"v 1590 4870\n"
"s 1840 4850\n"
"s 1350 4800\n"
"v 1730 4740\n"
"v 1530 4730\n"
"v 1460 4700\n"
"a 1820 4650\n"
"k 2140 4650\n"
"a 1260 4610\n"
"a 1610 4580\n"
"a 950 4570\n"
"b 1750 4560\n"
"v 2070 4550\n"
"a 2510 4540\n"
"k 1360 4530\n"
"b 1480 4530\n"
"b 1170 4510\n"
"v 2340 4510\n"
"v 700 4500\n"
"k 790 4500\n"
"v 410 4480\n"
"a 1910 4440\n"
"a 1310 4420\n"
"v 1620 4410\n"
"v 2220 4410\n"
"v 40 4400\n"
"k 2380 4400\n"
"b 1770 4380\n"
"v 650 4370\n"
"v 940 4370\n"
"b 2030 4350\n"
"v 2540 4350\n"
"v 1100 4340\n"
"b 1460 4340\n"
"a 360 4320\n"
"b 1300 4300\n"
"a 1170 4290\n"
"a 980 4270\n"
"a 1360 4270\n"
"v 1580 4270\n"
"a 560 4250\n"
"b 780 4250\n"
"a 2180 4250\n"
"k 1740 4240\n"
"b 2340 4230\n"
"a 2500 4220\n"
"a 1980 4210\n"
"b 2090 4180\n"
"v 2750 4180\n"
"a 1560 4160\n"
"b 1690 4160\n"
"b 1090 4150\n"
"b 1250 4150\n"
"b 1460 4150\n"
"b 1870 4150\n"
"v 520 4140\n"
"a 1200 4130\n"
"v 260 4120\n"
"k 850 4120\n"
"v 960 4120\n"
"a 1780 4120\n"
"v 730 4100\n"
"a 1320 4100\n"
"v 1560 4080\n"
"a 2260 4080\n"
"k 2410 4080\n"
"a 2020 4050\n"
"v 2960 4040\n"
"v 2180 4030\n"
"a 630 4010\n"
"b 2520 4010\n"
"k 1230 4000\n"
"k 1820 4000\n"
"b 890 3980\n"
"b 1120 3980\n"
"b 1450 3980\n"
"b 1310 3970\n"
"b 1940 3970\n"
"a 2720 3970\n"
"b 1700 3940\n"
"a 1150 3920\n"
"a 2350 3920\n"
"v 510 3910\n"
"v 1560 3910\n"
"k 2040 3900\n"
"b 2210 3900\n"
"k 980 3880\n"
"a 1410 3880\n"
"v 830 3850\n"
"k 2510 3840\n"
"a 1790 3820\n"
"v 2700 3820\n"
"v 1640 3810\n"
"b 1450 3800\n"
"b 1250 3790\n"
"a 2260 3780\n"
"v 1970 3760\n"
"b 1770 3750\n"
"b 1030 3730\n"
"a 760 3720\n"
"k 2070 3680\n"
"a 1370 3670\n"
"v 2440 3670\n"
"b 2170 3660\n"
"v 1570 3610\n"
"b 1360 3580\n"
"v 1900 3580\n"
"v 780 3560\n"
"b 1710 3560\n"
"k 1440 3470\n"
"v 2070 3460\n"
"s 1630 3290\n"
"s 2260 3060\n"
"s 2810 2740\n"
"s 700 2710\n"
"s 1500 2600\n"
"s 2230 2380\n"
"s 1190 2240\n"
"a 1070 1690\n"
"a 1100 1690\n"
"a 1140 1690\n"
"a 1180 1690\n"
"a 1210 1690\n"
"a 1250 1690\n"
"a 1280 1690\n"
"a 1320 1690\n"
"a 1350 1690\n"
"a 1390 1690\n"
"a 1430 1690\n"
"a 1460 1690\n"
"a 1500 1690\n"
"a 1530 1690\n"
"a 1570 1690\n"
"a 1600 1690\n"
"a 1640 1690\n"
"a 1680 1690\n"
"a 1710 1690\n"
"a 1750 1690\n"
"a 1780 1690\n"
"a 1820 1690\n"
"a 1850 1690\n"
"a 1890 1690\n"
"a 1920 1690\n"
"a 1960 1690\n"
"a 2000 1690\n"
"a 540 1680\n"
"a 570 1680\n"
"a 610 1680\n"
"a 650 1680\n"
"a 680 1680\n"
"a 720 1680\n"
"a 750 1680\n"
"a 2300 1680\n"
"a 2330 1680\n"
"a 2370 1680\n"
"a 2410 1680\n"
"a 2440 1680\n"
"a 2480 1680\n"
"a 2510 1680\n"
"a 1070 1660\n"
"a 1100 1660\n"
"a 1140 1660\n"
"a 1180 1660\n"
"a 1210 1660\n"
"a 1250 1660\n"
"a 1280 1660\n"
"a 1320 1660\n"
"a 1350 1660\n"
"a 1390 1660\n"
"a 1430 1660\n"
"a 1460 1660\n"
"a 1500 1660\n"
"a 1530 1660\n"
"a 1570 1660\n"
"a 1600 1660\n"
"a 1640 1660\n"
"a 1680 1660\n"
"a 1710 1660\n"
"a 1750 1660\n"
"a 1780 1660\n"
"a 1820 1660\n"
"a 1850 1660\n"
"a 1890 1660\n"
"a 1920 1660\n"
"a 1960 1660\n"
"a 2000 1660\n"
"a 540 1650\n"
"a 570 1650\n"
"a 610 1650\n"
"a 650 1650\n"
"a 680 1650\n"
"a 720 1650\n"
"a 750 1650\n"
"a 2300 1650\n"
"a 2330 1650\n"
"a 2370 1650\n"
"a 2410 1650\n"
"a 2440 1650\n"
"a 2480 1650\n"
"a 2510 1650\n"
"k 140 1630\n"
"k 170 1630\n"
"k 210 1630\n"
"k 240 1630\n"
"k 270 1630\n"
"a 540 1620\n"
"a 570 1620\n"
"a 610 1620\n"
"a 650 1620\n"
"a 680 1620\n"
"a 720 1620\n"
"a 750 1620\n"
"a 2300 1620\n"
"a 2330 1620\n"
"a 2370 1620\n"
"a 2410 1620\n"
"a 2440 1620\n"
"a 2480 1620\n"
"a 2510 1620\n"
"k 2800 1620\n"
"k 2830 1620\n"
"k 2870 1620\n"
"k 2900 1620\n"
"k 2930 1620\n"
"k 140 1600\n"
"k 170 1600\n"
"k 210 1600\n"
"k 240 1600\n"
"k 270 1600\n"
"a 540 1590\n"
"a 570 1590\n"
"a 610 1590\n"
"a 650 1590\n"
"a 680 1590\n"
"a 720 1590\n"
"a 750 1590\n"
"a 2300 1590\n"
"a 2330 1590\n"
"a 2370 1590\n"
"a 2410 1590\n"
"a 2440 1590\n"
"a 2480 1590\n"
"a 2510 1590\n"
"k 2800 1590\n"
"k 2830 1590\n"
"k 2870 1590\n"
"k 2900 1590\n"
"k 2930 1590\n"
"k 140 1570\n"
"k 170 1570\n"
"k 210 1570\n"
"k 240 1570\n"
"k 270 1570\n"
"k 360 1570\n"
"k 390 1570\n"
"k 430 1570\n"
"k 460 1570\n"
"k 490 1570\n"
"k 810 1570\n"
"k 840 1570\n"
"k 870 1570\n"
"k 910 1570\n"
"k 940 1570\n"
"k 970 1570\n"
"k 1000 1570\n"
"k 1030 1570\n"
"k 1060 1570\n"
"k 1090 1570\n"
"k 1130 1570\n"
"k 1160 1570\n"
"k 1190 1570\n"
"k 1330 1570\n"
"k 1360 1570\n"
"k 1390 1570\n"
"k 1430 1570\n"
"k 1460 1570\n"
"k 1490 1570\n"
"k 1520 1570\n"
"k 1550 1570\n"
"k 1580 1570\n"
"k 1610 1570\n"
"k 1650 1570\n"
"k 1680 1570\n"
"k 1710 1570\n"
"k 1850 1570\n"
"k 1880 1570\n"
"k 1910 1570\n"
"k 1950 1570\n"
"k 1980 1570\n"
"k 2010 1570\n"
"k 2040 1570\n"
"k 2070 1570\n"
"k 2100 1570\n"
"k 2130 1570\n"
"k 2170 1570\n"
"k 2200 1570\n"
"k 2230 1570\n"
"k 2600 1570\n"
"k 2630 1570\n"
"k 2670 1570\n"
"k 2700 1570\n"
"k 2730 1570\n"
"k 2800 1560\n"
"k 2830 1560\n"
"k 2870 1560\n"
"k 2900 1560\n"
"k 2930 1560\n"
"k 140 1540\n"
"k 170 1540\n"
"k 210 1540\n"
"k 240 1540\n"
"k 270 1540\n"
"k 360 1540\n"
"k 390 1540\n"
"k 430 1540\n"
"k 460 1540\n"
"k 490 1540\n"
"k 810 1540\n"
"k 840 1540\n"
"k 870 1540\n"
"k 910 1540\n"
"k 940 1540\n"
"k 970 1540\n"
"k 1000 1540\n"
"k 1030 1540\n"
"k 1060 1540\n"
"k 1090 1540\n"
"k 1130 1540\n"
"k 1160 1540\n"
"k 1190 1540\n"
"k 1330 1540\n"
"k 1360 1540\n"
"k 1390 1540\n"
"k 1430 1540\n"
"k 1460 1540\n"
"k 1490 1540\n"
"k 1520 1540\n"
"k 1550 1540\n"
"k 1580 1540\n"
"k 1610 1540\n"
"k 1650 1540\n"
"k 1680 1540\n"
"k 1710 1540\n"
"k 1850 1540\n"
"k 1880 1540\n"
"k 1910 1540\n"
"k 1950 1540\n"
"k 1980 1540\n"
"k 2010 1540\n"
"k 2040 1540\n"
"k 2070 1540\n"
"k 2100 1540\n"
"k 2130 1540\n"
"k 2170 1540\n"
"k 2200 1540\n"
"k 2230 1540\n"
"k 2600 1540\n"
"k 2630 1540\n"
"k 2670 1540\n"
"k 2700 1540\n"
"k 2730 1540\n"
"k 2800 1530\n"
"k 2830 1530\n"
"k 2870 1530\n"
"k 2900 1530\n"
"k 2930 1530\n"
"k 140 1510\n"
"k 170 1510\n"
"k 210 1510\n"
"k 240 1510\n"
"k 270 1510\n"
"k 360 1510\n"
"k 390 1510\n"
"k 430 1510\n"
"k 460 1510\n"
"k 490 1510\n"
"k 810 1510\n"
"k 840 1510\n"
"k 870 1510\n"
"k 910 1510\n"
"k 940 1510\n"
"k 970 1510\n"
"k 1000 1510\n"
"k 1030 1510\n"
"k 1060 1510\n"
"k 1090 1510\n"
"k 1130 1510\n"
"k 1160 1510\n"
"k 1190 1510\n"
"k 1330 1510\n"
"k 1360 1510\n"
"k 1390 1510\n"
"k 1430 1510\n"
"k 1460 1510\n"
"k 1490 1510\n"
"k 1520 1510\n"
"k 1550 1510\n"
"k 1580 1510\n"
"k 1610 1510\n"
"k 1650 1510\n"
"k 1680 1510\n"
"k 1710 1510\n"
"k 1850 1510\n"
"k 1880 1510\n"
"k 1910 1510\n"
"k 1950 1510\n"
"k 1980 1510\n"
"k 2010 1510\n"
"k 2040 1510\n"
"k 2070 1510\n"
"k 2100 1510\n"
"k 2130 1510\n"
"k 2170 1510\n"
"k 2200 1510\n"
"k 2230 1510\n"
"k 2600 1510\n"
"k 2630 1510\n"
"k 2670 1510\n"
"k 2700 1510\n"
"k 2730 1510\n"
"k 2800 1500\n"
"k 2830 1500\n"
"k 2870 1500\n"
"k 2900 1500\n"
"k 2930 1500\n"
"k 360 1480\n"
"k 390 1480\n"
"k 430 1480\n"
"k 460 1480\n"
"k 490 1480\n"
"k 810 1480\n"
"k 840 1480\n"
"k 870 1480\n"
"k 910 1480\n"
"k 940 1480\n"
"k 970 1480\n"
"k 1000 1480\n"
"k 1030 1480\n"
"k 1060 1480\n"
"k 1090 1480\n"
"k 1130 1480\n"
"k 1160 1480\n"
"k 1190 1480\n"
"k 1330 1480\n"
"k 1360 1480\n"
"k 1390 1480\n"
"k 1430 1480\n"
"k 1460 1480\n"
"k 1490 1480\n"
"k 1520 1480\n"
"k 1550 1480\n"
"k 1580 1480\n"
"k 1610 1480\n"
"k 1650 1480\n"
"k 1680 1480\n"
"k 1710 1480\n"
"k 1850 1480\n"
"k 1880 1480\n"
"k 1910 1480\n"
"k 1950 1480\n"
"k 1980 1480\n"
"k 2010 1480\n"
"k 2040 1480\n"
"k 2070 1480\n"
"k 2100 1480\n"
"k 2130 1480\n"
"k 2170 1480\n"
"k 2200 1480\n"
"k 2230 1480\n"
"k 2600 1480\n"
"k 2630 1480\n"
"k 2670 1480\n"
"k 2700 1480\n"
"k 2730 1480\n"
"k 360 1450\n"
"k 390 1450\n"
"k 430 1450\n"
"k 460 1450\n"
"k 490 1450\n"
"k 810 1450\n"
"k 840 1450\n"
"k 870 1450\n"
"k 910 1450\n"
"k 940 1450\n"
"k 970 1450\n"
"k 1000 1450\n"
"k 1030 1450\n"
"k 1060 1450\n"
"k 1090 1450\n"
"k 1130 1450\n"
"k 1160 1450\n"
"k 1190 1450\n"
"k 1330 1450\n"
"k 1360 1450\n"
"k 1390 1450\n"
"k 1430 1450\n"
"k 1460 1450\n"
"k 1490 1450\n"
"k 1520 1450\n"
"k 1550 1450\n"
"k 1580 1450\n"
"k 1610 1450\n"
"k 1650 1450\n"
"k 1680 1450\n"
"k 1710 1450\n"
"k 1850 1450\n"
"k 1880 1450\n"
"k 1910 1450\n"
"k 1950 1450\n"
"k 1980 1450\n"
"k 2010 1450\n"
"k 2040 1450\n"
"k 2070 1450\n"
"k 2100 1450\n"
"k 2130 1450\n"
"k 2170 1450\n"
"k 2200 1450\n"
"k 2230 1450\n"
"k 2600 1450\n"
"k 2630 1450\n"
"k 2670 1450\n"
"k 2700 1450\n"
"k 2730 1450\n"
"a 2920 1440\n"
"a 3090 1420\n"
"a 150 1380\n"
"a 1070 1380\n"
"a 1100 1380\n"
"a 1140 1380\n"
"a 1180 1380\n"
"a 1210 1380\n"
"a 1250 1380\n"
"a 1280 1380\n"
"a 1320 1380\n"
"a 1350 1380\n"
"a 1390 1380\n"
"a 1430 1380\n"
"a 1460 1380\n"
"a 1500 1380\n"
"a 1530 1380\n"
"a 1570 1380\n"
"a 1600 1380\n"
"a 1640 1380\n"
"a 1680 1380\n"
"a 1710 1380\n"
"a 1750 1380\n"
"a 1780 1380\n"
"a 1820 1380\n"
"a 1850 1380\n"
"a 1890 1380\n"
"a 1920 1380\n"
"a 1960 1380\n"
"a 2000 1380\n"
"a 2780 1380\n"
"a 230 1360\n"
"v 900 1360\n"
"v 930 1360\n"
"v 970 1360\n"
"a 1070 1350\n"
"a 1100 1350\n"
"a 1140 1350\n"
"a 1180 1350\n"
"a 1210 1350\n"
"a 1250 1350\n"
"a 1280 1350\n"
"a 1320 1350\n"
"a 1350 1350\n"
"a 1390 1350\n"
"a 1430 1350\n"
"a 1460 1350\n"
"a 1500 1350\n"
"a 1530 1350\n"
"a 1570 1350\n"
"a 1600 1350\n"
"a 1640 1350\n"
"a 1680 1350\n"
"a 1710 1350\n"
"a 1750 1350\n"
"a 1780 1350\n"
"a 1820 1350\n"
"a 1850 1350\n"
"a 1890 1350\n"
"a 1920 1350\n"
"a 1960 1350\n"
"a 2000 1350\n"
"v 2090 1350\n"
"v 2120 1350\n"
"v 2150 1350\n"
"v 900 1330\n"
"v 930 1330\n"
"v 970 1330\n"
"a 1070 1320\n"
"a 1100 1320\n"
"a 1140 1320\n"
"a 1180 1320\n"
"a 1210 1320\n"
"a 1250 1320\n"
"a 1280 1320\n"
"a 1320 1320\n"
"a 1350 1320\n"
"a 1390 1320\n"
"a 1430 1320\n"
"a 1460 1320\n"
"a 1500 1320\n"
"a 1530 1320\n"
"a 1570 1320\n"
"a 1600 1320\n"
"a 1640 1320\n"
"a 1680 1320\n"
"a 1710 1320\n"
"a 1750 1320\n"
"a 1780 1320\n"
"a 1820 1320\n"
"a 1850 1320\n"
"a 1890 1320\n"
"a 1920 1320\n"
"a 1960 1320\n"
"a 2000 1320\n"
"v 2090 1320\n"
"v 2120 1320\n"
"v 2150 1320\n"
"a 2640 1320\n"
"a 370 1310\n"
"a 490 1310\n"
"a 600 1300\n"
"a 720 1300\n"
"v 900 1300\n"
"v 930 1300\n"
"v 970 1300\n"
"a 1070 1290\n"
"a 1100 1290\n"
"a 1140 1290\n"
"a 1180 1290\n"
"a 1210 1290\n"
"a 1250 1290\n"
"a 1280 1290\n"
"a 1320 1290\n"
"a 1350 1290\n"
"a 1390 1290\n"
"a 1430 1290\n"
"a 1460 1290\n"
"a 1500 1290\n"
"a 1530 1290\n"
"a 1570 1290\n"
"a 1600 1290\n"
"a 1640 1290\n"
"a 1680 1290\n"
"a 1710 1290\n"
"a 1750 1290\n"
"a 1780 1290\n"
"a 1820 1290\n"
"a 1850 1290\n"
"a 1890 1290\n"
"a 1920 1290\n"
"a 1960 1290\n"
"a 2000 1290\n"
"v 2090 1290\n"
"v 2120 1290\n"
"v 2150 1290\n"
"a 2460 1290\n"
"a 2920 1260\n"
"k 810 1210\n"
"k 840 1210\n"
"k 870 1210\n"
"k 910 1210\n"
"k 940 1210\n"
"k 970 1210\n"
"k 1000 1210\n"
"k 1030 1210\n"
"k 1060 1210\n"
"k 1090 1210\n"
"k 1130 1210\n"
"k 1160 1210\n"
"k 1190 1210\n"
"k 1330 1210\n"
"k 1360 1210\n"
"k 1390 1210\n"
"k 1430 1210\n"
"k 1460 1210\n"
"k 1490 1210\n"
"k 1520 1210\n"
"k 1550 1210\n"
"k 1580 1210\n"
"k 1610 1210\n"
"k 1650 1210\n"
"k 1680 1210\n"
"k 1710 1210\n"
"k 1850 1210\n"
"k 1880 1210\n"
"k 1910 1210\n"
"k 1950 1210\n"
"k 1980 1210\n"
"k 2010 1210\n"
"k 2040 1210\n"
"k 2070 1210\n"
"k 2100 1210\n"
"k 2130 1210\n"
"k 2170 1210\n"
"k 2200 1210\n"
"k 2230 1210\n"
"k 810 1180\n"
"k 840 1180\n"
"k 870 1180\n"
"k 910 1180\n"
"k 940 1180\n"
"k 970 1180\n"
"k 1000 1180\n"
"k 1030 1180\n"
"k 1060 1180\n"
"k 1090 1180\n"
"k 1130 1180\n"
"k 1160 1180\n"
"k 1190 1180\n"
"k 1330 1180\n"
"k 1360 1180\n"
"k 1390 1180\n"
"k 1430 1180\n"
"k 1460 1180\n"
"k 1490 1180\n"
"k 1520 1180\n"
"k 1550 1180\n"
"k 1580 1180\n"
"k 1610 1180\n"
"k 1650 1180\n"
"k 1680 1180\n"
"k 1710 1180\n"
"k 1850 1180\n"
"k 1880 1180\n"
"k 1910 1180\n"
"k 1950 1180\n"
"k 1980 1180\n"
"k 2010 1180\n"
"k 2040 1180\n"
"k 2070 1180\n"
"k 2100 1180\n"
"k 2130 1180\n"
"k 2170 1180\n"
"k 2200 1180\n"
"k 2230 1180\n"
"a 2370 1160\n"
"k 810 1150\n"
"k 840 1150\n"
"k 870 1150\n"
"k 910 1150\n"
"k 940 1150\n"
"k 970 1150\n"
"k 1000 1150\n"
"k 1030 1150\n"
"k 1060 1150\n"
"k 1090 1150\n"
"k 1130 1150\n"
"k 1160 1150\n"
"k 1190 1150\n"
"k 1330 1150\n"
"k 1360 1150\n"
"k 1390 1150\n"
"k 1430 1150\n"
"k 1460 1150\n"
"k 1490 1150\n"
"k 1520 1150\n"
"k 1550 1150\n"
"k 1580 1150\n"
"k 1610 1150\n"
"k 1650 1150\n"
"k 1680 1150\n"
"k 1710 1150\n"
"k 1850 1150\n"
"k 1880 1150\n"
"k 1910 1150\n"
"k 1950 1150\n"
"k 1980 1150\n"
"k 2010 1150\n"
"k 2040 1150\n"
"k 2070 1150\n"
"k 2100 1150\n"
"k 2130 1150\n"
"k 2170 1150\n"
"k 2200 1150\n"
"k 2230 1150\n"
"a 640 1130\n"
"a 2670 1130\n"
"k 810 1120\n"
"k 840 1120\n"
"k 870 1120\n"
"k 910 1120\n"
"k 940 1120\n"
"k 970 1120\n"
"k 1000 1120\n"
"k 1030 1120\n"
"k 1060 1120\n"
"k 1090 1120\n"
"k 1130 1120\n"
"k 1160 1120\n"
"k 1190 1120\n"
"k 1330 1120\n"
"k 1360 1120\n"
"k 1390 1120\n"
"k 1430 1120\n"
"k 1460 1120\n"
"k 1490 1120\n"
"k 1520 1120\n"
"k 1550 1120\n"
"k 1580 1120\n"
"k 1610 1120\n"
"k 1650 1120\n"
"k 1680 1120\n"
"k 1710 1120\n"
"k 1850 1120\n"
"k 1880 1120\n"
"k 1910 1120\n"
"k 1950 1120\n"
"k 1980 1120\n"
"k 2010 1120\n"
"k 2040 1120\n"
"k 2070 1120\n"
"k 2100 1120\n"
"k 2130 1120\n"
"k 2170 1120\n"
"k 2200 1120\n"
"k 2230 1120\n"
"k 810 1090\n"
"k 840 1090\n"
"k 870 1090\n"
"k 910 1090\n"
"k 940 1090\n"
"k 970 1090\n"
"k 1000 1090\n"
"k 1030 1090\n"
"k 1060 1090\n"
"k 1090 1090\n"
"k 1130 1090\n"
"k 1160 1090\n"
"k 1190 1090\n"
"k 1330 1090\n"
"k 1360 1090\n"
"k 1390 1090\n"
"k 1430 1090\n"
"k 1460 1090\n"
"k 1490 1090\n"
"k 1520 1090\n"
"k 1550 1090\n"
"k 1580 1090\n"
"k 1610 1090\n"
"k 1650 1090\n"
"k 1680 1090\n"
"k 1710 1090\n"
"k 1850 1090\n"
"k 1880 1090\n"
"k 1910 1090\n"
"k 1950 1090\n"
"k 1980 1090\n"
"k 2010 1090\n"
"k 2040 1090\n"
"k 2070 1090\n"
"k 2100 1090\n"
"k 2130 1090\n"
"k 2170 1090\n"
"k 2200 1090\n"
"k 2230 1090\n"
"a 2530 1090\n"
"a 1020 1010\n"
"a 1240 1010\n"
"a 1460 1000\n"
"a 800 990\n"
"a 1840 970\n"
"a 2050 970\n"
"s 1680 780\n"
"s 1510 770\n"
"s 1660 740\n"
"s 1530 730\n"
"s 1690 710\n"
"s 1520 700\n"
"s 1680 660\n"
"s 1530 650\n"
"s 610 640\n"
"s 790 630\n"
"s 1140 630\n"
"s 340 620\n"
"s 520 620\n"
"s 940 620\n"
"s 1080 620\n"
"s 1660 620\n"
"s 440 610\n"
"s 720 610\n"
"s 670 600\n"
"s 870 600\n"
"s 1140 600\n"
"s 1260 600\n"
"s 1410 600\n"
"s 1530 600\n"
"s 1210 590\n"
"s 1330 590\n"
"s 1480 590\n"
"s 1010 580\n"
"s 1260 580\n"
"s 1330 570\n"
"s 1430 570\n"
"s 1690 570\n"
"s 1500 560\n"
"s 1790 560\n"
"s 1850 560\n"
"s 1980 560\n"
"s 2080 550\n"
"s 2260 550\n"
"s 2370 550\n"
"s 2470 550\n"
"s 2520 550\n"
"s 280 540\n"
"s 340 540\n"
"s 1730 540\n"
"s 1820 540\n"
"s 2190 540\n"
"s 2450 540\n"
"v 460 530\n"
"v 560 530\n"
"v 880 530\n"
"s 1900 530\n"
"s 2160 530\n"
"v 970 520\n"
"s 2400 520\n"
"s 2650 520\n"
"v 700 500\n"
"v 1680 500\n"
"v 2260 500\n"
"s 2600 500\n"
"v 610 490\n"
"v 820 490\n"
"v 1130 490\n"
"v 1320 490\n"
"v 1440 490\n"
"v 1540 490\n"
"v 1810 490\n"
"v 2040 490\n"
"v 1220 480\n"
"v 1890 480\n"
"v 2100 480\n"
"v 2330 480\n"
"v 1060 470\n"
"v 1400 470\n"
"v 1720 470\n"
"v 1770 470\n"
"v 1980 470\n"
"v 2170 470\n"
"v 2460 470\n"
"s 290 460\n"
"v 440 460\n"
"v 1290 460\n"
"v 1830 460\n"
"v 2240 460\n"
"v 2530 460\n"
"s 180 450\n"
"v 1480 450\n"
"s 2700 450\n"
"v 1710 440\n"
"v 2390 430\n"
"s 2800 430\n"
"v 360 420\n"
"s 260 410\n"
"v 1580 400\n"
"v 2540 400\n"
"s 2870 380\n"
"v 330 350\n"
"s 2790 350\n"
"b 550 340\n"
"b 800 340\n"
"b 920 340\n"
"b 1030 340\n"
"b 1150 340\n"
"b 1270 340\n"
"b 1390 340\n"
"b 1510 340\n"
"b 1700 340\n"
"b 1820 340\n"
"b 1930 340\n"
"b 2050 340\n"
"b 2170 340\n"
"b 2290 340\n"
"b 2410 340\n"
"s 190 330\n"
"s 230 320\n"
"v 660 320\n"
"s 2880 310\n"
"v 1620 300\n"
"v 340 290\n"
"v 550 280\n"
"v 1880 280\n"
"v 2270 280\n"
"v 2020 270\n"
"b 2660 270\n"
"s 2940 270\n"
"v 2450 260\n"
"s 2820 260\n"
"s 190 220\n"
"v 670 220\n"
"b 800 220\n"
"b 920 220\n"
"b 1030 220\n"
"b 1150 220\n"
"b 1270 220\n"
"b 1390 220\n"
"b 1510 220\n"
"b 1700 220\n"
"b 1820 220\n"
"b 1930 220\n"
"b 2050 220\n"
"b 2170 220\n"
"b 2290 220\n"
"b 2410 220\n"
"b 2550 210\n"
"s 270 200\n"
"v 1590 200\n"
"s 2870 200\n"
"v 470 190\n"
"v 2160 180\n"
"s 2960 180\n"
"v 2490 170\n"
"b 540 160\n"
"s 250 150\n"
"v 1590 130\n"
"s 2990 120\n"
"v 690 110\n"
"b 800 100\n"
"b 920 100\n"
"b 1030 100\n"
"b 1150 100\n"
"b 1270 100\n"
"b 1390 100\n"
"b 1510 100\n"
"b 1700 100\n"
"b 1820 100\n"
"b 1930 100\n"
"b 2050 100\n"
"b 2170 100\n"
"b 2290 100\n"
"b 2410 100\n"
"b 2620 100\n"
"s 190 90\n"
"s 3040 70\n"
"s 2960 60\n"
"s 230 50\n"
                               );
    stringstream ss_file3(string_file3);
    while( getline(ss_file3,line) )
    {
        st_pos pos;
        stringstream ss(line);
        ss>>word;
        ss>>word;
        pos.x=(int)atof(word.c_str());
        ss>>word;
        pos.y=(int)atof(word.c_str());

        switch(line[0])
        {
            case 'b'://building
            {
                m_vec_pBuildings.push_back( new building(pos,m_tex_building) );
            }break;

            case 's'://stone
            {
                m_vec_pObjects.push_back( new object(pos,m_tex_rock) );
            }break;

            case 'v'://villager
            {
                m_vec_preunits.push_back( st_pre_unit( unit_spec(ut_villager),pos,m_tex_unit_move ) );

                /*m_vec_pUnits.push_back( new unit( unit_spec(ut_villager), pos,m_tex_unit_move) );
                int col_square_x=pos.x/_col_grid_size;
                int col_square_y=pos.y/_col_grid_size;
                m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits.back() );
                m_vec_pUnits.back()->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );*/
            }break;

            case 'a'://bowman
            {
                m_vec_preunits.push_back( st_pre_unit( unit_spec(ut_bowman),pos,m_tex_bowman ) );

                /*m_vec_pUnits.push_back( new unit( unit_spec(ut_bowman), pos,m_tex_bowman) );
                int col_square_x=pos.x/_col_grid_size;
                int col_square_y=pos.y/_col_grid_size;
                m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits.back() );
                m_vec_pUnits.back()->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );*/
            }break;

            case 'k'://knight
            {
                m_vec_preunits.push_back( st_pre_unit( unit_spec(ut_knight),pos,m_tex_knight ) );

                /*m_vec_pUnits.push_back( new unit( unit_spec(ut_knight), pos,m_tex_knight) );
                int col_square_x=pos.x/_col_grid_size;
                int col_square_y=pos.y/_col_grid_size;
                m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits.back() );
                m_vec_pUnits.back()->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );*/
            }break;
        }
    }

    return true;
}

//convert bmp to level list data
bool game::convert_level(void)
{
    ofstream ofile("data\\list.txt");
    if(ofile==0)
    {
        cout<<"ERROR: Could not create file\n";
        return false;
    }

    FILE *fp = fopen("level.bmp", "r");
    if(!fp)
    {
        cout<<"ERROR: Could not find level.bmp\n";
        return 1;
    }
    unsigned short uint;
    unsigned int dword;
    unsigned short word;
    long llong;
    //read some data we don't need
    fread(&uint, sizeof(uint), 1, fp);
    fread(&dword, sizeof(dword), 1, fp);
    fread(&uint, sizeof(uint), 1, fp);
    fread(&uint, sizeof(uint), 1, fp);
    fread(&dword, sizeof(dword), 1, fp);
    fread(&dword, sizeof(dword), 1, fp);
    long width, height;
    fread(&width, sizeof(long), 1, fp);
    fread(&height, sizeof(long), 1, fp);
    fread(&word, sizeof(word), 1, fp);
    cout<<"Size of bitmap: "<<width<<", "<<height<<endl;
    if(width%4!=0 || height%4!=0)
    {
        cout<<"ERROR: Width and Height of the bitmap have to be devisible by 4\n";
        return 1;
    }
    unsigned short bitcount;
    fread(&bitcount, sizeof(unsigned short), 1, fp);
    if(bitcount != 24)
    {
        cout << "ERROR: Bitmap is not an 24bit image\n";
        return 1;
    }
    unsigned long compression;
    fread(&compression, sizeof(unsigned long), 1, fp);
    if(compression != 0)
    {
        cout << "ERROR: Invalid compression. Make sure the BMP is not using RLE\n";
        return 1;
    }
    fread(&dword, sizeof(dword), 1, fp);
    fread(&llong, sizeof(long), 1, fp);
    fread(&llong, sizeof(long), 1, fp);
    fread(&dword, sizeof(dword), 1, fp);
    fread(&dword, sizeof(dword), 1, fp);
    unsigned char *imagedata;
    imagedata = new unsigned char[width * height * 4];

    //pixel read
    //for(int x=0; x < width; x++)
    for(int y=0; y < height; y++)
    {
        //for(int y=0; y < height; y++)
        for(int x=0; x < width; x++)
        {
            fread(&imagedata[(x * height + y)*4+0], sizeof(char), 1, fp);
            fread(&imagedata[(x * height + y)*4+1], sizeof(char), 1, fp);
            fread(&imagedata[(x * height + y)*4+2], sizeof(char), 1, fp);

            //BGR to RGB format
            int r = imagedata[(x * height + y)*4+2];
            int g = imagedata[(x * height + y)*4+1];
            int b = imagedata[(x * height + y)*4+0];
            imagedata[(x * height + y)*4+0] = r;
            //imagedata[(x * height + y)*4+1] = g;
            imagedata[(x * height + y)*4+2] = b;

            /*//hot pink for alpha channel
            if(imagedata[(x * height + y)*4+0] == 255 &&
               imagedata[(x * height + y)*4+1] == 0   &&
               imagedata[(x * height + y)*4+2] == 255)
             imagedata[(x * height + y)*4+3] = 0;
            else
             imagedata[(x * height + y)*4+3] = 255;*/

            //cout<<r<<", "<<g<<", "<<b<<endl;
            //mark if have color
            //BLACK stone
            if(r==0 && g==0 && b==0)
            {
                ofile<<"s "<<x*10<<" "<<(height-y-1)*10<<endl;
            }
            //GRAY terrain coord and cave marker
            if(r==100 && g==100 && b==100)
            {
                ofile<<"v "<<x*10<<" "<<(height-y-1)*10<<endl;
            }
            //BLUE object
            if(r<10 && g<10 && b>250)
            {
                ofile<<"b "<<x*10<<" "<<(height-y-1)*10<<endl;
            }
            //RED enemy (underground pos)
            if(r>250 && g<10 && b<10)
            {
                ofile<<"a "<<x*10<<" "<<(height-y-1)*10<<endl;
            }
            //ORANGE enemy (surface pos)
            if(r>250 && g<130 && g>110 && b<10)
            {

            }
            //GREEN player start pos
            if(r<10 && g>250 && b<10)
            {

                ofile<<"k "<<x*10<<" "<<(height-y-1)*10<<endl;
            }
        }
    }
    delete[] imagedata;
    fclose(fp);

    ofile.close();

    /*ofstream out("temp");
    for(int i=0;i<200;i++)
    {
        for(int j=0;j<50;j++)
        {
            out<<"0";
        }
        out<<endl;
    }*/

    return true;
}
