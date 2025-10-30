# TME5 Correction: Parallelization of a Ray Tracer

## Baseline Sequential

### Question 1
Extracted TME5.zip from Moodle, added to repo, committed and pushed.

Configured project with CMake as previous TMEs. No dependencies, all handmade.

### Question 2

Ran `build/TME5` and generated `spheres.bmp`.

Platform: 
se$ ./TME5 
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode sequential
Total time 1775ms.

Pour la hauteuir et la largeur par défaut

Temps/baseline choisi :

## With Manual Threads

### Question 3
```
Implemented `void renderThreadPerPixel(const Scene& scene, Image& img)` in Renderer.
(Méthode à ne jamais penser utiliser lorsque H*W sont plus grands que 1000)

 ./TME5 -m ThreadPerPixel -W 100 -H 100
Ray tracer starting with output 'spheres.bmp', resolution 100x100, spheres 250, mode ThreadPerPixel
Total time 176ms.

idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME5/build-release$ ./TME5 -m ThreadPerPixel -W 2000 -H 2000
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode ThreadPerPixel
terminate called after throwing an instance of 'std::system_error'
  what():  Resource temporarily unavailable
Abandon (core dumped)
```

### Question 4
```
Implemented `void renderThreadPerRow(const Scene& scene, Image& img)` in Renderer.

$ ./TME5 -m ThreadPerRow
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode ThreadPerRow
Total time 280ms.

10X plus rapide que la version séquentiel
```

### Question 5
```
On est mieux que la version thread per Row (16 thread on arrive à faire celui de Rowthread 2000)

Implemented `void renderThreadManual(const Scene& scene, Image& img, int nbthread)` in Renderer.

se$ ./TME5 -m ThreadManual
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode ThreadManual, threads 4
Total time 520ms.
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME5/build-release$ ./TME5 -m ThreadPerRow
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode ThreadPerRow
Total time 277ms.
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME5/build-release$ ./TME5 -m ThreadManual -n 16
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode ThreadManual, threads 16
Total time 289ms.
idris-achabou@idris-achabou-LOQ-15IAX9E:~/Documents/M1_STL/PSCR/TME5/build-release$ ./TME5 -m ThreadManual -n 32
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode ThreadManual, threads 32
Total time 274ms.
```


## With Thread Pool

### Question 6
Queue class: blocking by default, can switch to non-blocking.

### Question 7
Pool class: constructor with queue size, start, stop.
Job abstract class with virtual run().

### Question 8
PixelJob: derives from Job, captures ?TODO?

renderPoolPixel: 

Mode "-m PoolPixel" with -n.

mesures

### Question 9
LineJob: derives from Job, captures TODO

renderPoolRow: 

Mode "-m PoolRow -n nbthread".

mesures

### Question 10
Best:

## Bonus

### Question 11

pool supportant soumission de lambda.