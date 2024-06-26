#include "first_window.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include "model.h"
#include <QSpacerItem>
#include <QMessageBox>
#include <QShowEvent>

namespace detail {
QLabel* GetLabel(const QString& txt, double r = 0, bool bold = false) {
    QLabel* res = new QLabel(txt);
    QFont font;
    font.setPixelSize(20);
    font.setBold(bold);
    res->setFont(font);

    QColor color;

    if (r > 0) {
        color = Qt::green;
    } else if (r < 0) {
        color = Qt::red;
    } else {
        color = Qt::black;
    }

    QPalette pal = res->palette();
    pal.setColor(QPalette::WindowText, color);
    res->setPalette(pal);

    return res;
}
}

FirstWindow::FirstWindow(Wallet& wallet, MainWindow& m_window, QWidget* parent)
                                        : QWidget{parent}
                                        , wallet_(wallet)
                                        , m_window_(m_window){

    main_layout_ = new QVBoxLayout;
    setLayout(main_layout_);
    main_layout_->setSpacing(5);
    main_layout_->setAlignment(Qt::AlignTop);
}

void FirstWindow::FillData() {
    FillAccs();
}

void FirstWindow::FillAccs() {
    for (auto acc_ptr : acc_widgets_) {
        delete acc_ptr;
    }
    acc_widgets_.clear();
    for (auto& acc : wallet_.GetAccounts()) {
        if (!acc->IsDeleted()) {
            acc_widgets_.emplaceBack(new AccountWidget(*acc));
            main_layout_->addWidget(acc_widgets_.back());
        }
    }
}

void FirstWindow::showEvent(QShowEvent*) {
    FillData();
}
