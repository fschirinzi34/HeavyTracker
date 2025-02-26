#define UNITY_INCLUDE_DOUBLE

#include <stdlib.h>
#include <string.h>
#include "xxhash.h"

#include "heavytracker.h"
#include "genetic_algorithm.h"
#include "Unity-master/src/unity.h"
#include "Unity-master/src/unity.c"


void setUp(void) {

}

void tearDown(void) {

}

//--------------------------------- Funzioni modificate per effettuare il test ---------------------------------------//



/*
 * --------------------------------------------crossover_test()--------------------------------------------------------/
 * La funzione crossover è stata modificata per prendere in input un numero casuale (serve per decidere se vogliamo
 * che la probabilità venga soddistatta o no senza affidarci alla generazione di numeri casuali che non possiamo controllare
 * e di cui non possiamo prevedere il risultato) e l'array index che contiene 2 valori, 1 di "start" e uno di "end" (anche
 * in questo caso non vogliamo che i due valori vengano generati casualmente poichè vogliamo testare il codice su comportamenti
 * deterministici).
 * --------------------------------------------------------------------------------------------------------------------/
*/
void crossover_test(Popolazione* popolazione, double numero_casuale, int index[2]) {

    if (popolazione == NULL || popolazione->popolazione == NULL) {
        printf("Popolazione non trovata");
        exit(EXIT_FAILURE);
    }

    int p = 0;  //Tiene conto se ho già preso il primo cromosoma o se devo ancora prenderlo
    int k = 0;  // Tiene conto della riga in cui risiedeva il primo cromosoma estratto
    unsigned int *cromosoma1 = (unsigned int *) malloc(SIZE_CROMOSOMA * sizeof(unsigned int));
    if (cromosoma1 == NULL) {
        printf("Malloc fallita");
        exit(EXIT_FAILURE);
    }

    unsigned int *cromosoma2 = (unsigned int *) malloc(SIZE_CROMOSOMA * sizeof(unsigned int));
    if (cromosoma2 == NULL) {
        printf("Malloc fallita");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        if (numero_casuale < P_CROSSOVER) {   // Con P_CROSSOVER = 0.7
            if (p == 0) { // Se p == 0 allora dobbiamo prendere ancora il primo cromosoma
                for (int j = 0; j < SIZE_CROMOSOMA; j++) {
                    cromosoma1[j] = popolazione->popolazione[i][j];
                }
                p = 1;
                k = i; // ci salviamo k = i per salvarci l'indice in cui risiede il primo cromosoma campionato.
            }
            else { // Se p == 1 allora dobbiamo campionare il secondo cromosoma.
                p = 0;
                for (int j = 0; j < SIZE_CROMOSOMA; j++) {
                    cromosoma2[j] = popolazione->popolazione[i][j];
                }

                swap(cromosoma1, cromosoma2, index[0], index[1]);

                // aggiorniamo la popolazione iniziale con i nuovi cromosomi
                for (int j = 0; j < SIZE_CROMOSOMA; j++) {
                    popolazione->popolazione[k][j] = cromosoma1[j];
                    popolazione->popolazione[i][j] = cromosoma2[j];
                }
            }

        }
    }
    free(cromosoma1);
    free(cromosoma2);
}

/*
 * --------------------------------------------mutazione_test()--------------------------------------------------------/
 * Anche in questo caso la funzione mutazione è stata modificata per prendere in input un numero casuale utilizzato per
 * verificare se la probabilità di mutazione è stata soddisfatta o meno. Scegliendo questo valore possiamo dunque decidere
 * se entrare nel ciclo if che effettua la mutazione o se non farlo potendo così effettuare il test su comportamenti che
 * riusciamo a prevedere.
 * --------------------------------------------------------------------------------------------------------------------/
*/
void mutazione_test(Popolazione *popolazione, double numero_casuale) {
    if (popolazione == NULL || popolazione->popolazione == NULL) {
        printf("Popolazione vuota");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        for (int j = 0; j < SIZE_CROMOSOMA; j++) {
            if (numero_casuale < (float) 1/SIZE_CROMOSOMA ) {
                if (popolazione->popolazione[i][j] == 0) {
                    popolazione->popolazione[i][j] = 1;
                }
                else {
                    popolazione->popolazione[i][j] = 0;
                }
            }
        }
    }
}


