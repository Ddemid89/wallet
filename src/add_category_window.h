#ifndef ADDCATEGORYWINDOW_H
#define ADDCATEGORYWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <optional>
#include <QLineEdit>
#include <QCheckBox>

#include "mainwindow.h"
#include "model.h"

class AddCategoryWindow : public Widgets {
    Q_OBJECT
public:
    explicit AddCategoryWindow(Wallet& wallet, MainWindow& m_window, QWidget* parent = nullptr);

signals:

public slots:
    void submit();
    void Edit();
private:
    void FillData(int n = 0);

    void showEvent(QShowEvent *event) override;
    void FillCategories(int n = 0);

    std::optional<QVector<CategoryInfo>> cats_;

    QCheckBox* to_inc_ = new QCheckBox;
    QCheckBox* to_dec_ = new QCheckBox;
    QPushButton* edit_  = new QPushButton("Редактировать");

    QListWidget* list_view = new QListWidget;
    QLineEdit* name_ = new QLineEdit;
};

#endif // ADDCATEGORYWINDOW_H
