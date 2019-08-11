#include "projectile.h"

projectile::projectile()
{
    m_damage_range=0;
}

projectile::projectile(st_pos start,st_pos end,float speed,float damage,float damage_range,int team_fire)
{
    m_pos_start=m_pos_curr=start;
    m_pos_end=end;
    m_damage=damage;
    m_damage_range=damage_range;
    m_team_fire=team_fire;

    //calc move vector
    st_pos rel_pos(end.x-start.x,end.y-start.y);
    float length=rel_pos.length();
    rel_pos/=length;//normalize
    m_speed.x=rel_pos.x*speed;
    m_speed.y=rel_pos.y*speed;

    //calc angle
    m_angle=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
    //cout<<"bullet angle: "<<m_angle<<endl;

    //calc lifetime
    m_life_time=length/speed;

    //std::cout<<"bullet dir: "<<rel_pos.x<<", "<<rel_pos.y<<std::endl;
}

bool projectile::update(void)
{
    m_pos_curr.x+=m_speed.x*_time_step;
    m_pos_curr.y+=m_speed.y*_time_step;

    //test lifetime
    m_life_time-=_time_step;
    if(m_life_time<0) return true;//time out

    //test if done (not required if life time is tested)
    bool end_reached=false;
    if(m_speed.x>=0)//going right
    {
        if(m_pos_curr.x>=m_pos_end.x)
        {
            //test y
            if(m_speed.y>=0)//going down
            {
                if(m_pos_curr.y>=m_pos_end.y) end_reached=true;
            }
            else//going up
            {
                if(m_pos_curr.y<m_pos_end.y) end_reached=true;
            }
        }
    }
    else//going left
    {
        if(m_pos_curr.x<m_pos_end.x)
        {
            //test y
            if(m_speed.y>=0)//going down
            {
                if(m_pos_curr.y>=m_pos_end.y) end_reached=true;
            }
            else//going up
            {
                if(m_pos_curr.y<m_pos_end.y) end_reached=true;
            }
        }
    }

    return end_reached;
}

bool projectile::draw(void)
{
    glPushMatrix();
    glTranslatef(m_pos_curr.x,m_pos_curr.y,0);
    glRotatef(m_angle+90,0,0,1);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    //glVertex2f(m_pos_curr.x,m_pos_curr.y);
    glTexCoord2f(0,0);
    glVertex2f(-3+3,-8);
    glTexCoord2f(0,1);
    glVertex2f(-3+3,8);
    glTexCoord2f(1,1);
    glVertex2f(3+3,8);
    glTexCoord2f(1,0);
    glVertex2f(3+3,-8);
    glEnd();

    glPopMatrix();

    return true;
}
