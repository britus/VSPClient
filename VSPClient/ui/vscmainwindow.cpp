// ********************************************************************
// vscmainwindow.cpp - Application window
//
// Copyright © 2025 by EoF Software Labs
// Copyright © 2024 Apple Inc. (some copied parts)
// SPDX-License-Identifier: MIT
// ********************************************************************
#include "ui_vscmainwindow.h"
#include <QAction>
#include <QDebug>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QMovie>
#include <QSplitter>
#include <QTimer>
#include <vscmainwindow.h>
#include <vspabstractpage.h>
#include <vspserialio.h>

#define COPYRIGHT "Copyright © 2025 by EoF Software Labs"

VSCMainWindow::VSCMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::VSCMainWindow)
    , m_vsp(nullptr)
    , m_buttonMap()
    , m_box(this)
{
    ui->setupUi(this);

    m_vsp = new VSPDriverClient(this);
    connect(m_vsp, &VSPDriverClient::didFailWithError, this, &VSCMainWindow::onSetupFailWithError);
    connect(m_vsp, &VSPDriverClient::didFinishWithResult, this, &VSCMainWindow::onSetupFinishWithResult);
    connect(m_vsp, &VSPDriverClient::needsUserApproval, this, &VSCMainWindow::onSetupNeedsUserApproval);
    connect(m_vsp, &VSPDriverClient::connected, this, &VSCMainWindow::onClientConnected);
    connect(m_vsp, &VSPDriverClient::disconnected, this, &VSCMainWindow::onClientDisconnected);
    connect(m_vsp, &VSPDriverClient::errorOccured, this, &VSCMainWindow::onClientError);
    connect(m_vsp, &VSPDriverClient::updateStatusLog, this, &VSCMainWindow::onUpdateStatusLog);
    connect(m_vsp, &VSPDriverClient::updateButtons, this, &VSCMainWindow::onUpdateButtons);
    connect(m_vsp, &VSPDriverClient::commandResult, this, &VSCMainWindow::onCommandResult);
    connect(m_vsp, &VSPDriverClient::complete, this, &VSCMainWindow::onComplete);

    QSplitter* splitter = new QSplitter(Qt::Vertical, ui->pnlContent);
    ui->pnlContent->layout()->addWidget(splitter);
    splitter->addWidget(ui->stackedWidget);
    splitter->addWidget(ui->groupBox);
    splitter->setHandleWidth(4);
    splitter->setSizes(QList<int>() << 180 << 140);

    m_buttonMap[ui->btn01SPCreate] = ui->pg01SPCreate;
    m_buttonMap[ui->btn02SPRemove] = ui->pg02SPRemove;
    m_buttonMap[ui->btn03LKCreate] = ui->pg03LKCreate;
    m_buttonMap[ui->btn04LKRemove] = ui->pg04LKRemove;
    m_buttonMap[ui->btn05PortList] = ui->pg05PortList;
    m_buttonMap[ui->btn06LinkList] = ui->pg06LinkList;
    m_buttonMap[ui->btn07Checks] = ui->pg07Checks;
    m_buttonMap[ui->btn08Traces] = ui->pg08Traces;
    m_buttonMap[ui->btn09Connect] = ui->pg09Connect;

    foreach (auto page, m_buttonMap.values()) {
        connect(page, &VSPAbstractPage::execute, this, &VSCMainWindow::onActionExecute);
    }

    foreach (auto button, m_buttonMap.keys()) {
        connect(button, &QPushButton::clicked, this, &VSCMainWindow::onSelectPage);
    }

    connect(ui->btn11SerialIO, &QPushButton::clicked, this, [this]() {
        (new VSPSerialIO(this))->setVisible(true);
    });

    connect(qApp, &QGuiApplication::saveStateRequest, this, [](QSessionManager&) {
        // -- saveSettings();
    });

    connect(qApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state != Qt::ApplicationActive) {
            showNotification(5000, "");
        }
    });

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        if (QSystemTrayIcon::supportsMessages()) {
            setupSystemTray();
        }
    }

    // ---------

    QAction* a;
    PopupMenu* menu = new PopupMenu(ui->toolButton, this);

    a = new QAction("Activate VSP Driver");
    connect(a, &QAction::triggered, this, [this]() {
        m_vsp->activateDriver();
    });
    menu->addAction(a);

    a = new QAction("Deactivate VSP Driver");
    connect(a, &QAction::triggered, this, [this]() {
        m_vsp->deactivateDriver();
    });
    menu->addAction(a);

    ui->toolButton->setMenu(menu);

    // show inital driver connection page
    ui->stackedWidget->setCurrentWidget(ui->pg09Connect);

    // try to install VSPDriver and/or connect driver UC instance
    enableDefaultButton(ui->btn09Connect);
    onActionExecute(vspControlPingPong, {});
}

