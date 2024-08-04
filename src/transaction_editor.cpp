#include "transaction_editor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>

#ifdef Q_OS_WINDOWS
    const int WIDTH = 108;
    const int LAB_WIDTH = 250;
    const int END_WIDTH = 259;
#else
    const int WIDTH = 97;
    const int LAB_WIDTH = 278;
    const int END_WIDTH = 200;
#endif

TransactionEditor::TransactionEditor(Wallet& wallet, MainWindow& m_window, QWidget* parent)
                                            : Widgets(wallet, m_window, parent) {
    QHBoxLayout* date_select_ = new QHBoxLayout;
    QHBoxLayout* type_select_ = new QHBoxLayout;
    QHBoxLayout* btns_layout_ = new QHBoxLayout;

    QPushButton* show_data = new QPushButton("Показать");

    type_->addItem("Всё");
    type_->addItem("Доходы");
    type_->addItem("Расходы");
    type_->addItem("Переводы");

    date_to_->setDate(QDate::currentDate());
    date_from_->setDate(QDate::currentDate().addDays(-7));

    date_to_->setDisplayFormat("dd MMMM yyyy");
    date_from_->setDisplayFormat("dd MMMM yyyy");

    date_to_->setMaximumDate(QDate::currentDate());
    date_from_->setMaximumDate(QDate::currentDate());

    date_select_->addWidget(new QLabel("Промежуток с: "));
    date_select_->addWidget(date_from_);
    date_select_->addWidget(new QLabel("по: "));
    date_select_->addWidget(date_to_);

    type_select_->addWidget(new QLabel("Операция: "));
    type_select_->addWidget(type_);
    type_select_->addWidget(new QLabel("Счет: "));
    type_select_->addWidget(acc_);
    type_select_->addWidget(new QLabel("Категория: "));
    type_select_->addWidget(target_);

    layout_->addLayout(date_select_);
    layout_->addLayout(type_select_);
    layout_->addWidget(show_data);
    layout_->addLayout(btns_layout_);
    layout_->addWidget(list_);

    setLayout(layout_);

    QFont font(MONOSPACE_FONT);

    connect(date_from_, &QDateEdit::dateChanged, [this]{
        date_to_->setMinimumDate(date_from_->date());
    });
    connect(date_to_, &QDateEdit::dateChanged, [this]{
        date_from_->setMaximumDate(date_to_->date());
    });

    list_->setFont(font);

    connect(list_, &MyList::itemDoubleClicked, this, &TransactionEditor::Edit);

    connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(FillTargets()));
    connect(show_data, SIGNAL(clicked()), this, SLOT(FillOps()));

    QPushButton* del_btn = new QPushButton("Удалить");
    del_btn->setFixedWidth(120);
    btns_layout_->addWidget(del_btn);

    QPushButton* edit_btn = new QPushButton("Редактировать");
    edit_btn->setFixedWidth(120);
    btns_layout_->addWidget(edit_btn);

    connect(del_btn, SIGNAL(clicked()), SLOT(Delete()));
    connect(edit_btn, SIGNAL(clicked()), SLOT(Edit()));
}

void TransactionEditor::FillData() {
    FillAccs();
    FillTargets();
    FillOps();
}

void TransactionEditor::FillAccs() {
    const auto& accs = wallet_.GetAccounts();

    acc_->clear();
    acc_->addItem("Любой");

    accs_idxs_.clear();
    accs_idxs_.reserve(accs.size() + 1);
    accs_idxs_.push_back(0);

    for (auto& acc : accs) {
        acc_->addItem(acc->GetName());
        accs_idxs_.push_back(acc->GetIndex());
    }
}

void TransactionEditor::FillTargets() {
    const auto cats = GetCats();

    target_->clear();
    cats_idxs_.clear();

    for (auto& cat : cats) {
        target_->addItem(cat.name);
        cats_idxs_.push_back(cat.idx);
    }
}

