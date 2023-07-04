#include <cmath>
#include "branch_predictor_correlation.hpp"

branch_predictor_correlation::branch_predictor_correlation(unsigned int m, unsigned int n) : m(m), n(n)
{
    c_predictions = 0;
    c_hits = 0;

    max = (1 << n) - 1;

    table.resize((1 << m), 0);
}

bool branch_predictor_correlation::predict_mn(unsigned int pc)
{
    c_predictions++;

    return table[pc % (1 << m)] & (1 << (n - 1));
}

void branch_predictor_correlation::mn_update_state(unsigned int pc, bool taken, bool hit)
{
    c_hits += hit;

    int aux = table[pc % (1 << m)];

    if (taken)
    {
        aux += (aux < max);
    }
    else
    {
        aux -= (aux > 0);
    }

    table[pc % (1 << m)] = aux;
}

float branch_predictor_correlation::get_predictor_mn_hit_rate()
{
    return ((float)c_hits / (float)c_predictions) * 100;
}
