#ifndef TRANSACTIONEDITOR_H
#define TRANSACTIONEDITOR_H

#include <QListWidget>
#include <QWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include "mainwindow.h"
#include "model.h"

class TransactionEditor : public Widgets {
    Q_OBJECT
public:
    explicit TransactionEditor(Wallet& wallet, MainWindow& m_window, QWidget *parent = nullptr);

private slots:
    void FillTargets();
    void FillOps();
signals:
private:
    void FillData();
    void FillAccs();

    const QVector<CategoryInfo> GetCats() const;

    QListWidget* list_ = new QListWidget;

    QComboBox* acc_ = new QComboBox;
    QComboBox* target_ = new QComboBox;
    QComboBox* type_ = new QComboBox;

    QDateEdit* date_from_ = new QDateEdit;
    QDateEdit* date_to_ = new QDateEdit;

    QVector<size_t> accs_idxs_;
    QVector<size_t> cats_idxs_;

    void showEvent(QShowEvent *event) override;
};

#endif // TRANSACTIONEDITOR_H
