#ifndef ADDINCDEC_H
#define ADDINCDEC_H

#include "mainwindow.h"
#include "model.h"

#include <QWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QListWidget>

class AddIncDec : public Widgets {
    Q_OBJECT
public:
    explicit AddIncDec(Wallet& wallet, MainWindow& m_window, bool arrive, QWidget* parent = nullptr);
    void SetArrive(bool arrive);
private slots:
    void commit();
signals:

private:
    void FillData();

    void showEvent(QShowEvent *event) override;

    QListWidget* last_ops_ = new QListWidget;

    void DrawInterface();

    QComboBox* account_w = new QComboBox;
    void FillAccounts();

    QComboBox* target_w = new QComboBox;
    void FillTarget();

    QDateEdit* date_w = new QDateEdit;

    QDoubleSpinBox* sum_w = new QDoubleSpinBox;

    void FillLastOps();

    QVector<size_t> accs_idxs_;
    QVector<size_t> cats_idxs_;

    bool arrive_;
};

#endif // ADDINCDEC_H
