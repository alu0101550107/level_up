#include "AppBackend.hpp"

AppBackend::AppBackend(const std::string& dbPath, QObject* parent)
    : QObject(parent), db_(dbPath), repo_(db_) {
  db_.migrate();

  day_ = new DayController(repo_, bus_, this);
  month_ = new MonthController(repo_, bus_, this);
  editor_ = new EditorController(repo_, bus_, this);

  // Tocar una celda del mes debe llevar a Dia a esa fecha -- Main.qml se
  // conecta a la misma senyal para ademas cambiar de pestaña (eso es
  // puramente de UI, no pertenece aqui).
  connect(month_, &MonthController::dateActivated, day_, &DayController::goToDate);
}
