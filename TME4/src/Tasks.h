#ifndef TASKS_H
#define TASKS_H

#include <QImage>
#include <filesystem>
#include "BoundedBlockingQueue.h"

namespace pr {

using FileQueue = BoundedBlockingQueue<std::filesystem::path>;

const std::filesystem::path FILE_POISON{};

// load/resize/save 
void treatImage(FileQueue& fileQueue, const std::filesystem::path& outputFolder);

// permet de véhiculer les données nécessaire pour le traitement 
struct TaskData {
    std::filesystem::path file;
    QImage original;
};

extern TaskData TASK_POISON;

using ImageTaskQueue = BoundedBlockingQueue<TaskData>;
// or
//using ImageTaskQueue = BoundedBlockingQueue<TaskData>;

void reader(FileQueue& fileQueue, ImageTaskQueue& imageQueue);
void resizer(ImageTaskQueue& imageQueue, ImageTaskQueue& resizedQueue);
void saver(ImageTaskQueue& resizedQueue, const std::filesystem::path& outputFolder);

} // namespace pr

#endif // TASKS_H