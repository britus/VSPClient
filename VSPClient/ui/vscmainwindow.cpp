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
#include <QIcon>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMacToolBar>
#endif
#include <QMenu>
#include <QMessageBox>
#include <QMovie>
#include <QSplitter>
#include <QTimer>
#include <QWindow>
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
    , m_firstStart(true)
    , m_demoMode(false)
{
    ui->setupUi(this);

    const QIcon icon1(":/vspclient_1");
    setWindowIcon(icon1);

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
    splitter->setSizes(QList<int>() << 280 << 0);

    m_buttonMap[ui->btn01SPCreate] = ui->pg01SPCreate;
    m_buttonMap[ui->btn02SPRemove] = ui->pg02SPRemove;
    m_buttonMap[ui->btn03LKCreate] = ui->pg03LKCreate;
    m_buttonMap[ui->btn04LKRemove] = ui->pg04LKRemove;
    m_buttonMap[ui->btn05PortList] = ui->pg05PortList;
    m_buttonMap[ui->btn06LinkList] = ui->pg06LinkList;
    m_buttonMap[ui->btn07Checks] = ui->pg07Checks;
    m_buttonMap[ui->btn08Traces] = ui->pg08Traces;
    m_buttonMap[ui->btn09Connect] = ui->pg09Connect;

    const QList<VSPAbstractPage*> pages = m_buttonMap.values();
    foreach (auto page, pages) {
        connect(page, &VSPAbstractPage::execute, this, &VSCMainWindow::onActionExecute);
    }

    /* Driver installation / removal */
    connect(ui->pg09Connect, &PGConnect::installDriver, this, &VSCMainWindow::onActionInstall);
    connect(ui->pg09Connect, &PGConnect::uninstallDriver, this, &VSCMainWindow::onActionUninstall);

    const QList<QPushButton*> buttons = m_buttonMap.keys();
    foreach (auto button, buttons) {
        connect(button, &QPushButton::clicked, this, &VSCMainWindow::onSelectPage);
    }

    connect(ui->btn11SerialIO, &QPushButton::clicked, this, [this]() {
        VSPSerialIO* d = new VSPSerialIO(this);
        d->setVisible(true);
        d->raise();
    });

    connect(qApp, &QGuiApplication::saveStateRequest, this, [](QSessionManager&) {
        // -- saveSettings();
    });

    connect(qApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state != Qt::ApplicationActive) {
            // dummy
            QTimer::singleShot(1000, this, [this]() {
                if (this->hasFocus()) {
                }
            });
        }
    });

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        setupSystemTray();
    }

    QAction* a;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // ---------
    QIcon tbicon(":/vspclient_2");
    QMacToolBar* toolBar = new QMacToolBar(this);
    QMacToolBarItem* item = toolBar->addItem(tbicon, tr("About"));
    connect(item, &QMacToolBarItem::activated, this, [this]() {
        showAboutBox();
    });
    toolBar->attachToWindow(this->windowHandle());
#endif

    // ---------
    if (this->menuBar()) {
        QMenu* appMenu = new QMenu(this);
        a = appMenu->addAction(tr("About..."));
        a->setMenuRole(QAction::ApplicationSpecificRole);
        connect(a, &QAction::triggered, this, [this]() {
            showAboutBox();
        });
        appMenu->addSeparator();
        this->menuBar()->addMenu(appMenu);
    }

    // ---------
    PopupMenu* menu = new PopupMenu(ui->toolButton, this);

    a = new QAction(tr("Activate VSP Driver"));
    connect(a, &QAction::triggered, this, [this]() {
        m_vsp->activateDriver();
    });
    menu->addAction(a);

    a = new QAction(tr("Deactivate VSP Driver"));
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
    qApp->quit();
}

void VSCMainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateOverlayGeometry();
}

