# TOMASULO SIMULATOR

### **OVERVIEW**

Event-driven simulation of the Tomasulo Algorithm using the C++ interface SystemC, with a complete Graphical User Interface. The purpose of this project is to ease the teaching of this algorithm, deemed extremely important to the understanding of out-of-order execution on modern microprocessors.

[![Icons](https://skillicons.dev/icons?i=cpp,py,git&theme=light)](https://skillicons.dev)

> [!CAUTION]
> Originally from https://github.com/lucasreis1/TFSim

---

### **ABSTRACT**

Practical work in Computer Architecture II with the objective of implementing, evaluating, and validating TFSim dynamic branch predictors. Extend the simulator with new significant program sections. Implement a branch prediction scheme based on correlation (m, n). Compare the performance of the implemented branch predictors in TFSim, including the correlation scheme.

---

### **INTRODUCTION**

This work addresses the implementation, evaluation, and validation of dynamic branch predictors in TFSim, a program execution simulator. The specific objectives include extending TFSim with significant program sections for prediction scheme evaluation, implementing the correlation-based branch prediction scheme (m, n), and comparing the performance of the implemented predictors. The study aims to enhance processor performance in dealing with conditional branches. The obtained results will be valuable for improving the efficiency of computational systems.

---

### **METHODOLOGY**

In the implementation of the correlation-based branch predictor, we added a predictor table represented as a vector (std::vector< int > table). Each entry in the table is a saturation counter that stores the history of previous branches. To predict the branch outcome for an instruction, we query the predictor table using the provided index. Based on the saturation counter's value, we decide whether the branch will be taken or not taken.

> [!IMPORTANT]
> To update the state of the predictor, we use the same index to access the corresponding saturation counter in the table and update it based on the actual branch outcome. If the branch was taken, we increment the saturation counter; otherwise, we decrement it.

Regarding the code sections used to evaluate the prediction schemes, they were created based on examples found in the "benchmark" folder of the repository. We selected code sections that covered different branch scenarios, such as many branches, branches with different patterns (upward, downward), and nested branches. These code sections were designed to exercise the branch predictor and evaluate its effectiveness in correctly predicting the behavior of conditional branches.

For the comparison between the two implemented prediction models, we executed the code sections using both the correlation-based branch predictor and the pre-existing predictor. We analyzed and compared the performance of these predictors in terms of prediction accuracy. This comparison allowed us to evaluate the efficiency of the new prediction scheme in relation to the existing model.

> [!NOTE]
> By carrying out these implementation steps, we were able to extend TFSim with new program sections, implement the correlation-based branch predictor, and perform a performance comparison between the two prediction models. These actions allowed us to achieve the specific objectives proposed for this work

---

### **AUTHORS**

- Giovane Hashinokuti Iwamoto - Computer Science student at UFMS - Brazil - MS
- Matheus Tavares Guerson - Computer Science student at UFMS - Brazil - MS
- Rafael Eitaro Oshiro - Computer Science student at UFMS - Brazil - MS
