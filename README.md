# HeavyTracker

## Istruzioni di utilizzo:

1. Scaricare la repository Git Hub contenente l'intero progetto:

2. Scaricare il file "Dataset-Unicauca-Version2-87Atts.csv" al seguente [link ][link_kaggle] 
   e attribuirgli il nome "Dat1.csv". Inoltre creare un altro file che contiene le prime 
   100000 righe di Dat1.csv e denominarlo Dat3.csv.
   Per effettuare i test è necessario scaricare la cartella del framework Unity presente al seguente [link ][link_Unity]

3. Compilare dando il seguente comando:
	```
	make
    ```
    
4. Per eseguire l'algoritmo dare il comando:

	```
	./heavyTracker
    ```
    
5. Per testare l'algoritmo genetico: 

	```
	./test_ga
    ```
    
6. Per eseguire i test dell'algoritmo HeavyTracker:

	```
	./test_heavyTracker
    ```
    
7. Per stampare in output i flussi che sono risultati frequenti (vengono stampati in modo
univoco):

	```
	python3 count_distinct.py
    ```
    
    
[link_kaggle]: <https://www.kaggle.com/datasets/jsrojas/ip-network-traffic-flows-labeled-with-87-apps?resource=download>
[link_Unity]: <https://github.com/ThrowTheSwitch/Unity>