void VSCMainWindow::onSetupFailWithError(uint32_t code, const char* message)
{
    qDebug("CTRLWIN::onSetupFailWithError(): code=%d msg=%s\n", code, message);

    ui->textBrowser->setLineWrapMode(QTextBrowser::LineWrapMode::WidgetWidth);
    ui->textBrowser->setPlainText(tr("VSP setup status #%1\n%2").arg(code).arg(message));
    if (code == 12) {
        showNotification(2750, tr("Wait for user approval."));
    }
    else {
        showNotification(2750, ui->textBrowser->toPlainText());
    }
}

void VSCMainWindow::onSetupFinishWithResult(uint32_t code, const char* message)
{
    qDebug("CTRLWIN::onSetupFinishWithResult(): code=%d msg=%s\n", code, message);

    ui->textBrowser->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);
    ui->textBrowser->setPlainText(tr("%1 %2").arg(code == 0 ? "" : QString::number(code), message).trimmed());

    if (code == 12) {
        showNotification(2750, "User approval.");
    }
    else {
        showNotification(2750, ui->textBrowser->toPlainText());
    }
}

void VSCMainWindow::onSetupNeedsUserApproval()
{
    ui->textBrowser->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);
    ui->textBrowser->setPlainText("Wait for approval..");
}

void VSCMainWindow::onClientConnected()
{
    qDebug("CTRLWIN::onClientConnected()\n");

    resetDefaultButton(ui->pnlButtons);
    onUpdateButtons(true);

    QString dn = (m_demoMode ? tr("Demo") : m_vsp->DeviceName());
    QString dp = (m_demoMode ? tr("Mode") : m_vsp->DevicePath());

    ui->textBrowser->setLineWrapMode(QTextBrowser::LineWrapMode::WidgetWidth);
    ui->textBrowser->setPlainText(tr("Connected. [%1: %2]").arg(dn, dp));
    ui->stackedWidget->setCurrentWidget(ui->pg01SPCreate);

    showNotification(2750, ui->textBrowser->toPlainText());
    enableDefaultButton(ui->btn01SPCreate);
    disableButton(ui->btn09Connect);
}

void VSCMainWindow::onClientDisconnected()
{
    qDebug("CTRLWIN::onClientDisconnected()\n");

    resetDefaultButton(ui->pnlButtons);
    onUpdateButtons(false);

    ui->stackedWidget->setCurrentWidget(ui->pg09Connect);

    ui->textBrowser->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);
    ui->textBrowser->setPlainText(tr("Disconnected."));

    showNotification(1750, ui->textBrowser->toPlainText());
    enableButton(ui->btn09Connect);
    enableDefaultButton(ui->btn09Connect);
}

void VSCMainWindow::onClientError(int error, const QString& message)
{
    qDebug("CTRLWIN::onClientError() error=%d msg=%s\n", error, qPrintable(message));

    m_errorStack[error] = message;

    QString text = "";

    QList<uint> codes = m_errorStack.keys();
    foreach (auto code, codes) {
        text += tr("VSP Error: 0x%1 %2\n").arg(code, 8, 16, QChar('0')).arg(m_errorStack[code]);
    }

    ui->textBrowser->setLineWrapMode(QTextBrowser::LineWrapMode::WidgetWidth);
    ui->textBrowser->setPlainText(text);

    if (!m_firstStart) {
        showNotification(1750, text);
        QTimer::singleShot(50, this, [this, error, text]() {
            if (windowIcon().isNull()) {
                m_box.setIcon(QMessageBox::Critical);
            }
            else {
                m_box.setIconPixmap(windowIcon().pixmap(QSize(32, 32)));
            }

            m_box.setWindowTitle(windowTitle());
            m_box.setText(text);

            if (!m_vsp->IsConnected() && error == kIOErrorNotFound) {
                m_box.setInformativeText(tr("You must install the VSP Driver extension first.\n"));
            }
            else {
                m_box.setInformativeText("");
            }

            m_box.show();
        });
    }
}

