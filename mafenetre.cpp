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

#include <iostream>

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
        RF_Power_Control(&reader, true, 0);
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



void MaFenetre::on_card_btn_clicked()
{
    uint16_t status = MI_OK;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uidlen = 12;
    status = ISO14443_3_A_PollCard(&reader, atq, sak, uid, &uidlen);
    qDebug() << (status == MI_OK);
    if (status == MI_OK) {
        //uint8_t data[16]; // 16 bytes per block
        uint8_t data[240]; // in a 15 blocks sector, 15 * 16bytes=240
        // read name
        // data is read for sector 0 with key 0
        //status = Mf_Classic_Read_Sector(&reader, true, 0, data, true, 0);
        // encrypted sectors are the 2, 3 and 4
        status = Mf_Classic_Read_Sector(&reader, true, 2, data, true, 2); // use key A for sector n, with key stored in slot n
        qDebug() << (status == MI_OK);
        if (status == MI_OK) {
            // first block contains "Identite", and is not used
            // second block contains "Vincent"
            // third block contains "Thivent"
            QString firstname;
            for (int offset = 0; offset < 16; offset++) {
                firstname.append(QChar(data[16 + offset]));
            }
            //qDebug() << firstname;
            ui->firstname_edit->setText(firstname);

            QString lastname;
            for (int offset = 0; offset < 16; offset++) {
                lastname.append(QChar(data[32 + offset]));
            }
            //qDebug() << lastname;
            ui->lastname_edit->setText(lastname);

        }

        // read counter value
        //status = Mf_Classic_Read_Sector(&reader, true, 3, data, true, 3); // works, but cannot decode the complete value
        // sector is 3
        // block is 14 according to the TDTP pdf
        uint32_t value;
        status = Mf_Classic_Read_Value(&reader, true, 14, &value, true, 3);
        if (status == MI_OK) {
            ui->counter_edit->setText(QString(std::to_string(value).c_str()));
        } else {
            qDebug() << "error while reading counter value";
        }
        // read sectors (using key 1, 2 and 3)
        //status = Mf_Classic_Read_Block(&reader, TRUE, 0, data, true, 0);
        //status = Mf_Classic_Read_Block(&reader, TRUE, 5, data, true, 0);
        //qDebug() << (status == MI_OK);
    }
}

void MaFenetre::on_spend_btn_clicked()
{
    double dvalue = ui->spend_amount->value();
    uint32_t value = (uint32_t) dvalue;

    // write on block 14 using key for sector 3
    uint16_t status;
    status = Mf_Classic_Write_Value(&reader, true, 14, value, false, 3);

    if (status == MI_OK) {
        qDebug() << "wrote correctly";
    } else {
        qDebug() << "could not write";
    }
}
