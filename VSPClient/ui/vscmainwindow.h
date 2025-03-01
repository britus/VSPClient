// ********************************************************************
// vscmainwindow.h - Application window
//
// Copyright © 2025 by EoF Software Labs
// Copyright © 2024 Apple Inc. (some copied parts)
// SPDX-License-Identifier: MIT
// ********************************************************************
#pragma once
#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QToolButton>
#include <QWidget>
#include <vspabstractpage.h>
#include <vspdatamodel.h>
#include <vspdriverclient.h>

QT_BEGIN_NAMESPACE

namespace Ui {
class VSCMainWindow;
}

QT_END_NAMESPACE

class VSCMainWindow: public QMainWindow
{
    Q_OBJECT

public:
    VSCMainWindow(QWidget* parent = nullptr);
    ~VSCMainWindow();

    void closeEvent(QCloseEvent* event) override Q_OVERRIDE(QMainWindow);

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onClientError(int error, const QString& message);
    void onUpdateStatusLog(const QByteArray& message);
    void onUpdateButtons(bool enabled = false);
    void onCommandResult(VSPClient::TVSPControlCommand command, VSPPortListModel* portModel, VSPLinkListModel* linkModel);
    void onComplete();

    void onSetupFailWithError(uint32_t code, const char* message);
    void onSetupFinishWithResult(uint32_t code, const char* message);
    void onSetupNeedsUserApproval();

    void onSelectPage();
    void onActionExecute(const VSPClient::TVSPControlCommand command, const QVariant& data);
    void onActionInstall();
    void onActionUninstall();

protected:
    void resizeEvent(QResizeEvent* event) override;

protected slots:
    virtual void updateOverlayGeometry();

private:
    Ui::VSCMainWindow* ui;
    VSPDriverClient* m_vsp;
    QMap<QPushButton*, VSPAbstractPage*> m_buttonMap;
    QMap<uint, QString> m_errorStack;
    QMessageBox m_box;
    QSystemTrayIcon stIcon;
    bool m_firstStart;
    bool m_demoMode;

private:
    inline void setupSystemTray();
    inline void showDemoMessage(const QString& message);
    inline void showNotification(int ms, const QString& text);
    inline void resetDefaultButton(QWidget* view);
    inline void enableDefaultButton(QPushButton* button);
    inline void disableDefaultButton(QPushButton* button);
    inline void enableButton(QList<QPushButton*> buttons);
    inline void enableButton(QPushButton* button);
    inline void disableButton(QList<QPushButton*> buttons);
    inline void disableButton(QPushButton* button);
    inline void showOverlay();
    inline void removeOverlay();
};

class PopupMenu: public QMenu
{
    Q_OBJECT

public:
    explicit PopupMenu(QWidget* target, QWidget* parent = 0);
    void showEvent(QShowEvent* event);

private:
    QWidget* w;
};
