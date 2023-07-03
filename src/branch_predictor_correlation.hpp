#include <vector>

class branch_predictor_correlation
{
public:
    branch_predictor_correlation(unsigned int m, unsigned int n);
    void mn_update_state(unsigned int pc, bool taken, bool hit);
    float get_predictor_mn_hit_rate();
    bool predict_mn(unsigned int pc);

private:
    unsigned int m_size;
    unsigned int n_bits;

    unsigned int history_size;
    unsigned int counter_max;

    std::vector<unsigned int> history_table;
    unsigned int c_predictions;
    unsigned int c_hits;
};