void TransactionEditor::FillOps() {
    list_->clear();
    trns_idxs_.clear();

    QDate from = date_from_->date();
    QDate to = date_to_->date();

    TransactShowType type = TransactShowType::All;

    switch (type_->currentIndex()) {
    case 0:
        type = TransactShowType::All;
        break;
    case 1:
        type = TransactShowType::Income;
        break;
    case 2:
        type = TransactShowType::Expense;
        break;
    case 3:
        type = TransactShowType::Transfer;
        break;
    }

    size_t acc = accs_idxs_.at(acc_->currentIndex());
    size_t cat = 0;

    if (type != TransactShowType::Transfer) {
        cat = cats_idxs_.at(target_->currentIndex());
    }

    auto ops = wallet_.GetTransactFiltred(from, to, type, acc, cat);

    Money inc, dec;

    for (auto& op : ops) {
        if (op->Type() == TransactionType::Income) {
            inc += op->Sum();
        } else if (op->Type() == TransactionType::Expense) {
            dec += op->Sum();
        }
        //list_->addItem(FormatOperation(wallet_, op) /*+ " (id: " + QString::number(op->Index()) + ")"*/ );
        bool transfer = op->Type() == TransactionType::Transfer;
        QString to;

        if (transfer) {
            to = wallet_.GetAccName(op->ToIdx());
        } else {
            to = wallet_.GetCategory(op->ToIdx())->GetName();
        }

        list_->addItem(op->Date(),
                       wallet_.GetAccName(op->AccountFromIdx()),
                       op->Sum().StringAbs(), to,
                       op->GetDescription(),
                       transfer,
                       op->Type() == TransactionType::Income
        );

        trns_idxs_.push_back(op->Index());
    }

    list_->SetSums(inc.StringAbs(), dec.StringAbs(), (inc - dec).String());
}

void TransactionEditor::Edit() {
    int idx = list_->currentRow();

    if (idx == -1) {
        return;
    }

    const Transaction* op = wallet_.FindTransact(trns_idxs_.at(idx));

    ModalEditor* me = new ModalEditor(op, wallet_);

    connect(me, SIGNAL(Updated()), SLOT(FillOps()));

    me->show();
}

void TransactionEditor::Delete() {
    int idx = list_->currentRow();

    if (idx == -1) {
        return;
    }

    size_t trns = trns_idxs_.at(idx);

    wallet_.DeleteTransaction(trns);
    FillOps();
}

const QVector<CategoryInfo> TransactionEditor::GetCats() const {
    switch(type_->currentIndex()) {
    case 0:
        target_->setEnabled(true);
        return wallet_.GetAllCategories();
    case 1:
        target_->setEnabled(true);
        return wallet_.GetCategories(true);
    case 2:
        target_->setEnabled(true);
        return wallet_.GetCategories(false);
    default:
        target_->setEnabled(false);
        return {};
    }
}

void TransactionEditor::showEvent(QShowEvent*) {
    FillData();
}

ModalEditor::ModalEditor(const Transaction *trns, Wallet &wallet, QWidget *parent)
                            : QWidget{parent}, trns_{trns}, wallet_{wallet}, type_{trns->Type()} {
    QFormLayout* layout_ = new QFormLayout;

    layout_->addRow("Дата:",  date_);
    layout_->addRow("Сумма:", sum_);
    layout_->addRow(acc_lab_,  acc_);

    sum_->setMinimum(0.01);
    sum_->setMaximum(1000000);
    sum_->setSuffix(" руб.");

    sum_->setValue(trns->Sum().Double());

    date_->setDate(trns->Date());
    date_->setMaximumDate(QDate::currentDate());

    desc_->setText(trns->GetDescription());

    if (type_ == TransactionType::Transfer) {
        acc_lab_->setText("Откуда:");
        cat_lab_->setText("Куда:");
        setWindowTitle("Редактировать перевод");
        FillAccs(acc_, trns_->AccountFromIdx());
        FillAccs(cat_, trns_->ToIdx());
        if (acc_->count() < 2 || cat_->count() < 2) {
            acc_->setEnabled(false);
            cat_->setEnabled(false);
        }
        connect(acc_, &QComboBox::currentIndexChanged, this, &ModalEditor::AccChanged);
        connect(cat_, &QComboBox::currentIndexChanged, this, &ModalEditor::CatChanged);
        layout_->addRow(cat_lab_,  cat_);
    } else {
        acc_lab_->setText("Счет:");
        cat_lab_->setText("Категория:");
        FillAccs(acc_, trns_->AccountFromIdx());
        setWindowTitle(type_ == TransactionType::Income ? "Редактировать доход" : "Редактировать расход");
        layout_->addRow(cat_lab_, cat_cont_lab_);
        cat_id_ = trns_->ToIdx();
        FillCatLab();
        cat_cont_lab_->setFrameStyle(QFrame::Box | QFrame::Plain);
        cat_cont_lab_->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(cat_cont_lab_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowMenu(QPoint)));
        connect(cat_cont_lab_, SIGNAL(clicked(QPoint)), this, SLOT(ShowMenu(QPoint)));
    }

    layout_->addRow("Описание:", desc_);

    QPushButton* done_ = new QPushButton("Готово");

    layout_->addRow("", done_);

    connect(done_, SIGNAL(clicked()), SLOT(ButtonPressed()));

    setLayout(layout_);
    setWindowModality(Qt::ApplicationModal);
    setWindowOpacity(0.9);
    setWindowFlag(Qt::Dialog);
    setFixedSize(300, 180);
}

