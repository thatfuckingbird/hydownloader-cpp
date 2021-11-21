/*
hydownloader-cpp
Copyright (C) 2021  thatfuckingbird

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "hydownloaderjsonobjectlistmodel.h"
#include <optional>

class HyDownloaderConnection;

class HyDownloaderMissedSubscriptionChecksModel : public HyDownloaderJSONObjectListModel
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool showArchived READ showArchived NOTIFY showArchivedChanged WRITE setShowArchived)

public:
    HyDownloaderMissedSubscriptionChecksModel();
    void setUpConnections(HyDownloaderConnection* oldConnection) override;
    std::uint64_t addOrUpdateObjects(const QJsonArray& objs) override;
    void refresh(bool full = true) override;
    void clear() override;
    Q_INVOKABLE QString statusText() const;
    Q_INVOKABLE bool showArchived() const;

public slots:
    void loadDataForSubscriptions(const QVector<int>& subscriptionIDs = {});
    void setShowArchived(bool show);

signals:
    void statusTextChanged(const QString&);
    void showArchivedChanged(bool);

private slots:
    void handleMissedSubscriptionChecksData(std::uint64_t requestID, const QJsonArray& data);

private:
    std::optional<QVector<int>> m_lastRequestedIDs;
    QString m_statusText;
    bool m_showArchived = false;
};
