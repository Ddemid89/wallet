#include "add_category_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>

AddCategoryWindow::AddCategoryWindow(Wallet& wallet, MainWindow& m_window, QWidget* parent)
                                                        : Widgets(wallet, m_window, parent) {
    QLabel* lab = new QLabel("Все категории:");
    QLabel* lab2 = new QLabel("Новая категория:");
    QHBoxLayout* input_layout = new QHBoxLayout;
    QPushButton* submit = new QPushButton("Добавить");

    to_inc_->setText("Для доходов");
    to_dec_->setText("Для расходов");

    to_inc_->setChecked(true);
    to_dec_->setChecked(true);

    lab2->setFrameStyle(2);

    input_layout->addWidget(new QLabel("Наименование категории: "));
    input_layout->addWidget(name_);

    edit_->setFixedWidth(120);

    layout_->addWidget(back_);
    layout_->addWidget(lab);
    layout_->addWidget(list_view);
    layout_->addWidget(edit_, 0, Qt::AlignCenter);
    layout_->addWidget(lab2);
    layout_->addLayout(input_layout);
    layout_->addWidget(to_inc_);
    layout_->addWidget(to_dec_);
    layout_->addWidget(submit);

    connect(submit, SIGNAL(clicked()), this, SLOT(submit()));
    connect(edit_, &QPushButton::clicked, this, &AddCategoryWindow::Edit);
    connect(list_view, &QListWidget::doubleClicked, this, &AddCategoryWindow::Edit);

    connect(to_inc_, &QCheckBox::stateChanged, [this](){
        if (to_inc_->isChecked() == false) {
            to_dec_->setChecked(true);
        }
    });

    QFont fnt("Monospace");
    list_view->setFont(fnt);

    connect(to_dec_, &QCheckBox::stateChanged, [this](){
        if (to_dec_->isChecked() == false) {
            to_inc_->setChecked(true);
        }
    });

    connect(name_, SIGNAL(returnPressed()), this, SLOT(submit()));

    FillData();

    setLayout(layout_);
}

void AddCategoryWindow::submit() {
    QString name = name_->text();
    name = name.trimmed();

    if (name == "") {
        name_->setText("Введите название категории");
        return;
    }

    bool inc = to_inc_->isChecked();
    bool dec = to_dec_->isChecked();

    size_t parrent_idx = cats_->at(list_view->currentRow()).idx;

    wallet_.AddCategory(name, parrent_idx, inc, dec);

    emit(m_window_.show_status("Категория \"" + name + "\" добавлена!"));

    FillData(parrent_idx);
}

void AddCategoryWindow::Edit() {
    int cur_row = list_view->currentRow();
    if (cur_row == -1) {
        return;
    }

    auto cat = cats_->at(cur_row);

    qDebug() << cat.name << "( id:" << cat.idx << ")";

}

void AddCategoryWindow::FillData(int n) {
    name_->clear();
    cats_.reset();

    FillCategories(n);
}

void AddCategoryWindow::showEvent(QShowEvent*) {
    FillData();
}

void AddCategoryWindow::FillCategories(int n) {
    if (!cats_.has_value()) {
        cats_ = wallet_.GetAllCategories();
    }

    list_view->clear();

    for (CategoryInfo& category : *cats_) {
        QString item;
        if (category.indent != 0) {
            item = "    ";
        }
        for (int i = 0; i < category.indent - 1; ++i) {
            if (i == category.indent - 2) {
                item += "|--";
            } else {
                item += "|  ";
            }
        }
        if (category.indent != 0) {
            item += "+---";
        }

        item += category.name;

        list_view->addItem(item);
    }
    list_view->setCurrentRow(n);
}
