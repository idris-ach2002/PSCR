# TME4 Answers

Tracer vos expériences et conclusions dans ce fichier.

Le contenu est indicatif, c'est simplement la copie rabotée d'une IA, utilisée pour tester une version de l'énoncé.
On a coupé ses réponses complètes (et souvent imprécises voire carrément fausses, deadlocks etc... en Oct 2025 les LLM ont encore beaucoup de mal sur ces questions, qui demandent du factuel et des mesures, et ont de fortes tendances à inventer).
Cependant on a laissé des indications en particulier des invocations de l'outil possibles, à adapter à votre code.

## Question 1: Baseline sequential

### Measurements (Release mode)

**Resize + pipe mode:**
```
./build/TME4 -m resize -i input_images -o output_images

On voit que les performances sont stables pratiquement (Y'a que le thread main qui travaille)
et on terme de consommation de mémoire par le process main on est autour de 80MG sur 168 taille de notre répertoire

le temps wall clock est légérement plus grand que le temps réel du traitement du process c'est logique
car on perd un peu du temps pour chercher les fichiers, les ramener à la RaM etc

Wall Clock Time

C’est le temps réel écoulé du début à la fin de l'exécution du programme, comme mesuré par une montre murale.

Il inclut :

Le temps d’attente,

La latence,

Le temps passé à chercher/charger des fichiers,

Les accès disques,

Les files d’attente entre threads, etc.

→ C’est ce que voit l’utilisateur.

Total CPU Time

C’est la somme des temps passés par chaque thread en exécution réelle sur le processeur.

Par exemple :

1 thread qui travaille pendant 2 secondes → 2s de CPU

4 threads qui travaillent pendant 2 secondes → 8s de CPU total

Il ne reflète pas le temps d’attente ou d’inactivité, seulement le travail actif.

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'resize', nthreads 4
Thread 124287574948160 (main): 3229 ms CPU
Total runtime (wall clock): 3321 ms
Memory usage: Resident: 77.9 MB, Peak: 168 MB
Total CPU time across all threads: 3229 ms



Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'resize', nthreads 4
Thread 128276634097984 (main): 3161 ms CPU
Total runtime (wall clock): 3251 ms
Memory usage: Resident: 78.1 MB, Peak: 168 MB
Total CPU time across all threads: 3161 ms


./build/TME4 -m pipe -i input_images -o output_images

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'pipe', nthreads 4
Thread 126956196591296 (treatImage): 3180 ms CPU
Thread 126956251212096 (main): 1 ms CPU
Total runtime (wall clock): 3272 ms
Memory usage: Resident: 68.9 MB, Peak: 159 MB
Total CPU time across all threads: 3181 ms

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'pipe', nthreads 4
Thread 139553092400832 (treatImage): 3333 ms CPU
Thread 139553134438720 (main): 1 ms CPU
Total runtime (wall clock): 3430 ms
Memory usage: Resident: 64.9 MB, Peak: 155 MB
Total CPU time across all threads: 3334 ms

```



## Question 2: Steps identification

I/O-bound: la recherche des fichiers dans opts.inputFolder (Système fichier) pr::loadImage(file); pr::saveImage(resized, outputFile); 
CPU-bound:  pr::resizeImage(original);

parallelisable a priori ? oui

## Question 3: BoundedBlockingQueue analysis
Cette classe est multi thread safe supporte plusieurs producteurs et consommateurs

un appel peut blocker dans le cas ou un producteur veut créer un élément alors que la fille est full
aussi un appel peut blocker dans le cas ou un consommateur qui tenete de bouffer un élément alors que la fille est empty

dans les appelles à cv_.wait 
   cv_.wait(lock, [this] { return queue_.size() < max_size_; });
   on capture l'objet appellant this qui est un pointeur (BoundedBlockingQueue *) donc pas besoin de passer ça références
   et réellement ({return this->queue_.size() < this->max_size_;})

   cette approche est plus lisible 
   on veut dire au thread qui détient le lock tu attends jusqu'à (util) ce que la condition soit validée 

