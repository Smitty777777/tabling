#ifndef OSMI_TABLE_HPP
#define OSMI_TABLE_HPP
#include <array>
#include <compare>
#include <concepts>
#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
namespace osmi{

  template <typename T>
  struct OType{
    using value_type = T;
    T value{};
    auto operator<=>(const OType<T>& rhs) const { return value <=> rhs.value; }
    bool operator==(const OType<T>& rhs) const { return value == rhs.value; }
    static constexpr bool valid_t = false;
    friend std::ostream& operator<<(std::ostream& out, const OType<T>&){
      out << "otype_unkown"; return out;
    }
  };

  struct OString : OType<std::basic_string<char>> {
    static constexpr bool valid_t = true;
    friend std::ostream& operator<<(std::ostream& out, const OString&){
      out << "otype_float_64"; return out;
    }
  };

  template<typename T> concept Numeric = std::integral<T> || std::floating_point<T>;
  template<Numeric numeric_t> struct OArithmetic : public OType<numeric_t>{
    static constexpr bool valid_t = true;
  };
  struct OBool : OArithmetic<bool>{
    friend std::ostream& operator<<(std::ostream& out, const OBool&){
      out << "otype_bool_1"; return out;
    }
  };
  struct OInteger : OArithmetic<int>{
    friend std::ostream& operator<<(std::ostream& out, const OInteger&){
      out << "otype_int_64"; return out;
    }
  };
  struct OFloat : OArithmetic<double>{
    friend std::ostream& operator<<(std::ostream& out, const OFloat&){
      out << "otype_float_64"; return out;
    }
  };


  template <typename T>
  concept OValue = requires {
    typename T::value_type;
    requires std::derived_from<T, OType<typename T::value_type>>;
    requires T::valid_t;
  };

  template <typename To, OValue From>
  requires std::is_object_v<To> &&
           std::constructible_from<To, const typename From::value_type&>
  constexpr To ConvertToStd(const From& from){
    return static_cast<To>(from.value);
  }

  template <typename T>
  concept ColumnItem = requires {
    typename T::type_;
    requires OValue<typename T::type_>;
    requires std::same_as<decltype(T::name_), std::string_view>;
  };

  class table{
  public:
    template<typename Name, OValue Type>
    requires std::same_as<std::string_view, Name>
    struct column_item {
      Name name_{};
      using type_ = Type;
    };
    template<ColumnItem... Columns>
    class column_traits {
      public:
        column_traits() = default;

        explicit column_traits(const Columns&... columns)
          requires (sizeof...(Columns) > 0)
          : headers_{columns.name_...} {}

        template <std::size_t index> auto& get(){
          static_assert(index < size_, "out of range");
          return std::get<index>(tuple_);
        }

        template <std::size_t index> const auto& get() const{
          static_assert(index < size_, "out of range");
          return std::get<index>(tuple_);
        }
      private:
        static constexpr std::size_t size_ = sizeof...(Columns);
        std::tuple<typename Columns::type_...> tuple_{};
        std::array<std::string_view, size_> headers_{};
    };
  };
}
#endif
