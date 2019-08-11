#ifndef KEY_REROUTE_H
#define KEY_REROUTE_H

#define _key_count 256

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

enum key_table
{
    key_ctrl=17,
    key_shift=16,
    key_caps=20,
    key_tab=9,
    key_tilde=220,
    key_enter=13,
    key_space=32,
    key_backspace=8,
    key_esc=27,

    key_np_0=96,
    key_np_1,
    key_np_2,
    key_np_3,
    key_np_4,
    key_np_5,
    key_np_6,
    key_np_7,
    key_np_8,
    key_np_9,
    key_np_mult=106,
    key_np_add=107,
    key_np_subs=109,
    key_np_dev=111,
    key_np_dot=110,

    key_pgup=33,
    key_pgdown,
    key_end,
    key_home,
    key_ins=45,
    key_del=46,

    key_left=37,
    key_up,
    key_right,
    key_down,

    key_0=48,
    key_1,
    key_2,
    key_3,
    key_4,
    key_5,
    key_6,
    key_7,
    key_8,
    key_9,

    key_a=65,
    key_b,
    key_c,
    key_d,
    key_e,
    key_f,
    key_g,
    key_h,
    key_i,
    key_j,
    key_k,
    key_l,
    key_m,
    key_n,
    key_o,
    key_p,
    key_q,
    key_r,
    key_s,
    key_t,
    key_u,
    key_v,
    key_w,
    key_x,
    key_y,
    key_z,
};

struct st_key_rule
{
    st_key_rule()
    {
        input=0;
        output=0;
    }

    st_key_rule(int _input,int _output)
    {
        input=_input;
        output=_output;
    }

    int input;
    int output;
};

class key_reroute
{
    public:
        key_reroute();

        //initialize with sending the pointer to the array that keeps the key state information
        bool init(bool* pKeys_real,bool* pKeys_translated);
        bool update(void);

        bool set_key_translation(int input_key,int output_key);
        bool load_key_translation_from_file(string file_name="key_config.dat");
        bool save_key_translation_to_file(string file_name="key_config.dat");
        bool reset_key_translation(void);

        bool report_status;

    private:

        bool* m_pKeys_real;
        bool* m_pKeys_translated;

        vector<st_key_rule> m_vec_key_rules;
};

#endif // KEY_REROUTE_H
