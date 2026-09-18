#include "levelup/Database.hpp"

#include <sqlite3.h>

#include <stdexcept>
#include <utility>

namespace levelup {

namespace {

void throwIfError(sqlite3* db, int rc, const std::string& context) {
  if (rc != SQLITE_OK && rc != SQLITE_ROW && rc != SQLITE_DONE) {
    throw std::runtime_error(context + ": " + sqlite3_errmsg(db));
  }
}

} // namespace

// --- Statement ---------------------------------------------------------

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db) {
  int rc = sqlite3_prepare_v2(db_, sql.c_str(), static_cast<int>(sql.size()), &stmt_, nullptr);
  throwIfError(db_, rc, "sqlite3_prepare_v2 fallo para: " + sql);
}

Statement::~Statement() {
  if (stmt_ != nullptr) {
    sqlite3_finalize(stmt_);
  }
}

Statement::Statement(Statement&& other) noexcept
    : db_(other.db_), stmt_(std::exchange(other.stmt_, nullptr)) {}

Statement& Statement::operator=(Statement&& other) noexcept {
  if (this != &other) {
    if (stmt_ != nullptr) {
      sqlite3_finalize(stmt_);
    }
    db_ = other.db_;
    stmt_ = std::exchange(other.stmt_, nullptr);
  }
  return *this;
}

void Statement::bindInt64(int index, int64_t value) {
  throwIfError(db_, sqlite3_bind_int64(stmt_, index, value), "bind_int64");
}

void Statement::bindText(int index, const std::string& value) {
  throwIfError(db_, sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()),
                                       SQLITE_TRANSIENT),
               "bind_text");
}

void Statement::bindNull(int index) {
  throwIfError(db_, sqlite3_bind_null(stmt_, index), "bind_null");
}

void Statement::bindOptText(int index, const std::optional<std::string>& value) {
  if (value.has_value()) {
    bindText(index, *value);
  } else {
    bindNull(index);
  }
}

void Statement::bindOptInt(int index, const std::optional<int>& value) {
  if (value.has_value()) {
    bindInt64(index, *value);
  } else {
    bindNull(index);
  }
}

bool Statement::step() {
  int rc = sqlite3_step(stmt_);
  if (rc == SQLITE_ROW) {
    return true;
  }
  throwIfError(db_, rc, "sqlite3_step");
  return false; // SQLITE_DONE
}

void Statement::reset() {
  sqlite3_reset(stmt_);
  sqlite3_clear_bindings(stmt_);
}

int64_t Statement::columnInt64(int index) const { return sqlite3_column_int64(stmt_, index); }

std::optional<int> Statement::columnOptInt(int index) const {
  if (sqlite3_column_type(stmt_, index) == SQLITE_NULL) {
    return std::nullopt;
  }
  return static_cast<int>(sqlite3_column_int64(stmt_, index));
}

std::string Statement::columnText(int index) const {
  const auto* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt_, index));
  return text != nullptr ? std::string(text) : std::string();
}

std::optional<std::string> Statement::columnOptText(int index) const {
  if (sqlite3_column_type(stmt_, index) == SQLITE_NULL) {
    return std::nullopt;
  }
  return columnText(index);
}

bool Statement::columnBool(int index) const { return sqlite3_column_int64(stmt_, index) != 0; }

// --- Database ------------------------------------------------------------

Database::Database(const std::string& path) {
  int rc = sqlite3_open(path.c_str(), &db_);
  if (rc != SQLITE_OK) {
    std::string message = db_ != nullptr ? sqlite3_errmsg(db_) : "sqlite3_open fallo";
    if (db_ != nullptr) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
    throw std::runtime_error("No se pudo abrir la base de datos '" + path + "': " + message);
  }
  exec("PRAGMA foreign_keys = ON;");
}

Database::~Database() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
  }
}

void Database::exec(const std::string& sql) {
  char* errorMessage = nullptr;
  int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMessage);
  if (rc != SQLITE_OK) {
    std::string message = errorMessage != nullptr ? errorMessage : "error desconocido";
    sqlite3_free(errorMessage);
    throw std::runtime_error("sqlite3_exec fallo: " + message);
  }
}

Statement Database::prepare(const std::string& sql) { return Statement(db_, sql); }

int64_t Database::lastInsertRowId() const { return sqlite3_last_insert_rowid(db_); }

int Database::changes() const { return sqlite3_changes(db_); }

void Database::migrate() {
  exec(R"sql(
    CREATE TABLE IF NOT EXISTS events (
      id            INTEGER PRIMARY KEY AUTOINCREMENT,
      title         TEXT NOT NULL,
      time          TEXT,
      day_of_week   INTEGER,
      specific_date TEXT,
      created_at    TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ','now')),
      CHECK (
        (day_of_week IS NOT NULL AND specific_date IS NULL AND day_of_week BETWEEN 1 AND 7)
        OR (day_of_week IS NULL AND specific_date IS NOT NULL)
      )
    );
  )sql");

  exec(R"sql(
    CREATE INDEX IF NOT EXISTS idx_events_day_of_week
      ON events(day_of_week) WHERE day_of_week IS NOT NULL;
  )sql");

  exec(R"sql(
    CREATE INDEX IF NOT EXISTS idx_events_specific_date
      ON events(specific_date) WHERE specific_date IS NOT NULL;
  )sql");

  // Existencia de fila = hecho (sin columna booleana): desmarcar es un
  // DELETE, no un UPDATE a 0 -- un estado menos que mantener consistente,
  // y los agregados de la vista de mes son COUNT(*) simples.
  exec(R"sql(
    CREATE TABLE IF NOT EXISTS completions (
      event_id        INTEGER NOT NULL REFERENCES events(id) ON DELETE CASCADE,
      occurrence_date TEXT NOT NULL,
      completed_at    TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ','now')),
      PRIMARY KEY (event_id, occurrence_date)
    ) WITHOUT ROWID;
  )sql");
}

} // namespace levelup
