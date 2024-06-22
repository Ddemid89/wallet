#include "category_editor.h"

#include <QFormLayout>
#include <QCloseEvent>
#include <QMessageBox>
#include <unordered_set>

CategoryEditor::CategoryEditor(Wallet& wallet, size_t cat_id, QWidget* parent)
    : QWidget{parent}, wallet_{wallet}, cat_id_{cat_id}, cat_{wallet_.GetCategory(cat_id_)}
{
    setWindowTitle("Редактировать категорию");
    setWindowModality(Qt::ApplicationModal);
    setWindowOpacity(0.9);
    setWindowFlag(Qt::Dialog);
    setFixedSize(400, 160);

    QFormLayout* form_layout = new QFormLayout;
    form_layout->addRow("Название:", name_);
    form_layout->addRow("Родительская категория:", parent_);
    form_layout->addWidget(inc_);
    form_layout->addWidget(dec_);

    inc_->setChecked(cat_->isInc());
    dec_->setChecked(cat_->isDec());

    name_->setText(cat_->GetName());

    auto [inc, dec] = wallet_.GetCategoryChildsType(cat_id);

    if (inc) {
        inc_->setEnabled(false);
        inc_->setToolTip("Категория должна использоваться для доходов, поскольку одна из подкатегорий используется для доходов");
    }
    if (dec) {
        dec_->setEnabled(false);
        dec_->setToolTip("Категория должна использоваться для расходов, поскольку одна из подкатегорий используется для расходов");
    }

    if (!inc && !dec) {
        connect(inc_, &QCheckBox::stateChanged, this, &CategoryEditor::IncChange);
        connect(dec_, &QCheckBox::stateChanged, this, &CategoryEditor::DecChange);
    }

    QPushButton* done = new QPushButton("Готово");
    form_layout->addWidget(done);
    done->setFixedWidth(100);

    connect(done, &QPushButton::clicked, this, &CategoryEditor::Done);

    FillParents();
    setLayout(form_layout);
}

void CategoryEditor::closeEvent(QCloseEvent* event) {
    if (!HasChanges()) {
        delete this;
        return;
    }

    QMessageBox::StandardButton reply
        = QMessageBox::question(this, "Сохранить изменения", "Хотите сохранить внесенные изменения?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        SaveChanges();
    } else if (reply == QMessageBox::Cancel) {
        event->ignore();
        return;
    }

    delete this;
}

void CategoryEditor::IncChange() {
    bool inc_checked = inc_->isChecked();
    bool dec_checked = dec_->isChecked();

    if (!dec_checked && !inc_checked) {
        dec_->setChecked(true);
    }
}

void CategoryEditor::DecChange() {
    bool inc_checked = inc_->isChecked();
    bool dec_checked = dec_->isChecked();

    if (!dec_checked && !inc_checked) {
        inc_->setChecked(true);
    }
}

void CategoryEditor::Done() {
    if (!HasChanges()) {
        delete this;
        return;
    }
    SaveChanges();
    delete this;
}

void CategoryEditor::FillParents() {
    auto cats = wallet_.GetAllCategories();
    size_t parent_idx = cat_->GetParentId();

    auto childs = wallet_.GetCategoryChilds(cat_id_);

    std::unordered_set<size_t> bad_parents{childs.begin(), childs.end()};
    bad_parents.insert(cat_id_);

    for (auto& cat : cats) {
        if (bad_parents.count(cat.idx) == 0) {
            parent_->addItem(QString(cat.indent * 2, ' ') + cat.name);
            cats_idx_.push_back(cat.idx);

            if (cat.idx == parent_idx) {
                parent_->setCurrentIndex(parent_->count() - 1);
            }
        }
    }

}

bool CategoryEditor::HasChanges() {
    bool name_changed   = cat_->GetName()     != name_->text().trimmed();
    bool parent_changed = cat_->GetParentId() != cats_idx_.at(parent_->currentIndex());
    bool inc_changed    = cat_->isInc()       != inc_->isChecked();
    bool dec_changed    = cat_->isDec()       != dec_->isChecked();
    return name_changed || parent_changed || inc_changed || dec_changed;
}

void CategoryEditor::SaveChanges() {
    QString new_name   = name_->text().trimmed();
    size_t  new_parent = cats_idx_.at(parent_->currentIndex());
    bool    new_inc    = inc_->isChecked();
    bool    new_dec    = dec_->isChecked();

    wallet_.EditCategory(cat_id_, new_name, new_parent, new_inc, new_dec);

    emit Changed();
}
