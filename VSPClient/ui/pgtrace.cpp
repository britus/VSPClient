// ********************************************************************
// pgtrace.cpp - Enable/Disable OS log traces
//
// Copyright © 2025 by EoF Software Labs
// Copyright © 2024 Apple Inc. (some copied parts)
// SPDX-License-Identifier: MIT
// ********************************************************************
#include "ui_pgtrace.h"
#include <pgtrace.h>
#include <vspabstractpage.h>

PGTrace::PGTrace(QWidget* parent)
    : VSPAbstractPage(parent)
    , ui(new Ui::PGTrace)
{
    ui->setupUi(this);

    connectButton(ui->btnUpdate);
}

PGTrace::~PGTrace()
{
    delete ui;
}

void PGTrace::onActionExecute()
{
    // const VSPDataModel::TPortItem p = seletion();
    // emit VSPAbstractPage::execute(vspControlEnableChecks, QVariant::fromValue(p));
}

void PGTrace::update(TVSPControlCommand command, VSPPortListModel* portModel, VSPLinkListModel* linkModel)
{
#if 0
    ui->cbPorts->clear();
    for (int i = 0; i < portModel->rowCount(); i++) {
        VSPDataModel::TDataRecord r = portModel->at(i).value<VSPDataModel::TDataRecord>();
        ui->cbPorts->addItem(r.port.name, QVariant::fromValue(r.port));
    }
#else
    Q_UNUSED(command);
    Q_UNUSED(portModel);
    Q_UNUSED(linkModel);
#endif
}
