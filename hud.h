#ifndef HUD_H
#define HUD_H

#include <iostream>
#include <gl/gl.h>
#include "definitions.h"
#include "display.h"

using namespace std;

class hud
{
    public:
        hud();

        bool init(int texture,int screen_size[2],int font_texture[3],int tex_hud_health);
        bool update(float player_health_rel);
        bool draw(void);

        bool add_score(int score_addition);
        bool end(bool won);

        int  m_score;
        bool m_won,m_end;


    private:

        float m_player_health_rel;
        int   m_texture,m_tex_hud_health;
        int   m_tex_font[3];
        int   m_screen_size[2];
        float m_noise_prog;

        display m_disp_score;
        display m_disp_message_won;
        display m_disp_message_lost;

};

#endif // HUD_H
