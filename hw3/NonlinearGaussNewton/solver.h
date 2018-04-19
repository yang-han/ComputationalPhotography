#include "hw3_gn.h"

class Solver4251:public GaussNewtonSolver{
    public:
    Solver4251(){}
    // virtual ~GaussNewtonSolver(){}

	virtual double solve(
		ResidualFunction *f, // 目标函数
		double *X,           // 输入作为初值，输出作为结果
		GaussNewtonParams param = GaussNewtonParams(), // 优化参数
		GaussNewtonReport *report = nullptr // 优化结果报告
		)
    {
        return 0;
    }
};