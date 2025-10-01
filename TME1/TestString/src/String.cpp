#include "String.h"
#include "strutil.h"

namespace pr
{

  // TODO: Implement constructor e.g. using initialization list
  String::String (const char *s)
  {
    std::cout << "String constructor called for: " << s << std::endl;
    data = newcopy(s);
  }

  String::~String ()
  {
    std::cout << "String destructor called for: " << (data ? data : "(null)")
        << std::endl;
    delete[] data;
  }

  // TODO : add other operators and functions
  //Copy constructor
  String::String(const String & other) {
    std::cout << "String Copy constructor called for: " << other << std::endl;
    data = newcopy(other.data);
  }

  //Operateur d'affectation (Copy assign)
  String & String::operator=(const String & other) {
    std::cout << "String Copy assignement called for: " << other << std::endl;
    //On s'assure de ne pas faire des cookies comme 
    // chaine = chaine
    if(this != &other) {
      // On libère l'espace créé pour la chaine de l'objet appelant
      delete data;
      //On alloue un nouveau espace pui on recopie la chaine dans cet espace
      data = newcopy(other.data);
    }
    return *this;
  }

  //Constructeur de déplacement (move constructor)
  String::String(String && other) noexcept{
    std::cout << "String Move constructor called for: " << other << std::endl;
    data = other.data;
    other.data = nullptr;
  }

  //Opération d'affectation : Move
  String & String::operator=(String && other) noexcept{
    std::cout << "String Move assignement called for: " << other << std::endl;
    if(this != &other) {
      data = other.data; // ou this->data = p.data;
      other.data = nullptr;
    }
    return *this;
  }

  //Opérateur < (Vérifier si l'objet appelant est < par rapport à la chaîne passée En ASCII)
  bool String::operator<(const String& other) const {
    return compare(data, other.data) < 0;
  }

  //Opérateur d'insertion (Stockage dans un flux de sortie)
  std::ostream& operator<<(std::ostream& os, const String& str) {
    os << str.data;
    return os;
  }

  // Symmetric equality
  bool operator==(const String& a, const String& b) {
    return compare(a.data, b.data) == 0;
  }

  String operator+(const String& a, const String& b) {
    return String(newcat(a.data, b.data));
  }
}// namespace pr