## Question 4: Pipe mode study

FILE_POISON ... c'est un marqueur qui indique la fin des fichiers à traiter on l'injècte par le thread main dans la fille bloquante à la fin de l'exploration des tous les fichiers
dans le répertoire input_images

Order/invert : non on ne peut pas toucher 

        // 1. Single-threaded pipeline: file discovery -> treatImage (load/resize/save)
        pr::FileQueue fileQueue(10);

        // 2. Start the worker thread
        std::thread worker(pr::treatImage, std::ref(fileQueue), std::ref(opts.outputFolder));

        // 3. Populate file queue synchronously
        pr::findImageFiles(opts.inputFolder, [&](const std::filesystem::path& file) {
            fileQueue.push(file);
        });

        // 4. Push poison pill
        fileQueue.push(pr::FILE_POISON);

        // 5. Join the worker thread
        worker.join();

        déjà changer 1 et 2 crée une erreur de compilation (décalration de la variable)

        4 et 5 
        si on attend indéfiniment sans injecter le poison 
        la boucle while dans treatImage exécutée par le thread qu'on vient de créer ne va 
        jamais s'arreter sur le break car on injecte pas le POISON 

        etc


## Question 5: Multi-thread pipe_mt

Implement pipe_mt mode with multiple worker threads.

For termination, ... poison pills...

Measurements:
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME4$ ./build-release/TME4 -m pipe_mt -i input_images -o output_images
Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'pipe_mt', nthreads 4
Thread 133809762715328 (treatImage): 891 ms CPU
Thread 133809787893440 (treatImage): 980 ms CPU
Thread 133809771108032 (treatImage): 1005 ms CPU
Thread 133809779500736 (treatImage): 1032 ms CPU
Thread 133809829910848 (main): 1 ms CPU
Total runtime (wall clock): 1064 ms
Memory usage: Resident: 151 MB, Peak: 358 MB
Total CPU time across all threads: 3909 ms

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'pipe_mt', nthreads 8
Thread 128125964900032 (treatImage): 549 ms CPU
Thread 128125931329216 (treatImage): 587 ms CPU
Thread 128125981685440 (treatImage): 592 ms CPU
Thread 128125948114624 (treatImage): 596 ms CPU
Thread 128125922936512 (treatImage): 661 ms CPU
Thread 128125939721920 (treatImage): 673 ms CPU
Thread 128125973292736 (treatImage): 696 ms CPU
Thread 128125956507328 (treatImage): 722 ms CPU
Thread 128126029207872 (main): 1 ms CPU
Total runtime (wall clock): 741 ms
Memory usage: Resident: 254 MB, Peak: 513 MB
Total CPU time across all threads: 5077 ms

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'pipe_mt', nthreads 16
Thread 139325962401472 (treatImage): 272 ms CPU
Thread 139325995972288 (treatImage): 348 ms CPU
Thread 139326054721216 (treatImage): 344 ms CPU
Thread 139325970794176 (treatImage): 330 ms CPU
Thread 139325945616064 (treatImage): 368 ms CPU
Thread 139326037935808 (treatImage): 364 ms CPU
Thread 139325937223360 (treatImage): 369 ms CPU
Thread 139326029543104 (treatImage): 362 ms CPU
Thread 139326021150400 (treatImage): 393 ms CPU
Thread 139326004364992 (treatImage): 404 ms CPU
Thread 139326012757696 (treatImage): 384 ms CPU
Thread 139325954008768 (treatImage): 452 ms CPU
Thread 139325987579584 (treatImage): 432 ms CPU
Thread 139326063113920 (treatImage): 491 ms CPU
Thread 139326046328512 (treatImage): 513 ms CPU
Thread 139325979186880 (treatImage): 541 ms CPU
Thread 139326107228480 (main): 1 ms CPU
Total runtime (wall clock): 578 ms
Memory usage: Resident: 356 MB, Peak: 840 MB
Total CPU time across all threads: 6368 ms
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME4$ 