/*
 * --------------------------------------frequenza_reale_test()--------------------------------------------------------/
 * La funzione frequenza_reale è stata modificata in modo da prendere in input non più il file definito dalla costante
* "PATH_FILE_GA" ma il file "File_test.txt" che contiene i seguenti flussi: "192.10.10.1:80",
 * "192.15.134.1:80", "192.64.80.12:80", "192.64.81.12:80" e "192.64.82.12:80" rispettivamente con conteggio 10, 5, 7, 1, 4.
 * In questo modo operiamo su un file di dimensioni ridotte di cui possiamo prevedere molto più velocemente i risultati.
 * Sostituiamo inoltre la costante "COLONNE_TRACKER" con il valore 3; in questo modo creiamo volontariamente delle collisioni
 * testando la funzione anche in questa circostanza.
 * --------------------------------------------------------------------------------------------------------------------/
*/
Tracker_unit* frequenza_reale_test() {
    FILE *file = fopen("File_test.txt", "r");
    if (file == NULL) {
        printf("File non trovato");
        exit(EXIT_FAILURE);
    }

    Conteggio* cont = (Conteggio*)malloc(3 * sizeof(Conteggio));
    if (cont == NULL) {
        printf("Malloc fallita");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    char stream [2048];
    int n = 3; // La variabile n viene utilizzata per reallocare più memoria se il numero di FPi distinti è > m.
    int visitato = 0;  // Tiene conto del numero di elementi distinti visitati fino a quel momento.
    while (fgets(stream, 2048, file)) {
        char *flusso = strtok(stream, ",");
        // Per ogni elemento dello stream calcoliamo FPi e bucket tramite la f. hash XXH64.
        unsigned int FPi = XXH64(flusso, strlen(flusso) , SEED_HASH);
        unsigned int bucket = FPi % 3;

        int count = 0;   // Tiene conto se sono entrato nel ciclo if o no

        // Controlliamo se il flusso FPi è già contenuto nella struttura Conteggio costruita fino a quel momento
        for (int i = 0; i < visitato + 1; i++) {
            if (cont[i].FPi == (int) FPi) {
                cont[i].frequenza++;
                count++;
            }
        }
        /* Se la variabile count è = 0 allora non siamo entrati nel ciclo if precedente e dunque il flusso FPi non è
         *  ancora presente nella struttura Conteggio
         */

        if (count == 0) {
            // Se visitato < n allora non c'è bisogno di reallocare la memoria e inseriamo il flusso FPi nella struttura
            if (visitato < n) {
                cont[visitato].FPi = (int) FPi;
                cont[visitato].frequenza = 1;
                cont[visitato].bucket = bucket;
                visitato ++;
            }
            else {
                // Altrimenti bisogna reallocare la struttura e successivamente aggiungo il conteggio
                n++;
                Conteggio* temp = realloc(cont, n * sizeof(Conteggio));
                if (temp == NULL) {
                    free(cont);
                    printf("Realloc fallita");
                    exit(EXIT_FAILURE);
                }
                cont = temp;

                cont[visitato].FPi = (int) FPi;
                cont[visitato].frequenza = 1;
                cont[visitato].bucket = bucket;
                visitato++;
            }
        }
    }
    // cont[i].size_array contiene il numero di occorrenze presenti nella struct Conteggio.
    for (int i = 0; i < visitato; i++) {
        cont[i].size_array = visitato;
    }

    Tracker_unit *tk_reale = tracker_unit_Init(3, 1);
    if (tk_reale == NULL) {
        free(cont);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < cont[0].size_array; i++) {
        if (tk_reale->Cr[0][cont[i].bucket] == 0){
            tk_reale->FPr[0][cont[i].bucket] = cont[i].FPi;
            tk_reale->Cr[0][cont[i].bucket] = cont[i].frequenza;
        }
        else if (tk_reale->Ca[0][cont[i].bucket] == 0 || tk_reale->Ca[0][cont[i].bucket] < cont[i].frequenza) {
            tk_reale->FPa[0][cont[i].bucket] = cont[i].FPi;
            tk_reale->Ca[0][cont[i].bucket] = cont[i].frequenza;
        }
        if (tk_reale->Ca[0][cont[i].bucket] > tk_reale->Cr[0][cont[i].bucket]) {
            swap_f(tk_reale, cont[i].bucket, 0);
            swap_c(tk_reale, cont[i].bucket, 0);
        }
    }

    fclose(file);
    free(cont);

    return tk_reale;
}

/*
 * --------------------------------------calcola_fitness_test()--------------------------------------------------------/
 * E' stata modificata la funzione calcola_fitness() sostituendo la costante COLONNE_TRACKER con il valore 3 (poichè
 * precedentemente, nella funzione frequenza_reale_test() abbiamo fissato le colonne del tracker reale = 3 e dobbiamo
 * confrontare i 2 tracker (stimato e reale) quindi devono avere lo stesso numero di colonne.)
 * --------------------------------------------------------------------------------------------------------------------/
*/
double calcola_fitness_test(unsigned int *cromosoma, int k, Popolazione *popolazione, Tracker_unit* tk_reale) {

    // Questo tracker contiene i conteggi stimati dall'algoritmo HeavyTracker.
    Tracker_unit * tracker = tracker_unit_Init(3,1);

    if (tracker == NULL) {
        printf("Il tracker unit è vuoto\n");
        free_popolazione(popolazione);
        tracker_unit_free(tk_reale);
        exit(EXIT_FAILURE);
    }

    if (tk_reale == NULL) {
        free_popolazione(popolazione);
        tracker_unit_free(tracker);
        printf("La struct Tracker_reale è vuota \n");
        exit(EXIT_FAILURE);
    }

    if (popolazione == NULL || popolazione->fitness == NULL || popolazione->popolazione == NULL)  {
        tracker_unit_free(tracker);
        tracker_unit_free(tk_reale);
        printf("La struct popolazione è vuota");
        exit(EXIT_FAILURE);
    }

    Parametri * parametri = decodifica_cromosoma(cromosoma);
    // I parametri estratti dal cromosoma saranno mandati in input all'algoritmo heavy tracker.
    FILE *file = fopen("File_test.txt", "r");
    if (file == NULL) {
        tracker_unit_free(tracker);
        tracker_unit_free(tk_reale);
        free_popolazione(popolazione);
        free(parametri);
        printf("File non trovato");
        exit(EXIT_FAILURE);
    }


    char stream [2048];
    /* Prendiamo una soglia molto grande poichè non vogliamo che si passi alla modalità B altrimenti molti conteggi
    vengono persi*/
    double t = 3000000000000.0;

    // Analizziamo il flusso e stimiamo i conteggi dei flussi più frequenti.
    while (fgets(stream, 2048, file)) {
        char *flusso = strtok(stream, ",");
        heavyTracker(flusso, parametri->b_hk, parametri->b, parametri->c, parametri->q, parametri->gamma, t, tracker);
    }

    int errore = 0;

    /* Viene presa la riga 0 poichè nell'algoritmo genetico per semplicità stiamo assumendo che il tracker abbia solo
    una riga*/
    for (int i = 0; i < 2; i++) {
        errore = errore + abs(tk_reale->Cr[0][i] - tracker->Cr[0][i]);
        errore = errore + abs(tk_reale->Ca[0][i] - tracker->Ca[0][i]);
    }

    double fitness =  - (double) errore / 3;


    popolazione->fitness[k] = fitness;   // Salva il fitness del k-esimo cromosoma

    free(parametri);
    fclose(file);
    tracker_unit_free(tracker);

    return fitness;
}


//---------------------------------------- Test genetich_algorithm.c -------------------------------------------------//


void test_inizializza_popolazione() {
    Popolazione *popolazione = inizializza_popolazione();

    TEST_ASSERT_NOT_NULL(popolazione);
    TEST_ASSERT_NOT_NULL(popolazione->popolazione);
    TEST_ASSERT_NOT_NULL(popolazione->fitness);

    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        TEST_ASSERT_NOT_NULL(popolazione->popolazione[i]);
    }

    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        for (int j = 0; j < SIZE_CROMOSOMA; j++) {
            unsigned int risultato = popolazione->popolazione[i][j];
            TEST_ASSERT_TRUE(risultato == 0 || risultato == 1);
        }
    }
}

