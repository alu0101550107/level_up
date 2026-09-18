#include "MonthGridModel.hpp"

#include <unordered_map>

int MonthGridModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(cells_.size());
}

QVariant MonthGridModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(cells_.size())) {
    return {};
  }
  const auto& cell = cells_[static_cast<size_t>(index.row())];
  switch (role) {
    case DateRole:
      return cell.date.toString(Qt::ISODate);
    case DayNumberRole:
      return cell.date.day();
    case IsCurrentMonthRole:
      return cell.isCurrentMonth;
    case HasEventsRole:
      return cell.hasEvents;
    case AllDoneRole:
      return cell.allDone;
    case IsTodayRole:
      return cell.date == QDate::currentDate();
    default:
      return {};
  }
}

QHash<int, QByteArray> MonthGridModel::roleNames() const {
  return {
      {DateRole, "date"},
      {DayNumberRole, "dayNumber"},
      {IsCurrentMonthRole, "isCurrentMonth"},
      {HasEventsRole, "hasEvents"},
      {AllDoneRole, "allDone"},
      {IsTodayRole, "isToday"},
  };
}

void MonthGridModel::setMonth(int year, int month, std::vector<levelup::DaySummary> summaries) {
  beginResetModel();

  std::unordered_map<std::string, levelup::DaySummary> byDate;
  byDate.reserve(summaries.size());
  for (auto& summary : summaries) {
    std::string date = summary.date;
    byDate.emplace(std::move(date), std::move(summary));
  }

  // dayOfWeek() de QDate: 1=lunes..7=domingo -- retrocede hasta el lunes
  // que empieza la primera semana visible del mes.
  QDate firstOfMonth(year, month, 1);
  QDate gridStart = firstOfMonth.addDays(-(firstOfMonth.dayOfWeek() - 1));

  cells_.clear();
  cells_.reserve(42);
  for (int i = 0; i < 42; ++i) {
    QDate date = gridStart.addDays(i);
    Cell cell;
    cell.date = date;
    cell.isCurrentMonth = (date.year() == year && date.month() == month);

    auto it = byDate.find(date.toString(Qt::ISODate).toStdString());
    if (it != byDate.end()) {
      cell.hasEvents = it->second.hasEvents();
      cell.allDone = it->second.allDone();
    }
    cells_.push_back(cell);
  }

  endResetModel();
}
