#include "unit.h"

unit::unit()
{
    //ctor
}

unit::unit(unit_spec specs)
{
    m_spec=specs;
}

unit::unit(unit_spec specs,st_pos pos,int texture)
{
    m_spec=specs;
    init(pos,texture);
}

bool unit::init(st_pos pos,int texture)
{
    m_pos_defence=m_target_pos=m_pos_new=m_pos=pos;
    m_pos_attack_move_target.x=-1; m_pos_attack_move_target.y=-1;
    m_size=_unit_size;
    m_selected=false;
    m_rotation_target_final=m_rotation_target=m_rotation_curr=rand()%360;
    if(m_spec.type==ut_knight)
     m_rotation_target_final=m_rotation_target=m_rotation_curr=90;
    m_awake=true;
    m_have_moved=true;
    m_attack_cooldown=0;//ready to fire
    m_attack_mode=am_pursuit;
    m_forced_move=false;
    m_attack_moving=false;
    m_is_idle=true;
    m_rotation_check_timer=float(rand()%1000)/1000.0*_unit_rotation_check_delay;
    m_defence_stay_timer=_unit_defence_stay_delay;
    m_col_id=0;
    m_col_state=none;
    m_pUnit_enemy_target=0;
    m_random_move_timer=float(rand()%500)/10.0;
    m_player_controlled=m_player_move_order=false;
    m_texture=texture;
    m_walk_prog=0;
    m_projectile_mode=false;
    m_attack_mode_cooldown=0;

    return true;
}