VSCMainWindow::~VSCMainWindow()
{
    delete m_vsp;
    delete ui;
}

void VSCMainWindow::closeEvent(QCloseEvent*)
{
    // ui->tvItems->removeEventFilter(this);
    qApp->quit();
}

inline void VSCMainWindow::showNotification(int ms, const QString& text)
{
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        stIcon.show();
        stIcon.showMessage(
           qApp->applicationDisplayName(),                     //
           COPYRIGHT + qApp->organizationName() + "\n" + text, //
           QSystemTrayIcon::MessageIcon::NoIcon,
           ms);
    }
}

inline void VSCMainWindow::setupSystemTray()
{
    stIcon.setIcon(QIcon(":/assets/png/vspclient_3.png"));
    stIcon.setToolTip(
       qApp->applicationDisplayName() //
       + " \n" COPYRIGHT + qApp->organizationName());

    connect(&stIcon, &QSystemTrayIcon::messageClicked, this, []() {
        //-
    });
    connect(&stIcon, &QSystemTrayIcon::activated, this, []() {
        //-
    });

    QMenu* menu = new QMenu(this);
    QAction* a;

    a = new QAction(stIcon.icon(), "About");
    connect(a, &QAction::triggered, this, [this]() {
        QMessageBox::about(
           this,
           qApp->applicationName(),                //
           qApp->applicationDisplayName() + "\n\n" //
              + tr(COPYRIGHT) + "\nWritten by "    //
              + qApp->organizationName());
        // showNotification(5000, "-:o:-");
    });
    menu->addAction(a);

    a = new QAction(stIcon.icon(), "Open");
    connect(a, &QAction::triggered, this, [this]() {
        setWindowState(Qt::WindowState::WindowActive);
        show();
        raise();
    });
    menu->addAction(a);
    menu->addSeparator();

    a = new QAction(stIcon.icon(), "Close");
    connect(a, &QAction::triggered, this, [this]() {
        qApp->postEvent(this, new QCloseEvent());
    });
    menu->addAction(a);

    stIcon.setContextMenu(menu);
    stIcon.setVisible(true);
}

inline void VSCMainWindow::showOverlay()
{
    QWidget* overlay;
    QLabel* gifLabel;

    qDebug("CTRLWIN::showOverlay():\n");

    // Overlay-Widget erstellen
    overlay = new QWidget(centralWidget());

    // Mausereignisse durchlassen
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 80% transparent
    overlay->setStyleSheet( //
       "background-color: rgba(0, 0, 0, 51); "
       "border-color: rgb(252, 115, 9); "
       "border-style: solid; "
       "border-width: 1px; "
       "border-radius: 5px;");
    setProperty("overlay", QVariant::fromValue(overlay));

    // GIF-Label erstellen
    gifLabel = new QLabel(overlay);
    QMovie* movie = new QMovie(":/progress1", QByteArray(), this);
    gifLabel->setMovie(movie);
    movie->start();

    // Overlay-Größe anpassen
    QTimer* resizer = new QTimer(this);
    connect(resizer, &QTimer::timeout, this, &VSCMainWindow::updateOverlayGeometry);
    resizer->setSingleShot(true);
    resizer->start(20);

    if (ui->stackedWidget->currentWidget() != ui->pg09Connect) {
        ui->pnlButtons->setEnabled(false);
        ui->pnlContent->setEnabled(false);
    }
}

