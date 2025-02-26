#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H


# define SIZE_POPOLAZIONE 10
# define SIZE_CROMOSOMA 32
# define P_CROSSOVER 0.7
# define RIGHE_TRACKER 10
# define COLONNE_TRACKER 1000
# define B_HK 1.1
# define SEED_HASH 0
# define EPSILON 0.1
# define PATH_FILE "/Users/francescoschirinzi/Documents/Università/Magistrale/Data_Mining/Progetto/HeavyTracker/Dat1.csv"
# define PATH_FILE_GA "/Users/francescoschirinzi/Documents/Università/Magistrale/Data_Mining/Progetto/HeavyTracker/Dat3.csv"

#include "heavytracker.h"
typedef struct {
    unsigned int bucket;
    int FPi ;
    int frequenza;
    int size_array;
} Conteggio ;

typedef struct {
    unsigned int **popolazione;
    double *fitness;
} Popolazione ;

typedef struct {
    double b;
    double q;
    double gamma;
    double c;
    double b_hk;
} Parametri;

Tracker_unit* frequenza_reale();
double calcola_fitness(unsigned int *cromosoma, int k, Popolazione * p, Tracker_unit* t);
Popolazione* inizializza_popolazione();
Parametri* decodifica_cromosoma(const unsigned int *cromosoma);
void free_popolazione(Popolazione *popolazione);
void crossover(Popolazione *popolazione);
void mutazione(Popolazione *popolazione);
Parametri* genetic_algotithm();
int binary_to_decimal (const unsigned int *array);
void swap(unsigned int *cromosoma1, unsigned int *cromosoma2 , int start, int end);

#endif
