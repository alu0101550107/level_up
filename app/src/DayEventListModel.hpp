#pragma once

#include <QAbstractListModel>

#include "levelup/Event.hpp"

// Expone el resultado de EventRepository::eventsForDate() a QML.
class DayEventListModel : public QAbstractListModel {
  Q_OBJECT
 public:
  enum Role {
    IdRole = Qt::UserRole + 1,
    TitleRole,
    TimeRole,
    EndTimeRole,
    DoneRole,
    IsRecurringRole,
  };

  using QAbstractListModel::QAbstractListModel;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setEvents(std::vector<levelup::ResolvedEvent> events);

 private:
  std::vector<levelup::ResolvedEvent> events_;
};
