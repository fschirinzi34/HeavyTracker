#ifndef HEAVYTRACKER_H
#define HEAVYTRACKER_H

#include <stdbool.h>

typedef struct {
    int **FPr;
    int **Cr;
    int **FPa;
    int **Ca;
    unsigned int **bit;
    unsigned int m; // numero di tracker unit
    unsigned int d; // numero di livelli
} Tracker_unit;

typedef struct {
    unsigned int bucket;
    int FPi ;
} Output_hash;

bool HeavyTracker(char *pl, double b_hk,double b,  double c, double q, double gamma, double t, Tracker_unit *tracker);
void print_contatori(Tracker_unit *tk);
void Tracker_unit_free(Tracker_unit *tk);
Tracker_unit * tracker_unit_Init(unsigned int m, unsigned int d);

#endif