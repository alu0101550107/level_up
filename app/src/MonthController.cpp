#include "MonthController.hpp"

#include <QDate>
#include <QLocale>

#include "EventBus.hpp"
#include "levelup/EventRepository.hpp"

MonthController::MonthController(levelup::EventRepository& repo, EventBus& bus, QObject* parent)
    : QObject(parent), repo_(repo), bus_(bus), model_(new MonthGridModel(this)) {
  QDate today = QDate::currentDate();
  year_ = today.year();
  month_ = today.month();
  connect(&bus_, &EventBus::changed, this, &MonthController::reload);
  reload();
}

QString MonthController::monthLabel() const {
  return QLocale().toString(QDate(year_, month_, 1), QStringLiteral("MMMM yyyy"));
}

void MonthController::nextMonth() {
  QDate next = QDate(year_, month_, 1).addMonths(1);
  year_ = next.year();
  month_ = next.month();
  emit monthChanged();
  reload();
}

void MonthController::prevMonth() {
  QDate prev = QDate(year_, month_, 1).addMonths(-1);
  year_ = prev.year();
  month_ = prev.month();
  emit monthChanged();
  reload();
}

void MonthController::goToToday() {
  QDate today = QDate::currentDate();
  year_ = today.year();
  month_ = today.month();
  emit monthChanged();
  reload();
}

void MonthController::reload() { model_->setMonth(year_, month_, repo_.monthSummary(year_, month_)); }
