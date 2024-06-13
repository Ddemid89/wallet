#ifndef FIRSTWINDOW_H
#define FIRSTWINDOW_H

#include <QVector>
#include <QWidget>
#include <QLabel>
#include "model.h"
#include "mainwindow.h"
#include "account_widget.h"
#include <QVBoxLayout>

class FirstWindow : public QWidget {
    Q_OBJECT
public:
    explicit FirstWindow(Wallet& wallet, MainWindow& m_window, QWidget *parent = nullptr);

signals:

private:
    void FillData();

    void FillAccs();

    QVector<AccountWidget*> acc_widgets_;

    void showEvent(QShowEvent *event) override;
    Wallet& wallet_;
    MainWindow& m_window_;

    quint64 a;

    QVBoxLayout* main_layout_;
};

#endif // FIRSTWINDOW_H
