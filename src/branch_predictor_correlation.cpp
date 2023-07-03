#include "branch_predictor_correlation.hpp"

branch_predictor_correlation::branch_predictor_correlation(unsigned int m, unsigned int n) : m_size(m), n_bits(n)
{
    history_size = 1 << m_size;
    counter_max = (1 << n_bits) - 1;
    history_table.resize(history_size, 0);
    c_predictions = 0;
    c_hits = 0;
}

bool branch_predictor_correlation::predict_mn(unsigned int pc)

{
    unsigned int history = history_table[pc % history_size];
    unsigned int counter = history >> (m_size - n_bits);
    bool prediction = (counter <= (counter_max >> 1));
    c_predictions++;
    return prediction;
}

void branch_predictor_correlation::mn_update_state(unsigned int pc, bool taken, bool hit)
{
    c_hits += hit;
    unsigned int history = history_table[pc % history_size];
    history = (history << 1) | taken;
    history_table[pc % history_size] = history;
    unsigned int counter_max_local = counter_max >> 1;

    unsigned int counter_min_local = counter_max_local + 1;
    unsigned int counter = history >> (m_size - n_bits);
    if (taken && counter < counter_max_local)
    {
        counter++;
    }
    else if (!taken && counter > counter_min_local)
    {
        counter--;
    }

    history = (history & ((1 << (m_size - n_bits)) - 1)) | (counter << (m_size - n_bits));
    history_table[pc % history_size] = history;
}

float branch_predictor_correlation::get_predictor_mn_hit_rate()
{

    if (c_predictions == 0)
    {
        return 0.0;
    }

    return static_cast<float>(c_hits) / static_cast<float>(c_predictions) * 100.0;
}
