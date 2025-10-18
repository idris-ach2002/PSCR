// ImageResizerApp.cpp
#include <QCoreApplication>  // For non-GUI Qt app

#include <iostream>
#include <filesystem>
#include <thread>
#include <vector>
#include <utility>
#include <chrono>
#include <cstdlib>
#include <sstream>

#include "util/CLI11.hpp" // Header only lib for argument parsing

#include "util/ImageUtils.h"
#include "BoundedBlockingQueue.h"
#include "Tasks.h"
#include "util/thread_timer.h"
#include "util/processRSS.h"

struct Options {
    std::filesystem::path inputFolder = "input_images/";
    std::filesystem::path outputFolder = "output_images/";
    std::string mode = "resize";
    int num_threads = 4;
    int nbread = 1;
    int nbresize = 1;
    int nbwrite = 1; 
    int queue_size = 10;

  friend std::ostream &operator<<(std::ostream &os, const Options &opts) {
    os << "input folder '" << opts.inputFolder.string() 
       << "', output folder '" << opts.outputFolder.string() 
       << "', mode '" << opts.mode 
       << "', nthreads " << opts.num_threads;
    return os;
  }
};

int parseOptions(int argc, char *argv[], Options& opts);

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);  // Initialize Qt for image format plugins

    Options opts;
    // rely on CLI11 to parse command line options
    // see implementation of parseOptions below main function
    int code = parseOptions(argc, argv, opts);
    if (code != 0) {
        return code;
    }

    std::cout << "Image resizer starting with " << opts << std::endl;

    auto start_time = std::chrono::steady_clock::now();
    pr::thread_timer main_timer;

    if (opts.mode == "resize") {
        // Single-threaded: direct load/resize/save in callback
        pr::findImageFiles(opts.inputFolder, [&](const std::filesystem::path& file) {
            QImage original = pr::loadImage(file);
            if (!original.isNull()) {
                QImage resized = pr::resizeImage(original);
                std::filesystem::path outputFile = opts.outputFolder / file.filename();
                pr::saveImage(resized, outputFile);
            }
        });
    } else if (opts.mode == "pipe") {
        // 1. Single-threaded pipeline: file discovery -> treatImage (load/resize/save)
        pr::FileQueue fileQueue(opts.queue_size);

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
    }  else if (opts.mode == "pipe_mt") {
        std::vector<std::thread> threads;
        threads.reserve(opts.num_threads);

        // 1. Single-threaded pipeline: file discovery -> treatImage (load/resize/save)
        pr::FileQueue fileQueue(10);

        // 2. Start the working threads
        for(int i = 0; i < opts.num_threads; i++) {
            threads.emplace_back(pr::treatImage, std::ref(fileQueue), std::ref(opts.outputFolder));
        }

        // 3. Populate file queue synchronously
        pr::findImageFiles(opts.inputFolder, [&](const std::filesystem::path& file) {
            fileQueue.push(file);
        });

        // 4. Push N poison pill
        for(int i = 0; i < opts.num_threads; i++) {
            fileQueue.push(pr::FILE_POISON);
        }

        // 5. Join the worker thread
        for(auto &t : threads) {
            t.join();
        }

    } else if (opts.mode == "mt_pipeline") {
        // Créer une file bloquante  à alimenter par des chemin de fichier (Main)
        pr::FileQueue fileQueue(10);

        // La file du reader
        pr::ImageTaskQueue imageQueue(10);  
        // 2. démarer le reader
        std::thread reader(pr::reader, std::ref(fileQueue), std::ref(imageQueue));

        // la file du resizer
        pr::ImageTaskQueue resizerQueue(10);  
        //démarer le resizer
        std::thread resizer(pr::resizer, std::ref(imageQueue), std::ref(resizerQueue));

        //démarer le saver
        std::thread saver(pr::saver, std::ref(resizerQueue), std::ref(opts.outputFolder));


        // 3. Populate file queue synchronously (Alimentation par le main)
        pr::findImageFiles(opts.inputFolder, [&](const std::filesystem::path& file) {
            fileQueue.push(file);
        });

        // 4. Push poison pill (After processing all files) (arreter le reader qui lit depuis la file du Main)
        fileQueue.push(pr::FILE_POISON);

        // 5. attendre le reader (Il va charger toutes les images)
        reader.join();

        //areter le resizer (Avec le poison) (il lit depuis la file de reader)
        imageQueue.push(pr::TASK_POISON);

         // 5. attendre le resizer (Il va redimensionner toutes les images)
        resizer.join();

         //areter le saver (Avec le poison)
        resizerQueue.push(pr::TASK_POISON);

        // 5. attendre le saver (Il va stocker toutes les images redimensionnées)
        saver.join();

        
    } else if (opts.mode == "mt_pipeline_setup") {
        int nb_reader = opts.nbread;
        int nb_resizer = opts.nbresize; 
        int nb_saver = opts.nbwrite;
        int queue_size = opts.queue_size;

        std::vector<std::thread> readers;
        readers.reserve(nb_reader);

        std::vector<std::thread> resizers;
        resizers.reserve(nb_resizer);

        std::vector<std::thread> savers;
        savers.reserve(nb_saver);

        // Créer une file bloquante  à alimenter par des chemin de fichier (Main)
        pr::FileQueue fileQueue(queue_size);

        // La file de reader(s)
        pr::ImageTaskQueue imageQueue(queue_size);  

        // 2. démarer le(s) reader(s)
        for(int i = 0; i < nb_reader; i++) {
            readers.emplace_back(pr::reader, std::ref(fileQueue), std::ref(imageQueue));
        }

        // la file du resizer(s)
        pr::ImageTaskQueue resizerQueue(queue_size);  
        //démarer le(s) resizer(s)
        for(int i = 0; i < nb_resizer; i++) {
            resizers.emplace_back(pr::resizer, std::ref(imageQueue), std::ref(resizerQueue));
        }

        //démarer le(s) saver(s)
        for(int i = 0; i < nb_saver; i++) {
            savers.emplace_back(pr::saver, std::ref(resizerQueue), std::ref(opts.outputFolder));
        }

        // 3. Populate file queue synchronously (Alimentation par le main)
        pr::findImageFiles(opts.inputFolder, [&](const std::filesystem::path& file) {
            fileQueue.push(file);
        });

        // 4. Push N poison pill (After processing all files) (arreter le(s) reader(s) qui lit depuis la file du Main)
        for(int i = 0; i < nb_reader; i++)
            fileQueue.push(pr::FILE_POISON);

        // 5. attendre le(s) reader(s) (Il va charger toutes les images)
        for(auto &r : readers) 
            r.join();
        

        //areter le resizer (Avec le(s) N poison) (il lit depuis la file de reader)
        for(int i = 0; i < nb_resizer; i++)
            imageQueue.push(pr::TASK_POISON);

         // 5. attendre le resizer (Il va redimensionner toutes les images)
        for(auto &r : resizers)
            r.join();

         //areter le(s) saver(s) (Avec le poison)
        for(int i = 0; i < nb_saver; i++)
            resizerQueue.push(pr::TASK_POISON);

        // 5. attendre le saver (Il va stocker toutes les images redimensionnées)
        for(auto &s : savers) 
            s.join();
        
    
    } else {
        std::cerr << "Unknown mode '" << opts.mode << "'. Supported modes: resize, pipe" << std::endl;
        return 1;
    }

    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (main): " << main_timer << " ms CPU" << std::endl;
    std::cout << ss.str();

    auto end = std::chrono::steady_clock::now();
    std::cout << "Total runtime (wall clock): " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count() << " ms" << std::endl;

    // Report memory usage at the end
    std::cout << "Memory usage: " << process::getResidentMemory() << std::endl;

    // Report total CPU time across all timers
    std::cout << "Total CPU time across all threads: " << pr::thread_timer::getTotalCpuTimeMs() << " ms" << std::endl;

    return 0;
}

