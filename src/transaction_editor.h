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
#include "mainwindow.h"
#include "model.h"

class ModalEditor : public QWidget {
    Q_OBJECT
public:
    ModalEditor(const Transaction_DEL* trns, Wallet& wallet, QWidget* parent = nullptr) : QWidget{parent}, trns_{trns}, wallet_{wallet}, type_{trns->Type()} {
        QFormLayout* layout_ = new QFormLayout;

        layout_->addRow("Дата:",  date_);
        layout_->addRow("Сумма:", sum_);
        layout_->addRow(acc_lab_,  acc_);
        layout_->addRow(cat_lab_,  cat_);

        sum_->setMinimum(0.01);
        sum_->setMaximum(1000000);
        sum_->setSuffix(" руб.");

        sum_->setValue(trns->Sum().Double());

        date_->setDate(trns->Date());
        date_->setMaximumDate(QDate::currentDate());

        if (type_ == TransactionType::Transfer) {
            acc_lab_->setText("Откуда:");
            cat_lab_->setText("Куда:");
            setWindowTitle("Редактировать перевод");
            FillAccs(acc_, trns_->AccountFromIdx());
            FillAccs(cat_, trns_->ToIdx());
            if (acc_->count() < 2 || cat_->count() < 2) {
                acc_->setEnabled(false);
                cat_lab_->setEnabled(false);
            }
            connect(acc_, &QComboBox::currentIndexChanged, this, &ModalEditor::AccChanged);
            connect(cat_, &QComboBox::currentIndexChanged, this, &ModalEditor::CatChanged);
        } else {
            acc_lab_->setText("Счет:");
            cat_lab_->setText("Категория:");
            FillAccs(acc_, trns_->AccountFromIdx());
            FillCats(type_ == TransactionType::Income, trns_->ToIdx());
            setWindowTitle(type_ == TransactionType::Income ? "Редактировать доход" : "Редактировать расход");
        }

        QPushButton* done_ = new QPushButton("Готово");

        layout_->addRow("", done_);

        connect(done_, SIGNAL(clicked()), SLOT(ButtonPressed()));

        setLayout(layout_);
        setWindowModality(Qt::ApplicationModal);
        setWindowOpacity(0.9);
        setWindowFlag(Qt::Dialog);
        setFixedSize(300, 150);

    }

    void closeEvent(QCloseEvent* event) override {
        if (!NoChanges()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Выберите действие", "Сохранить изменения?",
                                                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            if (reply == QMessageBox::Yes) {
                Submit();
                emit Updated();
            } else if (reply == QMessageBox::Cancel) {
                event->ignore();
                return;
            }

        }

        delete this;
    }

private slots:
    void AccChanged() {
        if (cat_->currentIndex() == acc_->currentIndex()) {
            if (acc_->currentIndex() == 0) {
                cat_->setCurrentIndex(1);
            } else {
                cat_->setCurrentIndex(0);
            }
        }
    }

    void CatChanged() {
        if (cat_->currentIndex() == acc_->currentIndex()) {
            if (cat_->currentIndex() == 0) {
                acc_->setCurrentIndex(1);
            } else {
                acc_->setCurrentIndex(0);
            }
        }
    }

    void ButtonPressed() {
        if (NoChanges()) {
            close();
            return;
        }

        Submit();
        emit Updated();
        delete this;
    }

    void Submit() {
        transactions_manager::TransactionAdder adder;
        adder.from_idx = acc_idx_.at(acc_->currentIndex());
        adder.to_idx   = type_ == TransactionType::Transfer ? acc_idx_.at(cat_->currentIndex()) : cat_idx_.at(cat_->currentIndex());
        adder.date     = date_->date();
        adder.sum      = sum_->value();
        adder.type     = type_;

        wallet_.EditTransact(trns_->Index(), adder);
    }
signals:
    void Updated();
private:
    bool NoChanges() {
        size_t new_from = acc_idx_.at(acc_->currentIndex());
        QDate  new_date = date_->date();
        Money new_sum   = sum_->value();
        size_t new_to   = cat_idx_.at(cat_->currentIndex());

        size_t old_from = trns_->AccountFromIdx();
        QDate  old_date = trns_->Date();
        Money old_sum   = trns_->Sum();
        size_t old_to   = trns_->ToIdx();

        return new_from == old_from && new_date == old_date && new_sum == old_sum && new_to == old_to;
    }

    void FillAccs(QComboBox* cb, size_t idx) {
        auto& accs = wallet_.GetAccounts();

        cb->clear();

        acc_idx_.clear();

        for (auto& acc : accs) {
            if (acc->IsDeleted()) {
                cb->addItem(acc->GetName() + " (Удален)");
            } else {
                cb->addItem(acc->GetName());
            }

            acc_idx_.push_back(acc->GetIndex());

            if (acc->GetIndex() == idx) {
                cb->setCurrentIndex(cb->count() - 1);
            }
        }
    }

    void FillCats(bool inc, size_t idx) {
        auto cats = wallet_.GetCategories(inc);

        cat_->clear();

        for (auto& cat : cats) {
            cat_->addItem(QString(cat.indent, ' ') + cat.name);
            cat_idx_.push_back(cat.idx);
            if (cat.idx == idx) {
                cat_->setCurrentIndex(cat_->count() - 1);
            }
        }
    }

    const Transaction_DEL* const trns_;
    Wallet& wallet_;

    const TransactionType type_;

    QVector<size_t> acc_idx_;
    QVector<size_t> cat_idx_;

    QDateEdit* date_     = new QDateEdit;
    QComboBox* acc_      = new QComboBox;
    QDoubleSpinBox* sum_ = new QDoubleSpinBox;
    QComboBox* cat_      = new QComboBox;

    QLabel* acc_lab_      = new QLabel;
    QLabel* cat_lab_      = new QLabel;
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

    QListWidget* list_ = new QListWidget;

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
