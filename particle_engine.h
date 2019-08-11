#ifndef PARTICLE_ENGINE_H
#define PARTICLE_ENGINE_H

#include <iostream>
#include <gl/gl.h>
#include <stdlib.h>
#include <math.h>
#include "definitions.h"

#define _max_particles 500

using namespace std;

struct st_particle
{
    st_particle()
    {
        pos[0]=0;
        pos[1]=0;
        speed[0]=0;
        speed[1]=0;
        time_start=1;
        time_left=0;
        color[0]=1;
        color[1]=1;
        color[2]=1;
        layer=0;
    }

    st_particle(float _pos[2],float _speed[2],float _time_start,float _color[3])
    {
        pos[0]=_pos[0];
        pos[1]=_pos[1];
        speed[0]=_speed[0];
        speed[1]=_speed[1];
        color[0]=_color[0];
        color[1]=_color[1];
        color[2]=_color[2];
        time_start=_time_start;
        time_left=time_start;
        layer=0;
    }

    float pos[2];
    float speed[2];
    float time_start;
    float time_left;
    float color[3];
    int   layer;
};

class particle_engine
{
    public:
        particle_engine();

        bool init(void);
        bool update(void);
        bool draw(int layer=0);
        bool add_explosion(float pos[2],int numof_part,float force,float life_time,
                           float color[3],float direction_angle=-1,
                           int layer=0);

    private:

        int   m_rand_accuracy;
        float m_directed_angle_state_return_chance,m_direction_spread,m_time_left_variation;
        st_particle m_arr_particles[_max_particles];
};

#endif // PARTICLE_ENGINE_H
