#pragma once

template <class cT, class traits = std::char_traits<cT>>
class basic_nullbuf : public std::basic_streambuf<cT, traits> {
  typename traits::int_type overflow(typename traits::int_type c) {
    return traits::not_eof(c); // indicate success
  }
};

template <class cT, class traits = std::char_traits<cT>>
class basic_onullstream : public std::basic_ostream<cT, traits> {
public:
  basic_onullstream()
      : std::basic_ios<cT, traits>(&m_sbuf),
        std::basic_ostream<cT, traits>(&m_sbuf) {
    this->init(&m_sbuf);
  }

  static basic_onullstream<cT, traits>& instance() {
    static basic_onullstream<cT, traits> inst{};
    return inst;
  };

private:
  basic_nullbuf<cT, traits> m_sbuf;
};

typedef basic_onullstream<char> onullstream;
typedef basic_onullstream<wchar_t> wonullstream;
