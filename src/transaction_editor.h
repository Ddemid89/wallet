#ifndef TRANSACTIONEDITOR_H
#define TRANSACTIONEDITOR_H

#include <QListWidget>
#include <QWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QMenu>
#include "mainwindow.h"
#include "model.h"
#include "my_list.h"

class ModalEditor : public QWidget {
    Q_OBJECT
public:
    ModalEditor(const Transaction* trns, Wallet& wallet, QWidget* parent = nullptr);
    void closeEvent(QCloseEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event);
private slots:
    void AccChanged();
    void CatChanged();
    void ButtonPressed();
    void Submit();
    void ShowMenu(const QPoint& point);
signals:
    void Updated();
private:
    bool NoChanges();
    void FillAccs(QComboBox* cb, size_t idx);
    //void FillCats(bool inc, size_t idx);
    void FillCatLab();
    void MakeMenu();
    void FillMenuChilds(QMenu& menu, const Category& info);

    const Transaction* const trns_;
    Wallet& wallet_;

    const TransactionType type_;

    QVector<size_t> acc_idx_;
    QVector<size_t> cat_idx_;

    QDateEdit* date_      = new QDateEdit;
    QComboBox* acc_       = new QComboBox;
    QDoubleSpinBox* sum_  = new QDoubleSpinBox;
    QComboBox* cat_       = new QComboBox;

    ClickableLabel* cat_cont_lab_ = new ClickableLabel;
    size_t  cat_id_;

    QLabel* acc_lab_     = new QLabel;
    QLabel* cat_lab_     = new QLabel;

    QLineEdit* desc_     = new QLineEdit;

    QMenu* menu_ = nullptr;
};


class TransactionEditor : public Widgets {
    Q_OBJECT
public:
    explicit TransactionEditor(Wallet& wallet, MainWindow& m_window, QWidget *parent = nullptr);
private slots:
    void FillTargets();
    void FillOps();
    void Edit();
    void Delete();
signals:
private:
    void FillData();
    void FillAccs();

    const QVector<CategoryInfo> GetCats() const;

    MyList* list_ = new MyList;

    QComboBox* acc_ = new QComboBox;
    QComboBox* target_ = new QComboBox;
    QComboBox* type_ = new QComboBox;

    QDateEdit* date_from_ = new QDateEdit;
    QDateEdit* date_to_ = new QDateEdit;

    QVector<size_t> accs_idxs_;
    QVector<size_t> cats_idxs_;

    QVector<size_t> trns_idxs_;

    void showEvent(QShowEvent *event) override;
};

#endif // TRANSACTIONEDITOR_H
