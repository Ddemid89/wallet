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

    parent_id_ = cat_->GetParentId();

    parent_->setFrameStyle(QFrame::Box | QFrame::Plain);
    parent_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(parent_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowMenu(QPoint)));
    connect(parent_, SIGNAL(clicked(QPoint)), this, SLOT(ShowMenu(QPoint)));

    FillParentLab();
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

bool CategoryEditor::HasChanges() {
    bool name_changed   = cat_->GetName()     != name_->text().trimmed();
    bool parent_changed = cat_->GetParentId() != parent_id_;
    bool inc_changed    = cat_->isInc()       != inc_->isChecked();
    bool dec_changed    = cat_->isDec()       != dec_->isChecked();
    return name_changed || parent_changed || inc_changed || dec_changed;
}

void CategoryEditor::SaveChanges() {
    QString new_name   = name_->text().trimmed();
    size_t  new_parent = parent_id_;
    bool    new_inc    = inc_->isChecked();
    bool    new_dec    = dec_->isChecked();

    wallet_.EditCategory(cat_id_, new_name, new_parent, new_inc, new_dec);

    emit Changed();
}

void CategoryEditor::FillParentLab() {
    parent_->setText(wallet_.GetCategory(parent_id_)->GetName());
}

void CategoryEditor::ShowMenu(const QPoint& point) {
    if (!menu_) {
        MakeMenu();
    }

    menu_->exec(point);
}

void CategoryEditor::MakeMenu() {
    menu_ = new QMenu(this);

    auto cat = wallet_.GetCategory(0);
    FillMenuChilds(*menu_, *cat);
}

void CategoryEditor::FillMenuChilds(QMenu& menu, const Category& cat) {
    menu.addAction(cat.GetName(), [&cat, this]{
        parent_id_ = cat.GetId();
        FillParentLab();
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
