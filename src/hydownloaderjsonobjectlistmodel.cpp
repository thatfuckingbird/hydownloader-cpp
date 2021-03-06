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

#include <hydownloader-cpp/hydownloaderjsonobjectlistmodel.h>
#include <hydownloader-cpp/hydownloaderconnection.h>
#include <QJsonObject>

void HyDownloaderJSONObjectListModel::setConnection(HyDownloaderConnection* connection)
{
    auto oldConnection = m_connection;
    m_connection = connection;
    setUpConnections(oldConnection);
    if(oldConnection) disconnect(oldConnection, &HyDownloaderConnection::replyReceived, this, &HyDownloaderJSONObjectListModel::handleReplyReceived);
    connect(m_connection, &HyDownloaderConnection::replyReceived, this, &HyDownloaderJSONObjectListModel::handleReplyReceived);
    clear();
}

QVector<int> HyDownloaderJSONObjectListModel::getIDs(const QModelIndexList& indices) const
{
    QVector<int> res;
    res.reserve(indices.size());
    for(const auto& index: indices) {
        res.append(m_data[index.row()].toObject()["id"].toInt());
    }
    return res;
}

void HyDownloaderJSONObjectListModel::clear(bool full)
{
    beginResetModel();
    m_data = {};
    if(full) {
        m_updateIDs.clear();
    }
    endResetModel();
}

int HyDownloaderJSONObjectListModel::columnCount(const QModelIndex& parent) const
{
    if(parent.isValid()) return 0;

    return m_columnData.size();
}

int HyDownloaderJSONObjectListModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) return 0;

    return m_data.size();
}

QVariant HyDownloaderJSONObjectListModel::data(const QModelIndex& index, int role) const
{
    if(index.parent().isValid() || (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::ToolTipRole)) return {};

    const auto& conversionFunction = std::get<3>(m_columnData[index.column()]);
    const auto& key = std::get<0>(m_columnData[index.column()]);
    return conversionFunction(m_data[index.row()][key]);
}

bool HyDownloaderJSONObjectListModel::setData(const QModelIndex& index, const QVariant& value, int)
{
    if(index.parent().isValid()) return false;
    if(!std::get<2>(m_columnData[index.column()])) return false;

    const auto& key = std::get<0>(m_columnData[index.column()]);
    const auto& conversionFunction = std::get<4>(m_columnData[index.column()]);
    QJsonValue jsonValue = conversionFunction(value);
    QJsonObject tmp = m_data[index.row()].toObject();
    tmp[key] = jsonValue;
    m_data[index.row()] = tmp;

    QJsonObject updateObj;
    updateObj["id"] = m_data[index.row()].toObject()["id"];
    updateObj[key] = jsonValue;

    m_updateIDs.insert(addOrUpdateObjects({updateObj}));

    return true;
}

Qt::ItemFlags HyDownloaderJSONObjectListModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | (index.isValid() && std::get<2>(m_columnData[index.column()]) ? Qt::ItemIsEditable : Qt::NoItemFlags);
}

QVariant HyDownloaderJSONObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section < 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};

    return std::get<1>(m_columnData[section]);
}

QJsonObject HyDownloaderJSONObjectListModel::getRowData(const QModelIndex& rowIndex) const
{
    return m_data[rowIndex.row()].toObject();
}

QJsonObject HyDownloaderJSONObjectListModel::getBasicRowData(const QModelIndex& rowIndex) const
{
    QJsonObject obj;
    obj[m_idColumnName] = m_data[rowIndex.row()].toObject()[m_idColumnName];
    return obj;
}

void HyDownloaderJSONObjectListModel::updateRowData(const QModelIndexList& indices, const QJsonArray& objs, bool removeRows)
{
    if(indices.size() != objs.size() || indices.isEmpty()) return;
    int minRow = std::numeric_limits<int>::max();
    int maxRow = 0;
    for(int i = 0; i < indices.size(); ++i) {
        const int row = indices[i].row();
        auto oldObject = m_data[row].toObject();
        auto newObject = objs[i].toObject();
        for(const auto& key: newObject.keys()) {
            oldObject[key] = newObject[key];
        }
        m_data[row] = oldObject;
        if(row > maxRow) {
            maxRow = row;
        }
        if(row < minRow) {
            minRow = row;
        }
    }
    m_updateIDs.insert(addOrUpdateObjects(objs));
    emit dataChanged(createIndex(minRow, 0), createIndex(maxRow, m_columnData.size() - 1));

    if(removeRows) {
        auto sortedIndices = indices;
        std::sort(sortedIndices.begin(), sortedIndices.end(), [](const QModelIndex& a, const QModelIndex& b){
            return a.row() < b.row();
        });
        std::vector<std::pair<int, int>> continuousRanges;
        int start = sortedIndices[0].row(), end = sortedIndices[0].row();
        for(int i = 1; i < sortedIndices.size(); ++i) {
            if(sortedIndices[i-1].row()+1 == sortedIndices[i].row()) {
                end = sortedIndices[i].row();
            } else {
                continuousRanges.push_back({start, end});
                start = sortedIndices[i].row();
                end = sortedIndices[i].row();
            }
        }
        continuousRanges.push_back({start, end});
        int alreadyRemoved = 0;
        for(const auto& range : continuousRanges) {
            beginRemoveRows({}, range.first, range.second);
            for(int i = 0; i < end - start + 1; ++i) {
                m_data.removeAt(start-alreadyRemoved);
            }
            alreadyRemoved += end - start + 1;
            endRemoveRows();
        }
    }
}

void HyDownloaderJSONObjectListModel::handleReplyReceived(uint64_t requestID, const QJsonDocument&)
{
    if(m_updateIDs.contains(requestID)) {
        m_updateIDs.remove(requestID);
        refresh(false);
    }
}

void HyDownloaderJSONObjectListModel::updateFromRowData(const QJsonArray& arr)
{
    if(arr.isEmpty()) {
        clear();
        return;
    }
    int commonPrefixLength = 0;
    for(; commonPrefixLength < arr.size() && commonPrefixLength < m_data.size(); ++commonPrefixLength) {
        if(arr[commonPrefixLength] != m_data[commonPrefixLength]) break;
    }
    int oldDataLength = m_data.size();
    int newDataLength = arr.size();
    if(oldDataLength < newDataLength) {
        beginInsertRows({}, oldDataLength, newDataLength - 1);
        m_data = arr;
        endInsertRows();
    } else if(oldDataLength > newDataLength) {
        beginRemoveRows({}, newDataLength, oldDataLength - 1);
        m_data = arr;
        endRemoveRows();
    } else {
        m_data = arr;
    }
    if(commonPrefixLength < m_data.size()) {
        emit dataChanged(createIndex(commonPrefixLength, 0), createIndex(m_data.size() - 1, m_columnData.size() - 1));
    }
}
