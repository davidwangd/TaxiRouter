#ifndef __DATA_H
#define __DATA_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <climits>

struct edge{
    int to, dis;
    edge(int to = 0, int dis = 0): to(to), dis(dis){}
};

struct vertex{
    double x, y;
    int id;
    vertex(double x = 0, double y = 0, int id = 0):
        x(x), y(y), id(id) {}
};

class graph{
private:
    int *start;
    edge *edges;
    int n, m;
    vertex *nodes;
public:
    graph(const char *edgefile, const char *nodefile);
    ~graph(){
        delete[] start;
        delete[] edges;
    }
public:
    std::pair<double,double> find_pos(int x) const {
        return std::make_pair(nodes[x].x, nodes[x].y);
    }
    // 利用data 参数实现一个线程安全的find_path函数，使用A*算法在source和target质检寻找一条最短路
    // 应用必须保证给定的size足够大。否则失败
    int find_path(int source, int target, int *data, int magic, int threshold = INT_MAX) const;
    
    // 找到给定起点中相邻的内容,实现线程安全的临近选择，依然需要void *中的数据足够多的。
    int find_adjanct(int source, int *data, int magic, int threshold = 10000) const;

    // 计算两个点之间的最短可能距离，结果取上整数
    int mindist(int x, int y) const;
    
    int vertCnt() const{ return n; }
    int edgeCnt() const{ return m; }
};

#endif /* !__DATA_H */