inline void VSCMainWindow::removeOverlay()
{
    QVariant v = property("overlay");
    if (v.isNull() || !v.isValid()) {
        return;
    }

    qDebug("CTRLWIN::removeOverlay():\n");

    QWidget* overlay;
    if (!(overlay = v.value<QWidget*>())) {
        return;
    }

    setProperty("overlay", {});
    overlay->hide();
    overlay->deleteLater();

    ui->pnlContent->setEnabled(true);
    ui->pnlButtons->setEnabled(true);
}

void VSCMainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateOverlayGeometry();
}

void VSCMainWindow::onSetupFailWithError(uint32_t code, const char* message)
{
    qDebug("CTRLWIN::onSetupFailWithError(): code=%d msg=%s\n", code, message);

    ui->textBrowser->setPlainText(tr("VSP setup status #%1\nInfo: %2") //
                                     .arg(code)
                                     .arg(message));
}

void VSCMainWindow::onSetupFinishWithResult(uint32_t code, const char* message)
{
    qDebug("CTRLWIN::onSetupFinishWithResult(): code=%d msg=%s\n", code, message);

    ui->textBrowser->setPlainText(tr("%1 %2").arg(code).arg(message));
}

void VSCMainWindow::onSetupNeedsUserApproval()
{
}

void VSCMainWindow::onClientConnected()
{
    qDebug("CTRLWIN::onClientConnected()\n");

    resetDefaultButton(ui->pnlButtons);
    enableButton(
       QList<QPushButton*>() //
       << ui->btn01SPCreate  //
       << ui->btn02SPRemove  //
       << ui->btn03LKCreate  //
       << ui->btn04LKRemove  //
       << ui->btn05PortList  //
       << ui->btn06LinkList  //
       << ui->btn07Checks    //
       << ui->btn08Traces);

    QString dn = m_vsp->DeviceName();
    QString dp = m_vsp->DevicePath();

    ui->textBrowser->setPlainText("Connected. [" + dn + ": " + dp + "]");
    ui->stackedWidget->setCurrentWidget(ui->pg01SPCreate);
    enableDefaultButton(ui->btn01SPCreate);
    disableButton(ui->btn09Connect);
}

void VSCMainWindow::onClientDisconnected()
{
    qDebug("CTRLWIN::onClientDisconnected()\n");

    resetDefaultButton(ui->pnlButtons);
    disableButton(
       QList<QPushButton*>() //
       << ui->btn01SPCreate  //
       << ui->btn02SPRemove  //
       << ui->btn03LKCreate  //
       << ui->btn04LKRemove  //
       << ui->btn05PortList  //
       << ui->btn06LinkList  //
       << ui->btn07Checks    //
       << ui->btn08Traces);

    ui->stackedWidget->setCurrentWidget(ui->pg09Connect);
    ui->textBrowser->setPlainText("Disconnected.");
    enableButton(ui->btn09Connect);
    enableDefaultButton(ui->btn09Connect);
}

void VSCMainWindow::onClientError(int error, const QString& message)
{
    qDebug("CTRLWIN::onClientError() error=%d msg=%s\n", error, qPrintable(message));

    m_errorStack[error] = message;

    QString text = "";
    foreach (auto code, m_errorStack.keys()) {
        text += tr("VSP Error %1:\n%2\n\n").arg(code).arg(m_errorStack[code]);
    }

    ui->textBrowser->setPlainText(text);

    QTimer::singleShot(50, this, [this, error, text]() {
        m_box.setWindowTitle(windowTitle());
        m_box.setText(text);
        if (!m_vsp->IsConnected() && error == kIOErrorNotFound) {
            m_box.setInformativeText(tr("You must install the VSP Driver extension first.\n"));
        }
        m_box.show();
    });
}

void VSCMainWindow::onUpdateStatusLog(const QByteArray& message)
{
    qDebug("CTRLWIN::onUpdateStatusLog(): %s\n", qPrintable(message));

    ui->textBrowser->setPlainText(message);
}

