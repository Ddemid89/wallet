#include "binary_loader.h"

QDataStream& operator<<(QDataStream& out, const model_representation::TransactionsPart& trs) {
    out << trs.id << trs.trs;
    return out;
}
QDataStream& operator>>(QDataStream& in, model_representation::TransactionsPart& trs) {
    in >> trs.id >> trs.trs;
    return in;
}
QDataStream& operator<<(QDataStream& out, const model_representation::TransactionRepresentation& trs) {
    out << trs.date
        << trs.from_id
        << trs.id
        << trs.sum
        << trs.to_id
        << trs.type;
    return out;
}
QDataStream& operator>>(QDataStream& in, model_representation::TransactionRepresentation& trs) {
    in >> trs.date
       >> trs.from_id
       >> trs.id
       >> trs.sum
       >> trs.to_id
       >> trs.type;
    return in;
}

namespace binary_loader {

} // namespace binary_loader
