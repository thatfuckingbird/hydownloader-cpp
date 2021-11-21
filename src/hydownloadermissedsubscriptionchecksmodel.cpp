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

#include <hydownloader-cpp/hydownloadermissedsubscriptionchecksmodel.h>
#include <hydownloader-cpp/hydownloaderconnection.h>
#include <QJsonObject>

HyDownloaderMissedSubscriptionChecksModel::HyDownloaderMissedSubscriptionChecksModel() :
    HyDownloaderJSONObjectListModel{
      {{"subscription_id", "Subscription ID", false, toVariant, &QJsonValue::fromVariant},
       {"reason", "Reason", false, toVariant, &QJsonValue::fromVariant},
       {"time", "Time", false, toDateTime, fromDateTime},
       {"data", "Data", true, toVariant, &QJsonValue::fromVariant},
       {"archived", "Archived", true, toBool, fromBool}},
      "rowid"},
    m_statusText{"No data loaded"} {}

void HyDownloaderMissedSubscriptionChecksModel::setUpConnections(HyDownloaderConnection* oldConnection)
{
    if(oldConnection) disconnect(oldConnection, &HyDownloaderConnection::missedSubscriptionChecksDataReceived, this, &HyDownloaderMissedSubscriptionChecksModel::handleMissedSubscriptionChecksData);
    connect(m_connection, &HyDownloaderConnection::missedSubscriptionChecksDataReceived, this, &HyDownloaderMissedSubscriptionChecksModel::handleMissedSubscriptionChecksData);
    emit statusTextChanged(m_statusText);
}

std::uint64_t HyDownloaderMissedSubscriptionChecksModel::addOrUpdateObjects(const QJsonArray& objs)
{
    return m_connection->addOrUpdateMissedSubscriptionChecks(objs);
}

void HyDownloaderMissedSubscriptionChecksModel::refresh(bool full)
{
    if(full) clear();
    if(m_lastRequestedIDs.has_value()) {
        m_connection->requestMissedSubscriptionChecksData(m_lastRequestedIDs.value(), m_showArchived);
        m_statusText = "Loading missed subscription checks...";
        emit statusTextChanged(m_statusText);
    }
}

void HyDownloaderMissedSubscriptionChecksModel::clear()
{
    HyDownloaderJSONObjectListModel::clear();
    m_statusText = "No data loaded";
    emit statusTextChanged(m_statusText);
}

void HyDownloaderMissedSubscriptionChecksModel::loadDataForSubscriptions(const QVector<int>& subscriptionIDs)
{
    m_lastRequestedIDs = subscriptionIDs;
    refresh();
}

QString HyDownloaderMissedSubscriptionChecksModel::statusText() const
{
    return m_statusText;
}

bool HyDownloaderMissedSubscriptionChecksModel::showArchived() const
{
    return m_showArchived;
}

void HyDownloaderMissedSubscriptionChecksModel::setShowArchived(bool show)
{
    if(m_showArchived != show) {
        m_showArchived = show;
        emit showArchivedChanged(show);
    }
}

void HyDownloaderMissedSubscriptionChecksModel::handleMissedSubscriptionChecksData(uint64_t, const QJsonArray& data)
{
    if(const auto& ids = m_lastRequestedIDs.value(); !ids.isEmpty()) {
        const auto size = ids.size();
        if(size == 1) {
            m_statusText = QString{"Missed subscription checks for subscription %1"}.arg(ids[0]);
        } else {
            m_statusText = QString{"Missed subscription checks for %1 subscriptions"}.arg(size);
        }
    } else {
        m_statusText = "Missed subscription checks for all subscriptions";
    }
    emit statusTextChanged(m_statusText);
    updateFromRowData(data);
}
