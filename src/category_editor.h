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
private:
    void FillParents();
    bool HasChanges();
    void SaveChanges();

    QLineEdit* name_   = new QLineEdit;
    QComboBox* parent_ = new QComboBox;
    QCheckBox* inc_    = new QCheckBox(" - для доходов");
    QCheckBox* dec_    = new QCheckBox(" - для расходов");

    Wallet& wallet_;
    size_t cat_id_;
    const Category* cat_;

    QVector<size_t> cats_idx_;
};

#endif // CATEGORY_EDITOR_H
