#include <librealsense/rs.hpp>
#include <vector>
#include <array>
#include <cstdint>
#include "jit.common.h"

#include "rs_options.hpp"
#include "jit.realsense.hpp"


jit_rs_options::jit_rs_options() {}

double jit_rs_options::get_value_from_name(const char* str)
{
    int index = search_rs_option_index(str);
    if (index < 0)
    {
        return -1.0l;
    }
    jit_rs_option& o = options_cache[index];
    if (o.supports)
    {
        return o.value;
    }
    return -1.0l;
}

t_jit_err jit_rs_options::set_value_for_name(rs::device* dev,
                                             const char* str,
                                             double val)
{
    int index = search_rs_option_index(str);
    if (index < 0)
    {
        // if this error occurs, something wrong...
        error("the attribute %s not found.", str);
        return JIT_ERR_NONE;
    }

    jit_rs_option& o = options_cache[index];
    if (!o.supports)
    {
        // the option is not supported, so can't change the value, but no error
        error("the attribute %s is not supported for the device.", str);
        return JIT_ERR_NONE;
    }

    // validate value
    if (val < o.min || o.max < val)
    {
        // the option is not supported, so can't change the value, but no error
        error("the value %f is out of the range of the attribute %s", val, str);
        return JIT_ERR_NONE;
    }     

    // regularize value
    double d = val - o.min;
    int m = (int)(d / o.step);
    double new_value = o.min + o.step * m;

    dev->set_option(o.opt, new_value);
    o.value = dev->get_option(o.opt);

    //post("set %s %d", str, (int)o.value);
    return JIT_ERR_NONE;
}

int jit_rs_options::search_rs_option_index(const char* str)
{
    for (std::size_t i = 0; i < options_cache.size(); i++)
    {
        if (options_cache[i].name == str)
        {
            return (int)i;
        }
        if (strcmp(options_cache[i].name, str) == 0)
        {
            return (int)i;
        }
    }
    return -1;
}

void jit_rs_options::update_options(rs::device* dev)
{
    for (int i = 0; i < RS_OPTION_COUNT; ++i)
    {
        jit_rs_option& o = options_cache[i];
        o.opt = (rs::option)i;

        if (!dev)
        {
            o.supports = false;
            o.value = -1.0l;
            continue;
        }
        try
        {
            o.supports = dev->supports_option(o.opt);
            o.name = rs_option_to_string((rs_option)o.opt);
            if (o.supports)
            {
                dev->get_option_range(o.opt, o.min, o.max, o.step, o.def);
                o.value = dev->get_option(o.opt);
            }
        }
        catch (...)
        {
            error("faild to check options\n");
        }
    }
}

// custom getter
t_jit_err rs_option_get(t_jit_realsense *x, void *attr, long *ac, t_atom **av)
{
    // get option name
    t_symbol* name = (t_symbol*)jit_object_method(attr, _jit_sym_getname);
    double result = x->rs_options.get_value_from_name(name->s_name);

    if (!*ac || !*av)
    {
        // allocate memory
        *ac = 1;
        *av = (t_atom*)jit_getbytes(sizeof(t_atom)*(*ac));
        if (!*av)
        {
            // allocation failed
            *ac = 0;
            return JIT_ERR_OUT_OF_MEM;
        }
    }

    jit_atom_setfloat(*av, result);
    return JIT_ERR_NONE;
}

// custom setter
t_jit_err rs_option_set(t_jit_realsense *x, void *attr, long ac, t_atom *av) {
    // get option name
    t_symbol* name = (t_symbol*)jit_object_method(attr, _jit_sym_getname);
    double v = -1.0l;

    if (!ac || !av)
    {
        error("set %s: invalid pointer", name->s_name);
        return JIT_ERR_INVALID_PTR;
    }
    v = jit_atom_getfloat(av);

    return x->rs_options.set_value_for_name(x->dev, name->s_name, v);
}

// function to add attributes to jit class
void add_rs_option_attributes(t_class* jit_class) {
    // add RS_OPTION as attribute
    for (int i = 0; i < RS_OPTION_COUNT; ++i)
    {
        const char* option_string = rs_option_to_string((rs_option)i);
        t_jit_object *attr = (t_jit_object*)jit_object_new(
            _jit_sym_jit_attr_offset,
            option_string,
            _jit_sym_float32,
            JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW,
            (method)rs_option_get,
            (method)rs_option_set,
            NULL);
        jit_class_addattr(jit_class, attr);
    }
}
