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


// read key is key A
// write key is key B
const bool key_a = true;
const bool key_b = false;


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
bool connected = false;


void MaFenetre::on_connect_btn_clicked() {
    if (connected) {
        // cannot connect when already connected
        // disconnect first to refresh the connection
        disconnect();
    }

    int16_t status = MI_OK;
    reader.Type = ReaderCDC;
    reader.device = 0;

    status = OpenCOM(&reader);
    if (status == 0) {
        connected = true;
        qDebug() << "Connected";
        status = Version(&reader);
        if (status == MI_OK) {
            ui->label_device->setText(reader.version);
        }
        status = RF_Power_Control(&reader, true, 0);
        status = LEDBuzzer(&reader, LED_GREEN_ON);

    }
}


void MaFenetre::disconnect() {
    // should disconnect even if the reader is physically disconnected
    // -> ignore status codes
    RF_Power_Control(&reader, false, 0);
    CloseCOM(&reader);
    connected = false;
}

void MaFenetre::on_disconnect_btn_clicked()
{
    disconnect();
    qDebug() << "disconnected";
}

// override window closing behaviour
void MaFenetre::closeEvent(QCloseEvent *close) {
    disconnect();
    close->accept();
}



void MaFenetre::on_card_btn_clicked()
{
    uint16_t status = MI_OK;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uidlen = 12;
    status = ISO14443_3_A_PollCard(&reader, atq, sak, uid, &uidlen);

    // sectors 2 and 3 are encrypted
    // sector 2 requires key 2, sector 3 requires key 3
    // other sectors can be read with key 0

    // Mf_Classic_Read_Block/Value(reader=&reader, auth=true, block, out=data, readkey=key_a|key_b, keynum)
    // out is uint8_t[16] for blocks

    if (status == MI_OK) {
        LEDBuzzer(&reader, LED_GREEN_ON + LED_YELLOW_ON + BUZZER_ON);

        // read first name
        uint8_t data[16];
        status = Mf_Classic_Read_Block(&reader, true, 9, data, key_a, 2); // block 9 in sector 2
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

        // read last name
        status = Mf_Classic_Read_Block(&reader, true, 10, data, key_a, 2);
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
        status = Mf_Classic_Read_Value(&reader, true, 14, &value, key_a, 3); // block 14, sector 3
        if (status == MI_OK) {
            ui->counter_edit->setText(QString::number(value));
        } else {
            qDebug() << "error: could not read counter value";
        }

        LEDBuzzer(&reader, LED_GREEN_ON + BUZZER_OFF);
    } else {
        qDebug() << "error: could not read card";
    }
}



void MaFenetre::on_spend_btn_clicked()
{
    // value is in block 14 (sector 3)
    uint32_t value = ui->spend_amount->value();

    uint16_t status;
    status = Mf_Classic_Decrement_Value(&reader, true, 14, value, 14, key_b, 3);
    if (status == MI_OK) {
        qDebug() << "decremented correctly";

        // read the card to update the UI
        on_card_btn_clicked();
    } else {
        qDebug() << "error: could not decrement";
    }
}

void MaFenetre::on_raise_btn_clicked()
{
    uint32_t value = ui->raise_amount->value();

    uint16_t status;
    status = Mf_Classic_Increment_Value(&reader, true, 14, value, 14, key_b, 3);
    if (status == MI_OK) {
        qDebug() << "incremented correctly";

        // read the card to refresh the UI
        on_card_btn_clicked();
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
    int i = 0;
    while (i < firstname.size()) {
        data[i] = firstname.at(i).toLatin1();
        i++;
    }
    while (i < 16) {
        data[i] = 0;
        i++;
    }
    status = Mf_Classic_Write_Block(&reader, true, 9, data, key_b, 2);
    if(status != MI_OK) {
        qDebug() << "error: could not write first name";
    }


    // last name
    QString lastname = ui->lastname_edit->text();
    i = 0;
    while (i < lastname.size()) {
        data[i] = lastname.at(i).toLatin1();
        i++;
    }
    while (i < 16) {
        data[i] = 0;
        i++;
    }
    status = Mf_Classic_Write_Block(&reader, true, 10, data, key_b, 2);
    if(status != MI_OK) {
        qDebug() << "error: could not write last name";
    }
}

