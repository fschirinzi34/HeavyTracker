#include <stdlib.h>
#include <stdio.h>
#include "heavytracker.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "xxhash.h"

/*
 *----------------------------------------hash_function()---------------------------------------------/
 * Funzione che prende in input il numero di colonne (Tracker unit per riga) del Tracker, l'input che vogliamo mappare
 * con la funzione hash e il seme per la funzione hash.
 * Utilizzando la funzione hash XXH64 situata nella libreria xxhash.h andiamo a mappare l'input al flusso
 * corrispondente e con FPi % m ci ricaviamo il bucket in cui viene mappato dal flusso
 * --------------------------------------------------------------------------------------------------/
 */
Output_hash * hash_function(unsigned int m, char *input, int seed) {
    Output_hash * oh = NULL;
    oh = (Output_hash *) malloc(sizeof(Output_hash));
    if (oh == NULL) {
        printf("Errore: malloc fallita\n");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(input);
    unsigned int FPi = XXH64(input, len, seed);
    oh->FPi = (int) FPi;

    unsigned int bucket = FPi % m;
    oh->bucket = bucket;

    return oh;
}

/*
 * ----------------------------------------tracker_unit_Init()---------------------------------------------/
 * Funzione che prende in input il numero di colonne(Tracker unit per riga) e di righe con cui vogliamo
 * inizializzare il nostro Tracker.
 * Inizialmente tutti i campi del Tracker saranno settati a 0 (stiamo usando la calloc).
 * --------------------------------------------------------------------------------------------------------/
*/

Tracker_unit * tracker_unit_Init(unsigned int m, unsigned int d)
{
    Tracker_unit *tk = NULL;

    tk=(Tracker_unit *) malloc(sizeof(Tracker_unit));

    if (tk) {

        tk->m = m;
        tk->d = d;
        tk->FPr=(int **) calloc(tk->d, sizeof(int *));
        tk->FPa=(int **) calloc(tk->d, sizeof(int *));
        tk->Ca=(int **) calloc(tk->d, sizeof(int *));
        tk->Cr=(int **) calloc(tk->d, sizeof(int *));
        tk->bit=(unsigned int **) calloc(tk->d, sizeof(unsigned int *));

        if (tk->FPa == NULL || tk->FPa == NULL || tk->Ca == NULL || tk->Cr == NULL) {
            printf("Errore: calloc fallita\n");
            exit(EXIT_FAILURE);
        }


        for (unsigned int j = 0; j < d; j++) {

            tk->FPr[j]=(int *) calloc(tk->m, sizeof(int));
            tk->FPa[j]=(int *) calloc(tk->m, sizeof(int));
            tk->Cr[j]=(int *) calloc(tk->m, sizeof(int));
            tk->Ca[j]=(int *) calloc(tk->m, sizeof(int));
            tk->bit[j]=(unsigned int *) calloc(tk->m, sizeof(unsigned int));

            if (tk->FPr[j] == NULL || tk->FPa[j] == NULL || tk->Cr[j] == NULL || tk->Ca[j] == NULL) {
                printf("Errore: calloc fallita\n");
                exit(EXIT_FAILURE);
            }
        }

    }

    return tk;
}

/*
 * -------------------------------------------swap_f()-----------------------------------------------------/
 * Funzione che prende in input il tracker(deve essere precedentemente inizializzato), un intero "bucker" che
 * indica il tracker unit a cui ci riferiamo e un intero "j" che indica la riga a cui ci riferiamo.
 * La seguente funzione fa uno swap dei valori presenti nei campi del FPr e FPa del tracker unit identificato
 * da [bucket] e [j].
 * --------------------------------------------------------------------------------------------------------/
*/
void swap_f(Tracker_unit * tk, unsigned int bucket, int j) {
    if (tk != NULL) {
        int temp = tk->FPr[j][bucket];
        tk->FPr[j][bucket]= tk->FPa[j][bucket];
        tk->FPa[j][bucket] = temp;
    } else {
        printf("Errore: Tracker Unit vuoto\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * -------------------------------------------swap_c()-----------------------------------------------------/
 * Funzione che prende in input il tracker(deve essere precedentemente inizializzato), un intero "bucker" che
 * indica il tracker unit a cui ci riferiamo e un intero "j" che indica la riga a cui ci riferiamo.
 * La seguente funzione fa uno swap dei valori presenti nei contatori  Cr e Ca del tracker unit identificato
 * da [bucket] e [j].
 * --------------------------------------------------------------------------------------------------------/
*/
void swap_c(Tracker_unit * tk, unsigned int bucket, int j) {
    if (tk != NULL) {
        int temp = tk->Cr[j][bucket];
        tk->Cr[j][bucket] = tk->Ca[j][bucket];
        tk->Ca[j][bucket] = temp;
    }
    else {
        printf("Errore: Tracker Unit vuoto\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * -------------------------------------------modeA_update()-----------------------------------------------------/
 * Funzione che prende in input il Tracker, i parametri b, c, g e γ (precedentemente stimati dall'algoritmo
 * genetico), un intero j che il livello del tracker a cui ci troviamo e la struttura Output_hash che
 * contiene i valori restituiti dalla funzione hash_function() (FPi e bucket a cui deve essere monitorato).
 * La funzione opera secondo la seguente strategia:
 * - Se il contatore Cr è vuoto o se FPi coincide con FPr della tracker unit che stiamo analizzando poniamo
 *   FPr = FPi e incrementiamo di 1 il valore del contatore Cr
 * - Altrimenti se il contatore Ca è vuoto o se FPi coincide con FPa della tracker unit che stiamo analizzando
 *   poniamo FPa = FPi e incrementiamo di 1 il valore del contatore Ca.
 * - Altrimenti nessuno dei due contatori è vuoto e FPi non coincide con i due flussi monitorati in quel momento
 *   e dunque calcoliamo la probabilità di espulsione e, se verificata, aggiorniamo il valore di FPa con il
 *   nuovo FPi e incrementiamo Ca di 1. Se Ca > Cr  utilizziamo swap_c() e swap_f() precedentemente
 *   descritte, in quanto in FPr e Cr deve essere monitorato il flusso più frequente.
 * -------------------------------------------------------------------------------------------------------------/
*/
void modeA_update(Tracker_unit *tk, double b, double c, double q, double γ, int j, Output_hash *output)
{
    unsigned int bucket = output->bucket;
    int FPi = output->FPi;

    if (tk == NULL) {
        printf("Errore: Tracker Unit vuoto\n");
        exit(EXIT_FAILURE);
    }

    if (tk->Cr[j][bucket] == 0 || tk->FPr[j][bucket] == FPi) {

        tk->FPr[j][bucket] = FPi;
        tk-> Cr[j][bucket] = tk-> Cr[j][bucket] + 1;

    }
    else if (tk->Ca[j][bucket] == 0 || tk->FPa[j][bucket] == FPi) {

        tk->FPa[j][bucket] = FPi;
        tk-> Ca[j][bucket] = tk-> Ca[j][bucket] + 1;

    }
    else {
        double p_plus = (q) / (pow(b, tk-> Ca[j][bucket]) + c) + γ;

        // Genera un numero casuale tra 0 e 1
        double rand_val = (double) rand() / RAND_MAX;

        if (rand_val < p_plus) {
            tk->FPa[j][bucket] = FPi;
            tk-> Ca[j][bucket] = tk-> Ca[j][bucket] + 1;
        }
    }
    if (tk->Ca[j][bucket] > tk->Cr[j][bucket]) {
        swap_f(tk, bucket, j);
        swap_c(tk, bucket, j);
    }
}

/*
 * -------------------------------------------modeB_update()-----------------------------------------------------/
 * Funzione che prende in input il Tracker, i parametri b, c, g e γ (precedentemente stimati dall'algoritmo
 * genetico),il parametro b_hk scelto da noi, un intero j che indica il livello del tracker a cui ci troviamo
 * e la struttura Output_hash che contiene i valori restituiti dalla funzione hash_function() (FPi e bucket a cui
 * deve essere monitorato).
 * La funzione opera secondo la seguente strategia:
 * - Se il contatore Cr è vuoto  poniamo FPr = FPi, Cr = 1 e Ca = 1.
 * - Altrimenti se FPi coincide con il flusso monitorato in FPr incrementiamo di 1 Ca e Cr.
 * - Altrimenti Cr non è vuoto e FPi non coincide con FPa e dunque calcoliamo la probabilità di espulsione e
 *   di decadimento. Se viene soddisfatta la P. di espulsione incrementiamo di 1 il valore di Cr, mentre se
 *   soddisfatta la P di decadimento viene sottratto 1 al valore del contatore Ca. Se Ca con questo decremento
 *   diventa = 0, aggiorniamo FPa con FPi e poniamo Ca = 1.
 * -------------------------------------------------------------------------------------------------------------/
*/
void modeB_update(Tracker_unit *tk, double b, double c, double q, double γ, double b_hk, int j, Output_hash * output) {

    unsigned int bucket = output->bucket;
    int FPi = output->FPi;

    if (tk->Cr[j][bucket] == 0) {

        tk->FPr[j][bucket] = FPi;
        tk-> Cr[j][bucket] = 1;
        tk-> Ca[j][bucket] = 1;

    }

    else if (tk->FPr[j][bucket] == FPi) {

        tk-> Ca[j][bucket] = tk-> Ca[j][bucket] + 1;
        tk-> Cr[j][bucket] = tk-> Cr[j][bucket] + 1;

    }

    else {

        double p_plus = (q) / (pow(b, tk-> Cr[j][bucket]) + c) + γ;
        double p_decay = (1) / pow(b_hk, tk-> Ca[j][bucket]);

        // Genera un numero casuale tra 0 e 1
        double rand_val = (double) rand() / RAND_MAX;

        if (rand_val < p_plus) {
            tk-> Cr[j][bucket] = tk-> Cr[j][bucket] + 1;
        }

        if (rand_val < p_decay) {
            tk-> Ca[j][bucket] = tk-> Ca[j][bucket] - 1;
            if (tk->Ca[j][bucket] == 0) {
                tk->FPr[j][bucket] = FPi;
                tk-> Ca[j][bucket] = 1;
            }
        }
    }
}

/*
 * -------------------------------------------heavyTracker()-----------------------------------------------------/
 * Funzione che prende in input il Tracker, i parametri b, c, g e γ (precedentemente stimati dall'algoritmo
 * genetico),il parametro b_hk scelto da noi e un double t che indica la soglia che decreta se un flusso è
 * frequente o meno.
 * La funzione restituisce un valore boolenao True o False che indica se l'input è un flusso frequente o no.
 * Per ogni livello del Tracker applichiamo la funzione hash_function() che ci restituisce il flusso FPi e
 * il contatore a cui deve essere monitorato quel flusso. Se il Tag Bit del tracker unit a cui è mappato
 * il flusso è 1 e FPi coincide con FPa o se FPi = FPr e il conteggio Cr è >= t allora il flusso FPi è
 * frequente e viene ritornato il valore True, altrimenti richiamiamo la funzione modeB_update().
 * Se il Tag bit è = 0, se il contatore è < t richiamiamo la funzione modeA_update() altrimenti il flusso
 * è diventato frequente per via dell'ultimo conteggio aggiunto e si imposta Tag Bit = 1, FPa = FPr e
 * FPa = Cr = Ca = 0. (Si passa da modalità A a modalità B)
 * -------------------------------------------------------------------------------------------------------------/
*/
bool HeavyTracker(char *pl, double b_hk, double b, double c, double q, double gamma, double t, Tracker_unit *tracker) {

    if (tracker == NULL) {
        printf("Errore: Tracker unit vuoto\n");
        exit(EXIT_FAILURE);
    }

    unsigned int m = tracker->m; // Numero di contatori
    unsigned int d = tracker->d; // Numero di righe

    Output_hash * output;
    for (int j = 0; j < d; j++) {
        output = hash_function(m, pl, j);
        unsigned int bucket = output->bucket;
        int FPi = output->FPi;

        if (tracker->bit[j][bucket] == 1) {

            if (FPi == tracker->FPa[j][bucket]) {
                free(output);
                return true;
            }
            if (FPi == tracker->FPr[j][bucket] && tracker->Cr[j][bucket] >= t) {
                free(output);
                return true;
            }
            modeB_update(tracker, b, c, q, gamma, b_hk, j, output);
        }
        else if (tracker -> Cr[j][bucket] < t) {
            modeA_update(tracker, b, c, q, gamma, j, output);
        }
        else {
            tracker->bit[j][bucket] = 1;
            tracker->FPa[j][bucket] = tracker->FPr[j][bucket];
            tracker->FPr[j][bucket] = 0;
            tracker->Cr[j][bucket] = 0;
            tracker->Ca[j][bucket] = 0;
            // Questo TRUE è stato aggiunto da me, non c'era nello pseudocodice.
            free(output);
            return true;
        }
        free(output);
    }
    return false;
}

/*
 * ----------------------------------------Tracker_unit_free()---------------------------------------------/
 * Funzione che prende in input il Tracker che dobbiamo deallocare e lo dealloca.
 * --------------------------------------------------------------------------------------------------------/
*/
void Tracker_unit_free(Tracker_unit *tk) {
    if (!tk) {
        return;
    }
    for (int j = 0; j < tk->d; j++) {
        free(tk->FPr[j]);
        free(tk->FPa[j]);
        free(tk->Cr[j]);
        free(tk->Ca[j]);
        free(tk->bit[j]);
    }
    free(tk->FPr);
    free(tk->FPa);
    free(tk->Cr);
    free(tk->Ca);
    free(tk->bit);
    free(tk);
}

/*
 * ----------------------------------------print_contatori()---------------------------------------------/
 * Funzione che prende in input il Tracker e stampa tutti i tracker unit che lo compongono
 * Per ogni tracker unit stampiamo il valore di Tag bit, FPr, FPa, Cr e Ca.
 * --------------------------------------------------------------------------------------------------------/
*/
void print_contatori(Tracker_unit *tk) {
    if (tk != NULL) {
        for (int j = 0; j < tk->d; j++) {
            printf("Livello: %d \n", j);
            printf("FPr:");
            for (int k = 0; k < tk->m; k++) {
                printf("%d ", tk->FPr[j][k]);
            }
            printf("\n");
            printf("FPa: ");
            for (int k = 0; k < tk->m; k++) {
                printf("%d ", tk->FPa[j][k]);
            }
            printf("\n");
            printf("Cr: ");
            for (int k = 0; k < tk->m; k++) {
                printf("%d ", tk->Cr[j][k]);
            }
            printf("\n");
            printf("Ca: ");
            for (int k = 0; k < tk->m; k++) {
                printf("%d ", tk->Ca[j][k]);
            }
            printf("\n");
            printf("Bit:");
            for (int k = 0; k < tk->m; k++) {
                printf("%d ", tk->bit[j][k]);
            }
            printf("\n");
            printf("\n");
        }
    }
    else {
        printf("NULL\n");
    }
}

