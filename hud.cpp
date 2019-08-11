#include "hud.h"

hud::hud()
{
    //ctor
}

bool hud::init(int texture,int screen_size[2],int font_texture[3],int tex_hud_health)
{
    m_player_health_rel=1;
    m_score=0;
    m_texture=texture;
    m_tex_hud_health=tex_hud_health;
    m_screen_size[0]=screen_size[0];
    m_screen_size[1]=screen_size[1];
    m_tex_font[0]=font_texture[0];
    m_tex_font[1]=font_texture[1];
    m_tex_font[2]=font_texture[2];
    m_noise_prog=0;
    m_won=false;
    m_end=false;

    //init score display
    int disp_x=m_screen_size[0]*0.65;
    int disp_y=m_screen_size[1]*0.06;
    int disp_width=m_screen_size[0]*0.3;
    int disp_height=m_screen_size[1]*0.05;
    m_disp_score=display(disp_x,disp_y,disp_width,disp_height,10,m_tex_font,0);

    disp_x=m_screen_size[0]*0.33;
    disp_y=m_screen_size[1]*0.90;
    disp_width=m_screen_size[0]*0.5;
    disp_height=m_screen_size[1]*0.04;
    m_disp_message_won=display(disp_x,disp_y,disp_width,disp_height,27,m_tex_font, "THE VILLAGE HAS BEEN SLAIN");
    m_disp_message_won.setting_flags(false);
    m_disp_message_lost=display(disp_x,disp_y,disp_width,disp_height,27,m_tex_font,"  KRAGZHOR HAS BEEN SLAIN ");
    m_disp_message_lost.setting_flags(false);

    return true;
}

bool hud::update(float player_health_rel)
{
    m_player_health_rel=player_health_rel;
    m_noise_prog+=_time_step*0.01;
    if(m_noise_prog>=1.0) m_noise_prog-=1;

    return true;
}

bool hud::draw(void)
{
    glPushMatrix();

    //m_disp_message_won.draw_display();
    //m_disp_message_lost.draw_display();
    //m_disp_score.draw_display();

    if(m_end)
    {
        //draw text
        if(m_won)
         m_disp_message_won.draw_display();
        else
         m_disp_message_lost.draw_display();

        //draw score
        m_disp_score.draw_display();
    }
    else
    {
        //draw health bar
        float bar_height=(float)m_screen_size[1]*0.045;
        float bar_width =(float)m_screen_size[0]*0.294;
        float bar_pos_x =(float)m_screen_size[0]*0.5-bar_width*0.5;
        float bar_pos_y =(float)m_screen_size[1]*0.9;
        //background
        /*glColor3f(0.2,0.2,0.2);
        glBegin(GL_QUADS);
        glVertex2f(bar_pos_x,bar_pos_y);
        glVertex2f(bar_pos_x,bar_pos_y+bar_height);
        glVertex2f(bar_pos_x+bar_width,bar_pos_y+bar_height);
        glVertex2f(bar_pos_x+bar_width,bar_pos_y);
        glEnd();*/

        //health
        float tex_x=0.25+0.25*cosf(m_noise_prog*3.0*_pi);
        float tex_y=0.25+0.25*sinf(m_noise_prog*6.0*_pi);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,m_texture);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor3f(0.7,0.3,0.3);
        glBegin(GL_QUADS);
        glTexCoord2f(tex_x*(m_player_health_rel),
                     tex_y);
        glVertex2f(bar_pos_x+(1.0-m_player_health_rel)*bar_width*0.5,
                   bar_pos_y);
        glTexCoord2f(tex_x*(m_player_health_rel),
                     tex_y+0.5);
        glVertex2f(bar_pos_x+(1.0-m_player_health_rel)*bar_width*0.5,
                   bar_pos_y+bar_height);
        glTexCoord2f(tex_x*(m_player_health_rel)+0.5*(m_player_health_rel),
                     tex_y+0.5);
        glVertex2f(bar_pos_x+(1.0+m_player_health_rel)*bar_width*0.5,
                   bar_pos_y+bar_height);
        glTexCoord2f(tex_x*(m_player_health_rel)+0.5*(m_player_health_rel),
                     tex_y);
        glVertex2f(bar_pos_x+(1.0+m_player_health_rel)*bar_width*0.5,
                   bar_pos_y);
        glEnd();

        //bar
        bar_height=(float)m_screen_size[1]*0.05;
        bar_width =(float)m_screen_size[0]*0.3;
        bar_pos_x =(float)m_screen_size[0]*0.5-bar_width*0.5;
        bar_pos_y =(float)m_screen_size[1]*0.9;
        glBindTexture(GL_TEXTURE_2D,m_tex_hud_health);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor3f(1.0,1.0,1.0);
        glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex2f(bar_pos_x, bar_pos_y);
        glTexCoord2f(0,1);
        glVertex2f(bar_pos_x, bar_pos_y+bar_height);
        glTexCoord2f(1,1);
        glVertex2f(bar_pos_x+bar_width, bar_pos_y+bar_height);
        glTexCoord2f(1,0);
        glVertex2f(bar_pos_x+bar_width, bar_pos_y);
        glEnd();


        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        //draw score
        m_disp_score.draw_display();
    }

    glPopMatrix();

    /*glBegin(GL_QUADS);
    glVertex2f(0,0);
    glVertex2f(0,0+m_screen_size[1]);
    glVertex2f(0+m_screen_size[0],0+m_screen_size[1]);
    glVertex2f(0+m_screen_size[0],0);
    glEnd();*/

    return true;
}

bool hud::add_score(int score_addition)
{
    m_score+=score_addition;

    //update display
    m_disp_score.set_value(m_score);

    return true;
}

bool hud::end(bool won)
{
    m_end=true;
    m_won=won;

    return true;
}
