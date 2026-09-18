#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace levelup {

// Un evento es SIEMPRE recurrente (dayOfWeek con valor, specificDate vacio)
// XOR puntual (specificDate con valor, dayOfWeek vacio) -- reflejo directo
// del CHECK de la tabla `events` (ver Database::migrate()).
struct Event {
  int64_t id = 0;
  std::string title;
  std::optional<std::string> time;         // "HH:MM", vacio = sin hora fija
  std::optional<int> dayOfWeek;            // 1=lunes..7=domingo (ISO 8601)
  std::optional<std::string> specificDate; // "YYYY-MM-DD"

  bool isRecurring() const { return dayOfWeek.has_value(); }
};

// Un Event resuelto para una fecha concreta, con su estado de "hecho" en
// esa fecha (el mismo Event recurrente puede estar `done` un lunes y no
// el siguiente -- por eso `done` vive aqui y no en Event).
struct ResolvedEvent {
  Event event;
  std::string occurrenceDate; // "YYYY-MM-DD"
  bool done = false;
};

// Resumen agregado de una fecha, para la vista de mes (evita tener que
// resolver cada evento individualmente solo para pintar un punto).
struct DaySummary {
  std::string date; // "YYYY-MM-DD"
  int totalEvents = 0;
  int doneEvents = 0;

  bool hasEvents() const { return totalEvents > 0; }
  bool allDone() const { return totalEvents > 0 && doneEvents == totalEvents; }
};

} // namespace levelup
