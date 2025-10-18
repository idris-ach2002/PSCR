#include "Tasks.h"
#include "util/ImageUtils.h"
#include "util/thread_timer.h"
#include <thread>
#include <sstream>

namespace pr {

void treatImage(FileQueue& fileQueue, const std::filesystem::path& outputFolder) {
    // measure CPU time in this thread
    pr::thread_timer timer;
    
    while (true) {
        std::filesystem::path file = fileQueue.pop();
        if (file == pr::FILE_POISON) break; // poison pill
        QImage original = pr::loadImage(file);
        if (!original.isNull()) {
            QImage resized = pr::resizeImage(original);
            std::filesystem::path outputFile = outputFolder / file.filename();
            pr::saveImage(resized, outputFile);
        }
    }

    // trace
    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (treatImage): " << timer << " ms CPU" << std::endl;
    std::cout << ss.str();
}

/*
 pr::findImageFiles(opts.inputFolder, [&](const std::filesystem::path& file) {
            QImage original = pr::loadImage(file);
            if (!original.isNull()) {
                QImage resized = pr::resizeImage(original);
                std::filesystem::path outputFile = opts.outputFolder / file.filename();
                pr::saveImage(resized, outputFile);
            }
        });

          std::filesystem::path file = fileQueue.pop();
        if (file == pr::FILE_POISON) break; // poison pill
        QImage original = pr::loadImage(file);
        if (!original.isNull()) {
            QImage resized = pr::resizeImage(original);
            std::filesystem::path outputFile = outputFolder / file.filename();
            pr::saveImage(resized, outputFile);
        }
*/

TaskData TASK_POISON {pr::FILE_POISON};

void reader(FileQueue& fileQueue, ImageTaskQueue& imageQueue) {
    pr::thread_timer timer;
    while (true) {
      std::filesystem::path f = fileQueue.pop();
        if (f == pr::FILE_POISON) break; // poison pill
        QImage original = pr::loadImage(f);
        TaskData tache = {f, original};
        imageQueue.push(tache);
    }
    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (reader): " << timer << " ms CPU" << std::endl;
    std::cout << ss.str();
}

void resizer(ImageTaskQueue& imageQueue, ImageTaskQueue& resizedQueue) {
    pr::thread_timer timer;
    while (true) {
        TaskData task = imageQueue.pop();
        if(task.file == pr::FILE_POISON) break;
        if (!task.original.isNull()) {
            QImage resized = pr::resizeImage(task.original);
            TaskData tache = {task.file, resized};
            resizedQueue.push(tache);
            //pr::saveImage(resized, outputFile);
        }
    }
    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (resizer): " << timer << " ms CPU" << std::endl;
    std::cout << ss.str();
}

void saver(ImageTaskQueue& resizedQueue, const std::filesystem::path& outputFolder) {
    pr::thread_timer timer;
    while (true) {
        TaskData task = resizedQueue.pop();
        if(task.file == pr::FILE_POISON) break;
        if (!task.original.isNull()) {
            std::filesystem::path outputFile = outputFolder / task.file.filename();
            pr::saveImage(task.original, outputFile);
        }
    }
    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (saver): " << timer << " ms CPU" << std::endl;
    std::cout << ss.str();
}



} // namespace pr