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

/*
void MaFenetre::on_input_btn_clicked()
{
    QString text = ui->input_text->toPlainText();
    qDebug() << "Text: " << text;
}
*/

void MaFenetre::on_quit_btn_clicked()
{
    // move to "disconnect" button
    // (still not in the UI, see TDTP pdf)
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

    // conseil du prof: lire des blocs plutôt que des secteurs pour l’identité (facilite son écriture après en n’accédant qu’aux parties utiles)

    // see TDTP pdf page 9 for block details
    // sectors 2 and 3 are encrypted (check if 4 is encrypted)
    // sector 2 requires key 2, sector 3 requires key 3
    // other sectors can be read with key 0 (or auth=false ?)

    // Mf_Classic_Read_Block/Sector/Value(reader=&reader, auth=true, sector/block, out=data, readkey=true, keynum)
    // out is uint8_t[16] for blocks and [240] for sectors (15*16=240, 15 blocks per sector)
    // readkey is key A if true (key B is used for writing)
    // keynum is 2 for sector 2,...

    if (status == MI_OK) {

        uint8_t data[16];
        status = Mf_Classic_Read_Block(&reader, true, 9, data, true, 2); // block 9 in sector 2
        if (status == MI_OK) {
            QString firstname;
            for (int i = 0; i < 16; i++) {
                if (data[i] == 0) {
                    break;
                }
                firstname.append(QChar(data[i]));
            }
            ui->firstname_edit->setText(firstname);
        } else {
            qDebug() << "error: could not read first name";
        }

        status = Mf_Classic_Read_Block(&reader, true, 10, data, true, 2);

        if (status == MI_OK) {
            QString lastname;
            for (int i = 0; i < 16; i++) {
                if (data[i] == 0) {
                    break;
                }
                lastname.append(QChar(data[i]));
            }
            ui->lastname_edit->setText(lastname);
        } else {
            qDebug() << "error: could not read last name";
        }

        // read counter value
        uint32_t value;
        status = Mf_Classic_Read_Value(&reader, true, 14, &value, true, 3); // block 14, sector 3
        if (status == MI_OK) {
            ui->counter_edit->setText(QString::number(value));
        } else {
            qDebug() << "error: could not read counter value";
        }
    } else {
        qDebug() << "error: could not read card";
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

void MaFenetre::on_update_identity_btn_clicked()
{
    uint16_t status;

    // write on sector 2 using key 2
    uint8_t data[16];

    // debug sector content
    // first name
    QString firstname = ui->firstname_edit->text();
    qDebug() << firstname.size();
    int i = 0;
    while (i < firstname.size()) {
        data[i] = firstname.at(i).toLatin1();
        i++;
    }
    while (i < 16) {
        data[i] = 0;
        i++;
    }
    status = Mf_Classic_Write_Block(&reader, true, 9, data, false, 2);
    if(status != MI_OK) {
        qDebug() << "error: could not write first name";
    }
    /*
    // last name
    status = Mf_Classic_Write_Block(&reader, true, 10, data, false, 2);
    if (status != MI_OK) {
        qDebug() << "error: could not write last name";
    }
    */
    //status = Mf_Classic_Write_Sector()
}
