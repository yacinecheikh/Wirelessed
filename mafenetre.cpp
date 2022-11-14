#include "MfErrNo.h"
#include "Core.h"
#include "Sw_Device.h"
#include "Sw_Mf_Classic.h"
#include "Sw_Poll.h"
#include "Sw_ISO14443A-3.h"
#include "TypeDefs.h"
#include "Tools.h"

#include "mafenetre.h"
#include "ui_mafenetre.h"

#include <QtGui>

MaFenetre::MaFenetre(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MaFenetre)
{
    ui->setupUi(this);
}

MaFenetre::~MaFenetre()
{
    delete ui;
}


ReaderName reader;

void MaFenetre::on_connect_btn_clicked() {
    int16_t status = MI_OK;
    reader.Type = ReaderCDC;
    reader.device = 0;

    status = OpenCOM(&reader);
    if (status == 0) {
        qDebug() << "Connected";
        status = Version(&reader);
        ui->device_label->setText(reader.version);
        ui->device_label->update();
    }
    qDebug() << "OpenCOM " << status;
}

void MaFenetre::on_input_btn_clicked()
{
    QString text = ui->input_text->toPlainText();
    qDebug() << "Text: " << text;
}

void MaFenetre::on_quit_btn_clicked()
{
    int16_t status = MI_OK;
    RF_Power_Control(&reader, FALSE, 0);
    status = LEDBuzzer(&reader, LED_OFF);
    status = CloseCOM(&reader);
    qApp->quit();
}
