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

#include <hydownloader-cpp/hydownloaderconnection.h>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

HyDownloaderConnection::HyDownloaderConnection(QObject* parent) :
    QObject(parent)
{
    m_sslConfig = QSslConfiguration::defaultConfiguration();
    m_nam = new QNetworkAccessManager{this};
    connect(this->m_nam, &QNetworkAccessManager::finished, this, &HyDownloaderConnection::handleNetworkReplyFinished);
    connect(this->m_nam, &QNetworkAccessManager::sslErrors, this, &HyDownloaderConnection::sslErrors);
}

bool HyDownloaderConnection::enabled() const
{
    return m_enabled;
}

void HyDownloaderConnection::setAPIURL(const QString& url)
{
    m_apiURL = url;
}

QString HyDownloaderConnection::apiURL() const
{
    return m_apiURL;
}

void HyDownloaderConnection::setAccessKey(const QString& key)
{
    m_accessKey = key;
}

QString HyDownloaderConnection::accessKey() const
{
    return m_accessKey;
}

void HyDownloaderConnection::setStrictTransportSecurityEnabled(bool enabled)
{
    m_nam->setStrictTransportSecurityEnabled(enabled);
}

bool HyDownloaderConnection::isStrictTransportSecurityEnabled() const
{
    return m_nam->isStrictTransportSecurityEnabled();
}

void HyDownloaderConnection::setCertificateVerificationEnabled(bool enabled)
{
    m_sslConfig.setPeerVerifyMode(enabled ? QSslSocket::AutoVerifyPeer : QSslSocket::VerifyNone);
}

bool HyDownloaderConnection::isCertificateVerificationEnabled() const
{
    return m_sslConfig.peerVerifyMode() != QSslSocket::VerifyNone;
}

void HyDownloaderConnection::setTransferTimeout(int timeout)
{
    m_nam->setTransferTimeout(timeout);
}

int HyDownloaderConnection::transferTimeout() const
{
    return m_nam->transferTimeout();
}

void HyDownloaderConnection::setEnabled(bool enabled)
{
    if(m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged(m_enabled);
    }
}

