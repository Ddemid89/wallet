#ifndef ACCOUNT_EDITOR_H
#define ACCOUNT_EDITOR_H

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "model.h"

class AccountEditor : public QWidget, public AccVisitorInterface {
    Q_OBJECT
public:
    AccountEditor(Wallet& wallet, size_t acc_id, QWidget* parent = 0) : QWidget{parent}, wallet_{wallet}, id_{acc_id} {
        QVBoxLayout* main_layout = new QVBoxLayout;

        sum_lab_->setText("Баланс:");

        layout_->addRow("Название:", name_);
        layout_->addRow(sum_lab_, sum_);
        layout_->addWidget(consider_);
        AccountBase* acc = wallet_.GetOneAccount(id_);

        name_->setText(acc->GetName());

        sum_->setMinimum(-1000000);
        sum_->setMaximum(1000000);
        sum_->setSuffix(" руб.");
        sum_->setValue(acc->GetSum().Double());

        consider_->setChecked(acc->IsConsider());

        setWindowModality(Qt::ApplicationModal);
        setWindowOpacity(0.9);
        setWindowFlag(Qt::Dialog);
        setFixedSize(330, 160);

        main_layout->addLayout(layout_);

        QPushButton* done = new QPushButton("Готово");

        main_layout->addWidget(done, 0, Qt::AlignCenter);
        done->setFixedWidth(120);

        connect(done, &QPushButton::clicked, this, &AccountEditor::OnDone);

        setLayout(main_layout);
    }

    void SetAccount(Debet&) override;
    void SetAccount(Credit&) override;
    void SetAccount(Deposit&) override;
    void SetAccount(OverdraftCard&) override;

    void closeEvent(QCloseEvent*) override;

signals:
    void Changed();

private slots:
    void DepClicked();
    void OvrClicked();
    void OnDone();
private:

    class AccountChanger : public AccVisitorInterface{
    public:
        AccountChanger(AccountEditor& editor);
        void SetAccount(Debet&) override;
        void SetAccount(Credit&) override;
        void SetAccount(Deposit&) override;
        void SetAccount(OverdraftCard&) override;
    private:
        void FillCommon(AccountBase& acc);
        void FillPercDay(PercentBase& acc);
        AccountEditor& editor_;
    };

    void DrawPercentDay();
    void DrawPayment();
    void DrawDepOvr();

    void ChangeData();
    void ChangeTypeAndData();
    void SaveAccount(QCloseEvent* account = nullptr);

    AccountType GetNewType();

    bool NoChanged();
    bool Correct();

    AccountType type_;

    QFormLayout* layout_ = new QFormLayout;
    QLineEdit* name_     = new QLineEdit;
    QLabel* sum_lab_     = new QLabel;
    QDoubleSpinBox* sum_ = new QDoubleSpinBox;
    QCheckBox* consider_ = new QCheckBox("Учитывать в балансе");

    QDoubleSpinBox* perc_ = nullptr;
    QSpinBox*       day_  = nullptr;
    QLabel* payment_lab_  = nullptr;
    QDoubleSpinBox* payment_ = nullptr;

    QCheckBox* dep_;
    QCheckBox* ovr_;

    Wallet& wallet_;
    size_t id_;
};



#endif // ACCOUNT_EDITOR_H
