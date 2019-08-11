#include "particle_engine.h"

particle_engine::particle_engine()
{
    //ctor
}

bool particle_engine::init(void)
{
    m_rand_accuracy=100000;
    m_directed_angle_state_return_chance=0.05;
    m_direction_spread=0.5;
    m_time_left_variation=0.9;

    return true;
}

bool particle_engine::update(void)
{
    for(int part_i=0;part_i<_max_particles;part_i++)
    {
        if(m_arr_particles[part_i].time_left>0)
        {
            //update age
            m_arr_particles[part_i].time_left-=_time_step;
            //update pos
            m_arr_particles[part_i].pos[0]+=(m_arr_particles[part_i].speed[0])*_time_step;
            m_arr_particles[part_i].pos[1]+=(m_arr_particles[part_i].speed[1])*_time_step;
            //update color

        }
    }

    return true;
}

bool particle_engine::draw(int layer)
{
    glPointSize(4);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_POINTS);
    for(int part_i=0;part_i<_max_particles;part_i++)
    {
        if(m_arr_particles[part_i].layer!=layer) continue;

        if(m_arr_particles[part_i].time_left>0)//only draw alive particles
        {
            /*glColor4f(m_arr_particles[part_i].time_left/m_arr_particles[part_i].time_start*m_arr_particles[part_i].color[0],
                      m_arr_particles[part_i].time_left/m_arr_particles[part_i].time_start*m_arr_particles[part_i].color[1],
                      m_arr_particles[part_i].time_left/m_arr_particles[part_i].time_start*m_arr_particles[part_i].color[2]);*/

            glColor4f(m_arr_particles[part_i].color[0],
                      m_arr_particles[part_i].color[1],
                      m_arr_particles[part_i].color[2],
                      m_arr_particles[part_i].time_left/m_arr_particles[part_i].time_start);
            glVertex2f(m_arr_particles[part_i].pos[0],m_arr_particles[part_i].pos[1]);

            //cout<<m_arr_particles[part_i].pos[0]<<", "<<m_arr_particles[part_i].pos[1]<<endl;
        }
    }
    glEnd();
    glPointSize(1);
    glDisable(GL_BLEND);

    return true;
}

bool particle_engine::add_explosion(float pos[2],int numof_part,float force,float life_time,
                                    float color[3],float direction_angle,
                                    int layer)
{
    bool directed_explosion=true;
    if(direction_angle==-1.0) directed_explosion=false;

    //cout<<pos[0]<<", "<<pos[1]<<endl;

    //calc direction x,y
    float direction_focus[2]={0,0};
    float direction_focus_start[2]={0,0};
    float directed_sum=0.0;
    if(directed_explosion)
    {
        direction_focus[0]=cosf((direction_angle-90.0))*0.6;
        direction_focus[1]=sinf((direction_angle-90.0))*0.6;
        directed_sum=fabs(direction_focus[0])+fabs(direction_focus[1]);
    }
    direction_focus_start[0]=direction_focus[0];
    direction_focus_start[1]=direction_focus[1];

    //make n particles with random direction
    for(int new_part=0;new_part<numof_part;new_part++)
    {
        //find free slot
        int free_slot=-1;
        for(int part_i=0;part_i<_max_particles;part_i++)
        {
            if(m_arr_particles[part_i].time_left<=0)
            {
                free_slot=part_i;
                break;
            }
        }
        if(free_slot==-1) return false;//no more free slots

        //set pos
        m_arr_particles[free_slot].pos[0]=pos[0];
        m_arr_particles[free_slot].pos[1]=pos[1];

        //set color
        m_arr_particles[free_slot].color[0]=color[0]+float(rand()%200)/1000.0-0.1;
        m_arr_particles[free_slot].color[1]=color[1]+float(rand()%200)/1000.0-0.1;
        m_arr_particles[free_slot].color[2]=color[2]+float(rand()%200)/1000.0-0.1;

        //set direction

        if(directed_explosion)
        {
            //semi normal distrub. with drift
            float rand_val=float(rand()%1000)/1000.0;// 0.0 - 1.0
                 if(rand_val<0.025) direction_focus[0]-=0.4;
            else if(rand_val<0.05)  direction_focus[0]-=0.3;
            else if(rand_val<0.10)  direction_focus[0]-=0.2;
            else if(rand_val<0.225) direction_focus[0]-=0.1;
            //else if(rand_val<0.50) direction_focus.x+=0.0;
            else if(rand_val<0.725) direction_focus[0]+=0.0;
            else if(rand_val<0.90)  direction_focus[0]+=0.1;
            else if(rand_val<0.95)  direction_focus[0]+=0.2;
            else if(rand_val<0.975) direction_focus[0]+=0.3;
            else                    direction_focus[0]+=0.4;
            //and for y
            rand_val=float(rand()%1000)/1000.0;// 0.0 - 1.0
                 if(rand_val<0.025) direction_focus[1]-=0.4;
            else if(rand_val<0.05)  direction_focus[1]-=0.3;
            else if(rand_val<0.10)  direction_focus[1]-=0.2;
            else if(rand_val<0.225) direction_focus[1]-=0.1;
            //else if(rand_val<0.50) direction_focus.x+=0.0;
            else if(rand_val<0.725) direction_focus[1]+=0.0;
            else if(rand_val<0.90)  direction_focus[1]+=0.1;
            else if(rand_val<0.95)  direction_focus[1]+=0.2;
            else if(rand_val<0.975) direction_focus[1]+=0.3;
            else                    direction_focus[1]+=0.4;

            //without directed_sum
            float direction_x=force*(direction_focus[0]+direction_focus[0]*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_direction_spread );
            float direction_y=force*(direction_focus[1]+direction_focus[1]*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_direction_spread );

            m_arr_particles[free_slot].speed[0]=direction_x;
            m_arr_particles[free_slot].speed[1]=direction_y;

            //chance to return to start direction
            if(rand_val<0.50+m_directed_angle_state_return_chance && rand_val>0.5-m_directed_angle_state_return_chance)
            {
                direction_focus[0]=direction_focus_start[0];
                direction_focus[1]=direction_focus_start[1];
            }
        }
        else//full random
        {
            float direction_x=force*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5);
            float direction_y=force*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5);

            m_arr_particles[free_slot].speed[0]=direction_x;
            m_arr_particles[free_slot].speed[1]=direction_y;
        }

        //no random
        //float direction_x=force*direction_focus.x;
        //float direction_y=force*direction_focus.y;

        //set time left
        m_arr_particles[free_slot].time_start=m_arr_particles[free_slot].time_left=life_time+life_time*
                                  ((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_time_left_variation;

        m_arr_particles[free_slot].layer=layer;
    }

    return true;
}



