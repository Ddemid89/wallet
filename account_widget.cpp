#include "account_widget.h"
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QString>

const int FIXED_WIDTH = 320;

AccountWidget::AccountWidget(AccountBase& acc, QWidget *parent)
    : QWidget{parent} {
    frame_ = new QFrame(this);

    frame_->setFrameStyle(2);

    frame_->setLayout(layout_);

    QLabel* acc_name = new QLabel(acc.GetName());

    QFont font;

    font.setBold(true);
    font.setPixelSize(16);

    acc_name->setFont(font);

    frame_->setFixedWidth(FIXED_WIDTH);

    sum_wid_->setAlignment(Qt::AlignRight);

    layout_->addRow(acc_name);
    layout_->addRow(sum_lab_, sum_wid_);
    sum_lab_->setFixedWidth(200);
    acc.Visit(*this);
}

void AccountWidget::SetAccount(Debet& acc) {
    sum_lab_->setText("Остаток на счете: ");
    sum_wid_->setText(acc.GetSum().String() + " руб.");
    frame_->setFixedSize(FIXED_WIDTH, 60);
    setFixedSize(FIXED_WIDTH, 60);
}

void AccountWidget::SetAccount(Credit& acc) {
    sum_lab_->setText("Остаток задолженности: ");
    sum_wid_->setText(acc.GetSum().String() + " руб.");
    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);
    sum_wid_->setPalette(palette);
    frame_->setFixedSize(FIXED_WIDTH, 60);
    setFixedSize(FIXED_WIDTH, 60);
}

void AccountWidget::SetAccount(Deposit& acc) {
    sum_lab_->setText("Остаток на счете: ");
    sum_wid_->setText(acc.GetSum().String() + " руб.");
    frame_->setFixedSize(FIXED_WIDTH, 60);
    setFixedSize(FIXED_WIDTH, 60);
}

void AccountWidget::SetAccount(OverdraftCard& acc) {
    sum_lab_->setText("Остаток на счете: ");
    sum_wid_->setText(acc.GetSum().String() + " руб.");

    QLabel* own = new QLabel(acc.OwnSum().String() + " руб.");
    own->setAlignment(Qt::AlignRight);

    layout_->addRow("Собственные средства: ", own);
    Money debt = acc.Debt();
    int height = 90;

    if (debt.Kopek() != 0) {
        QPalette palette;
        palette.setColor(QPalette::WindowText, Qt::red);
        QLabel* l = new QLabel("Задолженность: ");
        QLabel* w = new QLabel(debt.String() + " руб.");

        l->setPalette(palette);
        w->setPalette(palette);

        w->setAlignment(Qt::AlignRight);

        layout_->addRow(l, w);
        height = 110;
    }

    frame_->setFixedSize(FIXED_WIDTH, height);
    setFixedSize(FIXED_WIDTH, height);
}

AccountWidget::~AccountWidget() {

}
