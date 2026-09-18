#include "DayController.hpp"

#include "EventBus.hpp"
#include "levelup/EventRepository.hpp"

DayController::DayController(levelup::EventRepository& repo, EventBus& bus, QObject* parent)
    : QObject(parent),
      repo_(repo),
      bus_(bus),
      currentDate_(QDate::currentDate()),
      model_(new DayEventListModel(this)) {
  connect(&bus_, &EventBus::changed, this, &DayController::reload);
  reload();
}

QString DayController::currentDate() const { return currentDate_.toString(Qt::ISODate); }

QString DayController::currentDateLabel() const { return currentDate_.toString("dddd d MMMM"); }

void DayController::toggleCompletion(qlonglong eventId) {
  std::string date = currentDate().toStdString();
  bool done = repo_.isDone(eventId, date);
  repo_.setCompletion(eventId, date, !done);
  bus_.notify();
}

void DayController::nextDay() {
  currentDate_ = currentDate_.addDays(1);
  emit currentDateChanged();
  reload();
}

void DayController::prevDay() {
  currentDate_ = currentDate_.addDays(-1);
  emit currentDateChanged();
  reload();
}

void DayController::goToToday() {
  currentDate_ = QDate::currentDate();
  emit currentDateChanged();
  reload();
}

void DayController::goToDate(const QString& isoDate) {
  QDate date = QDate::fromString(isoDate, Qt::ISODate);
  if (!date.isValid()) {
    return;
  }
  currentDate_ = date;
  emit currentDateChanged();
  reload();
}

void DayController::reload() { model_->setEvents(repo_.eventsForDate(currentDate().toStdString())); }