bool unit::update(void)
{
    //projectile mode, thrown
    if(m_projectile_mode)
    {
        m_pos.x+=m_projectile_direction.x*_unit_projectile_speed;
        m_pos.y+=m_projectile_direction.y*_unit_projectile_speed;

        m_target_pos=m_pos_defence=m_pos_new=m_pos;

        //rotate
        m_rotation_curr+=_object_rotation_speed*_time_step;

        if(m_pos.x<0 || m_pos.x>_world_width ||
           m_pos.y<0 || m_pos.y>_world_height ) m_spec.hp_curr=0;

        return true;
    }

    //time
    if(m_attack_mode_cooldown>0 && m_attack_cooldown<=0)
     m_attack_mode_cooldown-=_time_step;
    //else m_attack_mode_cooldown=0.1;
    if(m_attack_cooldown>0) m_attack_cooldown-=_time_step;



    //stop here if player controlled
    if(m_player_controlled)
    {
        while(m_rotation_curr>180.0) m_rotation_curr-=360.0;
        while(m_rotation_curr<-180.0) m_rotation_curr+=360.0;
        while(m_rotation_target>180.0) m_rotation_target-=360.0;
        while(m_rotation_target<-180.0) m_rotation_target+=360.0;

        float flip_dir=1.0;
        if( m_rotation_curr-m_rotation_target<-180 ||
            m_rotation_curr-m_rotation_target>180 )
        {
            flip_dir=-1.0;
        }

        if(m_rotation_curr<m_rotation_target)
        {
            m_rotation_curr+=m_spec.speed_rotate*flip_dir*5.0;
            if(m_rotation_curr>=m_rotation_target)
            {
                m_rotation_curr=m_rotation_target;
            }
        }
        else if(m_rotation_curr>m_rotation_target)
        {
            m_rotation_curr-=m_spec.speed_rotate*flip_dir*5.0;
            if(m_rotation_curr<=m_rotation_target)
            {
                m_rotation_curr=m_rotation_target;
            }
        }

        return true;
    }


    if(!m_awake) return false;

    //if idle and have old attack move order, resume
    if(m_is_idle && m_pos_attack_move_target.x!=-1 && m_pos_attack_move_target.y!=-1)
    {
        m_is_idle=false;
        m_attack_moving=true;

        m_target_pos=m_pos_attack_move_target;
        //recalc new target angle
        st_pos rel_pos(m_target_pos.x-m_pos.x,m_target_pos.y-m_pos.y);
        m_rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
    }

    //start random move
    if(m_is_idle && m_spec.type==ut_villager)
    {
        if(m_random_move_timer>0) m_random_move_timer-=_time_step;
        if(m_random_move_timer<=0)
        {
            m_random_move_timer=float(rand()%500)/10.0;

            //give move order
            st_pos new_pos(rand()%30-15,rand()%30-15);
            new_pos+=m_pos;
            //move_to(new_pos);

            m_pos_attack_move_target=new_pos;
        }
    }

    //test if attack move complete
    if(m_attack_moving && is_pos_inside(m_pos_attack_move_target) )
    {
        m_pos_attack_move_target=st_pos(-1,-1);
        m_attack_moving=false;
    }


    //test rotation
    if(m_rotation_curr!=m_rotation_target)
    {
        m_is_idle=false;

        while(m_rotation_curr>180.0) m_rotation_curr-=360.0;
        while(m_rotation_curr<-180.0) m_rotation_curr+=360.0;
        while(m_rotation_target>180.0) m_rotation_target-=360.0;
        while(m_rotation_target<-180.0) m_rotation_target+=360.0;

        float flip_dir=1.0;
        if( m_rotation_curr-m_rotation_target<-180 ||
            m_rotation_curr-m_rotation_target>180 )
        {
            flip_dir=-1.0;
        }

        if(m_rotation_curr<m_rotation_target)
        {
            m_rotation_curr+=m_spec.speed_rotate*flip_dir;
            if(m_rotation_curr>=m_rotation_target)
            {
                m_rotation_curr=m_rotation_target;

                //calc move vector
                //st_pos rel_pos(m_target_pos.x-m_pos.x,m_target_pos.y-m_pos.y);
                //m_target_direction=rel_pos.normalize();

                m_target_direction.x=cosf(m_rotation_curr*_Deg2Rad);
                m_target_direction.y=sinf(m_rotation_curr*_Deg2Rad);
            }
        }
        else if(m_rotation_curr>m_rotation_target)
        {
            m_rotation_curr-=m_spec.speed_rotate*flip_dir;
            if(m_rotation_curr<=m_rotation_target)
            {
                m_rotation_curr=m_rotation_target;

                //calc move vector
                //st_pos rel_pos(m_target_pos.x-m_pos.x,m_target_pos.y-m_pos.y);
                //m_target_direction=rel_pos.normalize();

                m_target_direction.x=cosf(m_rotation_curr*_Deg2Rad);
                m_target_direction.y=sinf(m_rotation_curr*_Deg2Rad);
            }
        }
        //cout<<m_rotation_curr<<"\t"<<m_rotation_target<<"\t"<<m_rotation_target_final<<endl;
    }

    //test position, if rotation is correct
    if(m_rotation_curr==m_rotation_target && m_pos!=m_target_pos)
    {
        //if(m_attack_cooldown<=0 && m_attack_mode_cooldown<=0)
        {
            m_is_idle=false;

            //measure square dist, have target been reached
            if( is_pos_inside(m_target_pos) )
            {
                //test if a final rotation should be reached
                if(m_rotation_target_final==m_rotation_target)
                {
                    //movement complete
                    m_target_pos=m_pos;

                    //cancel the no-col for formation
                    if(m_col_state==active) m_col_state=passive;

                    //test if attack move complete
                    if(m_attack_moving && is_pos_inside(m_pos_attack_move_target) )
                    {
                        m_pos_attack_move_target=st_pos(-1,-1);
                        m_attack_moving=false;
                    }

                    m_forced_move=false;
                    m_attack_moving=false;
                    m_is_idle=true;
                    m_pUnit_enemy_target=0;

                    //cout<<"Unit move complete\n";
                }
                else//update target rotation
                {
                    m_rotation_target=m_rotation_target_final;
                }
            }
            //calc a new pos to move to
            else if(m_pos.x!=m_target_pos.x || m_pos.y!=m_target_pos.y )
            {
                //cout<<"Unit is moving\n";

                //target rotation check, pushing can shift the target angle and need to be checked
                if(m_rotation_check_timer>0.0) m_rotation_check_timer-=_time_step;
                if(m_rotation_check_timer<=0)
                {
                    m_rotation_check_timer=_unit_rotation_check_delay;

                    //test angle to target
                    st_pos rel_pos(m_target_pos.x-m_pos.x,m_target_pos.y-m_pos.y);
                    m_rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;

                    //cout<<"Unit: Target angle updated\n";
                }

                //test if have moved, otherwise prob colided
                int slide=0;
                if(m_pos.x!=m_pos_new.x || m_pos.y!=m_pos_new.y)
                {
                    //reset walk prog
                    m_walk_prog=0;

                    //slide, cancel x or y
                    slide=rand()%3;
                }
                //calc new pos
                switch(slide)
                {
                    case 0://slide off, normal
                    {
                        m_pos_new.x=m_pos.x+m_target_direction.x*m_spec.speed_move;
                        m_pos_new.y=m_pos.y+m_target_direction.y*m_spec.speed_move;
                    }break;
                    case 1://slide off, normal
                    {
                        m_pos_new.x=m_pos.x+m_target_direction.x*m_spec.speed_move*2.0;
                        m_pos_new.y=m_pos.y;
                    }break;
                    case 2://slide off, normal
                    {
                        m_pos_new.x=m_pos.x;
                        m_pos_new.y=m_pos.y+m_target_direction.y*m_spec.speed_move*2.0;
                    }break;
                }

                //calc new pos
                //m_pos_new.x=m_pos.x+m_target_direction.x*m_spec.speed_move;
                //m_pos_new.y=m_pos.y+m_target_direction.y*m_spec.speed_move;

                m_walk_prog+=m_spec.speed_move*_time_step*_unit_walk_anim_speed;
                if(m_walk_prog>1.0) m_walk_prog-=1.0;

                //test defence area
                if(m_attack_mode==am_defence && !m_forced_move)
                {
                    //the new pos should be inside the defence area unless forced move
                    if( !is_pos_inside_rect( st_pos( m_pos_defence.x-m_spec.defence_range,m_pos_defence.y-m_spec.defence_range ),
                                             st_pos( m_pos_defence.x+m_spec.defence_range,m_pos_defence.y+m_spec.defence_range ) ) )
                    {
                        //exception, if moving towards the defence pos
                        st_pos rel_pos(m_pos_defence.x-m_pos.x,m_pos_defence.y-m_pos.y);
                        if( (rel_pos.x>=0 && m_target_direction.x<0) ||
                            (rel_pos.x<=0 && m_target_direction.x>0) ||
                            (rel_pos.y>=0 && m_target_direction.y<0) ||
                            (rel_pos.y<=0 && m_target_direction.y>0) )
                        {
                            //wrong direction towards defence pos, stop movement
                            m_pos_new=m_pos;
                            m_target_pos=m_pos;
                        }
                        //else moving towards the defence pos, allow movement

                    }
                }

                //world edge test
                if(m_pos_new.x<=0 || m_pos_new.x>=_world_width ||
                   m_pos_new.y<=0 || m_pos_new.y>=_world_height )
                {
                    //stop unit
                    m_pos_new=m_pos;
                    m_target_pos=m_pos;
                }
            }
        }
    }

    //test if a new pos is wanted
    if(m_pos.x==m_pos_new.x && m_pos.y==m_pos_new.y) m_have_moved=false;
    else m_have_moved=true;

    //test if idle
    if(m_pos==m_target_pos && m_rotation_curr==m_rotation_target &&
       /*m_attack_mode_cooldown<=0.0 &&*/ m_attack_cooldown<=0 )
    {
        m_is_idle=true;

        //return to defence point
        if(m_attack_mode==am_defence && !is_pos_inside(m_pos_defence) )
        {
            if(m_defence_stay_timer>0) m_defence_stay_timer-=_time_step;
            if(m_defence_stay_timer<=0)
            {
                m_defence_stay_timer=_unit_defence_stay_delay;

                //calc new move order
                m_target_pos=m_pos_defence;
                st_pos rel_pos(m_target_pos.x-m_pos.x,m_target_pos.y-m_pos.y);
                m_rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;

                //cout<<"Unit: Defence mode, return to defence point\n";
            }
        }

        //restore legs
        if(m_walk_prog!=0)
        {
            if(m_walk_prog<0.5)
            {
                m_walk_prog-=_unit_walk_anim_speed*1.0*_time_step;
                if(m_walk_prog<0.0)
                {
                    m_walk_prog=0;//complete
                }
            }
            else//increase val
            {
                m_walk_prog+=_unit_walk_anim_speed*1.0*_time_step;
                if(m_walk_prog>1.0)
                {
                    m_walk_prog=0;//complete
                }
            }
        }
    }
    else//not idle
    {
        //reset defence stay delay timer
        m_defence_stay_timer=_unit_defence_stay_delay;
    }

    return true;
}

