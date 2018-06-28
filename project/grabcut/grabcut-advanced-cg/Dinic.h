#ifndef GRABCUT_DINIC_H
#define GRABCUT_DINIC_H

const int MAXN = 251000, MAXM = 2000000;
const double eps = 1e-5;
#define isZero(x) ((x)>-eps&&(x)<eps)

struct Dinic {
public:
    struct edge {
        int x, y;//两个顶点
        double c;//容量
        double f;//当前流量
        edge *next, *back;//下一条边，反向边
        edge(int x, int y, int c, edge *next) : x(x), y(y), c(c), f(.0), next(next), back(0) {}

        void *operator new(size_t, void *p) { return p; }
    } *E[MAXN], *data;//E[i]保存顶点i的边表
    char storage[2 * MAXM * sizeof(edge)];
    int S, T;//源、汇

    int Q[MAXN];//DFS用到的queue
    int D[MAXN];//距离标号，-1表示不可达
    void DFS();

    edge *cur[MAXN];//当前弧
    edge *path[MAXN];//当前找到的增广路
    double flow();

    int cut(int *s);

    bool sourceSet(int id);

    void init(int _S, int _T);

    void addEdge(int x, int y, double w);
};


#endif
