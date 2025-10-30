#pragma once

#include "Scene.h"
#include "Image.h"
#include "Ray.h"
#include "PixelJob.h"
#include "LineJob.h"
#include "Pool.h"
#include <thread>

namespace pr {

// Classe pour rendre une scène dans une image
class Renderer {
public:
    // Rend la scène dans l'image
    void render(const Scene& scene, Image& img) {
        // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
        // on tire un rayon de l'observateur vers chacun de ces points
        const Scene::screen_t& screen = scene.getScreenPoints();

        // pour chaque pixel, calculer sa couleur
        for (int x = 0; x < scene.getWidth(); x++) {
            for (int y = 0; y < scene.getHeight(); y++) {
                // le point de l'ecran par lequel passe ce rayon
                auto& screenPoint = screen[y][x];
                // le rayon a inspecter
                Ray ray(scene.getCameraPos(), screenPoint);

                int targetSphere = scene.findClosestInter(ray);

                if (targetSphere == -1) {
                    // keep background color
                    continue;
                } else {
                    const Sphere& obj = scene.getObject(targetSphere);
                    // pixel prend la couleur de l'objet
                    Color finalcolor = scene.computeColor(obj, ray);
                    // mettre a jour la couleur du pixel dans l'image finale.
                    img.pixel(x, y) = finalcolor;
                }
            }
        }
    }

    void renderThreadPerPixel(const Scene & scene, Image& img) {
         // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
        // on tire un rayon de l'observateur vers chacun de ces points
        const Scene::screen_t& screen = scene.getScreenPoints();
        std::vector<std::thread> threads;
        threads.reserve(scene.getWidth()*scene.getHeight());

        // pour chaque pixel, calculer sa couleur
        for (int x = 0; x < scene.getWidth(); x++) {
            for(int y = 0; y < scene.getHeight(); y++) {
                threads.emplace_back([&, x, y]() {
                     // le point de l'ecran par lequel passe ce rayon
                auto& screenPoint = screen[y][x];
                // le rayon a inspecter
                Ray ray(scene.getCameraPos(), screenPoint);

                int targetSphere = scene.findClosestInter(ray);

                if (targetSphere != -1) {
                    const Sphere& obj = scene.getObject(targetSphere);
                    // pixel prend la couleur de l'objet
                    Color finalcolor = scene.computeColor(obj, ray);
                    // mettre a jour la couleur du pixel dans l'image finale.
                    img.pixel(x, y) = finalcolor;
                }
                });
            }
        }

        for(auto &it : threads) {
            it.join();
        }
    }

    void renderThreadPerRow(const Scene & scene, Image& img) {
         // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
        // on tire un rayon de l'observateur vers chacun de ces points
        const Scene::screen_t& screen = scene.getScreenPoints();
        std::vector<std::thread> threads;
        threads.reserve(scene.getWidth());

        // pour chaque pixel, calculer sa couleur
        for (int x = 0; x < scene.getWidth(); x++) {
            threads.emplace_back([&, x] {
                for (int y = 0; y < scene.getHeight(); y++) {
                    // le point de l'ecran par lequel passe ce rayon
                    auto& screenPoint = screen[y][x];
                    // le rayon a inspecter
                    Ray ray(scene.getCameraPos(), screenPoint);

                    int targetSphere = scene.findClosestInter(ray);

                    if (targetSphere == -1) {
                        // keep background color
                        continue;
                    } else {
                        const Sphere& obj = scene.getObject(targetSphere);
                        // pixel prend la couleur de l'objet
                        Color finalcolor = scene.computeColor(obj, ray);
                        // mettre a jour la couleur du pixel dans l'image finale.
                        img.pixel(x, y) = finalcolor;
                    }
                }
            });
        }

        for(auto &it : threads) {
            it.join();
        }
    }

    void renderThreadManual(const Scene & scene, Image& img, int nbThread) {
         // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
        // on tire un rayon de l'observateur vers chacun de ces points
        const Scene::screen_t& screen = scene.getScreenPoints();
        std::vector<std::thread> threads;
        threads.reserve(nbThread);

        //Soit la hauteur est un multiple de nbThread 
        // ou le dernier thread fera le reste
        int taillePartie = scene.getHeight() / nbThread;


       for(int i = 0; i < nbThread; i++) {
            threads.emplace_back([&, i] {
                // le dernier thread fera le reste (Si la hauteur n'est pas un multiple de nbThread)
                int minY = i*taillePartie;
                int maxY = (i+1)*taillePartie;
                if(i == nbThread - 1) maxY = scene.getHeight();

                // pour chaque pixel, calculer sa couleur
                for (int x = 0; x < scene.getWidth(); x++) {
                    for (int y = minY; y < maxY; ++y) {
                        // le point de l'ecran par lequel passe ce rayon
                        auto& screenPoint = screen[y][x];
                        // le rayon a inspecter
                        Ray ray(scene.getCameraPos(), screenPoint);

                        int targetSphere = scene.findClosestInter(ray);

                        if (targetSphere == -1) {
                            // keep background color
                            continue;
                        } else {
                            const Sphere& obj = scene.getObject(targetSphere);
                            // pixel prend la couleur de l'objet
                            Color finalcolor = scene.computeColor(obj, ray);
                            // mettre a jour la couleur du pixel dans l'image finale.
                            img.pixel(x, y) = finalcolor;
                        }
                    }
                } 
            });
       }

        for(auto &it : threads) {
            it.join();
        }
    }
    void renderPoolPixel(const Scene & scene, Image& img, int nbThread) {
        Pool pool(scene.getWidth() * scene.getHeight());
        pool.start(nbThread);
        for (int x = 0; x < scene.getWidth(); x++) {
            for (int y = 0; y < scene.getHeight(); y++) {
                pool.submit(new PixelJob(scene, img, x, y));
            }
        }
        pool.stop();
    }

    void renderPoolRow(const Scene & scene, Image& img, int nbThread) {
        Pool pool(scene.getHeight());
        pool.start(nbThread);
            for (int y = 0; y < scene.getHeight(); y++) {
                pool.submit(new LineJob(scene, img, y));
            }
        pool.stop();
    }
};


} // namespace pr