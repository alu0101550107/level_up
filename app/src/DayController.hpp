#pragma once

#include <QDate>
#include <QObject>

#include "DayEventListModel.hpp"

namespace levelup {
class EventRepository;
}
class EventBus;

class DayController : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentDate READ currentDate NOTIFY currentDateChanged)
  Q_PROPERTY(QString currentDateLabel READ currentDateLabel NOTIFY currentDateChanged)
  Q_PROPERTY(QObject* events READ eventsModel CONSTANT)

 public:
  DayController(levelup::EventRepository& repo, EventBus& bus, QObject* parent = nullptr);

  QString currentDate() const;
  QString currentDateLabel() const;
  QObject* eventsModel() const { return model_; }

  Q_INVOKABLE void toggleCompletion(qlonglong eventId);
  Q_INVOKABLE void nextDay();
  Q_INVOKABLE void prevDay();
  Q_INVOKABLE void goToToday();
  Q_INVOKABLE void goToDate(const QString& isoDate);

 signals:
  void currentDateChanged();

 private:
  void reload();

  levelup::EventRepository& repo_;
  EventBus& bus_;
  QDate currentDate_;
  DayEventListModel* model_;
};
