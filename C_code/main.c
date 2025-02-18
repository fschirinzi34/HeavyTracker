#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genetic_algorithm.h"
#include "heavytracker.h"


/*
 * Fare funzione che prende in input il file di output dell'algoritmo heavy_tracker e restituisce solo elementi
 * univoci per vedere solo quali sono i flussi elefantici.
 */

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

    FILE *file = fopen(PATH_FILE, "r");
    if (file == NULL) {
        printf("File non trovato\n");
        exit(EXIT_FAILURE);
    }

    FILE *file_output = fopen("/Users/francescoschirinzi/Documents/Università/Magistrale/Data_Mining/Progetto/HeavyTracker/Output_HeavyTracker.txt", "w");
    if (file_output == NULL) {
        printf("File non trovato\n");
        exit(EXIT_FAILURE);
    }

    Tracker_unit * tracker = tracker_unit_Init(COLONNE_TRACKER, RIGHE_TRACKER);
    double soglia = 80;
    char pacchetto[2048];

    printf("INIZIA LO STREAM DI PACCHETTI: \n \n");

    while (fgets(pacchetto, 2048, file)) {
        char *flusso = strtok(pacchetto, ",");
        bool ht = HeavyTracker(flusso,parametri->b_hk, parametri->b, parametri->c, parametri->q, parametri->gamma, soglia, tracker);
        if (ht == true) {
            fprintf(file_output, "%s: true\n", flusso);
        }
    }

    fclose(file);
    fclose(file_output);
    free(parametri);
    Tracker_unit_free(tracker);


    return 0;
}
