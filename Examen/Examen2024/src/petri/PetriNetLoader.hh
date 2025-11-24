#ifndef PETRINETLOADER_H
#define PETRINETLOADER_H

#include "PetriNet.hh"
#include "PTNetHandler.hh"
#include <expat.h>
#include <chrono>
#include <cstdio>
#include <iostream>

PetriNet* loadPNML(const std::string& filename) {
    PetriNet* net = new PetriNet();

    auto start_time = std::chrono::steady_clock::now();

    XML_Parser parser = XML_ParserCreate(nullptr);
    PTNetHandler handler(net);

    XML_SetUserData(parser, &handler);
    XML_SetElementHandler(parser, &PTNetHandler::startElement, &PTNetHandler::endElement);
    XML_SetCharacterDataHandler(parser, &PTNetHandler::characters);

    FILE* in = fopen(filename.c_str(), "r");
    if (!in) {
        std::cerr << "Failed to open PNML file: " << filename << std::endl;
        delete net;
        return nullptr;
    }

    char buf[BUFSIZ];
    bool done = false;
    do {
        size_t len = fread(buf, 1, sizeof(buf), in);
        done = len < sizeof(buf);
        if (XML_Parse(parser, buf, (int)len, done) == XML_STATUS_ERROR) {
            std::cerr << "XML Parse error: " << XML_ErrorString(XML_GetErrorCode(parser))
                      << " at line " << XML_GetCurrentLineNumber(parser) << std::endl;
            XML_ParserFree(parser);
            fclose(in);
            delete net;
            return nullptr;
        }
    } while (!done);

    fclose(in);
    XML_ParserFree(parser);

    auto end_time = std::chrono::steady_clock::now();
    auto parse_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Parsed PNML file: " << filename << std::endl;
    std::cout << "Parsing took " << parse_duration << " ms." << std::endl;
    std::cout << "Petri net contains "
              << net->getPlaceCount() << " places, "
              << net->getTransitionCount() << " transitions."
              << std::endl;

    return net;
}

#endif // PETRINETLOADER_H
