
#------------------------------------------Count_distinc.py--------------------------------------------------#

# Prende in input un file e restituisce in output le righe distinte che quel file contiene

if __name__ == '__main__':

    lista = []

    with open("Output_HeavyTracker.txt", "r") as f:
        for riga in f:
            flusso = riga.split(":")
            lista.append(flusso[0])

    elementi_distinti = set(lista)
    for elemento in elementi_distinti:
        print(elemento, "\n")
