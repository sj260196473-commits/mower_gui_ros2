#ifndef POINT_H
#define POINT_H

/*结构体点*/
template <class T>
struct point {
    /// 构造零点，默认坐标为 (0, 0)。
    inline point() : x(0), y(0) {}

    /// 使用给定 x/y 坐标构造二维点。
    inline point(T _x, T _y) : x(_x), y(_y) {}
    T x, y;
};

/*点的 + 运算符的重载*/
template <class T>
/// 返回两个二维点逐坐标相加后的结果。
inline point<T> operator+(const point<T> &p1, const point<T> &p2) {
    return point<T>(p1.x + p2.x, p1.y + p2.y);
}

/*点的 - 运算符的重载*/
template <class T>
/// 返回两个二维点逐坐标相减后的结果。
inline point<T> operator-(const point<T> &p1, const point<T> &p2) {
    return point<T>(p1.x - p2.x, p1.y - p2.y);
}

/*点的 * 运算符的重载  乘以一个数*/
template <class T>
/// 返回点坐标同时乘以标量后的结果。
inline point<T> operator*(const point<T> &p, const T &v) {
    return point<T>(p.x * v, p.y * v);
}

/*点的 * 运算符的重载  乘以一个数*/
template <class T>
/// 返回标量左乘点坐标后的结果。
inline point<T> operator*(const T &v, const point<T> &p) {
    return point<T>(p.x * v, p.y * v);
}

/*点的 * 运算符的重载  point的内积*/
template <class T>
/// 返回两个二维点向量的内积。
inline T operator*(const point<T> &p1, const point<T> &p2) {
    return p1.x * p2.x + p1.y * p2.y;
}

#endif // POINT_H