Best: 32 mais les peak de ram commence à nous faire peur

idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME4$ ./build-release/TME4 -m pipe_mt -i input_images -o output_images -n 32
Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'pipe_mt', nthreads 32
Thread 129487662458560 (treatImage): 168 ms CPU
Thread 129487469426368 (treatImage): 145 ms CPU
Thread 129487612102336 (treatImage): 157 ms CPU
Thread 129487561746112 (treatImage): 117 ms CPU
Thread 129487679243968 (treatImage): 212 ms CPU
Thread 129487536568000 (treatImage): 162 ms CPU
Thread 129487628887744 (treatImage): 156 ms CPU
Thread 129487603709632 (treatImage): 167 ms CPU
Thread 129487494604480 (treatImage): 194 ms CPU
Thread 129487544960704 (treatImage): 219 ms CPU
Thread 129487595316928 (treatImage): 234 ms CPU
Thread 129487586924224 (treatImage): 118 ms CPU
Thread 129487553353408 (treatImage): 209 ms CPU
Thread 129487696029376 (treatImage): 207 ms CPU
Thread 129487645673152 (treatImage): 240 ms CPU
Thread 129487461033664 (treatImage): 288 ms CPU
Thread 129487620495040 (treatImage): 141 ms CPU
Thread 129487511389888 (treatImage): 284 ms CPU
Thread 129487477819072 (treatImage): 241 ms CPU
Thread 129487637280448 (treatImage): 140 ms CPU
Thread 129487578531520 (treatImage): 279 ms CPU
Thread 129487486211776 (treatImage): 267 ms CPU
Thread 129487519782592 (treatImage): 207 ms CPU
Thread 129487528175296 (treatImage): 255 ms CPU
Thread 129487670851264 (treatImage): 302 ms CPU
Thread 129487570138816 (treatImage): 318 ms CPU
Thread 129487452640960 (treatImage): 309 ms CPU
Thread 129487502997184 (treatImage): 171 ms CPU
Thread 129487654065856 (treatImage): 295 ms CPU
Thread 129487444248256 (treatImage): 313 ms CPU
Thread 129487687636672 (treatImage): 326 ms CPU
Thread 129487704422080 (treatImage): 433 ms CPU
Thread 129487751944512 (main): 2 ms CPU
Total runtime (wall clock): 532 ms
Memory usage: Resident: 474 MB, Peak: 1.29 GB
Total CPU time across all threads: 7276 ms


## Question 6: TaskData struct

```cpp
// permet de véhiculer les données nécessaire pour le traitement 
struct TaskData {
    std::filesystem::path file;
    QImage image;
};

```

d'après la référence 
"" On note que QImage est a priori "lourd" en mémoire. Cependant il est bien implanté, e.g. movable, et son
implantation de la copie utilise une sémantique "copy on write", i.e. il n'y a vraiment copie complète que si on
modifie une des deux. ""

