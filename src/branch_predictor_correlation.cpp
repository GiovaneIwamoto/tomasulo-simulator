#include "branch_predictor_correlation.hpp"

branch_predictor_correlation::branch_predictor_correlation(unsigned int m, unsigned int n) : m(m), n(n)
{
    max = (1 << n) - 1;
    bms = (1 << (n - 1));
    tabel.resize(1 << m, 0);
    c_predictions = 0;
    c_hits = 0;
}

bool branch_predictor_correlation::predict_mn(unsigned int pc)
{
    c_predictions++;
    return (tabel[pc % (1 << m)] & bms);
}

void branch_predictor_correlation::mn_update_state(unsigned int pc, bool taken, bool hit)
{
    int index = pc % (1 << m);
    int temp = tabel[index];

    c_hits += hit;

    if (taken)
    {
        temp = ++temp > max ? max : temp;
    }
    else
    {
        temp = --temp < 0 ? 0 : temp;
    }

    tabel[index] = temp;
}

float branch_predictor_correlation::get_predictor_mn_hit_rate()
{
    return ((float)c_hits / (float)c_predictions) * 100;
}
