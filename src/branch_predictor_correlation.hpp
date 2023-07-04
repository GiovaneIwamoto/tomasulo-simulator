#include <vector>

class branch_predictor_correlation
{
public:
    branch_predictor_correlation(unsigned int m, unsigned int n);
    bool predict_mn(unsigned int pc);
    void mn_update_state(unsigned int pc, bool taken, bool hit);
    float get_predictor_mn_hit_rate();

private:
    unsigned int m, n;
    int max, bms;
    std::vector<int> tabel;
    int c_predictions, c_hits;
};