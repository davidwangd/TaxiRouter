#include "data.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <ctime>
#include <omp.h>
#include <algorithm>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cmath>
#include <unistd.h>

using namespace std;

struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
char recbuf[1026];
char curbuf[1026];
char sendbuf[10000000];
char opt[1026];
char *argv[100];

struct Taxi{
    int start;
    int n;
    int id;
    int pos[4];
    int dis[4][4];
};
graph *g;
const double pi = acos(-1);
const int ntaxis = 100000;
const int thres = 10000;
Taxi taxis[ntaxis + 1];

void process(int argc, char *argv[]){
    sendbuf[0] = 0;
    static int *list = NULL;
    static int *list2 = NULL;
    static int magic = 1;
    if (list == NULL){
        list = new int[g->vertCnt() * 5];
        list2 = new int[g->vertCnt() * 4];
        std::memset(list, 0, g->vertCnt() * 5 * sizeof(int));
        std::memset(list2, 0, g->vertCnt() * 4 * sizeof(int));
    }
    
    printf("process Request!\n");
    for (int i = 0;i < argc;i++){
        printf("\t%s\n", argv[i]);
    }
    if (strcmp(argv[0], "findpos") == 0){
        int x;
        sscanf(argv[1], "%d", &x);
        pair<double,double> ret = g->find_pos(x);
        sprintf(sendbuf+strlen(sendbuf), "Pos %.6lf %.6lf", ret.first * 180 / pi, ret.second * 180 / pi);
    }else if (strcmp(argv[0], "findroute") == 0){
        int x, y;
        sscanf(argv[1], "%d", &x);
        sscanf(argv[2], "%d", &y);
        int cnt = g->find_path(x, y, list, magic++);        
        sprintf(sendbuf+strlen(sendbuf), "Route %d", cnt);
        for (int i = cnt - 1;i >= 0;i--){
            pair<double,double> ret = g->find_pos(list[i]);
            sprintf(sendbuf+strlen(sendbuf), " %.6lf %.6lf", ret.first * 180 / pi, ret.second * 180 / pi);
        }
    }else if (strcmp(argv[0], "findtaxi") == 0){
        int source, target;
        sscanf(argv[1], "%d", &source);
        sscanf(argv[2], "%d", &target);

        g->find_adjanct(source, list, magic++, INT_MAX);
        g->find_adjanct(target, list2, magic++, INT_MAX);
        int cnt = 0, mcount = 5;
        vector<int> ans[6];
        // car id : wait-dis : current-guest-more-dis : already-incar-more-dis : route 
        for (int i = 0; i < 100000;i++){
            Taxi &cur = taxis[i];
            if (cnt >= mcount){
                break;
            }
            switch(cur.n){
            case 0:
                if (list[g->vertCnt() + cur.pos[0]] <= thres){
                    printf("Taxi %d: wait-dis = %d\n", cur.id, list[g->vertCnt() + cur.pos[0]]);
                    ans[cnt].push_back(cur.id);
                    ans[cnt].push_back(list[g->vertCnt() + cur.pos[0]]);
                    ans[cnt].push_back(0); 
                    ans[cnt].push_back(0);
                    ans[cnt].push_back(cur.pos[0]);
                    ans[cnt].push_back(source);
                    ans[cnt].push_back(target);
                    cnt++;
                }
                break;
            case 1:
                int f1 = list[g->vertCnt() + cur.pos[0]];   // pick up dis
                int f2 = list[g->vertCnt() + cur.pos[1]];   // source -> pick1  
                int d  = list[target + g->vertCnt()];       // source -> target
                int t1 = list2[g->vertCnt() + cur.pos[1]];   // pick1 -> target
                // 车上乘客绕路一定符合
                if (f2 + t1 - d <= thres){
                    int mindis = g->mindist(cur.pos[0], cur.pos[1]);
                    if (f1 * 2 - mindis <= thres || (f1 + f2) - mindis <= thres){
                        cnt++;
                        printf("Taxi %d: wait-dis = %d, cur = %d\n", cur.id, f1, f2+t1-d);
                        ans[cnt].push_back(cur.id);
                        ans[cnt].push_back(list[g->vertCnt() + cur.pos[0]]);
                        ans[cnt].push_back(min(f1 + f2, f1 * 2) - mindis);
                        ans[cnt].push_back(f2 + t1 - d);
                        ans[cnt].push_back(cur.pos[0]);
                        ans[cnt].push_back(source);
                        ans[cnt].push_back(cur.pos[1]);
                        ans[cnt].push_back(target);
                        cnt++;
                    }
                }
                break;
            }
        }
        sprintf(sendbuf+strlen(sendbuf), "TaxiList");
        for (int i = 0;i < 5;i++){
            sprintf(sendbuf+strlen(sendbuf), " ");
            for (int j = 0;j < ans[i].size();j++){
                sprintf(sendbuf+strlen(sendbuf), "%d", ans[i][j]);
                if (j != ans[i].size() - 1) sprintf(sendbuf+strlen(sendbuf), ",");
            }
        }
    }else if (strcmp(argv[0], "findtaxiroute") == 0){
        sprintf(sendbuf+strlen(sendbuf), "TaxiRoute %d", 0);
        for (int i = 1;i < argc - 1;i++){
            pair<double, double> cur = g->find_pos(atoi(argv[i]));
            sprintf(sendbuf+strlen(sendbuf), " %.6lf %.6lf", cur.first * 180 / pi, cur.second * 180 / pi);
        }
        sprintf(sendbuf+strlen(sendbuf), " -1 -1");
        for (int i = 1;i < argc - 2;i++){
            int ret = g->find_path(atoi(argv[i]), atoi(argv[i+1]), list, magic++);
            for (int j = ret - 1;j >= 0;j--){
                pair<double, double> cur = g->find_pos(list[j]);
                sprintf(sendbuf+strlen(sendbuf), " %.6lf %.6lf", cur.first * 180 / pi, cur.second * 180 / pi);
            }
        }
    }
    printf("Respond: \n\t%s\n", sendbuf);
}

