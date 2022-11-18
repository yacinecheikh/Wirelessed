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
            // 3 blocks
            printf("reading block 1 in sector 1");
            int block = 0;
            for (int offset = 0; offset < 16; offset++) {
                printf("%c", data[block * 16 + offset]);
            }
        }

        // read points
        status = Mf_Classic_Read_Sector(&reader, true, 2, data, true, 2);
        qDebug() << (status == MI_OK);
        if (status == MI_OK) {
            // 3 blocks to read
        }
        // read sectors (using key 1, 2 and 3)
        //status = Mf_Classic_Read_Block(&reader, TRUE, 0, data, true, 0);
        //status = Mf_Classic_Read_Block(&reader, TRUE, 5, data, true, 0);
        //qDebug() << (status == MI_OK);
        if (status == MI_OK) {
            qDebug() << data;
        }
    }
}
