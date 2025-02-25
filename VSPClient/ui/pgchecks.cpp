// ********************************************************************
// pgchecks.cpp - Enable/Disable port parameter checks
//
// Copyright © 2025 by EoF Software Labs
// Copyright © 2024 Apple Inc. (some copied parts)
// SPDX-License-Identifier: MIT
// ********************************************************************
#include "ui_pgchecks.h"
#include <pgchecks.h>
#include <vspabstractpage.h>

PGChecks::PGChecks(QWidget* parent)
    : VSPAbstractPage(parent)
    , ui(new Ui::PGChecks)
{
    ui->setupUi(this);

    connectButton(ui->btnUpdate);
}

PGChecks::~PGChecks()
{
    delete ui;
}

void PGChecks::onActionExecute()
{
    uint flags = 0;
    if (ui->cbxBaudRate->isChecked())
        flags |= 0x01;
    if (ui->cbxDataBits->isChecked())
        flags |= 0x02;
    if (ui->cbxStopBits->isChecked())
        flags |= 0x04;
    if (ui->cbxParity->isChecked())
        flags |= 0x08;
    if (ui->cbxFlowCtrl->isChecked())
        flags |= 0x10;

    emit VSPAbstractPage::execute(vspControlEnableChecks, QVariant::fromValue(flags));
}

void PGChecks::update(TVSPControlCommand, VSPPortListModel*, VSPLinkListModel*)
{
    //
}
