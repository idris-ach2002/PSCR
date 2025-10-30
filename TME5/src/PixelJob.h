#pragma once

#include "Job.h"
#include "Image.h"
#include "Ray.h"
#include "Scene.h"
#include <thread>

namespace pr {

class PixelJob : public Job {
  const Scene &scene;
  Image &img;
  int x;
  int y;

public:
  PixelJob(const Scene &scene_, Image &img_, int x_, int y_) : scene(scene_), img(img_), x(x_), y(y_) {}
  void run() {
    // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
    // on tire un rayon de l'observateur vers chacun de ces points
    const Scene::screen_t &screen = scene.getScreenPoints();

    // pour chaque pixel, calculer sa couleur

    // le point de l'ecran par lequel passe ce rayon
    auto &screenPoint = screen[y][x];
    // le rayon a inspecter
    Ray ray(scene.getCameraPos(), screenPoint);

    int targetSphere = scene.findClosestInter(ray);

    if (targetSphere != -1) {
      const Sphere &obj = scene.getObject(targetSphere);
      // pixel prend la couleur de l'objet
      Color finalcolor = scene.computeColor(obj, ray);
      // mettre a jour la couleur du pixel dans l'image finale.
      img.pixel(x, y) = finalcolor;
    }
  }
  ~PixelJob() {}
};
}