bool unit::draw(void)
{
    //return false;//temp

    if(m_size==_player_size) return true;//skip drawing player, drawn later

    float size=m_size;
    if(m_player_controlled)
    {
        glColor3f(0.2,0.8,0.2);
        size=_player_size;
    }
    else glColor3f(0.5,0.0,0.0);

    //temp for attacked drawing
    if(m_selected)
    {
        glColor3f(1.0,1.0,1.0);
        m_selected=false;
    }

    glPushMatrix();
    glTranslatef(m_pos.x,m_pos.y,0.0);
    glRotatef(m_rotation_curr-90.0,0,0,1);

    float image_size_x=208;
    float image_size_y=260;
    float frame_size=52;//52 38
    int frame=int(m_walk_prog*8.0);
    if(frame>=8) frame=0;
    int row=int(float(frame)/4.0);
    int column=frame-(row*4);

    //attack move
    if(m_attack_cooldown>0)
    {
        m_walk_prog=0;

        if(m_spec.type==ut_knight)
        {
            frame=int((1.0-(m_attack_cooldown/m_spec.attack_speed))*10.0);
            if(frame==10) frame=0;
        }
        if(m_spec.type==ut_bowman)
        {
            frame=int((1.0-(m_attack_cooldown/m_spec.attack_speed))*4.0);
            if(frame==4) frame=0;
        }

        frame+=8;
        row=int(float(frame)/4.0);
        column=frame-(row*4);
    }

    glColor3f(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_texture);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glTexCoord2f(frame_size*(column)/image_size_x,frame_size*(row)/image_size_y);
    glVertex2f(frame_size*0.5,frame_size*0.5);
    glTexCoord2f(frame_size*(column)/image_size_x,frame_size*(row+1.0)/image_size_y);
    glVertex2f(frame_size*0.5,-frame_size*0.5);
    glTexCoord2f(frame_size*(column+1.0)/image_size_x,frame_size*(row+1.0)/image_size_y);
    glVertex2f(-frame_size*0.5,-frame_size*0.5);
    glTexCoord2f(frame_size*(column+1.0)/image_size_x,frame_size*(row)/image_size_y);
    glVertex2f(-frame_size*0.5,frame_size*0.5);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glPopMatrix();

    /*glBegin(GL_QUADS);
    glVertex2f(m_size,-m_size);
    glVertex2f(-m_size,-m_size);
    glVertex2f(-m_size,m_size);
    glVertex2f(m_size,m_size);
    glEnd();

    glPopMatrix();*/

    /*//temp
    glVertex2f(m_pos.x+m_size,m_pos.y-m_size);
    glVertex2f(m_pos.x-m_size,m_pos.y-m_size);
    glVertex2f(m_pos.x-m_size,m_pos.y+m_size);
    glVertex2f(m_pos.x+m_size,m_pos.y+m_size);*/

    return true;
}

