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

#include <QObject>
#include <QSslConfiguration>
#include <QNetworkReply>

class QNetworkAccessManager;
class QNetworkReply;

class HyDownloaderConnection : public QObject
{
    Q_OBJECT

public:
    explicit HyDownloaderConnection(QObject* parent = nullptr);
    enum class RequestType {
        StaticData,
        StatusInformation,
        SubscriptionData,
        SubscriptionChecksData,
        MissedSubscriptionChecksData,
        SingleURLQueueData,
        APIVersion
    };
    Q_ENUM(RequestType)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_INVOKABLE bool enabled() const;
    Q_INVOKABLE QString apiURL() const;
    Q_INVOKABLE QString accessKey() const;
    Q_INVOKABLE bool isStrictTransportSecurityEnabled() const;
    Q_INVOKABLE bool isCertificateVerificationEnabled() const;
    Q_INVOKABLE int transferTimeout() const;

public slots:
    void setAPIURL(const QString& url);
    void setAccessKey(const QString& key);
    void setStrictTransportSecurityEnabled(bool enabled);
    void setCertificateVerificationEnabled(bool enabled);
    void setTransferTimeout(int timeout);
    void setEnabled(bool enabled);
    std::uint64_t requestURLForSubscriptionData(const QString& downloader, const QString& keywords);
    std::uint64_t requestLastFilesForURLs(const QVector<int>& urlIDs);
    std::uint64_t requestLastFilesForSubscriptions(const QVector<int>& subscriptionIDs);
    std::uint64_t requestStaticData(QString filePath);
    std::uint64_t requestStatusInformation();
    std::uint64_t requestSubscriptionData();
    std::uint64_t requestSubscriptionChecksData(const QVector<int>& subscriptionIDs = {}, bool showArchived = false);
    std::uint64_t requestMissedSubscriptionChecksData(const QVector<int>& subscriptionIDs = {}, bool showArchived = false);
    std::uint64_t requestSingleURLQueueData(bool showArchived = false);
    std::uint64_t requestAPIVersion();
    std::uint64_t deleteURLs(const QVector<int>& ids);
    std::uint64_t deleteSubscriptions(const QVector<int>& ids);
    std::uint64_t addOrUpdateURLs(const QJsonArray& data);
    std::uint64_t addOrUpdateSubscriptions(const QJsonArray& data);
    std::uint64_t addOrUpdateSubscriptionChecks(const QJsonArray& data);
    std::uint64_t addOrUpdateMissedSubscriptionChecks(const QJsonArray& data);
    std::uint64_t pauseSubscriptions();
    std::uint64_t resumeSubscriptions();
    std::uint64_t pauseSingleURLQueue();
    std::uint64_t resumeSingleURLQueue();
    std::uint64_t runTests(const QStringList& sites);
    std::uint64_t runReport(bool verbose);
    std::uint64_t stopCurrentSubscription();
    std::uint64_t stopCurrentURL();
    void shutdown();


signals:
    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors);
    void networkError(std::uint64_t requestID, int status, QNetworkReply::NetworkError error, const QString& errorText);
    void staticDataReceived(std::uint64_t requestID, const QByteArray& data);
    void statusInformationReceived(std::uint64_t requestID, const QJsonObject& info);
    void subscriptionDataReceived(std::uint64_t requestID, const QJsonArray& data);
    void singleURLQueueDataReceived(std::uint64_t requestID, const QJsonArray& data);
    void subscriptionChecksDataReceived(std::uint64_t requestID, const QJsonArray& data);
    void missedSubscriptionChecksDataReceived(std::uint64_t requestID, const QJsonArray& data);
    void apiVersionReceived(std::uint64_t requestID, int version);
    void replyReceived(std::uint64_t requestID, const QJsonDocument& data);
    void enabledChanged(bool);

private slots:
    void handleNetworkReplyFinished(QNetworkReply* reply);
    QNetworkReply* post(const QString& endpoint, const QJsonDocument& body);
    QNetworkReply* get(const QString& endpoint, const QMap<QString, QString>& args);

private:
    QNetworkAccessManager* m_nam = nullptr;
    QSslConfiguration m_sslConfig;
    QString m_apiURL;
    QString m_accessKey;
    std::atomic_uint64_t m_requestIDCounter = 0;
    QNetworkReply* setRequestID(QNetworkReply* reply);
    bool m_enabled = true;
};
