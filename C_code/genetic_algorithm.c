#include "genetic_algorithm.h"

#include <stdio.h>
#include <stdlib.h>
#include "heavytracker.h"
#include "xxhash.h"
#include <string.h>

#include <math.h>

/* Il puntatore alla struttura Conteggio lo inizializzo a livello globale altrimenti non riesco a liberare la
memoria allocata nel momento in cui falliscono le altre funzioni presenti nell'algoritmo genetico */
static Conteggio* cont = NULL;

/*
 * ----------------------------------------print_popolazione()---------------------------------------------/
 * Funzione che prende in input la popolazione e ne stampa in output i cromosomi che la compongono.
 * --------------------------------------------------------------------------------------------------------/
*/
void print_popolazione(Popolazione *popolazione) {
    if (popolazione == NULL) {
        printf("Popolazione NULL\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        for (int j = 0; j < SIZE_CROMOSOMA; j++) {
            printf("%d ", popolazione->popolazione[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/*
 * ----------------------------------------inizializza_popolazione()---------------------------------------/
 * Funzione che inizializza la struttura Popolazione allocandola in memoria.
 * La popolazione viene vista in questo modo: le righe sono i cromosomi mentre le colonne sono i valori dei
 * cromosomi in quelle celle.
 * La struttura Popolazione è composta anche da un array "fitness" che indica la fitness associata ad ogni
 * cromosoma.
 * La popolazione viene inizializzata in modo che i cromosomi siano composti da bit casuali (0 o 1).
 * --------------------------------------------------------------------------------------------------------/
*/
Popolazione * inizializza_popolazione() {

    // La popolazione viene vista in questo modo. Le righe sono i cromosomi mentre le colonne sono i valori dei cromosomi in quelle celle

    Popolazione* popolazione = NULL;

    popolazione = (Popolazione*) malloc(sizeof(Popolazione));
    if (popolazione == NULL) {
        printf("Errore allocazione popolazione\n");
        exit(EXIT_FAILURE);
    }


    popolazione->popolazione = (unsigned int **) calloc(SIZE_POPOLAZIONE, sizeof(unsigned int* ));
    if (popolazione->popolazione == NULL) {
        printf("Errore allocazione popolazione\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        popolazione->popolazione[i] = (unsigned int *) calloc(SIZE_CROMOSOMA, sizeof(unsigned int));
        if (popolazione->popolazione[i] == NULL) {
            printf("Errore allocazione popolazione\n");
            exit(EXIT_FAILURE);
        }
    }

    popolazione->fitness = (double *) calloc(SIZE_POPOLAZIONE, sizeof(double));
    if (!popolazione->fitness) {
        printf("Errore allocazione popolazione\n");
        exit(EXIT_FAILURE);
    }


    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        for (int j = 0; j < SIZE_CROMOSOMA; j++) {
            popolazione->popolazione[i][j] = rand() % 2;
        }
    }

    return popolazione;

}

/*
 * ----------------------------------------free_popolazione()---------------------------------------/
 * Funzione utilizzata per deallocare la memoria allocata per la struttura Popolazione
 * --------------------------------------------------------------------------------------------------------/
*/
void free_popolazione(Popolazione* popolazione) {

    if (popolazione == NULL || popolazione->popolazione == NULL) {
        printf("Popolazione NULL\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        free(popolazione->popolazione[i]);
    }
    free(popolazione->popolazione);
    free(popolazione->fitness);
    free(popolazione);
}

int binary_to_decimal (const unsigned int *array) {
    int value = 0;
    for (int i = (SIZE_CROMOSOMA/4)-1; i > -1; i--) {
        value = value + (int)array[i] * (int)pow(2, (i));
    }
    return value;
}

/*
 * ----------------------------------------decodifica_popolazione()---------------------------------------/
 * Funzione che prende in input un cromosoma, lo divide in 4 parti e ad ogni parte associa un parametro
 * (b, c, q e gamma).
 * Ogni pezzo contiene n bit, dunque associamo a quel parametro associato a quel pezzo il valore ricavato
 * dalla conversione da binario a decimale di quella stringa di bit.
 * L'output è un puntatore alla Struct Parametri che contiene i valori ricavati per i parametri (b, c, q e gamma).
 * In realtà contiene anche il valore per il parametro b_hk che è scelto da noi mediante una costante situata
 * nella libreria "genetic_algorithm.h".
 * Poichè i parametri hanno dei vincoli relativi al valore che possono assumere, utilizziamo la normalizzazione
 * min-max per rispettarli.
 * La funzione binary_to_decimal() permette di effettuare la conversione da binario a decimale.
 * --------------------------------------------------------------------------------------------------------/
*/
Parametri* decodifica_cromosoma(const unsigned int *cromosoma) {

    int size = SIZE_CROMOSOMA/4;
    double b, q, gamma = 0.0, c;
    unsigned int temp[size];

    Parametri *parametri = (Parametri *) malloc(sizeof(Parametri));
    if (parametri == NULL) {
        printf("Errore allocazione parametri\n");
        exit(EXIT_FAILURE);
    }


    parametri->b_hk = B_HK;

    int count = 0;   // E' il contatore dell'array temp
    int n = 1; // Tiene conto che pezzo del cromosoma stiamo processando: 1, 2, 3 o 4

    for (int i = 0; i < SIZE_CROMOSOMA; i++) {
        int value = 0;
        // L'array temp tiene conto del pezzo del cromosoma associato ad un parametro
        temp[count] = cromosoma[i];
        count++;
        if (i+1 == n * size) {
            count = 0;   // Poniamo a 0 poichè ora temp dovrà contenere un altro pezzo del cromosoma
            value = binary_to_decimal(temp);
            unsigned int val_max = (unsigned int) pow(2, size) -1;
            unsigned int val_min = 0;
            if (n * size == SIZE_CROMOSOMA/4) {   //Stiamo nel primo pezzo poichè 1/4 della size del cromosoma
                // Diamo il valore a b che deve essere compreso tra 1 e 1.1:
                b = 1 + (value - val_min) * (1.1 - 1) / ((val_max - val_min));
                parametri->b = b;
            }
            else if (n * size == SIZE_CROMOSOMA/2) {   // Stiamo nel secondo pezzo
                // Diamo il valore a gamma che deve essere compreso tra 0 e 0.001:
                gamma = (value - val_min) * (0.001) / ((val_max - val_min));
                parametri->gamma = gamma;
            }
            else if (n * size == 3* SIZE_CROMOSOMA/4) {    // Stiamo nel terzo pezzo
                // Diamo il valore a q che deve essere compresa nell'intervallo [0, 1-gamma]
                q = (value - val_min) * (1 - gamma) / ((val_max - val_min));
                parametri->q = q;
            }
            else {   // Stiamo nel quarto pezzo
                // Diamo a c un valore compreso tra [0, 10]
                c = (double) (value - val_min) * (10)/ ((val_max - val_min));
                parametri->c = c;
            }

            n ++;
        }

    }
    return parametri;
}


/*
 * ----------------------------------------frequenza_reale()----------------------------------------------/
 * Funzione che prende in input il numero di tracker unit per livello
 * Apriamo il file che contiene lo stream di pacchetti e analizziamo riga per riga. Ogni riga contiene
 * un pacchetto <indirizzpIP:Porta>.
 * Mappiamo ogni pacchettto ricavato dal file un un flusso FPi e medianto il %m ci ricaviamo anche il
 * bucket in cui è mappato; L'algoritmo resituisce un puntatore alla struttura Conteggio che contiene
 * per ogni flusso FPi la sua frequenza reale. Per ulteriori dettagli consultare i commenti inseriti nel
 * codice.
 * --------------------------------------------------------------------------------------------------------/
*/
Conteggio* frequenza_reale(int m) {
    FILE *file = fopen("/Users/francescoschirinzi/Documents/Università/Magistrale/Data_Mining/Progetto/HeavyTracker/Indirizzi_IP.txt", "r");
    if (file == NULL) {
        printf("File non trovato");
        exit(EXIT_FAILURE);
    }

    cont = (Conteggio*)malloc(m * sizeof(Conteggio));
    if (cont == NULL) {
        printf("Malloc fallita");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    char stream [1024];
    int n = m; // La variabile n viene utilizzata per reallocare più memoria se il numero di FPi distinti è > m.
    int visitato = 0;  // Tiene conto del numero di elementi distinti visitati fino a quel momento.
    while (fgets(stream, 1024, file)) {
        size_t len = strlen(stream);
        // Per ogni elemento dello stream calcoliamo FPi e bucket tramite la f. hash XXH64.
        unsigned int FPi = XXH64(stream, len , SEED_HASH);
        unsigned int bucket = FPi % m;

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

    fclose(file);

    return cont;
}

/*
 * ----------------------------------------calcola_fitness()----------------------------------------------/
 * Funzione che prende in input un cromosoma, un intero k che indica il k-esimo cromosoma della popolazione
 * che stiamo passando come input, un puntatore alla struttura Popolazione e un contatore alla struttura
 * Conteggio che contiene i conteggi reali dei flussi FPi.
 * Viene addestrato l'algoritmo heavy_tracker con i parametri derivanti dalla decodifica del cromosoma
 * disucssa precedentemente e infine viene confrontato il conteggio del flusso stimato dal tracker con il
 * conteggio reale presente nella Struct conteggio. L'errore totale sarà dato dalla somma dei singoli errori
 * e la fitness totale sara data da : fitness= -errore.
 * Infine la fitness viene salvata in popolazione.fitness[k], cioè all'interno della struttura popolazione
 * andiamo a contenere il valore di fitness del k-esimo cromosoma.
 * --------------------------------------------------------------------------------------------------------/
*/
double calcola_fitness(unsigned int *cromosoma, int k, Popolazione *popolazione, Conteggio *conteggio) {

    Tracker_unit * tracker = tracker_unit_Init(COLONNE_TRACKER,RIGHE_TRACKER);

    if (tracker == NULL) {
        printf("Il tracker unit è vuoto\n");
        free_popolazione(popolazione);
        free(conteggio);
        exit(EXIT_FAILURE);
    }

    if (conteggio == NULL) {
        free_popolazione(popolazione);
        Tracker_unit_free(tracker);
        printf("La struct conteggio è vuota \n");
        exit(EXIT_FAILURE);
    }

    if (popolazione == NULL || popolazione->fitness == NULL || popolazione->popolazione == NULL)  {
        Tracker_unit_free(tracker);
        free(conteggio);
        printf("La struct popolazione è vuota");
        exit(EXIT_FAILURE);
    }

    Parametri * parametri = decodifica_cromosoma(cromosoma);
    // I parametri estratti dal cromosoma saranno mandati in input all'algoritmo heavy tracker.
    FILE *file = fopen("/Users/francescoschirinzi/Documents/Università/Magistrale/Data_Mining/Progetto/HeavyTracker/Indirizzi_IP.txt", "r");
    if (file == NULL) {
        Tracker_unit_free(tracker);
        free(conteggio);
        free_popolazione(popolazione);
        free(parametri);
        printf("File non trovato");
        exit(EXIT_FAILURE);
    }


    char stream [1024];
    /* Prendiamo una soglia molto grande poichè non vogliamo che si passi alla modalità B altrimenti molti conteggi
    vengono persi*/
    double t = 3000000000000.0;

    while (fgets(stream, 1024, file)) {
        HeavyTracker(stream, parametri->b_hk, parametri->b, parametri->c, parametri->q, parametri->gamma, t, tracker);
    }

    // Ora bisogna trovare l'errore associato
    int errore = 0;

    /* Viene presa la riga 0 poichè nell'algoritmo genetico per semplicità stiamo assumendo che il tracker abbia solo
    una riga*/
    for (int i = 0; i < conteggio[0].size_array; i++) {
        if (tracker->FPr[0][conteggio[i].bucket] == conteggio[i].FPi) {
            errore = errore +  abs(conteggio[i].frequenza - tracker->Cr[0][conteggio[i].bucket]);
        }
        else if (tracker->FPa[0][conteggio[i].bucket] == conteggio[i].FPi) {
            errore = errore +  abs(conteggio[i].frequenza - tracker->Ca[0][conteggio[i].bucket]);
        }
        else {
            // In questo caso andiamo a dare come errore la frequenza per intero poichè quel pacchetto non è proprio presente nei contatori
            errore = errore + conteggio[i].frequenza;
        }
    }

    double fitness =  - (double) errore;

    popolazione->fitness[k] = fitness;   // Salva il fitness del k-esimo cromosoma

    free(parametri);
    fclose(file);
    Tracker_unit_free(tracker);

    return fitness;
}


/*
 * -------------------------------------------SUS()-------------------------------------------------------/
 * Funzione che prende in input un puntatore alla struttura Popolazione.
 * La funzione SUS calcola il valore di fitness totale associato alla popolazione e inizializza un intervallo
 * definitio come F/K in cui F è la fitness totale e K la dimensione della popolazione (quanti cromosomi
 * contiene).
 * Ora l'array fitness della struttura Popolazione viene modificato in modo che alla k'esima cella contenga
 * la fitness totale avuta fino a quel momento.
 * Viene scelto un parametro r nell'intervallo [o, F/K] da cui partire e di volta in volta verifichiamo se
 * r è minore della cella k-esima di fitness. Se è minore si aggiunge il cromosoma k-esimo alla nuova
 * popolazione e si aggiorna il valore  r = r + intervallo, altrimenti si incrementa di 1 la variabile k
 * per vedere se inserendo il valore di fitness del cromosoma k-esimo ricadiamo nell'intervallo r.
 * Lo scopo di questa funzione è quella di restituire una nuova popolazione che ha con P alta cromosomi che
 * hanno fintess alta, ma c'è anche poca probabilità di avere cromosomi con fitness bassa avendo così
 * diversità nella nuova popolazione. (Per maggiori chiarimenti consultare la documentazione).
 * --------------------------------------------------------------------------------------------------------/
*/
Popolazione* SUS(Popolazione* popolazione) {

    Popolazione * popol_temp = inizializza_popolazione();
    if (popolazione == NULL) {
        printf("Popolazione vuota!! \n");
        free(cont);
        exit(EXIT_FAILURE);
    }


    double total_fitness = 0;
    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        total_fitness += popolazione->fitness[i];
    }

    // Faccio il valore assoluto in quanto fitness è negativa ma in questo caso ci serve un valore di intervallo positivo
    double intervallo = fabs(total_fitness / SIZE_POPOLAZIONE);
    double r = (double)rand() / RAND_MAX * intervallo;

    // Aggiorno l'array fitness in modo che la cella i-esima contenga la fitness totale fino a quel momento
    for (int i = 1; i < SIZE_POPOLAZIONE; i++) {
        popolazione->fitness[i] = popolazione->fitness[i] + popolazione->fitness[i - 1];
    }

    int i = 0;
    while (i < SIZE_POPOLAZIONE) {
        if (r < fabs(popolazione->fitness[i])) {
            for (int j = 0; j < SIZE_CROMOSOMA; j++) {
                popol_temp->popolazione[i][j] = popolazione->popolazione[i][j];
            }
            r = r + intervallo;
        }
        else {
            i ++;
        }
    }

    // Copio la popolazione temporanea nella popolazione originale.
    for (int k = 0; k < SIZE_POPOLAZIONE; k++) {
        for (int j = 0; j < SIZE_CROMOSOMA; j++) {
            popolazione->popolazione[k][j] = popol_temp->popolazione[k][j];
        }
    }
    free_popolazione(popol_temp);
    return popolazione;
}

/*
 * -------------------------------------------swap()-----------------------------------------------------/
 * Funzione che prende in input 2 cromosomi, una cella di partenza e una cella di fine e fa lo swap delle
 * celle che si trovano nell'intervallo [start; end] per i due cromosomi.
 * --------------------------------------------------------------------------------------------------------/
*/
void swap(unsigned int *cromosoma1, unsigned int *cromosoma2 , int start, int end) {
    int size = end - start;

    unsigned int temp1[size];
    unsigned int temp2[size];

    for (int i = start + 1; i < end; i++) {
        temp1[i - start + 1] = cromosoma1[i];
        temp2[i - start + 1] = cromosoma2[i];
    }
    for (int i = start + 1; i < end; i++) {
        cromosoma1[i] = temp2[i - start + 1];
        cromosoma2[i] = temp1[i - start + 1];
    }
}

/*
 * -------------------------------------------crossover()---------------------------------------------------/
 * Funzione che prende in input un puntatore alla struttura Popolazione, analizza tutti i cromosomi che ne
 * fanno parte e li campione con P = 0.7. Prende successivamente a 2 a 2 i cromosomi dal campione, generi
 * due numeri casuali start ed end (se start > end si fa lo swap) e richiamiamo la funzione swap, precedentemente
 * descritta, per i 2 cromosomi. Per ulteriori dettagli consultare i commenti presenti nel codice.
 * --------------------------------------------------------------------------------------------------------/
*/
void crossover(Popolazione* popolazione) {

    if (popolazione == NULL || popolazione->popolazione == NULL) {
        printf("Popolazione non trovata");
        free(cont);
        exit(EXIT_FAILURE);
    }

    int p = 0;  //Tiene conto se ho già preso il primo cromosoma o se devo ancora prenderlo
    int k = 0;  // Tiene conto della riga in cui risiedeva il primo cromosoma estratto
    unsigned int *cromosoma1 = (unsigned int *) malloc(SIZE_CROMOSOMA * sizeof(unsigned int));
    if (cromosoma1 == NULL) {
        printf("Malloc fallita");
        free(cont);
        exit(EXIT_FAILURE);
    }

    unsigned int *cromosoma2 = (unsigned int *) malloc(SIZE_CROMOSOMA * sizeof(unsigned int));
    if (cromosoma2 == NULL) {
        printf("Malloc fallita");
        free(cont);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        if (((float)rand() / (float)RAND_MAX) < P_CROSSOVER) {   // Con P_CROSSOVER = 0.7
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

                // Qui dentro faccio lo swap delle 2 righe
                int index[2]; // L'array index contiene i valori di start ed end.
                for (int j = 0; j < 2; j++) {
                    int random_number = (int)(((float)rand() / (float)RAND_MAX) * (float)(SIZE_CROMOSOMA - 1));
                    index[j] = random_number;
                }

                // Ordino l'array index in modo tale che index[0] < index[1]:
                if (index[0] > index[1]) {
                    int temp = index[0];
                    index[0] = index[1];
                    index[1] = temp;
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
 * -------------------------------------------mutazione()---------------------------------------------------/
 * Funzione che prende in input un puntatore alla struttura Popolazione e analizza cromosoma per cromosoma
 * tutti i valori che assumono. Campiona un valore di un cromosoma con P = 1/SIZE_CROMOSOMA e se viene
 * soddisfatta questa probabilità facciamo lo swap del bit (se = 0 allora diventa 1, se = 1 diventa 0).
 * --------------------------------------------------------------------------------------------------------/
*/
void mutazione(Popolazione *popolazione) {
    if (popolazione == NULL || popolazione->popolazione == NULL) {
        printf("Popolazione vuota");
        free(cont);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        for (int j = 0; j < SIZE_CROMOSOMA; j++) {
            if (((float)rand() / (float)RAND_MAX) < (float) 1/SIZE_CROMOSOMA ) {
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
 * -------------------------------------------mean_fitness()---------------------------------------------------/
 * Funzione che prende in input un puntatore alla struttura Popolazione e restituisce la fitness media dei
 * cromosomi che la compongono.
 * --------------------------------------------------------------------------------------------------------/
*/
double mean_fitness(Popolazione *popolazione) {
    double sum = 0;
    for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
        sum = sum + popolazione->fitness[i];
    }
    return sum / SIZE_POPOLAZIONE;
}

/*
 * -------------------------------------------genetic_algorithm()---------------------------------------------------/
 * Funzione che sviluppa l'algoritmo genetico. Per ulteriori dettagli confrontare la documentazione.
 * Laa funzione restituisce in output un puntatore alla struttura Parametri che contiene i parametri associati
 * al cromosoma che ha fitness maggiore.
 * -----------------------------------------------------------------------------------------------------------------/
*/
Parametri* genetic_algotithm() {
    // Inizializzo la popolazione con bit casuali.
    Popolazione* popolazione = inizializza_popolazione();

    // Calcolo per ogni FPi la sua frequenza reale.
    Conteggio *conteggio = frequenza_reale(COLONNE_TRACKER);

    unsigned int cromosma[SIZE_CROMOSOMA];
    unsigned int best_cromosoma[SIZE_CROMOSOMA];
    double fitness_best = -1000000;
    double fitness = 0;
    int k = 0;


    // In media riesco a trovare una soluzione accettabile già dopo 25/50 iterazioni(Vedere documentazione)
    while (k < 25) {

        // Calcolo del fitness per ogni cromosoma presente nella popolazione
        for (int i = 0; i < SIZE_POPOLAZIONE; i++) {
            // Estraggo il cromosoma dalla popolazione
            for (int j = 0; j < SIZE_CROMOSOMA; j++) {
                cromosma[j] = popolazione->popolazione[i][j];
            }
            // Calcolo il fitness per quel cromosoma
            fitness = calcola_fitness(cromosma, i, popolazione, conteggio);

            // Se il fitness trovato è migliore del precedente aggiorniamo il nuovo fitness e teniamo traccia del cromosoma che ha quel fitness
            if (fitness > fitness_best) {
                fitness_best = fitness;
                for (int j = 0; j < SIZE_CROMOSOMA; j++) {
                    best_cromosoma[j] = cromosma[j];
                }
            }
        }

        // Applico la funzione SUS per vedere quale cromosoma della popolazione sopravvive
        popolazione = SUS(popolazione);

        // Applico il crossover per andare a combinare con P = P_CROSSOVER i cromosomi tra loro
        crossover(popolazione);

        // Applico la mutazione per modificare con P = P_MUTAZIONE i cromosomi
        mutazione(popolazione);
        k ++;
    }

    Parametri *parametri = decodifica_cromosoma(best_cromosoma);

    free_popolazione(popolazione);
    free(conteggio);

    return parametri;
}


