#ifndef ADDACCOUNTWINDOW_H
#define ADDACCOUNTWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QLabel>
#include <QListWidget>

#include "model.h"
#include "mainwindow.h"

class AddAccountWindow : public Widgets {
    Q_OBJECT
public:
    explicit AddAccountWindow(Wallet& wallet, MainWindow& m_window, QWidget *parent = nullptr);

signals:

public slots:
    void select();
    void submit();
    void DeleteRestore();
    void Edit();
    void SelectAccount();
    void FillAccs();
private:
    void FillData();



    void showEvent(QShowEvent *event) override;

    QListWidget* accs_ = new QListWidget;

    QDoubleSpinBox* payment_ = new QDoubleSpinBox;
    QDoubleSpinBox* overdraft_ = new QDoubleSpinBox;
    QDoubleSpinBox* percent_rate_ = new QDoubleSpinBox;
    QSpinBox* date_ = new QSpinBox;

    QCheckBox* consider_ = new QCheckBox("Учитывать в общем балансе: ");
    QLineEdit* name_ = new QLineEdit;
    QComboBox* type_ = new QComboBox;
    QDoubleSpinBox* start_sum_ = new QDoubleSpinBox;

    QPushButton* del_  = new QPushButton("Удалить");

    QVector<size_t> accs_index_;
};

#endif // ADDACCOUNTWINDOW_H
