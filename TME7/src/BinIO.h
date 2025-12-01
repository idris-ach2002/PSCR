// BinIO.h
#pragma once

#include "Graph.h"
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

struct NodeHeader {
  size_t id;
  size_t child_count;
};


/**
 * Sérialise un noeud et le sous graphe qu'il permet d'atteindre dans le fichier.
 * Arguments :
 * - fd : descripteur de fichier ouvert en écriture
 * - node : pointeur vers le Vertex à sérialiser
 * - ptr_to_offset : table de hash associant aux pointeurs de Vertex les offsets dans le fichier où ils ont été sérialisés
 * Retourne l'offset dans le fichier où le noeud a été sérialisé.
 */
off_t serialize(int fd, const Vertex *node,
                std::unordered_map<const Vertex *, off_t> &ptr_to_offset) {
  // 1. Tester le map; si présent rendre la valeur.

  auto it = ptr_to_offset.find(node);
  if (it != ptr_to_offset.end()) {
    return it->second;
  }

  // 2. Sinon, on doit créer un nouveau noeud, en fin du fichier.
  // Seek à l'offset de fin, noter cette position : c'est celle du nouveau noeud.
  // L'ajouter au map ptr->offset (immediatement, avant toute récursion).
  off_t fin ;
  if((fin = lseek (fd, 0, SEEK_END)) < 0) {
    perror("SEEK AT THE END a échoué");
    exit(1);
  }

  ptr_to_offset[node] = fin;

  // seek à changé l'offset dans la table des fichier ouvert dans le sytème
  // il est positioné en fin du fichier représenté par fd
  

  // 3. Au bon offset écrire le header (id, child_count)
  size_t child_count = node->children.size();

  // Prépare le header
  NodeHeader header;
  header.id = node->id;
  header.child_count = child_count;

  if (pwrite(fd, &header, sizeof(header), fin) != (ssize_t)sizeof(header)) {
    perror("pwrite header");
    exit(1);
  }

  // 4. s'assurer de faire grandir le fichier suffisamment pour loger le nouveau noeud mais pas trop.
  // Plusieurs options ici : ftruncate, ou write de zéros (puis seek), ou lseek au delà de la fin et write...

                            // ==========================================
                            // ============Allocation====================
                            // ==========================================
  // man 2 ftruncate
  /*
    the ftruncate() function shall
    cause the size of the file to be truncated to length.
  */
  // Comme on a écrit deux entier à la fin du fichier à partir de la position fin (id, child_count)
  //================     On commence à prtir    ===============

// Taille totale du bloc du nœud : header + tableau d'offsets
  off_t node_size = sizeof(NodeHeader) + child_count * sizeof(off_t);
  // pour chaque fils on luis stocke une information cruciale pour les trouver et bien
  // Leur offset dans le fichier pour le père ===> y'en a child_count fils pour le noeud courant
 
  if (ftruncate(fd, fin + node_size) < 0) {
      perror("ftruncate");
      exit(1);
  }


  // 5. en boucle sur child_count (lseek/write ou pwrite recommandé),
  // itérer sur les enfants, appeler récursivement serialize pour chaque enfant. (Attention décale le curseur de fd!)
  // Récupérer l'offset retourné, et l'écrire à la bonne position dans le fichier (après le header, dans le tableau d'offsets).
  for (size_t i = 0; i < child_count; ++i) {
    const Vertex *child = node->children[i];

    // Récursion : obtient l'offset du nœud enfant
    off_t child_off = serialize(fd, child, ptr_to_offset);

    // Position dans le fichier où écrire cet offset :
    //   début du nœud + taille du header + i * sizeof(off_t)
    off_t pos = fin + sizeof(NodeHeader) + i * sizeof(off_t);

    if (pwrite(fd, &child_off, sizeof(child_off), pos) != (ssize_t)sizeof(child_off)) {
      perror("pwrite child offset");
      exit(1);
    }
  }

  // 6. rendre l'offset du noeud nouvellement sérialisé.
  return fin;
}

/**
 * Désérialise un noeud à partir du fichier.
 * Arguments :
 * - fd : descripteur de fichier ouvert en lecture
 * - offset : position dans le fichier du noeud à désérialiser
 * - offset_to_vertex : table de hash associant aux offsets des pointeurs vers les Vertex déjà
 * désérialisés
 * - graph : le graphe dans lequel insérer les Vertex désérialisés
 * Retourne un pointeur vers le Vertex désérialisé.
 */
