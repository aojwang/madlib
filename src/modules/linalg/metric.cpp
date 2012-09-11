/* ----------------------------------------------------------------------- *//**
 *
 * @file metric.cpp
 *
 * @brief Metric operations
 *
 *//* ----------------------------------------------------------------------- */

#include <dbconnector/dbconnector.hpp>

#include "metric.hpp"

namespace madlib {

namespace modules {

namespace linalg {

std::tuple<Index, double>
closestColumnAndDistance(
    const MappedMatrix& inMatrix,
    const MappedColumnVector& inVector,
    FunctionHandle& inMetric) {

    Index closestColumn = 0;
    double minDist = std::numeric_limits<double>::infinity();

    for (Index i = 0; i < inMatrix.cols(); ++i) {
        double currentDist
            = inMetric(MappedColumnVector(inMatrix.col(i)), inVector)
                .getAs<double>();
        if (currentDist < minDist) {
            closestColumn = i;
            minDist = currentDist;
        }
    }

    return std::tuple<Index, double>(closestColumn, minDist);
}

/**
 * @brief Compute the minimum distance between a vector and any column of a
 *     matrix
 *
 * This function calls a user-supplied function, for which it does not do
 * garbage collection. It is therefore meant to be called only constantly many
 * times before control is returned to the backend.
 */
AnyType
closest_column::run(AnyType& args) {
    MappedMatrix M = args[0].getAs<MappedMatrix>();
    MappedColumnVector x = args[1].getAs<MappedColumnVector>();
    FunctionHandle dist = args[2].getAs<FunctionHandle>()
        .unsetFunctionCallOptions(FunctionHandle::GarbageCollectionAfterCall);

    std::tuple<Index, double> result = closestColumnAndDistance(M, x, dist);
/*    std::tuple<Index, double> result(0, std::numeric_limits<double>::infinity());
    for (Index i = 0; i < M.cols(); ++i) {
        double currentDist = (M.col(i) - x).squaredNorm();
        if (currentDist < std::get<1>(result)) {
            std::get<0>(result) = i;
            std::get<1>(result) = currentDist;
        }
    }
*/
    AnyType tuple;
    return tuple
        << static_cast<int16_t>(std::get<0>(result))
        << std::get<1>(result);
}


AnyType
norm2::run(AnyType& args) {
    return static_cast<double>(args[0].getAs<MappedColumnVector>().norm());
}

AnyType
norm1::run(AnyType& args) {
    return static_cast<double>(args[0].getAs<MappedColumnVector>().lpNorm<1>());
}

AnyType
dist_norm2::run(AnyType& args) {
    // FIXME: it would be nice to declare this as a template function (so it
    // works for dense and sparse vectors), and the C++ AL takes care of the
    // rest...
    MappedColumnVector x = args[0].getAs<MappedColumnVector>();
    MappedColumnVector y = args[1].getAs<MappedColumnVector>();

    return static_cast<double>( (x-y).norm() );
}

AnyType
dist_norm1::run(AnyType& args) {
    // FIXME: it would be nice to declare this as a template function (so it
    // works for dense and sparse vectors), and the C++ AL takes care of the
    // rest...
    MappedColumnVector x = args[0].getAs<MappedColumnVector>();
    MappedColumnVector y = args[1].getAs<MappedColumnVector>();

    return static_cast<double>( (x-y).lpNorm<1>() );
}

AnyType
squared_dist_norm2::run(AnyType& args) {
    // FIXME: it would be nice to declare this as a template function (so it
    // works for dense and sparse vectors), and the C++ AL takes care of the
    // rest...
    MappedColumnVector x = args[0].getAs<MappedColumnVector>();
    MappedColumnVector y = args[1].getAs<MappedColumnVector>();

    return static_cast<double>( (x-y).squaredNorm() );
}

AnyType
squared_dist_norm1::run(AnyType& args) {
    // FIXME: it would be nice to declare this as a template function (so it
    // works for dense and sparse vectors), and the C++ AL takes care of the
    // rest...
    MappedColumnVector x = args[0].getAs<MappedColumnVector>();
    MappedColumnVector y = args[1].getAs<MappedColumnVector>();
    double l1norm = (x-y).lpNorm<1>();

    return l1norm * l1norm;
}

AnyType
squared_angle::run(AnyType& args) {
    MappedColumnVector x = args[0].getAs<MappedColumnVector>();
    MappedColumnVector y = args[1].getAs<MappedColumnVector>();

    double cosine = dot(x, y) / (x.norm() * y.norm());
    if (cosine > 1)
        cosine = 1;
    else if (cosine < -1)
        cosine = -1;
    double angle = std::acos(cosine);
    return angle * angle;
}

AnyType
squared_tanimoto::run(AnyType& args) {
    MappedColumnVector x = args[0].getAs<MappedColumnVector>();
    MappedColumnVector y = args[1].getAs<MappedColumnVector>();

    // Note that this is not a metric in general!
    double dotProduct = dot(x,y);
    double tanimoto = x.squaredNorm() + y.squaredNorm();
    tanimoto = (tanimoto - 2 * dotProduct) / (tanimoto - dotProduct);
    return tanimoto * tanimoto;
}

} // namespace linalg

} // namespace modules

} // namespace regress
