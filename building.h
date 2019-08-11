#ifndef BUILDING_H
#define BUILDING_H

#include <gl/gl.h>
#include "definitions.h"


class building
{
    public:
        building();
        building(const st_pos& pos,int texture);

        bool m_selected;
        float m_hp_curr;

        bool init(const st_pos& pos,int texture);
        bool update(void);
        bool draw(void) ;
        bool is_pos_inside(const st_pos& pos) const;
        bool is_pos_inside_rect(const st_pos& pos_top_left, const st_pos& pos_low_right) const;
        bool is_pos_inside_circle2(st_pos pos,float radius2);
        st_pos get_curr_pos(void);
        bool take_damage(float damage);


    private:

        st_pos m_pos;
        float  m_size;
        int m_texture,m_tex_type;
};

#endif // BUILDING_H
