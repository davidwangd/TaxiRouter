#include <pthread.h>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
using namespace std;

const int N = 100000000;
double a[N];
double r2[N];
double r3[N];

double calc(double x){
    return sqrt(log(x) / log(2.0)) * cos(x) * cos(x) 
        + sqrt(log(x) / log(2.0)) * sin(x) * sin(x);
}

int main(){
    int start, end;
    
    for (int i = 0;i < N;i++) a[i] = rand();

    start = clock();
    //for (int i = 0;i < N;i++) r2[i] = calc(a[i]);
    end = clock();

    printf("%lf\n", (double)(end - start) /  CLOCKS_PER_SEC);

    start = clock();

#pragma omp parallel for num_threads(3)
    for (int i = 0;i < N;i++)
        r3[i] = calc(a[i]);
    
    end = clock();
    printf("%lf\n", (double)(end - start) / CLOCKS_PER_SEC);

    double gmax = 0;
    for (int i = 0;i < N;i++)
        gmax = max(gmax, abs(r2[i] - r3[i]));
    
    printf("%lf\n", gmax);
}