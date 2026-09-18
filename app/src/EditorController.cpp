#include "EditorController.hpp"

#include <optional>
#include <string>

#include "EventBus.hpp"
#include "levelup/EventRepository.hpp"

namespace {
std::optional<std::string> toOptionalString(const QString& value) {
  return value.isEmpty() ? std::nullopt : std::make_optional(value.toStdString());
}
} // namespace

EditorController::EditorController(levelup::EventRepository& repo, EventBus& bus, QObject* parent)
    : QObject(parent), repo_(repo), bus_(bus) {}

void EditorController::addWeeklyEvent(int dayOfWeek, const QString& title, const QString& time,
                                       const QString& endTime) {
  repo_.createWeeklyEvent(dayOfWeek, title.toStdString(), toOptionalString(time),
                           toOptionalString(endTime));
  bus_.notify();
}

void EditorController::addOneOffEvent(const QString& date, const QString& title, const QString& time,
                                       const QString& endTime) {
  repo_.createOneOffEvent(date.toStdString(), title.toStdString(), toOptionalString(time),
                           toOptionalString(endTime));
  bus_.notify();
}

void EditorController::updateEvent(qlonglong id, const QString& title, const QString& time,
                                    const QString& endTime) {
  repo_.updateEvent(id, title.toStdString(), toOptionalString(time), toOptionalString(endTime));
  bus_.notify();
}

void EditorController::deleteEvent(qlonglong id) {
  repo_.deleteEvent(id);
  bus_.notify();
}

QVariantMap EditorController::getEvent(qlonglong id) const {
  auto event = repo_.findById(id);
  if (!event.has_value()) {
    return {};
  }
  QVariantMap map;
  map["title"] = QString::fromStdString(event->title);
  map["time"] = event->time ? QString::fromStdString(*event->time) : QString();
  map["endTime"] = event->endTime ? QString::fromStdString(*event->endTime) : QString();
  return map;
}
