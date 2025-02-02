#include <stdio.h>
#include <stdlib.h>

#include "genetic_algorithm.h"
#include "heavytracker.h"

/*
 * -------------------------------------------main()---------------------------------------------------/
 * In questa sezione andiamo a simulare l'esecuzione degli algoritmi genetic_algoritmh e heavytracker.
 * Facciamo partire l'algortimo genetic_algoritmh per ricavarci i parametri migliori (che massimizzano
 * la fitness) da passare all'algoritmo heavytracker. Successivamente eseguiamo l'aloritmo heavytracker
 * passandogli di volta in volta un pacchetto (Indirizzo_IP:Porta). L'algoritmo ci restituisce per ogni
 * input che gli passiamo se quel pacchetto appartiene ad un flusso frequente o no.
 * L'output dell'algorimto è salavato in un file chiamato "Output_HeavyTracker.txt".
 * --------------------------------------------------------------------------------------------------------/
*/
int main(int argc, char **argv) {

    // Cerchiamo i migliori parametri con l'algoritmo genetico
    Parametri* parametri= genetic_algotithm();

    printf("b_hk: %f \n", parametri->b_hk);
    printf("b: %f \n", parametri->b);
    printf("c: %f \n", parametri->c);
    printf("q: %f \n", parametri->q);
    printf("gamma: %f \n", parametri->gamma);

    FILE *file = fopen("/Users/francescoschirinzi/Documents/Università/Magistrale/Data_Mining/Progetto/HeavyTracker/Indirizzi_IP.txt", "r");
    if (file == NULL) {
        printf("File non trovato\n");
        exit(EXIT_FAILURE);
    }

    FILE *file_output = fopen("/Users/francescoschirinzi/Documents/Università/Magistrale/Data_Mining/Progetto/HeavyTracker/Output_HeavyTracker.txt", "w");
    if (file_output == NULL) {
        printf("File non trovato\n");
        exit(EXIT_FAILURE);
    }

    Tracker_unit * tracker = tracker_unit_Init(COLONNE_TRACKER, 5);
    double soglia = 9999;
    char pacchetto[1024];

    printf("INIZIA LO STREAM DI PACCHETTI \n \n");

    while (fgets(pacchetto, 1024, file)) {
        bool ht = HeavyTracker(pacchetto,parametri->b_hk, parametri->b, parametri->c, parametri->q, parametri->gamma, soglia, tracker);
        fprintf(file_output, "%s: %s\n", pacchetto, ht ? "true" : "false");
    }

    fclose(file);
    fclose(file_output);
    free(parametri);
    Tracker_unit_free(tracker);


    return 0;
}