bool unit::is_pos_inside(st_pos pos,float size_factor,bool test_new_pos)
{
    float hit_size=m_size*size_factor;
    //float hit_size=m_size*1.0;//1.0 ok for mouse, real but if unit-unit col use 2.0, size of the other unit

    if(test_new_pos)
    {
        if( ( m_pos.x+hit_size > pos.x && m_pos.x-hit_size < pos.x &&
              m_pos.y+hit_size > pos.y && m_pos.y-hit_size < pos.y ) ||
            ( m_pos_new.x+hit_size > pos.x && m_pos_new.x-hit_size < pos.x &&
              m_pos_new.y+hit_size > pos.y && m_pos_new.y-hit_size < pos.y ) )
        {
            return true;
        }
        else return false;
    }

    if( m_pos.x+hit_size > pos.x && m_pos.x-hit_size < pos.x &&
        m_pos.y+hit_size > pos.y && m_pos.y-hit_size < pos.y )
    {
        return true;
    }

    return false;
}

bool unit::is_pos_inside_rect(st_pos pos_top_left,st_pos pos_low_right)
{
    if( pos_top_left.x < m_pos.x && pos_low_right.x > m_pos.x &&
        pos_top_left.y < m_pos.y && pos_low_right.y > m_pos.y )
    {
        return true;
    }

    return false;
}