càd si on modifie par une image on transmet réellement ça référence (ce qui est utile pour la dernière étape (Stockage de l'image))

On gros le main s'occupe d'allimenter la file bloquante avec les noms de fichiers

et puis le reader il lit ces fichiers et produit des images dans imageQueue (Le champs image est nécessaire
pour enchainer les traitements)

le resizer il va lire depuis imageQueue (donc il lit le champs image transmis dans la structure)
le resizer crée une nouvelle image rédimensionnée et produit dans resizedQueue (et en plus stocke le chemin de cette dernière
ce qui est important pour la dernière étape)

enfin le saver (lit depuis resizedQueue et stocke l'image avec le chemin (file) dans le répertoire de sortie)

const struct TaskData TASK_POISON {};

## Question 7: ImageTaskQueue typing
Quels sont les avantages et inconvénients de chaque approche ? Choisissez-en une (dans Tasks.h)
et justifiez votre choix. (indice : on pense en termes de gestion mémoire, de copies, etc.) Comment
définir une constante TASK_POISON dans ces deux cas ?

si on transmet l'adresse tâche elle même dans la fille on risque d'avoir des erreur de segmetation
car une fois le thread reader termine ça tache on injecte le poison par le main (FILE_POISON)
et bien on sort de la boucle et on perd tous les task's si on essaie de les accéder par le resizer 
on se prend une erreur de segmentation

Pareil pour les autres workers

d'ou BoundedBlockingQueue<TaskData>

donc la solution finale c'est :
BoundedBlockingQueue<TaskData>

TaskData TASK_POISON {pr::FILE_POISON};


et à la fin pour le déclenchement de la fin du traitement on insère  la sentinelle (TASK_POISON)

## Question 8: Pipeline functions

Implement reader, resizer, saver in Tasks.cpp.

mt_pipeline mode: Creates threads for each stage, with configurable numbers.

Termination: Main pushes the appropriate number of poisons after joining the previous stage.

Measurements: 
```
./build/TME4 -m mt_pipeline -i input_images -o output_images
...
```

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'mt_pipeline', nthreads 4
Thread 127801927661248 (reader): 3401 ms CPU
Thread 127801919268544 (resizer): 113 ms CPU
Thread 127801910875840 (saver): 421 ms CPU
Thread 127801975175488 (main): 2 ms CPU
Total runtime (wall clock): 3480 ms
Memory usage: Resident: 124 MB, Peak: 213 MB
Total CPU time across all threads: 3937 ms



## Question 9: Configurable parallelism

Added nbread, nbresize, nbwrite options.


Timings:
- 1/1/1 (default): 
```
./build/TME4 -m mt_pipeline -i input_images -o output_images

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'mt_pipeline_setup', nthreads 4
Thread 124689085101760 (reader): 2904 ms CPU
Thread 124689076709056 (resizer): 63 ms CPU
Thread 124689068316352 (saver): 340 ms CPU
Thread 124689127102784 (main): 2 ms CPU
Total runtime (wall clock): 2923 ms
Memory usage: Resident: 124 MB, Peak: 213 MB
Total CPU time across all threads: 3309 ms


...
```
- 1/4/1: 
```
./build/TME4 -m mt_pipeline --nbread 1 --nbresize 4 --nbwrite 1 -i input_images -o output_images

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'mt_pipeline_setup', nthreads 4
Thread 139182366258880 (reader): 2897 ms CPU
Thread 139182357866176 (resizer): 18 ms CPU
Thread 139182349473472 (resizer): 12 ms CPU
Thread 139182332688064 (resizer): 20 ms CPU
Thread 139182341080768 (resizer): 19 ms CPU
Thread 139182324295360 (saver): 327 ms CPU
Thread 139182373685568 (main): 2 ms CPU
Total runtime (wall clock): 2919 ms
Memory usage: Resident: 208 MB, Peak: 293 MB
Total CPU time across all threads: 3295 ms

```

- 4/1/1: 
```
./build/TME4 -m mt_pipeline --nbread 4 --nbresize 1 --nbwrite 1 -i input_images -o output_images

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'mt_pipeline_setup', nthreads 4
Thread 132532819453632 (reader): 809 ms CPU
Thread 132532827846336 (reader): 903 ms CPU
Thread 132532802668224 (reader): 914 ms CPU
Thread 132532811060928 (reader): 959 ms CPU
Thread 132532794275520 (resizer): 84 ms CPU
Thread 132532785882816 (saver): 419 ms CPU
Thread 132532875360576 (main): 2 ms CPU
Total runtime (wall clock): 980 ms
Memory usage: Resident: 225 MB, Peak: 424 MB
Total CPU time across all threads: 4090 ms
```
... autres configs

Best config: 
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME4$ ./build-release/TME4 -m mt_pipeline_setup -i input_images -o output_images --nbread 16 --nbresize 8 --nbwrite 8
Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'mt_pipeline_setup', nthreads 4
Thread 127714029672128 (reader): 298 ms CPU
Thread 127714038064832 (reader): 294 ms CPU
Thread 127714130384576 (reader): 313 ms CPU
Thread 127714071635648 (reader): 259 ms CPU
Thread 127714054850240 (reader): 317 ms CPU
Thread 127714088421056 (reader): 347 ms CPU
Thread 127714063242944 (reader): 350 ms CPU
Thread 127714121991872 (reader): 375 ms CPU
Thread 127714046457536 (reader): 353 ms CPU
Thread 127714138777280 (reader): 353 ms CPU
Thread 127714021279424 (reader): 408 ms CPU
Thread 127714113599168 (reader): 420 ms CPU
Thread 127714105206464 (reader): 421 ms CPU
Thread 127714147169984 (reader): 447 ms CPU
Thread 127714080028352 (reader): 496 ms CPU
Thread 127714096813760 (reader): 514 ms CPU
Thread 127713979315904 (resizer): 15 ms CPU
Thread 127713996101312 (resizer): 16 ms CPU
Thread 127713962530496 (resizer): 10 ms CPU
Thread 127714012886720 (resizer): 12 ms CPU
Thread 127713987708608 (resizer): 8 ms CPU
Thread 127713954137792 (resizer): 10 ms CPU
Thread 127713970923200 (resizer): 14 ms CPU
Thread 127714004494016 (resizer): 29 ms CPU
Thread 127713903781568 (saver): 44 ms CPU
Thread 127713928959680 (saver): 96 ms CPU
Thread 127713886996160 (saver): 82 ms CPU
Thread 127713895388864 (saver): 92 ms CPU
Thread 127713937352384 (saver): 82 ms CPU
Thread 127713912174272 (saver): 71 ms CPU
Thread 127713920566976 (saver): 95 ms CPU
Thread 127713945745088 (saver): 90 ms CPU
Thread 127714206411072 (main): 2 ms CPU
Total runtime (wall clock): 543 ms
Memory usage: Resident: 426 MB, Peak: 909 MB
Total CPU time across all threads: 6733 ms

interprétation

🔹 1/1/1 (monothread pour chaque étape)

Wall clock : ~2923 ms

La lecture est très lente car elle est entièrement séquentielle.

Le CPU du reader est surchargé, les autres étapes (resize/saver) sont sous-utilisées.

🔹 1/4/1 (1 reader, 4 resizeurs, 1 saver)

Wall clock : ~2919 ms

L’étape de redimensionnement est bien parallélisée.

Mais le reader reste un goulot d’étranglement, car un seul thread doit charger toutes les images.

🔹 4/1/1 (4 readers, 1 resizer, 1 saver)

Wall clock : ~980 ms

Cette configuration divise par 3 le temps global, car l’étape de lecture est parallélisée, ce qui est très efficace ici.

Cependant, un seul thread pour redimensionner devient le nouveau goulot.

🔹 16/8/8 (config "optimale")

Wall clock : ~543 ms

Meilleur temps global observé.

Tous les étages sont bien parallélisés, le travail est équilibré.

Le coût mémoire augmente significativement (1.29 GB de RAM peak dans les tests précédents), donc attention à ne pas aller trop loin selon la machine disponible.
## Question 10: 

J'ai testé plusieurs configurations de parallélisme pour évaluer les performances de l'application :

| Config r/s/w | Wall clock | CPU total | Mémoire peak | Observations |
|--------------|------------|-----------|--------------|--------------|
| 1 / 32 / 1   | 2939 ms    | ≈3500 ms  | 493 MB       | Reader = goulot d’étranglement |
| 4 / 4 / 4    | 970 ms     | 4115 ms   | 472 MB       | Configuration équilibrée |
| 8 / 8 / 8    | 703 ms     | 5497 ms   | 665 MB       | Meilleure performance globale |

Ces résultats montrent que le goulot se déplace :
- Avec peu de threads en lecture, le chargement devient lent.
- Avec trop de resizers et peu de lecture, les resizers restent inactifs.
- Un bon équilibre améliore les performances (4/4/4 ou 8/8/8).

L’application supporte très bien les I/O parallèles, en particulier la lecture (`loadImage`). Elles **ne posent aucun problème**, et permettent au contraire **d’accélérer fortement le traitement** global. L’écriture parallèle contribue aussi à la performance mais de manière moins critique.


With size 1: 
```
./build/TME4 -m pipe_mt -n 2 --queue-size 1 -i input_images -o output_images
...
```

With size 100: 
```
./build/TME4 -m pipe_mt -n 2 --queue-size 100 -i input_images -o output_images
...
```

impact

Complexity: 


## Question 11: 

Oui, ma réponse était entièrement correcte. Les mesures le confirment :

En 1/32/1, on voit que le reader est surchargé (CPU ≈ 2900 ms) ⇒ preuve que c’est I/O-bound.

Les resizers consomment peu de CPU individuellement, mais leur effet combiné en 4 ou 8 fils améliore les perfs ⇒ preuve de CPU-bound, parallélisable.

Les savers ont un coût CPU modéré, et leur parallélisation aide mais n’est pas critique ⇒ I/O-bound léger.

### Q12. Impact de la taille des files sur les performances et la mémoire

j'ai étudié l'effet de la taille des files (`--queue_size`) sur le comportement de l'application dans sa version multithreadée optimale (`--nbread 8 --nbresize 8 --nbwrite 8`) ainsi qu'en version séquentielle (`pipe`).

#### Résultats observés :

| Mode        | Queue size | Temps total (ms) | RAM peak | CPU total (ms) | Observation |
|-------------|------------|------------------|----------|----------------|-------------|
| mt_pipeline_setup | 1          | 733              | 649 MB   | 5462           | Débit ralenti par blocages |
| mt_pipeline_setup | 100        | 724              | 558 MB   | 5406           | Pipeline fluide, meilleurs résultats |
| pipe        | 1          | 3225             | 148 MB   | 3134           | Pas d'impact visible |
| pipe        | 100        | 3246             | 148 MB   | 3157           | Pas d'impact visible |

#### Analyse :

- En mode multithreadé, une taille de file **trop petite** empêche le bon déroulement parallèle, car les producteurs/consommateurs doivent attendre.
- Une taille plus grande (ex: 100) permet un **meilleur débit**, car les étapes peuvent **travailler plus indépendamment**.
- L'utilisation mémoire reste sous contrôle, même pour `queue_size=100`.

#### Estimation de la mémoire maximale :

La mémoire dépend de :
- La taille des files (`Q`)
- Le nombre de threads (`R`, `Z`, `W`)
- La taille moyenne d'une image (`S`)

```bash
./TME4 -m mt_pipeline_setup -i input_images -o output_images \
--nbread 16 --nbresize 1 --nbwrite 1 --queue_size 100
```
```
Résultat :
Temps total : 635 ms (rapide)

Mémoire pic : 901 MB

Total CPU time : 6532 ms

Analyse :
Les 16 threads reader ont été très rapides à charger les images.

Un seul resizer et un seul saver n'ont pas pu suivre, créant un bouchon.

Les files (imageQueue, resizerQueue) ont atteint leur capacité maximale.

Cela confirme que la mémoire peut fortement augmenter si la production dépasse largement la capacité de traitement.

Conclusion :
Même si queue_size=100 ne pose pas de problème dans une configuration équilibrée, une configuration déséquilibrée (beaucoup de producteurs / peu de consommateurs) sature la mémoire rapidement, jusqu’à presque 1 Go ici.

 Pour éviter cela, il est nécessaire d'équilibrer nbread / nbresize / nbwrite en fonction du coût CPU/I/O de chaque étape, et non pas seulement d'augmenter la taille des files.
```

## Bonus

