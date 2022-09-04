#include <rlottie.h>

#include <cstdio>
#include<iostream>
#include<string>
#include<vector>
#include<array>
#include <iostream>
#include <format>
#include <string>
#ifndef _WIN32
#include<libgen.h>
#else
#include <windows.h>
#include <stdlib.h>
#endif

#include "png/fpng.h"


class PngSaver {
public:
    explicit PngSaver(const std::string &fileName , const uint32_t width,
                        const uint32_t height, const uint32_t delay = 2)
    {
        fpng::fpng_init();
    }

    void saveFrame(rlottie::Surface &surface, std::string const& fileName)
    {
        argbTorgba(surface);
        uint32_t num_chans = 4;

        const void *pImage = reinterpret_cast<const void*>(surface.buffer());
        auto numButes = surface.bytesPerLine();
        bool res = fpng::fpng_encode_image_to_file(fileName.data(), pImage, surface.width(), surface.height(), num_chans);
        // sprintf(str, idx)
        // Png name should come from signature and be of the format: "<name>%d.png"
        
    }
    void argbTorgba(rlottie::Surface &s)
    {
        auto     bgColorR = 0xFF;
        auto     bgColorG = 0xFF;
        auto     bgColorB = 0xFF;
        uint8_t *buffer = reinterpret_cast<uint8_t *>(s.buffer());
        uint32_t totalBytes = s.height() * s.bytesPerLine();

        for (uint32_t i = 0; i < totalBytes; i += 4) {
           unsigned char a = buffer[i+3];
           // compute only if alpha is non zero
           if (a) {
               unsigned char r = buffer[i+2];
               unsigned char g = buffer[i+1];
               unsigned char b = buffer[i];

               if (a != 255) { //un premultiply
                   unsigned char r2 = (unsigned char) ((float) bgColorR * ((float) (255 - a) / 255));
                   unsigned char g2 = (unsigned char) ((float) bgColorG * ((float) (255 - a) / 255));
                   unsigned char b2 = (unsigned char) ((float) bgColorB * ((float) (255 - a) / 255));
                   buffer[i] = r + r2;
                   buffer[i+1] = g + g2;
                   buffer[i+2] = b + b2;

               } else {
                 // only swizzle r and b
                 buffer[i] = r;
                 buffer[i+2] = b;
               }
           } else {
               buffer[i+2] = bgColorB;
               buffer[i+1] = bgColorG;
               buffer[i] = bgColorR;
           }
        }
    }
};

class App {
public:
    int render(uint32_t w, uint32_t h)
    {
        
        auto player = rlottie::Animation::loadFromFile(fileName);
        if (!player) return help();

        auto buffer = std::unique_ptr<uint32_t[]>(new uint32_t[w * h]);
        size_t frameCount = player->totalFrame();

        PngSaver builder(pngName.data(), w, h);
        for (size_t i = 0; i < frameCount ; i++) {
            rlottie::Surface surface(buffer.get(), w, h, w * 4);
            player->renderSync(i, surface);

            /////////////////
            //printf(argv);
            char outFileName[256];
            sprintf(outFileName, pngName.c_str(), static_cast<int>(i));
            std::string outFileNameStr = std::string(outFileName);
            builder.saveFrame(surface, outFileNameStr);
        }
        return result();
    }

    int setup(int argc, char **argv, size_t *width, size_t *height)
    {
        char *path{nullptr};
        char *pngPath{nullptr};

        *width = *height = 200;   //default png size

        if (argc != 4) return help();
        // Lottie file
        path = argv[1];

        // resolution
        char tmp[20];
        char *x = strstr(argv[2], "x");
        if (x) {
            snprintf(tmp, x - argv[2] + 1, "%s", argv[2]);
            *width = atoi(tmp);
            snprintf(tmp, sizeof(tmp), "%s", x + 1);
            *height = atoi(tmp);
        }
        
        // Png out path
        pngPath = argv[3];

        std::array<char, 5000> memory;

#ifdef _WIN32
        path = _fullpath(memory.data(), path, memory.size());
#else
        path = realpath(path, memory.data());
#endif
        fileName = std::string(path);
        pngName = std::string(pngPath);

        if (!jsonFile()) return help();

        return 0;
    }

private:
    std::string basename(const std::string &str)
    {
        return str.substr(str.find_last_of("/\\") + 1);
    }

    bool jsonFile() {
        std::string extn = ".json";
        if ( fileName.size() <= extn.size() ||
             fileName.substr(fileName.size()- extn.size()) != extn )
            return false;

        return true;
    }

    int result() {
        std::cout<<"Generated PNG files : "<< pngName <<std::endl;
        return 0;
    }

    int help() {
        std::cout<<"Usage: \n   lottie2png [lottieFileName] [Resolution] [PngOutPath] \n\nExample: $ lottie2png input.json 200x200 out%d.png\n\n";
        return 1;
    }

private:
    std::string fileName;
    std::string pngName;
};

int
main(int argc, char **argv)
{
    App app;
    size_t w, h;

    if (app.setup(argc, argv, &w, &h)) return 1;
    app.render(w, h);

    return 0;
}
