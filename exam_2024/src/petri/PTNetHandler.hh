#ifndef PTNETHANDLER_H
#define PTNETHANDLER_H

#include "PetriNet.hh"
#include <expat.h>
#include <stack>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>

class PTNetHandler {
    // Context stack
    std::stack<void*> stack;

    // PetriNet object being constructed
    PetriNet* net;

    // Map from object name to <isPlace, index>
    typedef std::pair<bool, int> node_t;
    std::unordered_map<std::string, node_t> index;

    // Arcs to be patched later
    struct arc_t {
        std::string source;
        std::string target;
        int weight;
    };
    std::vector<arc_t*> topatch;

    std::string lastseen;
    bool readtext;

    long lastint;
    bool readint;
    bool inOpaqueToolSpecific;

    bool doIt;

public:
    PTNetHandler(PetriNet* net_)
        : net(net_), readtext(false), lastint(-1), readint(false), inOpaqueToolSpecific(false), doIt(false)
    {
    }

    static void characters(void* userData, const XML_Char* chars, int length) {
        PTNetHandler* tthis = (PTNetHandler*)userData;
        if (tthis->inOpaqueToolSpecific) {
            return;
        } else if (tthis->doIt) {
            if (tthis->readtext) {
                tthis->lastseen = std::string(chars, length);
            } else if (tthis->readint) {
                std::string laststr(chars, length);
                tthis->lastint = std::stol(laststr);
            }
        }
    }

    static void startElement(void* userData, const XML_Char* name, const XML_Char** atts) {
        PTNetHandler* tthis = (PTNetHandler*)userData;
        std::string baliseName(name);
        if (tthis->inOpaqueToolSpecific) {
            return;
        }

        if ("net" == baliseName) {
            for (int i = 0; atts[i] != nullptr; i += 2) {
                std::string bname = atts[i];
                std::string bval = atts[i + 1];

                if ("id" == bname) {
                  // ignore net name
                   // tthis->net->setName(bval);
                } else if ("type" == bname) {
                    if ("http://www.pnml.org/version-2009/grammar/ptnet" != bval) {
                        throw std::runtime_error("Net is not a P/T net. Colors are not supported currently.");
                    }
                }
            }
            tthis->stack.push((void*)tthis->net);
        } else if ("name" == baliseName) {
            tthis->readtext = true;
        } else if ("page" == baliseName) {
            // Pages are ignored
        } else if ("place" == baliseName) {
            std::string id;
            for (int i = 0; atts[i] != nullptr; i += 2) {
                if ( ! strcmp(atts[i],"id")) {
                    id = atts[i + 1];
                    break;
                }
            }
            size_t pid = tthis->net->addPlace(id);
            tthis->index[id] = std::make_pair(true, pid);
            tthis->stack.push((void*)pid);
        } else if ("initialMarking" == baliseName) {
            tthis->readint = true;
        } else if ("inscription" == baliseName) {
            tthis->readint = true;
        } else if ("transition" == baliseName) {
            std::string id;
            for (int i = 0; atts[i] != nullptr; i += 2) {
                if (! strcmp("id",atts[i])) {
                    id = atts[i + 1];
                    break;
                }
            }
            size_t tid = tthis->net->addTransition(id);
            tthis->index[id] = std::make_pair(false, tid);
            tthis->stack.push((void*)tid);
        } else if ("arc" == baliseName) {
            std::string source;
            std::string target;
            for (int i = 0; atts[i] != nullptr; i += 2) {
                if (!strcmp("source", atts[i])) {
                    source = atts[i + 1];
                } else if (! strcmp("target", atts[i])) {
                    target = atts[i + 1];
                }
            }
            arc_t* arc = new arc_t{source, target, 1};
            tthis->stack.push(arc);
        } else if ("toolspecific" == baliseName) {
            tthis->inOpaqueToolSpecific = true;
        } else if ("text" == baliseName) {
            tthis->doIt = true;
        } else if ("graphics" == baliseName || "offset" == baliseName ||
                   "position" == baliseName || "fill" == baliseName ||
                   "line" == baliseName || "dimension" == baliseName) {
            // Skip graphics-related tags
        } else if ("pnml" == baliseName) {
            // Skip
        } else {
            std::cerr << "Unknown XML tag in source file: " << baliseName << std::endl;
        }
    }

    static void endElement(void* userData, const XML_Char* name) {
        PTNetHandler* tthis = (PTNetHandler*)userData;
        std::string baliseName(name);
        if ("toolspecific" == baliseName) {
            tthis->inOpaqueToolSpecific = false;
        } else if (tthis->inOpaqueToolSpecific) {
            return;
        } else if ("net" == baliseName) {
            tthis->stack.pop();
        } else if ("page" == baliseName) {
            return;
        } else if ("name" == baliseName) {
            tthis->readtext = false;
            tthis->lastseen = "";
        } else if ("place" == baliseName) {
            tthis->stack.pop();
        } else if ("transition" == baliseName) {
            tthis->stack.pop();
        } else if ("arc" == baliseName) {
            arc_t* arc = (arc_t*)tthis->stack.top();
            tthis->stack.pop();

            auto itSrc = tthis->index.find(arc->source);
            auto itTgt = tthis->index.find(arc->target);

            if (itSrc != tthis->index.end() && itTgt != tthis->index.end()) {
                if (itSrc->second.first) {
                    // Source is a place
                    int pindex = itSrc->second.second;
                    int tindex = itTgt->second.second;
                    tthis->net->addPreArc(tindex, pindex, arc->weight);
                } else {
                    // Source is a transition
                    int tindex = itSrc->second.second;
                    int pindex = itTgt->second.second;
                    tthis->net->addPostArc(tindex, pindex, arc->weight);
                }
                delete arc;
            } else {
                tthis->topatch.push_back(arc);
            }
        } else if ("text" == baliseName) {
            tthis->doIt = false;
        } else if ("initialMarking" == baliseName) {
            size_t pindex = (size_t)tthis->stack.top();
            tthis->net->setInitialTokens(pindex, tthis->lastint);
            tthis->readint = false;
            tthis->lastint = -1;
        } else if ("inscription" == baliseName) {
            arc_t* arc = (arc_t*)tthis->stack.top();
            arc->weight = tthis->lastint;
            tthis->readint = false;
            tthis->lastint = -1;
        } else if ("graphics" == baliseName || "offset" == baliseName ||
                   "position" == baliseName || "fill" == baliseName ||
                   "line" == baliseName || "dimension" == baliseName) {
            // Skip
        } else if ("pnml" == baliseName) {
            // Patch missing arc targets
            for (arc_t* arc : tthis->topatch) {
                auto itSrc = tthis->index.find(arc->source);
                auto itTgt = tthis->index.find(arc->target);

                if (itSrc != tthis->index.end() && itTgt != tthis->index.end()) {
                    if (itSrc->second.first) {
                        // Source is a place
                        int pindex = itSrc->second.second;
                        int tindex = itTgt->second.second;
                        tthis->net->addPreArc(tindex, pindex, arc->weight);
                    } else {
                        // Source is a transition
                        int tindex = itSrc->second.second;
                        int pindex = itTgt->second.second;
                        tthis->net->addPostArc(tindex, pindex, arc->weight);
                    }
                    delete arc;
                } else {
                    std::cerr << "Problem when linking arc: source or target node not found <"
                              << arc->source << "," << arc->target << ">" << std::endl;
                    delete arc;
                }
            }
            tthis->topatch.clear();
        } else {
            std::cerr << "Unknown XML tag in source file: " << baliseName << std::endl;
        }
    }
};

#endif // PTNETHANDLER_H
