#include "vspdatamodel.h"
#include <QDebug>
#include <QList>
#include <QRect>
#include <QSize>
#include <QString>

VSPDataModel::VSPDataModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

QModelIndex VSPDataModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return {};
}

int VSPDataModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_records.size();
}

int VSPDataModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if (dataType() == TDataType::PortItem) {
        return 2;
    }
    return 4;
}

QVariant VSPDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        switch (role) {
            case Qt::DisplayRole: {
                switch (section) {
                    case 0:
                        return QVariant::fromValue(tr("ID"));
                    case 1:
                        return QVariant::fromValue(tr("Name"));
                    case 2:
                        return QVariant::fromValue(tr("Port A"));
                    case 3:
                        return QVariant::fromValue(tr("Port B"));
                }
                break;
            }
            case Qt::SizeHintRole: {
                switch (section) {
                    case 0:
                        return QVariant::fromValue(QSize(30, 22));
                    case 1:
                        return QVariant::fromValue(QSize(230, 22));
                    case 2:
                        return QVariant::fromValue(QSize(60, 22));
                    case 3:
                        return QVariant::fromValue(QSize(60, 22));
                }
                break;
            }
        }
    }
    return QVariant();
}

bool VSPDataModel::hasChildren(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return false;
}

bool VSPDataModel::canFetchMore(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return false;
}

void VSPDataModel::fetchMore(const QModelIndex& parent)
{
    Q_UNUSED(parent);
}

QVariant VSPDataModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_records.size()) {
        return QVariant();
    }

    const TDataRecord r = m_records.at(index.row());

    switch (role) {
        case Qt::DisplayRole: {
            switch (r.type) {
                case TDataType::PortItem: {
                    switch (index.column()) {
                        case 0: { // * id
                            return QVariant::fromValue(QString::number(r.port.id));
                        }
                        case 1: { // * name
                            return QVariant::fromValue(r.port.name);
                        }
                    }
                    break;
                }
                case TDataType::PortLink: {
                    switch (index.column()) {
                        case 0: { // * id
                            return QVariant::fromValue(QString::number(r.link.id));
                        }
                        case 1: { // * name
                            return QVariant::fromValue(r.link.name);
                        }
                        case 2: { // port 1
                            return QVariant::fromValue(r.link.source.name);
                        }
                        case 3: { // port 2
                            return QVariant::fromValue(r.link.target.name);
                        }
                    }
                    break;
                }
            }
            break;
        }
        case Qt::UserRole: {
            return QVariant::fromValue(r);
            break;
        }
        case Qt::FontRole: {
            break;
        }
        case Qt::BackgroundRole: {
            break;
        }
        case Qt::ForegroundRole: {
            break;
        }
    }

    return QVariant();
}

void VSPDataModel::resetModel()
{
    beginResetModel();
    m_records.clear();
    endResetModel();
}

void VSPDataModel::append(const TPortItem& port)
{
    if (dataType() != TDataType::PortItem)
        return;

    foreach (auto r, m_records) {
        if (r.port.id == port.id) {
            qWarning() << "Serial port" << port.id << "already assigned, skip.";
            return;
        }
    }
    beginResetModel();
    m_records.append({TDataType::PortItem, port, {}});
    endResetModel();
}

void VSPDataModel::append(const TPortLink& link)
{
    if (dataType() != TDataType::PortLink)
        return;

    foreach (auto r, m_records) {
        if (r.link.id == link.id) {
            qWarning() << "Port link" << link.id << "already assigned, skip.";
            return;
        }
    }
    beginResetModel();
    m_records.append({TDataType::PortLink, {}, link});
    endResetModel();
}

QVariant VSPDataModel::at(int index) const
{
    if (index < 0 || index >= m_records.size()) {
        return {};
    }
    return QVariant::fromValue(m_records.at(index));
}
