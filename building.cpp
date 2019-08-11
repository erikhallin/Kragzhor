#include "building.h"

building::building()
{
    //ctor
}

building::building(const st_pos& pos,int texture)
{
    init(pos,texture);
}

bool building::init(const st_pos& pos,int texture)
{
	m_pos=pos;
	m_size=_building_size;
	m_selected=false;
	m_hp_curr=_building_hp;
	m_texture=texture;
	m_tex_type=0;

	return true;
}

bool building::update(void)
{
    if(m_hp_curr<=0) return false;

	return true;
}

bool building::draw(void)
{
	if(m_selected)
	{
	    glColor3f(1.0, 1.0, 0.7);
	    m_selected=false;
	}
	else glColor3f(0.8, 0.8, 0.0);

	glPushMatrix();
    glTranslatef(m_pos.x,m_pos.y,0);

	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f( (float)m_tex_type*92.0/276.0 ,0.0 );
	glVertex2f(-46,-46);
	glTexCoord2f( (float)m_tex_type*92.0/276.0 ,1.0 );
	glVertex2f(-46,46);
	glTexCoord2f( (float)m_tex_type*92.0/276.0+92.0/276.0 ,1.0 );
	glVertex2f(46,46);
	glTexCoord2f( (float)m_tex_type*92.0/276.0+92.0/276.0 ,0.0 );
	glVertex2f(46,-46);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glPopMatrix();

	return true;
}

bool building::is_pos_inside(const st_pos& pos) const
{
	if (m_pos.x + m_size + _unit_size > pos.x && m_pos.x - m_size - _unit_size < pos.x &&
		m_pos.y + m_size + _unit_size > pos.y && m_pos.y - m_size - _unit_size < pos.y)
	{
		return true;
	}

	return false;
}

bool building::is_pos_inside_rect(const st_pos& pos_top_left,const st_pos& pos_low_right) const
{
	if (pos_top_left.x < m_pos.x && pos_low_right.x > m_pos.x &&
		pos_top_left.y < m_pos.y && pos_low_right.y > m_pos.y)
	{
		return true;
	}

	return false;
}

bool building::is_pos_inside_circle2(st_pos pos,float radius2)
{
    return (m_pos.distance2(pos)<radius2);
}

st_pos building::get_curr_pos(void)
{
	return m_pos;
}

bool building::take_damage(float damage)
{
    //update current HP
    m_hp_curr-=damage;

    if(m_hp_curr/_building_hp>=0.50) m_tex_type=0;
    else m_tex_type=1;
    //if(m_hp_curr/_building_hp<0.67) m_tex_type=1;
    //if(m_hp_curr/_building_hp<0.33) m_tex_type=2;

    return true;
}

