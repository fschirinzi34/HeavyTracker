#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "genetic_algorithm.h"
#include "heavytracker.h"

/*
 * -------------------------------------------main()---------------------------------------------------/
 * In questa funzione andiamo a simulare l'esecuzione degli algoritmi genetic_algoritmh e HeavyTracker.
 * Facciamo partire l'algortimo genetic_algoritmh per ricavarci i parametri migliori (che massimizzano
 * la fitness) da passare all'algoritmo heavytracker. Successivamente eseguiamo l'aloritmo heavytracker
 * passandogli di volta in volta un flusso. Per ogni flusso passato in input, l'algoritmo restituisce un
 * valore True o False, cioè indica se il conteggio del flusso supera o no la soglia t.
 * L'output dell'algorimto è salvato in un file chiamato "Output_HeavyTracker.txt".
 * --------------------------------------------------------------------------------------------------------/
*/
int main(int argc, char **argv) {

    long int seed = time(NULL);
    srand(seed);

    // Cerchiamo i migliori parametri con l'algoritmo genetico
    Parametri* parametri= genetic_algotithm();

    printf("b_hk: %f \n", parametri->b_hk);
    printf("b: %f \n", parametri->b);
    printf("c: %f \n", parametri->c);
    printf("q: %f \n", parametri->q);
    printf("gamma: %f \n", parametri->gamma);

    FILE *file = fopen(PATH_FILE, "r");
    if (file == NULL) {
        printf("File non trovato\n");
        exit(EXIT_FAILURE);
    }

    FILE *file_output = fopen("Output_HeavyTracker.txt", "w");
    if (file_output == NULL) {
        printf("File non trovato\n");
        exit(EXIT_FAILURE);
    }

    Tracker_unit * tracker = tracker_unit_Init(COLONNE_TRACKER, RIGHE_TRACKER);
    char pacchetto[2048];

    printf("INIZIA LO STREAM \n \n");

    while (fgets(pacchetto, 2048, file)) {
        char *flusso = strtok(pacchetto, ",");
        bool ht = heavyTracker(flusso,parametri->b_hk, parametri->b, parametri->c, parametri->q, parametri->gamma, SOGLIA, tracker);
        if (ht == true) {
            fprintf(file_output, "%s: true\n", flusso);
        }
    }


    fclose(file);
    fclose(file_output);
    free(parametri);
    tracker_unit_free(tracker);

    printf("STREAM TERMINATO \n");
    printf("Risultati memorizzati nel file \"Output_HeavyTracker.txt\" \n");

    return 0;
}
