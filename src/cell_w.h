#ifndef CELL_W_H
#define CELL_W_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDate>
#include <QVector>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include "model.h"
#include "mainwindow.h"
#include "clickable_label.h"
#include <QScrollBar>
#include <QLineEdit>
#include <QMenu>
#include <QStackedWidget>

class MenuWrapper {
public:
    MenuWrapper(const Wallet& wallet, QWidget* parent) {
        inc_menu_ = new QMenu{parent};
        dec_menu_ = new QMenu{parent};

        auto cat = wallet.GetCategory(0);
        FillMenuChilds(wallet, *inc_menu_, *cat, true);
        FillMenuChilds(wallet, *dec_menu_, *cat, false);
    }

    QMenu* GetMenu(size_t* field, QLabel* lab, bool inc) {
        cat_id_field_ = field;
        lab_ = lab;
        if (inc) {
            return inc_menu_;
        }
        return dec_menu_;
    }
private:

    void FillMenuChilds(const Wallet& wallet, QMenu& menu, const Category& cat, bool inc) {
        menu.addAction(cat.GetName(), [&cat, this]{
            *cat_id_field_ = cat.GetId();
            lab_->setText(cat.GetName());
        });

        const auto childs = cat.GetChilds();

        bool is_first = true;

        QMenu* submenu;
        for (size_t child_id : childs) {
            auto subcat = wallet.GetCategory(child_id);
            if ((subcat->isInc() && inc) || (subcat->isDec() && !inc)) {
                if (is_first) {
                    submenu = menu.addMenu("         ->");
                }

                is_first = false;

                FillMenuChilds(wallet, *submenu, *subcat, inc);
            }
        }

        menu.addSeparator();
    }

    QMenu* inc_menu_;
    QMenu* dec_menu_;
\
    size_t* cat_id_field_;
    QLabel* lab_;
};

const int RECENT_LINES = 10;

using NamesIndex = std::unordered_map<size_t, QString>;

enum class OpType{
    Inc,
    Dec,
    Transfer
};

struct CellTransaction {
    QDate date;
    OpType type;
    double sum;
    size_t from;
    size_t to;
    QString description;
};

class LabelRow : public QWidget {
    Q_OBJECT
public:
    LabelRow(QWidget* parent = nullptr);
    void SetTransaction(const Transaction* trns, const NamesIndex& acc_names, const NamesIndex& cat_names, QColor);
private:
    QLabel* date_;
    QLabel* op_;
    QLabel* sum_;
    QLabel* from_;
    QLabel* to_;
    QLabel* desc_;
};

class Row : public QWidget {
    Q_OBJECT
public:
    Row(Wallet& wallet, MenuWrapper& menu_wrapper, QDate date = QDate::currentDate(), size_t acc = 0, size_t cat = 0, size_t op = 0, QWidget* parent = nullptr);

    CellTransaction Get();
    QDate GetDate() const;
    size_t GetAccId() const;
    size_t GetCatId() const;
    size_t GetOp() const;
    QString GetDescription() const;
    void SetDescription(const QString& new_desc_);

private slots:
    void ChangeOp();
    void ChangeAcc();
    void ChangeCat();
    void ShowMenu(const QPoint&);
private:
    void FillAcs(QComboBox& cb);
    void FillCats(bool inc);

    Wallet& wallet_;

    QVector<int> acc_idx_;

    MenuWrapper& menu_wrapper_;
    size_t cat_id_ = 0;

    QDateEdit* date_label_ = new QDateEdit;
    QDoubleSpinBox* sum_   = new QDoubleSpinBox;
    QComboBox* op_         = new QComboBox;
    QComboBox* acc_from_   = new QComboBox;

    QStackedWidget* acc_cat_widget_ = new QStackedWidget;
    QComboBox*      acc_cat_to_ = new QComboBox;
    ClickableLabel* acc_cat_to_label_ = new ClickableLabel("Все категории");

    QPushButton* add_desc_ = new QPushButton("+ Описание");
    QString potential_description_;
};

class ModalDescriptionEditor : public QWidget {
    Q_OBJECT
public:
    ModalDescriptionEditor (Row& row, QWidget* parent = nullptr);
private slots:
    void Done();
    void closeEvent(QCloseEvent*) override;
private:
    Row& row_;
    QLineEdit* new_desc_ = new QLineEdit;
};

class CellWindow : public Widgets {
    Q_OBJECT
public:
    CellWindow(Wallet& wallet, MainWindow& m_window, QWidget *parent = nullptr);
    void Deactivate() override;
    void showEvent(QShowEvent *event) override;
    void wheelEvent(QWheelEvent* event);
private slots:
    void AddRow();
    void PopRow();
    void FillRecent();
    void Done();


private:
    void AddTransaction(CellTransaction&& ct);

    void FillRecentLine(size_t line_idx, const Transaction* trns, QColor);

    void AddAllTransactions();
    void DeleteRows();

    void GetAndFillRecent();

    MenuWrapper* menu_wrapper_ = nullptr;

    QVector<Row*> rows_;

    QScrollBar* scroll_        = new QScrollBar;
    QScrollArea* s_area_       = new QScrollArea;
    QWidget* container_        = new QWidget;
    QVBoxLayout* main_layout_  = new QVBoxLayout;
    QVBoxLayout* cells_layout_ = new QVBoxLayout;

    QVector<LabelRow*> recent_ops_lines_;
    NamesIndex acc_id_to_name_;
    NamesIndex cat_id_to_name_;
    QVector<const Transaction*> recent_ops_;
};

#endif // CELL_W_H