int parseOptions(int argc, char *argv[], Options& opts) {
    Options default_opts;  // Use defaults from struct for CLI11 help
    CLI::App cli_app("Image Resizer Application. Scales down images in input folder, writes to output folder.");

    cli_app.add_option("-i,--input", opts.inputFolder, "Input folder containing images")
        ->check(CLI::ExistingDirectory)
        ->default_str(default_opts.inputFolder.string());

    cli_app.add_option("-o,--output", opts.outputFolder, "Output folder for resized images (will be created if needed)")
        ->default_str(default_opts.outputFolder.string());

    cli_app.add_option("-m,--mode", opts.mode, "Processing mode")
        ->check(CLI::IsMember({"resize", "pipe" , "pipe_mt", "mt_pipeline", "mt_pipeline_setup"})) // TODO : add modes
        ->default_str(default_opts.mode);

    cli_app.add_option("-n,--nthreads", opts.num_threads, "Number of threads")
        ->check(CLI::PositiveNumber)
        ->default_val(default_opts.num_threads);

    cli_app.add_option("-r,--nbread", opts.nbread, "Number of readers")
        ->check(CLI::PositiveNumber)
        ->default_val(default_opts.nbread);

    cli_app.add_option("-s,--nbresize", opts.nbresize, "Number of resizers")
        ->check(CLI::PositiveNumber)
        ->default_val(default_opts.nbresize);

    cli_app.add_option("-w,--nbwrite", opts.nbwrite, "Number of writers")
        ->check(CLI::PositiveNumber)
        ->default_val(default_opts.nbwrite);

    cli_app.add_option("-q,--queue_size", opts.queue_size, "Size of the queues between pipeline stages")
        ->check(CLI::PositiveNumber)
        ->default_val(default_opts.queue_size);

    try {
        cli_app.parse(argc, argv);
    } catch (const CLI::CallForHelp &e) {
        cli_app.exit(e);
        std::exit(0);
    } catch (const CLI::ParseError &e) {
        return cli_app.exit(e);
    }

    if (!std::filesystem::exists(opts.outputFolder)) {
        if (!std::filesystem::create_directories(opts.outputFolder)) {
            std::cerr << "Failed to create the output folder." << std::endl;
            return 1;
        }
    }

    return 0;
}
