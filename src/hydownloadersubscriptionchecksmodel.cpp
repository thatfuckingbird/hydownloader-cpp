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

#include <hydownloader-cpp/hydownloadersubscriptionchecksmodel.h>
#include <hydownloader-cpp/hydownloaderconnection.h>
#include <QJsonObject>

HyDownloaderSubscriptionChecksModel::HyDownloaderSubscriptionChecksModel() :
    HyDownloaderJSONObjectListModel{
      {{"subscription_id", "Subscription ID", false, toVariant, &QJsonValue::fromVariant},
       {"time_started", "Time started", false, toDateTime, fromDateTime},
       {"time_finished", "Time finished", false, toDateTime, fromDateTime},
       {"status", "Result status", false, toVariant, &QJsonValue::fromVariant},
       {"new_files", "New files", false, toVariant, &QJsonValue::fromVariant},
       {"already_seen_files", "Already seen files", false, toVariant, &QJsonValue::fromVariant},
       {"archived", "Archived", true, toBool, fromBool}},
      "rowid"},
    m_statusText{"No data loaded"} {}

void HyDownloaderSubscriptionChecksModel::setUpConnections(HyDownloaderConnection* oldConnection)
{
    if(oldConnection) disconnect(oldConnection, &HyDownloaderConnection::subscriptionChecksDataReceived, this, &HyDownloaderSubscriptionChecksModel::handleSubscriptionChecksData);
    connect(m_connection, &HyDownloaderConnection::subscriptionChecksDataReceived, this, &HyDownloaderSubscriptionChecksModel::handleSubscriptionChecksData);
    emit statusTextChanged(m_statusText);
}

std::uint64_t HyDownloaderSubscriptionChecksModel::addOrUpdateObjects(const QJsonArray& objs)
{
    return m_connection->addOrUpdateSubscriptionChecks(objs);
}

void HyDownloaderSubscriptionChecksModel::refresh(bool full)
{
    if(full) clear();
    if(m_lastRequestedIDs.has_value()) {
        m_connection->requestSubscriptionChecksData(m_lastRequestedIDs.value(), m_showArchived);
        m_statusText = "Loading subscription check history...";
        emit statusTextChanged(m_statusText);
    }
}

void HyDownloaderSubscriptionChecksModel::clear(bool full)
{
    HyDownloaderJSONObjectListModel::clear(full);
    if(full) {
        m_lastRequestedIDs.reset();
    }
    m_statusText = "No data loaded";
    emit statusTextChanged(m_statusText);
}

void HyDownloaderSubscriptionChecksModel::loadDataForSubscriptions(const QVector<int>& subscriptionIDs)
{
    m_lastRequestedIDs = subscriptionIDs;
    refresh();
}

QString HyDownloaderSubscriptionChecksModel::statusText() const
{
    return m_statusText;
}

bool HyDownloaderSubscriptionChecksModel::showArchived() const
{
    return m_showArchived;
}

void HyDownloaderSubscriptionChecksModel::setShowArchived(bool show)
{
    if(m_showArchived != show) {
        m_showArchived = show;
        emit showArchivedChanged(show);
    }
}

void HyDownloaderSubscriptionChecksModel::handleSubscriptionChecksData(uint64_t, const QJsonArray& data)
{
    if(!m_lastRequestedIDs) {
        return;
    }
    if(const auto& ids = m_lastRequestedIDs.value(); !ids.isEmpty()) {
        const auto size = ids.size();
        if(size == 1) {
            m_statusText = QString{"Subscription check history for subscription %1"}.arg(ids[0]);
        } else {
            m_statusText = QString{"Subscription check history for %1 subscriptions"}.arg(size);
        }
    } else {
        m_statusText = "Subscription check history for all subscriptions";
    }
    emit statusTextChanged(m_statusText);
    updateFromRowData(data);
}
