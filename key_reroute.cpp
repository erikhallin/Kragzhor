#include "key_reroute.h"

key_reroute::key_reroute()
{
    report_status=false;
}

bool key_reroute::init(bool* pKeys_real,bool* pKeys_translated)
{
    m_pKeys_real=pKeys_real;
    m_pKeys_translated=pKeys_translated;

    if(report_status)
    {
        cout<<"KEY REROUTE: Initialized\n";
    }

    return true;
}

bool key_reroute::update(void)
{
    //copy key state from the real keys
    for(int key_i=0;key_i<_key_count;key_i++)
    {
        m_pKeys_translated[key_i]=m_pKeys_real[key_i];
    }

    //translate keys bassed of the rules list
    for(unsigned int rule_i=0;rule_i<m_vec_key_rules.size();rule_i++)
    {
        //translate key
        if(m_pKeys_real[ m_vec_key_rules[rule_i].input ])
        {
            m_pKeys_translated[ m_vec_key_rules[rule_i].output ]=true;

            //real input key be false if no other rules are involved
            bool turn_off_real_input_key=true;
            for(unsigned int rule_b=0;rule_b<m_vec_key_rules.size();rule_b++)
            {
                if(m_vec_key_rules[rule_b].output==m_vec_key_rules[rule_i].input &&
                   m_pKeys_real[ m_vec_key_rules[rule_b].input ] )
                {
                    turn_off_real_input_key=false;
                    break;
                }
            }
            if(turn_off_real_input_key)
             m_pKeys_translated[ m_vec_key_rules[rule_i].input ]=false;
        }
    }

    return true;
}

bool key_reroute::set_key_translation(int input_key,int output_key)
{
    m_vec_key_rules.push_back( st_key_rule(input_key,output_key) );

    if(report_status)
    {
        cout<<"KEY REROUTE: New key rule added: "<<input_key<<", to "<<output_key<<endl;
    }

    return true;
}

bool key_reroute::load_key_translation_from_file(string file_name)
{
    //test if file exists
    ifstream file(file_name.c_str());
    if(file==0)
    {
        cout<<"KEY REROUTE: Could not load key rules from file: "<<file_name<<endl;
        return false;
    }

    string line,word;
    getline(file,line);//skip first line
    //[key input] [key output]
    while(getline(file,line))
    {
        stringstream ss(line);
        ss>>word;
        int input=(int)atof(word.c_str());
        ss>>word;
        int output=(int)atof(word.c_str());

        if(input>-1 && input<_key_count && output>-1 && output<_key_count)
        {
            //search for copies
            bool old_rule=false;
            for(unsigned int rule_i=0;rule_i<m_vec_key_rules.size();rule_i++)
            {
                if(m_vec_key_rules[rule_i].input==input && m_vec_key_rules[rule_i].output==output)
                {
                    old_rule=true;
                    break;
                }
            }
            //save rule
            if(!old_rule) m_vec_key_rules.push_back( st_key_rule(input,output) );
        }
    }

    return true;
}

bool key_reroute::save_key_translation_to_file(string file_name)
{
    if(m_vec_key_rules.empty()) return true;

    ofstream file(file_name.c_str());
    if(file==0)
    {
        cout<<"KEY REROUTE: Could not load key rules from file: "<<file_name<<endl;
        return false;
    }

    file<<"Key configuration: Input_id Output_id\n";
    for(unsigned int rule_i=0;rule_i<m_vec_key_rules.size();rule_i++)
    {
        file<<m_vec_key_rules[rule_i].input<<" "<<m_vec_key_rules[rule_i].output<<endl;
    }

    return true;
}

bool key_reroute::reset_key_translation(void)
{
    m_vec_key_rules.clear();

    if(report_status)
    {
        cout<<"KEY REROUTE: Key translation reset\n";
    }

    return true;
}
