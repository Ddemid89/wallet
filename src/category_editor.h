#ifndef CATEGORY_EDITOR_H
#define CATEGORY_EDITOR_H

#include <QWidget>
#include "model.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QVector>
#include <QCheckBox>
#include <QMenu>
#include "clickable_label.h"

class CategoryEditor : public QWidget {
    Q_OBJECT
public:
    CategoryEditor(Wallet& wallet, size_t cat_id, QWidget* parent = nullptr);
signals:
    void Changed();
private slots:
    void closeEvent(QCloseEvent *event) override;
    void IncChange();
    void DecChange();
    void Done();
    void ShowMenu(const QPoint& point);
private:
    bool HasChanges();
    void SaveChanges();

    void FillParentLab();
    void MakeMenu();
    void FillMenuChilds(QMenu& menu, const Category& info);

    QLineEdit* name_        = new QLineEdit;
    ClickableLabel* parent_ = new ClickableLabel;
    QCheckBox* inc_         = new QCheckBox(" - для доходов");
    QCheckBox* dec_         = new QCheckBox(" - для расходов");

    Wallet& wallet_;
    size_t cat_id_;
    const Category* cat_;

    QMenu* menu_ = nullptr;
    size_t parent_id_;

    QVector<size_t> cats_idx_;
};

#endif // CATEGORY_EDITOR_H
