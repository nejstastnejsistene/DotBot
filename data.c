#include <stdio.h>
#include <time.h>
#include "data.h"

#define N (45*36)
#define NUM_TURNS 30
#define NUM_DOTS 36


void print_dist(double dist[N]) {
    int i;
    for (i = 0; i < N; i++) {
        if (dist[i] > 1e-06) {
            printf("%d: %f\n", i + 1, dist[i]);
        }
    }
}


void project_score(int turn, int score, double dist[N]) {
    int i, j, k; 
    for (i = 0; i < N; i++) {
        dist[i] = (i + 1 == score) ? 1.0 : 0.0;
    }
    for (i = turn; i < NUM_TURNS; i++) {
        double tmp[N] = {0};
        for (j = 0; j < NUM_DOTS; j++) {
            for (k = 0; k < N; k++) {
                tmp[j + k + 1] += data[i][j] * dist[k];
            }
        }
        for (j = 0; j < N; j++) {
            dist[j] = tmp[j]; 
        }
    }
}

double score_above(int target, int turn, int score) {
    double dist[N];
    project_score(turn, score, dist);
    double p = 0;
    int i;
    for (i = target + 1; i < N; i++) {
        p += dist[i];
    }
    return p;
}

int main() {
    int target = 300;
    printf("P(X >= %d) = %f\n", target, score_above(target, 10, 35));
    return 0;
}
