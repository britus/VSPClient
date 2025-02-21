// ********************************************************************
// vspdriverclient.h - VSPDriver user client interface
//
// Copyright © 2025 by EoF Software Labs
// Copyright © 2024 Apple Inc. (some copied parts)
// SPDX-License-Identifier: MIT
// ********************************************************************
#pragma once
#include <QObject>
#include <vspcontroller.hpp>
#include <vspdatamodel.h>
#include <vspdriversetup.hpp>

#define kIOErrorNotFound -536870160

using namespace VSPClient;

class VSPDriverClient: public QObject, public VSPController, public VSPDriverSetup
{
    Q_OBJECT

public:
    VSPDriverClient(QObject* parent = nullptr);

    virtual ~VSPDriverClient();

    inline VSPPortListModel* portList()
    {
        return &m_portList;
    }

    inline VSPLinkListModel* linkList()
    {
        return &m_linkList;
    }

    // Interface VSPSetup.framework
    void OnDidFailWithError(uint32_t /*code*/, const char* /*message*/) override;
    void OnDidFinishWithResult(uint32_t /*code*/, const char* /*message*/) override;
    void OnNeedsUserApproval() override;

signals:
    // VSPSetup interface events
    void didFailWithError(uint32_t code, const char* message);
    void didFinishWithResult(uint32_t code, const char* message);
    void needsUserApproval();
    // VSPController interface events
    void connected();
    void disconnected();
    void errorOccured(int error, const QString& message);
    void updateStatusLog(const QByteArray& message);
    void updateButtons(bool enabled = false);
    void complete();
    // --
    void commandResult(TVSPControlCommand command, VSPPortListModel* portModel, VSPLinkListModel* linkModel);

protected:
    // Interface VSPController.framework
    void OnConnected() override;
    void OnDisconnected() override;
    void OnIOUCCallback(int result, void* args, uint32_t numArgs) override;
    void OnErrorOccured(int error, const char* message) override;
    void OnDataReady(void* data) override;

private:
    VSPPortListModel m_portList;
    VSPLinkListModel m_linkList;
};
Q_DECLARE_METATYPE(TVSPControllerData)
Q_DECLARE_METATYPE(TVSPPortParameters)
