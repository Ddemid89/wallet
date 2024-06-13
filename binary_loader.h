#ifndef BINARYLOADER_H
#define BINARYLOADER_H

#include <QFile>
#include <QDataStream>
#include "transactions_manager.h"


QDataStream& operator<<(QDataStream& out, const model_representation::TransactionsPart& trs);
QDataStream& operator>>(QDataStream& in, model_representation::TransactionsPart& trs);
QDataStream& operator<<(QDataStream& out, const model_representation::TransactionRepresentation& trs);
QDataStream& operator>>(QDataStream& in, model_representation::TransactionRepresentation& trs);

namespace binary_loader {
class BinaryLoader : public transactions_manager::LoaderInterface {
public:
    BinaryLoader(const QString& dir) : dir_(dir) {}
    model_representation::TransactionsPart Load(transactions_manager::Date date) override {
        QString path = GetPath(date);
        QFile file(path);

        model_representation::TransactionsPart result;

        if(!file.open(QIODevice::ReadOnly)) {
            result.id = 0;
            qDebug() << "No such file '" << path << "'";
            return result;
        }

        QDataStream in(&file);

        in >> result;

        file.close();

        return result;
    }
    void Save(transactions_manager::Date date, model_representation::TransactionsPart&& transacts) override {
        QString path = GetPath(date);
        QFile file(path);

        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to open or create file '" << path << "'";
            throw std::runtime_error("Failed to open or create file");
        }

        QDataStream out(&file);

        out << std::move(transacts);

        file.close();
    }
private:
    QString GetPath(transactions_manager::Date date) {
        return dir_ + "/" + date.String() + ".bin";
    }

    QString dir_;
};
} // namespace binary_loader

#endif // BINARYLOADER_H