void ModalEditor::closeEvent(QCloseEvent *event) {
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

void ModalEditor::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    } else if (event->key() == Qt::Key_Return) {
        ButtonPressed();
    }
}

void ModalEditor::AccChanged() {
    if (cat_->currentIndex() == acc_->currentIndex()) {
        if (acc_->currentIndex() == 0) {
            cat_->setCurrentIndex(1);
        } else {
            cat_->setCurrentIndex(0);
        }
    }
}

void ModalEditor::CatChanged() {
    if (cat_->currentIndex() == acc_->currentIndex()) {
        if (cat_->currentIndex() == 0) {
            acc_->setCurrentIndex(1);
        } else {
            acc_->setCurrentIndex(0);
        }
    }
}

void ModalEditor::ButtonPressed() {
    if (NoChanges()) {
        close();
        return;
    }

    Submit();
    emit Updated();
    delete this;
}

void ModalEditor::Submit() {
    transactions_manager::TransactionAdder adder;
    adder.from_idx    = acc_idx_.at(acc_->currentIndex());
    adder.to_idx      = type_ == TransactionType::Transfer ? acc_idx_.at(cat_->currentIndex()) : cat_id_;
    adder.date        = date_->date();
    adder.sum         = sum_->value();
    adder.type        = type_;
    adder.description = desc_->text().trimmed();

    wallet_.EditTransact(trns_->Index(), adder);
}

void ModalEditor::ShowMenu(const QPoint& point) {
    if (!menu_) {
        MakeMenu();
    }

    menu_->exec(point);
}

bool ModalEditor::NoChanges() {
    size_t new_from = acc_idx_.at(acc_->currentIndex());
    QDate  new_date = date_->date();
    Money new_sum   = sum_->value();
    size_t new_to;
    if (type_ == TransactionType::Transfer) {
        new_to = acc_idx_.at(cat_->currentIndex());
    } else {
        new_to = cat_id_;
    }
    QString new_desc = desc_->text().trimmed();

    size_t old_from  = trns_->AccountFromIdx();
    QDate  old_date  = trns_->Date();
    Money old_sum    = trns_->Sum();
    size_t old_to    = trns_->ToIdx();
    QString old_desc = trns_->GetDescription();

    return new_from == old_from && new_date == old_date && new_sum == old_sum && new_to == old_to && new_desc == old_desc;
}

void ModalEditor::FillAccs(QComboBox *cb, size_t idx) {
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

void ModalEditor::FillCatLab() {
    cat_cont_lab_->setText(wallet_.GetCatName(cat_id_));
}

void ModalEditor::MakeMenu() {
    menu_ = new QMenu(this);

    auto cat = wallet_.GetCategory(0);
    FillMenuChilds(*menu_, *cat);
}

void ModalEditor::FillMenuChilds(QMenu& menu, const Category& cat) {
    menu.addAction(cat.GetName(), [&cat, this]{
        cat_id_ = cat.GetId();
        FillCatLab();
    });

    const auto childs = cat.GetChilds();

    if (!childs.empty()) {
        QMenu* submenu = menu.addMenu("         ->");
        for (size_t child_id : childs) {
            auto subcat = wallet_.GetCategory(child_id);
            FillMenuChilds(*submenu, *subcat);
        }
    }

    menu.addSeparator();
}
