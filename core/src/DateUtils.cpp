#include "levelup/DateUtils.hpp"

#include <charconv>
#include <chrono>
#include <cstdio>
#include <stdexcept>

namespace levelup::DateUtils {

namespace {

int parseField(const std::string& s, size_t begin, size_t len) {
  if (begin + len > s.size()) {
    throw std::invalid_argument("fecha malformada: " + s);
  }
  int value = 0;
  auto [ptr, ec] = std::from_chars(s.data() + begin, s.data() + begin + len, value);
  if (ec != std::errc() || ptr != s.data() + begin + len) {
    throw std::invalid_argument("fecha malformada: " + s);
  }
  return value;
}

std::chrono::year_month_day toChronoDate(const DateParts& parts) {
  using namespace std::chrono;
  return year{parts.year} / month{static_cast<unsigned>(parts.month)} /
         day{static_cast<unsigned>(parts.day)};
}

} // namespace

DateParts parseDate(const std::string& date) {
  // Formato estricto "YYYY-MM-DD" (10 caracteres, guiones en posicion fija)
  // en vez de un parser mas permisivo -- es el unico formato que produce y
  // consume el resto del proyecto (SQLite, QDate::toString(Qt::ISODate)).
  if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
    throw std::invalid_argument("fecha malformada: " + date);
  }
  DateParts parts{parseField(date, 0, 4), parseField(date, 5, 2), parseField(date, 8, 2)};

  auto chronoDate = toChronoDate(parts);
  if (!chronoDate.ok()) {
    throw std::invalid_argument("fecha invalida: " + date);
  }
  return parts;
}

std::string formatDate(int year, int month, int day) {
  char buf[11];
  std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
  return std::string(buf);
}

int isoWeekday(const std::string& date) {
  using namespace std::chrono;
  auto parts = parseDate(date);
  sys_days days = toChronoDate(parts);
  return static_cast<int>(weekday{days}.iso_encoding());
}

int daysInMonth(int y, int m) {
  using namespace std::chrono;
  year_month_day_last lastDay{year{y} / month{static_cast<unsigned>(m)} / last};
  return static_cast<unsigned>(lastDay.day());
}

std::string addDays(const std::string& date, int days) {
  using namespace std::chrono;
  auto parts = parseDate(date);
  sys_days base = toChronoDate(parts);
  year_month_day result = base + std::chrono::days(days);
  return formatDate(static_cast<int>(result.year()), static_cast<unsigned>(result.month()),
                     static_cast<unsigned>(result.day()));
}

} // namespace levelup::DateUtils