void VSCMainWindow::onUpdateButtons(bool enabled)
{
    qDebug("CTRLWIN::onUpdateButtons(): enabled=%d\n", enabled);
    if (!enabled) {
        disableButton(
           QList<QPushButton*>() //
           << ui->btn01SPCreate  //
           << ui->btn02SPRemove  //
           << ui->btn03LKCreate  //
           << ui->btn04LKRemove  //
           << ui->btn05PortList  //
           << ui->btn06LinkList  //
           << ui->btn07Checks    //
           << ui->btn08Traces);
    }
    else {
        enableButton(
           QList<QPushButton*>() //
           << ui->btn01SPCreate  //
           << ui->btn02SPRemove  //
           << ui->btn03LKCreate  //
           << ui->btn04LKRemove  //
           << ui->btn05PortList  //
           << ui->btn06LinkList  //
           << ui->btn07Checks    //
           << ui->btn08Traces);
    }
}

void VSCMainWindow::onCommandResult(TVSPControlCommand command, VSPPortListModel* portModel, VSPLinkListModel* linkModel)
{
    qDebug("CTRLWIN::onCommandResult(): cmd=%d\n", command);

    VSPAbstractPage* page;
    if ((page = dynamic_cast<VSPAbstractPage*>(ui->stackedWidget->currentWidget())) == nullptr) {
        return;
    }

    page->update(command, portModel, linkModel);
}

void VSCMainWindow::onComplete()
{
    qDebug("CTRLWIN::onComplete():\n");
    removeOverlay();
}

void VSCMainWindow::onSelectPage()
{
    QPushButton* button;

    if (!(button = dynamic_cast<QPushButton*>(sender()))) {
        return;
    }

    qDebug("CTRLWIN::onSelectPage(): button=%s\n", qPrintable(button->text()));

    VSPAbstractPage* page;
    if (!(page = m_buttonMap[button])) {
        return;
    }

    resetDefaultButton(ui->pnlButtons);
    enableDefaultButton(button);

    // show selected page
    ui->stackedWidget->setCurrentWidget(page);

    // force data model update
    onActionExecute(vspControlGetStatus, {});
}

void VSCMainWindow::onActionExecute(const TVSPControlCommand command, const QVariant& data)
{
    qDebug("CTRLWIN::onActionExecute(): cmd=%d\n", command);

    // VSPAbstractPage* page = ui->stackedWidget->currentWidget();
    // int index = ui->stackedWidget->currentIndex();

    // reset error message stack
    m_errorStack.clear();

    showOverlay();

    switch (command) {
        case vspControlGetStatus: {
            if (!m_vsp->GetStatus()) {
                onClientError(0xfa000001, tr("Failed to read driver status."));
                goto error_exit;
            }
            break;
        }
        case vspControlCreatePort: {
            TVSPPortParameters p = data.value<TVSPPortParameters>();
            if (!m_vsp->CreatePort(&p)) {
                onClientError(0xfa000001, tr("Failed to create port."));
                goto error_exit;
            }
            break;
        }
        case vspControlRemovePort: {
            VSPDataModel::TPortItem p = data.value<VSPDataModel::TPortItem>();
            if (!m_vsp->RemovePort(p.id)) {
                onClientError(0xfa000001, tr("Failed to remove port."));
                goto error_exit;
            }
            break;
        }
        case vspControlLinkPorts: {
            VSPDataModel::TPortLink link = data.value<VSPDataModel::TPortLink>();
            if (link.source.id == link.target.id) {
                onClientError(0xfa000002, "You can't link same port together.");
                goto error_exit;
            }
            if (!m_vsp->LinkPorts(link.source.id, link.target.id)) {
                onClientError(0xfa000001, tr("Failed to create port link status."));
                goto error_exit;
            }
            break;
        }
        case vspControlUnlinkPorts: {
            VSPDataModel::TPortLink link = data.value<VSPDataModel::TPortLink>();
            if (!m_vsp->UnlinkPorts(link.source.id, link.target.id)) {
                onClientError(0xfa000001, tr("Failed to remove link."));
                goto error_exit;
            }
            break;
        }
        case vspControlGetPortList: {
            if (!m_vsp->GetPortList()) {
                onClientError(0xfa000001, tr("Failed to get active ports."));
                goto error_exit;
            }
            break;
        }
        case vspControlGetLinkList: {
            if (!m_vsp->GetLinkList()) {
                onClientError(0xfa000001, tr("Failed to get active port links."));
                goto error_exit;
            }
            break;
        }
        case vspControlEnableChecks: {
            VSPDataModel::TDataRecord r = data.value<VSPDataModel::TDataRecord>();
            if (!m_vsp->EnableChecks(r.port.id)) {
                onClientError(0xfa000001, tr("Failed to enable/disable checks."));
                goto error_exit;
            }
            break;
        }
        case vspControlEnableTrace: {
            VSPDataModel::TDataRecord r = data.value<VSPDataModel::TDataRecord>();
            if (!m_vsp->EnableTrace(r.port.id)) {
                onClientError(0xfa000001, tr("Failed to enable/disable traces."));
                goto error_exit;
            }
            break;
        }
        case vspControlPingPong: {
            if (!m_vsp->IsConnected() && !m_vsp->ConnectDriver()) {
                onClientError(0xfa0000f0, "Failed to connect driver interface.");
                if (!QDesktopServices::openUrl(QUrl(QStringLiteral("vspinstall://")))) {
                    m_vsp->activateDriver();
                }
                goto error_exit;
            }
            else if (!m_vsp->GetStatus()) {
                onClientError(0xfa000001, tr("Failed to read driver status."));
                goto error_exit;
            }
            break;
        }
        default: {
            break;
        }
    }

    // done
    return;

error_exit:
    removeOverlay();
}

