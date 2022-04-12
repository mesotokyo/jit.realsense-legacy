struct jit_rs_option
{
    rs::option opt;
    double min, max, step, value, def;
    bool supports;
    const char* name;
};

void add_rs_option_attributes(t_class* jit_class);

class jit_rs_options
{
public:
    jit_rs_options();
    double get_value_from_name(const char* str);
    t_jit_err set_value_for_name(rs::device* dev,
                                 const char* str,
                                 double val);
    void update_options(rs::device* dev);

private:
    // options cache
    std::array<jit_rs_option, RS_OPTION_COUNT> options_cache;

    int search_rs_option_index(const char* str);

};
