#ifndef MAFENETRE_H
#define MAFENETRE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MaFenetre; }
QT_END_NAMESPACE

class MaFenetre : public QWidget
{
    Q_OBJECT

public:
    MaFenetre(QWidget *parent = nullptr);
    ~MaFenetre();

private slots:
    void on_connect_btn_clicked();

    //void on_input_btn_clicked();

    void on_quit_btn_clicked();

    void on_card_btn_clicked();

    void on_spend_btn_clicked();

    void on_update_identity_btn_clicked();

    void on_raise_btn_clicked();

private:
    Ui::MaFenetre *ui;
};
#endif // MAFENETRE_H
