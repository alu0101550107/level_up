#include "DayEventListModel.hpp"

int DayEventListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(events_.size());
}

QVariant DayEventListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(events_.size())) {
    return {};
  }
  const auto& resolved = events_[static_cast<size_t>(index.row())];
  switch (role) {
    case IdRole:
      return QVariant::fromValue(static_cast<qlonglong>(resolved.event.id));
    case TitleRole:
      return QString::fromStdString(resolved.event.title);
    case TimeRole:
      return resolved.event.time ? QString::fromStdString(*resolved.event.time) : QString();
    case DoneRole:
      return resolved.done;
    case IsRecurringRole:
      return resolved.event.isRecurring();
    default:
      return {};
  }
}

QHash<int, QByteArray> DayEventListModel::roleNames() const {
  return {
      {IdRole, "eventId"},
      {TitleRole, "title"},
      {TimeRole, "time"},
      {DoneRole, "done"},
      {IsRecurringRole, "isRecurring"},
  };
}

void DayEventListModel::setEvents(std::vector<levelup::ResolvedEvent> events) {
  beginResetModel();
  events_ = std::move(events);
  endResetModel();
}
