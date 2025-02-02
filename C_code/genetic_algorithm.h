#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H


# define SIZE_POPOLAZIONE 50
# define SIZE_CROMOSOMA 32
# define P_CROSSOVER 0.7
# define RIGHE_TRACKER 1
# define COLONNE_TRACKER 50
# define B_HK 1.1
# define SEED_HASH 0
# define EPSILON 0.1

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

Conteggio* frequenza_reale(int m);
double calcola_fitness(unsigned int *cromosoma, int k, Popolazione * p, Conteggio *c);
Popolazione* inizializza_popolazione();
Parametri* decodifica_cromosoma(unsigned int *cromosoma);
void free_popolazione(Popolazione *popolazione);
void crossover(Popolazione *popolazione);
void mutazione(Popolazione *popolazione);
Parametri* genetic_algotithm();

#endif
