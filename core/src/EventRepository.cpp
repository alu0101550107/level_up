#include "levelup/EventRepository.hpp"

#include <map>
#include <utility>

#include "levelup/Database.hpp"
#include "levelup/DateUtils.hpp"

namespace levelup {

EventRepository::EventRepository(Database& db) : db_(db) {}

Event EventRepository::eventFromRow(Statement& stmt) {
  Event event;
  event.id = stmt.columnInt64(0);
  event.title = stmt.columnText(1);
  event.time = stmt.columnOptText(2);
  event.endTime = stmt.columnOptText(3);
  event.dayOfWeek = stmt.columnOptInt(4);
  event.specificDate = stmt.columnOptText(5);
  return event;
}

int64_t EventRepository::createWeeklyEvent(int dayOfWeek, const std::string& title,
                                            std::optional<std::string> time,
                                            std::optional<std::string> endTime) {
  auto stmt = db_.prepare(
      "INSERT INTO events (title, time, end_time, day_of_week, specific_date) "
      "VALUES (?1, ?2, ?3, ?4, NULL)");
  stmt.bindText(1, title);
  stmt.bindOptText(2, time);
  stmt.bindOptText(3, endTime);
  stmt.bindInt64(4, dayOfWeek);
  stmt.step();
  return db_.lastInsertRowId();
}

int64_t EventRepository::createOneOffEvent(const std::string& date, const std::string& title,
                                            std::optional<std::string> time,
                                            std::optional<std::string> endTime) {
  auto stmt = db_.prepare(
      "INSERT INTO events (title, time, end_time, day_of_week, specific_date) "
      "VALUES (?1, ?2, ?3, NULL, ?4)");
  stmt.bindText(1, title);
  stmt.bindOptText(2, time);
  stmt.bindOptText(3, endTime);
  stmt.bindText(4, date);
  stmt.step();
  return db_.lastInsertRowId();
}

bool EventRepository::updateEvent(int64_t id, const std::string& title,
                                   std::optional<std::string> time,
                                   std::optional<std::string> endTime) {
  auto stmt = db_.prepare("UPDATE events SET title = ?1, time = ?2, end_time = ?3 WHERE id = ?4");
  stmt.bindText(1, title);
  stmt.bindOptText(2, time);
  stmt.bindOptText(3, endTime);
  stmt.bindInt64(4, id);
  stmt.step();
  return db_.changes() > 0;
}

bool EventRepository::deleteEvent(int64_t id) {
  auto stmt = db_.prepare("DELETE FROM events WHERE id = ?1");
  stmt.bindInt64(1, id);
  stmt.step();
  return db_.changes() > 0;
}

std::optional<Event> EventRepository::findById(int64_t id) {
  auto stmt = db_.prepare(
      "SELECT id, title, time, end_time, day_of_week, specific_date FROM events WHERE id = ?1");
  stmt.bindInt64(1, id);
  if (!stmt.step()) {
    return std::nullopt;
  }
  return eventFromRow(stmt);
}

std::vector<Event> EventRepository::listWeeklyEvents(int dayOfWeek) {
  auto stmt = db_.prepare(
      "SELECT id, title, time, end_time, day_of_week, specific_date FROM events "
      "WHERE day_of_week = ?1 ORDER BY time IS NULL, time, id");
  stmt.bindInt64(1, dayOfWeek);
  std::vector<Event> result;
  while (stmt.step()) {
    result.push_back(eventFromRow(stmt));
  }
  return result;
}

std::vector<Event> EventRepository::listAllWeeklyEvents() {
  auto stmt = db_.prepare(
      "SELECT id, title, time, end_time, day_of_week, specific_date FROM events "
      "WHERE day_of_week IS NOT NULL ORDER BY day_of_week, time IS NULL, time, id");
  std::vector<Event> result;
  while (stmt.step()) {
    result.push_back(eventFromRow(stmt));
  }
  return result;
}

std::vector<Event> EventRepository::listOneOffEvents(const std::string& fromDate,
                                                      const std::string& toDate) {
  auto stmt = db_.prepare(
      "SELECT id, title, time, end_time, day_of_week, specific_date FROM events "
      "WHERE specific_date BETWEEN ?1 AND ?2 ORDER BY specific_date, time IS NULL, time, id");
  stmt.bindText(1, fromDate);
  stmt.bindText(2, toDate);
  std::vector<Event> result;
  while (stmt.step()) {
    result.push_back(eventFromRow(stmt));
  }
  return result;
}

std::vector<ResolvedEvent> EventRepository::eventsForDate(const std::string& date) {
  int weekday = DateUtils::isoWeekday(date);
  auto stmt = db_.prepare(
      "SELECT e.id, e.title, e.time, e.end_time, e.day_of_week, e.specific_date, "
      "(c.event_id IS NOT NULL) AS done "
      "FROM events e "
      "LEFT JOIN completions c ON c.event_id = e.id AND c.occurrence_date = ?1 "
      "WHERE e.day_of_week = ?2 OR e.specific_date = ?1 "
      "ORDER BY e.time IS NULL, e.time, e.id");
  stmt.bindText(1, date);
  stmt.bindInt64(2, weekday);

  std::vector<ResolvedEvent> result;
  while (stmt.step()) {
    ResolvedEvent resolved;
    resolved.event = eventFromRow(stmt);
    resolved.occurrenceDate = date;
    resolved.done = stmt.columnBool(6);
    result.push_back(std::move(resolved));
  }
  return result;
}

std::vector<DaySummary> EventRepository::monthSummary(int year, int month) {
  int numDays = DateUtils::daysInMonth(year, month);
  std::string firstDate = DateUtils::formatDate(year, month, 1);
  std::string lastDate = DateUtils::formatDate(year, month, numDays);

  // Tres consultas agrupadas en vez de una por dia (hasta 42 round-trips
  // para pintar la vista de mes) -- se combinan por fecha en C++.
  std::map<int, int> recurringCountByWeekday;
  {
    auto stmt = db_.prepare(
        "SELECT day_of_week, COUNT(*) FROM events "
        "WHERE day_of_week IS NOT NULL GROUP BY day_of_week");
    while (stmt.step()) {
      recurringCountByWeekday[static_cast<int>(stmt.columnInt64(0))] =
          static_cast<int>(stmt.columnInt64(1));
    }
  }

  std::map<std::string, int> oneOffCountByDate;
  {
    auto stmt = db_.prepare(
        "SELECT specific_date, COUNT(*) FROM events "
        "WHERE specific_date BETWEEN ?1 AND ?2 GROUP BY specific_date");
    stmt.bindText(1, firstDate);
    stmt.bindText(2, lastDate);
    while (stmt.step()) {
      oneOffCountByDate[stmt.columnText(0)] = static_cast<int>(stmt.columnInt64(1));
    }
  }

  std::map<std::string, int> doneCountByDate;
  {
    auto stmt = db_.prepare(
        "SELECT occurrence_date, COUNT(*) FROM completions "
        "WHERE occurrence_date BETWEEN ?1 AND ?2 GROUP BY occurrence_date");
    stmt.bindText(1, firstDate);
    stmt.bindText(2, lastDate);
    while (stmt.step()) {
      doneCountByDate[stmt.columnText(0)] = static_cast<int>(stmt.columnInt64(1));
    }
  }

  std::vector<DaySummary> summaries;
  summaries.reserve(numDays);
  for (int day = 1; day <= numDays; ++day) {
    DaySummary summary;
    summary.date = DateUtils::formatDate(year, month, day);
    int weekday = DateUtils::isoWeekday(summary.date);

    auto recurringIt = recurringCountByWeekday.find(weekday);
    auto oneOffIt = oneOffCountByDate.find(summary.date);
    summary.totalEvents = (recurringIt != recurringCountByWeekday.end() ? recurringIt->second : 0) +
                           (oneOffIt != oneOffCountByDate.end() ? oneOffIt->second : 0);

    auto doneIt = doneCountByDate.find(summary.date);
    summary.doneEvents = doneIt != doneCountByDate.end() ? doneIt->second : 0;

    summaries.push_back(std::move(summary));
  }
  return summaries;
}

void EventRepository::setCompletion(int64_t eventId, const std::string& occurrenceDate, bool done) {
  if (done) {
    auto stmt = db_.prepare("INSERT OR IGNORE INTO completions (event_id, occurrence_date) VALUES (?1, ?2)");
    stmt.bindInt64(1, eventId);
    stmt.bindText(2, occurrenceDate);
    stmt.step();
  } else {
    auto stmt = db_.prepare("DELETE FROM completions WHERE event_id = ?1 AND occurrence_date = ?2");
    stmt.bindInt64(1, eventId);
    stmt.bindText(2, occurrenceDate);
    stmt.step();
  }
}

bool EventRepository::isDone(int64_t eventId, const std::string& occurrenceDate) {
  auto stmt = db_.prepare("SELECT 1 FROM completions WHERE event_id = ?1 AND occurrence_date = ?2");
  stmt.bindInt64(1, eventId);
  stmt.bindText(2, occurrenceDate);
  return stmt.step();
}

} // namespace levelup
