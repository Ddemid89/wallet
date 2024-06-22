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
#include <QScrollBar>
#include <QLineEdit>

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
    Row(Wallet& wallet, QDate date = QDate::currentDate(), size_t acc = 0, size_t cat = 0, size_t op = 0, QWidget* parent = nullptr);

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
private:
    void FillAcs(QComboBox& cb);
    void FillCats(bool inc);

    Wallet& wallet_;

    QVector<int> acc_idx_;
    QVector<int> cat_idx_;

    QDateEdit* date_label_ = new QDateEdit;
    QDoubleSpinBox* sum_   = new QDoubleSpinBox;
    QComboBox* op_         = new QComboBox;
    QComboBox* acc_from_   = new QComboBox;
    QComboBox* acc_cat_to_ = new QComboBox;

    QPushButton* add_desc_ = new QPushButton("+ Описание");
    QString potential_description_;
};

class ModalDescriptionEditor : public QWidget {
    Q_OBJECT
public:
    ModalDescriptionEditor (Row& row, QWidget* parent = nullptr) : QWidget{parent}, row_{row} {
        QVBoxLayout* v_lyt = new QVBoxLayout;
        QHBoxLayout* h_lyt = new QHBoxLayout;

        setWindowModality(Qt::ApplicationModal);
        setWindowOpacity(0.9);
        setWindowFlag(Qt::Dialog);
        setFixedSize(330, 80);
        setWindowTitle("Добавить описание");

        h_lyt->addWidget(new QLabel("Описание:"));
        h_lyt->addWidget(new_desc_);
        v_lyt->addLayout(h_lyt);

        QPushButton* done = new QPushButton("Готово");
        v_lyt->addWidget(done);
        connect(done, &QPushButton::clicked, this, &ModalDescriptionEditor::Done);

        new_desc_->setText(row_.GetDescription());

        setLayout(v_lyt);
    }
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
    explicit CellWindow(Wallet& wallet, MainWindow& m_window, QWidget *parent = nullptr);
    void Deactivate() override;
    void showEvent(QShowEvent *event) override;
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
