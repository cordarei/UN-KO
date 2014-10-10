#include <iostream>

#include <experimental/optional>

#include "picojson/picojson.h"


namespace json {
  enum class json_type { null, boolean, number, string, array, object };

  class value;

  struct null_value {};
  typedef bool boolean_value;
  typedef double number_value;
  typedef std::string string_value;
  typedef std::vector<value> array_value;
  typedef std::map<std::string, value> object_value;

  class value {
    static value null;
  public:
    value();
    explicit value(boolean_value b);
    explicit value(number_value n);
    explicit value(string_value s);
    explicit value(array_value a);
    explicit value(object_value o);

    const value & operator[](std::string) const;
    const value & operator[](size_t) const;

    json_type type() const;

    boolean_value boolean() const;
    number_value number() const;
    string_value string() const;
    array_value array() const;
    object_value object() const;

    template<typename T>
    std::experimental::optional<T> get() const;
  };
}


int main() {

  picojson::value v;
  std::cin >> v;
  if (std::cin.fail()) {
    std::cerr << picojson::get_last_error() << std::endl;
    return 1;
  }


  size_t sentence_count = 0;
  auto & sentences = v.get<picojson::object>()["sentences"].get<picojson::array>();
  for (auto & s : sentences) {
    ++sentence_count;
    std::cout << "sentence "
              << sentence_count
              << " size: "
              << s.get<picojson::object>()["words"].get<picojson::array>().size()
              << std::endl;
  }

  return 0;
}
