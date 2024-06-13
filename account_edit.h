#ifndef ACCOUNT_EDIT_H
#define ACCOUNT_EDIT_H

#include <QWidget>
#include "model.h"
#include "mainwindow.h"

class AccountEdit : public Widgets {
    Q_OBJECT
public:
    explicit AccountEdit(Wallet& wallet, MainWindow& m_window, QWidget *parent = nullptr);

signals:
private:

};

#endif // ACCOUNT_EDIT_H
