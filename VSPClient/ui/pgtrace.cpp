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
    uint flags = 0;
    if (ui->cbxTraceRX->isChecked())
        flags |= 0x1;
    if (ui->cbxTraceTX->isChecked())
        flags |= 0x2;
    if (ui->cbxTraceCmd->isChecked())
        flags |= 0x4;

    emit VSPAbstractPage::execute(vspControlEnableChecks, QVariant::fromValue(flags));
}

void PGTrace::update(TVSPControlCommand, VSPPortListModel*, VSPLinkListModel*)
{
    //
}