void VSCMainWindow::updateOverlayGeometry()
{
    QVariant v = property("overlay");
    if (v.isNull() || !v.isValid()) {
        return;
    }

    QWidget* overlay;
    if (!(overlay = v.value<QWidget*>())) {
        return;
    }

    QLabel* gifLabel;
    if (overlay->children().isEmpty() || !(gifLabel = dynamic_cast<QLabel*>(overlay->children().at(0)))) {
        return;
    }

    if (centralWidget()) {
        QRect ovr = centralWidget()->rect();
        int width = 100;  // ovr.width() - (ovr.width() * 15 / 100);
        int height = 100; // ovr.height() - (ovr.height() * 15 / 100);
        int x = ovr.width() / 2 - (width / 2);
        int y = ovr.height() / 2 - (height / 2);

        ovr.setTopLeft(QPoint(x, y));
        ovr.setSize(QSize(width, height));

        // Overlay an zentrale Widget-Größe anpassen
        overlay->setGeometry(ovr);

        // GIF zentrieren und skalieren
        gifLabel->setGeometry(overlay->rect().adjusted(2, 2, -2, -2));
        gifLabel->setAlignment(Qt::AlignCenter);
        gifLabel->movie()->setScaledSize(gifLabel->size());
    }

    QTimer* t;
    if ((t = dynamic_cast<QTimer*>(sender()))) {
        overlay->show();
        if (t->isActive()) {
            t->stop();
        }
        t->deleteLater();
    }
}

inline void VSCMainWindow::resetDefaultButton(QWidget* view)
{
    QPushButton* b;
    foreach (auto w, view->children()) {
        if ((b = dynamic_cast<QPushButton*>(w)) != nullptr) {
            b->setAutoDefault(false);
            b->setDefault(false);
        }
    }
}

inline void VSCMainWindow::enableButton(QList<QPushButton*> buttons)
{
    foreach (auto button, buttons) {
        button->setEnabled(true);
    }
}

inline void VSCMainWindow::disableButton(QList<QPushButton*> buttons)
{
    foreach (auto button, buttons) {
        button->setEnabled(false);
    }
}

inline void VSCMainWindow::enableButton(QPushButton* button)
{
    button->setEnabled(true);
}

inline void VSCMainWindow::disableButton(QPushButton* button)
{
    button->setEnabled(false);
}

inline void VSCMainWindow::enableDefaultButton(QPushButton* button)
{
    button->setAutoDefault(true);
    button->setDefault(true);
}

inline void VSCMainWindow::disableDefaultButton(QPushButton* button)
{
    button->setAutoDefault(false);
    button->setDefault(false);
}

// ------------------------------------------------------------------

PopupMenu::PopupMenu(QWidget* target, QWidget* parent)
    : QMenu(parent)
    , w(target)
{
}

void PopupMenu::showEvent(QShowEvent* event)
{
    // QPoint p = this->pos();
    // QRect geo = w->geometry();
    // this->move(w->x(), w->y());
    QMenu::showEvent(event);
}
