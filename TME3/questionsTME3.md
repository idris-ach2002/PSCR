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
Est-ce que cela résoud nos problèmes ? je dirai tout simplement que avec ce mode (mt_atomic_max) on résoud 70 % de nos problèmes
je veux dire par ça qu'on résoud l'accès concurrent à la variable partagé (compteur des mots total_words)
aussi au compteur des occurrences des mots dans les buckets comme on a changé l'implantation de la hashmap
désormais elle utilise un entier atomique pour compter les occurences de chaque mot

Pourquoi ?

les 30 % qui nous reste c'est pour la réallocation de la taille de vecteur utilisé dans la hashmap 
imaginons un scénario ou plusieurs threads ajoutent des paires 
 -- on considére que notre tables est surchargée si la taille actuelle 
    (nbr des entry uiques) est supérieure ou égale à 80% du nombre de buckets (taille du vecteur)
maintenant si on veut aggrandir notre table (on crée une nouvelle table avec la taille doublée on recopie tout les anciens couples
et on libère l'epace de l'ancienne table pour lui affecter la nouvelle)

mais il peut y avoir un cas ou un thread ajoute une paire donc on fait la réallocation dans ce thread 
alors qu'un autre essaie d'ajouter à l'ancinne table cependant son espace mémoire est désallouer 
'''''  on aaura une Segmentation fault ''''
...

## Question 8.
On remarque que le résultat est correct (sur le nombre total des mots et même les occurences)

mais une dégradation importante au niveau des performances c'est pas choquant car en d'autre termes on ralenti les threads avec la notion
des mutex si ils ont pas le verou il sont endormi jusqu'à un thread le libère puis un autre va le prendre et ainsi de suite 
on garanti l'atomicité des opérations mais pas la vitesse du traitement même si on augmente le nombre des threads

Voici un benchmark (4 threads vs 8)

./TME3 ../WarAndPeace.txt mt_mutex
Preparing to parse ../WarAndPeace.txt (mode=mt_mutex N=4), containing 3235342 bytes
Nb des mots total 565527
Nb des mots uniques 20332
Total runtime (wall clock) : 130 ms


./TME3 ../WarAndPeace.txt mt_mutex 8
Preparing to parse ../WarAndPeace.txt (mode=mt_mutex N=8), containing 3235342 bytes
Nb des mots total 565527
Nb des mots uniques 20332
Total runtime (wall clock) : 209 ms

## Question 9.
pas de gros choses qui changent on a mit la logique de l'exclusion mutuelle directement dans 
l'implantation de la hashmap (Les performances restesnt comme la version précédente Avec 4 threads)

idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ./TME3 ../WarAndPeace.txt mt_hmutex
Preparing to parse ../WarAndPeace.txt (mode=mt_hmutex N=4), containing 3235342 bytes
Nb des mots uniques 20332
Nb des mots uniques 20332
Total runtime (wall clock) : 174 ms
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ls ..
build        build-release  CMakeLists.txt  perf2csv.sh       README.md  WarAndPeace.txt
build-debug  check.sh       measurePerf.sh  questionsTME3.md  src
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ../check.sh *.freq
All files are identical

## Question 10.
Une version  parfaite qui gère très bien les ressources machine sans latence (comme les mutex)
On gros ici on alloue plus mais on gagne de l'efficacité (chaque thread à ça propre map qui n'est pas accédée de façon 
concurrente est géré par lui même)  sauf le compteur on aura le même problème que le départ d'ou le atomic
On fusionne le résultat de chaque traitement du thread dans une map finale puis on exploite le résultat

==>  Bon Benshmak

idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ./TME3 ../WarAndPeace.txt mt_hashes
Preparing to parse ../WarAndPeace.txt (mode=mt_hashes N=4), containing 3235342 bytes
Nb des mots total 565527
Nb des mots uniques 20332
Total runtime (wall clock) : 53 ms
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ../check.sh *.freq
All files are identical

## Question 11.
Pareil que la précédente On utilise notre HashMap faites maison à la place de celle du standard

idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ./TME3 ../WarAndPeace.txt mt_hhashes
Preparing to parse ../WarAndPeace.txt (mode=mt_hhashes N=4), containing 3235342 bytes
Nb des mots total 565527
Nb des mots uniques 20332
Total runtime (wall clock) : 54 ms
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ../check.sh *.freq
All files are identical

## Question 12.
dans incrementFrequency
//ici le compteur est en atomic obligatoire (Car on a un vecteur de mutex et pas un seul partagé)
//car un thread verroue sur un bucket i et un autre sur un bucket j
//les occurences son bonnes mais le compteur est en data race (i et j incrémente au même temps)

idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ./TME3 ../WarAndPeace.txt mt_hfine
Preparing to parse ../WarAndPeace.txt (mode=mt_hfine N=4), containing 3235342 bytes
Nb des mots uniques 20332
Nb des mots uniques 20332
Total runtime (wall clock) : 79 ms
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME3/build-release$ ../check.sh *.freq
All files are identical








