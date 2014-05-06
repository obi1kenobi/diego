#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include <sstream>

class JsonSerializer {
  public:
    JsonSerializer();

    std::string Get() {
        return _os.str();
    }

    void Indent();

    void Open();

    void Close();

    void AddKeyValue(const std::string &key, const std::string &value);

 private:
    int _level;
    std::ostringstream _os;
};

#endif // JSON_SERIALIZER_H
