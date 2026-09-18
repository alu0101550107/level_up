#include <iostream>
#include <stdexcept>
#include <string>

#include "levelup/DateUtils.hpp"

using namespace levelup;

namespace {

void check(bool cond, const char* what) {
  if (!cond) throw std::runtime_error(std::string("FALLO: ") + what);
}

void testIsoWeekdayKnownDates() {
  // 2026-09-18 (hoy, al escribir esto) es viernes -> 5.
  check(DateUtils::isoWeekday("2026-09-18") == 5, "2026-09-18 deberia ser viernes (5)");
  // 2024-01-01 fue lunes -> 1.
  check(DateUtils::isoWeekday("2024-01-01") == 1, "2024-01-01 deberia ser lunes (1)");
  // 2024-01-07 fue domingo -> 7.
  check(DateUtils::isoWeekday("2024-01-07") == 7, "2024-01-07 deberia ser domingo (7)");
}

void testDaysInMonthLeapAndNonLeap() {
  check(DateUtils::daysInMonth(2024, 2) == 29, "febrero de 2024 (bisiesto) deberia tener 29 dias");
  check(DateUtils::daysInMonth(2023, 2) == 28, "febrero de 2023 (no bisiesto) deberia tener 28 dias");
  check(DateUtils::daysInMonth(2024, 4) == 30, "abril deberia tener 30 dias");
  check(DateUtils::daysInMonth(2024, 12) == 31, "diciembre deberia tener 31 dias");
}

void testAddDaysCrossesMonthAndYearBoundaries() {
  check(DateUtils::addDays("2024-01-31", 1) == "2024-02-01", "sumar 1 dia deberia cruzar de mes");
  check(DateUtils::addDays("2024-12-31", 1) == "2025-01-01", "sumar 1 dia deberia cruzar de anyo");
  check(DateUtils::addDays("2024-03-01", -1) == "2024-02-29", "restar 1 dia deberia volver a febrero bisiesto");
}

void testFormatDateZeroPads() {
  check(DateUtils::formatDate(2024, 1, 5) == "2024-01-05", "formatDate deberia rellenar con ceros");
}

void testParseDateRejectsMalformed() {
  bool threw = false;
  try {
    DateUtils::parseDate("2024-13-01");
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "parseDate deberia rechazar un mes invalido (13)");

  threw = false;
  try {
    DateUtils::parseDate("not-a-date");
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "parseDate deberia rechazar un formato invalido");
}

} // namespace

int main() {
  try {
    testIsoWeekdayKnownDates();
    testDaysInMonthLeapAndNonLeap();
    testAddDaysCrossesMonthAndYearBoundaries();
    testFormatDateZeroPads();
    testParseDateRejectsMalformed();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << "OK: todos los tests de DateUtils pasaron" << std::endl;
  return 0;
}