void test_binary_to_decimal() {
    unsigned int array[SIZE_CROMOSOMA] = {0, 0, 0, 0, 0, 0, 0, 1};
    int valore = binary_to_decimal(array);
    TEST_ASSERT_EQUAL_INT(1, valore);
}

void test_decodifica_cromosoma() {
    unsigned int array[SIZE_CROMOSOMA] = {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1};
    Parametri *parametri = decodifica_cromosoma(array);

    TEST_ASSERT_NOT_NULL(parametri);

    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 1.00039216, parametri->b);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 0.00000392, parametri->gamma);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 0.00392155, parametri->q);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 0.03921569, parametri->c);

    free(parametri);
}

void test_frequenza_reale() {
    Tracker_unit *tracker_reale = frequenza_reale_test();
    TEST_ASSERT_NOT_NULL(tracker_reale);

    TEST_ASSERT_EQUAL_INT(12,tracker_reale->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(10,tracker_reale->Ca[0][0]);
    TEST_ASSERT_EQUAL_INT(7,tracker_reale->Cr[0][2]);

    tracker_unit_free(tracker_reale);
}

void test_calcola_fitness() {
    Tracker_unit *tk_reale = frequenza_reale_test();
    unsigned int array[SIZE_CROMOSOMA] = {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1};
    Popolazione *popolazione = inizializza_popolazione();
    double fitness = calcola_fitness_test(array, 0, popolazione, tk_reale);

    /*
     Abbiamo la seguente situazione:
     Tk_reale: FPr:-439214086 0 -709149827       Tracker_stimato: FPr:944425467 0 -709149827
                FPa: 944425467 0 -2070765119                       FPa: -1020100999 0 -2070765119
                Cr: 12 0 7                                         Cr: 10 0 7
                Ca: 10 0 1                                         Ca: 5 0 1
                Bit:0 0 0                                          Bit:0 0 0
    Dunque la fitness calcolata dovrebbe essere: -7/3 = -2.333
     */

    TEST_ASSERT_DOUBLE_WITHIN(1e-6, -2.33333333, fitness);

    free_popolazione(popolazione);
    tracker_unit_free(tk_reale);
}

void test_swap() {
    unsigned int cromosoma1[3] = {0, 0, 0};
    unsigned int cromosoma2[3] = {1, 1, 1};

    swap(cromosoma1, cromosoma2, 0, 2);
    TEST_ASSERT_EQUAL_INT(1, cromosoma1[1]);
}

void test_crossover() {
    Popolazione *popolazione = inizializza_popolazione();

    popolazione->popolazione[0][0] = 1;
    popolazione->popolazione[0][1] = 1;
    popolazione->popolazione[0][2] = 1;

    int index[2] = {0, 2};
    crossover_test(popolazione,0.2, index);

    // Ricordo che lo swap avviene solo per gli elementi compresi tra "start" ed "end".
    TEST_ASSERT_EQUAL_INT(0, popolazione->popolazione[0][1]);
    TEST_ASSERT_EQUAL_INT(1, popolazione->popolazione[1][0]);

    free_popolazione(popolazione);
}

void test_mutazione() {
    Popolazione *popolazione = inizializza_popolazione();
    for (int i = 0; i < SIZE_CROMOSOMA; i++) {
        popolazione->popolazione[0][i] = 1;
    }

    mutazione_test(popolazione, 1);
    TEST_ASSERT_EQUAL_INT(1, popolazione->popolazione[0][1]);

    mutazione_test(popolazione, 0);
    TEST_ASSERT_EQUAL_INT(0, popolazione->popolazione[0][1]);

    free_popolazione(popolazione);
}

void test_genetic_algorithm() {
    Parametri *parametri = genetic_algotithm();
    TEST_ASSERT_NOT_NULL(parametri);

    TEST_ASSERT(parametri->b >= 1 && parametri->b <= 1.1);
    TEST_ASSERT(parametri->gamma >= 0 && parametri->gamma <= 0.001);
    TEST_ASSERT(parametri->q >= 0 && parametri->q <= 1 - parametri->gamma);
    TEST_ASSERT(parametri->c >= 0 && parametri->c <= 10);

    free(parametri);
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_inizializza_popolazione);
    RUN_TEST(test_binary_to_decimal);
    RUN_TEST(test_decodifica_cromosoma);
    RUN_TEST(test_swap);
    RUN_TEST(test_crossover);
    RUN_TEST(test_mutazione);
    RUN_TEST(test_frequenza_reale);
    RUN_TEST(test_calcola_fitness);

    return UNITY_END();
}