uint64_t HyDownloaderConnection::requestURLForSubscriptionData(const QString &downloader, const QString &keywords)
{
    if(!m_enabled) return 0;
    QJsonObject obj;
    obj["downloader"] = downloader;
    obj["keywords"] = keywords;
    return post("/subscription_data_to_url", QJsonDocument{obj})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestLastFilesForURLs(const QVector<int>& urlIDs)
{
    if(!m_enabled) return 0;
    QJsonArray array;
    for(const auto id: urlIDs) array.push_back(id);
    QJsonObject obj;
    obj["ids"] = array;
    return post("/urls_last_files", QJsonDocument{obj})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestLastFilesForSubscriptions(const QVector<int>& subscriptionIDs)
{
    if(!m_enabled) return 0;
    QJsonArray array;
    for(const auto id: subscriptionIDs) array.push_back(id);
    QJsonObject obj;
    obj["ids"] = array;
    return post("/subscriptions_last_files", QJsonDocument{obj})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestStaticData(QString filePath)
{
    if(!m_enabled) return 0;
    if(!filePath.startsWith("/")) filePath = "/" + filePath;
    auto reply = get(filePath, {});
    reply->setProperty("requestType", QVariant::fromValue(RequestType::StaticData));
    return reply->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestStatusInformation()
{
    if(!m_enabled) return 0;
    auto reply = post("/get_status_info", {});
    reply->setProperty("requestType", QVariant::fromValue(RequestType::StatusInformation));
    return reply->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestSubscriptionData()
{
    if(!m_enabled) return 0;
    auto reply = post("/get_subscriptions", {});
    reply->setProperty("requestType", QVariant::fromValue(RequestType::SubscriptionData));
    return reply->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestSubscriptionChecksData(const QVector<int>& subscriptionIDs, bool showArchived)
{
    if(!m_enabled) return 0;
    QJsonObject obj;
    QJsonArray array;
    for(const auto id: subscriptionIDs) array.push_back(id);
    obj["ids"] = array;
    obj["archived"] = showArchived;
    auto reply = post("/get_subscription_checks", QJsonDocument{obj});
    reply->setProperty("requestType", QVariant::fromValue(RequestType::SubscriptionChecksData));
    return reply->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestMissedSubscriptionChecksData(const QVector<int>& subscriptionIDs, bool showArchived)
{
    if(!m_enabled) return 0;
    QJsonObject obj;
    QJsonArray array;
    for(const auto id: subscriptionIDs) array.push_back(id);
    obj["ids"] = array;
    obj["archived"] = showArchived;
    auto reply = post("/get_missed_subscription_checks", QJsonDocument{obj});
    reply->setProperty("requestType", QVariant::fromValue(RequestType::MissedSubscriptionChecksData));
    return reply->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestSingleURLQueueData(bool showArchived)
{
    if(!m_enabled) return 0;
    QJsonObject obj;
    obj["archived"] = showArchived;
    auto reply = post("/get_queued_urls", QJsonDocument{obj});
    reply->setProperty("requestType", QVariant::fromValue(RequestType::SingleURLQueueData));
    return reply->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::requestAPIVersion()
{
    if(!m_enabled) return 0;
    auto reply = post("/api_version", {});
    return reply->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::deleteURLs(const QVector<int>& ids)
{
    if(!m_enabled) return 0;
    QJsonArray array;
    for(const auto id: ids) array.push_back(id);
    QJsonObject obj;
    obj["ids"] = array;
    return post("/delete_urls", QJsonDocument{obj})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::deleteSubscriptions(const QVector<int>& ids)
{
    if(!m_enabled) return 0;
    QJsonArray array;
    for(const auto id: ids) array.push_back(id);
    QJsonObject obj;
    obj["ids"] = array;
    return post("/delete_subscriptions", QJsonDocument{obj})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::addOrUpdateURLs(const QJsonArray& data)
{
    if(!m_enabled) return 0;
    return post("/add_or_update_urls", QJsonDocument{data})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::addOrUpdateSubscriptions(const QJsonArray& data)
{
    if(!m_enabled) return 0;
    return post("/add_or_update_subscriptions", QJsonDocument{data})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::addOrUpdateSubscriptionChecks(const QJsonArray& data)
{
    if(!m_enabled) return 0;
    return post("/add_or_update_subscription_checks", QJsonDocument{data})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::addOrUpdateMissedSubscriptionChecks(const QJsonArray& data)
{
    if(!m_enabled) return 0;
    return post("/add_or_update_missed_subscription_checks", QJsonDocument{data})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::pauseSubscriptions()
{
    if(!m_enabled) return 0;
    return post("/pause_subscriptions", {})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::resumeSubscriptions()
{
    if(!m_enabled) return 0;
    return post("/resume_subscriptions", {})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::pauseSingleURLQueue()
{
    if(!m_enabled) return 0;
    return post("/pause_single_urls", {})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::resumeSingleURLQueue()
{
    if(!m_enabled) return 0;
    return post("/resume_single_urls", {})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::runTests(const QStringList& sites)
{
    if(!m_enabled) return 0;
    QJsonObject obj;
    obj["sites"] = sites.join(",");
    return post("/run_tests", QJsonDocument{obj})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::runReport(bool verbose)
{
    if(!m_enabled) return 0;
    QJsonObject obj;
    obj["verbose"] = verbose;
    return post("/run_report", QJsonDocument{obj})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::stopCurrentSubscription()
{
    if(!m_enabled) return 0;
    return post("/kill_current_sub", {})->property("requestID").toULongLong();
}

uint64_t HyDownloaderConnection::stopCurrentURL()
{
    if(!m_enabled) return 0;
    return post("/kill_current_url", {})->property("requestID").toULongLong();
}

void HyDownloaderConnection::shutdown()
{
    if(!m_enabled) return;
    post("/shutdown", {});
}

QNetworkReply* HyDownloaderConnection::get(const QString& endpoint, const QMap<QString, QString>& args)
{
    auto apiURL = QUrl{m_apiURL + endpoint};

    QUrlQuery query{apiURL};
    for(auto it = args.begin(); it != args.end(); ++it) query.addQueryItem(it.key(), it.value());
    apiURL.setQuery(query);

    QNetworkRequest req{apiURL};
    req.setSslConfiguration(m_sslConfig);
    req.setPriority(QNetworkRequest::HighPriority);
    req.setRawHeader("HyDownloader-Access-Key", m_accessKey.toUtf8());

    return setRequestID(m_nam->get(req));
}

QNetworkReply* HyDownloaderConnection::setRequestID(QNetworkReply* reply)
{
    reply->setProperty("requestID", QVariant::fromValue(++m_requestIDCounter));
    return reply;
}

QNetworkReply* HyDownloaderConnection::post(const QString& endpoint, const QJsonDocument& body)
{
    auto apiURL = QUrl{m_apiURL + endpoint};

    QNetworkRequest req{apiURL};
    req.setSslConfiguration(m_sslConfig);
    req.setRawHeader("HyDownloader-Access-Key", m_accessKey.toUtf8());
    req.setRawHeader("Content-Type", "application/json");

    return setRequestID(m_nam->post(req, body.isEmpty() ? "{}" : body.toJson()));
}

void HyDownloaderConnection::handleNetworkReplyFinished(QNetworkReply* reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    std::uint64_t reqID = reply->property("requestID").toULongLong();
    if(statusCode == 200) {
        if(auto reqType = reply->property("requestType"); reqType.isValid()) {
            switch(reqType.value<RequestType>()) {
                case RequestType::StaticData:
                    emit staticDataReceived(reqID, reply->readAll());
                    break;
                case RequestType::StatusInformation:
                    emit statusInformationReceived(reqID, QJsonDocument::fromJson(reply->readAll()).object());
                    break;
                case RequestType::SubscriptionData:
                    emit subscriptionDataReceived(reqID, QJsonDocument::fromJson(reply->readAll()).array());
                    break;
                case RequestType::SingleURLQueueData:
                    emit singleURLQueueDataReceived(reqID, QJsonDocument::fromJson(reply->readAll()).array());
                    break;
                case RequestType::APIVersion:
                    emit apiVersionReceived(reqID, QJsonDocument::fromJson(reply->readAll()).object()["version"].toInt());
                    break;
                case RequestType::SubscriptionChecksData:
                    emit subscriptionChecksDataReceived(reqID, QJsonDocument::fromJson(reply->readAll()).array());
                case RequestType::MissedSubscriptionChecksData:
                    emit missedSubscriptionChecksDataReceived(reqID, QJsonDocument::fromJson(reply->readAll()).array());
            }
        } else {
            emit replyReceived(reqID, QJsonDocument::fromJson(reply->readAll()));
        }
    } else {
        emit networkError(reqID, statusCode, reply->error(), reply->errorString());
    }
    reply->deleteLater();
}
