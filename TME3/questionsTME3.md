# Questions - TME 3 : Threads

Instructions : copiez vos réponses dans ce fichier (sous la question correspondante). A la fin de la séance, commitez vos réponses.

## Question 1.

```
cd build-release && ./TME3 ../WarAndPeace.txt freqstd && ./TME3 ../WarAndPeace.txt freqstdf && ./TME3 ../WarAndPeace.txt freq && check.sh *.freq


traces pour les 3 modes, invocation a check qui ne rapporte pas d'erreur

Preparing to parse ../WarAndPeace.txt (mode=freqstd N=4), containing 3235342 bytes
Total runtime (wall clock) : 120 ms

build-release$ ./TME3 ../WarAndPeace.txt freqstdf
Preparing to parse ../WarAndPeace.txt (mode=freqstdf N=4), containing 3235342 bytes
Total runtime (wall clock) : 113 ms

Preparing to parse ../WarAndPeace.txt (mode=freq N=4), containing 3235342 bytes
Total runtime (wall clock) : 140 ms

All files are identical


```

## Question 2.

start vaut 0

end vaut file_size  (fin fu fichier)

Code des lambdas :
```
[&](const std::string& word) {
    total_words++;
    um[word]++;
}
```

Accès identifiés : tout est accédé par références càd total_words et la map um sont 
passées par références


## Question 3.

Preparing to parse ../WarAndPeace.txt (mode=partition N=4), containing 3235342 bytes
Nb des mots uniques 20332
Total runtime (wall clock) : 146 ms

Preparing to parse ../WarAndPeace.txt (mode=freq N=4), containing 3235342 bytes
Nb des mots uniques 20332
Total runtime (wall clock) : 134 ms

## Question 4.
Preparing to parse ../WarAndPeace.txt (mode=mt_naive N=4), containing 3235342 bytes
Erreur de segmentation

logique car y'a du data race on gros les threads essaie d'écrire dans la hash map 
à un moment il faut qu'on face de la réallocation pour plus d'espace mes y'a un thread qui vient pour écrire dans
la hashmap

## Question 5.
ça marche mais ça fait n'importe quoi sur l'incrémentation des occurences on en trouve moins par rapport au autre modes qui marche

la raison pour ça on utilise un compteur non atomique partagé entre les threads 
on assure pas l'atomicité de l'incrémentation 
(load modify store)

## Question 6.
le nombre total des mots correspond exactement à ce que on attend dans les autre fichiers avec un mode monothread 

mais les occurences sont erronnées car les classes du standard ne sont pas thread safe

## Question 7.
...


