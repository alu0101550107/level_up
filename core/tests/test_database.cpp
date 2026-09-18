#include <iostream>
#include <stdexcept>
#include <string>

#include "levelup/Database.hpp"

using namespace levelup;

namespace {

void check(bool cond, const char* what) {
  if (!cond) throw std::runtime_error(std::string("FALLO: ") + what);
}

void testMigrateIsIdempotent() {
  Database db(":memory:");
  db.migrate();
  db.migrate(); // no deberia lanzar ni duplicar nada

  auto stmt = db.prepare(
      "SELECT name FROM sqlite_master WHERE type='table' AND name IN ('events', 'completions')");
  int tableCount = 0;
  while (stmt.step()) {
    ++tableCount;
  }
  check(tableCount == 2, "migrate() deberia crear exactamente las tablas events y completions");
}

void testEventsCheckConstraintRejectsBothNull() {
  Database db(":memory:");
  db.migrate();

  bool threw = false;
  try {
    auto stmt = db.prepare("INSERT INTO events (title, day_of_week, specific_date) VALUES ('x', NULL, NULL)");
    stmt.step();
  } catch (const std::runtime_error&) {
    threw = true;
  }
  check(threw, "el CHECK deberia rechazar un evento sin day_of_week ni specific_date");
}

void testEventsCheckConstraintRejectsBothSet() {
  Database db(":memory:");
  db.migrate();

  bool threw = false;
  try {
    auto stmt = db.prepare(
        "INSERT INTO events (title, day_of_week, specific_date) VALUES ('x', 1, '2024-01-01')");
    stmt.step();
  } catch (const std::runtime_error&) {
    threw = true;
  }
  check(threw, "el CHECK deberia rechazar un evento con day_of_week y specific_date a la vez");
}

void testCompletionCascadesOnEventDelete() {
  Database db(":memory:");
  db.migrate();

  auto insertEvent = db.prepare("INSERT INTO events (title, day_of_week) VALUES ('gym', 1)");
  insertEvent.step();
  int64_t eventId = db.lastInsertRowId();

  auto insertCompletion =
      db.prepare("INSERT INTO completions (event_id, occurrence_date) VALUES (?1, '2024-01-01')");
  insertCompletion.bindInt64(1, eventId);
  insertCompletion.step();

  auto deleteEvent = db.prepare("DELETE FROM events WHERE id = ?1");
  deleteEvent.bindInt64(1, eventId);
  deleteEvent.step();

  auto countCompletions = db.prepare("SELECT COUNT(*) FROM completions");
  countCompletions.step();
  check(countCompletions.columnInt64(0) == 0,
        "borrar un evento deberia arrastrar (cascade) sus completions");
}

} // namespace

int main() {
  try {
    testMigrateIsIdempotent();
    testEventsCheckConstraintRejectsBothNull();
    testEventsCheckConstraintRejectsBothSet();
    testCompletionCascadesOnEventDelete();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << "OK: todos los tests de Database pasaron" << std::endl;
  return 0;
}
