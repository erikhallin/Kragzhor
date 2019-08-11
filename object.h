#ifndef OBJECT_H
#define OBJECT_H

#include <gl/gl.h>
#include "definitions.h"

class object
{
    public:
        object();
        object(const st_pos& pos,int texture);

        float  m_rotation_curr;
        bool   m_projectile_mode,m_in_hands;
        st_pos m_projectile_direction;

        bool init(const st_pos& pos,int texture);
        bool update(void);
        bool draw(void) ;
        bool is_pos_inside(const st_pos& pos) const;
        bool is_pos_inside_rect(const st_pos& pos_top_left, const st_pos& pos_low_right) const;
        bool is_pos_inside_circle2(st_pos pos,float radius2);
        st_pos get_curr_pos(void);
        bool take_damage(float damage,bool kill=false);
        bool force_pos(st_pos pos,float rotation);
        bool throw_object(st_pos direction);

    private:

        st_pos m_pos;
        float  m_size,m_hp_curr;
        int    m_texture,m_tex_type;
};

#endif // OBJECT_H
