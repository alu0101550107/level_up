#pragma once

#include <QObject>
#include <string>

#include "levelup/Database.hpp"
#include "levelup/EventRepository.hpp"

#include "DayController.hpp"
#include "EditorController.hpp"
#include "EventBus.hpp"
#include "MonthController.hpp"

// Unico context property expuesto a QML. Posee la base de datos y el
// repositorio, y los tres controllers de pantalla -- todos comparten la
// misma EventRepository y el mismo EventBus, asi que una mutacion en
// cualquiera de ellos se propaga a los demas (ver EventBus.hpp).
class AppBackend : public QObject {
  Q_OBJECT
  Q_PROPERTY(QObject* day READ day CONSTANT)
  Q_PROPERTY(QObject* month READ month CONSTANT)
  Q_PROPERTY(QObject* editor READ editor CONSTANT)

 public:
  explicit AppBackend(const std::string& dbPath, QObject* parent = nullptr);

  QObject* day() const { return day_; }
  QObject* month() const { return month_; }
  QObject* editor() const { return editor_; }

 private:
  levelup::Database db_;
  levelup::EventRepository repo_;
  EventBus bus_;
  DayController* day_;
  MonthController* month_;
  EditorController* editor_;
};
