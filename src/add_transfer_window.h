#ifndef ADDTRANSFERWINDOW_H
#define ADDTRANSFERWINDOW_H

#include <QListWidget>
#include <QWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include "model.h"
#include "mainwindow.h"

class AddTransferWindow : public Widgets {
    Q_OBJECT
public:
    explicit AddTransferWindow(Wallet& wallet, MainWindow& m_window, QWidget *parent = nullptr);

signals:

private slots:
    void submit();
private:
    void FillData();

    void FillAccs();

    void FillLastOps();

    void showEvent(QShowEvent *event) override;

    QDateEdit* date_ = new QDateEdit;

    QComboBox* from_ = new QComboBox;
    QComboBox* to_ = new QComboBox;

    QVector<size_t> accs_idxs_;

    QDoubleSpinBox* sum_ = new QDoubleSpinBox;

    QListWidget* last_ops_ = new QListWidget;
};

#endif // ADDTRANSFERWINDOW_H
