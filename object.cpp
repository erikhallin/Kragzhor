#include "object.h"

object::object()
{
    //ctor
}

object::object(const st_pos& pos,int texture)
{
    init(pos,texture);
}

bool object::init(const st_pos& pos,int texture)
{
	m_pos=pos;
	m_size=_object_size;
	m_hp_curr=_object_hp;
	m_rotation_curr=0;
	m_projectile_mode=m_in_hands=false;
	m_texture=texture;
	m_tex_type=rand()%4;

	return true;
}

bool object::update(void)
{
    if(m_hp_curr<=0) return false;

    //projectile mode, thrown
    if(m_projectile_mode)
    {
        m_pos.x+=m_projectile_direction.x*_object_projectile_speed;
        m_pos.y+=m_projectile_direction.y*_object_projectile_speed;

        //damp speed
        if(m_projectile_direction.x>0)
        {
            m_projectile_direction.x-=_object_throw_damping;
            if(m_projectile_direction.x<0) m_projectile_direction.x=0;
        }
        else
        {
            m_projectile_direction.x+=_object_throw_damping;
            if(m_projectile_direction.x>0) m_projectile_direction.x=0;
        }
        if(m_projectile_direction.y>0)
        {
            m_projectile_direction.y-=_object_throw_damping;
            if(m_projectile_direction.y<0) m_projectile_direction.y=0;
        }
        else
        {
            m_projectile_direction.y+=_object_throw_damping;
            if(m_projectile_direction.y>0) m_projectile_direction.y=0;
        }

        //stop test
        if(m_projectile_direction.length2()<_object_throw_speed_limit)
        {
            m_projectile_mode=false;
            m_in_hands=false;
        }


        //rotate in air
        m_rotation_curr+=_object_rotation_speed*_time_step*m_projectile_direction.length2();
        if(m_rotation_curr>=360) m_rotation_curr-=360;

        if(m_pos.x<0 || m_pos.x>_world_width ||
           m_pos.y<0 || m_pos.y>_world_height ) m_hp_curr=0;

        return true;
    }

	return true;
}

bool object::draw(void)
{
    glPushMatrix();
    glTranslatef(m_pos.x,m_pos.y,0);
    glRotatef(m_rotation_curr,0,0,1);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f( (float)m_tex_type*34.0/136.0 ,0.0 );
	glVertex2f(-m_size,-m_size);
	glTexCoord2f( (float)m_tex_type*34.0/136.0 ,1.0 );
	glVertex2f(-m_size,m_size);
	glTexCoord2f( (float)m_tex_type*34.0/136.0+34.0/136.0 ,1.0 );
	glVertex2f(m_size,m_size);
	glTexCoord2f( (float)m_tex_type*34.0/136.0+34.0/136.0 ,0.0 );
	glVertex2f(m_size,-m_size);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glPopMatrix();

	return true;
}

bool object::is_pos_inside(const st_pos& pos) const
{
	if(m_in_hands) return false;//disable col

	if (m_pos.x + m_size + _unit_size > pos.x && m_pos.x - m_size - _unit_size < pos.x &&
		m_pos.y + m_size + _unit_size > pos.y && m_pos.y - m_size - _unit_size < pos.y)
	{
		return true;
	}

	return false;
}

bool object::is_pos_inside_rect(const st_pos& pos_top_left,const st_pos& pos_low_right) const
{
	if(m_in_hands) return false;//disable col

	if (pos_top_left.x < m_pos.x && pos_low_right.x > m_pos.x &&
		pos_top_left.y < m_pos.y && pos_low_right.y > m_pos.y)
	{
		return true;
	}

	return false;
}

bool object::is_pos_inside_circle2(st_pos pos,float radius2)
{
    if(m_in_hands) return false;//disable col

    return (m_pos.distance2(pos)<radius2);
}

st_pos object::get_curr_pos(void)
{
	return m_pos;
}

bool object::take_damage(float damage,bool kill)
{
    if(kill)
    {
        m_hp_curr=0;
    }

    //update current HP
    m_hp_curr-=damage;

    return true;
}

bool object::force_pos(st_pos pos,float rotation)
{
    m_pos=pos;
    m_rotation_curr=rotation;

    return true;
}

bool object::throw_object(st_pos direction)
{
    m_projectile_mode=true;
    m_projectile_direction=direction;

    return true;
}
