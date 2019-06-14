#ifndef UTILS_h
#define UTILS_h
//Generic template to enable << operator on streams
template <class T>
inline Print &operator << (Print & stream, T arg)
{
    stream.print(arg);
    return stream;
}

struct _BASED {
    long val;
    int  base;
    _BASED(long v, int b) : val(v), base(b){ }
};

struct _BYTE_CODE {
    byte val;
    _BYTE_CODE(byte v) : val(v)
    { }
};

#define _BYTE(a) _BYTE_CODE(a)

inline Print &operator << (Print & obj, const _BYTE_CODE &arg)
{
    obj.write(arg.val);
    return obj;
}

#define _HEX(a) _BASED(a, HEX)
#define _DEC(a) _BASED(a, DEC)
#define _OCT(a) _BASED(a, OCT)
#define _BIN(a) _BASED(a, BIN)

inline Print &operator << (Print & obj, const _BASED &arg)
{
    obj.print(arg.val, arg.base);
    return obj;
}

struct _double {
    double val;
    int    digits;
    _double(double v, int d) : val(v), digits(d){ }
};

inline Print &operator << (Print & obj, const _double &arg)
{
    obj.print(arg.val, arg.digits);
    return obj;
}

enum _EndLineCode { endl };

inline Print &operator << (Print & obj, _EndLineCode arg)
{
    obj.println();
    return obj;
}

#endif