#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace levelup {

// Wrapper RAII sobre un sqlite3_stmt* -- evita repetir el boilerplate de
// sqlite3_bind_*/sqlite3_column_*/comprobacion de errores en cada consulta
// de EventRepository.
class Statement {
 public:
  Statement(sqlite3* db, const std::string& sql);
  ~Statement();

  Statement(const Statement&) = delete;
  Statement& operator=(const Statement&) = delete;
  Statement(Statement&& other) noexcept;
  Statement& operator=(Statement&& other) noexcept;

  void bindInt64(int index, int64_t value);
  void bindText(int index, const std::string& value);
  void bindNull(int index);
  void bindOptText(int index, const std::optional<std::string>& value);
  void bindOptInt(int index, const std::optional<int>& value);

  // Avanza a la siguiente fila. Devuelve true si hay una fila disponible,
  // false si se agotaron los resultados.
  bool step();
  void reset();

  int64_t columnInt64(int index) const;
  std::optional<int> columnOptInt(int index) const;
  std::string columnText(int index) const;
  std::optional<std::string> columnOptText(int index) const;
  bool columnBool(int index) const;

 private:
  sqlite3* db_ = nullptr;
  sqlite3_stmt* stmt_ = nullptr;
};

class Database {
 public:
  // `path` puede ser ":memory:" (usado por los tests -- sin limpieza de
  // ficheros entre casos).
  explicit Database(const std::string& path);
  ~Database();

  Database(const Database&) = delete;
  Database& operator=(const Database&) = delete;

  // Crea el esquema si no existe. Idempotente -- se puede (y se debe)
  // llamar en cada arranque de la app, no solo en la primera instalacion.
  void migrate();

  void exec(const std::string& sql);
  Statement prepare(const std::string& sql);
  int64_t lastInsertRowId() const;
  // Filas afectadas por el ultimo INSERT/UPDATE/DELETE completado.
  int changes() const;

  sqlite3* handle() const { return db_; }

 private:
  sqlite3* db_ = nullptr;
};

} // namespace levelup
