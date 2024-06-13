#ifndef ACCOUNTWIDGET_H
#define ACCOUNTWIDGET_H

#include <QFormLayout>
#include <QLabel>
#include <QWidget>
#include "model.h"

class AccountWidget : public QWidget, AccVisitorInterface {
    Q_OBJECT
public:
    explicit AccountWidget(AccountBase& acc, QWidget *parent = nullptr);

    void SetAccount(Debet& acc) override;
    void SetAccount(Credit& acc) override;
    void SetAccount(Deposit& acc) override;
    void SetAccount(OverdraftCard& acc) override;

    ~AccountWidget() override;
signals:
private:
    QFrame* frame_;
    QFormLayout* layout_ = new QFormLayout;
    QLabel* sum_lab_ = new QLabel;
    QLabel* sum_wid_ = new QLabel;
};

#endif // ACCOUNTWIDGET_H