bool unit::player_move(st_pos move_vec)
{
    if(true)//without col
    {
        //calc rotation target
        if(move_vec.x==0.0 && move_vec.y==0.0) ;//skip rotation
        else
        {
            st_pos rel_pos(move_vec.x,move_vec.y);
            m_rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
        }

        //stop outside world
        if(m_pos.x+move_vec.x<0) move_vec.x=0;
        if(m_pos.x+move_vec.x>_world_width) move_vec.x=0;
        if(m_pos.y+move_vec.y<0) move_vec.y=0;
        if(m_pos.y+move_vec.y>_world_height) move_vec.y=0;

        m_pos_new=m_pos+=move_vec;//direct
    }
    else//have to pass col test
    {
        m_pos_new=m_pos+move_vec;//indirect
        //calc rotation target
        if(move_vec.x==0.0 && move_vec.y==0.0) ;//skip rotation
        else
        {
            st_pos rel_pos(m_pos_new.x-m_pos.x,m_pos_new.y-m_pos.y);
            m_rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
        }
    }


    return true;
}

bool unit::move_to(st_pos end_pos,bool is_forced,float final_rotation,unit* pEnemy_target)//used to move unit to enemy pos within sight
{
    //cancel the no-col for formation
    if(m_col_state==active) m_col_state=none;

    //ignore if under attack move order, or forced move
    if( (m_attack_moving || m_forced_move) && !is_forced ) return false;

    m_awake=true;
    m_target_pos=end_pos;
    st_pos rel_pos(m_target_pos.x-m_pos.x,m_target_pos.y-m_pos.y);
    m_rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
    //cout<<m_rotation_target<<endl;
    if(final_rotation==9999) m_rotation_target_final=m_rotation_target;
    else m_rotation_target_final=final_rotation;
    m_pUnit_enemy_target=pEnemy_target;

    if(is_forced)
    {
        m_forced_move=true;
        m_attack_moving=false;
        m_pos_attack_move_target=st_pos(-1,-1);//inactivate
        m_pos_defence=m_target_pos;
    }

    //cout<<m_spec.team<<" UNIT: Move target: "<<m_target_pos.x<<", "<<m_target_pos.y<<" Target angle: "<<m_rotation_target<<endl;

    return true;
}

bool unit::move_to_rel(st_pos rel_pos_shift,float move_direction,bool forced_move,float rot_val[4])//used by the user to move units
{
    //cancel the no-col for formation
    if(m_col_state==active) m_col_state=none;

    m_awake=true;
    m_forced_move=forced_move;
    m_attack_moving=!m_forced_move;
    m_target_pos=m_pos+rel_pos_shift;
    m_rotation_target_final=m_rotation_target=move_direction;
    m_pos_defence=m_target_pos;
    m_pUnit_enemy_target=0;

    if(m_attack_moving) m_pos_attack_move_target=m_target_pos;
    else m_pos_attack_move_target=st_pos(-1,-1);//inactivate

    //calc new target pos if specific rotation required
    if(rot_val[0]==0&&rot_val[1]==0&&rot_val[2]==0&&rot_val[3]==0)
    {
        ;//normal rel pos move
    }
    else//new target pos after rotation around center pos (rot_val[0 and 1])
    {
        float rot_pos_x=rot_val[0]+( rot_val[2]*(m_target_pos.x-rot_val[0])+rot_val[3]*(m_target_pos.y -rot_val[1]));
        float rot_pos_y=rot_val[1]+( -rot_val[3]*(m_target_pos.x-rot_val[0])+rot_val[2]*(m_target_pos.y -rot_val[1]));

        m_target_pos.x=rot_pos_x;
        m_target_pos.y=rot_pos_y;
        m_pos_defence=m_target_pos;

        //update rotation target (will otherwise be updated later)
        st_pos rel_pos(m_target_pos.x-m_pos.x,m_target_pos.y-m_pos.y);
        m_rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
    }

    //if(m_attack_moving) cout<<"Unit attack move\n";
    //cout<<m_rotation_target<<endl;

    return true;
}

bool unit::push_unit(st_pos push_dir,bool pushing_cancels_movement)
{
    //m_awake=true;
    bool was_moving=is_moving();
    m_have_moved=true;
    m_pos.x+=push_dir.x*_unit_push_speed;
    m_pos.y+=push_dir.y*_unit_push_speed;

    //world edge test
    if(m_pos.x<0) m_pos.x=0;
    else if(m_pos.x>_world_width) m_pos.x=_world_width;
    if(m_pos.y<0) m_pos.y=0;
    else if(m_pos.y>_world_height) m_pos.y=_world_height;

    m_pos_new=m_pos;

    if(pushing_cancels_movement && !was_moving) m_target_pos=m_pos;

    return true;
}

