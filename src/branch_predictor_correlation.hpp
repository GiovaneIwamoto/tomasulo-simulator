#include <iostream>
#include <vector>

class branch_predictor_correlation
{
private:
    std::vector<int> table;
    unsigned int m, n;
    unsigned int c_predictions;
    int max;
    int c_hits;

public:
    branch_predictor_correlation(unsigned int m, unsigned int n);
    bool predict_mn(unsigned int pc);
    void mn_update_state(unsigned int pc, bool taken, bool hit);
    float get_predictor_mn_hit_rate();
};