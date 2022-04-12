static const constexpr int jit_realsense_num_outlets = 6;

struct jit_rs_streaminfo
{
    long stream;
    long format;
    long rate;
    long dimensions_size;
    std::array<long, 2> dimensions;

    friend bool operator!=(const jit_rs_streaminfo& lhs, const jit_rs_streaminfo& rhs);
    friend bool operator==(const jit_rs_streaminfo& lhs, const jit_rs_streaminfo& rhs);
};

typedef struct _jit_realsense {
    t_object	ob{};
    rs::device* dev{};
    
    long device;
    long out_count;

    std::array<jit_rs_streaminfo, jit_realsense_num_outlets> outputs;

    long device_cache;
    long out_count_cache;
    std::array<jit_rs_streaminfo, jit_realsense_num_outlets> outputs_cache;

    jit_rs_options rs_options;

    void construct();
    void rebuild();
    void rebuild_streams();
    static rs::context& context();
    void cleanup();
} t_jit_realsense;
