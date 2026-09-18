#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "levelup/Event.hpp"

namespace levelup {

class Database;
class Statement;

// DAO sobre la tabla `events`/`completions`. Toda la aritmetica de fechas
// (que dia de la semana es una fecha, cuantos dias tiene un mes) se delega
// en DateUtils -- este tipo solo sabe hablar SQL.
class EventRepository {
 public:
  explicit EventRepository(Database& db);

  int64_t createWeeklyEvent(int dayOfWeek, const std::string& title,
                             std::optional<std::string> time);
  int64_t createOneOffEvent(const std::string& date, const std::string& title,
                             std::optional<std::string> time);

  // Solo edita titulo/hora -- cambiar un evento de recurrente a puntual (o
  // viceversa) se modela como borrar+recrear, no como un update ambiguo.
  bool updateEvent(int64_t id, const std::string& title, std::optional<std::string> time);

  // Borra el evento y, por el ON DELETE CASCADE de `completions`, todo su
  // historial de "hecho".
  bool deleteEvent(int64_t id);

  std::optional<Event> findById(int64_t id);
  std::vector<Event> listWeeklyEvents(int dayOfWeek);
  std::vector<Event> listAllWeeklyEvents();
  std::vector<Event> listOneOffEvents(const std::string& fromDate, const std::string& toDate);

  // Eventos recurrentes de ese dia de la semana + puntuales de esa fecha
  // exacta, cada uno con su estado `done` para esa fecha concreta.
  std::vector<ResolvedEvent> eventsForDate(const std::string& date);

  // Un DaySummary por cada dia del mes (month: 1..12).
  std::vector<DaySummary> monthSummary(int year, int month);

  void setCompletion(int64_t eventId, const std::string& occurrenceDate, bool done);
  bool isDone(int64_t eventId, const std::string& occurrenceDate);

 private:
  Database& db_;

  Event eventFromRow(Statement& stmt);
};

} // namespace levelup
