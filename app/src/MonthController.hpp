#pragma once

#include <QObject>

#include "MonthGridModel.hpp"

namespace levelup {
class EventRepository;
}
class EventBus;

class MonthController : public QObject {
  Q_OBJECT
  Q_PROPERTY(int year READ year NOTIFY monthChanged)
  Q_PROPERTY(int month READ month NOTIFY monthChanged)
  Q_PROPERTY(QString monthLabel READ monthLabel NOTIFY monthChanged)
  Q_PROPERTY(QObject* grid READ gridModel CONSTANT)

 public:
  MonthController(levelup::EventRepository& repo, EventBus& bus, QObject* parent = nullptr);

  int year() const { return year_; }
  int month() const { return month_; }
  QString monthLabel() const;
  QObject* gridModel() const { return model_; }

  Q_INVOKABLE void nextMonth();
  Q_INVOKABLE void prevMonth();
  Q_INVOKABLE void goToToday();
  // Llamado desde QML cuando se toca una celda del grid -- reemite la
  // fecha para que Main.qml pueda cambiar a la pestaña de Dia.
  Q_INVOKABLE void activateDate(const QString& isoDate) { emit dateActivated(isoDate); }

 signals:
  void monthChanged();
  void dateActivated(const QString& isoDate);

 private:
  void reload();

  levelup::EventRepository& repo_;
  EventBus& bus_;
  int year_;
  int month_;
  MonthGridModel* model_;
};
