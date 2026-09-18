#pragma once

#include <string>

namespace levelup::DateUtils {

// Matematica de calendario pura -- deliberadamente sin nada de zona
// horaria ni "que dia es hoy" (el soporte de zonas horarias de
// std::chrono no es fiable en todas las versiones del NDK de Android).
// "Hoy" lo resuelve la capa Qt (QDate::currentDate(), correcto en
// Android por construccion) y se lo pasa a esta capa como string ISO.

struct DateParts {
  int year;
  int month; // 1..12
  int day;   // 1..31
};

// Lanza std::invalid_argument si `date` no tiene formato "YYYY-MM-DD" o
// no es una fecha valida del calendario.
DateParts parseDate(const std::string& date);

std::string formatDate(int year, int month, int day);

// 1=lunes..7=domingo (ISO 8601).
int isoWeekday(const std::string& date);

// month: 1..12.
int daysInMonth(int year, int month);

// `days` puede ser negativo.
std::string addDays(const std::string& date, int days);

} // namespace levelup::DateUtils