int Link(){
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);
    
    socklen_t len = sizeof(struct sockaddr_in);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    while (bind(fd, (sockaddr *)&server_addr, len) == -1);
    printf("Bind Success!\n");

    while (listen(fd, 1) == -1);
    printf("Listening...!\n");

    int new_fd = accept(fd, (sockaddr*)&client_addr, &len);
    int numbytes = 0;
    printf("conneted!\n");
    while (1){
        numbytes += recv(new_fd, recbuf + numbytes, 1024, 0);
        for (int i = 0;i < numbytes;i++){
            if (recbuf[i] == '$'){
                memcpy(recbuf+i+1, curbuf, (strlen(recbuf) - i) * sizeof(char));
                numbytes = i+1;
                break;
            }
        }
        if (recbuf[numbytes-1] == '$'){
            printf("recv %s\n", recbuf);
            int argc = 1;
            argv[0] = recbuf+0;
            for (int i = 1; i < numbytes; i++){
                if (recbuf[i-1] == ' ' && recbuf[i] != ' '){
                    argv[argc++] = recbuf + i;
                    recbuf[i-1] = 0;
                }
            }
            process(argc, argv);
            sprintf(sendbuf+strlen(sendbuf),"$");
            send(new_fd, sendbuf, strlen(sendbuf), 0);
            
            memcpy(recbuf, curbuf, (strlen(curbuf)+1) * sizeof(char));
            numbytes = strlen(recbuf);
        }

    }
    return 0;
}


void read_taxis(const char *filename){
    FILE *f = fopen(filename, "r");
    int cur = 0; char coma;
    double d;
    while (fscanf(f, "%d%d", &taxis[cur].id, &taxis[cur].n) != EOF){
        for (int i = 0;i <= taxis[cur].n;i++){
            fscanf(f, "%lf%c%lf%c%d", &d, &coma, &d, &coma, &taxis[cur].pos[i]);
        }
        cur++;
        //printf("%d ", cur);
    }
}


int main(){
    g = new graph("data/road.nedge","data/road.cnode");
    printf("graph build finshed!\n");
    read_taxis("data/car.txt");
    printf("Process Finished!\n");

    int source, target;
    sort(taxis, taxis+ntaxis, [](const Taxi& x, const Taxi& y){
        return x.n < y.n;
    });

    Link();
    // 149708
    static int lis1[5000000];
    static int lis2[5000000];
    static int number = 1;

    while (1){
        int source = 63871;
        int target = 187163;

        g->find_adjanct(source, lis1, ++number, INT_MAX);
        g->find_adjanct(target, lis2, ++number, INT_MAX);
        
        int cnt = 0;
        const int thres = 10000;

        const int mcount = 5;
        for (int i = 0; i < 100000;i++){
            Taxi &cur = taxis[i];
            if (cnt >= mcount){
                break;
            }
            switch(cur.n){
            case 0:
                if (lis1[g->vertCnt() + cur.pos[0]] <= thres){
                    printf("%d Taxi %d: wait-dis = %d\n", i,cur.id, lis1[g->vertCnt() + cur.pos[0]]);
                    printf("%d\n",g->mindist(cur.pos[0], source));
                    cnt++;
                }
                break;
            case 1:
                int f1 = lis1[g->vertCnt() + cur.pos[0]];   // pick up dis
                int f2 = lis1[g->vertCnt() + cur.pos[1]];   // source -> pick1  
                int d  = lis1[target + g->vertCnt()];       // source -> target
                int t1 = lis2[g->vertCnt() + cur.pos[1]];   // pick1 -> target
                // 车上乘客绕路一定符合
                if (f1 <= thres / 2 || (f1 + f2) <= thres){
                    if (f2 + t1 - d <= thres){
                        cnt++;
                        printf("Taxi %d: wait-dis = %d, cur = %d\n", cur.id, f1, f2+t1-d);
                    }
                }
                break;
            }
        }
        if (cnt < mcount){
            printf("\n\nerror %d %d\n", source, target);
            break;
        }
        break;
        number++;
        if (number % 1 == 0){
            printf("."); fflush(stdout);
        }
    }

    int *list = new int[2 * g->vertCnt()];
    memset(list, 0, 2*g->vertCnt() * sizeof(int));
    int pos = 1;
    g->find_adjanct(pos, list, 1, INT_MAX);
/*
    for (int i = 0;i < g->vertCnt();i++){
        if (list[i] == 1) printf("%6d:%7d%7d\n", i, list[i+g->vertCnt()], g->mindist(pos, i));
    }
*/
    int x[] = {1,3,4};
    int y[] = {2,10,15};
    int *pp = new int[g->vertCnt() * 4];

    int s = clock();
    
#pragma omp parallel for num_threads(8)
    for (int i = 0;i < 100;i++){
        int a = rand() % g->vertCnt();
        int b = rand() % g->vertCnt();

        printf("%d %d %d\n", a, b, g->find_path(a,b,pp, i+1));
    }
    int t = clock();

    fprintf(stderr, "%lf\n", (double)(t-s)/CLOCKS_PER_SEC);


    return 0;
}