st_pos unit::get_curr_pos(void)
{
    return m_pos;
}

st_pos unit::get_new_pos(void)
{
    return m_pos_new;
}

st_pos unit::get_curr_col_square_pos(void)
{
    return m_col_square_pos;
}

float unit::get_curr_rotation(void)
{
    return m_rotation_curr;
}

bool unit::set_curr_col_square_pos(st_pos new_pos)
{
    m_col_square_pos=new_pos;

    return true;
}

bool unit::accept_new_pos(void)
{
    m_pos=m_pos_new;

    return true;
}

bool unit::is_moving(void)
{
    if( m_pos!=m_target_pos ) return true;

    return false;
}

unit_spec unit::get_spec(void)
{
    return m_spec;
}

bool unit::is_ready_to_attack(void)
{
    if(m_attack_cooldown>0) return false;

    return true;
}

bool unit::take_damage(float damage,float time_delay,bool kill)
{
    //remove all hp
    if(kill)
    {
        //cout<<"kill unit\n";
        m_spec.hp_curr=0;
    }

    //cancel the no-col for formation
    if(m_col_state==active) m_col_state=none;

    //update damage for armour
    damage-=m_spec.armour;
    if(damage<0) damage=0;

    //update current HP
    m_spec.hp_curr-=damage;

    //cout<<"HP: "<<m_spec.hp_curr<<endl;

    return true;
}

bool unit::attack_action_done(void)
{
    //cancel the no-col for formation
    if(m_col_state==active) m_col_state=none;

    m_attack_cooldown=m_spec.attack_speed;
    m_attack_mode_cooldown=0.1;

    return true;
}

bool unit::aim_to_target(float rotation_target,bool special_attack_order_overide)
{
    m_is_idle=false;

    //return true;

    //test if currently moving
    if(m_forced_move && !special_attack_order_overide)
    {
        if(m_rotation_curr!=m_rotation_target) return false;//unit is rotating for moving
        if(m_target_pos!=m_pos) return false;//unit moving towards a target
    }
    else //if(!m_attack_moving)
    {
        //cancel movement if not under attack move order
        m_target_pos=m_pos;

        //cout<<"target move pos reseted\n";
    }

    //calc nearest rotation direction
    bool rotation_reversed=false;
    //if( fabs(rotation_target-m_rotation_curr)>180 ) rotation_reversed=true;

    //update aim
    if(rotation_target>m_rotation_curr)
    {
        if(rotation_reversed)
        {
            m_rotation_curr-=m_spec.speed_rotate;
            if(rotation_target>m_rotation_curr) m_rotation_curr=rotation_target;//rotation complete
        }
        else
        {
            m_rotation_curr+=m_spec.speed_rotate;
            if(rotation_target<m_rotation_curr) m_rotation_curr=rotation_target;//rotation complete
        }
    }
    else if(rotation_target<m_rotation_curr)
    {
        if(rotation_reversed)
        {
            m_rotation_curr+=m_spec.speed_rotate;
            if(rotation_target<m_rotation_curr) m_rotation_curr=rotation_target;//rotation complete
        }
        else
        {
            m_rotation_curr-=m_spec.speed_rotate;
            if(rotation_target>m_rotation_curr) m_rotation_curr=rotation_target;//rotation complete
        }
    }
    //cout<<"rot: "<<m_rotation_curr<<"\ttarget: "<<rotation_target<<"\t"<<m_rotation_target<<endl;
    m_rotation_target=m_rotation_curr;//to avoid counter rotation



    //test if target is within aim
    if( fabs(m_rotation_curr-rotation_target)<m_spec.aim_angle_tolerance ) return true;//aim ok

    return false;//aim not ok
}

bool unit::cancel_movement(void)
{
    m_target_pos=m_pos_defence=m_pos_new=m_pos;
    m_pos_attack_move_target=st_pos(-1,-1);
    m_rotation_target=m_rotation_curr;
    m_is_idle=true;
    m_attack_moving=false;
    m_forced_move=false;
    m_pUnit_enemy_target=0;

    return true;
}

bool unit::force_pos(st_pos pos,float rotation)
{
    m_target_pos=m_pos_defence=m_pos_new=m_pos=pos;
    m_rotation_curr=rotation;

    return true;
}

bool unit::throw_unit(st_pos direction)
{
    m_projectile_mode=true;
    m_projectile_direction=direction;

    return true;
}
