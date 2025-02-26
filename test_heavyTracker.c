#include <stdlib.h>

#include "heavytracker.h"
#include "genetic_algorithm.h"
#include "Unity-master/src/unity.h"
#include "Unity-master/src/unity.c"

void setUp(void) {

}

void tearDown(void) {

}


//--------------------------- Funzioni leggermente modificate per effettuare il test ---------------------------------//


/*
 * --------------------------------------------modeA_update_test()-----------------------------------------------------/
 * La funzione è stata modificata per prendere in input anche un numero casuale utilizzato per verificare se viene
 * soddisfatta la probabilità di espulsione o no.
 * Utilizziamo questa strategia per avere sotto controllo i risulati della funzione, cosa non possibile se ci
 * affidiamo alla generazione di numeri casuali.
 * --------------------------------------------------------------------------------------------------------------------/
*/
void modeA_update_test(Tracker_unit *tk, double b, double c, double q, double γ, int j, Output_hash *output, double numero_casuale)
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

        if (numero_casuale < p_plus) {
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
 * --------------------------------------------modeB_update_test()------------------------------------------------------/
 * Seguendo lo stesso ragionamento di prima modifichiamo la funzione in modo che prenda in input 2 numeri casuali:
 * 1 viene utilizzato per vedere se viene soddisfatta la probabilità di espulsione e 1 viene utilizzato per vedere se viene
 * soddifatta la probabilità di decadimento.
 * ---------------------------------------------------------------------------------------------------------------------/
*/
void modeB_update_test(Tracker_unit *tk, double b, double c, double q, double γ, double b_hk, int j, Output_hash * output, double numero_casuale_plus, double numero_casuale_decay) {

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


        if (numero_casuale_plus < p_plus) {
            tk-> Cr[j][bucket] = tk-> Cr[j][bucket] + 1;
        }

        if (numero_casuale_decay < p_decay) {
            tk-> Ca[j][bucket] = tk-> Ca[j][bucket] - 1;
            if (tk->Ca[j][bucket] == 0) {
                tk->FPr[j][bucket] = FPi;
                tk-> Ca[j][bucket] = 1;
            }
        }
    }
}