void VSCMainWindow::onUpdateStatusLog(const QByteArray& message)
{
    qDebug("CTRLWIN::onUpdateStatusLog(): %s\n", qPrintable(message));

    ui->textBrowser->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);
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
           << ui->btn08Traces    //
           << ui->btn11SerialIO);
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
           << ui->btn08Traces    //
           << ui->btn11SerialIO);
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

    // reset error message stack
    m_errorStack.clear();

    showOverlay();

    switch (command) {
        case vspControlGetStatus: {
            if (!m_demoMode && m_vsp->IsConnected()) {
                if (!m_vsp->GetStatus()) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this, command]() {
                    onCommandResult(command, m_vsp->portList(), m_vsp->linkList());
                    onComplete();
                });
            }
            break;
        }
        case vspControlCreatePort: {
            TVSPPortParameters p = data.value<TVSPPortParameters>();
            if (!m_demoMode) {
                if (!m_vsp->CreatePort(&p)) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this]() {
                    m_vsp->createDemoPort();
                    showDemoMessage(tr("Virtual demo port created."));
                });
            }
            break;
        }
        case vspControlRemovePort: {
            VSPDataModel::TPortItem p = data.value<VSPDataModel::TPortItem>();
            if (!m_demoMode) {
                if (!m_vsp->RemovePort(p.id)) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this, p]() {
                    m_vsp->removeDemoPort(p.id);
                    showDemoMessage(tr("Virtual demo port removed."));
                });
            }
            break;
        }
        case vspControlLinkPorts: {
            VSPDataModel::TPortLink link = data.value<VSPDataModel::TPortLink>();
            if (link.source.id == link.target.id) {
                onClientError(
                   0xfa100001,
                   tr("\n\nYou cannot link same ports together.\n"
                      "Any unlinked port is in loopback mode by default."));
                goto error_exit;
            }
            if (!m_demoMode) {
                if (!m_vsp->LinkPorts(link.source.id, link.target.id)) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this, link]() {
                    m_vsp->createDemoLink(link.source.id, link.target.id);
                    showDemoMessage(tr("Demo port link created."));
                });
            }
            break;
        }
        case vspControlUnlinkPorts: {
            VSPDataModel::TPortLink link = data.value<VSPDataModel::TPortLink>();
            if (!m_demoMode) {
                if (!m_vsp->UnlinkPorts(link.source.id, link.target.id)) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this, link]() {
                    m_vsp->removeDemoLink(link.source.id, link.target.id);
                    showDemoMessage(tr("Demo port link removed."));
                });
            }
            break;
        }
        case vspControlGetPortList: {
            if (!m_demoMode) {
                if (!m_vsp->GetPortList()) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this, command]() {
                    onCommandResult(command, m_vsp->portList(), m_vsp->linkList());
                    onComplete();
                });
            }
            break;
        }
        case vspControlGetLinkList: {
            if (!m_demoMode) {
                if (!m_vsp->GetLinkList()) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this, command]() {
                    onCommandResult(command, m_vsp->portList(), m_vsp->linkList());
                    onComplete();
                });
            }
            break;
        }
        case vspControlEnableChecks: {
            quint64 value = data.toUInt();
            quint8 portId = value & 0x00ffL;
            value = (value >> 16) & 0xffffL;
            if (!m_demoMode) {
                if (!m_vsp->EnableChecks(portId, value)) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this, command]() {
                    onCommandResult(command, m_vsp->portList(), m_vsp->linkList());
                    onComplete();
                    showDemoMessage(tr("Demo port checks updated."));
                });
            }
            break;
        }
        case vspControlEnableTrace: {
            quint64 value = data.toUInt();
            quint8 portId = value & 0x00ffL;
            value = (value >> 16) & 0xffffL;
            if (!m_demoMode) {
                if (!m_vsp->EnableTrace(portId, value)) {
                    goto error_exit;
                }
            }
            else {
                QTimer::singleShot(700, this, [this, command]() {
                    onCommandResult(command, m_vsp->portList(), m_vsp->linkList());
                    onComplete();
                    showDemoMessage(tr("Demo port traces updated."));
                });
            }
            break;
        }
        case vspControlPingPong: {
            if (!m_vsp->IsConnected()) {
                quint64 value = data.toUInt();
                if (value == 1) {
                    m_demoMode = true;
                    m_firstStart = false;
                    setWindowTitle(tr("%1 [DEMO MODE]").arg(windowTitle()));
                    onClientConnected();
                    goto error_exit;
                }
                if (!m_vsp->ConnectDriver()) {
                    onUpdateButtons(false);
                    onActionInstall();
                    m_firstStart = false;
                    goto error_exit;
                }
                m_firstStart = false;
                m_demoMode = false;
            }
            if (!m_vsp->GetStatus()) {
                onUpdateButtons(false);
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

void VSCMainWindow::onActionInstall()
{
    if (!m_vsp->IsConnected()) {
        m_vsp->activateDriver();
    }
}

void VSCMainWindow::onActionUninstall()
{
    m_vsp->deactivateDriver();
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

inline void VSCMainWindow::showDemoMessage(const QString& message)
{
    if (m_demoMode) {
        QTimer::singleShot(50, this, [this, message]() {
            if (windowIcon().isNull()) {
                m_box.setIcon(QMessageBox::Information);
            }
            else {
                m_box.setIconPixmap(windowIcon().pixmap(QSize(32, 32)));
            }
            m_box.setWindowTitle(windowTitle());
            m_box.setInformativeText("");
            m_box.setText(message);
            m_box.show();
        });
    }
}

inline void VSCMainWindow::showNotification(int ms, const QString& text)
{
    if (QSystemTrayIcon::supportsMessages()) {
        stIcon.showMessage( //
           qApp->applicationDisplayName(),
           tr(COPYRIGHT) + "\n\n" + text,
           QSystemTrayIcon::NoIcon /* stIcon.icon()*/,
           ms);
    }
}

inline void VSCMainWindow::showAboutBox()
{
    showNotification(5000, tr("Version %1").arg(qApp->applicationVersion()));
    QMessageBox::about(
       this,
       qApp->applicationName(),                //
       qApp->applicationDisplayName() + "\n\n" //
          + tr(COPYRIGHT) + "\n\n"             //
          + tr("Version %1").arg(qApp->applicationVersion()));
}

inline void VSCMainWindow::setupSystemTray()
{
    stIcon.setIcon(QIcon(":/vspclient_4"));
    stIcon.setToolTip(qApp->applicationDisplayName() + " \n\n" + tr(COPYRIGHT));

    connect(&stIcon, &QSystemTrayIcon::messageClicked, this, []() {
        //-
    });
    connect(&stIcon, &QSystemTrayIcon::activated, this, [/*this*/]() {
        // setWindowState(Qt::WindowState::WindowActive);
    });

    QMenu* menu = new QMenu(this);
    QAction* a;

    a = new QAction(stIcon.icon(), tr("About..."));
    connect(a, &QAction::triggered, this, [this]() {
        showAboutBox();
    });
    menu->addAction(a);

    a = new QAction(stIcon.icon(), tr("Open Controller"));
    connect(a, &QAction::triggered, this, [this]() {
        setWindowState(Qt::WindowState::WindowActive);
        show();
        raise();
    });
    menu->addAction(a);
    a = new QAction(stIcon.icon(), tr("Open Serial I/O"));
    connect(a, &QAction::triggered, this, [this]() {
        VSPSerialIO* d = new VSPSerialIO(this);
        d->setVisible(true);
        d->raise();
    });
    menu->addAction(a);
    menu->addSeparator();

    a = new QAction(stIcon.icon(), tr("Close"));
    connect(a, &QAction::triggered, this, [this]() {
        qApp->postEvent(this, new QCloseEvent());
    });
    menu->addAction(a);

    stIcon.setContextMenu(menu);
    stIcon.show();
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
       "border-radius: 7px;");
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