Vertex *deserialize(int fd, off_t offset, std::unordered_map<off_t, Vertex *> &offset_to_vertex,
                    Graph &graph) {

  // 1. Tester le map; si présent rendre la valeur.
  auto it = offset_to_vertex.find(offset);
  if(it != offset_to_vertex.end()) return it->second;

  // 2. Sinon, on doit créer/mettre à jour le noeud dans Graph.
  // Seek à l'offset demandé; lire un header (id, child_count).

  // Redondant avec pread car elle fait le job de seek en plus
  // Mais bon c'est ce qui est demandé à l'exo

  // Seek à la position du noeud passée en argument
  off_t pos = lseek (fd, offset, SEEK_SET);
  // Pratique (Intérprétation du retour)
  if(pos < 0 || pos != offset) {
    perror("SEEK AT THE given position a échoué");
    exit(1);
  }
  // Préparation du header + Lecture
  NodeHeader header;
  ssize_t r = pread(fd, &header, sizeof(NodeHeader), offset);
  if (!r || r != (ssize_t)sizeof(NodeHeader)) {
    perror("pread header");
    exit(1);
  }

  // 3. demander à Graph le Vertex correspondant à id (findNode).
  Vertex * noeud = graph.findNode(header.id);
  // 4. Mettre à jour le map offset->Vertex (avant toute récursion).
  offset_to_vertex[offset] = noeud;
  // 5. en boucle sur child_count,

  // itérer et lire les offsets des enfants (pread recommandé).
  // Pour chaque offset, appeler récursivement deserialize (Attention ça va faire bouger le curseur de fd!)
  // Ajouter les pointeurs vers enfant qui reviennent de la récursion au Vertex en construction.
  for (size_t i = 0; i < header.child_count; ++i) {
    off_t child_offset;
    size_t child_pos = pos + sizeof(NodeHeader) + i*sizeof(off_t);
    r = pread(fd, &child_offset, sizeof(off_t), child_pos);
    if (!r || r != (ssize_t)sizeof(off_t)) {
      perror("pread child offset");
      exit(1);
    }
    Vertex * child = deserialize(fd, child_offset, offset_to_vertex, graph);
    graph.addEdge(noeud->id, child->id);
  }


  // 6. rendre le Vertex construit.                    

  return noeud;
}

void writeBin(const Graph &graph, const std::string &filename) {
  int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (fd == -1) {
    perror("open output");
    exit(1);
  }

  // Skip header: N (size_t)
  // but write a placeholder for now/leave some space
  // 1. Réserver la place pour N (size_t) en début de fichier
  size_t N_placeholder = 0;
  ssize_t w = write(fd, &N_placeholder, sizeof(N_placeholder));
  if (w != (ssize_t)sizeof(N_placeholder)) {
    perror("write placeholder N");
    close(fd);
    exit(1);
  }

  // Serialize from node 0 after the header
  // preparation du hash : associe aux pointeurs de Vertex les offsets dans le fichier.
  std::unordered_map<const Vertex *, off_t> ptr_to_offset;
  // Serialize récursif par simplicité. On serialise tout ce qui est atteignable depuis 0.
  serialize(fd, graph.findNode(0), ptr_to_offset);

  // N is ptr_to_offset.size() : nombre de noeuds sérialisés
  size_t N = ptr_to_offset.size();

  // Write header at file start
  if (lseek(fd, 0, SEEK_SET) < 0) {
      perror("lseek start");
      close(fd);
      exit(1);
  }

    w = write(fd, &N, sizeof(N));
    if (w != (ssize_t)sizeof(N)) {
      perror("write N");
      close(fd);
      exit(1);
    }

    close(fd);
}

Graph parseBin(const std::string &filename) {
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) {
    perror("open input");
    exit(1);
  }

  // Read header: N nombre de noeuds du graphe (offset 0)
  size_t N;

  ssize_t r = read(fd, &N, sizeof(N));
  if (r != (ssize_t)sizeof(N)) {
    perror("read N");
    close(fd);
    exit(1);
  }

  // Create graph; on prealloue.
  Graph graph(N);

  // Offset de la racine, juste après le N
  off_t root_offset = sizeof(size_t);

  // préparation du hash : associe aux offset des pointeurs de Vertex.
  std::unordered_map<off_t, Vertex *> offset_to_vertex;
  // Deserialize, récursif par simplicité.
  deserialize(fd, root_offset, offset_to_vertex, graph);

  // ok Graph loaded !

  close(fd);

  return graph;
}