/*
 * --------------------------------------------heavyTracker_test()------------------------------------------------------/
 * La funzione heavyTracker() è stata modificata andando a sostituire, dai parametri passati come input, l'elemento dello
 * stream pl con la struttura Output_hash precedentemente riempita.
 * Nel codice ovviamente viene eliminata la parte relativa al riempimento della struttura Output_hash.
 * Questa operazione è stata effettuata poichè vogliamo decidere noi il valore di FPi e di bucket che vogliamo passare;
 * in questo modo riusciamo a confrontare il valore che ci aspettiamo in output con il valore che realmente restituisce
 * la funzione.
 * ---------------------------------------------------------------------------------------------------------------------/
*/
bool heavyTracker_test(Output_hash *output, double b_hk, double b, double c, double q, double gamma, double t, Tracker_unit *tracker) {

    if (tracker == NULL) {
        printf("Errore: Tracker unit vuoto\n");
        exit(EXIT_FAILURE);
    }

    unsigned int m = tracker->m; // Numero di contatori
    unsigned int d = tracker->d; // Numero di righe

    unsigned int bucket = output->bucket;
    int FPi = output->FPi;

    for (int j = 0; j < d; j++) {

        if (tracker->bit[j][bucket] == 1) {

            if (FPi == tracker->FPa[j][bucket]) {
                return true;
            }
            if (FPi == tracker->FPr[j][bucket] && tracker->Cr[j][bucket] >= t) {
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
            return true;
        }
    }
    return false;
}

//--------------------------------------------- Test HeavyTracker.c --------------------------------------------------//

void test_Tracker_tracker_unit_Init() {

    Tracker_unit *tk = tracker_unit_Init(COLONNE_TRACKER, RIGHE_TRACKER);

    TEST_ASSERT_NOT_NULL(tk);

    TEST_ASSERT_EQUAL_INT(COLONNE_TRACKER, tk->m);
    TEST_ASSERT_EQUAL_INT(RIGHE_TRACKER, tk->d);

    TEST_ASSERT_NOT_NULL(tk->FPr);
    TEST_ASSERT_NOT_NULL(tk->FPa);
    TEST_ASSERT_NOT_NULL(tk->Cr);
    TEST_ASSERT_NOT_NULL(tk->Ca);
    TEST_ASSERT_NOT_NULL(tk->bit);

    for (int j = 0; j < RIGHE_TRACKER; j++) {
        TEST_ASSERT_NOT_NULL(tk->FPr[j]);
        TEST_ASSERT_NOT_NULL(tk->FPa[j]);
        TEST_ASSERT_NOT_NULL(tk->Cr);
        TEST_ASSERT_NOT_NULL(tk->Ca);
        TEST_ASSERT_NOT_NULL(tk->bit[j]);
    }

    for (int i = 0; i < RIGHE_TRACKER; i++) {
        for (int j = 0; j < COLONNE_TRACKER; j++) {
            TEST_ASSERT_EQUAL_INT(0, tk->FPr[i][j]);
            TEST_ASSERT_EQUAL_INT(0, tk->FPa[i][j]);
            TEST_ASSERT_EQUAL_INT(0, tk->Cr[i][j]);
            TEST_ASSERT_EQUAL_INT(0, tk->Ca[i][j]);
            TEST_ASSERT_EQUAL_INT(0, tk->bit[i][j]);
        }
    }

    tracker_unit_free(tk);
}

void test_swap() {
    Tracker_unit *tk = tracker_unit_Init(COLONNE_TRACKER, RIGHE_TRACKER);

    /* Test della funzione swap_f*/
    tk->FPr[0][0] = 1;
    tk->FPa[0][0] = 200;

    swap_f(tk, 0, 0);
    TEST_ASSERT_EQUAL_INT(200, tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(1, tk->FPa[0][0]);

    /* Test della funzione swap_c*/
    tk->Cr[0][0] = 1;
    tk->Ca[0][0] = 200;

    swap_c(tk, 0, 0);
    TEST_ASSERT_EQUAL_INT(200, tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(1, tk->Ca[0][0]);

    tracker_unit_free(tk);

}

void test_modeA_update() {
    /* Il test viene strutturato in 3 casi: */

    // Caso "if (tk->Cr[j][bucket] == 0 || tk->FPr[j][bucket] == FPi)":
    Tracker_unit *tk = tracker_unit_Init(COLONNE_TRACKER, RIGHE_TRACKER);

    Output_hash * oh = NULL;
    oh = (Output_hash *) malloc(sizeof(Output_hash));
    TEST_ASSERT_NOT_NULL(oh);

    oh->FPi = 10;
    oh->bucket = 0;

    modeA_update(tk,1.086667, 4.392157, 0.336920, 0.000992, 0, oh);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(1,tk->Cr[0][0]);

    modeA_update(tk,1.086667, 4.392157, 0.336920, 0.000992, 0, oh);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Cr[0][0]);

    // Caso "if (tk->Ca[j][bucket] == 0 || tk->FPa[j][bucket] == FPi)":
    oh->FPi = 100;
    modeA_update(tk,1.086667, 4.392157, 0.336920, 0.000992, 0, oh);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(100,tk->FPa[0][0]);
    TEST_ASSERT_EQUAL_INT(1,tk->Ca[0][0]);

    modeA_update(tk,1.086667, 4.392157, 0.336920, 0.000992, 0, oh);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(100,tk->FPa[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Ca[0][0]);

    // Else (Se nessuna delle 2 ipotesi precedenti viene soddisfatta):
    oh->FPi = 1000;
    modeA_update_test(tk,1.086667, 4.392157, 0.336920, 0.000992, 0, oh, 0.9);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(100,tk->FPa[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Ca[0][0]);

    modeA_update_test(tk,1.086667, 4.392157, 0.336920, 0.000992, 0, oh, 0.0);
    TEST_ASSERT_EQUAL_INT(1000,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(3,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(10,tk->FPa[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Ca[0][0]);

    tracker_unit_free(tk);

    free(oh);
}

void test_modeB_update() {

    /* Il test di questa funzione si struttura in 4 casi: */

    Tracker_unit *tk = tracker_unit_Init(COLONNE_TRACKER, RIGHE_TRACKER);

    Output_hash * oh = NULL;
    oh = (Output_hash *) malloc(sizeof(Output_hash));
    TEST_ASSERT_NOT_NULL(oh);

    // Caso "if (tk->Cr[j][bucket] == 0)":
    oh->FPi = 10;
    oh->bucket = 0;

    modeB_update(tk,1.086667, 4.392157, 0.336920, 0.000992, 1.100, 0, oh);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(1,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(1,tk->Ca[0][0]);

    // Caso "else if (tk->FPr[j][bucket] == FPi)":
    modeB_update(tk,1.086667, 4.392157, 0.336920, 0.000992, 1.100, 0, oh);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Ca[0][0]);

    // Caso "else" e "if (rand_val < p_plus)":
    oh->FPi = 100;
    modeB_update_test(tk,1.086667, 4.392157, 0.336920, 0.000992, 1.100, 0, oh, 0.0, 0.9);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(3,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(2,tk->Ca[0][0]);

    // Caso "else" e "if (rand_val < p_decay)":
    modeB_update_test(tk,1.086667, 4.392157, 0.336920, 0.000992, 1.100, 0, oh, 0.9, 0.0);
    TEST_ASSERT_EQUAL_INT(10,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(3,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(1,tk->Ca[0][0]);

    modeB_update_test(tk,1.086667, 4.392157, 0.336920, 0.000992, 1.100, 0, oh, 0.9, 0.0);
    TEST_ASSERT_EQUAL_INT(100,tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(3,tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(1,tk->Ca[0][0]);

    free(oh);
    tracker_unit_free(tk);
}

void test_heavyTraker() {
    Tracker_unit * tk = tracker_unit_Init(COLONNE_TRACKER, RIGHE_TRACKER);
    Output_hash * oh = NULL;

    oh = (Output_hash *) malloc(sizeof(Output_hash));

    tk->FPr[0][0] = 23200;
    tk->FPa[0][0] = 10432;
    tk->Cr[0][0] = 10;
    tk->Ca[0][0] = 10;
    tk->bit[0][0] = 1;

    // Caso tracker->bit[i][j] == 1 e FPi == tracker->FPa[i][j]:
    oh->FPi = 10432;
    oh->bucket = 0;
    bool risultato = heavyTracker_test(oh,1.1, 1.086667, 4.392157, 0.336920, 0.000992, 10, tk);
    TEST_ASSERT_TRUE(risultato);

    // Caso tracker->bit[i][j] == 1 e FPi == tracker->FPr[i][j] && tracker->Cr[i][j] >= t
    oh->FPi = 23200;
    risultato = heavyTracker_test(oh,1.1, 1.086667, 4.392157, 0.336920, 0.000992, 10, tk);
    TEST_ASSERT_TRUE(risultato);

    // Caso tracker->bit[i][j] == 0 e tracker -> Cr[i][j] > t
    tk->Cr[0][0] = 11;
    tk->bit[0][0] = 0;
    risultato = heavyTracker_test(oh,1.1, 1.086667, 4.392157, 0.336920, 0.000992, 10, tk);
    TEST_ASSERT_TRUE(risultato);
    TEST_ASSERT_EQUAL_INT(23200, tk->FPa[0][0]);
    TEST_ASSERT_EQUAL_INT(0, tk->FPr[0][0]);
    TEST_ASSERT_EQUAL_INT(0, tk->Cr[0][0]);
    TEST_ASSERT_EQUAL_INT(0, tk->Ca[0][0]);

    free(oh);
    tracker_unit_free(tk);
}




int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_Tracker_tracker_unit_Init);
    RUN_TEST(test_swap);
    RUN_TEST(test_modeA_update);
    RUN_TEST(test_modeB_update);
    RUN_TEST(test_heavyTraker);

    return UNITY_END();
}
