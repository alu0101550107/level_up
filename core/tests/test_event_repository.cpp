#include <iostream>
#include <stdexcept>
#include <string>

#include "levelup/Database.hpp"
#include "levelup/EventRepository.hpp"

using namespace levelup;

namespace {

void check(bool cond, const char* what) {
  if (!cond) throw std::runtime_error(std::string("FALLO: ") + what);
}

void testCreateAndListWeeklyEvents() {
  Database db(":memory:");
  db.migrate();
  EventRepository repo(db);

  repo.createWeeklyEvent(1, "gimnasio", std::string("07:00"));
  repo.createWeeklyEvent(1, "estudiar", std::nullopt);
  repo.createWeeklyEvent(3, "compra", std::nullopt);

  auto monday = repo.listWeeklyEvents(1);
  check(monday.size() == 2, "deberia haber 2 eventos recurrentes en lunes");
  check(monday[0].isRecurring(), "un evento semanal deberia ser recurrente");
  check(!monday[0].specificDate.has_value(), "un evento semanal no deberia tener specific_date");

  auto wednesday = repo.listWeeklyEvents(3);
  check(wednesday.size() == 1, "deberia haber 1 evento recurrente en miercoles");
}

void testCreateOneOffEvent() {
  Database db(":memory:");
  db.migrate();
  EventRepository repo(db);

  repo.createOneOffEvent("2024-06-15", "cumpleanos", std::nullopt);

  auto events = repo.listOneOffEvents("2024-06-01", "2024-06-30");
  check(events.size() == 1, "deberia haber 1 evento puntual en el rango de junio");
  check(!events[0].isRecurring(), "un evento puntual no deberia ser recurrente");
  check(events[0].specificDate == "2024-06-15", "el evento puntual deberia guardar su fecha exacta");
}

void testEventsForDateMergesRecurringAndOneOff() {
  Database db(":memory:");
  db.migrate();
  EventRepository repo(db);

  // 2024-01-01 es lunes (weekday ISO 1).
  repo.createWeeklyEvent(1, "gimnasio", std::nullopt);
  repo.createOneOffEvent("2024-01-01", "cita medica", std::nullopt);
  repo.createWeeklyEvent(2, "otro dia", std::nullopt); // no deberia aparecer

  auto resolved = repo.eventsForDate("2024-01-01");
  check(resolved.size() == 2, "eventsForDate deberia mezclar recurrente + puntual de esa fecha");
}

void testCompletionIsPerConcreteDateNotPerTemplate() {
  Database db(":memory:");
  db.migrate();
  EventRepository repo(db);

  // El requisito central del usuario: marcar "gimnasio" hecho el lunes 1
  // no deberia afectar al lunes 8 (misma plantilla semanal, dia_of_week=1).
  int64_t eventId = repo.createWeeklyEvent(1, "gimnasio", std::nullopt);

  repo.setCompletion(eventId, "2024-01-01", true);

  check(repo.isDone(eventId, "2024-01-01"), "deberia estar marcado hecho el 2024-01-01");
  check(!repo.isDone(eventId, "2024-01-08"), "NO deberia estar marcado hecho el 2024-01-08 (otro lunes)");

  auto resolvedFirstMonday = repo.eventsForDate("2024-01-01");
  check(resolvedFirstMonday.size() == 1 && resolvedFirstMonday[0].done,
        "eventsForDate del 2024-01-01 deberia reflejar done=true");

  auto resolvedSecondMonday = repo.eventsForDate("2024-01-08");
  check(resolvedSecondMonday.size() == 1 && !resolvedSecondMonday[0].done,
        "eventsForDate del 2024-01-08 deberia reflejar done=false");

  // Desmarcar tambien es por fecha concreta.
  repo.setCompletion(eventId, "2024-01-01", false);
  check(!repo.isDone(eventId, "2024-01-01"), "desmarcar deberia borrar la completion");
}

void testDeleteEventCascadesCompletions() {
  Database db(":memory:");
  db.migrate();
  EventRepository repo(db);

  int64_t eventId = repo.createOneOffEvent("2024-01-01", "unico", std::nullopt);
  repo.setCompletion(eventId, "2024-01-01", true);
  check(repo.isDone(eventId, "2024-01-01"), "deberia estar marcado hecho antes de borrar");

  check(repo.deleteEvent(eventId), "deleteEvent deberia devolver true si el evento existia");
  check(!repo.findById(eventId).has_value(), "el evento no deberia existir tras borrarlo");

  auto resolved = repo.eventsForDate("2024-01-01");
  check(resolved.empty(), "no deberian quedar completions huerfanas tras el cascade");
}

void testMonthSummaryCombinesRecurringOneOffAndCompletions() {
  Database db(":memory:");
  db.migrate();
  EventRepository repo(db);

  // Enero 2024: dias 1 y 8 son lunes.
  int64_t recurringId = repo.createWeeklyEvent(1, "gimnasio", std::nullopt);
  repo.createOneOffEvent("2024-01-01", "cita", std::nullopt);
  repo.setCompletion(recurringId, "2024-01-01", true);

  auto summaries = repo.monthSummary(2024, 1);
  check(summaries.size() == 31, "enero deberia tener 31 DaySummary");

  const DaySummary* jan1 = nullptr;
  const DaySummary* jan8 = nullptr;
  for (const auto& summary : summaries) {
    if (summary.date == "2024-01-01") jan1 = &summary;
    if (summary.date == "2024-01-08") jan8 = &summary;
  }
  check(jan1 != nullptr && jan1->totalEvents == 2, "2024-01-01 deberia tener 2 eventos (recurrente + puntual)");
  check(jan1 != nullptr && jan1->doneEvents == 1, "2024-01-01 deberia tener 1 completado");
  check(jan1 != nullptr && !jan1->allDone(), "2024-01-01 no deberia estar 'todo hecho' (1 de 2)");

  check(jan8 != nullptr && jan8->totalEvents == 1, "2024-01-08 deberia tener solo el recurrente");
  check(jan8 != nullptr && jan8->doneEvents == 0, "2024-01-08 no deberia heredar el 'hecho' del 2024-01-01");
}

} // namespace

int main() {
  try {
    testCreateAndListWeeklyEvents();
    testCreateOneOffEvent();
    testEventsForDateMergesRecurringAndOneOff();
    testCompletionIsPerConcreteDateNotPerTemplate();
    testDeleteEventCascadesCompletions();
    testMonthSummaryCombinesRecurringOneOffAndCompletions();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << "OK: todos los tests de EventRepository pasaron" << std::endl;
  return 0;
}
