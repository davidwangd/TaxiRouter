#include "data.h"
#include <queue>
#include <deque>
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace std;

class cmp{
    const int *a, *b;
public:
    cmp(const int *a){
        this->a = a;
    }
    int operator()(const int &x, const int &y) const{
        return a[x] < a[y] ;
    }
};

int graph::find_adjanct(int source, int *data, int magic, int threshold) const {
    int *vis = data;
    int *dis = vis + n;
    cmp now(dis);

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> Q;

    dis[source] = 0;
    vis[source] = magic;
    Q.push(make_pair(0, source));

    while (!Q.empty()){
        while (!Q.empty() && dis[Q.top().second] != Q.top().first) Q.pop();
        if (Q.empty()) break;

        int x = Q.top().second;
        Q.pop();

        if (dis[x] >= threshold) break;

        for (int i = start[x]; i < start[x+1]; i++){
            int to = edges[i].to;
            int di = dis[x] + edges[i].dis;
            if (vis[to] != magic){
                vis[to] = magic;
                dis[to] = di; 
                Q.push(make_pair(di, to));
            }else if (dis[to] > di){
                dis[to] = di; 
                Q.push(make_pair(di, to));
            }
        }
    }
}
// https://segmentfault.com/a/1190000013922206
// 计算经纬度公式
const double pi = acos(-1);
const double R = 6378137.00;

inline double sqr(double x){ return x * x; }

int graph::mindist(int x, int y) const {
    double lung1 = nodes[x].x, lat1 = nodes[x].y;
    double lung2 = nodes[y].x, lat2 = nodes[x].y;
    double a = lat1 - lat2, b = lung1 - lung2;

    return (int)ceil(2 * asin(sqrt( 
                    sqr(sin(a/2))+cos(lat1)*cos(lat2)*sqr(sin(b/2))
               )) * R);
}

struct Reader{ 
    FILE *file;
    void setFile(FILE *file){
        s = t = NULL;
        this->file = file;
    }
    static const int IN_LEN = 1 << 20;    
    char buf[IN_LEN], *s, *t;
    char _getchar(){
        return (s==t)&&(t=(s=buf)+fread(buf,1,IN_LEN,file)),s==t?EOF:*s++;
    }
    int _getint(char &ch, double &v){
        int ret = 0, flag = 1;
        for (ch = _getchar(); ch < '0' || ch > '9'; ch = _getchar())
            if (ch == '-') flag = -flag;
        for (;'0' <= ch && ch <= '9'; ch = _getchar()){
            ret = ret * 10 + ch - '0'; v *= 10;
        }
        return ret * flag;
    }

    int readInt(){
        char ch; double x;
        return _getint(ch, x);
    }

    double readDouble(){
        char ch; double v;
        double cur = _getint(ch, v);
        if (ch == '.'){
            v = 1;
            double p = _getint(ch, v);
            cur += p / v;
        }
        return cur;
    }
}reader;

int graph::find_path(int source, int target, int *data, int magic, int threshold) const {
    int *ret = data;
    int *visit = ret + n;
    int *g = visit + n;
    int *h = g + n;
    int *pre = h + n;

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> q;
    
    visit[source] = magic;
    // g -> mindist
    g[source] = mindist(source, target);
    // h -> distance
    h[source] = 0;
    pre[source] = -1;

    q.push(make_pair(mindist(source, target), source));
    int flag = 0;
    while (!q.empty()){
        while (!q.empty() && q.top().first != g[q.top().second] + h[q.top().second]) q.pop();
        if (q.empty()) break;

        int x = q.top().second; q.pop();

        //fprintf(stderr, "%d, %d\n", x, h[x] + g[x]);

        if (x == target){
            flag = 1;
            break;
        }
        if (h[x] + g[x] > threshold) return -1;

        for (int i = start[x]; i < start[x+1]; i++){
            int to = edges[i].to, d = h[x] + edges[i].dis;
            if (visit[to] != magic || h[to] > d){
                h[to] = d;
                pre[to] = x;
                if (visit[to] != magic){
                    visit[to] = magic;
                    g[to] = mindist(to, target);
                }
                q.push(make_pair(h[to] + g[to], to));
            }
        }
    }
    if (flag){
        int cnt = 0;
        for (int i = 0, pos = target; pos != -1; pos = pre[pos]){
            ret[cnt++] = pos;
        }
        return cnt;
    }
    return -1;
}

graph::graph(const char *edgeFile, const char *nodeFile){
    FILE *f = fopen(edgeFile, "r");
    if (f == NULL){
        printf(" Error Open File %s\n", edgeFile);
        return;
    }
    reader.setFile(f);
    n = reader.readInt();
    m = reader.readInt();
    
    std::vector<pair<int,int> > *g = new std::vector<pair<int,int> >[n];
    
    {
        // there is bug in hightlight.
    }
    
    for (int i = 0;i < m;i++){
        int x = reader.readInt();
        int y = reader.readInt();
        int z = reader.readInt();   
        g[x].push_back(make_pair(y, z));
        g[y].push_back(make_pair(x, z));
    }

    start = new int[n+1];
    edges = new edge[m<<1|1];
    start[0] = 0;
    for (int i = 0;i < n;i++){
        start[i+1] = start[i] + g[i].size();
        memcpy(edges + start[i], &g[i][0], g[i].size() * sizeof(pair<int,int>));
    }

    printf("Edges Read Finished!\n");
    
    f = fopen(nodeFile, "r");
    nodes = new vertex[n];

    for (int i = 0;i < n;i++){
        fscanf(f, "%d%lf%lf", &nodes[i].id, &nodes[i].x, &nodes[i].y);
        nodes[i].x *= pi / 180;
        nodes[i].y *= pi / 180;
     //   printf("%10.5lf %10.5lf\n", nodes[i].x, nodes[i].y);
    }
    delete[] g;
}
