#include "add_category_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include "category_editor.h"

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

    tree_view_->setColumnCount(3);
    tree_view_->setHeaderLabels({"Название", "Доход", "Расход"});

    layout_->addWidget(back_);
    layout_->addWidget(lab);
    layout_->addWidget(tree_view_);
    layout_->addWidget(edit_, 0, Qt::AlignCenter);
    layout_->addWidget(lab2);
    layout_->addLayout(input_layout);
    layout_->addWidget(to_inc_);
    layout_->addWidget(to_dec_);
    layout_->addWidget(submit);

    connect(submit, SIGNAL(clicked()), this, SLOT(submit()));
    connect(edit_, &QPushButton::clicked, this, &AddCategoryWindow::Edit);
    connect(tree_view_, &QTreeWidget::doubleClicked, this, &AddCategoryWindow::Edit);

    connect(to_inc_, &QCheckBox::stateChanged, [this](){
        if (to_inc_->isChecked() == false) {
            to_dec_->setChecked(true);
        }
    });

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
    static const char* chars[] = {" ", "V"};
    QString name = name_->text();
    name = name.trimmed();

    if (name == "") {
        name_->setText("Введите название категории");
        return;
    }

    bool inc = to_inc_->isChecked();
    bool dec = to_dec_->isChecked();

    size_t parrent_idx = tree_view_->currentItem()->type();

    auto new_id = wallet_.AddCategory(name, parrent_idx, inc, dec);

    emit(m_window_.show_status("Категория \"" + name + "\" добавлена!"));

    QTreeWidgetItem* new_item = new QTreeWidgetItem({name, chars[inc], chars[dec]}, new_id);

    tree_view_->currentItem()->addChild(new_item);

    auto parent = new_item->parent();
    while (parent) {
        UpdateCat(parent);
        parent = parent->parent();
    }
}

void AddCategoryWindow::Edit() {
    auto cur_item = tree_view_->currentItem();

    if (!cur_item) {
        return;
    }

    size_t cat_id = cur_item->type();

    if (cat_id == 0) {
        return;
    }

    CategoryEditor* ce = new CategoryEditor(wallet_, cat_id);
    connect(ce, &CategoryEditor::Changed, this, &AddCategoryWindow::Update);
    ce->show();
}

void AddCategoryWindow::Update() {
    cats_.reset();
    size_t idx = tree_view_->currentItem()->type();
    FillCategories(idx);
}

void AddCategoryWindow::FillData(int n) {
    name_->clear();
    cats_.reset();

    FillCategories(n);
}

void AddCategoryWindow::showEvent(QShowEvent*) {
    FillData();
}

void AddCategoryWindow::FillCategories(size_t id) {
    if (!cats_.has_value()) {
        cats_ = wallet_.GetAllCategories();
    }

    const Category* main = wallet_.GetCategory(0);

    tree_view_->clear();
    tree_view_->setColumnWidth(0, 500);
    QTreeWidgetItem* root = new QTreeWidgetItem({"Все категории", "V", "V"});
    tree_view_->addTopLevelItem(root);

    if (id == 0) {
        tree_view_->setCurrentItem(root);
    }

    for (size_t cat_id : main->GetChilds()) {
        auto cat = wallet_.GetCategory(cat_id);
        AddChildToTree(root, cat, id);
    }

}

void AddCategoryWindow::AddChildToTree(QTreeWidgetItem* item, const Category* cat, size_t id) {
    static const char* chars[] = {" ", "V"};
    QTreeWidgetItem* new_item = new QTreeWidgetItem({cat->GetName(),
                                                     chars[cat->isInc()],
                                                     chars[cat->isDec()]
                                                    }, cat->GetId());

    item->addChild(new_item);
    if (cat->GetId() == id) {
        tree_view_->setCurrentItem(new_item);
    }

    for (size_t cat_id : cat->GetChilds()) {
        auto cur_cat = wallet_.GetCategory(cat_id);
        AddChildToTree(new_item, cur_cat, id);
    }
}

void AddCategoryWindow::UpdateCat(QTreeWidgetItem* item) {
    auto cat = wallet_.GetCategory(item->type());

    item->setText(0, cat->GetName());
    item->setText(1, (cat->isInc() ? "V" : " "));
    item->setText(2, (cat->isDec() ? "V" : " "));
}
